#ifndef execute_js_h
#define execute_js_h

typedef struct message_command_payload_t
{
    char *payload;
    size_t payload_size;

} message_command_payload_t;

extern xQueueHandle sendCommand;
extern xQueueHandle receivedCommand;

void execute_js(void *params);
void mqtt_task(void *params);

#endif