#include "parser.h"
#include <cstring>
#include <type_traits>

#define INFLUX 1
#define MQTT 0

uint32_t lastPublishTimeCellData = 0;
uint32_t lastDataReceivedTime = 0;
bool first_run = true;
const uint8_t pos_of_Counter = 5; // position of FrameCounter in the message

// variables for charge/discharge / day calculation
float Q_charged = 0;    // charge in As
float Q_discharged = 0; // discharge in As
float Q_charged_mAh = 0;
float Q_discharged_mAh = 0;

float Q_charged_mAh_old[2] = {0};
float Q_discharged_mAh_old[2] = {0};
char Q_charged_mAh_str[32];
char Q_discharged_mAh_str[32];
uint32_t battery_power_calculated[2] = {0};

uint8_t counter_last = 0;

constexpr uint32_t PARSER_PERF_LOG_INTERVAL_MS = 30000;
uint32_t parserPerfLastLogMs = 0;
uint32_t parserPerfFrameCount = 0;
uint32_t parserPerfHeapBeforeMin = UINT32_MAX;
uint32_t parserPerfHeapAfterMin = UINT32_MAX;
int32_t parserPerfHeapDeltaMax = 0;
int64_t parserPerfHeapDeltaSum = 0;

struct CellTopicCache
{
    bool valid;
    char devicename[32];
    char base_data[128];
    char base_debug[128];
    char base_device[128];
    char base_config[128];
    char cell_voltage[32][128];
    char cell_resistance[32][128];
};

CellTopicCache cellTopicCache = {};

char uart_protocol_number_str[][50]= {
    "000-4G-GPS Remote module Common protocol",
    "001-JK BMS RS485 Modbus V1.0",
    "002-MIU U SERIES",
    "003-China tower shared batterie cabinet V1.1",
    "004-PACE_RS485_Modbus_V1.3",
    "005-PYLON_low_volage_Protocol_RS485_V...",
    "006-Growatt_BMS_RS485_Protocol_1xSxxP...",
    "007-Voltronic_Inverter_and_BMS_485_com...",
    "008-China tower shared batterie cabinet V.02",
    "009-WOW_RS485_Modbus_V1.3",
    "010-JK BMS LCD Protocol V2.0",
    "011-UART1 User customization",
    "012-UART2 User customization",
    "013-(9600)JK BMS RS485 Modbus V1.0",
    "014-(9600)PYLON_low_voltage_Protocol_R...",
    "015-JK BMS PBxx SERIES LCD Protocol V1.0",
    "016-JKBMS LIN BUS V1.0",
    "017-RS485 Protocol 17",
    "018-RS485 Protocol 18",
    "019-RS485 Protocol 19",
    "020-RS485 Protocol 20"};

char can_protocol_number_str[][50]= {
    "000-JK BMS CAN Protocol (250k) V2.0",
    "001-Deye Low-voltage hybrid inverter CAN c...",
    "002-PYLON-Low-voltage-V1.2",
    "003-Growatt BMS CAN-Bus-protocol-low-vol...",
    "004-Victron_CANbus_BMS_protocol_20170...",
    "005-MEGAREVO_Hybrid_BMSCAN_Protocol...",
    "006-JK BMS CAN Protocol (500k) V2.0",
    "007-INVT BMS CAN Bus protocol V1.02",
    "008-GoodWe LV BMS Protocol(EX/EM/S-B...",
    "009-FSS-ConnectingBat-TI-en-10 | Version 1.0",
    "010-MUST PV1800F-CAN communication P...",
    "011-LuxpowerTek Battery CAN protocol V01",
    "012-CAN BUS User customization",
    "013-CAN BUS User customization2",
    "014-CAN BUS Protocol 014",
    "015-CAN BUS Protocol 015",
    "016-CAN BUS Protocol 016",
    "017-CAN BUS Protocol 017",
    "018-CAN BUS Protocol 018",
    "019-CAN BUS Protocol 019",
    "020-CAN BUS Protocol 020"
};

char Trigger_values_str[][30] = {
    "00-OFF",
    "01-Low SOC",
    "02-Battery Over Voltage",
    "03-Battery Under Voltage",
    "04-Battery Cell Over Voltage",
    "05-Battery Cell Under Voltage",
    "06-Charge Over Current",
    "07-Discharge Over Current",
    "08-Battery Over Temperature",
    "09-MOSFET Over Temperature",
    "10-System Alarm",
    "11-Battery Low Temperature",
    "12-Remote Control",
    "13-Above SOC",
    "14-MOSFET Abnormal"
};


struct PublishTimeSlot
{
    uint32_t hash;
    uint32_t lastPublishTime;
    bool used;
};

constexpr size_t PUBLISH_TIME_SLOT_COUNT = 128;
PublishTimeSlot publishTimeSlots[PUBLISH_TIME_SLOT_COUNT] = {};

static uint32_t hashTopic(const char *topic)
{
    // FNV-1a 32-bit hash for deterministic, allocation-free lookup.
    uint32_t hash = 2166136261u;
    while (*topic != '\0')
    {
        hash ^= static_cast<uint8_t>(*topic++);
        hash *= 16777619u;
    }
    return hash;
}

static PublishTimeSlot &resolvePublishTimeSlot(const char *topic)
{
    const uint32_t hash = hashTopic(topic);
    const size_t start = hash % PUBLISH_TIME_SLOT_COUNT;

    for (size_t i = 0; i < PUBLISH_TIME_SLOT_COUNT; ++i)
    {
        const size_t idx = (start + i) % PUBLISH_TIME_SLOT_COUNT;
        if (!publishTimeSlots[idx].used || publishTimeSlots[idx].hash == hash)
        {
            publishTimeSlots[idx].used = true;
            publishTimeSlots[idx].hash = hash;
            return publishTimeSlots[idx];
        }
    }

    // Fallback: overwrite deterministic slot if table is saturated.
    publishTimeSlots[start].used = true;
    publishTimeSlots[start].hash = hash;
    return publishTimeSlots[start];
}

