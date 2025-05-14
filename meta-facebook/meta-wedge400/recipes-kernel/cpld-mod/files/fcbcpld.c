/*
 * fcbcpld.c - The i2c driver for FCBCPLD
 *
 * Copyright 2019-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

//#define DEBUG

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "i2c_dev_sysfs.h"

static ssize_t fcbcpld_fan_rpm_show(struct device *dev,
                                    struct device_attribute *attr,
                                    char *buf)
{
  int val;

  val = i2c_dev_read_byte(dev, attr);
  if (val < 0) {
    return val;
  }
  /* Multiply by 150 to get the RPM */
  val *= 150;

  return scnprintf(buf, PAGE_SIZE, "%u\n", val);
}

#define fan_pwm_help_str                        \
  "each value represents 1/64 duty cycle"
  
#define fan_led_ctrl_help_str                   \
  "0x0: Under HW control\n"                     \
  "0x1: Red off, Blue on\n"                     \
  "0x2: Red on, Blue off\n"                     \
  "0x3: Red off, Blue off"

#define fan_led_blink_help_str                  \
  "0: no blink\n"                               \
  "1: blink"

#define intr_help_str                           \
  "0: Interrupt active\n"                       \
  "1: No interrupt"
  
#define cpld_cpu_blk_help_str                   \
  "0: CPLD passes the interrupt to CPU\n"       \
  "1: CPLD blocks incoming the interrupt"
	  
#define enable_help_str                         \
  "0: Disable\n"                                \
  "1: Enable"

