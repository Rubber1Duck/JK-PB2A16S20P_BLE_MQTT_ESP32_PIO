#ifndef CD_DATA_STRUCT_H
#define CD_DATA_STRUCT_H
#include <Arduino.h>

// Struktur für die Konfigurationsdaten (ConfigData; FrameType 0x03)
struct ConfigData
{
  uint8_t Header[4];               // 4     Header                        #
  uint8_t FrameType;               // 1     Frame type                    #
  uint8_t FrameCounter;            // 1     Frame counter                 #
  uint32_t VolSmartSleep;          // 4     VolSmartSleep                 mV         0.001 multiplier
  uint32_t VolCellUV;              // 4     VolCellUV                     mV         0.001 multiplier
  uint32_t VolCellUVPR;            // 4     VolCellUVPR                   mV         0.001 multiplier
  uint32_t VolCellOV;              // 4     VolCellOV                     mV         0.001 multiplier
  uint32_t VolCellOVPR;            // 4     VolCellOVPR                   mV         0.001 multiplier
  uint32_t VolBalanTrig;           // 4     VolBalanTrig                  mV         0.001 multiplier
  uint32_t VolSOC100percent;       // 4     VolSOC100percent              mV         0.001 multiplier
  uint32_t VolSOC0percent;         // 4     VolSOC0percent                mV         0.001 multiplier
  uint32_t VolCellRCV;             // 4     VolCellRCV                    mV         0.001 multiplier
  uint32_t VolCellRFV;             // 4     VolCellRFV                    mV         0.001 multiplier
  uint32_t VolSysPwrOff;           // 4     VolSysPwrOff                  mV         0.001 multiplier
  uint32_t CurBatCOC;              // 4     CurBatCOC                     mA         0.001 multiplier
  uint32_t TimBatCOCPDly;          // 4     TimBatCOCPDly                 s
  uint32_t TimBatCOCPRDly;         // 4     TimBatCOCPRDly                s
  uint32_t CurBatDcOC;             // 4     CurBatDcOC                    mA         0.001 multiplier
  uint32_t TimBatDcOCPDly;         // 4     TimBatDcOCPDly                s
  uint32_t TimBatDcOCPRDly;        // 4     TimBatDcOCPRDly               s
  uint32_t TimBatSCPRDly;          // 4     TimBatSCPRDly                 s
  uint32_t CurBalanMax;            // 4     CurBalanMax                   mA         0.001 multiplier
  int32_t TmpBatCOT;               // 4     TmpBatCOT                     ℃         0.1 multiplier, can be negative
  int32_t TmpBatCOTPR;             // 4     TmpBatCOTPR                   ℃         0.1 multiplier, can be negative
  int32_t TmpBatDcOT;              // 4     TmpBatDcOT                    ℃         0.1 multiplier, can be negative
  int32_t TmpBatDcOTPR;            // 4     TmpBatDcOTPR                  ℃         0.1 multiplier, can be negative
  int32_t TmpBatCUT;               // 4     TmpBatCUT                     ℃         0.1 multiplier, can be negative
  int32_t TmpBatCUTPR;             // 4     TmpBatCUTPR                   ℃         0.1 multiplier, can be negative
  int32_t TmpMosOT;                // 4     TmpMosOT                      ℃         0.1 multiplier, can be negative
  int32_t TmpMosOTPR;              // 4     TmpMosOTPR                    ℃         0.1 multiplier, can be negative
  uint8_t CellCount[4];            // 4     CellCount                     #          Only first byte used
  uint8_t BatChargeEN[4];          // 4     BatChargeEN                   #          Only first byte used; 1: On; 0: Off
  uint8_t BatDisChargeEN[4];       // 4     BatDisChargeEN                #          Only first byte used; 1: On; 0: Off
  uint8_t BalanEN[4];              // 4     BalanEN                       #          Only first byte used; 1: On; 0: Off
  uint32_t CapBatCell;             // 4     CapBatCell                    mAH        0.001 multiplier
  uint32_t ScpDelay;               // 4     ScpDelay                      microseconds
  uint32_t VolStartBalan;          // 4     VolStartBalan                 mV         0.001 multiplier
  uint32_t CellConWireRes[32];     // 128   CellConWireRes                mΩ         0.001 multiplier, will not be published, just for information
  uint32_t DevAddr;                // 4     DevAddr                       #          The address of the device
  uint32_t TimProdischarge;        // 4     TimProdischarge               s
  uint8_t Unknown1[4];             // 4     Unknown1                      #
  uint16_t SwitchesBits;           // 2     SwitchesBits                  Bitmask    BIT0: HeatEN (1: On; 0: Off),
                                   //                                                BIT1: DisableTempSensor (1:On; 0: Off),
                                   //                                                BIT2: GPSHeartbeat (1: On; 0: Off),
                                   //                                                BIT3: PortSwitch (1: RS485; 0: CAN),
                                   //                                                BIT4: LCDAlwaysOn (1: On; 0: Off),
                                   //                                                BIT5: SpecialCharger (1: On; 0: Off),
                                   //                                                BIT6: SmartSleep (1: On; 0: Off),
                                   //                                                BIT7: DisablePCLMpdule (1: On; 0: Off),
                                   //                                                BIT8: TimedStoredData (1: On; 0: Off),
                                   //                                                BIT9: ChargingFloatMode (1: On; 0: Off),
                                   //                                                BIT10: Reserved, Unknown,
                                   //                                                BIT11: Reserved, Unknown,
                                   //                                                BIT12: Reserved, Unknown,
                                   //                                                BIT13: Reserved, Unknown,
                                   //                                                BIT14: Reserved, Unknown,
                                   //                                                BIT15: Reserved, Unknown
  int8_t TmpHeatingStart;          // 1     TmpHeatingStart               ℃         0.1 multiplier, can be negative
  int8_t TmpHeatingStop;           // 1     TmpHeatingStop                ℃         0.1 multiplier, can be negative
  uint8_t TimSmartSleep;           // 1     TimSmartSleep                 #
  uint8_t DataDomainEnableControl; // 1     DataDomainEnableControl       #
  uint8_t Unknown2[11];            // 11    Unknown2                      #
  uint8_t Checksum;                // 1     CRC Checksum                  #
                                   // Total: 300 bytes

