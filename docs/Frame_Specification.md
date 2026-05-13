# Frame Specification (JK02_32)

Following information is what i collect from serval documents and in the community about the **BLE protocol**

All RW values are theoreticaly writeable, but this project is not support writing values and will not support writing values in future

## Device Info Frame (Frame Type 0x03)

| BMS          | Index  | Type   | Length | R/W | Content                | Unit    | Note                                |
|--------------|--------|--------|--------|-----|------------------------|---------|-------------------------------------|
| all JK02_32S | 0x0000 | Byte   | 4      | R   | Frame header           |         | Fix value: 0x55, 0xAA, 0xEB, 0x90   |
| all JK02_32S | 0x0004 | UINT8  | 1      | R   | Frame type             | #       | Device Info Frame = Fix value: 0x03 |
| all JK02_32S | 0x0005 | UINT8  | 1      | R   | Frame counter          | #       | Increments with each frame          |
| all JK02_32S | 0x0006 | ASCII  | 16     | R   | ManufacturerDeviceID   | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0016 | ASCII  | 8      | R   | HardwareVersion        | TEXT    | Null-terminated string              |
| all JK02_32S | 0x001E | ASCII  | 8      | R   | SoftwareVersion        | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0026 | UINT32 | 4      | R   | ODDRunTime             | s       |                                     |
| all JK02_32S | 0x002A | UINT32 | 4      | R   | PWROnTimes             | #       |                                     |
| all JK02_32S | 0x002E | ASCII  | 16     | RW  | DeviceName             | TEXT    | Null-terminated string              |
| all JK02_32S | 0x003E | ASCII  | 16     | RW  | DevicePasscode         | TEXT    | Null-terminated string              |
| all JK02_32S | 0x004E | ASCII  | 8      | R   | ManufacturingDate      | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0056 | ASCII  | 12     | R   | SerialNumber           | TEXT    | Null-terminated string              |
| V19          | 0x0056 | ASCII  | 16     | R   | SerialNumber           | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0062 | ASCII  | 4      | RW  | Passcode               | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0066 | ASCII  | 16     | RW  | User data              | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0076 | ASCII  | 16     | RW  | Setup passcode         | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0086 | ASCII  | 16     | RW  | User data 2            | TEXT    | Null-terminated string              |
| all JK02_32S | 0x0096 |        | 34     |     |                        |         | unknown                             |
| all JK02_32S | 0x00B8 | UINT8  | 1      | RW  | UART1MPRTOLNbr         | #       | Depend on Protocol Version see **   |
| all JK02_32S | 0x00B9 | UINT8  | 1      | RW  | CANMPRTOLNbr           | #       | Depend on Protocol Version see ***  |
| all JK02_32S | 0x00BA | UINT8  | 1      | R   | UART1MPRTOLEnable      | BITMASK |                                     |
| all JK02_32S | 0x00BB |        | 15     |     |                        |         | unknown                             |
| all JK02_32S | 0x00CA | UINT16 | 2      | RW  | UARTMPRTOLEnable[0-15] | BITMASK |                                     |
| all JK02_32S | 0x00CC |        | 14     |     |                        |         | unknown                             |
| all JK02_32S | 0x00DA | UINT8  | 1      | RW  | UART2MPRTOLNbr         | #       | Depend on Protocol Version see **   |
| all JK02_32S | 0x00DB | UINT8  | 1      | R   | UART2MPRTOLEnable[0]   | BIT 0   |                                     |
| all JK02_32S | 0x00DC |        | 14     |     |                        |         | unknown                             |
| all JK02_32S | 0x00EA | UINT8  | 1      | RW  | LCDBuzzerTrigger       | #       | known trigger values see *          |
| all JK02_32S | 0x00EB | UINT8  | 1      | RW  | DRY1Trigger            | #       | known trigger values see *          |
| all JK02_32S | 0x00EC | UINT8  | 1      | RW  | DRY2Trigger            | #       | known trigger values see *          |
| all JK02_32S | 0x00ED | UINT8  | 1      | R   | UARTMPTLVer            | #       | UART Protocol Libary Version        |
| all JK02_32S | 0x00EE | UINT32 | 4      | RW  | LCDBuzzerTriggerVal    | #       |                                     |
| all JK02_32S | 0x00F2 | UINT32 | 4      | RW  | LCDBuzzerReleaseVal    | #       |                                     |
| all JK02_32S | 0x00F6 | UINT32 | 4      | RW  | DRY1TriggerVal         | #       |                                     |
| all JK02_32S | 0x00FA | UINT32 | 4      | RW  | DRY1ReleaseVal         | #       |                                     |
| all JK02_32S | 0x00FE | UINT32 | 4      | RW  | DRY2TriggerVal         | #       |                                     |
| all JK02_32S | 0x0102 | UINT32 | 4      | RW  | DRY2ReleaseVal         | #       |                                     |
| all JK02_32S | 0x0106 | UINT32 | 4      | RW  | DataStoredPeriod       | ?       |                                     |
| all JK02_32S | 0x010A | UINT8  | 1      | RW  | RCVTime                | 0.1 H   |                                     |
| all JK02_32S | 0x010B | UINT8  | 1      | RW  | RFVTime                | 0.1 H   |                                     |
| all JK02_32S | 0x010C | UINT8  | 1      | R   | CANMPTLVer             | #       | CAN Protocol Libary Version         |
| all JK02_32S | 0x010D | UINT8  | 1      | R   | RVD                    |         | Reserved                            |
| all JK02_32S | 0x010E |        | 8      |     |                        |         | unknown                             |
| V19          | 0x0116 | UINT8  | 1      | RW  | ReBulkSOC              | %       |                                     |
| all JK02_32S | 0x0117 |        | 20     |     |                        |         | unknown                             |
| all JK02_32S | 0x012B | UINT8  | 1      | R   | CRC Checksum           | #       |                                     |

