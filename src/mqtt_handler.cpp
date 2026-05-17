#include "mqtt_handler.h"
#include "publish.h"
#include "parser.h"

#include <ctype.h>

constexpr unsigned long RECONNECT_DELAY = 2000;
unsigned long lastReconnectAttempt = 0;
constexpr int MQTT_BUFFER_SIZE = 2048;

bool mqtt_buffer_maxed = false;

const char *mqtt_server = MQTT_SERVER;
const char *mqtt_username = MQTT_USERNAME;
const char *mqtt_password = MQTT_PASSWORD;
const char *mqtt_devicename = DEVICENAME;
#ifdef USE_TLS
const int mqtt_tls_port = MQTT_TLS_PORT;

#else
const int mqtt_port = MQTT_PORT;
#endif

String mqtt_main_topic = String(TOPIC_BASE);
String mqttname = mqtt_main_topic + mqtt_devicename;

String willTopic = mqttname + String("/status/status");
String willMessage = "offline";
byte willQoS = 0;
boolean willRetain = true;
uint32_t reconnect_attempts = 0;

// Define the map to store key-value pairs
std::map<String, String> stateMap;

// Define toMqttQueue mutex
std::mutex mqttQueueMutex;

// Pre-computed MQTT topics to avoid repeated allocations
String topic_debug_active;
String topic_debug_active_full;
String topic_publish_delay;
String topic_min_pub_time;
String topic_publish_interval;

static uint32_t rawdata_enqueued_count = 0;
static uint32_t rawdata_drop_init_failed_count = 0;
static uint32_t rawdata_drop_oversize_count = 0;
static uint32_t rawdata_drop_pool_exhausted_count = 0;
static uint32_t rawdata_drop_queue_full_count = 0;

#ifdef USE_HA_DISCOVERY
#ifndef HA_DISCOVERY_PREFIX
#define HA_DISCOVERY_PREFIX "homeassistant"
#endif

