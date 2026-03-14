#ifndef PUBLISH_H
#define PUBLISH_H
#include <Arduino.h>
#include "macros.h"
#include "mqtt_handler.h"
#include "struct_MQTT_Queue.h"

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
