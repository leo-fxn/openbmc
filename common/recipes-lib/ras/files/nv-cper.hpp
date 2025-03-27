#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  // 192byte nv definition
  unsigned char ip_sig[16];
  unsigned char error_type[2];
  unsigned char error_instance[2];
  unsigned char severity;
  unsigned char socket_number;
  unsigned char number_of_reg;
  unsigned char reserved;
  unsigned char instance_base[8];
  unsigned int val0;
  unsigned int val1;
  unsigned int mb1_error_code;  //val2
  unsigned char mb1_version_string[64]; //val3 - val18
  unsigned int channel_retired_mask_bad_pages;  // val19
  unsigned int channel_retired_mask_training_fail;  //val20
  unsigned int channel_1st_bad_pages; //val21
  unsigned int channel_2nd_bad_pages; //val22
  unsigned int channel_3rd_bad_pages; //val23
  unsigned int channel_fail_mask_frequency;
  unsigned int scratch_dram_channel_disable; //val25
  unsigned char data_undefined[];
} nv_cper_fwerror1_t;

int createNvSsifCperDumpEntry(const uint8_t payloadId, 
                              const uint8_t* data, const size_t dataSize);

#ifdef __cplusplus
} // extern "C"
#endif