namespace
{
struct DiscoveryEntity
{
    const char *key;
    const char *name;
    const char *topicSuffix;
    const char *unit;
    const char *deviceClass;
    const char *stateClass;
    const char *icon;
};

const DiscoveryEntity DISCOVERY_ENTITIES[] = {
    {"status", "JKBMS Status", "status/status", nullptr, nullptr, nullptr, "mdi:heart-pulse"},
    {"uptime", "JKBMS Uptime", "status/uptime", nullptr, nullptr, nullptr, "mdi:clock-outline"},
    {"ipaddress", "JKBMS IP Address", "status/ipaddress", nullptr, nullptr, nullptr, "mdi:ip-network"},
    {"wifi_rssi", "JKBMS WiFi RSSI", "status/wifi_rssi", "dBm", "signal_strength", "measurement", "mdi:wifi"},
    {"ble_connection", "JKBMS BLE Connection", "status/ble_connection", nullptr, nullptr, nullptr, "mdi:bluetooth-connect"},
    {"device_name", "JKBMS Device Name", "device/device_name", nullptr, nullptr, nullptr, "mdi:battery"},
    {"sw_version", "JKBMS SW Version", "device/sw_version", nullptr, nullptr, nullptr, "mdi:tag-text-outline"},
    {"runtime_fmt", "JKBMS Runtime", "device/uptime_fmt", nullptr, nullptr, nullptr, "mdi:timer-outline"},
    {"battery_voltage", "JKBMS Battery Voltage", "data/battery_voltage", "V", "voltage", "measurement", "mdi:flash"},
    {"battery_current", "JKBMS Battery Current", "data/battery_current", "A", "current", "measurement", "mdi:current-dc"},
    {"battery_power", "JKBMS Battery Power", "data/battery_power", "W", "power", "measurement", "mdi:lightning-bolt"},
    {"battery_soc", "JKBMS SOC", "data/battery_soc", "%", "battery", "measurement", "mdi:battery-medium"},
    {"battery_soh", "JKBMS SOH", "data/battery_soh", "%", nullptr, "measurement", "mdi:heart"},
    {"capacity_remaining", "JKBMS Capacity Remaining", "data/battery_capacity_remaining", "Ah", nullptr, "measurement", "mdi:battery-clock"},
    {"total_runtime_fmt", "JKBMS Total Runtime", "data/battery_total_runtime_fmt", nullptr, nullptr, nullptr, "mdi:calendar-clock"},
    {"temp_mosfet", "JKBMS MOS Temperature", "data/temperatures/temp_mosfet", "C", "temperature", "measurement", "mdi:thermometer"},
    {"temp_sensor1", "JKBMS Battery Temperature 1", "data/temperatures/temp_sensor1", "C", "temperature", "measurement", "mdi:thermometer"},
    {"alarm_raw", "JKBMS Alarm Raw", "data/alarms/alarm_raw", nullptr, nullptr, nullptr, "mdi:alarm-light"},
    {"alarm_mask", "JKBMS Alarm Mask", "data/alarms/alarms_mask", nullptr, nullptr, nullptr, "mdi:shield-alert"},
    {"cell_count", "JKBMS Cell Count", "config/cell_count", nullptr, nullptr, "measurement", "mdi:counter"},
};

String jsonEscape(const String &in)
{
    String out;
    out.reserve(in.length() + 8);
    for (size_t i = 0; i < in.length(); i++)
    {
        char c = in[i];
        if (c == '\\')
            out += "\\\\";
        else if (c == '"')
            out += "\\\"";
        else if (c == '\n')
            out += "\\n";
        else if (c == '\r')
            out += "\\r";
        else if (c == '\t')
            out += "\\t";
        else
            out += c;
    }
    return out;
}

String sanitizeEntityId(const char *in)
{
    String out;
    if (in == nullptr)
    {
        return "jkbms";
    }

    for (size_t i = 0; in[i] != '\0'; i++)
    {
        unsigned char c = static_cast<unsigned char>(in[i]);
        if (isalnum(c) || c == '_' || c == '-')
        {
            out += static_cast<char>(tolower(c));
        }
        else
        {
            out += '_';
        }
    }

    if (out.length() == 0)
    {
        out = "jkbms";
    }
    return out;
}

bool publishHomeAssistantDiscovery()
{
    const String deviceId = sanitizeEntityId(mqtt_devicename);
    const String availabilityTopic = mqttname + "/status/status";

    for (const auto &entity : DISCOVERY_ENTITIES)
    {
        String objectId = deviceId + "_" + entity.key;
        String configTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + objectId + "/config";
        String stateTopic = mqttname + "/" + entity.topicSuffix;

        String payload = "{";
        payload += "\"name\":\"" + jsonEscape(entity.name) + "\",";
        payload += "\"uniq_id\":\"" + jsonEscape(objectId) + "\",";
        payload += "\"stat_t\":\"" + jsonEscape(stateTopic) + "\",";
        payload += "\"avty_t\":\"" + jsonEscape(availabilityTopic) + "\",";
        payload += "\"pl_avail\":\"online\",";
        payload += "\"pl_not_avail\":\"offline\",";

        if (entity.unit != nullptr)
        {
            payload += "\"unit_of_meas\":\"" + jsonEscape(entity.unit) + "\",";
        }
        if (entity.deviceClass != nullptr)
        {
            payload += "\"dev_cla\":\"" + jsonEscape(entity.deviceClass) + "\",";
        }
        if (entity.stateClass != nullptr)
        {
            payload += "\"stat_cla\":\"" + jsonEscape(entity.stateClass) + "\",";
        }
        if (entity.icon != nullptr)
        {
            payload += "\"ic\":\"" + jsonEscape(entity.icon) + "\",";
        }

        payload += "\"dev\":{";
        payload += "\"ids\":[\"" + jsonEscape(deviceId) + "\"],";
        payload += "\"name\":\"" + jsonEscape(String("JK BMS ") + mqtt_devicename) + "\",";
        payload += "\"mf\":\"JK\",";
        payload += "\"mdl\":\"JK BLE Listener\"";
        payload += "}}";

        if (!mqtt_client.publish(configTopic.c_str(), payload.c_str(), true))
        {
            DEBUG_PRINTLN("HA discovery publish failed: " + configTopic);
            return false;
        }
    }

    DEBUG_PRINTLN("HA discovery published.");
    return true;
}
} // namespace
#endif // USE_HA_DISCOVERY

String formatUptime(time_t uptime)
{
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int minutes = (uptime % 3600) / 60;
    int secs = uptime % 60;

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%dd %02d:%02d:%02d", days, hours, minutes, secs);
    return String(buffer);
}

