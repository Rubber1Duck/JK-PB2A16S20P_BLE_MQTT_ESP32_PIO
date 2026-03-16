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

static const char *get_reset_reason_class(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
    case ESP_RST_BROWNOUT:
        return "reason-critical";
    case ESP_RST_SW:
    case ESP_RST_EXT:
    case ESP_RST_SDIO:
        return "reason-warning";
    case ESP_RST_POWERON:
    case ESP_RST_DEEPSLEEP:
        return "reason-neutral";
    default:
        return "reason-unknown";
    }
}

const char *NVS_KEY = "reset_history";
ResetEntry history[MAX_RESET_REASONS]; // Array für MAX_RESET_REASONS Einträge

WebServer server(80);

uint32_t ota_progress_millis = 0;

static String fixedFieldToString(const char *field, size_t maxLen)
{
    char tmp[32];
    size_t copyLen = maxLen;
    if (copyLen >= sizeof(tmp))
    {
        copyLen = sizeof(tmp) - 1;
    }
    memcpy(tmp, field, copyLen);
    tmp[copyLen] = '\0';

    size_t actualLen = strnlen(tmp, copyLen);
    return String(tmp).substring(0, actualLen);
}

static String jsonEscape(const String &in)
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

// ---------------------------------------------------------------------------
// BMS Dashboard – JSON API  (/api/bms)
// ---------------------------------------------------------------------------
void handleBmsApi()
{
    String j = "{";
    j += "\"ts\":\"" + getLocalTimeString() + "\",";
    j += "\"device_ready\":" + String(has_device_info ? "true" : "false") + ",";
    j += "\"cells_ready\":" + String(has_cell_data ? "true" : "false") + ",";

    String deviceName = jsonEscape(fixedFieldToString(deviceinfo.DeviceName, sizeof(deviceinfo.DeviceName)));
    String vendorId = jsonEscape(fixedFieldToString(deviceinfo.ManufacturerDeviceID, sizeof(deviceinfo.ManufacturerDeviceID)));
    String hwVersion = jsonEscape(fixedFieldToString(deviceinfo.HardwareVersion, sizeof(deviceinfo.HardwareVersion)));
    String swVersion = jsonEscape(fixedFieldToString(deviceinfo.SoftwareVersion, sizeof(deviceinfo.SoftwareVersion)));
    String bmsUptime = jsonEscape(deviceinfo.getOddRunTimeStr());
    String espUptime = jsonEscape(formatUptime(esp_timer_get_time() / 1000000));
    esp_reset_reason_t lastResetReason = (esp_reset_reason_t)history[0].reason;
    String lastResetReasonText = jsonEscape(get_reset_reason_string(lastResetReason));
    String lastResetReasonClass = jsonEscape(String(get_reset_reason_class(lastResetReason)));

    // --- Device Info ---
    j += "\"device\":{";
    j += "\"name\":\"" + deviceName + "\",";
    j += "\"vendor_id\":\"" + vendorId + "\",";
    j += "\"hw_version\":\"" + hwVersion + "\",";
    j += "\"sw_version\":\"" + swVersion + "\",";
    j += "\"bms_uptime\":\"" + bmsUptime + "\",";
    j += "\"esp_uptime\":\"" + espUptime + "\",";
    j += "\"last_reset_reason\":\"" + lastResetReasonText + "\",";
    j += "\"last_reset_reason_class\":\"" + lastResetReasonClass + "\"";
    j += "},";

    // --- Cell Data ---
    j += "\"cells\":{";
    j += "\"vol\":[";
    for (int i = 0; i < 32; i++) {
        j += "\"" + String(celldata.CellVol_fmt[i]) + "\"";
        if (i < 31) j += ",";
    }
    j += "],";
    j += "\"sta\":" + String(celldata.CellSta) + ",";
    j += "\"vol_ave\":\"" + String(celldata.CellVolAve_fmt) + "\",";
    j += "\"vol_dif\":\"" + String(celldata.CellVdifMax_fmt) + "\",";
    j += "\"max_cell\":" + String(celldata.MaxVolCellNbr) + ",";
    j += "\"min_cell\":" + String(celldata.MinVolCellNbr) + ",";
    j += "\"bat_vol\":\"" + String(celldata.BatVol_fmt) + "\",";
    j += "\"bat_watt\":\"" + String(celldata.BatWatt_fmt) + "\",";
    j += "\"bat_cur\":\"" + String(celldata.BatCurrent_fmt) + "\",";
    j += "\"temp1\":\"" + String(celldata.TempBat1_fmt) + "\",";
    j += "\"temp2\":\"" + String(celldata.TempBat2_fmt) + "\",";
    j += "\"temp3\":\"" + String(celldata.TempBat3_fmt) + "\",";
    j += "\"temp4\":\"" + String(celldata.TempBat4_fmt) + "\",";
    j += "\"temp5\":\"" + String(celldata.TempBat5_fmt) + "\",";
    j += "\"soc\":" + String(celldata.SOCStateOfcharge) + ",";
    j += "\"cap_remain\":\"" + String(celldata.SOCCapRemain_fmt) + "\",";
    j += "\"cycles\":" + String(celldata.SOCCycleCount) + ",";
    j += "\"cycle_cap\":\"" + String(celldata.SOCCycleCap_fmt) + "\"";
    j += "}}";

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", j);
}

