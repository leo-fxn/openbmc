#ifndef __TDA38640_H__
#define __TDA38640_H__

#include "vr.h"

#define TDA38640_CNFG_BYTE_NUM 4
#define TDA38640_MAX_SECT_NUM 48
#define TDA38640_SECT_COLUMN_NUM 64
#define TDA38640_MAX_PAGE_NUM 4 //Each device has 1024 registers – address 0x000-0x3FF, they are divided into 4 pages - page number 0 to 3.

enum {
  CRC_LOW_REG = 0xAE,
  CRC_HIGH_REG = 0xB0,
  CNFG_REG = 0xB2,
  USER_1_REG = 0xB4,
  USER_2_REG = 0xB6,
  USER_3_REG = 0xB8,
  UNLOCK_REGS_REG = 0xD4,
  PROG_CMD_REG = 0xD6,
  REVISION_REG = 0xFD, //The silicon version value is stored in register 0x00FD [7:0] of page 0.
  PAGE_REG = 0xff,
};

enum {
  CNFG_WR = 0x12,
  USER_RD = 0x41,
  USER_WR = 0x42,
};

struct register_otp_info {
  // Registers to write, organized by page to enable faster binary search
  const uint16_t* start[TDA38640_MAX_PAGE_NUM]; // Start address of the register list for each page
  size_t size[TDA38640_MAX_PAGE_NUM];     // Number of registers in each page's list
};

struct tda38640_config_sect {
  uint16_t offset[TDA38640_SECT_COLUMN_NUM];
  uint8_t offset_count;
  uint8_t data[TDA38640_SECT_COLUMN_NUM * 16];
};

struct tda38640_config {
  uint32_t checksum;
  uint8_t cnfg_data[TDA38640_CNFG_BYTE_NUM];
  uint8_t sect_count;
  struct tda38640_config_sect section[TDA38640_MAX_SECT_NUM];
};

int get_tda38640_ver(struct vr_info*, char*);
void * tda38640_parse_file(struct vr_info*, const char*);
int tda38640_fw_update(struct vr_info*, void*);

#endif