template <typename T>
void publishIfChanged(T &currentValue, T newValue, const char *publishValue, const char *topic)
{
    if (topic == nullptr || publishValue == nullptr)
    {
        return;
    }

    uint32_t currentTime = millis();
    PublishTimeSlot &slot = resolvePublishTimeSlot(topic);
    uint32_t lastPublishTimeTopic = slot.lastPublishTime;

    // Check if the value has changed or MIN_PUB_TIME is greater than 0 and the time has passed since the last publish
    if (currentValue != newValue || (min_pub_time > 0 && (currentTime - lastPublishTimeTopic) >= (static_cast<uint32_t>(min_pub_time) * 1000UL)))
    {
        toMqttQueue(topic, publishValue);
        currentValue = newValue;
        slot.lastPublishTime = currentTime; // Update the last publish time for the topic
    }
}

template <typename T>
void publishIfChangedWithSuffix(T &currentValue, T newValue, const char *publishValue, const char *baseTopic, const char *suffix)
{
    char topic[192];
    snprintf(topic, sizeof(topic), "%s%s", baseTopic, suffix);
    publishIfChanged(currentValue, newValue, publishValue, topic);
}

static void toMqttQueueWithSuffix(const char *baseTopic, const char *suffix, const char *payload)
{
    char topic[192];
    snprintf(topic, sizeof(topic), "%s%s", baseTopic, suffix);
    toMqttQueue(topic, payload);
}

static void toMqttQueueWithSuffix(const char *baseTopic, const char *suffix, const String &payload)
{
    char topic[192];
    snprintf(topic, sizeof(topic), "%s%s", baseTopic, suffix);
    toMqttQueue(topic, payload.c_str());
}

template <typename T>
static typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, void>::type
toMqttQueueWithSuffixNumber(const char *baseTopic, const char *suffix, T payload)
{
    char payload_buffer[24];
    snprintf(payload_buffer, sizeof(payload_buffer), "%lu", static_cast<unsigned long>(payload));
    toMqttQueueWithSuffix(baseTopic, suffix, payload_buffer);
}

template <typename T>
static typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, void>::type
toMqttQueueWithSuffixNumber(const char *baseTopic, const char *suffix, T payload)
{
    char payload_buffer[24];
    snprintf(payload_buffer, sizeof(payload_buffer), "%ld", static_cast<long>(payload));
    toMqttQueueWithSuffix(baseTopic, suffix, payload_buffer);
}

static void toMqttQueueWithSuffixFloat(const char *baseTopic, const char *suffix, float payload, uint8_t precision)
{
    char payload_buffer[32];
    dtostrf(payload, 0, precision, payload_buffer);
    toMqttQueueWithSuffix(baseTopic, suffix, payload_buffer);
}

static void rebuildCellTopicCache(const char *devicename)
{
    memset(&cellTopicCache, 0, sizeof(cellTopicCache));
    cellTopicCache.valid = true;
    strncpy(cellTopicCache.devicename, devicename, sizeof(cellTopicCache.devicename) - 1);

    snprintf(cellTopicCache.base_data, sizeof(cellTopicCache.base_data), "%s%s/data/", mqtt_main_topic.c_str(), devicename);
    snprintf(cellTopicCache.base_debug, sizeof(cellTopicCache.base_debug), "%s%s/debug/", mqtt_main_topic.c_str(), devicename);
    snprintf(cellTopicCache.base_device, sizeof(cellTopicCache.base_device), "%s%s/device/", mqtt_main_topic.c_str(), devicename);
    snprintf(cellTopicCache.base_config, sizeof(cellTopicCache.base_config), "%s%s/config/", mqtt_main_topic.c_str(), devicename);

    for (uint8_t i = 0; i < 32; ++i)
    {
        snprintf(cellTopicCache.cell_voltage[i], sizeof(cellTopicCache.cell_voltage[i]), "%scells/voltage/cell_v_%02d", cellTopicCache.base_data, i + 1);
        snprintf(cellTopicCache.cell_resistance[i], sizeof(cellTopicCache.cell_resistance[i]), "%scells/resistance/cell_r_%02d", cellTopicCache.base_data, i + 1);
    }
}

static void ensureCellTopicCache(const char *devicename)
{
    if (!cellTopicCache.valid || strncmp(cellTopicCache.devicename, devicename, sizeof(cellTopicCache.devicename) - 1) != 0)
    {
        rebuildCellTopicCache(devicename);
    }
}

template <typename T>
void publishIfChanged(T &currentValue, T newValue, const char *publishValue, const String &topic)
{
    publishIfChanged(currentValue, newValue, publishValue, topic.c_str());
}
#ifdef USE_INFLUXDB
template <typename T>
void publishIfChangedInflux(T &currentValue, T newValue, const char *publishValue, const String &topic)
{
    if (currentValue != newValue)
    {
        publishToInfluxDB(topic, publishValue);
        currentValue = newValue;
    }
}
#endif

String getLocalTimeString()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        return "[NO TIME]";
    }
    char timeBuffer[32];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    return String(timeBuffer);
}

DeviceInfo deviceinfo;

