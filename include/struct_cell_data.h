#ifndef STRUCT_CELL_DATA_H
#define STRUCT_CELL_DATA_H
#include <Arduino.h>
#include "arduino_base64.hpp"
#include <bitset>

// Struktur für die Zellendaten (LiveDaten) (CellData; FrameType 0x02)
struct CellData
{                               //  bytes Description                   Unit      Multiplier or format
  uint8_t Header[4];            //	4		  Header                        #
  uint8_t FrameType;            //	1		  Frame type                    #
  uint8_t FrameCounter;         //	1		  Frame counter                 #
  uint16_t CellVol[32];         //	64	  32 mal Cell voltages          mV        0.001 multiplier
  uint32_t CellSta;             //	4		  Enabled cells bitmask         BITMASK   A value of 1 for BIT[n] indicates that the battery exists.
  uint16_t CellVolAve;          //	2		  Average cell voltage          mV        0.001 multiplier
  uint16_t CellVdifMax;         //	2		  Cell voltage difference       mV        0.001 multiplier or 1 mV depending on DIFFV_DIVIDER
  uint8_t MaxVolCellNbr;        //	1		  MaxVolCellNbr                 #
  uint8_t MinVolCellNbr;        //	1		  MinVolCellNbr                 #
  uint16_t CellWireRes[32];     //	64	  32 mal Cell wire resistance   mΩ
  int16_t TempMos;              //	2		  TempMos                       0.1℃     0.1 multiplier, can be negative
  uint32_t CellWireResSta;      //	4		  CellWireResSta                BITMASK   A value of 1 for BIT[n] indicates that the equalization line is alarming.
  uint32_t BatVol;              //	4		  BatVol                        mV        0.001 multiplier
  uint32_t BatWatt;             //	4		  BatWatt                       mW        0.001 multiplier
  int32_t BatCurrent;           //	4		  BatCurrent                    mA        0.001 multiplier, can be negative
  int16_t TempBat1;             //	2		  TempBat 1                     0.1℃     0.1 multiplier, can be negative
  int16_t TempBat2;             //	2		  TempBat 2                     0.1℃     0.1 multipler, can be negative
  uint32_t AlarmBitMask;        //  4     AlarmBitMask                  BITMASK   BIT0: AlarmWireRes (1: Fault; 0: Normal),
                                //                                                BIT1: AlarmMosOTP (1: Fault; 0: Normal),
                                //                                                BIT2: AlarmCellQuantity (1: Fault; 0: Normal),
                                //                                                BIT3: AlarmCurSensorErr (1: Fault; 0: Normal),
                                //                                                BIT4: AlarmCellOVP (1: Fault; 0: Normal),
                                //                                                BIT5: AlarmBatOVP (1: Fault; 0: Normal),
                                //                                                BIT6: AlarmChOCP (1: Fault; 0: Normal),
                                //                                                BIT7: AlarmChSCP (1: Fault; 0: Normal),
                                //                                                BIT8: AlarmChOTP (1: Fault; 0: Normal),
                                //                                                BIT9: AlarmChUTP (1: Fault; 0: Normal),
                                //                                                BIT10: AlarmCPUAuxCommuErr (1: Fault; 0: Normal)
                                //                                                BIT11: AlarmCellUVP (1: Fault; 0: Normal)
                                //                                                BIT12: AlarmBatUVP (1: Fault; 0: Normal)
                                //                                                BIT13: AlarmDchOCP (1: Fault; 0: Normal)
                                //                                                BIT14: AlarmDchSCP (1: Fault; 0: Normal)
                                //                                                BIT15: AlarmDchOTP (1: Fault; 0: Normal)
                                //                                                BIT16: AlarmChargeMOS (1: Fault; 0: Normal)
                                //                                                BIT17: AlarmDischargeMOS (1: Fault; 0: Normal)
                                //                                                BIT18: GPSDisconneted (1: Fault; 0: Normal)
                                //                                                BIT19: Modify PWD. in time (1: Fault; 0: Normal)
                                //                                                BIT20: Discharge On Failed (1: Fault; 0: Normal)
                                //                                                BIT21: Battery Over Temp Alarm (1: Fault; 0: Normal)
                                //                                                BIT22: Temperature sensor anomaly (1: Fault; 0: Normal)
                                //                                                BIT23: PLCModule anomaly (1: Fault; 0: Normal)
  int16_t BalanCurrent;         //	2		  BalanCurrent                  mA        0.001 multiplier, can be negative
  uint8_t BalanSta;             //	1		  BalanSta                      #         2: Discharge; 1: Charge; 0: Off
  uint8_t SOCStateOfcharge;     //  1     SOCStateOfcharge              %         1 multiplier
  int32_t SOCCapRemain;         //  4     SOCCapRemain                  mAH       0.001 multiplier, can be negative
  uint32_t SOCFullChargeCap;    //  4     SOCFullChargeCap              mAH	      0.001 multiplier
  uint32_t SOCCycleCount;       //  4     SOCCycleCount                 #
  uint32_t SOCCycleCap;         //  4     SOCCycleCap                   mAH       0.001 multiplier
  uint8_t SOCSOH;               //  1     SOCSOH                        %
  uint8_t Precharge;            //  1     Precharge                     #         1: On; 0: Off
  uint16_t UserAlarm;           //  2     UserAlarm                     #
  uint32_t RunTime;             //  4     RunTime                       s
  uint8_t Charge;               //  1     Charge                        #         1: On; 0: Off
  uint8_t Discharge;            //  1     Discharge                     #         1: On; 0: Off
  uint16_t UserAlarm2;          //  2     UserAlarm2                    #
  uint16_t TimeDcOCPR;          //  2     TimeDcOCPR                    s
  uint16_t TimeDcSCPR;          //  2     TimeDcSCPR                    s
  uint16_t TimeCOCPR;           //  2     TimeCOCPR                     s
  uint16_t TimeCSCPR;           //  2     TimeCSCPR                     s
  uint16_t TimeUVPR;            //  2     TimeUVPR                      s
  uint16_t TimeOVPR;            //  2     TimeOVPR                      s
  uint8_t TempSensorAbsentMask; //  1     TempSensorAbsentMask          BITMASK   BIT0: MOS TempSensorAbsent (1: Normal; 0: Missing)
                                //                                                BIT1: BATTempSensor1Absent (1: Normal; 0: Missing)
                                //                                                BIT2: BATTempSensor2Absent (1: Normal; 0: Missing)
                                //                                                BIT3: BATTempSensor3Absent (1: Normal; 0: Missing)
                                //                                                BIT4: BATTempSensor4Absent (1: Normal; 0: Missing)
                                //                                                BIT5: BATTempSensor5Absent (1: Normal; 0: Missing)
  uint8_t Heating;              //  1     Heating                       #         1: On; 0: Off
  uint16_t unknown1;            //  2		  Reserved
  uint16_t TimeEmergency;       //  2     TimeEmergency                 s
  uint16_t BatDisCurCorrect;    //  2     BatDisCurCorrect              #
  uint16_t VolChargCur;         //  2     VolChargCur                   mV        0.001 multiplier
  uint16_t VolDischargCur;      //  2     VolDischargCur                mV        0.001 multiplier
  float BatVolCorrect;          //  4     BatVolCorrect                 #
  uint8_t unknown2[4];          //  4		  Reserved
  uint16_t BatVol2;             //  2     BatVol                        mV        0.01 multiplier
  int16_t HeatCurrent;          //  2     HeatCurrent                   mA        0.001 multiplier
  uint8_t unknown3[7];          //  7		  Reserved
  uint8_t ChargerPlugged;       //  1     ChargerPlugged                #         1: Inserted; 0: Not inserted
  uint32_t SysRunTicks;         //  4     SysRunTicks                   0.1S      0.1 multiplier
  uint8_t unknown4[4];          //  4		  Reserved
  int16_t TempBat3;             //  2     TempBat 3                     0.1℃	   0.1 multiplier, can be negative
  int16_t TempBat4;             //  2     TempBat 4                     0.1℃	   0.1 multiplier, can be negative
  int16_t TempBat5;             //  2     TempBat 5                     0.1℃	   0.1 multiplier, can be negative
  uint8_t unknown5[2];          //  2		  Reserved
  uint32_t RTCTicks;            //  4     RTCTicks		                  #         The countdown begins on January 1, 2020. RTC ticks, 1 tick = 1/32768 second, will overflow after around 136 years
  uint8_t unknown6[4];          //  4		  Reserved
  uint32_t TimeEnterSleep;      //  4     TimeEnterSleep                s
  uint8_t PCLModuleSta;         //  1     PCLModuleSta                  #         1: On; 0: Off
  uint8_t unknown7[24];         //  24		Reserved
  uint8_t Checksum;             //  1     CRC Checksum                  #
  // Total: 300 bytes
  // Hilfsvariablen für die Ausgabe zu MQTT und InfluxDB
  char CellVol_fmt[32][8];     // Hilfsvariable für die Formatierung der CellVoltages mit 3 Dezimalstellen eg. 3.456 V
  char CellSta_fmt[33];        // Hilfsvariable für die Interpretation des CellSta Bitmasks (32 Zellen + Nullterminator) 00000000 00000000 00000000 00000000
  char CellVolAve_fmt[8];      // Hilfsvariable für die Formatierung der CellVolAve mit 3 Dezimalstellen eg. 3.456 V
  char CellVdifMax_fmt[8];     // Hilfsvariable für die Formatierung der CellVdifMax mit 3 Dezimalstellen eg. 0.123 V or 123 mV depending on DIFFV_DIVIDER
  char MaxVolCellNbr_fmt[3];   // Hilfsvariable für die Formatierung des MaxVolCellNbr eg. 12
  char MinVolCellNbr_fmt[3];   // Hilfsvariable für die Formatierung des MinVolCellNbr eg. 5
  char CellWireRes_fmt[32][8]; // Hilfsvariable für die Formatierung der CellWireRes mit 3 Dezimalstellen eg. 0.123 Ω
  char TempMos_fmt[8];         // Hilfsvariable für die Formatierung der TempMos mit 1 Dezimalstelle eg. 25.3 ℃
  char CellWireResSta_fmt[33]; // Hilfsvariable für die Interpretation des CellWireResSta Bitmasks (32 Zellen + Nullterminator) 00000000 00000000 00000000 00000000
  char BatVol_fmt[8];          // Hilfsvariable für die Formatierung der BatVol mit 3 Dezimalstellen eg. 48.123 V
  char BatWatt_fmt[10];        // Hilfsvariable für die Formatierung der BatWatt mit 3 Dezimalstellen eg. 1234.567 W
  char BatCurrent_fmt[10];     // Hilfsvariable für die Formatierung der BatCurrent mit 3 Dezimalstellen eg. -12.345 A
  char TempBat1_fmt[8];        // Hilfsvariable für die Formatierung der TempBat1 mit 1 Dezimalstelle eg. 25.3 ℃
  char TempBat2_fmt[8];        // Hilfsvariable für die Formatierung der TempBat2 mit 1 Dezimalstelle eg. 25.3 ℃
  char Alarm_raw_fmt[11];      // Hilfsvariable für die Formatierung des Alarm_raw als Dezimalzahl eg. 1234567890
  char AlarmBitMask_fmt[33];   // Hilfsvariable für die Interpretation des AlarmBitMask Bitmasks (24 Alarme + Nullterminator) 00000000 00000000 00000000 00000000
  char AlarmsValue_fmt[24][8]; // Hilfsvariable für die Interpretation der einzelnen Alarme 1: Fault; 0: Normal for every Alarm
  const char *AlarmsTopics[24] = {"AlarmWireRes", "AlarmMosOTP", "AlarmCellQuantity", "AlarmCurSensorErr", "AlarmCellOVP", "AlarmBatOVP", "AlarmChOCP", "AlarmChSCP",
                                  "AlarmChOTP", "AlarmChUTP", "AlarmCPUAuxCommuErr", "AlarmCellUVP", "AlarmBatUVP", "AlarmDchOCP", "AlarmDchSCP", "AlarmDchOTP",
                                  "AlarmChargeMOS", "AlarmDischargeMOS", "AlarmGPSDisconneted", "AlarmModifyPWD_in_time", "AlarmDischargeOnFailed", "BatteryOverTempAlarm", "TemperatureSensorAnomaly", "AlarmPLCModuleAnomaly"}; // Hilfsvariable für die Bereitstellung der MQTT Topics für die einzelnen Alarme
  char BalanCurrent_fmt[8];                                                                                                                                                                                                       // Hilfsvariable für die Formatierung der BalanCurrent mit 3 Dezimalstellen eg. -12.345 A
  char BalanSta_fmt[10];                                                                                                                                                                                                          // Hilfsvariable für die Interpretation des BalanSta (2: Discharge; 1: Charge; 0: Off)
  char SOCStateOfcharge_fmt[4];                                                                                                                                                                                                   // Hilfsvariable für die Formatierung der SOCStateOfcharge eg. 85 %
  char SOCCapRemain_fmt[10];                                                                                                                                                                                                      // Hilfsvariable für die Formatierung der SOCCapRemain mit 3 Dezimalstellen eg. 1.234 Ah
  char SOCFullChargeCap_fmt[10];                                                                                                                                                                                                  // Hilfsvariable für die Formatierung der SOCFullChargeCap mit 3 Dezimalstellen eg. 12.345 Ah
  char SOCCycleCount_fmt[10];                                                                                                                                                                                                     // Hilfsvariable für die Formatierung der SOCCycleCount eg. 1234
  char SOCCycleCap_fmt[10];                                                                                                                                                                                                       // Hilfsvariable für die Formatierung der SOCCycleCap mit 3 Dezimalstellen eg. 1.234 Ah
  char SOCSOH_fmt[4];                                                                                                                                                                                                             // Hilfsvariable für die Formatierung derSOCSOH eg. 85 %
  char Precharge_fmt[4];                                                                                                                                                                                                          // Hilfsvariable für die Interpretation des Precharge (1: On; 0: Off)
  char UserAlarm_fmt[10];                                                                                                                                                                                                         // Hilfsvariable für die Formatierung des UserAlarm eg. 1234
  char RunTime_fmt[10];                                                                                                                                                                                                           // Hilfsvariable für die Formatierung des RunTime eg. 123456 s
  char RunTime_fmt_dhms[20];                                                                                                                                                                                                      // Hilfsvariable für die Formatierung des RunTime im Tage Stunden:Minuten:Sekunden Format eg. 23 days 14:17:36
  char Charge_fmt[4];                                                                                                                                                                                                             // Hilfsvariable für die Interpretation des Charge (1: On; 0: Off)
  char Discharge_fmt[4];                                                                                                                                                                                                          // Hilfsvariable für die Interpretation des Discharge (1: On; 0: Off)
  char UserAlarm2_fmt[10];                                                                                                                                                                                                        // Hilfsvariable für die Formatierung des UserAlarm2 eg. 1234
  char TimeDcOCPR_fmt[8];                                                                                                                                                                                                         // Hilfsvariable für die Formatierung des TimeDcOCPR eg. 1234 s
  char TimeDcSCPR_fmt[8];                                                                                                                                                                                                         // Hilfsvariable für die Formatierung des TimeDcSCPR eg. 1234 s
  char TimeCOCPR_fmt[8];                                                                                                                                                                                                          // Hilfsvariable für die Formatierung des TimeCOCPR eg. 1234 s
  char TimeCSCPR_fmt[8];                                                                                                                                                                                                          // Hilfsvariable für die Formatierung des TimeCSCPR eg. 1234 s
  char TimeUVPR_fmt[8];                                                                                                                                                                                                           // Hilfsvariable für die Formatierung des TimeUVPR eg. 1234 s
  char TimeOVPR_fmt[8];                                                                                                                                                                                                           // Hilfsvariable für die Formatierung desTimeOVPR eg. 1234 s
  char TempSensorAbsentMask_fmt[9];                                                                                                                                                                                               // Hilfsvariable für die Formatierung des TempSensorAbsentMask als Bitmask eg. 00000000
  char TempSensorAbsent_fmt[4];                                                                                                                                                                                                   // Hilfsvariable für die Formatierung des TempSensorAbsentMask als dezimal Zahl eg. 123
  char TempSensAbsentValues_fmt[6][8];                                                                                                                                                                                            // Hilfsvariable für die Interpretation der einzelnen TempSensorAbsent Werte 1: Normal; 0: Missing for every Temp Sensor
  const char *TempSensorsAbsentTopics[6] = {"MOS_TempSensorAbsent", "BATTempSensor1Absent", "BATTempSensor2Absent",
                                            "BATTempSensor3Absent", "BATTempSensor4Absent", "BATTempSensor5Absent"}; // Hilfsvariable für die Bereitstellung der MQTT Topics für die einzelnen Temp Sensor Absent Werte
  char Heating_fmt[4];                                                                                               // Hilfsvariable für die Interpretation des Heating (1: On; 0: Off)
  char TimeEmergency_fmt[10];                                                                                        // Hilfsvariable für die Formatierung des TimeEmergency eg. 1234 s
  char BatDisCurCorrect_fmt[10];                                                                                     // Hilfsvariable für die Formatierung des BatDisCurCorrect eg. 1.234
  char VolChargCur_fmt[10];                                                                                          // Hilfsvariable für die Formatierung des VolChargCur eg. 1.234 V
  char VolDischargCur_fmt[10];                                                                                       // Hilfsvariable für die Formatierung des VolDischargCur eg. 1.234 V
  char BatVolCorrect_fmt[10];                                                                                        // Hilfsvariable für die Formatierung des BatVolCorrect mit 3 Dezimalstellen eg. 1.234 V
  char BatVol2_fmt[10];                                                                                              // Hilfsvariable für die Formatierung des BatVol2 mit 2 Dezimalstellen eg. 48.12 V
  char HeatCurrent_fmt[10];                                                                                          // Hilfsvariable für die Formatierung des HeatCurrent mit 3 Dezimalstellen eg. 1.234 A
  char ChargerPlugged_fmt[13];                                                                                       // Hilfsvariable für die Interpretation des ChargerPlugged (1: Inserted; 0: Not inserted)
  char SysRunTicks_fmt[10];                                                                                          // Hilfsvariable für die Formatierung des SysRunTicks mit 1 Dezimalstelle eg. 123456.7 S
  char TempBat3_fmt[8];                                                                                              // Hilfsvariable für die Formatierung der TempBat3 mit 1 Dezimalstelle eg. 25.3 ℃
  char TempBat4_fmt[8];                                                                                              // Hilfsvariable für die Formatierung der TempBat4 mit 1 Dezimalstelle eg. 25.3 ℃
  char TempBat5_fmt[8];                                                                                              // Hilfsvariable für die Formatierung der TempBat5 mit 1 Dezimalstelle eg. 25.3 ℃
  char RTCTicksToSeconds_fmt[16];                                                                                    // Hilfsvariable für die Umrechnung der RTCTicks in Sekunden (1 tick = 1/32768 second) und Formatierung auf 3 Dezimalstellen eg. 123456.789 s
  char TimeEnterSleep_fmt[10];                                                                                       // Hilfsvariable für die Formatierung des TimeEnterSleep eg. 1234 s
  char PCLModuleSta_fmt[4];                                                                                          // Hilfsvariable für die Interpretation des PCLModuleSta (1: On; 0: Off)
  void prepareOutValues()
  {
    // Aktualisiere die Hilfsvariablen basierend auf den gelesenen Daten
    // CellVoltages von mV in V umrechnen und mit 3 Dezimalstellen formatieren
    for (int i = 0; i < 32; ++i)
    {
      dtostrf(CellVol[i] * 0.001, 6, 3, CellVol_fmt[i]);
    }
    // CellSta, CellWireResSta und AlarmBitMask in lesbare Bitmask-Strings umwandeln
    snprintf(CellSta_fmt, sizeof(CellSta_fmt), std::bitset<32>(CellSta).to_string().c_str());
    snprintf(CellWireResSta_fmt, sizeof(CellWireResSta_fmt), std::bitset<32>(CellWireResSta).to_string().c_str());
    snprintf(AlarmBitMask_fmt, sizeof(AlarmBitMask_fmt), std::bitset<32>(AlarmBitMask).to_string().c_str());
    // CellVolAve und CellVdifMax von mV in V umrechnen und mit 3 Dezimalstellen formatieren
    dtostrf(CellVolAve * 0.001, 5, 3, CellVolAve_fmt);
#if DIFFV_DIVIDER == 1
    dtostrf(CellVdifMax * 1.0, 5, 0, CellVdifMax_fmt); // Ohne Umrechnung, da bereits in mV
#else
    dtostrf(CellVdifMax * 0.001, 5, 3, CellVdifMax_fmt); // Umrechnung von mV in V
#endif
    // MaxVolCellNbr und MinVolCellNbr formatieren
    snprintf(MaxVolCellNbr_fmt, sizeof(MaxVolCellNbr_fmt), "%d", MaxVolCellNbr);
    snprintf(MinVolCellNbr_fmt, sizeof(MinVolCellNbr_fmt), "%d", MinVolCellNbr);
    // CellWireRes von mΩ in Ω umrechnen und mit 3 Dezimalstellen formatieren
    for (int i = 0; i < 32; ++i)
    {
      dtostrf(CellWireRes[i] * 0.001, 6, 3, CellWireRes_fmt[i]);
    }
    // TempMos von 0.1℃ in ℃ umrechnen und mit 1 Dezimalstelle formatieren
    dtostrf(TempMos * 0.1, 5, 1, TempMos_fmt);
    // BatVol von mV in V umrechnen und mit 3 Dezimalstellen formatieren
    dtostrf(BatVol * 0.001, 7, 3, BatVol_fmt);
    // BatWatt von mW in W umrechnen und mit 3 Dezimalstellen formatieren
    dtostrf(BatWatt * 0.001, 7, 3, BatWatt_fmt);
    // BatCurrent von mA in A umrechnen und mit 3 Dezimalstellen formatieren
    dtostrf(BatCurrent * 0.001, 7, 3, BatCurrent_fmt);
    // TempBat1 und TempBat2 von 0.1℃ in ℃ umrechnen und mit 1 Dezimalstelle formatieren
    dtostrf(TempBat1 * 0.1, 5, 1, TempBat1_fmt);
    dtostrf(TempBat2 * 0.1, 5, 1, TempBat2_fmt);
    // AlarmRaw als Dezimalzahl formatieren
    snprintf(Alarm_raw_fmt, sizeof(Alarm_raw_fmt), "%u", AlarmBitMask);
    // Alarme einzeln interpretieren
    for (int i = 0; i < 24; ++i)
    {
      strncpy(AlarmsValue_fmt[i], ((AlarmBitMask & (1 << i)) ? "Fault" : "Normal"), sizeof(AlarmsValue_fmt[i]));
    }
    // BalanCurrent von mA in A umrechnen und mit 3 Dezimalstellen formatieren
    dtostrf(BalanCurrent * 0.001, 7, 3, BalanCurrent_fmt);
    // BalanSta interpretieren (2: Discharge; 1: Charge; 0: Off)
    if (BalanSta == 2)
    {
      strncpy(BalanSta_fmt, "Discharge", sizeof(BalanSta_fmt));
    }
    else if (BalanSta == 1)
    {
      strncpy(BalanSta_fmt, "Charge", sizeof(BalanSta_fmt));
    }
    else
    {
      strncpy(BalanSta_fmt, "Off", sizeof(BalanSta_fmt));
    }
    // SOCStateOfcharge formatieren
    snprintf(SOCStateOfcharge_fmt, sizeof(SOCStateOfcharge_fmt), "%d", SOCStateOfcharge);
    // SOCCapRemain von mAH in AH umrechnen und mit 3 Dezimalstellen formatieren
    snprintf(SOCCapRemain_fmt, sizeof(SOCCapRemain_fmt), "%.3f", SOCCapRemain * 0.001);
    // SOCFullChargeCap von mAH in AH umrechnen und mit 3 Dezimalstellen formatieren
    snprintf(SOCFullChargeCap_fmt, sizeof(SOCFullChargeCap_fmt), "%.3f", SOCFullChargeCap * 0.001);
    // SOCCycleCount formatieren
    snprintf(SOCCycleCount_fmt, sizeof(SOCCycleCount_fmt), "%d", SOCCycleCount);
    // SOCCycleCap von mAH in AH umrechnen und mit 3 Dezimalstellen formatieren
    snprintf(SOCCycleCap_fmt, sizeof(SOCCycleCap_fmt), "%.3f", SOCCycleCap * 0.001);
    // SOCSOH formatieren
    snprintf(SOCSOH_fmt, sizeof(SOCSOH_fmt), "%d", SOCSOH);
    // Precharge interpretieren (1: On; 0: Off)
    strncpy(Precharge_fmt, Precharge ? "On" : "Off", sizeof(Precharge_fmt));
    // UserAlarm formatieren
    snprintf(UserAlarm_fmt, sizeof(UserAlarm_fmt), "%d", UserAlarm);
    // RunTime formatieren
    snprintf(RunTime_fmt, sizeof(RunTime_fmt), "%d", RunTime);
    // RunTime im Tage Stunden:Minuten:Sekunden Format umrechnen
    int days = RunTime / 86400;
    int hours = (RunTime % 86400) / 3600;
    int minutes = (RunTime % 3600) / 60;
    int seconds = RunTime % 60;
    snprintf(RunTime_fmt_dhms, sizeof(RunTime_fmt_dhms), "%d days %02d:%02d:%02d", days, hours, minutes, seconds);
    // Charge interpretieren (1: On; 0: Off)
    strncpy(Charge_fmt, Charge ? "On" : "Off", sizeof(Charge_fmt));
    // Discharge interpretieren (1: On; 0: Off)
    strncpy(Discharge_fmt, Discharge ? "On" : "Off", sizeof(Discharge_fmt));
    // UserAlarm2 formatieren
    snprintf(UserAlarm2_fmt, sizeof(UserAlarm2_fmt), "%d", UserAlarm2);
    // TimeDcOCPR, TimeDcSCPR, TimeCOCPR, TimeCSCPR, TimeUVPR und TimeOVPR formatieren
    snprintf(TimeDcOCPR_fmt, sizeof(TimeDcOCPR_fmt), "%d", TimeDcOCPR);
    snprintf(TimeDcSCPR_fmt, sizeof(TimeDcSCPR_fmt), "%d", TimeDcSCPR);
    snprintf(TimeCOCPR_fmt, sizeof(TimeCOCPR_fmt), "%d", TimeCOCPR);
    snprintf(TimeCSCPR_fmt, sizeof(TimeCSCPR_fmt), "%d", TimeCSCPR);
    snprintf(TimeUVPR_fmt, sizeof(TimeUVPR_fmt), "%d", TimeUVPR);
    snprintf(TimeOVPR_fmt, sizeof(TimeOVPR_fmt), "%d", TimeOVPR);
    // TempSensorAbsentMask interpretieren
    snprintf(TempSensorAbsentMask_fmt, sizeof(TempSensorAbsentMask_fmt), std::bitset<8>(TempSensorAbsentMask).to_string().c_str());
    snprintf(TempSensorAbsent_fmt, sizeof(TempSensorAbsent_fmt), "%d", TempSensorAbsentMask);
    for (int i = 0; i < 6; ++i)
    {
      strncpy(TempSensAbsentValues_fmt[i], (TempSensorAbsentMask & (1 << i)) ? "Normal" : "Missing", sizeof(TempSensAbsentValues_fmt[i]));
    }
    // Heating interpretieren (1: On; 0: Off)
    strncpy(Heating_fmt, Heating ? "On" : "Off", sizeof(Heating_fmt));
    // TimeEmergency formatieren
    snprintf(TimeEmergency_fmt, sizeof(TimeEmergency_fmt), "%d", TimeEmergency);
    // BatDisCurCorrect formatieren
    snprintf(BatDisCurCorrect_fmt, sizeof(BatDisCurCorrect_fmt), "%d", BatDisCurCorrect);
    // VolChargCur und VolDischargCur von mV in V umrechnen und mit 3 Dezimalstellen formatieren
    dtostrf(VolChargCur * 0.001, 7, 3, VolChargCur_fmt);
    dtostrf(VolDischargCur * 0.001, 7, 3, VolDischargCur_fmt);
    // BatVolCorrect mit 3 Dezimalstellen formatieren
    dtostrf(BatVolCorrect, 7, 3, BatVolCorrect_fmt);
    // BatVol2 von mV in V umrechnen und mit 2 Dezimalstellen formatieren
    dtostrf(BatVol2 * 0.01, 7, 2, BatVol2_fmt);
    // HeatCurrent von mA in A umrechnen und mit 3 Dezimalstellen formatieren
    dtostrf(HeatCurrent * 0.001, 7, 3, HeatCurrent_fmt);
    // ChargerPlugged interpretieren (1: Inserted; 0: Not inserted)
    strncpy(ChargerPlugged_fmt, ChargerPlugged ? "Inserted" : "Not inserted", sizeof(ChargerPlugged_fmt));
    // SysRunTicks von 0.1s in s umrechnen und mit 1 Dezimalstelle formatieren
    dtostrf(SysRunTicks * 0.1, 9, 1, SysRunTicks_fmt);
    // TempBat3, TempBat4 und TempBat5 von 0.1℃ in ℃ umrechnen und mit 1 Dezimalstelle formatieren
    dtostrf(TempBat3 * 0.1, 5, 1, TempBat3_fmt);
    dtostrf(TempBat4 * 0.1, 5, 1, TempBat4_fmt);
    dtostrf(TempBat5 * 0.1, 5, 1, TempBat5_fmt);
    // RTCTicks in Sekunden umrechnen (1 tick = 1/32768 second)
    dtostrf(RTCTicks * (1.0 / 32768.0), 9, 3, RTCTicksToSeconds_fmt);
    // TimeEnterSleep formatieren
    snprintf(TimeEnterSleep_fmt, sizeof(TimeEnterSleep_fmt), "%d", TimeEnterSleep);
    // PCLModuleSta interpretieren (1: On; 0: Off)
    strncpy(PCLModuleSta_fmt, PCLModuleSta ? "On" : "Off", sizeof(PCLModuleSta_fmt));
  }
} __attribute__((packed)); // Verhindert Padding und sorgt dafür, dass die Struktur genau so im Speicher liegt wie definiert;

