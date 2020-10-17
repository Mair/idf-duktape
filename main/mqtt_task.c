#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "execute_js.h"

#define TAG "MQTT"



xQueueHandle sendCommand;
xQueueHandle receivedCommand;

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
      message_command_payload_t message_command_payload; 
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, "jsexecute", 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        message_command_payload.payload_size = event->data_len;
        message_command_payload.payload = malloc(event->data_len + 1);
        memset(message_command_payload.payload, 0, event->data_len +1);
        memcpy(message_command_payload.payload,event->data, event->data_len);
        xQueueSend(receivedCommand,&message_command_payload,pdMS_TO_TICKS(300));
        break;
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_task(void *params)
{
    sendCommand = xQueueCreate(10, sizeof(message_command_payload_t));
    receivedCommand = xQueueCreate(10, sizeof(message_command_payload_t));

    esp_mqtt_client_config_t mqttConfig = {
        .uri = "mqtt://test.mosquitto.org:1883"};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    message_command_payload_t message_command_payload;
    while (true)
    {
        if (xQueueReceive(sendCommand, &message_command_payload, portMAX_DELAY))
        {
        }
    }
}