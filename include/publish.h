#ifndef PUBLISH_H
#define PUBLISH_H
#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include "mqtt_handler.h"
#include "struct_MQTT_Queue.h"

// sizeof(PublishMessage) shrank from 257 to ~167 Bytes (see struct_MQTT_Queue.h) after right-sizing
// topic/payload to the actually configured TOPIC_BASE/DEVICENAME/longest topic - raised from 255 to 400
// so the non-PSRAM queue budget stays roughly the same as before (255*257 ~= 400*167 Bytes).
#define PUBLISH_QUEUE_COUNT 400u // Default/fallback queue depth for boards without PSRAM,
// be careful with too high values as it can cause stability issues with the MQTT client if the queue is filling up.
// On boards with PSRAM (BOARD_HAS_PSRAM), publish_init() raises the actual depth at runtime -
// see publishQueueCount for the value actually in use, monitor the max used queue size via MQTT and adjust if needed
void publish_init();
bool ensureRawPublishInfraInitialized();
bool rawDataPoolAllocSlot(uint16_t *slotIndex, TickType_t waitTicks);
void rawDataPoolFreeSlot(uint16_t slotIndex);
const uint8_t *rawDataPoolSlotPtr(uint16_t slotIndex);
uint16_t rawDataPoolFreeCount();

// Define the queue handle
extern QueueHandle_t publishQueue;
extern QueueHandle_t rawPublishQueue;
extern UBaseType_t publishQueueCount; // actual publish queue depth in use (may exceed PUBLISH_QUEUE_COUNT on PSRAM boards)

extern PubSubClient mqtt_client;
extern void setState(String key, String value, bool publish);

#endif // PUBLISH_H
