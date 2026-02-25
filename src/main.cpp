#include "main.h"

#ifdef NTPSERVER
const char *ntpServer = NTPSERVER;
#ifdef TIMEZONE
const char *time_zone = TIMEZONE;
#else
const long gmtOffset_sec = GMTOFFSET;
const int daylightOffset_sec = DLOFFSET;
#endif
#endif // NTPSERVER

void setup() {
#ifdef SERIAL_OUT
    Serial.begin(115200);
    delay(1000); // Warte kurz, damit die serielle Verbindung stabil ist
#endif
    init_settings();

    DEBUG_PRINTLN("");
    DEBUG_PRINTLN(String("JK-BMS Listener V ") + VERSION);
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("Starting ...");
    DEBUG_PRINTLN("");

#ifdef USE_TLS
    // 1. Zertifikat im Flash belassen (Standard auf ESP32)
    const char* cert_flash = MQTT_ROOT_CA_CERT;
    // 2. Pointer für PSRAM vorbereiten
    char* root_ca_cert_psram = nullptr;
    // Prüfen, ob PSRAM verfügbar ist
    if(psramFound()) {
        size_t cert_len = strlen(cert_flash) + 1;
        // Speicher explizit im PSRAM anfordern
        root_ca_cert_psram = (char*) ps_malloc(cert_len);
        if (root_ca_cert_psram != nullptr) {
            memcpy(root_ca_cert_psram, cert_flash, cert_len);
            DEBUG_PRINTLN("Zertifikat erfolgreich in PSRAM kopiert.");
        }
        else {
            DEBUG_PRINTLN("Fehler: Kein Speicher im PSRAM verfügbar.");
        }
    }
    else {
        DEBUG_PRINTLN("PSRAM nicht gefunden, Zertifikat bleibt im Flash.");
    }
    const char* root_ca_cert = root_ca_cert_psram ? root_ca_cert_psram : cert_flash;
#endif

#ifdef USELED
    init_led();
    // Send LED_FLASH state to the LED task
    set_led(LedState::LED_DOUBLE_FLASH);
#endif

    // WIFI Setup
    init_wifi();
#ifdef USE_TLS
    secure_wifi_client.setCACert(root_ca_cert);
#endif

#ifdef NTPSERVER
    // NTP Setup
#ifdef TIMEZONE
    configTzTime(time_zone, ntpServer);
#else
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#endif
    DEBUG_PRINTLN("NTP-Time synced");
    DEBUG_PRINTLN("Current time: " + getLocalTimeString());
#endif // NTPSERVER

    publish_init();

    mqtt_init();

#ifdef USE_RS485
    init_rs485();
#endif

#ifdef USE_INFLUXDB
    // InfluxDB Setup
    init_influxdb();
#endif

    ble_setup();
}

void loop() {
    wifi_loop();
    mqtt_loop();
    ble_loop();

#ifdef USE_RS485
    rs485_loop();
#endif
}
