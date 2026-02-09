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

// Define the mutex
SemaphoreHandle_t mqttMutex;
SemaphoreHandle_t serialMutex = NULL;  // Wird in main.cpp oder mqtt_init() erstellt

// Define the map to store key-value pairs
std::map<String, String> stateMap;

String formatUptime(time_t uptime) {
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int minutes = (uptime % 3600) / 60;
    int secs = uptime % 60;

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%dd %02d:%02d:%02d", days, hours, minutes, secs);
    return String(buffer);
}

void setState(String key, String value, bool publish) {
    stateMap[key] = value;
    if (publish && mqtt_client.connected()) {
        String fullTopic = mqttname + "/status/" + key;
        TickType_t timeout = pdMS_TO_TICKS(5000);  // 5s Timeout statt portMAX_DELAY
        if (xSemaphoreTake(mqttMutex, timeout) == pdTRUE) {
            bool success = mqtt_client.publish(fullTopic.c_str(), value.c_str());
            xSemaphoreGive(mqttMutex);
            if (!success) {
                String failMsg = "MQTT publish failed: " + fullTopic;
                DEBUG_PRINTLN(failMsg);
            }
        } else {
            String timeoutMsg = "MQTT mutex timeout: " + fullTopic;
            DEBUG_PRINTLN(timeoutMsg);
        }
    }
}