// CellDataOld to store the old values for comparison and change detection, it has the same structure as CellData but without the helper fields and the Base64 encoded message
struct CellDataOld
{                                       //  bytes Description                   Unit      Multiplier or format
  uint16_t CellVol[32];                 //	64	  Cell voltages                 mV        0.001 multiplier
  uint32_t CellSta;                     //	4		  Enabled cells bitmask         BITMASK   BIT0: Cell1, BIT1: Cell2, ..., BIT31: Cell32
  uint16_t CellVolAve;                  //	2		  Average cell voltage          mV        0.001 multiplier
  uint16_t CellVdifMax;                 //	2		  Cell voltage difference       mV        0.001 multiplier or 1 mV depending on DIFFV_DIVIDER
  uint8_t MaxVolCellNbr;                //	1		  MaxVolCellNbr                 #
  uint8_t MinVolCellNbr;                //	1		  MinVolCellNbr                 #
  uint16_t CellWireRes[32];             //	64	  Cell wire resistance          mΩ
  int16_t TempMos;                      //	2		  TempMos                       0.1℃     0.1 multiplier, can be negative
  uint32_t CellWireResSta = 0xFFFFFFFF; //	4		  CellWireResSta                BITMASK   A value of 1 for BIT[n] indicates that the equalization line is alarming.
  uint32_t BatVol;                      //	4		  BatVol                        mV        0.001 multiplier
  uint32_t BatWatt;                     //	4		  BatWatt                       mW        0.001 multiplier
  int32_t BatCurrent;                   //	4		  BatCurrent                    mA        0.001 multiplier, can be negative
  int16_t TempBat1;                     //	2		  TempBat 1                     0.1℃     0.1 multiplier, can be negative
  int16_t TempBat2;                     //	2		  TempBat 2                     0.1℃     0.1 multiplier, can be negative
  uint32_t AlarmBitMask;                //  4     AlarmBitMask                  BITMASK   BIT0 - BIT23
  int16_t BalanCurrent;                 //	2		  BalanCurrent                  mA        0.001 multiplier, can be negative
  uint8_t BalanSta;                     //	1		  BalanSta                      #         2: Discharge; 1: Charge; 0: Off
  uint8_t SOCStateOfcharge;             //  1     SOCStateOfcharge              %         1 multiplier
  int32_t SOCCapRemain;                 //  4     SOCCapRemain                  mAH       0.001 multiplier, can be negative
  uint32_t SOCFullChargeCap;            //  4     SOCFullChargeCap              mAH	      0.001 multiplier
  uint32_t SOCCycleCount;               //  4     SOCCycleCount                 #
  uint32_t SOCCycleCap;                 //  4     SOCCycleCap                   mAH       0.001 multiplier
  uint8_t SOCSOH;                       //  1     SOCSOH                        %
  uint8_t Precharge;                    //  1     Precharge                     #         1: On; 0: Off
  uint16_t UserAlarm;                   //  2     UserAlarm                     #
  uint32_t RunTime;                     //  4     RunTime                       s
  uint8_t Charge;                       //  1     Charge                        #         1: On; 0: Off
  uint8_t Discharge;                    //  1     Discharge                     #         1: On; 0: Off
  uint16_t UserAlarm2;                  //  2     UserAlarm2                    #
  uint16_t TimeDcOCPR;                  //  2     TimeDcOCPR                    s
  uint16_t TimeDcSCPR;                  //  2     TimeDcSCPR                    s
  uint16_t TimeCOCPR;                   //  2     TimeCOCPR                     s
  uint16_t TimeCSCPR;                   //  2     TimeCSCPR                     s
  uint16_t TimeUVPR;                    //  2     TimeUVPR                      s
  uint16_t TimeOVPR;                    //  2     TimeOVPR                      s
  uint8_t TempSensorAbsentMask;         //  1     TempSensorAbsentMask          BITMASK   BIT0: MOS TempSensorAbsent; BIT1: BATTempSensor1Absent; BIT2: BATTempSensor2Absent; BIT3: BATTempSensor3Absent; BIT4: BATTempSensor4Absent; BIT5: BATTempSensor5Absent
  uint8_t Heating;                      //  1     Heating                       #         1: On; 0: Off
  uint16_t TimeEmergency;               //  2     TimeEmergency                 s
  uint16_t BatDisCurCorrect;            //  2     BatDisCurCorrect              #
  uint16_t VolChargCur;                 //  2     VolChargCur                   mV        0.001 multiplier
  uint16_t VolDischargCur;              //  2     VolDischargCur                mV        0.001 multiplier
  float BatVolCorrect;                  //  4     BatVolCorrect                 #
  uint16_t BatVol2;                     //  2     BatVol                        mV        0.01 multiplier
  int16_t HeatCurrent;                  //  2     HeatCurrent                   mA        0.001 multiplier
  uint8_t ChargerPlugged;               //  1     ChargerPlugged                #         1: Inserted; 0: Not inserted
  uint32_t SysRunTicks;                 //  4     SysRunTicks                   0.1S      0.1 multiplier
  int16_t TempBat3;                     //  2     TempBat 3                     0.1℃	   0.1 multiplier, can be negative
  int16_t TempBat4;                     //  2     TempBat 4                     0.1℃	   0.1 multiplier, can be negative
  int16_t TempBat5;                     //  2     TempBat 5                     0.1℃	   0.1 multiplier, can be negative
  uint32_t RTCTicks;                    //  4     RTCTicks		                  #         The countdown begins on January 1, 2020. RTC ticks, 1 tick = 1/32768 second, will overflow after around 136 years
  uint32_t TimeEnterSleep;              //  4     TimeEnterSleep                s
  uint8_t PCLModuleSta;                 //  1     PCLModuleSta                  #         1: On; 0: Off
  uint32_t RunTime2;                    //  4     Hilfsvariable zur Bestimmung ob ein neuer RunTime Wert vorliegt, da dieser in CellData aufgrund der Formatierung als String nicht direkt mit dem gelesenen Wert verglichen werden kann
};

#endif // STRUCT_CELL_DATA_H