static const i2c_dev_attr_st fcbcpld_attr_table[] = {
  {
    "board_id",
    "0x0: Wedge400 FCB board\n"
    "other: Reserved",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x0, 0, 2,
  },
  {
    "board_ver",
    "0x0: R0A\n"
    "other: Reserved",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x0, 4, 3,
  },
  {
    "cpld_ver",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x01, 0, 6,
  },
  {
    "cpld_released",
    "0: Not released\n"
    "1: Released version after PVT",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x01, 6, 1,
  },
  {
    "cpld_sub_ver",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x02, 0, 8,
  },
  {
    "lm75_int_1_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x07, 0, 1,
  },
  {
    "lm75_int_2_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x07, 1, 1,
  },
  {
    "fan1_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x09, 0, 1,
  },
  {
    "fan2_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x09, 1, 1,
  },
  {
    "fan3_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x09, 2, 1,
  },
  {
    "fan4_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x09, 3, 1,
  },
  {
    "fcb_eeprom_wp",
    "0: Not protect\n"
    "1: Protect",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x0f, 0, 1,
  },
  {
    "fan1_power_en",
    enable_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x10, 0, 1,
  },
  {
    "fan2_power_en",
    enable_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x10, 1, 1,
  },
  {
    "fan3_power_en",
    enable_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x10, 2, 1,
  },
  {
    "fan4_power_en",
    enable_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x10, 3, 1,
  },
  {
    "hotswap_pg_alert_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x11, 0, 1,
  },
  {
    "hotswap_alert_1_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x11, 1, 1,
  },
  {
    "hotswap_alert_2_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x11, 2, 1,
  },
  {
    "hotswap_fault_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x11, 3, 1,
  },
  {
    "hotswap_pg_alert_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x12, 0, 1,
  },
  {
    "hotswap_alert_1_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x12, 1, 1,
  },
  {
    "hotswap_alert_2_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x12, 2, 1,
  },
  {
    "hotswap_fault_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x12, 3, 1,
  },
  {
    "fcb_efuse_fltb_fan1_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 0, 1,
  },
  {
    "fcb_efuse_fltb_fan2_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 1, 1,
  },
  {
    "fcb_efuse_fltb_fan3_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 2, 1,
  },
  {
    "fcb_efuse_fltb_fan4_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 3, 1,
  },
  {
    "fcb_efuse_pg_fan1_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 4, 1,
  },
  {
    "fcb_efuse_pg_fan2_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 5, 1,
  },
  {
    "fcb_efuse_pg_fan3_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 6, 1,
  },
  {
    "fcb_efuse_pg_fan4_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 7, 1,
  },
  {
    "fcb_efuse_fltb_fan1_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 0, 1,
  },
  {
    "fcb_efuse_fltb_fan2_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 1, 1,
  },
  {
    "fcb_efuse_fltb_fan3_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 2, 1,
  },
  {
    "fcb_efuse_fltb_fan4_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 3, 1,
  },
  {
    "fcb_efuse_pg_fan1_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 4, 1,
  },
  {
    "fcb_efuse_pg_fan2_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 5, 1,
  },
  {
    "fcb_efuse_pg_fan3_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 6, 1,
  },
  {
    "fcb_efuse_pg_fan4_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 7, 1,
  },
  {
    "fan1_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x20, 0, 8,
  },
  {
    "fan2_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x21, 0, 8,
  },
  {
    "fan1_pwm",
    fan_pwm_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x22, 0, 6,
  },
  {
    "fan1_led_ctrl",
    fan_led_ctrl_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x24, 0, 2,
  },
  {
    "fan1_eeprom_wp",
    "Fan1 EERPOM Write Protect\n"
    "0: Not protect\n"
    "1: Protect",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x25, 0, 1,
  },
  {
    "fan1_present",
    "Fan-1 Present\n"
    "0: Present\n"
    "1: Not Present",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x28, 0, 1,
  },
  {
    "fan1_alive",
    "Fan1 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x28, 1, 1,
  },
  {
    "fan1_rear_alive",
    "Rear Fan1 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x28, 2, 1,
  },
  {
    "fan1_front_alive",
    "Front Fan1 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x28, 3, 1,
  },
  {
    "fan1_present_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x29, 0, 1,
  },
  {
    "fan1_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x29, 1, 1,
  },
  {
    "fan1_rear_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x29, 2, 1,
  },
  {
    "fan1_front_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x29, 3, 1,
  },
  {
    "fan1_present_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x2a, 0, 1,
  },
  {
    "fan1_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x2a, 1, 1,
  },
  {
    "fan1_rear_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x2a, 2, 1,
  },
  {
    "fan1_front_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x2a, 3, 1,
  },
  {
    "fan3_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x30, 0, 8,
  },
  {
    "fan4_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x31, 0, 8,
  },
  {
    "fan2_pwm",
    fan_pwm_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x32, 0, 6,
  },
  {
    "fan2_led_ctrl",
    fan_led_ctrl_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x34, 0, 2,
  },
  {
    "fan2_eeprom_wp",
    "Fan2 EERPOM Write Protect\n"
    "0: Not protect\n"
    "1: Protect",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x35, 0, 1,
  },
  {
    "fan2_present",
    "Fan-2 Present\n"
    "0: Present\n"
    "1: Not Present",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x38, 0, 1,
  },
  {
    "fan2_alive",
    "Fan2 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x38, 1, 1,
  },
  {
    "fan2_rear_alive",
    "Rear Fan2 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x38, 2, 1,
  },
  {
    "fan2_front_alive",
    "Front Fan2 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x38, 3, 1,
  },
  {
    "fan2_present_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x39, 0, 1,
  },
  {
    "fan2_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x39, 1, 1,
  },
  {
    "fan2_rear_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x39, 2, 1,
  },
  {
    "fan2_front_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x39, 3, 1,
  },
  {
    "fan2_present_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x3a, 0, 1,
  },
  {
    "fan2_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x3a, 1, 1,
  },
  {
    "fan2_rear_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x3a, 2, 1,
  },
  {
    "fan2_front_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x3a, 3, 1,
  },
  {
    "fan5_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x40, 0, 8,
  },
  {
    "fan6_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x41, 0, 8,
  },
  {
    "fan3_pwm",
    fan_pwm_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x42, 0, 6,
  },
  {
    "fan3_led_ctrl",
    fan_led_ctrl_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x44, 0, 2,
  },
  {
    "fan3_eeprom_wp",
    "Fan3 EERPOM Write Protect\n"
    "0: Not protect\n"
    "1: Protect",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x45, 0, 1,
  },
  {
    "fan3_present",
    "Fan-3 Present\n"
    "0: Present\n"
    "1: Not Present",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x48, 0, 1,
  },
  {
    "fan3_alive",
    "Fan3 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x48, 1, 1,
  },
  {
    "fan3_rear_alive",
    "Rear Fan3 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x48, 2, 1,
  },
  {
    "fan3_front_alive",
    "Front Fan1 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x48, 3, 1,
  },
  {
    "fan3_present_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x49, 0, 1,
  },
  {
    "fan3_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x49, 1, 1,
  },
  {
    "fan3_rear_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x49, 2, 1,
  },
  {
    "fan3_front_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x49, 3, 1,
  },
  {
    "fan3_present_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x4a, 0, 1,
  },
  {
    "fan3_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x4a, 1, 1,
  },
  {
    "fan3_rear_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x4a, 2, 1,
  },
  {
    "fan3_front_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x4a, 3, 1,
  },
  {
    "fan7_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x50, 0, 8,
  },
  {
    "fan8_input",
    NULL,
    fcbcpld_fan_rpm_show,
    NULL,
    0x51, 0, 8,
  },
  {
    "fan4_pwm",
    fan_pwm_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x52, 0, 6,
  },
  {
    "fan4_led_ctrl",
    fan_led_ctrl_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x54, 0, 2,
  },
  {
    "fan4_eeprom_wp",
    "Fan4 EERPOM Write Protect\n"
    "0: Not protect\n"
    "1: Protect",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x55, 0, 1,
  },
  {
    "fan4_present",
    "Fan-4 Present\n"
    "0: Present\n"
    "1: Not Present",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x58, 0, 1,
  },
  {
    "fan4_alive",
    "Fan4 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x58, 1, 1,
  },
  {
    "fan4_rear_alive",
    "Rear Fan4 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x58, 2, 1,
  },
  {
    "fan4_front_alive",
    "Front Fan5 Alive Status\n"
    "0: Alive\n"
    "1: Bad",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x58, 3, 1,
  },
  {
    "fan4_present_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x59, 0, 1,
  },
  {
    "fan4_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x59, 1, 1,
  },
  {
    "fan4_rear_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x59, 2, 1,
  },
  {
    "fan4_front_alive_int_mask",
    cpld_cpu_blk_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x59, 3, 1,
  },
  {
    "fan4_present_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x5a, 0, 1,
  },
  {
    "fan4_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x5a, 1, 1,
  },
  {
    "fan4_rear_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x5a, 2, 1,
  },
  {
    "fan4_front_alive_int_status",
    intr_help_str,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x5a, 3, 1,
  }
};

/* FCBCPLD id */
static const struct i2c_device_id fcbcpld_id[] = {
  { "fcbcpld", 0 },
  { },
};
MODULE_DEVICE_TABLE(i2c, fcbcpld_id);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
static int fcbcpld_probe(struct i2c_client *client)
#else
static int fcbcpld_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
#endif
{
  i2c_dev_data_st *pdata;

  pdata = devm_kmalloc(&client->dev, sizeof(*pdata), GFP_KERNEL);
  if (pdata == NULL)
    return -ENOMEM;
  i2c_set_clientdata(client, pdata);

  return devm_i2c_dev_sysfs_init(client, pdata, fcbcpld_attr_table,
                                 ARRAY_SIZE(fcbcpld_attr_table));
}

static struct i2c_driver fcbcpld_driver = {
  .class    = I2C_CLASS_HWMON,
  .driver = {
    .name = "fcbcpld",
  },
  .probe    = fcbcpld_probe,
  .id_table = fcbcpld_id,
};

module_i2c_driver(fcbcpld_driver);

MODULE_AUTHOR("Siyu Li");
MODULE_DESCRIPTION("fcbcpld Driver");
MODULE_LICENSE("GPL");