void publishStates() {
    if (!mqtt_client.connected()) {
        return;  // Early return wenn nicht verbunden
    }
    
    TickType_t timeout = pdMS_TO_TICKS(5000);  // 5s Timeout
    for (const auto &kv : stateMap) {
        String fullTopic = mqttname + "/status/" + kv.first;
        String payload = kv.second;
        if (xSemaphoreTake(mqttMutex, timeout) == pdTRUE) {
            bool success = mqtt_client.publish(fullTopic.c_str(), payload.c_str());
            xSemaphoreGive(mqttMutex);
            if (!success) {
                String failMsg = "MQTT publish failed: " + fullTopic;
                DEBUG_PRINTLN(failMsg);
            }
        } else {
            DEBUG_PRINTLN("MQTT mutex timeout in publishStates");
            break;  // Bei Timeout abbrechen
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void publishStatesTask(void *pvParameters) {
    while (true) {
        // Watchdog-Feed
        vTaskDelay(100 / portTICK_PERIOD_MS);

        time_t now = time(nullptr);
        time_t bootTime = now - esp_timer_get_time() / 1000000;
        setState("uptime", formatUptime(now - bootTime), false);

        if (mqtt_client.connected()) {
            publishStates();
        }
        
        // Warte min_publish_time Sekunden, aber in kleineren Schritten
        uint32_t total_delay = min_publish_time * 1000;
        uint32_t chunk_delay = 1000;  // 1 Sekunde Chunks
        for (uint32_t i = 0; i < total_delay; i += chunk_delay) {
            vTaskDelay(chunk_delay / portTICK_PERIOD_MS);
        }
    }
}

void handleMQTTSettingsMessage(const char *topic, const char *command, byte *payload, unsigned int length, bool &flag) {
    // Find the last occurrence of '/'
    const char *lastSlash = strrchr(topic, '/');
    if (lastSlash != nullptr) {
        // Move past the last '/'
        const char *lastPart = lastSlash + 1;
        // Compare the last part of the topic with the command
        if (strcmp(lastPart, command) == 0) {
            // Check if the payload is numeric
            bool isNumeric = true;
            for (unsigned int i = 0; i < length; i++) {
                if (!isdigit(payload[i])) {
                    isNumeric = false;
                    break;
                }
            }

            // If the payload is numeric, convert it to uint16_t and call write_setting
            if (isNumeric) {
                // Ensure the payload is null-terminated
                char payloadStr[length + 1];
                memcpy(payloadStr, payload, length);
                payloadStr[length] = '\0';

                uint16_t value = atoi(payloadStr);
                String convMsg = String("Converted value: ") + value;
                DEBUG_PRINTLN(convMsg);
                write_setting(command, value);
            } else {
                mqtt_client.publish(String(topic).c_str(), String(0).c_str());
            }
        }
    }
}

void handleMQTTMessage(const char *topic, const char *command, byte *payload, unsigned int length, bool &flag, bool setting = false) {

    if (setting) {
        return handleMQTTSettingsMessage(topic, command, payload, length, flag);
    }

    if (strcmp(topic, command) == 0) {
        String Command = String((char *)payload, length);
        flag = (Command == "true");
        write_setting("debug_flg", debug_flg);
        write_setting("debug_flg_full_log", debug_flg_full_log);
    }
}

// handle Subscriptions
void MQTTCallback(char *topic, byte *payload, unsigned int length) {
    handleMQTTMessage(topic, (mqttname + "/parameter/debugging_active").c_str(), payload, length, debug_flg);
    handleMQTTMessage(topic, (mqttname + "/parameter/debugging_active_full").c_str(), payload, length, debug_flg_full_log);

    handleMQTTMessage(topic, String("publish_delay").c_str(), payload, length, debug_flg_full_log, true);
    handleMQTTMessage(topic, String("min_publish_time").c_str(), payload, length, debug_flg_full_log, true);
}

#ifdef USE_TLS
WiFiClientSecure secure_wifi_client;
PubSubClient mqtt_client(mqtt_server, mqtt_tls_port, MQTTCallback, secure_wifi_client);

#else
WiFiClient wifi_client;
PubSubClient mqtt_client(mqtt_server, mqtt_port, MQTTCallback, wifi_client);
#endif

// Reconnect to MQTT broker
boolean mqtt_reconnect() {
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
    if (mqtt_client.connect(random_client_id.c_str(), mqtt_username, mqtt_password, willTopic.c_str(), willQoS, willRetain, willMessage.c_str(),true)) {

        int ErrorCnt = 0;
        String debug_flg_status = debug_flg ? "true" : "false";
        mqtt_client.publish((mqttname + "/parameter/debugging_active").c_str(), debug_flg_status.c_str()) || ErrorCnt++;
        mqtt_client.subscribe((mqttname + "/parameter/debugging_active").c_str()) || ErrorCnt++; // debug_flg

        String debug_flg_full_log_status = debug_flg_full_log ? "true" : "false";
        mqtt_client.publish((mqttname + "/parameter/debugging_active_full").c_str(), debug_flg_full_log_status.c_str()) || ErrorCnt++;
        mqtt_client.subscribe((mqttname + "/parameter/debugging_active_full").c_str()) || ErrorCnt++; // debug_flg_full_log

        mqtt_client.publish((mqttname + "/parameter/publish_delay").c_str(), String(publish_delay).c_str()) || ErrorCnt++;
        mqtt_client.subscribe((mqttname + "/parameter/publish_delay").c_str()) || ErrorCnt++; // debug_flg_full_log

        mqtt_client.publish((mqttname + "/parameter/min_publish_time").c_str(), String(min_publish_time).c_str()) || ErrorCnt++;
        mqtt_client.subscribe((mqttname + "/parameter/min_publish_time").c_str()) || ErrorCnt++; // min_publish_time

        if ( ErrorCnt > 0) {
            String errorMsg = "Connected to broker but initial publish or subscriptions failed, error count: " + String(ErrorCnt);
            DEBUG_PRINTLN(errorMsg);
            mqtt_client.disconnect();
            DEBUG_PRINTLN("Disconnected from broker.");
            return false;
        } else {
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
void mqtt_loop() {
    if (!mqtt_client.connected()) {
        DEBUG_PRINTLN("MQTT mqtt_client not connected, attempting to reconnect... (Attempt " + String(reconnect_attempts) + ")");
        unsigned long now = millis();
        if (now - lastReconnectAttempt > RECONNECT_DELAY) { // 5 seconds delay

            lastReconnectAttempt = now;
            if (mqtt_reconnect()) {
                lastReconnectAttempt = 0;
                DEBUG_PRINTLN("MQTT Reconnected.");
                mqtt_client.loop();
            } else {
                DEBUG_PRINTLN("MQTT Reconnect failed.");
                // lastReconnectAttempt = 0;
            }
        }
    } else {
        mqtt_client.loop();
    }
}

void mqtt_init() {
    DEBUG_PRINTLN("Connecting MQTT ...");

    // Initialize the mutexes
    mqttMutex = xSemaphoreCreateMutex();
    if (serialMutex == NULL) {
        serialMutex = xSemaphoreCreateMutex();  // Falls noch nicht in main.cpp erstellt
    }

    if (mqtt_reconnect()) {
        DEBUG_PRINTLN("MQTT Connected.");
        // Create the task to call publishStates() every min_publish_time seconds
        // Stack erhöht von 2048 auf 4096 für Stabilität
        xTaskCreate(publishStatesTask, "Publish States Task", 4096, NULL, 1, NULL);

        setState("version", VERSION, true);
        setState("ipaddress", WiFi.localIP().toString(), true);
        setState("ble_connection", "startup", true);
        setState("status", "online", true);

    } else
        DEBUG_PRINTLN("MQTT Connect failed.");
}