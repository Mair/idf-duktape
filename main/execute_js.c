#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "duktape.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "execute_js.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#define TAG "execute js"

void ductape_err(void *udata, const char *message)
{
    ESP_LOGE(TAG, "ERR %s", message);
    vTaskDelay(portMAX_DELAY);
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

void register_bindings(duk_context *ctx)
{
    duk_push_c_function(ctx, native_print, 1);
    duk_put_global_string(ctx, "print");
}

void initialize_js(duk_context *ctx)
{
    esp_vfs_spiffs_conf_t esp_vfs_spiffs_conf = {
        .base_path = "/spiffs",
        .format_if_mount_failed = true,
        .partition_label = NULL,
        .max_files = 5};
    esp_vfs_spiffs_register(&esp_vfs_spiffs_conf);

    FILE *file = fopen("/spiffs/init.js", "r");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to load file from spiffs");
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    char *buffer = malloc(length + 1);
    memset(buffer, 0, length + 1);
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, length, file);
    fclose(file);
    esp_vfs_spiffs_unregister(NULL);

    duk_eval_string_noresult(ctx, "new Function('return this')().Duktape = Object.create(Duktape);");
    //duk_module_duktape_init(ctx); // Initialize the duktape module functions.
    //esp32_duktape_stash_init(ctx); // Initialize the stash environment.

    duk_push_string(ctx, "init.js");
    duk_compile_lstring_filename(ctx, 0, buffer, length);

    free(buffer);
    //int rc = duk_peval_lstring(ctx, fileData, fileSize);
    int rc = duk_pcall(
        ctx,
        0 // Number of arguments
    );
    if (rc != 0)
    {
        ESP_LOGE(TAG, "INIT ERROR");
    }
    duk_pop(ctx);
}

void execute_js(void *params)
{
    duk_context *ctx = duk_create_heap(js_malloc, js_realloc, js_free, NULL, ductape_err);
    initialize_js(ctx);
    register_bindings(ctx);

    while (true)
    {
        message_command_payload_t message_command_payload;
        if (xQueueReceive(receivedCommand, &message_command_payload, portMAX_DELAY))
        {
            duk_eval_string(ctx, message_command_payload.payload);
            free(message_command_payload.payload);
        }
    }
    duk_destroy_heap(ctx);
    vTaskDelete(NULL);
}