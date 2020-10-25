#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ductape/duktape.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "execute_js.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "driver/gpio.h"

#define TAG "execute js"

void ductape_err(void *udata, const char *message)
{
    ESP_LOGE(TAG, "ERR %s", message);
    //vTaskDelay(portMAX_DELAY);
}

void *js_malloc(void *udata, duk_size_t size)
{
    return malloc(size);
}

void *js_realloc(void *udata, void *ptr, duk_size_t size)
{
    return realloc(ptr, size);
}

void js_free(void *udata, void *ptr)
{
    free(ptr);
}

static duk_ret_t native_print(duk_context *ctx)
{
    duk_push_string(ctx, " ");
    duk_insert(ctx, 0);
    duk_join(ctx, duk_get_top(ctx) - 1);
    printf("%s\n", duk_safe_to_string(ctx, -1));
    return 0;
}

typedef struct timer_callback_payload_t
{
    duk_context *ctx;
    esp_timer_handle_t timer_handler;
    int timeoutIndex;
    bool should_delete_timer;
    bool timer_was_deleted;
} timer_callback_payload_t;
int timeoutIndex = 0;
timer_callback_payload_t *timer_callback_payload_cache[20];

static void timer_callback(void *args)
{
    timer_callback_payload_t *timer_callback_payload = args;
    duk_context *ctx = timer_callback_payload->ctx;
    duk_get_global_string(ctx, "_callbackFunc");
    duk_get_prop_index(ctx, -1, timer_callback_payload->timeoutIndex);
    duk_call(ctx, 0);
    duk_pop(ctx);
    duk_pop(ctx);
    if (timer_callback_payload->should_delete_timer)
    {
        esp_timer_delete(timer_callback_payload->timer_handler);
        timer_callback_payload->timer_was_deleted = true;
        //free(timer_callback_payload);
    }
}

static duk_ret_t setTimeout(duk_context *ctx)
{
    duk_require_function(ctx, 0);
    long timer_val = duk_require_uint(ctx, 1);
    timer_callback_payload_t *timer_callback_payload = malloc(sizeof(timer_callback_payload_t));
    timer_callback_payload->ctx = ctx;
    timer_callback_payload->timeoutIndex = timeoutIndex++;
    timer_callback_payload->should_delete_timer = true;
    timer_callback_payload_cache[timer_callback_payload->timeoutIndex] = timer_callback_payload;
    duk_get_global_string(ctx, "_callbackFunc");
    duk_dup(ctx, 0);
    duk_put_prop_index(ctx, -2, timer_callback_payload->timeoutIndex);

    const esp_timer_create_args_t my_timer_args = {
        .callback = &timer_callback,
        .name = "a timer",
        .arg = timer_callback_payload};

    ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_callback_payload->timer_handler));
    ESP_ERROR_CHECK(esp_timer_start_once(timer_callback_payload->timer_handler, timer_val * 1000));
    duk_push_int(ctx, timer_callback_payload->timeoutIndex);

    return 1;
}

static duk_ret_t setInterval(duk_context *ctx)
{
    duk_require_function(ctx, 0);
    long timer_val = duk_require_uint(ctx, 1);
    timer_callback_payload_t *timer_callback_payload = malloc(sizeof(timer_callback_payload_t));
    timer_callback_payload->ctx = ctx;
    timer_callback_payload->timeoutIndex = timeoutIndex++;
    timer_callback_payload->should_delete_timer = false;
    timer_callback_payload_cache[timer_callback_payload->timeoutIndex] = timer_callback_payload;

    duk_get_global_string(ctx, "_callbackFunc");
    duk_dup(ctx, 0);
    duk_put_prop_index(ctx, -2, timer_callback_payload->timeoutIndex);

    const esp_timer_create_args_t my_timer_args = {
        .callback = &timer_callback,
        .name = "a timer",
        .arg = timer_callback_payload};

    ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_callback_payload->timer_handler));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_callback_payload->timer_handler, timer_val * 1000));
    duk_push_int(ctx, timer_callback_payload->timeoutIndex);

    return 1;
}

duk_ret_t config_gpio(duk_context *ctx)
{
    // printf("config at 0 %d\n",duk_get_int(ctx, 0));
    // printf("config at -1 %d\n",duk_get_int(ctx, -1));
    // printf("config at -2 %d\n",duk_get_int(ctx, -2));
    // printf("config at -3 %d\n",duk_get_int(ctx, -3));
    int pin = duk_get_int(ctx, 0);
    int direction = duk_get_int(ctx, -1);
     printf("pin %d direction %d\n",pin, direction);
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, direction);
     return 0;
}

duk_ret_t set_gpio(duk_context *ctx)
{
// printf("set dir at 0 %d\n",duk_get_int(ctx, 0));
// printf("set dir at -1 %d\n",duk_get_int(ctx, -1));
// printf("set dir at -2 %d\n",duk_get_int(ctx, -2));
// printf("set dir at -3 %d\n",duk_get_int(ctx, -3));

    int pin = duk_get_int(ctx, 0);
    int value = duk_get_int(ctx, -1);
    printf("pin %d value %d\n",pin, value);
    gpio_set_level(pin, value);
    return 0;
}

void register_bindings(duk_context *ctx)
{
    //timer functions
    duk_push_array(ctx);
    duk_put_global_string(ctx, "_callbackFunc");
    duk_push_c_function(ctx, setTimeout, 2);
    duk_put_global_string(ctx, "setTimeout");
    duk_push_c_function(ctx, setInterval, 2);
    duk_put_global_string(ctx, "setInterval");

    //print functions
    duk_push_c_function(ctx, native_print, 1);
    duk_put_global_string(ctx, "print");

    //gpio
    duk_push_c_function(ctx, config_gpio, 2);
    duk_put_global_string(ctx, "config_gpio");
    duk_push_c_function(ctx, set_gpio, 2);
    duk_put_global_string(ctx, "set_gpio");
}

duk_context *ctx;
void execute_js(void *params)
{
    gpio_pad_select_gpio(0);
    gpio_pad_select_gpio(2);
    gpio_pad_select_gpio(4);
    gpio_set_direction(0,GPIO_MODE_OUTPUT);
     gpio_set_direction(2,GPIO_MODE_OUTPUT);
     gpio_set_direction(4,GPIO_MODE_OUTPUT);
    gpio_set_level(0,0);
    gpio_set_level(2,1);
    gpio_set_level(4,0);

    while (true)
    {
        message_command_payload_t message_command_payload;
        if (xQueueReceive(receivedCommand, &message_command_payload, portMAX_DELAY))
        {
            for (int i = 0; i < timeoutIndex ; i++)
            {
               if(!timer_callback_payload_cache[i]->timer_was_deleted)
               {
                   printf("fee timer %d\n",i);
                   esp_timer_stop(timer_callback_payload_cache[i]->timer_handler);
                   esp_timer_delete(timer_callback_payload_cache[i]->timer_handler);
                   free(timer_callback_payload_cache[i]);
               }
            }
            timeoutIndex = 0;
            
            duk_destroy_heap(ctx);
            ctx = NULL;
            ctx = duk_create_heap(js_malloc, js_realloc, js_free, NULL, ductape_err);
            register_bindings(ctx);
            duk_eval_string(ctx, message_command_payload.payload);
            free(message_command_payload.payload);
        }
    }
    duk_destroy_heap(ctx);
    vTaskDelete(NULL);
}