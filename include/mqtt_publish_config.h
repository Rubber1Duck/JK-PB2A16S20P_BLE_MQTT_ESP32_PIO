#ifndef MQTT_PUBLISH_CONFIG_H
#define MQTT_PUBLISH_CONFIG_H

#include <Arduino.h>

struct MqttPublishField
{
    const char *suffix;
    const char *label;
};

struct MqttPublishCategory
{
    const char *id;
    const char *label;
    const MqttPublishField *fields;
    size_t fieldCount;
};

const MqttPublishCategory *getMqttPublishCategories(size_t &categoryCount);
bool isMqttPublishFieldEnabled(const char *topic);
void setMqttPublishFieldEnabled(const char *categoryId, const char *suffix, bool enabled);
bool getMqttPublishFieldEnabled(const char *categoryId, const char *suffix);

#endif