// ---------------------------------------------------------------------------
// BMS Dashboard – HTML Seite  (/bms)
// ---------------------------------------------------------------------------
void handleBmsPage()
{
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    server.sendContent("<!DOCTYPE html><html lang='de'><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>JK-BMS Dashboard</title>"
        "<style>"
        "*{box-sizing:border-box;margin:0;padding:0}"
        "body{font-family:sans-serif;background:#1a1a2e;color:#eee;padding:12px}"
        "h1{font-size:1.4rem;color:#00d4ff;margin-bottom:8px}"
        "h2{font-size:.95rem;color:#aaa;margin-bottom:8px;border-bottom:1px solid #333;padding-bottom:4px}"
        ".card{background:#16213e;border-radius:8px;padding:12px;margin-bottom:12px}"
        ".g2{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}"
        ".kv{display:flex;justify-content:space-between;padding:4px 0;border-bottom:1px solid #222}"
        ".kv:last-child{border-bottom:none}"
        ".lbl{color:#aaa;font-size:.82rem}"
        ".val{font-weight:bold;color:#fff;font-size:.88rem}"
        ".status{display:flex;gap:10px;margin-bottom:12px;flex-wrap:wrap}"
        ".sc{background:#16213e;border-radius:8px;padding:10px 14px;flex:1;min-width:90px;text-align:center}"
        ".sv{font-size:1.5rem;font-weight:bold}"
        ".sl{font-size:.72rem;color:#888;margin-top:2px}"
        ".soc-bar{background:#222;border-radius:4px;height:10px;margin:4px 0;overflow:hidden}"
        ".soc-fill{height:100%;border-radius:4px;transition:width .5s}"
        ".cg{display:grid;grid-template-columns:repeat(4,1fr);gap:5px}"
        ".cell{border-radius:6px;padding:6px 2px;text-align:center;font-size:.78rem}"
        ".cn{color:rgba(255,255,255,.5);font-size:.65rem}"
        ".cv{font-weight:bold;font-size:.9rem;margin-top:1px}"
        ".ok{background:#0d3318;border:1px solid #1a6b30}"
        ".hi{background:#2d2000;border:1px solid #e6a800}"
        ".lo{background:#2d0a0a;border:1px solid #cc2200}"
        ".mx{border:2px solid #00ff88!important}"
        ".mn{border:2px solid #ff4444!important}"
        ".off{background:#222;border:1px solid #444;opacity:.35}"
        "#dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#555;margin-right:6px;vertical-align:middle}"
        ".on{background:#00cc66!important}.oo{background:#cc3300!important}"
        "#ts{font-size:.72rem;color:#555;text-align:right;margin-bottom:8px}"
        ".cv-meta{display:flex;gap:14px;margin-bottom:8px;font-size:.8rem;flex-wrap:wrap}"
        ".footer-links{display:flex;gap:16px;justify-content:center;flex-wrap:wrap;margin:18px 0 8px}"
        ".footer-links a{color:#00d4ff;text-decoration:none;font-size:.9rem}"
        ".footer-links a:hover{text-decoration:underline}"
        ".reason-badge{display:inline-block;padding:4px 10px;border-radius:999px;font-size:.82rem;font-weight:600;border:1px solid transparent}"
        ".reason-critical{background:#3a1111;color:#ff8d8d;border-color:#8c2d2d}"
        ".reason-warning{background:#3b280d;color:#ffc266;border-color:#8e5b16}"
        ".reason-neutral{background:#1c2538;color:#b9d3ff;border-color:#30486e}"
        ".reason-unknown{background:#2b2b2b;color:#d5d5d5;border-color:#555}"
        "</style></head><body>");

    server.sendContent(
        "<h1><span id='dot'></span>JK-BMS Dashboard</h1>"
        "<p id='ts'>Warte auf Daten...</p>"
                "<div class='card'><h2>Ger&#228;teinformationen</h2><div class='g2'>"
                    "<div>"
                        "<div class='kv'><span class='lbl'>Ger&#228;tename</span><span class='val' id='di-name'>--</span></div>"
                        "<div class='kv'><span class='lbl'>Hersteller-ID</span><span class='val' id='di-vid'>--</span></div>"
                        "<div class='kv'><span class='lbl'>BMS - Laufzeit</span><span class='val' id='di-bms-up'>--</span></div>"
                    "</div><div>"
                        "<div class='kv'><span class='lbl'>HW-Version</span><span class='val' id='di-hw'>--</span></div>"
                        "<div class='kv'><span class='lbl'>SW-Version</span><span class='val' id='di-sw'>--</span></div>"
                        "<div class='kv'><span class='lbl'>ESP - Laufzeit</span><span class='val' id='di-esp-up'>--</span></div>"
                        "<div class='kv'><span class='lbl'>Letzter Reset Grund</span><span class='val'><span id='di-reset-reason' class='reason-badge reason-unknown'>--</span></span></div>"
                    "</div>"
                "</div></div>"
        "<div class='status'>"
          "<div class='sc'><div class='sv' id='sv-soc'>--</div><div class='sl'>SOC %</div>"
            "<div class='soc-bar'><div class='soc-fill' id='soc-bar' style='width:0%;background:#00cc66'></div></div></div>"
          "<div class='sc'><div class='sv' id='sv-vol'>--</div><div class='sl'>Spannung (V)</div></div>"
          "<div class='sc'><div class='sv' id='sv-cur'>--</div><div class='sl'>Strom (A)</div></div>"
          "<div class='sc'><div class='sv' id='sv-pwr'>--</div><div class='sl'>Leistung (W)</div></div>"
        "</div>");

    server.sendContent(
        "<div class='card'><h2>Zellspannungen</h2>"
        "<div class='cv-meta'>"
          "<span>&#216; <b id='cv-ave'>--</b> V</span>"
          "<span>&#916; <b id='cv-dif'>--</b></span>"
          "<span>Max: Zelle <b id='cv-max'>--</b></span>"
          "<span>Min: Zelle <b id='cv-min'>--</b></span>"
        "</div>"
        "<div class='cg' id='cg'></div></div>");

    server.sendContent(
        "<div class='card'><h2>Batterie &amp; Kapazit&#228;t</h2><div class='g2'><div>"
          "<div class='kv'><span class='lbl'>Kap. verf&#252;gbar</span><span class='val' id='cd-cap'>--</span></div>"
          "<div class='kv'><span class='lbl'>Zyklen</span><span class='val' id='cd-cyc'>--</span></div>"
          "<div class='kv'><span class='lbl'>Zykluskapazit&#228;t</span><span class='val' id='cd-ccap'>--</span></div>"
        "</div><div>"
                    "<div class='kv' id='row-t1'><span class='lbl'>Batterie Temperatur 1</span><span class='val' id='cd-t1'>--</span></div>"
                    "<div class='kv' id='row-t2'><span class='lbl'>Batterie Temperatur 2</span><span class='val' id='cd-t2'>--</span></div>"
                    "<div class='kv' id='row-t3'><span class='lbl'>MOSFET Temperatur</span><span class='val' id='cd-t3'>--</span></div>"
                    "<div class='kv' id='row-t4'><span class='lbl'>Batterie Temperatur 4</span><span class='val' id='cd-t4'>--</span></div>"
                    "<div class='kv' id='row-t5'><span class='lbl'>Batterie Temperatur 5</span><span class='val' id='cd-t5'>--</span></div>"
        "</div></div></div>");

        server.sendContent(
                "<div class='footer-links'>"
                    "<a href='/reset_history'>Reset-Historie</a>"
                    "<a href='/update'>Firmware-Update</a>"
                "</div>");

    server.sendContent("<script>"
        "function s(id,v){var e=document.getElementById(id);if(e)e.textContent=v;}"
                "function setReasonBadge(id,text,cls){"
                    "var e=document.getElementById(id);"
                    "if(!e)return;"
                    "e.textContent=text;"
                    "e.className='reason-badge '+(cls||'reason-unknown');"
                "}"
                "function setTemp(id,rowId,val){"
                    "var row=document.getElementById(rowId);"
                    "if(!row)return;"
                    "var n=parseFloat(val);"
                    "var hide=(n===-200);"
                    "row.style.display=hide?'none':'flex';"
                    "if(!hide)s(id,val+' C');"
                "}"
        "function cc(v,a){"
          "var d=Math.abs(v-a);"
          "if(d>0.050)return 'lo';"
          "if(d>0.020)return 'hi';"
          "return 'ok';"
        "}"
        "function buildCells(d){"
          "var g=document.getElementById('cg'),h='',a=parseFloat(d.cells.vol_ave);"
          "for(var i=0;i<32;i++){"
            "var b=(d.cells.sta>>i)&1,v=parseFloat(d.cells.vol[i]);"
                        "if(!b)continue;"
            "var c=b?cc(v,a):'off';"
                        "if((i+1)==d.cells.max_cell)c+=' mx';"
                        "if((i+1)==d.cells.min_cell)c+=' mn';"
            "h+='<div class=\"cell '+c+'\"><div class=\"cn\">Z'+(i+1)+'</div><div class=\"cv\">'+d.cells.vol[i]+'</div></div>';"
          "}"
          "g.innerHTML=h;"
        "}"
        "function upd(d){"
          "document.getElementById('dot').className=d.cells_ready?'on':'oo';"
          "s('ts','Zuletzt: '+d.ts);"
          "if(d.device_ready){"
            "s('di-name',d.device.name);s('di-vid',d.device.vendor_id);"
            "s('di-hw',d.device.hw_version);s('di-sw',d.device.sw_version);"
                        "s('di-bms-up',d.device.bms_uptime);s('di-esp-up',d.device.esp_uptime);"
          "}"
                    "setReasonBadge('di-reset-reason',d.device.last_reset_reason,d.device.last_reset_reason_class);"
          "if(d.cells_ready){"
            "s('sv-soc',d.cells.soc);s('sv-vol',d.cells.bat_vol);"
            "s('sv-cur',d.cells.bat_cur);s('sv-pwr',d.cells.bat_watt);"
            "var f=document.getElementById('soc-bar'),soc=parseInt(d.cells.soc);"
            "f.style.width=soc+'%';"
            "f.style.background=soc>50?'#00cc66':soc>20?'#ffaa00':'#cc3300';"
            "s('cv-ave',d.cells.vol_ave);s('cv-dif',d.cells.vol_dif);"
            "s('cv-max',d.cells.max_cell);s('cv-min',d.cells.min_cell);"
            "s('cd-cap',d.cells.cap_remain+' Ah');s('cd-cyc',d.cells.cycles);"
            "s('cd-ccap',d.cells.cycle_cap+' Ah');"
                        "setTemp('cd-t1','row-t1',d.cells.temp1);setTemp('cd-t2','row-t2',d.cells.temp2);"
                        "setTemp('cd-t3','row-t3',d.cells.temp3);setTemp('cd-t4','row-t4',d.cells.temp4);"
                        "setTemp('cd-t5','row-t5',d.cells.temp5);"
            "buildCells(d);"
          "}"
        "}"
        "function poll(){"
          "fetch('/api/bms')"
            ".then(function(r){return r.json();})"
            ".then(function(d){upd(d);})"
            ".catch(function(){document.getElementById('dot').className='oo';});"
        "}"
        "poll();setInterval(poll,2000);"
        "</script></body></html>");

    server.sendContent(""); // flush
}

