#ifndef PUBLISH_H
#define PUBLISH_H
#include <Arduino.h>
#include "macros.h"
#include "mqtt_handler.h"
#include "struct_MQTT_Queue.h"

void publish_init();

// Define the queue handle
QueueHandle_t publishQueue;

extern PubSubClient mqtt_client;
extern void setState(String key, String value, bool publish);

#endif // PUBLISH_H
