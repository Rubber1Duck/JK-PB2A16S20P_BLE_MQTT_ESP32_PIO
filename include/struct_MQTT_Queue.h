#ifndef STRUCT_MQTT_QUEUE_H
#define STRUCT_MQTT_QUEUE_H

struct PublishMessage {
    char topic[128];  // Adjust size as needed
    char payload[48]; // Adjust size as needed
};

#endif // STRUCT_MQTT_QUEUE_H
