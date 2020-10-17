#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "duktape.h"
#include "esp_log.h"
#include "protocol_examples_common.h"
#include "execute_js.h"

#define TAG "DUKTAPE"



// static duk_ret_t native_adder(duk_context *ctx) {
// 	int i;
// 	int n = duk_get_top(ctx);  /* #args */
// 	double res = 0.0;

// 	for (i = 0; i < n; i++) {
// 		res += duk_to_number(ctx, i);
// 	}

// 	duk_push_number(ctx, res);
// 	return 1;  /* one return value */
// }


void app_main(void)
{
  nvs_flash_init();
  tcpip_adapter_init();
  esp_event_loop_create_default();
  example_connect();


 xTaskCreate(mqtt_task, "mqtt_task", 1024 * 10, NULL, 6, NULL);
 xTaskCreate(execute_js, "duc_calc", 1024 * 10, NULL, 5, NULL);
  
}
