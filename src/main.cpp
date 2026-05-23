#include "main.h"
#ifdef USE_WEBSERVER
#include "app_webserver.h"
#endif

#ifdef NTPSERVER
const char *ntpServer = NTPSERVER;
#ifdef TIMEZONE
const char *time_zone = TIMEZONE;
#else
const long gmtOffset_sec = GMTOFFSET;
const int daylightOffset_sec = DLOFFSET;
#endif
#endif // NTPSERVER

const char *NVS_KEY = "reset_history";
ResetEntry history[MAX_RESET_REASONS];

void setup()
{
#ifdef SERIAL_OUT
    Serial.begin(115200);
    delay(1000);
#endif
    init_settings();

    DEBUG_PRINTLN("");
    DEBUG_PRINTLN(String("JK-BMS Listener V ") + VERSION);
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("Starting ...");
    DEBUG_PRINTLN("");

#ifdef USE_TLS
    const char *cert_flash = MQTT_ROOT_CA_CERT;
    char *root_ca_cert_psram = nullptr;
    if (psramFound())
    {
        size_t cert_len = strlen(cert_flash) + 1;
        root_ca_cert_psram = (char *)ps_malloc(cert_len);
        if (root_ca_cert_psram != nullptr)
        {
            memcpy(root_ca_cert_psram, cert_flash, cert_len);
            Serial.println("Zertifikat erfolgreich in PSRAM kopiert.");
        }
        else
        {
            Serial.println("Fehler: Kein Speicher im PSRAM verfügbar.");
        }
    }
    else
    {
        Serial.println("PSRAM nicht gefunden, Zertifikat bleibt im Flash.");
    }
    const char *root_ca_cert = root_ca_cert_psram ? root_ca_cert_psram : cert_flash;
#endif

#ifdef USELED
    init_led();
    set_led(LedState::LED_DOUBLE_FLASH);
#endif

    init_wifi();
#ifdef USE_TLS
    secure_wifi_client.setTimeout(15000);
#ifdef MQTT_SKIP_CERT_VERIFY
    // Debug mode: disable certificate validation completely.
    secure_wifi_client.setInsecure();
    Serial.println("WARNING: SSL/TLS certificate verification disabled!");
#else
    secure_wifi_client.setCACert(root_ca_cert);
#endif
#endif

#ifdef NTPSERVER
#ifdef TIMEZONE
    configTzTime(time_zone, ntpServer);
#else
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#endif
    DEBUG_PRINTLN("NTP-Time synced");
    DEBUG_PRINTLN("Current time: " + getLocalTimeString());
#endif

    // Wait for NTP time synchronization before attempting MQTT connection
    // This prevents SSL certificate verification errors
    if (!waitForTimeSync())
    {
        DEBUG_PRINTLN("WARNING: Proceeding without confirmed NTP sync - SSL/TLS may fail");
    }

    DEBUG_PRINTLN("\n--- ESP32 Reset History ---");
    uint8_t currentReason = (uint8_t)esp_reset_reason();
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        Serial.println("Zeit-Sync fehlgeschlagen");

    prefs.begin("system", false);
    prefs.getBytes(NVS_KEY, history, sizeof(history));
    memmove(&history[1], &history[0], sizeof(ResetEntry) * (MAX_RESET_REASONS - 1));
    history[0].reason = currentReason;
    time(&history[0].timestamp);
    prefs.putBytes(NVS_KEY, history, sizeof(history));
    prefs.end();

    for (int i = 0; i < MAX_RESET_REASONS; i++)
    {
        if (history[i].reason == 0 && i > 0)
            continue;
        DEBUG_PRINT("Eintrag [");
        DEBUG_PRINT(i);
        DEBUG_PRINT("]: ");
        DEBUG_PRINT(formatTime(history[i].timestamp));
        DEBUG_PRINT(" - ");
        DEBUG_PRINT(get_reset_reason_string((esp_reset_reason_t)history[i].reason));
        DEBUG_PRINT(" (Code: ");
        DEBUG_PRINT((int)history[i].reason);
        DEBUG_PRINTLN(")");
    }
    DEBUG_PRINTLN("---------------------------\n");

#ifdef USE_WEBSERVER
    setupWebserver(history, MAX_RESET_REASONS, NVS_KEY);
#endif

    publish_init();
    mqtt_init();
    ble_setup();
}

void loop()
{
#ifdef USE_WEBSERVER
    webserverLoop();
#endif
    wifi_loop();
    mqtt_loop();
    ble_loop();
}