void readDeviceInfoRecord(void *message, const char *devicename)
{

    // Startzeit für die Verarbeitung des Datensatzes
    // uint32_t start_time = millis();
    memcpy(&deviceinfo, message, 300); // Kopiere 300 Bytes in die Struktur

    ensureCellTopicCache(devicename);
    const char *base_device = cellTopicCache.base_device;

    // Publish the FrameCounter to MQTT
    toMqttQueueWithSuffixNumber(base_device, "read_count", deviceinfo.FrameCounter);

    // Publish Manufacturer Device ID
    toMqttQueueWithSuffix(base_device, "vendor_id", deviceinfo.ManufacturerDeviceID);

    // Publish Hardware Version and Software Version
    toMqttQueueWithSuffix(base_device, "hw_revision", deviceinfo.HardwareVersion);
    toMqttQueueWithSuffix(base_device, "sw_version", deviceinfo.SoftwareVersion);

    // Publish Uptime in seconds and in human-readable format (days, hours, minutes, seconds)
    toMqttQueueWithSuffixNumber(base_device, "uptime", deviceinfo.OddRunTime);
    toMqttQueueWithSuffix(base_device, "uptime_fmt", deviceinfo.getOddRunTimeStr());

    // Publish Power On Times
    toMqttQueueWithSuffixNumber(base_device, "power_up_times", deviceinfo.PwrOnTimes);

    // Publish Device Name, Passcode, Manufacturing Date, Serial Number, User Data, Setup Passcode and User Data 2
    toMqttQueueWithSuffix(base_device, "device_name", deviceinfo.DeviceName);
    toMqttQueueWithSuffix(base_device, "device_passwd", deviceinfo.DevicePasscode);
    toMqttQueueWithSuffix(base_device, "manufacturing_date", deviceinfo.ManufacturingDate);
    toMqttQueueWithSuffix(base_device, "serial_number", deviceinfo.SerialNumber);
#ifndef V19
    toMqttQueueWithSuffix(base_device, "passcode", deviceinfo.Passcode);
#endif
    toMqttQueueWithSuffix(base_device, "user_data", deviceinfo.UserData);
    toMqttQueueWithSuffix(base_device, "setup_passcode", deviceinfo.SetupPasscode);
    toMqttQueueWithSuffix(base_device, "user_data2", deviceinfo.UserData2);

// Publish UART and CAN protocol numbers and enable status
#ifdef PROTOCOL_NUMBERS_AND_ENABLE_STATUS
    toMqttQueueWithSuffixNumber(base_device, "uart1_protocol_number", deviceinfo.UART1MPRTOLNbr);
    toMqttQueueWithSuffix(base_device, "uart1_protocol_txt", uart_protocol_number_str[deviceinfo.UART1MPRTOLNbr]);
    toMqttQueueWithSuffixNumber(base_device, "uart1_protocol_enable", deviceinfo.UART1MPRTOLEnable);
    toMqttQueueWithSuffixNumber(base_device, "can_protocol_number", deviceinfo.CANMPRTOLNbr);
    toMqttQueueWithSuffix(base_device, "can_protocol_txt", can_protocol_number_str[deviceinfo.CANMPRTOLNbr]);
    toMqttQueueWithSuffixNumber(base_device, "uart_protocol_enable_0_15", deviceinfo.UARTMPRTOLEnable0_15);
    toMqttQueueWithSuffixNumber(base_device, "uart2_protocol_number", deviceinfo.UART2MPRTOLNbr);
    toMqttQueueWithSuffix(base_device, "uart2_protocol_txt", uart_protocol_number_str[deviceinfo.UART2MPRTOLNbr]);
    toMqttQueueWithSuffixNumber(base_device, "uart2_protocol_enable", deviceinfo.UART2MPRTOLEnable);
    toMqttQueueWithSuffixNumber(base_device, "uart_protocol_lib_version", deviceinfo.UARTMPTLVer);
#endif

// Publish trigger values for LCD buzzer and dry contacts
#ifdef LCD_AND_DRY_TRIGGER_VALUES
    toMqttQueueWithSuffixNumber(base_device, "lcd_buzzer_trigger", deviceinfo.LCDBuzzerTrigger);
    toMqttQueueWithSuffix(base_device, "lcd_buzzer_trigger_txt", Trigger_values_str[deviceinfo.LCDBuzzerTrigger]);
    toMqttQueueWithSuffixNumber(base_device, "lcd_buzzer_trigger_value", deviceinfo.LCDBuzzerTriggerVal);
    toMqttQueueWithSuffixNumber(base_device, "lcd_buzzer_release_value", deviceinfo.LCDBuzzerReleaseVal);
    toMqttQueueWithSuffixNumber(base_device, "dry1_trigger", deviceinfo.DRY1Trigger);
    toMqttQueueWithSuffix(base_device, "dry1_trigger_txt", Trigger_values_str[deviceinfo.DRY1Trigger]);
    toMqttQueueWithSuffixNumber(base_device, "dry1_trigger_value", deviceinfo.DRY1TriggerVal);
    toMqttQueueWithSuffixNumber(base_device, "dry1_release_value", deviceinfo.DRY1ReleaseVal);
    toMqttQueueWithSuffixNumber(base_device, "dry2_trigger", deviceinfo.DRY2Trigger);
    toMqttQueueWithSuffix(base_device, "dry2_trigger_txt", Trigger_values_str[deviceinfo.DRY2Trigger]);
    toMqttQueueWithSuffixNumber(base_device, "dry2_trigger_value", deviceinfo.DRY2TriggerVal);
    toMqttQueueWithSuffixNumber(base_device, "dry2_release_value", deviceinfo.DRY2ReleaseVal);
    toMqttQueueWithSuffixNumber(base_device, "can_protocol_lib_version", deviceinfo.CANMPTLVer);
#endif

    // Publish RCV Time and RFV Time
    toMqttQueueWithSuffixFloat(base_device, "rcv_time", static_cast<float>(deviceinfo.RCVTime) * 0.1f, 1); // Assuming RCVTime is in 0.1h units
    toMqttQueueWithSuffixFloat(base_device, "rfv_time", static_cast<float>(deviceinfo.RFVTime) * 0.1f, 1); // Assuming RFVTime is in 0.1h units

}

CellData celldata;
CellDataOld cdOld[2]; // Array to store old values for MQTT and INFLUX, used for comparison and change detection

