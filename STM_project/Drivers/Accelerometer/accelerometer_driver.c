/*
 * accelerometer_driver.c
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#include "port_i2c.h"
#include "accelerometer_driver.h"



void init_accelerometer(void)
{
  // CTRL1 settings: 25 Hz constant sempling in High Performance mode
  uint8_t ctrl1_set_value = (ODR_25_HZ << 4) | (MODE_HIGH_PERF << 2)
      | LP_MODE_XX;
  port_i2c_transmit(DEV_ADDR, CTRL1_ADDR, 1, &ctrl1_set_value, 1, 100);

  // CTRL6 settings: ±4g range
  uint8_t ctrl6_set_value = (FULL_SCALE_SELECTION << 4);

  port_i2c_transmit(DEV_ADDR, CTRL6_ADDR, 1, &ctrl6_set_value, 1, 100);
}