(*) possible trigger values
 -  0 = OFF
 -  1 = Low SOC
 -  2 = Battery Over Voltage
 -  3 = Battery Under Voltage
 -  4 = Battery Cell Over Voltage
 -  5 = Battery Cell Under Voltage
 -  6 = Charge Over Current
 -  7 = Discharge Over Current
 -  8 = Battery Over Temperature
 -  9 = MOSFET Over Temperature
 - 10 = System Alarm
 - 11 = Battery Low Temperature
 - 12 = Remote Control
 - 13 = Above SOC
 - 14 = MOSFET Abnormal

(**) possible UART protocols for UART protocol libary version 1 (i only know about libary version 1)
 -  0 = 000-4G-GPS Remote module Common protocol
 -  1 = 001-JK BMS RS485 Modbus V1.0
 -  2 = 002-MIU U SERIES
 -  3 = 003-China tower shared batterie cabinet V1.1
 -  4 = 004-PACE_RS485_Modbus_V1.3
 -  5 = 005-PYLON_low_volage_Protocol_RS485_V... (not complete readable in the app)
 -  6 = 006-Growatt_BMS_RS485_Protocol_1xSxxP... (not complete readable in the app)
 -  7 = 007-Voltronic_Inverter_and_BMS_485_com... (not complete readable in the app)
 -  8 = 008-China tower shared batterie cabinet V.02
 -  9 = 009-WOW_RS485_Modbus_V1.3
 - 10 = 010-JK BMS LCD Protocol V2.0
 - 11 = 011-UART1 User customization
 - 12 = 012-UART2 User customization
 - 13 = 013-(9600)JK BMS RS485 Modbus V1.0
 - 14 = 014-(9600)PYLON_low_voltage_Protocol_R... (not complete readable in the app)
 - 15 = 015-JK BMS PBxx SERIES LCD Protocol V1.0
 - 16 = 016-JKBMS LIN BUS V1.0
 - 17 = 017-RS485 Protocol 17
 - 18 = 018-RS485 Protocol 18
 - 19 = 019-RS485 Protocol 19
 - 20 = 020-RS485 Protocol 20