void readCellDataRecord(void *message, const char *devicename)
{
    uint32_t start_time = millis();
    
    // if first run initialize lastDataReceivedTime and lastPublishTimeCellData with the current time and return without processing the data.
    if (first_run)
    {
        lastDataReceivedTime = start_time;
        lastPublishTimeCellData = start_time;
        first_run = false;
        return;
    }
    
    // Calculate the time difference since the last data was received
    // and integrate the charge and discharge current over time to get the charged and discharged capacity
    float deltaT = (start_time - lastDataReceivedTime) / 1000.0; // time in seconds
    lastDataReceivedTime = start_time;
    // actual charge current
    int32_t current_raw;
    // get only the current value from the message
    memcpy(&current_raw, (uint8_t *)message + 158, sizeof(int32_t)); // current is at byte offset 158
    float_t current = current_raw * 0.001; // convert to A using the 0.001 multiplier from the documentation

    // Integration of charge/discharge current
    if (current > 0)
    {
        Q_charged += current * deltaT;
    }
    else
    {
        Q_discharged += (-current) * deltaT;
    }
        
    // If publish_delay is set and the time since the last publish is less than the delay, skip publishing
    if (publish_delay > 0 && (start_time - lastPublishTimeCellData) < (publish_delay * 1000)) return;

    uint32_t heapBeforeCycle = ESP.getFreeHeap();
        
    // Update the last publish time
    lastPublishTimeCellData = millis();
    
    // publishing values from here on if either the publish delay has passed

    // prepare charge and discharge in mAh with 3 decimal places
    Q_charged_mAh = Q_charged / 3.6;
    Q_discharged_mAh = Q_discharged / 3.6;
    dtostrf(Q_charged_mAh, 0, 3, Q_charged_mAh_str);
    dtostrf(Q_discharged_mAh, 0, 3, Q_discharged_mAh_str);
        
    memcpy(&celldata, message, 300); // Kopiere 300 Bytes message in die Struktur
    celldata.prepareOutValues(); // Bereite die formatierten Strings für die Ausgabe vor

    ensureCellTopicCache(devicename);
    const char *base_data = cellTopicCache.base_data;
    const char *base_debug = cellTopicCache.base_debug;

    // Publish Frame Counter
    char readcount_payload[16];
    snprintf(readcount_payload, sizeof(readcount_payload), "%u", celldata.FrameCounter);
    toMqttQueueWithSuffix(base_data, "readcount", readcount_payload);
    // Publish charge and discharge in mAh with change detection with 3 decimal places
    publishIfChangedWithSuffix(Q_charged_mAh_old[MQTT], Q_charged_mAh, Q_charged_mAh_str, base_data, "battery_charged_mAh");
    publishIfChangedWithSuffix(Q_discharged_mAh_old[MQTT], Q_discharged_mAh, Q_discharged_mAh_str, base_data, "battery_discharged_mAh");

    if (debug_flg_full)
    {
        char message_base64[base64::encodeLength(300)];                   // Buffer für die Base64-kodierte Nachricht
        uint8_t *rawDataPtr = (uint8_t *)&celldata;                       // Zeiger auf die Rohdaten der Struktur
        base64::encode((const uint8_t *)rawDataPtr, 300, message_base64); // Base64-kodieren der Rohdaten);
        char rawdata_topic[192];
        snprintf(rawdata_topic, sizeof(rawdata_topic), "%s%s", base_debug, "rawdata");
        toMqttQueueRawData(rawdata_topic, message_base64, strlen(message_base64));
        toMqttQueueWithSuffix(base_debug, "enabled", "true");
    }
    else
    {
        if (debug_flg)
        {
            toMqttQueueWithSuffix(base_debug, "rawdata", "not published");
            toMqttQueueWithSuffix(base_debug, "enabled", "false");
        }
    }

    // Cell Voltages
    for (uint8_t i = 0; i < 32; i++)
    {
        if (celldata.CellVol[i] != 0)
        {
            publishIfChanged(cdOld[MQTT].CellVol[i], celldata.CellVol[i], celldata.CellVol_fmt[i], cellTopicCache.cell_voltage[i]);
#ifdef INFLUX_CELLS_VOLTAGE
            publishToInfluxDB("cell_" + String(i + 1), celldata.CellVol[i]);
#endif
        }
    }

    // CellSta as bitmask
    if (debug_flg)
    {
        publishIfChangedWithSuffix(cdOld[MQTT].CellSta, celldata.CellSta, celldata.CellSta_fmt, base_data, "cells_used");
    }

    // Cell Average Voltage
    publishIfChangedWithSuffix(cdOld[MQTT].CellVolAve, celldata.CellVolAve, celldata.CellVolAve_fmt, base_data, "cells/voltage/cell_avg_voltage");

    // Cell Voltage Difference
    publishIfChangedWithSuffix(cdOld[MQTT].CellVdifMax, celldata.CellVdifMax, celldata.CellVdifMax_fmt, base_data, "cells/voltage/cell_diff_voltage");

    // High Voltage Cell
    publishIfChangedWithSuffix(cdOld[MQTT].MaxVolCellNbr, celldata.MaxVolCellNbr, celldata.MaxVolCellNbr_fmt, base_data, "cells/voltage/high_voltage_cell");

    // Low Voltage Cell
    publishIfChangedWithSuffix(cdOld[MQTT].MinVolCellNbr, celldata.MinVolCellNbr, celldata.MinVolCellNbr_fmt, base_data, "cells/voltage/low_voltage_cell");

    // Cell resistances
    for (uint8_t i = 0; i < 32; i++)
    {
        if (celldata.CellWireRes[i] != 0)
        {
            publishIfChanged(cdOld[MQTT].CellWireRes[i], celldata.CellWireRes[i], celldata.CellWireRes_fmt[i], cellTopicCache.cell_resistance[i]);
        }
    }

    // Temp MOSFET
    publishIfChangedWithSuffix(cdOld[MQTT].TempMos, celldata.TempMos, celldata.TempMos_fmt, base_data, "temperatures/temp_mosfet");
#ifdef INFLUX_TEMP_SENSOR_MOSFET
    publishIfChangedInflux(cdOld[INFLUX].TempMos, celldata.TempMos, celldata.TempMos_fmt, "temp_mosfet");
#endif

    // Cell Wire Resistance Status as bitmask
    if (debug_flg)
    {
        publishIfChangedWithSuffix(cdOld[MQTT].CellWireResSta, celldata.CellWireResSta, celldata.CellWireResSta_fmt, base_data, "cell_resistance_alert");
    }

    // Battery Total Voltage
    publishIfChangedWithSuffix(cdOld[MQTT].BatVol, celldata.BatVol, celldata.BatVol_fmt, base_data, "battery_voltage");
#ifdef INFLUX_BATTERY_VOLTAGE
    publishIfChangedInflux(cdOld[INFLUX].BatVol, celldata.BatVol, celldata.BatVol_fmt, "battery_voltage");
#endif

    // Battery Power
    publishIfChangedWithSuffix(cdOld[MQTT].BatWatt, celldata.BatWatt, celldata.BatWatt_fmt, base_data, "battery_power");
#ifdef INFLUX_BATTERY_POWER
    publishIfChangedInflux(cdOld[INFLUX].BatWatt, celldata.BatWatt, celldata.BatWatt_fmt, "battery_power");
#endif

    // Battery Current
    publishIfChangedWithSuffix(cdOld[MQTT].BatCurrent, celldata.BatCurrent, celldata.BatCurrent_fmt, base_data, "battery_current");
#ifdef INFLUX_BATTERY_CURRENT
    publishIfChangedInflux(cdOld[INFLUX].BatCurrent, celldata.BatCurrent, celldata.BatCurrent_fmt, "battery_current");
#endif

    // Signed Battery Power for MQTT and InfluxDB
    // this is to have the correct sign for the power (discharging "-" or charging)
    char signed_BatWatt_str[32];
    dtostrf((celldata.BatCurrent < 0 ? (float)(0 - celldata.BatWatt * 0.001) : (float)(celldata.BatWatt * 0.001)), 0, 3, signed_BatWatt_str); // to have the correct sign for the power (discharging "-" or charging)
    publishIfChangedWithSuffix(battery_power_calculated[MQTT], celldata.BatWatt, signed_BatWatt_str, base_data, "battery_power_calculated");
#ifdef INFLUX_BATTERY_POWER
    publishIfChangedInflux(battery_power_calculated[INFLUX], celldata.BatWatt, signed_BatWatt_str, "battery_power");
#endif

    // Batterie Temp Sensor 1
    publishIfChangedWithSuffix(cdOld[MQTT].TempBat1, celldata.TempBat1, celldata.TempBat1_fmt, base_data, "temperatures/temp_sensor1");
#ifdef INFLUX_TEMP_SENSOR_1
    publishIfChangedInflux(cdOld[INFLUX].TempBat1, celldata.TempBat1, celldata.TempBat1_fmt, "temp_sensor1");
#endif

    // Batterie Temp Sensor 2
    publishIfChangedWithSuffix(cdOld[MQTT].TempBat2, celldata.TempBat2, celldata.TempBat2_fmt, base_data, "temperatures/temp_sensor2");
#ifdef INFLUX_TEMP_SENSOR_2
    publishIfChangedInflux(cdOld[INFLUX].TempBat2, celldata.TempBat2, celldata.TempBat2_fmt, "temp_sensor2");
#endif

    // Alarms as raw dezimal, bitmask and resolved alarms according to BMS RS485 ModbusV1.1 2024.02 Page 10
    publishIfChangedWithSuffix(cdOld[MQTT].AlarmBitMask, celldata.AlarmBitMask, celldata.Alarm_raw_fmt, base_data, "alarms/alarm_raw");

    if (debug_flg)
    {
        publishIfChangedWithSuffix(cdOld[MQTT].AlarmBitMask, celldata.AlarmBitMask, celldata.AlarmBitMask_fmt, base_data, "alarms/alarms_mask");
    }
    for (int i = 0; i < 24; ++i)
    {
        char alarm_topic[192];
        snprintf(alarm_topic, sizeof(alarm_topic), "%salarms/%s", base_data, celldata.AlarmsTopics[i]);
        publishIfChanged(cdOld[MQTT].AlarmBitMask, celldata.AlarmBitMask, celldata.AlarmsValue_fmt[i], alarm_topic);
    }

    // Balance Current
    publishIfChangedWithSuffix(cdOld[MQTT].BalanCurrent, celldata.BalanCurrent, celldata.BalanCurrent_fmt, base_data, "balance_current");

    // Balance Status
    publishIfChangedWithSuffix(cdOld[MQTT].BalanSta, celldata.BalanSta, celldata.BalanSta_fmt, base_data, "balance_status");

    // State of Charge
    publishIfChangedWithSuffix(cdOld[MQTT].SOCStateOfcharge, celldata.SOCStateOfcharge, celldata.SOCStateOfcharge_fmt, base_data, "battery_soc");
#ifdef INFLUX_BATTERY_SOC
    publishIfChangedInflux(cdOld[INFLUX].SOCStateOfcharge, celldata.SOCStateOfcharge, celldata.SOCStateOfcharge_fmt, "battery_soc");
#endif

    // Battery Capacity Remaining
    publishIfChangedWithSuffix(cdOld[MQTT].SOCCapRemain, celldata.SOCCapRemain, celldata.SOCCapRemain_fmt, base_data, "battery_capacity_remaining");

    // Battery Capacity Total
    publishIfChangedWithSuffix(cdOld[MQTT].SOCFullChargeCap, celldata.SOCFullChargeCap, celldata.SOCFullChargeCap_fmt, base_data, "battery_capacity_total");

    // Battery Cycle Count
    publishIfChangedWithSuffix(cdOld[MQTT].SOCCycleCount, celldata.SOCCycleCount, celldata.SOCCycleCount_fmt, base_data, "battery_cycle_count");

    // Battery Cycle Capacity Total
    publishIfChangedWithSuffix(cdOld[MQTT].SOCCycleCap, celldata.SOCCycleCap, celldata.SOCCycleCap_fmt, base_data, "battery_cycle_capacity_total");

    // Battery SOH
    publishIfChangedWithSuffix(cdOld[MQTT].SOCSOH, celldata.SOCSOH, celldata.SOCSOH_fmt, base_data, "battery_soh");

    // Battery Precharge Status
    publishIfChangedWithSuffix(cdOld[MQTT].Precharge, celldata.Precharge, celldata.Precharge_fmt, base_data, "battery_precharge_status");

    // Battery User Alarm 1
    publishIfChangedWithSuffix(cdOld[MQTT].UserAlarm, celldata.UserAlarm, celldata.UserAlarm_fmt, base_data, "battery_user_alarm1");

    // Battery Total Runtime in seconds and in human-readable format
    publishIfChangedWithSuffix(cdOld[MQTT].RunTime, celldata.RunTime, celldata.RunTime_fmt, base_data, "battery_total_runtime_sec");
    publishIfChangedWithSuffix(cdOld[MQTT].RunTime2, celldata.RunTime, celldata.RunTime_fmt_dhms, base_data, "battery_total_runtime_fmt");

    // Charging MOSFET Status
    publishIfChangedWithSuffix(cdOld[MQTT].Charge, celldata.Charge, celldata.Charge_fmt, base_data, "charging_mosfet_status");

    // Discharging MOSFET Status
    publishIfChangedWithSuffix(cdOld[MQTT].Discharge, celldata.Discharge, celldata.Discharge_fmt, base_data, "discharging_mosfet_status");

    // Battery User Alarm 2
    publishIfChangedWithSuffix(cdOld[MQTT].UserAlarm2, celldata.UserAlarm2, celldata.UserAlarm2_fmt, base_data, "battery_user_alarm2");

    // Time Discharge Over Current Protection Release (timeDcOCPR)
    publishIfChangedWithSuffix(cdOld[MQTT].TimeDcOCPR, celldata.TimeDcOCPR, celldata.TimeDcOCPR_fmt, base_data, "timeDcOCPR");

    // Time Discharge Short Circuit Protection Release (timeDcSCPR)
    publishIfChangedWithSuffix(cdOld[MQTT].TimeDcSCPR, celldata.TimeDcSCPR, celldata.TimeDcSCPR_fmt, base_data, "timeDcSCPR");

    // Time Charge Over Current Protection Release (timeCOCPR)
    publishIfChangedWithSuffix(cdOld[MQTT].TimeCOCPR, celldata.TimeCOCPR, celldata.TimeCOCPR_fmt, base_data, "timeCOCPR");

    // Time Charge Short Circuit Protection Release (timeCSCPR)
    publishIfChangedWithSuffix(cdOld[MQTT].TimeCSCPR, celldata.TimeCSCPR, celldata.TimeCSCPR_fmt, base_data, "timeCSCPR");

    // Time Under Voltage Protection Release (timeUVPR)
    publishIfChangedWithSuffix(cdOld[MQTT].TimeUVPR, celldata.TimeUVPR, celldata.TimeUVPR_fmt, base_data, "timeUVPR");

    // Time Over Voltage Protection Release (timeOVPR)
    publishIfChangedWithSuffix(cdOld[MQTT].TimeOVPR, celldata.TimeOVPR, celldata.TimeOVPR_fmt, base_data, "timeOVPR");

    // Temperature Sensor Absent Mask as raw dezimal, bitmask and resolved according to BMS RS485 ModbusV1.1 2024.02 Page 11
    publishIfChangedWithSuffix(cdOld[MQTT].TempSensorAbsentMask, celldata.TempSensorAbsentMask, celldata.TempSensorAbsent_fmt, base_data, "temperatures/temp_sensor_absent");
    if (debug_flg)
    {
        publishIfChangedWithSuffix(cdOld[MQTT].TempSensorAbsentMask, celldata.TempSensorAbsentMask, celldata.TempSensorAbsentMask_fmt, base_data, "temperatures/temp_sensor_absent_mask");
    }
    for (int i = 0; i < 6; ++i)
    {
        char temp_absent_topic[192];
        snprintf(temp_absent_topic, sizeof(temp_absent_topic), "%stemperatures/%s", base_data, celldata.TempSensorsAbsentTopics[i]);
        publishIfChanged(cdOld[MQTT].TempSensorAbsentMask, celldata.TempSensorAbsentMask, celldata.TempSensAbsentValues_fmt[i], temp_absent_topic);
    }

    // Battery Heating
    publishIfChangedWithSuffix(cdOld[MQTT].Heating, celldata.Heating, celldata.Heating_fmt, base_data, "temperatures/battery_heating");

    // Time Emergency
    publishIfChangedWithSuffix(cdOld[MQTT].TimeEmergency, celldata.TimeEmergency, celldata.TimeEmergency_fmt, base_data, "time_emergency");

    // Heating Current
    publishIfChangedWithSuffix(cdOld[MQTT].HeatCurrent, celldata.HeatCurrent, celldata.HeatCurrent_fmt, base_data, "heat_current");

    // System run ticks, unit: 0.1s, range: 0~4294967295
    publishIfChangedWithSuffix(cdOld[MQTT].SysRunTicks, celldata.SysRunTicks, celldata.SysRunTicks_fmt, base_data, "sys_run_ticks");

    // Batterie Temp Sensor3
    publishIfChangedWithSuffix(cdOld[MQTT].TempBat3, celldata.TempBat3, celldata.TempBat3_fmt, base_data, "temperatures/temp_sensor3");
#ifdef INFLUX_TEMP_SENSOR_3
    publishIfChangedInflux(cdOld[INFLUX].TempBat3, celldata.TempBat3, celldata.TempBat3_fmt, "temp_sensor3");
#endif

    // Batterie Temp Sensor4
    if (celldata.TempBat4 > 1200 || celldata.TempBat4 < -400)
    { // if the value is out of range, it could be that the sensor is not present or not working, so we publish a separate topic for this
        publishIfChangedWithSuffix(cdOld[MQTT].TempBat4, celldata.TempBat4, "out of range", base_data, "temperatures/temp_sensor4");
    }
    else
    {
        publishIfChangedWithSuffix(cdOld[MQTT].TempBat4, celldata.TempBat4, celldata.TempBat4_fmt, base_data, "temperatures/temp_sensor4");
#ifdef INFLUX_TEMP_SENSOR_4
        publishIfChangedInflux(cdOld[INFLUX].TempBat4, celldata.TempBat4, celldata.TempBat4_fmt, "temp_sensor4");
#endif
    }

    // Batterie Temp Sensor5
    if (celldata.TempBat5 > 1200 || celldata.TempBat5 < -400)
    { // if the value is out of range, it could be that the sensor is not present or not working, so we publish a separate topic for this
        publishIfChangedWithSuffix(cdOld[MQTT].TempBat5, celldata.TempBat5, "out of range", base_data, "temperatures/temp_sensor5");
    }
    else
    {
        publishIfChangedWithSuffix(cdOld[MQTT].TempBat5, celldata.TempBat5, celldata.TempBat5_fmt, base_data, "temperatures/temp_sensor5");
#ifdef INFLUX_TEMP_SENSOR_5
        publishIfChangedInflux(cdOld[INFLUX].TempBat5, celldata.TempBat5, celldata.TempBat5_fmt, "temp_sensor5");
#endif
    }

    // RTC Ticks
    publishIfChangedWithSuffix(cdOld[MQTT].RTCTicks, celldata.RTCTicks, celldata.RTCTicksToSeconds_fmt, base_data, "rtc_ticks");

    uint32_t heapAfterCycle = ESP.getFreeHeap();
    int32_t heapDelta = static_cast<int32_t>(heapBeforeCycle) - static_cast<int32_t>(heapAfterCycle);

    parserPerfFrameCount++;
    parserPerfHeapBeforeMin = std::min(parserPerfHeapBeforeMin, heapBeforeCycle);
    parserPerfHeapAfterMin = std::min(parserPerfHeapAfterMin, heapAfterCycle);
    parserPerfHeapDeltaMax = std::max(parserPerfHeapDeltaMax, heapDelta);
    parserPerfHeapDeltaSum += heapDelta;

    if (millis() - parserPerfLastLogMs >= PARSER_PERF_LOG_INTERVAL_MS)
    {
        char valueBuf[24];

        snprintf(valueBuf, sizeof(valueBuf), "%lu", static_cast<unsigned long>(parserPerfFrameCount));
        setState("parser_frames_30s", valueBuf, false);

        snprintf(valueBuf, sizeof(valueBuf), "%lu", static_cast<unsigned long>(parserPerfHeapBeforeMin));
        setState("parser_heap_before_min", valueBuf, false);

        snprintf(valueBuf, sizeof(valueBuf), "%lu", static_cast<unsigned long>(parserPerfHeapAfterMin));
        setState("parser_heap_after_min", valueBuf, false);

        snprintf(valueBuf, sizeof(valueBuf), "%ld", static_cast<long>(parserPerfHeapDeltaMax));
        setState("parser_heap_delta_max", valueBuf, false);

        int32_t parserHeapDeltaAvg = (parserPerfFrameCount > 0)
                                         ? static_cast<int32_t>(parserPerfHeapDeltaSum / static_cast<int64_t>(parserPerfFrameCount))
                                         : 0;
        snprintf(valueBuf, sizeof(valueBuf), "%ld", static_cast<long>(parserHeapDeltaAvg));
        setState("parser_heap_delta_avg", valueBuf, false);

        DEBUG_PRINTF(
            "Parser perf 30s: frames=%lu heap_before_min=%lu heap_after_min=%lu heap_delta_max=%ld heap_delta_avg=%ld\n",
            static_cast<unsigned long>(parserPerfFrameCount),
            static_cast<unsigned long>(parserPerfHeapBeforeMin),
            static_cast<unsigned long>(parserPerfHeapAfterMin),
            static_cast<long>(parserPerfHeapDeltaMax),
            static_cast<long>(parserHeapDeltaAvg));

        parserPerfLastLogMs = millis();
        parserPerfFrameCount = 0;
        parserPerfHeapBeforeMin = UINT32_MAX;
        parserPerfHeapAfterMin = UINT32_MAX;
        parserPerfHeapDeltaMax = 0;
        parserPerfHeapDeltaSum = 0;
    }

    if (debug_flg)
    {
        uint32_t MinFreeHeap = ESP.getMinFreeHeap();
        char timeBuffer[32];
        char finishCellMsg[192];
        String localTime = getLocalTimeString();

        strncpy(timeBuffer, localTime.c_str(), sizeof(timeBuffer) - 1);
        timeBuffer[sizeof(timeBuffer) - 1] = '\0';

        snprintf(
            finishCellMsg,
            sizeof(finishCellMsg),
            "%s: Finished parsing Cell data. Frame: %u (Processing time: %lu ms, Min Free Heap: %lu bytes)",
            timeBuffer,
            celldata.FrameCounter,
            static_cast<unsigned long>(millis() - start_time),
            static_cast<unsigned long>(MinFreeHeap));
        DEBUG_PRINTLN(finishCellMsg);
    }
}