// Hauptseite mit Link zur Reset-Historie
void handleRoot()
{
    handleBmsPage();
}

// Reset-Historie mit Zeitstempel anzeigen
void handleResetHistory()
{
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    
    server.sendContent("<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>ESP32 Time Log</title>");
    server.sendContent("<style>body{font-family:sans-serif;background:#0b0b12;color:#eee;padding:20px;}h1{color:#00d4ff;margin-bottom:16px;}table{width:100%;border-collapse:collapse;background:#16213e;border-radius:10px;overflow:hidden;}td,th{border-bottom:1px solid #24314f;padding:12px 10px;text-align:left;}th{background:#101a30;color:#9cb3d9;font-size:.85rem;text-transform:uppercase;letter-spacing:.04em;}tr:hover td{background:#1b2a49;}a{color:#00d4ff;text-decoration:none;}a:hover{text-decoration:underline;}.actions{display:flex;gap:18px;justify-content:center;flex-wrap:wrap;margin-top:18px;}.wrap{max-width:1100px;margin:0 auto;}.reason-badge{display:inline-block;padding:4px 10px;border-radius:999px;font-size:.82rem;font-weight:600;border:1px solid transparent;}.reason-critical{background:#3a1111;color:#ff8d8d;border-color:#8c2d2d;}.reason-warning{background:#3b280d;color:#ffc266;border-color:#8e5b16;}.reason-neutral{background:#1c2538;color:#b9d3ff;border-color:#30486e;}.reason-unknown{background:#2b2b2b;color:#d5d5d5;border-color:#555;}</style></head><body><div class='wrap'>");
    server.sendContent("<h1>Reset-Historie mit Zeitstempel</h1><table><tr><th>Nr.</th><th>Zeitpunkt</th><th>Grund</th></tr>");

    char row[320];
    for (int i = 0; i < MAX_RESET_REASONS; i++)
    {
        esp_reset_reason_t reason = (esp_reset_reason_t)history[i].reason;
        snprintf(row, sizeof(row), "<tr><td>%d</td><td>%s</td><td><span class='reason-badge %s'>%s</span></td></tr>",
            i + 1,
            formatTime(history[i].timestamp).c_str(),
            get_reset_reason_class(reason),
            get_reset_reason_string(reason).c_str()
        );
        server.sendContent(row);
    }

    server.sendContent("</table><div class='actions'><a href='/clear'>Log l&#246;schen</a><a href='/'>Zur&#252;ck zur Startseite</a></div></div></body></html>");
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
    memmove(&history[1], &history[0], sizeof(ResetEntry) * (MAX_RESET_REASONS - 1)); // Schneller als Schleife
    history[0].reason = currentReason;
    time(&history[0].timestamp); // Aktuelle Zeit speichern
    // 4. Zurück in den NVS schreiben
    prefs.putBytes(NVS_KEY, history, sizeof(history));
    prefs.end();
    // 5. Formatierte Ausgabe der Historie
    for (int i = 0; i < MAX_RESET_REASONS; i++)
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
    server.on("/reset_history", handleResetHistory);
    server.on("/clear", handleClear);
    server.on("/bms", handleBmsPage);
    server.on("/api/bms", handleBmsApi);
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
