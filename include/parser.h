#ifndef PARSER_H
#define PARSER_H

#include "config.h"
#include "settings.h"
#include <Arduino.h>
#include "mqtt_handler.h"
#include <map>
#include "time.h"
#include "struct_device_info.h" // DeviceInfo structure
#include "struct_cell_data.h"   // CellData structure
#include "struct_config_info.h" // ConfigInfo structure

void readDeviceInfoRecord(void *message, const char *devicename);
void readCellDataRecord(void *message, const char *devicename);
void readConfigInfoRecord(void *message, const char *devicename);
void republishCachedRecords(const char *devicename);
String getLocalTimeString();

extern DeviceInfo deviceinfo;
extern CellData celldata;
extern ConfigInfo configinfo;
extern bool has_device_info;
extern bool has_cell_data;
extern bool has_config_info;

extern uint16_t min_pub_time;
extern uint16_t publish_delay;
extern volatile bool debug_flg_full;
extern volatile bool debug_flg;

#endif // PARSER_H