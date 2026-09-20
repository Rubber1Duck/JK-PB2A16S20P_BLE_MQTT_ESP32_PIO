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
    bool defaultRetained; // default value for the "retained" checkbox of fields in this category
};

const MqttPublishCategory *getMqttPublishCategories(size_t &categoryCount);
bool isMqttPublishFieldEnabled(const char *topic);
void setMqttPublishFieldEnabled(const char *categoryId, const char *suffix, bool enabled);
bool getMqttPublishFieldEnabled(const char *categoryId, const char *suffix);

// Per-field "retained" flag, configurable via the MQTT config web page
bool isMqttPublishFieldRetained(const char *topic);
void setMqttPublishFieldRetained(const char *categoryId, const char *suffix, bool retained);
bool getMqttPublishFieldRetained(const char *categoryId, const char *suffix);

// True for fields that are only published while debug_flg is enabled
bool isMqttPublishFieldDebugOnly(const char *categoryId, const char *suffix);

#endif
