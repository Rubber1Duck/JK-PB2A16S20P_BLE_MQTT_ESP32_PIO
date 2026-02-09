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

void init_wifi() {
        
#ifdef USE_WIFI_STATIC_IP
    IPAddress local_IP;
    local_IP.fromString(ip_address);
    IPAddress local_GW;
    local_GW.fromString(gateway);
    IPAddress local_SubN;
    local_SubN.fromString(subnet);
    IPAddress local1DNS;   //optional
    local1DNS.fromString(dns);
    IPAddress local2DNS; //optional
    local2DNS.fromString(dns2);
#endif
    
    byte wifi_retry = 0;
    WiFi.disconnect();      // ensure WiFi is disconnected
    WiFi.persistent(false); // do not persist as WiFi.begin is not helpful then. Decreases connection speed but helps in other ways
    WiFi.setHostname(WIFI_DHCPNAME); // Set WiFi Hostname
    WiFi.mode(WIFI_STA);
    // esp_wifi_set_ps( WIFI_PS_NONE ); // do not set WiFi to sleep, increases stability
#ifdef USE_WIFI_STATIC_IP
    WiFi.config(local_IP, local_GW, local_SubN, local1DNS, local2DNS); // set static IP configuration
#else
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // use DHCP
#endif
    WiFi.begin(ssid, password);
    DEBUG_PRINTLN("Connecting to WiFi ..");
    
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && wifi_retry < 10) {
        if (millis() - start >= 10000) {  // Non-blocking 10s wait
            WiFi.begin(ssid, password);
            wifi_retry++;
            DEBUG_PRINTLN(".");  // Atomare Ausgabe
            start = millis();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Yield to other tasks
    }
    
    if (wifi_retry >= 10) {
        DEBUG_PRINTLN("\nReboot...");
        ESP.restart();
    }
    DEBUG_PRINTLN("Ready");
    String ipMsg = "IP address: " + WiFi.localIP().toString();
    DEBUG_PRINTLN(ipMsg);
}

void wifi_loop() {
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("WiFi not connected, attempting to reconnect...");
        init_wifi();
        setState("ipaddress", WiFi.localIP().toString(), false);
    } else {
        setState("wifi_rssi", String(WiFi.RSSI()), false);
    }
}