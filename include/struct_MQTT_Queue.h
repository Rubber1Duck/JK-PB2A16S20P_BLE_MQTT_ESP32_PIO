#ifndef STRUCT_MQTT_QUEUE_H
#define STRUCT_MQTT_QUEUE_H

#include <stdint.h>
#include <cstddef>
#include "config.h" // TOPIC_BASE, DEVICENAME

constexpr size_t cstrlen(const char *s) { return *s ? 1 + cstrlen(s + 1) : 0; }

// Runtime/build-configured lengths of the topic prefix shared by every published topic.
constexpr size_t MQTT_TOPIC_BASE_LEN = cstrlen(TOPIC_BASE);
constexpr size_t MQTT_DEVICENAME_LEN = cstrlen(DEVICENAME);

// Longest category+suffix tail appended after TOPIC_BASE+DEVICENAME across all published topics
// (see the field tables in mqtt_publish_config.cpp), e.g. "/data/temperatures/temp_sensor_absent_mask".
constexpr size_t MQTT_LONGEST_TOPIC_TAIL_LEN = sizeof("/data/temperatures/temp_sensor_absent_mask") - 1;

// Buffer size for a full MQTT topic string, sized from the actually configured TOPIC_BASE/DEVICENAME
// instead of a fixed guess, so it shrinks/grows automatically with the build configuration.
constexpr size_t MQTT_TOPIC_BUFFER_SIZE = MQTT_TOPIC_BASE_LEN + MQTT_DEVICENAME_LEN + MQTT_LONGEST_TOPIC_TAIL_LEN + 1;

// Longest raw value published: entries in uart_protocol_number_str/can_protocol_number_str (parser.cpp)
// go up to 64 chars (e.g. "Deye Low-voltage hybrid inverter CAN communication protocol V1.0"),
// which exceeds the device/config string fields (<=16 chars) and formatted numbers.
constexpr size_t MQTT_MAX_VALUE_LEN = 64;
// Literal JSON characters of {"time":<epoch_ms>,"value":"<value>"} excluding the timestamp digits and the value itself.
constexpr size_t MQTT_JSON_TEMPLATE_LITERAL_LEN = sizeof("{\"time\":,\"value\":\"\"}") - 1;
constexpr size_t MQTT_TIMESTAMP_MAX_DIGITS = 13; // epoch milliseconds fits in 13 digits until year 2286
constexpr size_t MQTT_PAYLOAD_BUFFER_SIZE = MQTT_JSON_TEMPLATE_LITERAL_LEN + MQTT_TIMESTAMP_MAX_DIGITS + MQTT_MAX_VALUE_LEN + 1;

struct PublishMessage
{
    char topic[MQTT_TOPIC_BUFFER_SIZE];
    char payload[MQTT_PAYLOAD_BUFFER_SIZE]; // holds the JSON-wrapped {"time":...,"value":...} payload
    bool retain = false;
};

// Dedicated fixed pool for large debug rawdata payloads.
constexpr uint16_t RAWDATA_POOL_SLOT_SIZE = 512;
// 16 slots ~= 4s buffer at ~4 raw frames/sec when PUBLISH_DELAY=0.
constexpr uint16_t RAWDATA_POOL_SLOT_COUNT = 16;

struct RawPublishMessage
{
    char topic[MQTT_TOPIC_BUFFER_SIZE];
    uint16_t payload_len;
    uint16_t slot_index;
};

#endif // STRUCT_MQTT_QUEUE_H
