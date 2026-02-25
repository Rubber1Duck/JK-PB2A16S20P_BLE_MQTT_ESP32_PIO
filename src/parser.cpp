#include "parser.h"

#define INFLUX 1
#define MQTT 0

uint32_t lastPublishTimeCellData = 0;
uint32_t lastDataReceivedTime = 0;
bool first_run = true;
const uint8_t pos_of_Counter = 5; // position of FrameCounter in the message

// variables for charge/discharge / day calculation
float Q_charged = 0;    // charge in As
float Q_discharged = 0; // discharge in As

float Q_charged_mAh_old[2] = {0};
float Q_discharged_mAh_old[2] = {0};
uint32_t battery_power_calculated[2] = {0};

uint8_t counter_last = 0;

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


// Map to store the last publish time for each topic
std::map<String, uint32_t> lastPublishTimes;

template <typename T>
void publishIfChanged(T &currentValue, T newValue, const char *publishValue, const String &topic)
{

    uint32_t currentTime = millis();
    uint32_t lastPublishTimeTopic = lastPublishTimes[topic];

    // Check if the value has changed or MIN_PUB_TIME is greater than 0 and the time has passed since the last publish
    if (currentValue != newValue || (min_pub_time > 0 && (currentTime - lastPublishTimeTopic) >= (static_cast<uint32_t>(min_pub_time) * 1000UL)))
    {
        toMqttQueue(topic.c_str(), publishValue);
        currentValue = newValue;
        lastPublishTimes[topic] = currentTime; // Update the last publish time for the topic
    }
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

DeviceData devicedata;

void readDeviceDataRecord(void *message, const char *devicename)
{

    // Startzeit für die Verarbeitung des Datensatzes
    // uint32_t start_time = millis();
    memcpy(&devicedata, message, 300); // Kopiere 300 Bytes in die Struktur

    String str_base_topic = mqtt_main_topic + devicename + "/device/";

    // Publish the FrameCounter to MQTT
    toMqttQueue(str_base_topic + "read_count", String(devicedata.FrameCounter));

    // Publish Manufacturer Device ID
    toMqttQueue(str_base_topic + "vendor_id", devicedata.ManufacturerDeviceID);

    // Publish Hardware Version and Software Version
    toMqttQueue(str_base_topic + "hw_revision", devicedata.HardwareVersion);
    toMqttQueue(str_base_topic + "sw_version", devicedata.SoftwareVersion);

    // Publish Uptime in seconds and in human-readable format (days, hours, minutes, seconds)
    toMqttQueue(str_base_topic + "uptime", String(devicedata.OddRunTime));
    toMqttQueue(str_base_topic + "uptime_fmt", devicedata.getOddRunTimeStr());

    // Publish Power On Times
    toMqttQueue(str_base_topic + "power_up_times", String(devicedata.PwrOnTimes));

    // Publish Device Name, Passcode, Manufacturing Date, Serial Number, User Data, Setup Passcode and User Data 2
    toMqttQueue(str_base_topic + "device_name", devicedata.DeviceName);
    toMqttQueue(str_base_topic + "device_passwd", devicedata.DevicePasscode);
    toMqttQueue(str_base_topic + "manufacturing_date", devicedata.ManufacturingDate);
    toMqttQueue(str_base_topic + "serial_number", devicedata.SerialNumber);
#ifndef V19
    toMqttQueue(str_base_topic + "passcode", devicedata.Passcode);
#endif
    toMqttQueue(str_base_topic + "user_data", devicedata.UserData);
    toMqttQueue(str_base_topic + "setup_passcode", devicedata.SetupPasscode);
    toMqttQueue(str_base_topic + "user_data2", devicedata.UserData2);

// Publish UART and CAN protocol numbers and enable status
#ifdef PROTOCOL_NUMBERS_AND_ENABLE_STATUS
    toMqttQueue(str_base_topic + "uart1_protocol_number", String(devicedata.UART1MPRTOLNbr));
    toMqttQueue(str_base_topic + "uart1_protocol_txt", uart_protocol_number_str[devicedata.UART1MPRTOLNbr]);
    toMqttQueue(str_base_topic + "uart1_protocol_enable", String(devicedata.UART1MPRTOLEnable));
    toMqttQueue(str_base_topic + "can_protocol_number", String(devicedata.CANMPRTOLNbr));
    toMqttQueue(str_base_topic + "can_protocol_txt", can_protocol_number_str[devicedata.CANMPRTOLNbr]);
    toMqttQueue(str_base_topic + "uart_protocol_enable_0_15", String(devicedata.UARTMPRTOLEnable0_15));
    toMqttQueue(str_base_topic + "uart2_protocol_number", String(devicedata.UART2MPRTOLNbr));
    toMqttQueue(str_base_topic + "uart2_protocol_txt", uart_protocol_number_str[devicedata.UART2MPRTOLNbr]);
    toMqttQueue(str_base_topic + "uart2_protocol_enable", String(devicedata.UART2MPRTOLEnable));
    toMqttQueue(str_base_topic + "uart_protocol_lib_version", String(devicedata.UARTMPTLVer));
#endif

// Publish trigger values for LCD buzzer and dry contacts
#ifdef LCD_AND_DRY_TRIGGER_VALUES
    toMqttQueue(str_base_topic + "lcd_buzzer_trigger", String(devicedata.LCDBuzzerTrigger));
    toMqttQueue(str_base_topic + "lcd_buzzer_trigger_txt", Trigger_values_str[devicedata.LCDBuzzerTrigger]);
    toMqttQueue(str_base_topic + "lcd_buzzer_trigger_value", String(devicedata.LCDBuzzerTriggerVal));
    toMqttQueue(str_base_topic + "lcd_buzzer_release_value", String(devicedata.LCDBuzzerReleaseVal));
    toMqttQueue(str_base_topic + "dry1_trigger", String(devicedata.DRY1Trigger));
    toMqttQueue(str_base_topic + "dry1_trigger_txt", Trigger_values_str[devicedata.DRY1Trigger]);
    toMqttQueue(str_base_topic + "dry1_trigger_value", String(devicedata.DRY1TriggerVal));
    toMqttQueue(str_base_topic + "dry1_release_value", String(devicedata.DRY1ReleaseVal));
    toMqttQueue(str_base_topic + "dry2_trigger", String(devicedata.DRY2Trigger));
    toMqttQueue(str_base_topic + "dry2_trigger_txt", Trigger_values_str[devicedata.DRY2Trigger]);
    toMqttQueue(str_base_topic + "dry2_trigger_value", String(devicedata.DRY2TriggerVal));
    toMqttQueue(str_base_topic + "dry2_release_value", String(devicedata.DRY2ReleaseVal));
    toMqttQueue(str_base_topic + "can_protocol_lib_version", String(devicedata.CANMPTLVer));
#endif

    // Publish RCV Time and RFV Time
    toMqttQueue(str_base_topic + "rcv_time", String((float)devicedata.RCVTime * 0.1, 1)); // Assuming RCVTime is in 0.1h units
    toMqttQueue(str_base_topic + "rfv_time", String((float)devicedata.RFVTime * 0.1, 1)); // Assuming RFVTime is in 0.1h units

    // String finishMsg = getLocalTimeString() + ": Finished parsing Device data, should now be available/updated in MQTT topics." + " (Processing time: " + String(millis() - start_time) + " ms)";
    // DEBUG_PRINTLN(finishMsg);
}

CellData celldata;
CellDataOld cdOld[2]; // Array to store old values for MQTT and INFLUX, used for comparison and change detection

void readCellDataRecord(void *message, const char *devicename)
{
    uint32_t start_time = millis();

    float deltaT = (start_time - lastDataReceivedTime) / 1000.0; // time in seconds
    lastDataReceivedTime = start_time;
    // actual charge current
    int32_t current_raw;
    memcpy(&current_raw, (uint8_t *)message + 158, sizeof(int32_t)); // assuming current is at byte offset 158
    float_t current = current_raw * 0.001;

    // Integration of charge/discharge current
    if (current > 0)
    {
        Q_charged += current * deltaT;
    }
    else
    {
        Q_discharged += (-current) * deltaT;
    }
    // to mAh
    float Q_charged_mAh = Q_charged / 3.6;
    float Q_discharged_mAh = Q_discharged / 3.6;
    char Q_charged_mAh_str[32];
    char Q_discharged_mAh_str[32];
    dtostrf(Q_charged_mAh, 0, 3, Q_charged_mAh_str);
    dtostrf(Q_discharged_mAh, 0, 3, Q_discharged_mAh_str);

    if (!first_run && (publish_delay > 0 && (start_time - lastPublishTimeCellData) < (publish_delay * 1000)))
    {
        // String finishCellMsg = getLocalTimeString() + ": Finished parsing Cell data(short version). Frame: " + String(receivedBytes_cell[pos_of_Counter]) + " (Processing time: " + String(millis() - start_time) + " ms)";
        // DEBUG_PRINTLN(finishCellMsg);
        return;
    }
    first_run = false;
    memcpy(&celldata, message, 300); // Kopiere 300 Bytes in die Struktur
    celldata.prepareOutValues();

    // MQTT-Themenbasis
    String base_data = mqtt_main_topic + devicename + "/data/";
    String base_debug = mqtt_main_topic + devicename + "/debug/";

    // Update the last publish time
    lastPublishTimeCellData = start_time;

    // Publish Frame Counter
    toMqttQueue(base_data + "readcount", String(celldata.FrameCounter));
    // Publish charge and discharge in mAh with change detection with 3 decimal places
    publishIfChanged(Q_charged_mAh_old[MQTT], Q_charged_mAh, Q_charged_mAh_str, base_data + "battery_charged_mAh");
    publishIfChanged(Q_discharged_mAh_old[MQTT], Q_discharged_mAh, Q_discharged_mAh_str, base_data + "battery_discharged_mAh");

    if (debug_flg_full)
    {
        char message_base64[base64::encodeLength(300)];                   // Buffer für die Base64-kodierte Nachricht
        uint8_t *rawDataPtr = (uint8_t *)&celldata;                       // Zeiger auf die Rohdaten der Struktur
        base64::encode((const uint8_t *)rawDataPtr, 300, message_base64); // Base64-kodieren der Rohdaten);
        toMqttQueue(base_debug + "rawdata", message_base64);
        toMqttQueue(base_debug + "enabled", "true");
    }
    else
    {
        if (debug_flg)
        {
            toMqttQueue(base_debug + "rawdata", "not published");
            toMqttQueue(base_debug + "enabled", "false");
        }
    }

    // Cell Voltages
    char topic_buffer[128];
    for (uint8_t i = 0; i < 32; i++)
    {
        snprintf(topic_buffer, sizeof(topic_buffer), "%s%02d", (base_data + "cells/voltage/cell_v_").c_str(), i + 1);
        if (celldata.CellVol[i] != 0)
        {
            publishIfChanged(cdOld[MQTT].CellVol[i], celldata.CellVol[i], celldata.CellVol_fmt[i], topic_buffer);
#ifdef INFLUX_CELLS_VOLTAGE
            publishToInfluxDB("cell_" + String(i + 1), celldata.CellVol[i]);
#endif
        }
    }

    // CellSta as bitmask
    if (debug_flg)
    {
        publishIfChanged(cdOld[MQTT].CellSta, celldata.CellSta, celldata.CellSta_fmt, base_data + "cells_used");
    }

    // Cell Average Voltage
    publishIfChanged(cdOld[MQTT].CellVolAve, celldata.CellVolAve, celldata.CellVolAve_fmt, base_data + "cells/voltage/cell_avg_voltage");

    // Cell Voltage Difference
    publishIfChanged(cdOld[MQTT].CellVdifMax, celldata.CellVdifMax, celldata.CellVdifMax_fmt, base_data + "cells/voltage/cell_diff_voltage");

    // High Voltage Cell
    publishIfChanged(cdOld[MQTT].MaxVolCellNbr, celldata.MaxVolCellNbr, celldata.MaxVolCellNbr_fmt, base_data + "cells/voltage/high_voltage_cell");

    // Low Voltage Cell
    publishIfChanged(cdOld[MQTT].MinVolCellNbr, celldata.MinVolCellNbr, celldata.MinVolCellNbr_fmt, base_data + "cells/voltage/low_voltage_cell");

    // Cell resistances
    for (uint8_t i = 0; i < 32; i++)
    {
        snprintf(topic_buffer, sizeof(topic_buffer), "%s%02d", (base_data + "cells/resistance/cell_r_").c_str(), i + 1);
        if (celldata.CellWireRes[i] != 0)
        {
            publishIfChanged(cdOld[MQTT].CellWireRes[i], celldata.CellWireRes[i], celldata.CellWireRes_fmt[i], topic_buffer);
        }
    }

    // Temp MOSFET
    publishIfChanged(cdOld[MQTT].TempMos, celldata.TempMos, celldata.TempMos_fmt, base_data + "temperatures/temp_mosfet");
#ifdef INFLUX_TEMP_SENSOR_MOSFET
    publishIfChangedInflux(cdOld[INFLUX].TempMos, celldata.TempMos, celldata.TempMos_fmt, "temp_mosfet");
#endif

    // Cell Wire Resistance Status as bitmask
    if (debug_flg)
    {
        publishIfChanged(cdOld[MQTT].CellWireResSta, celldata.CellWireResSta, celldata.CellWireResSta_fmt, base_data + "cell_resistance_alert");
    }

    // Battery Total Voltage
    publishIfChanged(cdOld[MQTT].BatVol, celldata.BatVol, celldata.BatVol_fmt, base_data + "battery_voltage");
#ifdef INFLUX_BATTERY_VOLTAGE
    publishIfChangedInflux(cdOld[INFLUX].BatVol, celldata.BatVol, celldata.BatVol_fmt, "battery_voltage");
#endif

    // Battery Power
    publishIfChanged(cdOld[MQTT].BatWatt, celldata.BatWatt, celldata.BatWatt_fmt, base_data + "battery_power");
#ifdef INFLUX_BATTERY_POWER
    publishIfChangedInflux(cdOld[INFLUX].BatWatt, celldata.BatWatt, celldata.BatWatt_fmt, "battery_power");
#endif

    // Battery Current
    publishIfChanged(cdOld[MQTT].BatCurrent, celldata.BatCurrent, celldata.BatCurrent_fmt, base_data + "battery_current");
#ifdef INFLUX_BATTERY_CURRENT
    publishIfChangedInflux(cdOld[INFLUX].BatCurrent, celldata.BatCurrent, celldata.BatCurrent_fmt, "battery_current");
#endif

    // Signed Battery Power for MQTT and InfluxDB
    // this is to have the correct sign for the power (discharging "-" or charging)
    char signed_BatWatt_str[32];
    dtostrf((celldata.BatCurrent < 0) ? (float)(0 - celldata.BatWatt * 0.001) : (float)celldata.BatWatt * 0.001, 0, 3, signed_BatWatt_str); // to have the correct sign for the power (discharging "-" or charging)
    publishIfChanged(battery_power_calculated[MQTT], celldata.BatWatt, signed_BatWatt_str, base_data + "battery_power_calculated");
#ifdef INFLUX_BATTERY_POWER
    publishIfChangedInflux(battery_power_calculated[INFLUX], celldata.BatWatt, signed_BatWatt_str, "battery_power");
#endif

    // Batterie Temp Sensor 1
    publishIfChanged(cdOld[MQTT].TempBat1, celldata.TempBat1, celldata.TempBat1_fmt, base_data + "temperatures/temp_sensor1");
#ifdef INFLUX_TEMP_SENSOR_1
    publishIfChangedInflux(cdOld[INFLUX].TempBat1, celldata.TempBat1, celldata.TempBat1_fmt, "temp_sensor1");
#endif

    // Batterie Temp Sensor 2
    publishIfChanged(cdOld[MQTT].TempBat2, celldata.TempBat2, celldata.TempBat2_fmt, base_data + "temperatures/temp_sensor2");
#ifdef INFLUX_TEMP_SENSOR_2
    publishIfChangedInflux(cdOld[INFLUX].TempBat2, celldata.TempBat2, celldata.TempBat2_fmt, "temp_sensor2");
#endif

    // Alarms as raw dezimal, bitmask and resolved alarms according to BMS RS485 ModbusV1.1 2024.02 Page 10
    publishIfChanged(cdOld[MQTT].AlarmBitMask, celldata.AlarmBitMask, celldata.Alarm_raw_fmt, base_data + "alarms/alarm_raw");

    if (debug_flg)
    {
        publishIfChanged(cdOld[MQTT].AlarmBitMask, celldata.AlarmBitMask, celldata.AlarmBitMask_fmt, base_data + "alarms/alarms_mask");
    }
    for (int i = 0; i < 24; ++i)
    {
        publishIfChanged(cdOld[MQTT].AlarmBitMask, celldata.AlarmBitMask, celldata.AlarmsValue_fmt[i], base_data + "alarms/" + String(celldata.AlarmsTopics[i]));
    }

    // Balance Current
    publishIfChanged(cdOld[MQTT].BalanCurrent, celldata.BalanCurrent, celldata.BalanCurrent_fmt, base_data + "balance_current");

    // Balance Status
    publishIfChanged(cdOld[MQTT].BalanSta, celldata.BalanSta, celldata.BalanSta_fmt, base_data + "balance_status");

    // State of Charge
    publishIfChanged(cdOld[MQTT].SOCStateOfcharge, celldata.SOCStateOfcharge, celldata.SOCStateOfcharge_fmt, base_data + "battery_soc");
#ifdef INFLUX_BATTERY_SOC
    publishIfChangedInflux(cdOld[INFLUX].SOCStateOfcharge, celldata.SOCStateOfcharge, celldata.SOCStateOfcharge_fmt, "battery_soc");
#endif

    // Battery Capacity Remaining
    publishIfChanged(cdOld[MQTT].SOCCapRemain, celldata.SOCCapRemain, celldata.SOCCapRemain_fmt, base_data + "battery_capacity_remaining");

    // Battery Capacity Total
    publishIfChanged(cdOld[MQTT].SOCFullChargeCap, celldata.SOCFullChargeCap, celldata.SOCFullChargeCap_fmt, base_data + "battery_capacity_total");

    // Battery Cycle Count
    publishIfChanged(cdOld[MQTT].SOCCycleCount, celldata.SOCCycleCount, celldata.SOCCycleCount_fmt, base_data + "battery_cycle_count");

    // Battery Cycle Capacity Total
    publishIfChanged(cdOld[MQTT].SOCCycleCap, celldata.SOCCycleCap, celldata.SOCCycleCap_fmt, base_data + "battery_cycle_capacity_total");

    // Battery SOH
    publishIfChanged(cdOld[MQTT].SOCSOH, celldata.SOCSOH, celldata.SOCSOH_fmt, base_data + "battery_soh");

    // Battery Precharge Status
    publishIfChanged(cdOld[MQTT].Precharge, celldata.Precharge, celldata.Precharge_fmt, base_data + "battery_precharge_status");

    // Battery User Alarm 1
    publishIfChanged(cdOld[MQTT].UserAlarm, celldata.UserAlarm, celldata.UserAlarm_fmt, base_data + "battery_user_alarm1");

    // Battery Total Runtime in seconds and in human-readable format
    publishIfChanged(cdOld[MQTT].RunTime, celldata.RunTime, celldata.RunTime_fmt, base_data + "battery_total_runtime_sec");
    publishIfChanged(cdOld[MQTT].RunTime2, celldata.RunTime, celldata.RunTime_fmt_dhms, base_data + "battery_total_runtime_fmt");

    // Charging MOSFET Status
    publishIfChanged(cdOld[MQTT].Charge, celldata.Charge, celldata.Charge_fmt, base_data + "charging_mosfet_status");

    // Discharging MOSFET Status
    publishIfChanged(cdOld[MQTT].Discharge, celldata.Discharge, celldata.Discharge_fmt, base_data + "discharging_mosfet_status");

    // Battery User Alarm 2
    publishIfChanged(cdOld[MQTT].UserAlarm2, celldata.UserAlarm2, celldata.UserAlarm2_fmt, base_data + "battery_user_alarm2");

    // Time Discharge Over Current Protection Release (timeDcOCPR)
    publishIfChanged(cdOld[MQTT].TimeDcOCPR, celldata.TimeDcOCPR, celldata.TimeDcOCPR_fmt, base_data + "timeDcOCPR");

    // Time Discharge Short Circuit Protection Release (timeDcSCPR)
    publishIfChanged(cdOld[MQTT].TimeDcSCPR, celldata.TimeDcSCPR, celldata.TimeDcSCPR_fmt, base_data + "timeDcSCPR");

    // Time Charge Over Current Protection Release (timeCOCPR)
    publishIfChanged(cdOld[MQTT].TimeCOCPR, celldata.TimeCOCPR, celldata.TimeCOCPR_fmt, base_data + "timeCOCPR");

    // Time Charge Short Circuit Protection Release (timeCSCPR)
    publishIfChanged(cdOld[MQTT].TimeCSCPR, celldata.TimeCSCPR, celldata.TimeCSCPR_fmt, base_data + "timeCSCPR");

    // Time Under Voltage Protection Release (timeUVPR)
    publishIfChanged(cdOld[MQTT].TimeUVPR, celldata.TimeUVPR, celldata.TimeUVPR_fmt, base_data + "timeUVPR");

    // Time Over Voltage Protection Release (timeOVPR)
    publishIfChanged(cdOld[MQTT].TimeOVPR, celldata.TimeOVPR, celldata.TimeOVPR_fmt, base_data + "timeOVPR");

    // Temperature Sensor Absent Mask as raw dezimal, bitmask and resolved according to BMS RS485 ModbusV1.1 2024.02 Page 11
    publishIfChanged(cdOld[MQTT].TempSensorAbsentMask, celldata.TempSensorAbsentMask, celldata.TempSensorAbsent_fmt, base_data + "temperatures/temp_sensor_absent");
    if (debug_flg)
    {
        publishIfChanged(cdOld[MQTT].TempSensorAbsentMask, celldata.TempSensorAbsentMask, celldata.TempSensorAbsentMask_fmt, base_data + "temperatures/temp_sensor_absent_mask");
    }
    for (int i = 0; i < 6; ++i)
    {
        publishIfChanged(cdOld[MQTT].TempSensorAbsentMask, celldata.TempSensorAbsentMask, celldata.TempSensAbsentValues_fmt[i], base_data + "temperatures/" + String(celldata.TempSensorsAbsentTopics[i]));
    }

    // Battery Heating
    publishIfChanged(cdOld[MQTT].Heating, celldata.Heating, celldata.Heating_fmt, base_data + "temperatures/battery_heating");

    // Time Emergency
    publishIfChanged(cdOld[MQTT].TimeEmergency, celldata.TimeEmergency, celldata.TimeEmergency_fmt, base_data + "time_emergency");

    // Heating Current
    publishIfChanged(cdOld[MQTT].HeatCurrent, celldata.HeatCurrent, celldata.HeatCurrent_fmt, base_data + "heat_current");

    // System run ticks, unit: 0.1s, range: 0~4294967295
    publishIfChanged(cdOld[MQTT].SysRunTicks, celldata.SysRunTicks, celldata.SysRunTicks_fmt, base_data + "sys_run_ticks");

    // Batterie Temp Sensor3
    publishIfChanged(cdOld[MQTT].TempBat3, celldata.TempBat3, celldata.TempBat3_fmt, base_data + "temperatures/temp_sensor3");
#ifdef INFLUX_TEMP_SENSOR_3
    publishIfChangedInflux(cdOld[INFLUX].TempBat3, celldata.TempBat3, celldata.TempBat3_fmt, "temp_sensor3");
#endif

    // Batterie Temp Sensor4
    if (celldata.TempBat4 > 1200 || celldata.TempBat4 < -400)
    { // if the value is out of range, it could be that the sensor is not present or not working, so we publish a separate topic for this
        publishIfChanged(cdOld[MQTT].TempBat4, celldata.TempBat4, "out of range", base_data + "temperatures/temp_sensor4");
    }
    else
    {
        publishIfChanged(cdOld[MQTT].TempBat4, celldata.TempBat4, celldata.TempBat4_fmt, base_data + "temperatures/temp_sensor4");
#ifdef INFLUX_TEMP_SENSOR_4
        publishIfChangedInflux(cdOld[INFLUX].TempBat4, celldata.TempBat4, celldata.TempBat4_fmt, "temp_sensor4");
#endif
    }

    // Batterie Temp Sensor5
    if (celldata.TempBat5 > 1200 || celldata.TempBat5 < -400)
    { // if the value is out of range, it could be that the sensor is not present or not working, so we publish a separate topic for this
        publishIfChanged(cdOld[MQTT].TempBat5, celldata.TempBat5, "out of range", base_data + "temperatures/temp_sensor5");
    }
    else
    {
        publishIfChanged(cdOld[MQTT].TempBat5, celldata.TempBat5, celldata.TempBat5_fmt, base_data + "temperatures/temp_sensor5");
#ifdef INFLUX_TEMP_SENSOR_5
        publishIfChangedInflux(cdOld[INFLUX].TempBat5, celldata.TempBat5, celldata.TempBat5_fmt, "temp_sensor5");
#endif
    }

    // RTC Ticks
    publishIfChanged(cdOld[MQTT].RTCTicks, celldata.RTCTicks, celldata.RTCTicksToSeconds_fmt, base_data + "rtc_ticks");

    if (debug_flg)
    {
        uint32_t MinFreeHeap = ESP.getMinFreeHeap();
        String finishCellMsg = getLocalTimeString() + ": Finished parsing Cell data. Frame: " + String(celldata.FrameCounter) + " (Processing time: " + String(millis() - start_time) + " ms, Min Free Heap: " + String(MinFreeHeap) + " bytes)";
        DEBUG_PRINTLN(finishCellMsg);
    }
}

ConfigData configdata;

void readConfigDataRecord(void *message, const char *devicename)
{
    // Startzeit für die Verarbeitung des Datensatzes
    // uint32_t start_time = millis();

    // Kopiere die empfangenen Bytes in die DeviceData-Struktur
    memcpy(&configdata, message, 300); // Kopiere 300 Bytes in die Struktur
    configdata.update_switches();

    // MQTT-Themenbasis
    String str_base_topic = mqtt_main_topic + devicename + "/config/";

    // Veröffentliche die Konfigurationsdaten auf MQTT
    // VolSmartSleep
    toMqttQueue(str_base_topic + "vol_smart_sleep", String((float)configdata.VolSmartSleep * 0.001, 3));
    // VolCellUV
    toMqttQueue(str_base_topic + "vol_cell_uv", String((float)configdata.VolCellUV * 0.001, 3));
    // VolCellUVPR
    toMqttQueue(str_base_topic + "vol_cell_uvpr", String((float)configdata.VolCellUVPR * 0.001, 3));
    // VolCellOV
    toMqttQueue(str_base_topic + "vol_cell_ov", String((float)configdata.VolCellOV * 0.001, 3));
    // VolCellOVPR
    toMqttQueue(str_base_topic + "vol_cell_ovpr", String((float)configdata.VolCellOVPR * 0.001, 3));
    // VolBalanTrig
    toMqttQueue(str_base_topic + "vol_balan_trig", String((float)configdata.VolBalanTrig * 0.001, 3));
    // VolSOC100%
    toMqttQueue(str_base_topic + "vol_100_percent", String((float)configdata.VolSOC100percent * 0.001, 3));
    // VolSOC0%
    toMqttQueue(str_base_topic + "vol_0_percent", String((float)configdata.VolSOC0percent * 0.001, 3));
    // VolCellRCV
    toMqttQueue(str_base_topic + "vol_cell_rcv", String((float)configdata.VolCellRCV * 0.001, 3));
    // VolCellRFV
    toMqttQueue(str_base_topic + "vol_cell_rfv", String((float)configdata.VolCellRFV * 0.001, 3));
    // VolSysPwrOff
    toMqttQueue(str_base_topic + "vol_sys_pwr_off", String((float)configdata.VolSysPwrOff * 0.001, 3));
    // CurBatCOC
    toMqttQueue(str_base_topic + "cur_bat_coc", String((float)configdata.CurBatCOC * 0.001, 3));
    // TIMBatCOCPDly
    toMqttQueue(str_base_topic + "time_bat_cocp_delay", String(configdata.TimBatCOCPDly));
    // TIMBatCOCPRDDly
    toMqttQueue(str_base_topic + "time_bat_cocprd_delay", String(configdata.TimBatCOCPRDly));
    // CurBatDcOC
    toMqttQueue(str_base_topic + "cur_bat_dc_oc", String((float)configdata.CurBatDcOC * 0.001, 3));
    // TIMBatDcOCPDly
    toMqttQueue(str_base_topic + "time_bat_dc_ocp_delay", String(configdata.TimBatDcOCPDly));
    // TIMBatDcOPRDDly
    toMqttQueue(str_base_topic + "time_bat_dc_oprd_delay", String(configdata.TimBatDcOCPRDly));
    // TIMBatSCPRDly
    toMqttQueue(str_base_topic + "time_bat_scprd_delay", String(configdata.TimBatSCPRDly));
    // CurBalanMax
    toMqttQueue(str_base_topic + "cur_balance_max", String((float)configdata.CurBalanMax * 0.001, 3));
    // TMPBatCOT
    toMqttQueue(str_base_topic + "tmp_bat_cot", String((float)configdata.TmpBatCOT * 0.1, 1));
    // TMPBatCOTPR
    toMqttQueue(str_base_topic + "tmp_bat_cotpr", String((float)configdata.TmpBatCOTPR * 0.1, 1));
    // TMPBatDcOT
    toMqttQueue(str_base_topic + "tmp_bat_dc_ot", String((float)configdata.TmpBatDcOT * 0.1, 1));
    // TMPBatDcOTPR
    toMqttQueue(str_base_topic + "tmp_bat_dc_otpr", String((float)configdata.TmpBatDcOTPR * 0.1, 1));
    // TMPBatCUT
    toMqttQueue(str_base_topic + "tmp_bat_cut", String((float)configdata.TmpBatCUT * 0.1, 1));
    // TMPBatCUTPR
    toMqttQueue(str_base_topic + "tmp_bat_cutpr", String((float)configdata.TmpBatCUTPR * 0.1, 1));
    // TMPMosOT
    toMqttQueue(str_base_topic + "tmp_mos_ot", String((float)configdata.TmpMosOT * 0.1, 1));
    // TMPMosOTPR
    toMqttQueue(str_base_topic + "tmp_mos_otpr", String((float)configdata.TmpMosOTPR * 0.1, 1));
    // CellCount
    toMqttQueue(str_base_topic + "cell_count", String(configdata.CellCount[0]));
    // BatChargeEN
    toMqttQueue(str_base_topic + "switches/bat_charge_enabled", configdata.BatChargeEN_fmt);
    // BatDisChargeEN
    toMqttQueue(str_base_topic + "switches/bat_discharge_enabled", configdata.BatDisChargeEN_fmt);
    // BalanEN
    toMqttQueue(str_base_topic + "switches/balancing_enabled", configdata.BalanEN_fmt);
    // CapBatCell
    toMqttQueue(str_base_topic + "cap_bat_cell", String((float)configdata.CapBatCell * 0.001, 3));
    // SCPDelay
    toMqttQueue(str_base_topic + "scp_delay", String(configdata.ScpDelay));
    // VolStartBalan
    toMqttQueue(str_base_topic + "vol_start_balance", String((float)configdata.VolBalanTrig * 0.001, 3));
    // DevAddr
    toMqttQueue(str_base_topic + "dev_address", String(configdata.DevAddr));
    // TIMProdischarge
    toMqttQueue(str_base_topic + "tim_pro_discharge", String(configdata.TimProdischarge));
    // HeatEN
    toMqttQueue(str_base_topic + "switches/heating_enabled", configdata.HeatEN);
    // Disable temp-sensor
    toMqttQueue(str_base_topic + "switches/temp_sensor_disabled", configdata.DisableTempSensor);
    // GPS Heartbeat
    toMqttQueue(str_base_topic + "switches/gps_heartbeat", configdata.GPSHeartbeat);
    // Port Switch
    toMqttQueue(str_base_topic + "switches/port_switch", configdata.PortSwitch);
    // LCD Always On
    toMqttQueue(str_base_topic + "switches/lcd_always_on", configdata.LCDAlwaysOn);
    // Special Charger
    toMqttQueue(str_base_topic + "switches/special_charger", configdata.SpecialCharger);
    // SmartSleep
    toMqttQueue(str_base_topic + "switches/smart_sleep", configdata.SmartSleep);
    // DisablePCLModule
    toMqttQueue(str_base_topic + "switches/disable_pcl_module", configdata.DisablePCLModule);
    // TimedStoredData
    toMqttQueue(str_base_topic + "switches/timed_stored_data", configdata.TimedStoredData);
    // ChargingFloatMode
    toMqttQueue(str_base_topic + "switches/charging_float_mode", configdata.ChargingFloatMode);
    // TMPHeatingStart
    toMqttQueue(str_base_topic + "tmp_heating_start", String(configdata.TmpHeatingStart));
    // TMPHeatingStop
    toMqttQueue(str_base_topic + "tmp_heating_stop", String(configdata.TmpHeatingStop));
    // TIMSmartSleep
    toMqttQueue(str_base_topic + "time_smart_sleep", String(configdata.TimSmartSleep));

    // String finishConfigMsg = getLocalTimeString() + ": Finished parsing Config data." + " (Processing time: " + String(millis() - start_time) + " ms)";
    // DEBUG_PRINTLN(finishConfigMsg);
}