(***) posible CAN protocols for CAN protocol libary version 1 (i only know about libary version 1)
 -  0 = 000-JK BMS CAN Protocol (250k) V2.0
 -  1 = 001-Deye Low-voltage hybrid inverter CAN c... (not complete readable in the app)
 -  2 = 002-PYLON-Low-voltage-V1.2
 -  3 = 003-Growatt BMS CAN-Bus-protocol-low-vol... (not complete readable in the app)
 -  4 = 004-Victron_CANbus_BMS_protocol_20170... (not complete readable in the app)
 -  5 = 005-MEGAREVO_Hybrid_BMSCAN_Protocol... (not complete readable in the app)
 -  6 = 006-JK BMS CAN Protocol (500k) V2.0
 -  7 = 007-INVT BMS CAN Bus protocol V1.02
 -  8 = 008-GoodWe LV BMS Protocol(EX/EM/S-B... (not complete readable in the app)
 -  9 = 009-FSS-ConnectingBat-TI-en-10 | Version 1.0
 - 10 = 010-MUST PV1800F-CAN communication P... (not complete readable in the app)
 - 11 = 011-LuxpowerTek Battery CAN protocol V01
 - 12 = 012-CAN BUS User customization
 - 13 = 013-CAN BUS User customization2
 - 14 = 014-CAN BUS Protocol 014
 - 15 = 015-CAN BUS Protocol 015
 - 16 = 016-CAN BUS Protocol 016
 - 17 = 017-CAN BUS Protocol 017
 - 18 = 018-CAN BUS Protocol 018
 - 19 = 019-CAN BUS Protocol 019
 - 20 = 020-CAN BUS Protocol 020


## Device Settings Frame (Frame Type 0x01)

| Index  | Type   | Length | R/W | Content                      | Unit | Note                                   |
|--------|--------|--------|-----|------------------------------|------|----------------------------------------|
| 0x0000 | byte   | 4      | R   | Frame header                 |      | Fix value: 0x55, 0xAA, 0xEB, 0x90      |
| 0x0004 | UINT8  | 1      | R   | Frame type                   | #    | Device SettingsFrame = Fix value: 0x01 |
| 0x0005 | UINT8  | 1      | R   | Frame Counter                | #    | Increments with each frame             |
| 0x0006 | UINT32 | 4      | RW  | VolSmartSleep                | mV   |                                        |
| 0x000A | UINT32 | 4      | RW  | VolCellUV                    | mV   |                                        |
| 0x000E | UINT32 | 4      | RW  | VolCellUVPR                  | mV   |                                        |
| 0x0012 | UINT32 | 4      | RW  | VolCellOV                    | mV   |                                        |
| 0x0016 | UINT32 | 4      | RW  | VolCellOVPR                  | mV   |                                        |
| 0x001A | UINT32 | 4      | RW  | VolBalanTrig                 | mV   |                                        |
| 0x001E | UINT32 | 4      | RW  | VolSOC100%                   | mV   |                                        |
| 0x0022 | UINT32 | 4      | RW  | VolSOC0%                     | mV   |                                        |
| 0x0026 | UINT32 | 4      | RW  | VolCellRCV                   | mV   |                                        |
| 0x002A | UINT32 | 4      | RW  | VolCellRFV                   | mV   |                                        |
| 0x002E | UINT32 | 4      | RW  | VolSysPwrOff                 | mV   |                                        |
| 0x0032 | UINT32 | 4      | RW  | CurBatCOC                    | mA   |                                        |
| 0x0036 | UINT32 | 4      | RW  | TIMBatCOCPDly                | s    |                                        |
| 0x003A | UINT32 | 4      | RW  | TIMBatCOCPRDly               | s    |                                        |
| 0x003E | UINT32 | 4      | RW  | CurBatDcOC                   | mA   |                                        |
| 0x0042 | UINT32 | 4      | RW  | TIMBatDcOCPDly               | s    |                                        |
| 0x0046 | UINT32 | 4      | RW  | TIMBatDcOCPRDly              | s    |                                        |
| 0x004A | UINT32 | 4      | RW  | TIMBatSCPRDly                | s    |                                        |
| 0x004E | UINT32 | 4      | RW  | CurBalanMax                  | mA   |                                        |
| 0x0052 | INT32  | 4      | RW  | TMPBatCOT                    | 0.1℃ |                                        |
| 0x0056 | INT32  | 4      | RW  | TMPBatCOTPR                  | 0.1℃ |                                        |
| 0x005A | INT32  | 4      | RW  | TMPBatDcOT                   | 0.1℃ |                                        |
| 0X005E | INT32  | 4      | RW  | TMPBatDcOTPR                 | 0.1℃ |                                        |
| 0X0062 | INT32  | 4      | RW  | TMPBatCUT                    | 0.1℃ |                                        |
| 0X0066 | INT32  | 4      | RW  | TMPBatCUTPR                  | 0.1℃ |                                        |
| 0X006A | INT32  | 4      | RW  | TMPMosOT                     | 0.1℃ |                                        |
| 0X006E | INT32  | 4      | RW  | TMPMosOTPR                   | 0.1℃ |                                        |
| 0X0072 | UINT32 | 4      | RW  | CellCount                    | #    | Only first byte used                   |
| 0X0076 | UINT32 | 4      | RW  | BatChargeEN                  | #    | Only first byte used; 1: On; 0: Off    |
| 0X007A | UINT32 | 4      | RW  | BatDisChargeEN               | #    | Only first byte used; 1: On; 0: Off    |
| 0X007E | UINT32 | 4      | RW  | BalanEN                      | #    | Only first byte used; 1: On; 0: Off    |
| 0X0082 | UINT32 | 4      | RW  | CapBatCell                   | mAH  |                                        |
| 0X0086 | UINT32 | 4      | RW  | SCPDelay                     | μs   |                                        |
| 0X008A | UINT32 | 4      | RW  | VolStartBalan                | mV   |                                        |
| 0X008E | UINT32 | 4      | RW  | CellConWireRes0              | μΩ   | 32 times CellConWireRes first          |
| .      | .      | .      | .   | .                            | .    | .                                      |
| 0X010A | UINT32 | 4      | RW  | CellConWireRes31             | μΩ   | 32 times CellConWireRes last           |
| 0X010E | UINT32 | 4      | RW  | DevAddr                      | #    |                                        |
| 0X0112 | UINT32 | 4      | RW  | TIMProdischarge              | s    |                                        |
| 0X0116 |        | 4      |     |                              |      | unknown                                |
| 0X011A | UINT16 | 2      | RW  | Switches BitMask             |      | Bit mask of switches                   |
| 0X011C | INT8   | 1      | RW  | TMPHeatingStart              | ℃    |                                        |
| 0X011D | INT8   | 1      | RW  | TMPHeatingStop               | ℃    |                                        |
| 0X011E | UINT8  | 1      | RW  | TIMSmartSleep                | H    |                                        |
| 0X011F | UINT8  | 1      | R   | Data domain enable control 0 |      |                                        |
| 0X0120 |        | 8      |     |                              |      | unknown                                |
| 0X0128 | INT8   | 1      | RW  | Discharge UTP                | ℃    |                                        |
| 0X0129 | INT8   | 1      | RW  | Discharge UTPR               | ℃    |                                        |
| 0X012A |        | 1      |     |                              |      | unknown                                |
| 0X012B | UINT8  | 1      |     | CRC checksum                 | #    |                                        |

(*) BitMap of Switches
 - BIT0 = HeatEN; 1: On; 0: Off	
 - BIT1 = Disable temp-sensor; 1: On; 0: Off	
 - BIT2 = GPS Heartbeat; 1: On; 0: Off	
 - BIT3 = Port Switch; 1: RS485; 0: CAN	
 - BIT4 = LCD Always On; 1: On; 0: Off	
 - BIT5 = Special Charger; 1: On; 0: Off	
 - BIT6 = SmartSleep; 1: On; 0: Off	
 - BIT7 = DisablePCLModule; 1: On; 0: Off	
 - BIT8 = TimedStoredData; 1: On; 0: Off	
 - BIT9 = ChargingFloatMode; 1: On; 0: Off	



## Cell Info Frame (Frame Type 0x02) also called **realtime data**

| Index  | Type   | Length | R/W | Content                  | Unit  | Note                                                                             |
|--------|--------|--------|-----|--------------------------|-------|----------------------------------------------------------------------------------|
| 0x0000 | byte   | 4      | R   | Frame header             |       | Fix value: 0x55, 0xAA, 0xEB, 0x90                                                |
| 0x0004 | UINT8  | 1      | R   | Frame type               | #     | Cell Info Frame = Fix value: 0x02                                                |
| 0x0005 | UINT8  | 1      | R   | Frame Counter            | #     | Increments with each frame                                                       |
| 0x0006 | UINT16 | 2      | R   | CellVol0                 | mV    | 32 times cell voltages first                                                     |
| .      | .      | .      | .   | .                        | .     | .                                                                                |
| 0x0044 | UINT16 | 2      | R   | CellVol31                | mV    | 32 times cell voltages last                                                      |
| 0x0046 | UINT32 | 4      | R   | CellSta                  | #     | Enabled cells bitmask A value of 1 for BIT[n] indicates that the battery exists. |
| 0x004A | UINT16 | 2      | R   | CellVolAve               | mV    |                                                                                  |
| 0x004C | UINT16 | 2      | R   | CellVdifMax              | mV    |                                                                                  |
| 0x004E | UINT8  | 1      | R   | MaxVolCellNbr            | #     |                                                                                  |
| 0x004F | UINT8  | 1      | R   | MinVolCellNbr            | #     |                                                                                  |
| 0x0050 | UINT16 | 2      | R   | CellWireRes0             | mΩ    | 32 times balance cable wire resistance first                                     |
| .      | .      | .      | .   | .                        | .     | .                                                                                |
| 0x008E | UINT16 | 2      | R   | CellWireRes31            | mΩ    | 32 times balance cable wire resistance last                                      |
| 0x0090 | INT16  | 2      | R   | TempMos                  | 0.1℃  |                                                                                  |
| 0x0092 | UINT32 | 4      | R   | CellWireResSta           | #     | A value of 1 for BIT[n] indicates that the equalization line is alarming.        |
| 0x0096 | UINT32 | 4      | R   | BatVol                   | mV    |                                                                                  |
| 0x009A | UINT32 | 4      | R   | BatWatt                  | mW    |                                                                                  |
| 0x009E | INT32  | 4      | R   | BatCurrent               | mA    |                                                                                  |
| 0x00A2 | INT16  | 2      | R   | TempBat 1                | 0.1℃  |                                                                                  |
| 0x00A4 | INT16  | 2      | R   | TempBat 2                | 0.1℃  |                                                                                  |
| 0x00A6 | UINT32 | 4      | R   | Alarm bitmask            | #     | Bit Mask of Alarms see (*)                                                       |
| 0x00AA | INT16  | 2      | R   | BalanCurrent             | mA    |                                                                                  |
| 0x00AC | UINT8  | 1      | R   | BalanSta                 | #     | 2: Discharge; 1: Charge; 0: Off                                                  |
| 0x00AD | UINT8  | 1      | R   | SOCStateOfcharge         | %     |                                                                                  |
| 0x00AE | INT32  | 4      | R   | SOCCapRemain             | mAH   |                                                                                  |
| 0x00B2 | UINT32 | 4      | R   | SOCFullChargeCap         | mAH   |                                                                                  |
| 0x00B6 | UINT32 | 4      | R   | SOCCycleCount            | #     |                                                                                  |
| 0x00BA | UINT32 | 4      | R   | SOCCycleCap              | mAH   |                                                                                  |
| 0x00BE | UINT8  | 1      | R   | SOCSOH                   | %     |                                                                                  |
| 0x00BF | UINT8  | 1      | R   | Precharge                | #     | 1: On; 0: Off                                                                    |
| 0x00C0 | UINT16 | 2      | R   | UserAlarm                | #     |                                                                                  |
| 0x00C2 | UINT32 | 4      | R   | RunTime                  | s     |                                                                                  |
| 0x00C6 | UINT8  | 1      | R   | Charge                   | #     | 1: On; 0: Off                                                                    |
| 0x00C7 | UINT8  | 1      | R   | Discharge                | #     | 1: On; 0: Off                                                                    |
| 0x00C8 | UINT16 | 2      | R   | UserAlarm2               | #     |                                                                                  |
| 0x00CA | UINT16 | 2      | R   | TimeDcOCPR               | s     |                                                                                  |
| 0x00CC | UINT16 | 2      | R   | TimeDcSCPR               | s     |                                                                                  |
| 0x00CE | UINT16 | 2      | R   | TimeCOCPR                | s     |                                                                                  |
| 0x00D0 | UINT16 | 2      | R   | TimeCSCPR                | s     |                                                                                  |
| 0x00D2 | UINT16 | 2      | R   | TimeUVPR                 | s     |                                                                                  |
| 0x00D4 | UINT16 | 2      | R   | TimeOVPR                 | s     |                                                                                  |
| 0x00D6 | UINT8  | 1      | R   | TempSensorAbsent bitmask | #     | Bit Mask of Absent Tempertur Sensors see (**)                                    |
| 0x00D7 | UINT8  | 1      | R   | Heating                  | #     | 1: On; 0: Off                                                                    |
| 0x00D8 | UINT16 | 2      | R   | Reserved                 | #     |                                                                                  |
| 0x00DA | UINT16 | 2      | R   | TimeEmergency            | s     |                                                                                  |
| 0x00DC | UINT16 | 2      | R   | BatDisCurCorrect         | #     |                                                                                  |
| 0x00DE | UINT16 | 2      | R   | VolChargCur              | mV    |                                                                                  |
| 0x00E0 | UINT16 | 2      | R   | VolDischargCur           | mV    |                                                                                  |
| 0x00E2 | FLOAT  | 4      | R   | BatVolCorrect            | #     |                                                                                  |
| 0x00E6 |        | 4      |     |                          |       | unknown                                                                          |
| 0x00EA | UINT16 | 2      | R   | BatVol                   | 0.01V |                                                                                  |
| 0x00EC | INT16  | 2      | R   | HeatCurrent              | mA    |                                                                                  |
| 0x00EE |        | 6      |     |                          |       | unknown                                                                          |
| 0x00F4 | UINT8  | 1      | R   | Reserve RVD              | #     |                                                                                  |
| 0x00F5 | UINT8  | 1      | R   | ChargerPlugged           | #     | 1: Inserted; 0: Not inserted                                                     |
| 0x00F6 | UINT32 | 4      | R   | SysRunTicks              | 0.1S  |                                                                                  |
| 0x00FA |        | 4      |     |                          |       | unknown                                                                          |
| 0x00FE | INT16  | 2      | R   | TempBat 3                | 0.1℃  |                                                                                  |
| 0x0100 | INT16  | 2      | R   | TempBat 4                | 0.1℃  |                                                                                  |
| 0x0102 | INT16  | 2      | R   | TempBat 5                | 0.1℃  |                                                                                  |
| 0x0104 |        | 2      |     |                          |       | unknown                                                                          |
| 0x0106 | UINT32 | 4      | R   | RTCTicks                 | #     | The countdown begins on January 1, 2020.                                         |
| 0x010A |        | 4      | R   |                          |       | unknown                                                                          |
| 0x010E | UINT32 | 4      | R   | TimeEnterSleep           | S     |                                                                                  |
| 0x0112 | UINT8  | 1      | R   | PCLModuleSta             | #     | 1: On; 0: Off                                                                    |
| 0x0113 | UINT8  | 1      | R   | RVD                      | #     | reserved                                                                         |
| 0x0114 |        | 2      |     |                          |       | unknown                                                                          |
| 0x0116 | UINT16 | 1      | R   | Charge Status Time       | S     |                                                                                  |
| 0x0118 | UINT8  | 1      | R   | Charge Status            | #     | 0x00: Bulk ; 0x01: Absorption ; 0x02: Float                                      |
| 0x0119 | UINT8  | 1      | R   | Dry contact bitmask      | #     | 0x00: DRY and DRY2 off ; 0x02: DRY1 on ; 0x04: DRY2 on ; 0x06: DRY1 and DRY2 on  |
| 0x011A |        | 17     |     |                          |       | unknown                                                                          |
| 0x012B | UINT8  | 1      | R   | CRC Checksum             |       |                                                                                  |

(*) Alarms Bitmap (1 = Fault; 0 = Normal)
 - BIT0 = AlarmWireRes
 - BIT1 = AlarmMosOTP
 - BIT2 = AlarmCellQuantity
 - BIT3 = AlarmCurSensorErr
 - BIT4 = AlarmCellOVP
 - BIT5 = AlarmBatOVP
 - BIT6 = AlarmChOCP
 - BIT7 = AlarmChSCP
 - BIT8 = AlarmChOTP
 - BIT9 = AlarmChUTP
 - BIT10 = AlarmCPUAuxCommuErr
 - BIT11 = AlarmCellUVP
 - BIT12 = AlarmBatUVP
 - BIT13 = AlarmDchOCP
 - BIT14 = AlarmDchSCP
 - BIT15 = AlarmDchOTP
 - BIT16 = AlarmChargeMOS
 - BIT17 = AlarmDischargeMOS
 - BIT18 = GPSDisconneted
 - BIT19 = Modify PWD. in time
 - BIT20 = Discharge On Failed
 - BIT21 = Battery Over Temp Alarm
 - BIT22 = Temperature sensor anomaly
 - BIT23 = PLCModule anomaly

(**) TempSensorAbsend Bitmap (1 = Normal; 0 = Missing)
 - BIT0 = MOS TempSensorAbsent
 - BIT1 = BATTempSensor1Absent
 - BIT2 = BATTempSensor2Absent
 - BIT3 = BATTempSensor3Absent
 - BIT4 = BATTempSensor4Absent
 - BIT5 = BATTempSensor5Absent
