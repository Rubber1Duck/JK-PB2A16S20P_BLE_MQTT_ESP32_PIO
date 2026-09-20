#include "main.h"

void setup()
{
#ifdef SERIAL_OUT
    Serial.begin(115200);
    delay(1000);
#endif // SERIAL_OUT
    init_settings();

    Serial.println("");
    Serial.println(String("JK-BMS Listener V ") + VERSION);
    Serial.println("");
    Serial.println("Starting ...");
    Serial.println("");

#ifdef USELED
    init_led();
    set_led(LedState::LED_DOUBLE_FLASH);
#endif // USELED

    init_wifi();

    init_mdns_handler();

#ifdef USE_SYSLOG
    init_syslog_handler();
#endif // USE_SYSLOG

#ifdef USE_TLS
    const char *root_ca_cert = MQTT_ROOT_CA_CERT;
    secure_wifi_client.setTimeout(15000);
#ifdef MQTT_SKIP_CERT_VERIFY
    // Debug mode: disable certificate validation completely.
    secure_wifi_client.setInsecure();
    DEBUG_PRINTLN("WARNING: SSL/TLS certificate verification disabled!");
#else
    secure_wifi_client.setCACert(root_ca_cert);
#endif // MQTT_SKIP_CERT_VERIFY
#endif // USE_TLS

#ifdef NTPSERVER
#ifdef TIMEZONE
    configTzTime(time_zone, ntpServer);
#else
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#endif // TIMEZONE
#endif // NTPSERVER

    // Wait for NTP time synchronization before attempting MQTT connection
    // This prevents SSL certificate verification errors
    if (!waitForTimeSync())
    {
        DEBUG_PRINTLN("WARNING: Proceeding without confirmed NTP sync - SSL/TLS may fail");
    }

    debug_print_reset_history();

    setupWebserver(history, MAX_RESET_REASONS, RESET_HISTORY_KEY);

    publish_init();
    
    mqtt_init();
    
    ble_setup();
}

void loop()
{
    webserverLoop();
    
    wifi_loop();
    
    mqtt_loop();
    
    ble_loop();
}
