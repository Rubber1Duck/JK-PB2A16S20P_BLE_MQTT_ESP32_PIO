##Device Info Frame

| BMS      | Index           | Type   | Length | R/W | Content                | Unit    | Note                                |
|:--------:|:---------------:|:------:|:------:|:---:|:----------------------:|:-------:|:-----------------------------------:|
| all      | 0x0000 - 0x0003 | Byte   | 4      | R   | Frame header           |         | Fix value: 0x55, 0xAA, 0xEB, 0x90   |
| all      | 0x0004          | UINT8  | 1      | R   | Frame type             | #       | Device Info Frame = Fix value: 0x03 |
| all      | 0x0005          | UINT8  | 1      | R   | Frame counter          | #       | Increments with each frame          |
| all      | 0x0006 - 0x0015 | ASCII  | 16     | R   | ManufacturerDeviceID   | TEXT    | Null-terminated string              |
| all      | 0x0016 - 0x001D | ASCII  | 8      | R   | HardwareVersion        | TEXT    | Null-terminated string              |
| all      | 0x001E - 0x0025 | ASCII  | 8      | R   | SoftwareVersion        | TEXT    | Null-terminated string              |
| all      | 0x0026 - 0x0029 | UINT32 | 4      | R   | ODDRunTime             | s       |                                     |
| all      | 0x002A - 0x002D | UINT32 | 4      | R   | PWROnTimes             | #       |                                     |
| all      | 0x002E - 0x003D | ASCII  | 16     | RW  | DeviceName             | TEXT    | Null-terminated string              |
| all      | 0x003E - 0x004D | ASCII  | 16     | RW  | DevicePasscode         | TEXT    | Null-terminated string              |
| all      | 0x004E - 0x0055 | ASCII  | 8      | R   | ManufacturingDate      | TEXT    | Null-terminated string              |
| V14; V15 | 0x0056 - 0x0061 | ASCII  | 12     | R   | SerialNumber           | TEXT    | Null-terminated string              |
| V19      | 0x0056 - 0x0065 | ASCII  | 16     | R   | SerialNumber           | TEXT    | Null-terminated string              |
| V14; V15 | 0x0062 - 0x0065 | ASCII  | 4      | RW  | Passcode               | TEXT    | Null-terminated string              |
| all      | 0x0066 - 0x0075 | ASCII  | 16     | RW  | User data              | TEXT    | Null-terminated string              |
| all      | 0x0076 - 0x0085 | ASCII  | 16     | RW  | Setup passcode         | TEXT    | Null-terminated string              |
| all      | 0x0086 - 0x0095 | ASCII  | 16     | RW  | User data 2            | TEXT    | Null-terminated string              |
| all      | 0x0096 - 0x00B7 |        | 34     |     |                        |         | unknown                             |
| all      | 0x00B8          | UINT8  | 1      | RW  | UART1MPRTOLNbr         | #       | Depend on Protocol Version          |
| all      | 0x00B9          | UINT8  | 1      | RW  | CANMPRTOLNbr           | #       | Depend on Protocol Version          |
| all      | 0x00BA          | UINT8  | 1      | R   | UART1MPRTOLEnable      | BITMASK |                                     |
| all      | 0x00BB - 0x00C9 |        | 15     |     |                        |         | unknown                             |
| all      | 0x00CA - 0x00CB | UINT16 | 2      | RW  | UARTMPRTOLEnable[0-15] | BITMASK |                                     |
| all      | 0x00CC - 0x00D9 |        | 14     |     |                        |         | unknown                             |
| all      | 0x00DA          | UINT8  | 1      | RW  | UART2MPRTOLNbr         | #       | Depend on Protocol Version          |
| all      | 0x00DB          | UINT8  | 1      | R   | UART2MPRTOLEnable[0]   | BIT 0   |                                     |
| all      | 0x00DC - 0x00E9 |        | 14     |     |                        |         | unknown                             |
| all      | 0x00EA          | UINT8  | 1      | RW  | LCDBuzzerTrigger       | #       |                                     |
| all      | 0x00EB          | UINT8  | 1      | RW  | DRY1Trigger            | #       |                                     |
| all      | 0x00EC          | UINT8  | 1      | RW  | DRY2Trigger            | #       |                                     |
| all      | 0x00ED          | UINT8  | 1      | R   | UARTMPTLVer            | #       | UART Protocol Libary Version        |
| all      | 0x00EE - 0x00F1 | UINT32 | 4      | RW  | LCDBuzzerTriggerVal    | #       |                                     |
| all      | 0x00F2 - 0x00F5 | UINT32 | 4      | RW  | LCDBuzzerReleaseVal    | #       |                                     |
| all      | 0x00F6 - 0x00F9 | UINT32 | 4      | RW  | DRY1TriggerVal         | #       |                                     |
| all      | 0x00FA - 0x00FD | UINT32 | 4      | RW  | DRY1ReleaseVal         | #       |                                     |
| all      | 0x00FE - 0x0101 | UINT32 | 4      | RW  | DRY2TriggerVal         | #       |                                     |
| all      | 0x0102 - 0x0105 | UINT32 | 4      | RW  | DRY2ReleaseVal         | #       |                                     |
| all      | 0x0106 - 0x0109 | UINT32 | 4      | RW  | DataStoredPeriod       | ?       |                                     |
| all      | 0x010A          | UINT8  | 1      | RW  | RCVTime                | 0.1 H   |                                     |
| all      | 0x010B          | UINT8  | 1      | RW  | RFVTime                | 0.1 H   |                                     |
| all      | 0x010C          | UINT8  | 1      | R   | CANMPTLVer             | #       | CAN Protocol Libary Version         |
| all      | 0x010D          | UINT8  | 1      | R   | RVD                    |         | Reserved                            |
| all      | 0x010E          |        | 8      |     |                        |         | unknown                             |
| V19      | 0x0116          | UINT8  | 1      | RW  | ReBulkSOC              | %       |                                     |
| all      | 0x0117 - 0x012A |        | 20     |     |                        |         | unknown                             |
| all      | 0x012B          | UINT8  | 1      | R   | CRC Checksum           | #       |                                     |
