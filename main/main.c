#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "ductape/duktape.h"
#include "esp_log.h"
#include "protocol_examples_common.h"
#include "execute_js.h"

#define TAG "DUKTAPE"

void ductape_err(void *udata, const char *message);
void *js_malloc(void *udata, duk_size_t size);
void *js_realloc(void *udata, void *ptr, duk_size_t size);
void js_free(void *udata, void *ptr);
void register_bindings(duk_context *ctx);

static duk_ret_t return_number_1(duk_context *ctx)
{
  int a = duk_get_int(ctx, -1);
  int b = duk_get_int(ctx, -2);
  printf("IN -1 %d\n", a);
  printf("IN -2 %d\n", b);
  duk_push_number(ctx, a + b);
  return 1;
}

static duk_ret_t return_string(duk_context *ctx)
{

  duk_push_string(ctx, "hello");
  return 1;
}

static duk_ret_t native_print(duk_context *ctx)
{
  duk_push_string(ctx, " ");
  duk_insert(ctx, 0);
  duk_join(ctx, duk_get_top(ctx) - 1);
  printf("%s\n", duk_safe_to_string(ctx, -1));
  return 0;
}

void testTask(void *params)
{
  duk_context *ctx = duk_create_heap(js_malloc, js_realloc, js_free, NULL, ductape_err);
  register_bindings(ctx);

  // duk_eval_string(ctx, "setTimeout(function(){print('hello')}, 1000)");
  // duk_eval_string(ctx, "setTimeout(function(){print('hello2')}, 1500)");
  // duk_eval_string(ctx, "var x = 0; setInterval(function(){print('hello ' + x++)}, 1000);");

  // duk_eval_string(ctx, "var y = 0; setInterval(function(){print('second interval ' + y++)}, 1500);");
  duk_eval_string(ctx, "var y = 0; setInterval(function(){print('second interval ' + y++)}, 1500);");
  
  vTaskDelay(portMAX_DELAY);
}

void app_main(void)
{
    

  //xTaskCreate(testTask, "testTask", 1024 * 20, NULL, 5, NULL);

    nvs_flash_init();
    tcpip_adapter_init();
    esp_event_loop_create_default();
    example_connect();

   xTaskCreate(mqtt_task, "mqtt_task", 1024 * 10, NULL, 6, NULL);
   xTaskCreate(execute_js, "duc_calc", 1024 * 30, NULL, 5, NULL);
}