bool toMqttQueue(const char *topic, const char *payload)
{
    std::lock_guard<std::mutex> lock(mqttQueueMutex);
    if (mqtt_client.state() != MQTT_CONNECTED || !isWifiConnected)
    {
        return false; // Wait until MQTT is connected before pushing topics to publish queue
    }

    if (topic == nullptr || payload == nullptr)
    {
        return false;
    }

    PublishMessage queue_in;
    strncpy(queue_in.topic, topic, sizeof(queue_in.topic) - 1);
    queue_in.topic[sizeof(queue_in.topic) - 1] = '\0';
    strncpy(queue_in.payload, payload, sizeof(queue_in.payload) - 1);
    queue_in.payload[sizeof(queue_in.payload) - 1] = '\0';
    if (xQueueSend(publishQueue, &queue_in, 0) != pdTRUE)
    {
        String failMsg = "Failed to send message to queue: " + String(topic);
        DEBUG_PRINTLN(failMsg);
        return false;
    }

    return true;
}

bool toMqttQueue(String topic, String payload)
{
    return toMqttQueue(topic.c_str(), payload.c_str());
}

void toMqttQueueRawData(String topic, const char *payload, size_t payloadLen)
{
    if (!ensureRawPublishInfraInitialized())
    {
        rawdata_drop_init_failed_count++;
        DEBUG_PRINTLN("Rawdata infra not available, dropping payload");
        return;
    }

    std::lock_guard<std::mutex> lock(mqttQueueMutex);
    if (mqtt_client.state() != MQTT_CONNECTED || !isWifiConnected)
    {
        return; // Wait until MQTT is connected before pushing topics to publish queue
    }

    if (payload == nullptr || payloadLen == 0)
    {
        return;
    }

    if (payloadLen > RAWDATA_POOL_SLOT_SIZE)
    {
        rawdata_drop_oversize_count++;
        String failMsg = "Rawdata payload too large for pool: " + String(payloadLen);
        DEBUG_PRINTLN(failMsg);
        return;
    }

    uint16_t slotIndex = 0;
    if (!rawDataPoolAllocSlot(&slotIndex, 0))
    {
        rawdata_drop_pool_exhausted_count++;
        DEBUG_PRINTLN("Rawdata pool exhausted, dropping payload");
        return;
    }

    const uint8_t *slotPtrConst = rawDataPoolSlotPtr(slotIndex);
    if (slotPtrConst == nullptr)
    {
        rawDataPoolFreeSlot(slotIndex);
        return;
    }

    uint8_t *slotPtr = const_cast<uint8_t *>(slotPtrConst);
    memcpy(slotPtr, payload, payloadLen);

    RawPublishMessage queue_in;
    strncpy(queue_in.topic, topic.c_str(), sizeof(queue_in.topic) - 1);
    queue_in.topic[sizeof(queue_in.topic) - 1] = '\0';
    queue_in.payload_len = static_cast<uint16_t>(payloadLen);
    queue_in.slot_index = slotIndex;

    if (xQueueSend(rawPublishQueue, &queue_in, 0) != pdTRUE)
    {
        rawdata_drop_queue_full_count++;
        rawDataPoolFreeSlot(slotIndex);
        String failMsg = "Failed to send rawdata message to queue: " + String(topic);
        DEBUG_PRINTLN(failMsg);
        return;
    }

    rawdata_enqueued_count++;
}

void setState(const char *key, const char *value, bool publish)
{
    if (key == nullptr || value == nullptr)
    {
        return;
    }

    stateMap[key] = value;
    if (publish)
    {
        char fullTopic[192];
        snprintf(fullTopic, sizeof(fullTopic), "%s/status/%s", mqttname.c_str(), key);
        toMqttQueue(fullTopic, value);
    }
}

void setState(String key, String value, bool publish)
{
    setState(key.c_str(), value.c_str(), publish);
}

static void setStateU32(const char *key, uint32_t value, bool publish)
{
    char valueBuf[16];
    snprintf(valueBuf, sizeof(valueBuf), "%lu", static_cast<unsigned long>(value));
    setState(key, valueBuf, publish);
}

void publishStates()
{
    for (const auto &kv : stateMap)
    {
        char fullTopic[192];
        snprintf(fullTopic, sizeof(fullTopic), "%s/status/%s", mqttname.c_str(), kv.first.c_str());
        toMqttQueue(fullTopic, kv.second.c_str());
    }
    vTaskDelay(25 / portTICK_PERIOD_MS);
}