ConfigInfo configinfo;

void readConfigInfoRecord(void *message, const char *devicename)
{
    // Startzeit für die Verarbeitung des Datensatzes
    // uint32_t start_time = millis();

    // Kopiere die empfangenen Bytes in die ConfigInfo-Struktur
    memcpy(&configinfo, message, 300); // Kopiere 300 Bytes in die Struktur
    configinfo.update_switches();

    ensureCellTopicCache(devicename);
    const char *base_config = cellTopicCache.base_config;

    // Veröffentliche die Konfigurationsdaten auf MQTT
    // VolSmartSleep
    toMqttQueueWithSuffixFloat(base_config, "vol_smart_sleep", static_cast<float>(configinfo.VolSmartSleep) * 0.001f, 3);
    // VolCellUV
    toMqttQueueWithSuffixFloat(base_config, "vol_cell_uv", static_cast<float>(configinfo.VolCellUV) * 0.001f, 3);
    // VolCellUVPR
    toMqttQueueWithSuffixFloat(base_config, "vol_cell_uvpr", static_cast<float>(configinfo.VolCellUVPR) * 0.001f, 3);
    // VolCellOV
    toMqttQueueWithSuffixFloat(base_config, "vol_cell_ov", static_cast<float>(configinfo.VolCellOV) * 0.001f, 3);
    // VolCellOVPR
    toMqttQueueWithSuffixFloat(base_config, "vol_cell_ovpr", static_cast<float>(configinfo.VolCellOVPR) * 0.001f, 3);
    // VolBalanTrig
    toMqttQueueWithSuffixFloat(base_config, "vol_balan_trig", static_cast<float>(configinfo.VolBalanTrig) * 0.001f, 3);
    // VolSOC100%
    toMqttQueueWithSuffixFloat(base_config, "vol_100_percent", static_cast<float>(configinfo.VolSOC100percent) * 0.001f, 3);
    // VolSOC0%
    toMqttQueueWithSuffixFloat(base_config, "vol_0_percent", static_cast<float>(configinfo.VolSOC0percent) * 0.001f, 3);
    // VolCellRCV
    toMqttQueueWithSuffixFloat(base_config, "vol_cell_rcv", static_cast<float>(configinfo.VolCellRCV) * 0.001f, 3);
    // VolCellRFV
    toMqttQueueWithSuffixFloat(base_config, "vol_cell_rfv", static_cast<float>(configinfo.VolCellRFV) * 0.001f, 3);
    // VolSysPwrOff
    toMqttQueueWithSuffixFloat(base_config, "vol_sys_pwr_off", static_cast<float>(configinfo.VolSysPwrOff) * 0.001f, 3);
    // CurBatCOC
    toMqttQueueWithSuffixFloat(base_config, "cur_bat_coc", static_cast<float>(configinfo.CurBatCOC) * 0.001f, 3);
    // TIMBatCOCPDly
    toMqttQueueWithSuffixNumber(base_config, "time_bat_cocp_delay", configinfo.TimBatCOCPDly);
    // TIMBatCOCPRDDly
    toMqttQueueWithSuffixNumber(base_config, "time_bat_cocprd_delay", configinfo.TimBatCOCPRDly);
    // CurBatDcOC
    toMqttQueueWithSuffixFloat(base_config, "cur_bat_dc_oc", static_cast<float>(configinfo.CurBatDcOC) * 0.001f, 3);
    // TIMBatDcOCPDly
    toMqttQueueWithSuffixNumber(base_config, "time_bat_dc_ocp_delay", configinfo.TimBatDcOCPDly);
    // TIMBatDcOPRDDly
    toMqttQueueWithSuffixNumber(base_config, "time_bat_dc_oprd_delay", configinfo.TimBatDcOCPRDly);
    // TIMBatSCPRDly
    toMqttQueueWithSuffixNumber(base_config, "time_bat_scprd_delay", configinfo.TimBatSCPRDly);
    // CurBalanMax
    toMqttQueueWithSuffixFloat(base_config, "cur_balance_max", static_cast<float>(configinfo.CurBalanMax) * 0.001f, 3);
    // TMPBatCOT
    toMqttQueueWithSuffixFloat(base_config, "tmp_bat_cot", static_cast<float>(configinfo.TmpBatCOT) * 0.1f, 1);
    // TMPBatCOTPR
    toMqttQueueWithSuffixFloat(base_config, "tmp_bat_cotpr", static_cast<float>(configinfo.TmpBatCOTPR) * 0.1f, 1);
    // TMPBatDcOT
    toMqttQueueWithSuffixFloat(base_config, "tmp_bat_dc_ot", static_cast<float>(configinfo.TmpBatDcOT) * 0.1f, 1);
    // TMPBatDcOTPR
    toMqttQueueWithSuffixFloat(base_config, "tmp_bat_dc_otpr", static_cast<float>(configinfo.TmpBatDcOTPR) * 0.1f, 1);
    // TMPBatCUT
    toMqttQueueWithSuffixFloat(base_config, "tmp_bat_cut", static_cast<float>(configinfo.TmpBatCUT) * 0.1f, 1);
    // TMPBatCUTPR
    toMqttQueueWithSuffixFloat(base_config, "tmp_bat_cutpr", static_cast<float>(configinfo.TmpBatCUTPR) * 0.1f, 1);
    // TMPMosOT
    toMqttQueueWithSuffixFloat(base_config, "tmp_mos_ot", static_cast<float>(configinfo.TmpMosOT) * 0.1f, 1);
    // TMPMosOTPR
    toMqttQueueWithSuffixFloat(base_config, "tmp_mos_otpr", static_cast<float>(configinfo.TmpMosOTPR) * 0.1f, 1);
    // CellCount
    toMqttQueueWithSuffixNumber(base_config, "cell_count", configinfo.CellCount[0]);
    // BatChargeEN
    toMqttQueueWithSuffix(base_config, "switches/bat_charge_enabled", configinfo.BatChargeEN_fmt);
    // BatDisChargeEN
    toMqttQueueWithSuffix(base_config, "switches/bat_discharge_enabled", configinfo.BatDisChargeEN_fmt);
    // BalanEN
    toMqttQueueWithSuffix(base_config, "switches/balancing_enabled", configinfo.BalanEN_fmt);
    // CapBatCell
    toMqttQueueWithSuffixFloat(base_config, "cap_bat_cell", static_cast<float>(configinfo.CapBatCell) * 0.001f, 3);
    // SCPDelay
    toMqttQueueWithSuffixNumber(base_config, "scp_delay", configinfo.ScpDelay);
    // VolStartBalan
    toMqttQueueWithSuffixFloat(base_config, "vol_start_balance", static_cast<float>(configinfo.VolStartBalan) * 0.001f, 3);
    // DevAddr
    toMqttQueueWithSuffixNumber(base_config, "dev_address", configinfo.DevAddr);
    // TIMProdischarge
    toMqttQueueWithSuffixNumber(base_config, "tim_pro_discharge", configinfo.TimProdischarge);
    // HeatEN
    toMqttQueueWithSuffix(base_config, "switches/heating_enabled", configinfo.HeatEN);
    // Disable temp-sensor
    toMqttQueueWithSuffix(base_config, "switches/temp_sensor_disabled", configinfo.DisableTempSensor);
    // GPS Heartbeat
    toMqttQueueWithSuffix(base_config, "switches/gps_heartbeat", configinfo.GPSHeartbeat);
    // Port Switch
    toMqttQueueWithSuffix(base_config, "switches/port_switch", configinfo.PortSwitch);
    // LCD Always On
    toMqttQueueWithSuffix(base_config, "switches/lcd_always_on", configinfo.LCDAlwaysOn);
    // Special Charger
    toMqttQueueWithSuffix(base_config, "switches/special_charger", configinfo.SpecialCharger);
    // SmartSleep
    toMqttQueueWithSuffix(base_config, "switches/smart_sleep", configinfo.SmartSleep);
    // DisablePCLModule
    toMqttQueueWithSuffix(base_config, "switches/disable_pcl_module", configinfo.DisablePCLModule);
    // TimedStoredData
    toMqttQueueWithSuffix(base_config, "switches/timed_stored_data", configinfo.TimedStoredData);
    // ChargingFloatMode
    toMqttQueueWithSuffix(base_config, "switches/charging_float_mode", configinfo.ChargingFloatMode);
    // TMPHeatingStart
    toMqttQueueWithSuffixNumber(base_config, "tmp_heating_start", configinfo.TmpHeatingStart);
    // TMPHeatingStop
    toMqttQueueWithSuffixNumber(base_config, "tmp_heating_stop", configinfo.TmpHeatingStop);
    // TIMSmartSleep
    toMqttQueueWithSuffixNumber(base_config, "time_smart_sleep", configinfo.TimSmartSleep);
}
