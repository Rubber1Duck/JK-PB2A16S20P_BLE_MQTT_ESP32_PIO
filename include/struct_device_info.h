#ifndef STRUCT_DEVICE_INFO_H
#define STRUCT_DEVICE_INFO_H
#include <Arduino.h>

struct publishItem
{
  char topic[128];  // Adjust the size as needed
  char payload[33]; // Adjust the size as needed (longest string should be a 32 bit value in binary format as string (00000000000000000000000000000000), which is 32 characters plus null terminator)
};

// Struktur für die Gerätedaten (DeviceInfo; FrameType 0x01)
struct DeviceInfo
{
  uint8_t Header[4];             // 4     Header                        #
  uint8_t FrameType;             // 1     Frame type                    #
  uint8_t FrameCounter;          // 1     Frame counter                 #
  char ManufacturerDeviceID[16]; // 16    Manufacturer Device ID        /0 terminated String  //show in HTML Device Info Block
  char HardwareVersion[8];       // 8     Hardware Version              /0 terminated String  //show in HTML Device Info Block
  char SoftwareVersion[8];       // 8     Software Version              /0 terminated String  //show in HTML Device Info Block
  uint32_t OddRunTime;           // 4     Odd Run Time                  s                     //show in HTML Device Info Block (converted to human readable format)
  uint32_t PwrOnTimes;           // 4     Power On Times                #
  char DeviceName[16];           // 16    Device Name                   /0 terminated String  //show in HTML Device Info Block
  char DevicePasscode[16];       // 16    Device Passcode               /0 terminated String
  char ManufacturingDate[8];     // 8     Manufacturing Date            /0 terminated String
#ifdef V19                       // it seems in V19 Serial Number is 16 bytes and there is no passcode field
  char SerialNumber[16];         // 16    Serial Number                 /0 terminated String
#else
  char SerialNumber[12]; // 12    Serial Number                 /0 terminated String
  char Passcode[4];      // 4     Passcode                      /0 terminated String
#endif
  char UserData[16];             // 16    User Data                     /0 terminated String
  char SetupPasscode[16];        // 16    Setup Passcode                /0 terminated String
  char UserData2[16];            // 16    User Data 2                   /0 terminated String
  uint8_t unknown1[34];          // 34    Unknown                       #
  uint8_t UART1MPRTOLNbr;        // 1     UART1 Protocol Number         #
  uint8_t CANMPRTOLNbr;          // 1     CAN Protocol Number           #
  uint8_t UART1MPRTOLEnable;     // 1     UART1 Protocol Enable         #
  uint8_t unknown2[15];          // 15    Unknown                       #
  uint16_t UARTMPRTOLEnable0_15; // 2     UART Protocol Enable 0-15     #
  uint8_t unknown3[14];          // 14    Unknown                       #
  uint8_t UART2MPRTOLNbr;        // 1     UART2 Protocol Number         #
  uint8_t UART2MPRTOLEnable;     // 1     UART2 Protocol Enable         #
  uint8_t unknown4[14];          // 14    Unknown                       #
  uint8_t LCDBuzzerTrigger;      // 1     LCD Buzzer Trigger            #
  uint8_t DRY1Trigger;           // 1     Dry1 Trigger                  #
  uint8_t DRY2Trigger;           // 1     Dry2 Trigger                  #
  uint8_t UARTMPTLVer;           // 1     UART Protocol Version         #
  uint32_t LCDBuzzerTriggerVal;  // 4     LCD Buzzer Trigger Value      #
  uint32_t LCDBuzzerReleaseVal;  // 4     LCD Buzzer Release Value      #
  uint32_t DRY1TriggerVal;       // 4     Dry1 Trigger Value            #
  uint32_t DRY1ReleaseVal;       // 4     Dry1 Release Value            #
  uint32_t DRY2TriggerVal;       // 4     Dry2 Trigger Value            #
  uint32_t DRY2ReleaseVal;       // 4     Dry2 Release Value            #
  uint32_t DataStoredPeriod;     // 4     Data Stored Period            #
  uint8_t RCVTime;               // 1     Receive Time                  #
  uint8_t RFVTime;               // 1     RFV Time                      #
  uint8_t CANMPTLVer;            // 1     CAN Protocol Version          #
  uint8_t unknown5[30];          // 30    Unknown                       #
  uint8_t Checksum;              // 1     Checksum                      #
  // Total: 300 bytes
  // Note: The structure is packed to ensure there is no padding between the fields, which allows us to directly copy the received byte array into this structure.
  // We will also add helper functions to convert the runtime and other relevant fields into human-readable
  // formats when needed.
  String getOddRunTimeStr() // Helper function to convert OddRunTime into a human-readable format
  {
    char OddRunTimeStr[64];
    uint32_t seconds = OddRunTime % 60;
    uint32_t minutes = (OddRunTime / 60) % 60;
    uint32_t hours = (OddRunTime / 3600) % 24;
    uint32_t days = OddRunTime / 86400;
    snprintf(OddRunTimeStr, sizeof(OddRunTimeStr), "%u days %02u:%02u:%02u", days, hours, minutes, seconds);
    return String(OddRunTimeStr);
  }
} __attribute__((packed)); // Verhindert Padding und sorgt dafür, dass die Struktur genau so im Speicher liegt wie definiert

#endif // STRUCT_DEVICE_INFO_H