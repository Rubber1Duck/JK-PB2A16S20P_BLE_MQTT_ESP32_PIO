#ifndef PUBLISH_H
#define PUBLISH_H
#include <Arduino.h>
#include "macros.h"
#include "mqtt_handler.h"
#include "struct_MQTT_Queue.h"

#define PUBLISH_QUEUE_COUNT 255u // Publish queue can hold 255 messages, adjust as needed,
// be careful with too high values as it can cause stability issues with the MQTT client if the queue is filling up
// (max value is 255 due to uint8_t queue size tracking), monitor the max used queue size via MQTT and adjust if needed
void publish_init();
bool ensureRawPublishInfraInitialized();
bool rawDataPoolAllocSlot(uint16_t *slotIndex, TickType_t waitTicks);
void rawDataPoolFreeSlot(uint16_t slotIndex);
const uint8_t *rawDataPoolSlotPtr(uint16_t slotIndex);
uint16_t rawDataPoolFreeCount();

// Define the queue handle
extern QueueHandle_t publishQueue;
extern QueueHandle_t rawPublishQueue;

extern PubSubClient mqtt_client;
extern void setState(String key, String value, bool publish);

#endif // PUBLISH_H
