#include "wifi_handler.h"

// wifi Setting
const char *ssid = SSID_NAME;
const char *password = SSID_PASSWORD;

#ifdef USE_WIFI_STATIC_IP
// Set your Static IP address
const char *ip_address = IP_ADDRESS;
const char *gateway = GATEWAY;
const char *subnet = SUBNET;
const char *dns = DNS;
const char *dns2 = DNS2;
#endif

// Flag, um zu wissen, ob das WLAN bereit ist
bool isWifiConnected = false;
uint32_t WIFI_RSSI_Update_Time = 0;
const uint32_t WIFI_RSSI_UPDATE_INTERVAL = 15000; // 15 Sekunden

static const char *wifi_event_to_text(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            return "WIFI_READY";
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            return "WIFI_SCAN_DONE";
        case ARDUINO_EVENT_WIFI_STA_START:
            return "WIFI_STA_START";
        case ARDUINO_EVENT_WIFI_STA_STOP:
            return "WIFI_STA_STOP";
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            return "WIFI_STA_CONNECTED";
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            return "WIFI_STA_DISCONNECTED";
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            return "WIFI_STA_AUTHMODE_CHANGE";
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            return "WIFI_STA_GOT_IP";
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            return "WIFI_STA_GOT_IP6";
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            return "WIFI_STA_LOST_IP";
        case ARDUINO_EVENT_WIFI_AP_START:
            return "WIFI_AP_START";
        case ARDUINO_EVENT_WIFI_AP_STOP:
            return "WIFI_AP_STOP";
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            return "WIFI_AP_STACONNECTED";
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            return "WIFI_AP_STADISCONNECTED";
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            return "WIFI_AP_STAIPASSIGNED";
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            return "WIFI_AP_PROBEREQRECVED";
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            return "WIFI_AP_GOT_IP6";
        case ARDUINO_EVENT_WIFI_FTM_REPORT:
            return "WIFI_FTM_REPORT";
        case ARDUINO_EVENT_ETH_START:
            return "ETH_START";
        case ARDUINO_EVENT_ETH_STOP:
            return "ETH_STOP";
        case ARDUINO_EVENT_ETH_CONNECTED:
            return "ETH_CONNECTED";
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            return "ETH_DISCONNECTED";
        case ARDUINO_EVENT_ETH_GOT_IP:
            return "ETH_GOT_IP";
        case ARDUINO_EVENT_ETH_GOT_IP6:
            return "ETH_GOT_IP6";
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            return "WPS_ER_SUCCESS";
        case ARDUINO_EVENT_WPS_ER_FAILED:
            return "WPS_ER_FAILED";
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            return "WPS_ER_TIMEOUT";
        case ARDUINO_EVENT_WPS_ER_PIN:
            return "WPS_ER_PIN";
        case ARDUINO_EVENT_WPS_ER_PBC_OVERLAP:
            return "WPS_ER_PBC_OVERLAP";
        case ARDUINO_EVENT_SC_SCAN_DONE:
            return "SC_SCAN_DONE";
        case ARDUINO_EVENT_SC_FOUND_CHANNEL:
            return "SC_FOUND_CHANNEL";
        case ARDUINO_EVENT_SC_GOT_SSID_PSWD:
            return "SC_GOT_SSID_PSWD";
        case ARDUINO_EVENT_SC_SEND_ACK_DONE:
            return "SC_SEND_ACK_DONE";
        case ARDUINO_EVENT_PROV_INIT:
            return "PROV_INIT";
        case ARDUINO_EVENT_PROV_DEINIT:
            return "PROV_DEINIT";
        case ARDUINO_EVENT_PROV_START:
            return "PROV_START";
        case ARDUINO_EVENT_PROV_END:
            return "PROV_END";
        case ARDUINO_EVENT_PROV_CRED_RECV:
            return "PROV_CRED_RECV";
        case ARDUINO_EVENT_PROV_CRED_FAIL:
            return "PROV_CRED_FAIL";
        case ARDUINO_EVENT_PROV_CRED_SUCCESS:
            return "PROV_CRED_SUCCESS";
        default:
            return "WIFI_EVENT_UNKNOWN";
    }
}

// Event-Handler Funktion
void WiFiEvent(WiFiEvent_t event) {
    DEBUG_PRINTF("[WiFi-event] Event: %s (%d)\n", wifi_event_to_text(event), event);
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            DEBUG_PRINT("IP-Adresse: ");
            DEBUG_PRINTLN(WiFi.localIP());
            setState("wifi_rssi", String(WiFi.RSSI()), false);
            isWifiConnected = true;
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            isWifiConnected = false;
            DEBUG_PRINTLN("WLAN Verbindung verloren! Reconnect...");
            WiFi.reconnect(); // Versucht automatisch neu zu verbinden
            break;
        default:
            break;
    }
}

void init_wifi() {

#ifdef USE_WIFI_STATIC_IP
    IPAddress local_IP;
    local_IP.fromString(ip_address);
    IPAddress local_GW;
    local_GW.fromString(gateway);
    IPAddress local_SubN;
    local_SubN.fromString(subnet);
    IPAddress local1DNS; // optional
    local1DNS.fromString(dns);
    IPAddress local2DNS; // optional
    local2DNS.fromString(dns2);
#endif

    byte wifi_retry = 0;
    WiFi.disconnect();               // ensure WiFi is disconnected
    WiFi.onEvent(WiFiEvent);         // Event Handler registrieren
    WiFi.setHostname(WIFI_DHCPNAME); // Hostname setzen (optional, aber hilfreich für die Identifikation im Netzwerk)
    //WiFi.setSleep(false);          // WLAN Sleep-Modus deaktivieren (für stabilere Verbindungen) nicht möglich bei gleichzeitiger Nutzung von Bluetooth, siehe
                                     // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#power-save
    WiFi.setAutoReconnect(true);     // Auto-Reconnect aktivieren (wichtig für Stabilität)
#ifdef USE_WIFI_STATIC_IP
    WiFi.config(local_IP, local_GW, local_SubN, local1DNS, local2DNS); // set static IP configuration
#else
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // use DHCP
#endif
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (++wifi_retry >= 40) { // Nach 40 Versuchen abbrechen
            DEBUG_PRINTLN("\nFailed to connect to WiFi, check your SSID and/or password. Reboot ESP...");
            ESP.restart(); // Neustart des ESP, um es erneut zu versuchen
        }
    }
}

void wifi_loop() {
    uint32_t currentTime = millis();
    if (currentTime - WIFI_RSSI_Update_Time >=WIFI_RSSI_UPDATE_INTERVAL) {
        setState("wifi_rssi", String(WiFi.RSSI()), false);
        setState("MinFreeHeap", String(ESP.getMinFreeHeap()), false);
        WIFI_RSSI_Update_Time = currentTime;
    }
}