void publishStatesTask(void *pvParameters)
{
    while (true)
    {
        // update uptime before publishing states
        setState("uptime", formatUptime(esp_timer_get_time() / 1000000), false);
        setStateU32("rawpool_free_slots", rawDataPoolFreeCount(), false);
        setStateU32("rawpool_capacity", RAWDATA_POOL_SLOT_COUNT, false);
        setStateU32("rawdata_enqueued", rawdata_enqueued_count, false);
        setStateU32("rawdata_drop_init_failed", rawdata_drop_init_failed_count, false);
        setStateU32("rawdata_drop_oversize", rawdata_drop_oversize_count, false);
        setStateU32("rawdata_drop_pool_exhausted", rawdata_drop_pool_exhausted_count, false);
        setStateU32("rawdata_drop_queue_full", rawdata_drop_queue_full_count, false);
        publishStates();
        // Publish parameter topics periodically
        toMqttQueue(topic_debug_active, debug_flg ? "true" : "false");
        toMqttQueue(topic_debug_active_full, debug_flg_full ? "true" : "false");
        toMqttQueue(topic_publish_delay, String(publish_delay).c_str());
        toMqttQueue(topic_min_pub_time, String(min_pub_time).c_str());
        toMqttQueue(topic_publish_interval, String(publishInterval).c_str());
        // Warte min_pub_time Sekunden, aber in kleineren Schritten
        uint32_t total_delay = min_pub_time * 1000;
        uint32_t chunk_delay = 1000; // 1 Sekunde Chunks
        for (uint32_t i = 0; i < total_delay; i += chunk_delay)
        {
            vTaskDelay(chunk_delay / portTICK_PERIOD_MS);
        }
    }
}

// Callback function header; The callback function header needs to
//  be declared before the PubSubClient constructor and the
//  actual callback defined afterwards.
//  This ensures the client reference in the callback function
//  is valid. (see pubsubclient example "mqtt_publish_in_callback")
void MQTTCallback(char *topic, byte *payload, unsigned int length);

#ifdef USE_TLS
WiFiClientSecure secure_wifi_client;
PubSubClient mqtt_client(mqtt_server, mqtt_tls_port, MQTTCallback, secure_wifi_client);
#else
WiFiClient wifi_client;
PubSubClient mqtt_client(mqtt_server, mqtt_port, MQTTCallback, wifi_client);
#endif

