#ifndef STRUCT_MQTT_QUEUE_H
#define STRUCT_MQTT_QUEUE_H

#include <stdint.h>

struct PublishMessage
{
    char topic[128];  // Adjust size as needed
    char payload[48]; // Adjust size as needed
};

// Dedicated fixed pool for large debug rawdata payloads.
constexpr uint16_t RAWDATA_POOL_SLOT_SIZE = 512;
// 16 slots ~= 4s buffer at ~4 raw frames/sec when PUBLISH_DELAY=0.
constexpr uint16_t RAWDATA_POOL_SLOT_COUNT = 16;

struct RawPublishMessage
{
    char topic[128];
    uint16_t payload_len;
    uint16_t slot_index;
};

#endif // STRUCT_MQTT_QUEUE_H
