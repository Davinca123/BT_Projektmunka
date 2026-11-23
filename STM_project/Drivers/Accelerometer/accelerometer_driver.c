/*
 * accelerometer_driver.c
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#include "port_i2c.h"
#include "accelerometer_driver.h"


//@todo add return code
void init_accelerometer(void)
{
  // CTRL1 settings: 25 Hz constant sampling in High Performance mode
  uint8_t ctrl1_set_value = (ODR_100_HZ << 4) | (MODE_HIGH_PERF << 2)
      | LP_MODE_XX;
  port_i2c_transmit(DEV_ADDR, CTRL1_ADDR, 1, &ctrl1_set_value, 1, 100);

  // CTRL6 settings: ±4g range
  uint8_t ctrl6_set_value = (FULL_SCALE_SELECTION << 4);

  port_i2c_transmit(DEV_ADDR, CTRL6_ADDR, 1, &ctrl6_set_value, 1, 100);
}


void read_accelerometer(float* x_g, float* y_g, float* z_g)
{
  uint8_t buffer[6];
  // Only proceed if I2C reading was successful
  if (port_i2c_receive(DEV_ADDR, OUT_X_L, 1, buffer, 6, ACCEL_TIMEOUT_MS)
    == 0) {
    //Burst read from OUT_X_L (0x28) for 6 bytes
    port_i2c_receive(DEV_ADDR, OUT_X_L, 1, buffer, 6, ACCEL_TIMEOUT_MS);
  }
  else
  {
    //@todo: add return code
    return;
  }
  // Combining the lower and higher bytes
  int16_t raw_x = (int16_t) ((buffer[1] << 8) | buffer[0]);
  int16_t raw_y = (int16_t) ((buffer[3] << 8) | buffer[2]);
  int16_t raw_z = (int16_t) ((buffer[5] << 8) | buffer[4]);

  // Shifting by 2 bits to the right, according to the reference manual
  raw_x = raw_x >> 2;
  raw_y = raw_y >> 2;
  raw_z = raw_z >> 2;

  // Converting to g
  *x_g = raw_x * ACCEL_SENSITIVITY_4G;
  *y_g = raw_y * ACCEL_SENSITIVITY_4G;
  *z_g = raw_z * ACCEL_SENSITIVITY_4G;
}