// handle Subscriptions - optimized version
void MQTTCallback(char *topic, byte *payload, unsigned int length)
{
    // Early return pattern - check each topic and return immediately after handling

    // Check debugging_active
    if (strcmp(topic, topic_debug_active.c_str()) == 0)
    {
        String cmd = String((char *)payload, length);
        debug_flg = (cmd == "true");
        write_setting("debug_flg", debug_flg);
        return;
    }

    // Check debugging_active_full
    if (strcmp(topic, topic_debug_active_full.c_str()) == 0)
    {
        String cmd = String((char *)payload, length);
        debug_flg_full = (cmd == "true");
        write_setting("debug_flg_full", debug_flg_full);
        return;
    }

    // Check publish_delay
    if (strcmp(topic, topic_publish_delay.c_str()) == 0)
    {
        char payloadStr[length + 1];
        memcpy(payloadStr, payload, length);
        payloadStr[length] = '\0';

        bool isNumeric = true;
        for (unsigned int i = 0; i < length; i++)
        {
            if (!isdigit(payload[i]))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            uint16_t value = atoi(payloadStr);
            write_setting("publish_delay", value);
        }
        return;
    }

    // Check min_publish_time
    if (strcmp(topic, topic_min_pub_time.c_str()) == 0)
    {
        char payloadStr[length + 1];
        memcpy(payloadStr, payload, length);
        payloadStr[length] = '\0';

        bool isNumeric = true;
        for (unsigned int i = 0; i < length; i++)
        {
            if (!isdigit(payload[i]))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric){
            uint16_t value = atoi(payloadStr);
            write_setting("min_pub_time", value);
        }
        return;
    }

    // Check publish_interval
    if (strcmp(topic, topic_publish_interval.c_str()) == 0) {
        char payloadStr[length + 1];
        memcpy(payloadStr, payload, length);
        payloadStr[length] = '\0';

        bool isNumeric = true;
        for (unsigned int i = 0; i < length; i++) {
            if (!isdigit(payload[i])) {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric) {
            uint16_t value = atoi(payloadStr);
            write_setting("publishInterval", value);
        }
        return;
    }
}

uint32_t lastMQTTreconnectAttempt = 0;

// Reconnect to MQTT broker
boolean mqtt_reconnect()
{
    mqtt_buffer_maxed = mqtt_client.setBufferSize(MQTT_BUFFER_SIZE);
    reconnect_attempts++;

#ifdef USE_RANDOM_CLIENT_ID
    String random_client_id = mqtt_devicename + String("-");
    random_client_id += String(random(0xffff), HEX);
#else
    String random_client_id = mqtt_devicename;
#endif

    DEBUG_PRINTLN("Attempting MQTT connection... " + random_client_id + " (Attempt " + String(reconnect_attempts) + ")");
    // Attempt to reconnect to the MQTT broker
    if (mqtt_client.connect(random_client_id.c_str(), mqtt_username, mqtt_password, willTopic.c_str(), willQoS, willRetain, willMessage.c_str(), true))
    {

        int ErrorCnt = 0;
        String debug_flg_status = debug_flg ? "true" : "false";
        mqtt_client.publish(topic_debug_active.c_str(), debug_flg_status.c_str()) || ErrorCnt++;
        mqtt_client.subscribe(topic_debug_active.c_str()) || ErrorCnt++; // debug_flg

        String debug_flg_full_log_status = debug_flg_full ? "true" : "false";
        mqtt_client.publish(topic_debug_active_full.c_str(), debug_flg_full_log_status.c_str()) || ErrorCnt++;
        mqtt_client.subscribe(topic_debug_active_full.c_str()) || ErrorCnt++; // debug_flg_full

        mqtt_client.publish(topic_publish_delay.c_str(), String(publish_delay).c_str()) || ErrorCnt++;
        mqtt_client.subscribe(topic_publish_delay.c_str()) || ErrorCnt++; // publish_delay

        mqtt_client.publish(topic_min_pub_time.c_str(), String(min_pub_time).c_str()) || ErrorCnt++;
        mqtt_client.subscribe(topic_min_pub_time.c_str()) || ErrorCnt++; // min_pub_time

        mqtt_client.publish(topic_publish_interval.c_str(), String(publishInterval).c_str()) || ErrorCnt++;
        mqtt_client.subscribe(topic_publish_interval.c_str()) || ErrorCnt++; // publish_interval

    #ifdef USE_HA_DISCOVERY
        publishHomeAssistantDiscovery() || ErrorCnt++;
    #endif

        // Re-publish cached records so HA sensors receive values immediately after reconnect.
        republishCachedRecords(mqtt_devicename);

        if (ErrorCnt > 0)
        {
            String errorMsg = "Connected to broker but initial publish or subscriptions failed, error count: " + String(ErrorCnt);
            DEBUG_PRINTLN(errorMsg);
            mqtt_client.disconnect();
            DEBUG_PRINTLN("Disconnected from broker.");
            return false;
        }
        else
        {
            DEBUG_PRINTLN("Connected to broker, initial publish and subscriptions successful.");
        }
#ifdef USELED
        // Send LED_ON state to the LED task
        set_led(LedState::LED_FLASH);
#endif
    }
    return mqtt_client.connected();
}

// MQTT Check
void mqtt_loop()
{
    if (!mqtt_client.connected())
    {
        if (!isWifiConnected)
        {
            static uint32_t lastWifiWaitLog = 0;
            uint32_t nowMs = millis();
            if (nowMs - lastWifiWaitLog > 5000)
            {
                DEBUG_PRINTLN("Waiting for WiFi connection before attempting MQTT reconnect...");
                lastWifiWaitLog = nowMs;
            }
            return;
        }
    
        unsigned long now = millis();
        if (now - lastReconnectAttempt > RECONNECT_DELAY)
        { // 5 seconds delay
            DEBUG_PRINTLN("MQTT client not connected, attempting to reconnect... (Attempt " + String(reconnect_attempts) + ")");
            lastReconnectAttempt = now;
            if (mqtt_reconnect())
            {
                lastReconnectAttempt = 0;
                DEBUG_PRINTLN("MQTT Reconnected.");
            }
        }
    }
    else
    {
        mqtt_client.loop();
    }
}

void mqtt_init()
{
    // Initialize pre-computed topic strings once
    topic_debug_active = mqttname + "/parameter/debugging_active";
    topic_debug_active_full = mqttname + "/parameter/debugging_active_full";
    topic_publish_delay = mqttname + "/parameter/publish_delay";
    topic_min_pub_time = mqttname + "/parameter/min_publish_time";
    topic_publish_interval = mqttname + "/parameter/publish_interval";

    if (mqtt_reconnect())
    {
        DEBUG_PRINTLN("MQTT Connected.");
        setState("version", VERSION, false);
        setState("ipaddress", WiFi.localIP().toString(), false);
        setState("ble_connection", "startup", false);
        setState("status", "online", false);
                
        // Create the task to call publishStates() every min_publish_time seconds
        // Stack erhöht von 2048 auf 4096 für Stabilität
        xTaskCreate(publishStatesTask, "Publish States Task", 4096, NULL, 1, NULL);

    }
    else
    {
        DEBUG_PRINTLN("MQTT Connect failed.");
    }
}