  // Hilfsvariablen, die nicht in der Struktur enthalten sind, aber zur Verarbeitung der Daten nützlich sein können
  char BatChargeEN_fmt[4];    // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char BatDisChargeEN_fmt[4]; // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char BalanEN_fmt[4];        // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char HeatEN[4];             // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char DisableTempSensor[4];  // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char GPSHeartbeat[4];       // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char PortSwitch[6];         // 6, wegen "RS485", Länge von 5 + 1 für Nullterminierung
  char LCDAlwaysOn[4];        // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char SpecialCharger[4];     // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char SmartSleep[4];         // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char DisablePCLModule[4];   // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char TimedStoredData[4];    // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  char ChargingFloatMode[4];  // 4, wegen "On"/"Off", Länge von 3 + 1 für Nullterminierung
  void update_switches()
  {
    snprintf(BatChargeEN_fmt, sizeof(BatChargeEN_fmt), "%s", (BatChargeEN[0] & 0x0001) ? "On" : "Off");
    snprintf(BatDisChargeEN_fmt, sizeof(BatDisChargeEN_fmt), "%s", (BatDisChargeEN[0] & 0x0001) ? "On" : "Off");
    snprintf(BalanEN_fmt, sizeof(BalanEN_fmt), "%s", (BalanEN[0] & 0x0001) ? "On" : "Off");
    snprintf(HeatEN, sizeof(HeatEN), "%s", (SwitchesBits & 0x0001) ? "On" : "Off");
    snprintf(DisableTempSensor, sizeof(DisableTempSensor), "%s", (SwitchesBits & 0x0002) ? "On" : "Off");
    snprintf(GPSHeartbeat, sizeof(GPSHeartbeat), "%s", (SwitchesBits & 0x0004) ? "On" : "Off");
    snprintf(PortSwitch, sizeof(PortSwitch), "%s", (SwitchesBits & 0x0008) ? "RS485" : "CAN");
    snprintf(LCDAlwaysOn, sizeof(LCDAlwaysOn), "%s", (SwitchesBits & 0x0010) ? "On" : "Off");
    snprintf(SpecialCharger, sizeof(SpecialCharger), "%s", (SwitchesBits & 0x0020) ? "On" : "Off");
    snprintf(SmartSleep, sizeof(SmartSleep), "%s", (SwitchesBits & 0x0040) ? "On" : "Off");
    snprintf(DisablePCLModule, sizeof(DisablePCLModule), "%s", (SwitchesBits & 0x0080) ? "On" : "Off");
    snprintf(TimedStoredData, sizeof(TimedStoredData), "%s", (SwitchesBits & 0x0100) ? "On" : "Off");
    snprintf(ChargingFloatMode, sizeof(ChargingFloatMode), "%s", (SwitchesBits & 0x0200) ? "On" : "Off");
  }

} __attribute__((packed)); // Verhindert Padding und sorgt dafür, dass die Struktur genau so im Speicher liegt wie definiert;

#endif // CD_DATA_STRUCT_H
