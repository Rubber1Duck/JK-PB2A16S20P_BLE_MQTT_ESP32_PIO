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

// Struktur für einen Log-Eintrag (Größe: 1 Byte Grund + 8 Byte Zeit = 9 Bytes)
struct ResetEntry
{
    uint8_t reason;
    time_t timestamp;
};

// Wandelt Zeitstempel in lesbaren Text um
String formatTime(time_t t)
{
    if (t == 0)
        return "Keine Daten";
    struct tm *timeinfo = localtime(&t);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timeinfo);
    return String(buffer);
}

String get_reset_reason_string(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_UNKNOWN:
        return "Unknown";
    case ESP_RST_POWERON:
        return "Power-on";
    case ESP_RST_EXT:
        return "External pin";
    case ESP_RST_SW:
        return "Software reset";
    case ESP_RST_PANIC:
        return "Panic/Exception";
    case ESP_RST_INT_WDT:
        return "Interrupt Watchdog";
    case ESP_RST_TASK_WDT:
        return "Task Watchdog";
    case ESP_RST_WDT:
        return "Other Watchdog";
    case ESP_RST_DEEPSLEEP:
        return "Deep Sleep Wakeup";
    case ESP_RST_BROWNOUT:
        return "Brownout";
    case ESP_RST_SDIO:
        return "SDIO Reset";
    default:
        return "Invalid reason code";
    }
}

const char *NVS_KEY = "reset_history";
ResetEntry history[25]; // Array für 25 Einträge

WebServer server(80);

uint32_t ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  DEBUG_PRINTLN("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    DEBUG_PRINTF("OTA Progress Current: %zu bytes, Final: %zu bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    DEBUG_PRINTLN("OTA update finished successfully!");
  } else {
    DEBUG_PRINTLN("There was an error during OTA update!");
  }
  // <Add your own code here>
}


// Hauptseite
void handleRoot()
{
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    
    server.sendContent("<html><head><meta charset='UTF-8'><title>ESP32 Time Log</title>");
    server.sendContent("<style>body{font-family:sans-serif; padding:20px;} table{width:100%; border-collapse:collapse;} td,th{border:1px solid #ddd; padding:10px;}</style></head><body>");
    server.sendContent("<h1>Reset-Historie mit Zeitstempel</h1><table><tr><th>Nr.</th><th>Zeitpunkt</th><th>Grund</th></tr>");

    char row[320];
    for (int i = 0; i < 25; i++)
    {
        snprintf(row, sizeof(row), "<tr><td>%d</td><td>%s</td><td>%s</td></tr>",
            i + 1,
            formatTime(history[i].timestamp).c_str(),
            get_reset_reason_string((esp_reset_reason_t)history[i].reason).c_str()
        );
        server.sendContent(row);
    }

    server.sendContent("</table><br><a href='/clear'>Log löschen</a></body></html>");
    server.sendContent(""); // Antwort sauber abschließen
}

// Historie im NVS nullen
void handleClear()
{
    memset(history, 0, sizeof(history));
    prefs.begin("system", false);
    prefs.putBytes(NVS_KEY, history, sizeof(history));
    prefs.end();

    // Umleitung zurück zur Hauptseite
    server.sendHeader("Location", "/");
    server.send(303);
}

void setup()
{
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
    if(psramFound()){
        size_t cert_len = strlen(cert_flash) + 1;
        // Speicher explizit im PSRAM anfordern
        root_ca_cert_psram = (char*) ps_malloc(cert_len);
        if (root_ca_cert_psram != nullptr)
        {
            memcpy(root_ca_cert_psram, cert_flash, cert_len);
            Serial.println("Zertifikat erfolgreich in PSRAM kopiert.");
        } else {
            Serial.println("Fehler: Kein Speicher im PSRAM verfügbar.");
        }
    } else {
        Serial.println("PSRAM nicht gefunden, Zertifikat bleibt im Flash.");
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

    DEBUG_PRINTLN("\n--- ESP32 Reset History ---");
    // 1. Aktuellen Grund ermitteln
    uint8_t currentReason = (uint8_t)esp_reset_reason();
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        Serial.println("Zeit-Sync fehlgeschlagen");
    // 2. NVS öffnen und Daten laden
    prefs.begin("system", false);
    prefs.getBytes(NVS_KEY, history, sizeof(history));
    // 3. Historie rotieren (Index 0 ist immer der neuste)
    memmove(&history[1], &history[0], sizeof(ResetEntry) * 24); // Schneller als Schleife
    history[0].reason = currentReason;
    time(&history[0].timestamp); // Aktuelle Zeit speichern
    // 4. Zurück in den NVS schreiben
    prefs.putBytes(NVS_KEY, history, sizeof(history));
    prefs.end();
    // 5. Formatierte Ausgabe der Historie
    for (int i = 0; i < 25; i++)
    {
        if (history[i].reason == 0 && i > 0)
            continue; // Leere Einträge überspringen
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

    server.on("/", handleRoot);
    server.on("/clear", handleClear);
    server.on("/ota", []() {
        server.send(200, "text/plain", "Hi! This is ElegantOTA Demo 2.");
    });

    ElegantOTA.begin(&server);    // Start ElegantOTA
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    
    server.begin();

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

void loop()
{
    server.handleClient();
    ElegantOTA.loop();
    wifi_loop();
    mqtt_loop();
    ble_loop();

#ifdef USE_RS485
    rs485_loop();
#endif
}
