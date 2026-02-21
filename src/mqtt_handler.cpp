#include "mqtt_handler.h"

constexpr unsigned long RECONNECT_DELAY = 5000;
unsigned long lastReconnectAttempt = 0;
constexpr int MQTT_BUFFER_SIZE = 2048;

bool mqtt_buffer_maxed = false;

const char *mqtt_server = MQTT_SERVER;
const char *mqtt_username = MQTT_USERNAME;
const char *mqtt_password = MQTT_PASSWORD;
const char *mqtt_devicename = DEVICENAME;
#ifdef USE_TLS
const int mqtt_tls_port = MQTT_TLS_PORT;
const char *root_ca_cert PROGMEM = MQTT_ROOT_CA_CERT;
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

void toMqttQueue(String topic, String payload)
{
    std::lock_guard<std::mutex> lock(mqttQueueMutex);
    if (mqtt_client.state() != MQTT_CONNECTED || !isWifiConnected)
    {
        return; // Wait until MQTT is connected before pushing topics to publish queue
    }
    PublishMessage queue_in;
    strncpy(queue_in.topic, topic.c_str(), sizeof(queue_in.topic) - 1);
    queue_in.topic[sizeof(queue_in.topic) - 1] = '\0';
    strncpy(queue_in.payload, payload.c_str(), sizeof(queue_in.payload) - 1);
    queue_in.payload[sizeof(queue_in.payload) - 1] = '\0';
    if (xQueueSend(publishQueue, &queue_in, 0) != pdTRUE)
    {
        String failMsg = "Failed to send message to queue: " + String(topic);
        DEBUG_PRINTLN(failMsg);
    }
}

void setState(String key, String value, bool publish)
{
    stateMap[key] = value;
    if (publish)
    {
        String fullTopic = mqttname + "/status/" + key;
        toMqttQueue(fullTopic.c_str(), value.c_str());
    }
}

void publishStates()
{
    for (const auto &kv : stateMap)
    {
        String fullTopic = mqttname + "/status/" + kv.first;
        String payload = kv.second;
        toMqttQueue(fullTopic.c_str(), payload.c_str());
    }
    vTaskDelay(25 / portTICK_PERIOD_MS);
}

void publishStatesTask(void *pvParameters)
{
    while (true)
    {
        // update uptime before publishing states
        setState("uptime", formatUptime(esp_timer_get_time() / 1000000), false);
        publishStates();
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
        else
        {
            toMqttQueue(topic, "5"); // Default to 5 seconds if invalid
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

        if (isNumeric)
        {
            uint16_t value = atoi(payloadStr);
            write_setting("min_pub_time", value);
        }
        else
        {
            toMqttQueue(topic, "300"); // Default to 300 seconds if invalid
        }
        return;
    }
}

// Reconnect to MQTT broker
boolean mqtt_reconnect()
{
    while (!isWifiConnected)
    {
        DEBUG_PRINTLN("Waiting for WiFi connection before attempting MQTT reconnect...");
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
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
        DEBUG_PRINTLN("MQTT client not connected, attempting to reconnect... (Attempt " + String(reconnect_attempts) + ")");
        unsigned long now = millis();
        if (now - lastReconnectAttempt > RECONNECT_DELAY)
        { // 5 seconds delay

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