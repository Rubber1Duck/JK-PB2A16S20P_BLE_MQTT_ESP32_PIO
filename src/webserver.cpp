#include "app_webserver.h"

#ifdef USE_WEBSERVER

#include <WebServer.h>
#include <ElegantOTA.h>
#include <Preferences.h>
#include <cstring>

#include "macros.h"
#include "mqtt_handler.h"
#include "mqtt_publish_config.h"
#include "parser.h"

extern Preferences prefs;

namespace
{
WebServer server(80);
uint32_t ota_progress_millis = 0;

ResetEntry *g_history = nullptr;
size_t g_historyCount = 0;
const char *g_nvsKey = nullptr;

const char *BMS_ALARM_NAMES[24] = {
    "AlarmWireRes", "AlarmMosOTP", "AlarmCellQuantity", "AlarmCurSensorErr", "AlarmCellOVP", "AlarmBatOVP",
    "AlarmChOCP", "AlarmChSCP", "AlarmChOTP", "AlarmChUTP", "AlarmCPUAuxCommuErr", "AlarmCellUVP",
    "AlarmBatUVP", "AlarmDchOCP", "AlarmDchSCP", "AlarmDchOTP", "AlarmChargeMOS", "AlarmDischargeMOS",
    "AlarmGPSDisconneted", "AlarmModifyPWD_in_time", "AlarmDischargeOnFailed", "BatteryOverTempAlarm",
    "TemperatureSensorAnomaly", "AlarmPLCModuleAnomaly"};

String fixedFieldToString(const char *field, size_t maxLen)
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

void onOTAStart()
{
    DEBUG_PRINTLN("OTA update started!");
}

void onOTAProgress(size_t current, size_t final)
{
    if (millis() - ota_progress_millis > 1000)
    {
        ota_progress_millis = millis();
        DEBUG_PRINTF("OTA Progress Current: %zu bytes, Final: %zu bytes\n", current, final);
    }
}

void onOTAEnd(bool success)
{
    if (success)
    {
        DEBUG_PRINTLN("OTA update finished successfully!");
    }
    else
    {
        DEBUG_PRINTLN("There was an error during OTA update!");
    }
}

void handleBmsApi()
{
    String j = "{";
    j += "\"ts\":\"" + getLocalTimeString() + "\",";
    j += "\"device_ready\":" + String(has_device_info ? "true" : "false") + ",";
    j += "\"cells_ready\":" + String(has_cell_data ? "true" : "false") + ",";
    j += "\"config_ready\":" + String(has_config_info ? "true" : "false") + ",";

    String deviceName = jsonEscape(fixedFieldToString(deviceinfo.DeviceName, sizeof(deviceinfo.DeviceName)));
    String vendorId = jsonEscape(fixedFieldToString(deviceinfo.ManufacturerDeviceID, sizeof(deviceinfo.ManufacturerDeviceID)));
    String hwVersion = jsonEscape(fixedFieldToString(deviceinfo.HardwareVersion, sizeof(deviceinfo.HardwareVersion)));
    String swVersion = jsonEscape(fixedFieldToString(deviceinfo.SoftwareVersion, sizeof(deviceinfo.SoftwareVersion)));
    String bmsUptime = jsonEscape(deviceinfo.getOddRunTimeStr());
    String espUptime = jsonEscape(formatUptime(esp_timer_get_time() / 1000000));

    esp_reset_reason_t lastResetReason = ESP_RST_UNKNOWN;
    if (g_history != nullptr && g_historyCount > 0)
    {
        lastResetReason = (esp_reset_reason_t)g_history[0].reason;
    }
    String lastResetReasonText = jsonEscape(get_reset_reason_string(lastResetReason));
    String lastResetReasonClass = jsonEscape(String(get_reset_reason_class(lastResetReason)));

    j += "\"device\":{";
    j += "\"name\":\"" + deviceName + "\",";
    j += "\"vendor_id\":\"" + vendorId + "\",";
    j += "\"hw_version\":\"" + hwVersion + "\",";
    j += "\"sw_version\":\"" + swVersion + "\",";
    j += "\"fw_version\":\"" + String(VERSION) + "\",";
    j += "\"bms_uptime\":\"" + bmsUptime + "\",";
    j += "\"esp_uptime\":\"" + espUptime + "\",";
    j += "\"last_reset_reason\":\"" + lastResetReasonText + "\",";
    j += "\"last_reset_reason_class\":\"" + lastResetReasonClass + "\"";
    j += "},";

    j += "\"cells\":{";
    j += "\"vol\":[";
    for (int i = 0; i < 32; i++)
    {
        j += "\"" + String(celldata.CellVol_fmt[i]) + "\"";
        if (i < 31)
            j += ",";
    }
    j += "],";
    j += "\"sta\":" + String(celldata.CellSta) + ",";
    j += "\"vol_ave\":\"" + String(celldata.CellVolAve_fmt) + "\",";
    j += "\"vol_dif\":\"" + String(celldata.CellVdifMax_fmt) + "\",";
    j += "\"max_cell\":" + String(celldata.MaxVolCellNbr) + ",";
    j += "\"min_cell\":" + String(celldata.MinVolCellNbr) + ",";
    j += "\"bat_vol\":\"" + String(celldata.BatVol_fmt) + "\",";
    float calculatedBatteryPower = celldata.BatCurrent < 0
                                       ? -static_cast<float>(celldata.BatWatt) * 0.001f
                                       : static_cast<float>(celldata.BatWatt) * 0.001f;
    j += "\"bat_watt\":\"" + String(calculatedBatteryPower, 3) + "\",";
    j += "\"bat_cur\":\"" + String(celldata.BatCurrent_fmt) + "\",";
    j += "\"temp1\":\"" + String(celldata.TempBat1_fmt) + "\",";
    j += "\"temp2\":\"" + String(celldata.TempBat2_fmt) + "\",";
    j += "\"temp3\":\"" + String(celldata.TempBat3_fmt) + "\",";
    j += "\"temp4\":\"" + String(celldata.TempBat4_fmt) + "\",";
    j += "\"temp5\":\"" + String(celldata.TempBat5_fmt) + "\",";
    j += "\"soc\":" + String(celldata.SOCStateOfcharge) + ",";
    j += "\"soh\":" + String(celldata.SOCSOH) + ",";
    j += "\"cap_remain\":\"" + String(celldata.SOCCapRemain_fmt) + "\",";
    j += "\"cycles\":" + String(celldata.SOCCycleCount) + ",";
    j += "\"cycle_cap\":\"" + String(celldata.SOCCycleCap_fmt) + "\",";
    j += "\"runtime_fmt\":\"" + String(celldata.RunTime_fmt_dhms) + "\",";
    j += "\"charge_mos\":\"" + String(celldata.Charge_fmt) + "\",";
    j += "\"discharge_mos\":\"" + String(celldata.Discharge_fmt) + "\",";
    j += "\"balance_status\":\"" + String(celldata.BalanSta_fmt) + "\",";
    j += "\"precharge\":\"" + String(celldata.Precharge_fmt) + "\",";
    j += "\"heating\":\"" + String(celldata.Heating_fmt) + "\",";
    j += "\"alarm_mask\":" + String(celldata.AlarmBitMask) + ",";

    uint8_t alarmCount = 0;
    for (int i = 0; i < 24; i++)
    {
        if ((celldata.AlarmBitMask & (1UL << i)) != 0)
            alarmCount++;
    }
    j += "\"alarm_count\":" + String(alarmCount) + ",";
    j += "\"alarms\":[";
    bool firstAlarm = true;
    for (int i = 0; i < 24; i++)
    {
        if ((celldata.AlarmBitMask & (1UL << i)) != 0)
        {
            if (!firstAlarm)
                j += ",";
            firstAlarm = false;
            j += "\"" + jsonEscape(String(BMS_ALARM_NAMES[i])) + "\"";
        }
    }
    j += "]";
    j += "},";
    uint32_t uptimeSeconds = static_cast<uint32_t>(esp_timer_get_time() / 1000000ULL);
    uint32_t totalPublishedMessages = getTotalPublishedMessages();
    uint32_t messagesPerMinute = (uptimeSeconds > 0 && totalPublishedMessages > 0) ? (totalPublishedMessages * 60U) / uptimeSeconds : 0;
    String publishQueueSize = getState("publishqueuesize");
    String maxPublishQueue = getState("maxpubqueue");
    String maxPublishQueuePercent = "";
    uint32_t queueSize = static_cast<uint32_t>(publishQueueSize.toInt());
    if (queueSize > 0 && maxPublishQueue.length() > 0)
    {
        float percent = (static_cast<float>(maxPublishQueue.toInt()) * 100.0f) / static_cast<float>(queueSize);
        maxPublishQueuePercent = String(percent, 1) + "%";
    }
    String maxBufferTimeMinutes = "";
    if (queueSize > 0 && messagesPerMinute > 0)
    {
        float bufferMinutes = static_cast<float>(queueSize) / static_cast<float>(messagesPerMinute);
        maxBufferTimeMinutes = String(bufferMinutes, 1);
    }
    // publish task drains one message per publishInterval; incoming messages refill the queue while draining
    String drainTimeMinutes = "";
    if (queueSize > 0 && publishInterval > 0)
    {
        float drainRatePerMinute = 60000.0f / static_cast<float>(publishInterval);
        float netDrainRatePerMinute = drainRatePerMinute - static_cast<float>(messagesPerMinute);
        drainTimeMinutes = (netDrainRatePerMinute > 0.0f)
                               ? String(static_cast<float>(queueSize) / netDrainRatePerMinute, 1)
                               : "unendlich";
    }

    j += "\"status\":{";
    j += "\"messages_total\":" + String(totalPublishedMessages) + ",";
    j += "\"messages_per_minute\":" + String(messagesPerMinute) + ",";
    j += "\"publish_queue_size\":\"" + jsonEscape(publishQueueSize) + "\",";
    j += "\"max_publish_queue\":\"" + jsonEscape(maxPublishQueue) + "\",";
    j += "\"max_publish_queue_percent\":\"" + jsonEscape(maxPublishQueuePercent) + "\",";
    j += "\"max_buffer_time_min\":\"" + jsonEscape(maxBufferTimeMinutes) + "\",";
    j += "\"drain_time_min\":\"" + jsonEscape(drainTimeMinutes) + "\"";
    j += "},";
    j += "\"config\":{";
    j += "\"cell_count\":" + String(configinfo.CellCount[0]) + ",";
    j += "\"capacity_ah\":\"" + String(static_cast<float>(configinfo.CapBatCell) * 0.001f, 3) + "\",";
    j += "\"charge_en\":\"" + String(configinfo.BatChargeEN_fmt) + "\",";
    j += "\"discharge_en\":\"" + String(configinfo.BatDisChargeEN_fmt) + "\",";
    j += "\"balance_en\":\"" + String(configinfo.BalanEN_fmt) + "\",";
    j += "\"smart_sleep\":\"" + String(configinfo.SmartSleep) + "\",";
    j += "\"port_switch\":\"" + String(configinfo.PortSwitch) + "\"";
    j += "}}";

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", j);
}

void handleRoot()
{
    handleBmsPage(server);
}

void handleMqttConfig()
{
    if (!server.hasArg("field") || !server.hasArg("enabled"))
    {
        server.send(400, "text/plain", "Fehlende Parameter");
        return;
    }

    String field = server.arg("field");
    int separator = field.indexOf(':');
    if (separator <= 0 || separator >= field.length() - 1)
    {
        server.send(400, "text/plain", "Ungueltiges Feld");
        return;
    }

    String category = field.substring(0, separator);
    String suffix = field.substring(separator + 1);
    bool enabled = server.arg("enabled") == "1";
    setMqttPublishFieldEnabled(category.c_str(), suffix.c_str(), enabled);
    server.send(204, "text/plain", "");
}

void handleResetHistory()
{
    handleResetHistoryPage(server, g_history, g_historyCount);
}

void handleClear()
{
    if (g_history == nullptr || g_nvsKey == nullptr)
    {
        server.send(500, "text/plain", "Webserver not initialized");
        return;
    }

    memset(g_history, 0, sizeof(ResetEntry) * g_historyCount);
    prefs.begin("system", false);
    prefs.putBytes(g_nvsKey, g_history, sizeof(ResetEntry) * g_historyCount);
    prefs.end();

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleEspReset()
{
    server.send(200, "text/html", "<html><head><meta charset='UTF-8'><meta http-equiv='refresh' content='3;url=/'></head><body><p>ESP32 wird neu gestartet...</p><p><a href='/'>Zur&#252;ck</a></p></body></html>");
    delay(150);
    DEBUG_PRINT("Get Restart from Website by client ");
    DEBUG_PRINT(server.client().remoteIP());
    DEBUG_PRINTLN("...Restarting ESP32...");
    delay(1000);
    ESP.restart();
}
} // namespace

void setupWebserver(ResetEntry *history, size_t historyCount, const char *nvsKey)
{
    g_history = history;
    g_historyCount = historyCount;
    g_nvsKey = nvsKey;

    server.on("/", handleRoot);
    server.on("/reset_history", handleResetHistory);
    server.on("/clear", handleClear);
    server.on("/reset_esp", handleEspReset);
    server.on("/bms", handleRoot);
    server.on("/mqtt_config", []() { handleMqttConfigPage(server); });
    server.on("/api/mqtt_config", HTTP_POST, handleMqttConfig);
    server.on("/api/bms", handleBmsApi);
    server.on("/ota", []() {
        server.send(200, "text/plain", "Hi! This is ElegantOTA Demo 2.");
    });

    ElegantOTA.begin(&server);
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);

    server.begin();
}

void webserverLoop()
{
    server.handleClient();
    ElegantOTA.loop();
}

#endif // USE_WEBSERVER
