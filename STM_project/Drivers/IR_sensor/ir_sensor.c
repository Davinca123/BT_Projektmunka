/*
 * pulse_meter.c
 *
 *  Created on: Nov 23, 2026
 *      Author: hallgato
 */

#include <stdbool.h>
#include <stdlib.h>
#include "port_i2c.h"

// MAX30100 I2C address and registers
#define MAX30100_ADDR       (0x57 << 1) // 7-bit address shifted to the left
#define MAX_REG_MODE_CONFIG 0x06
#define MAX_REG_SPO2_CONFIG 0x07
#define MAX_REG_LED_CONFIG  0x09
#define MAX_REG_FIFO_WR_PTR 0x02
#define MAX_REG_OVF_COUNTER 0x03
#define MAX_REG_FIFO_RD_PTR 0x04
#define MAX_REG_FIFO_DATA   0x05

#define IR_REFLECTION_THRESHOLD 10000u


void init_ir_sensor()
{

  // MAX30100  initialization
  uint8_t max_init_data;

  // Mode: Heart Rate only (only the IR LED is needed, the red LED is turned off)
  max_init_data = 0x02;
  port_i2c_transmit(MAX30100_ADDR, MAX_REG_MODE_CONFIG, 1, &max_init_data, 1, 100);

  // SpO2 Config: 100 Hz sampling rate, 1600us pulse width (16-bites ADC resolution)
  max_init_data = 0x07;
  port_i2c_transmit(MAX30100_ADDR, MAX_REG_SPO2_CONFIG, 1, &max_init_data, 1, 100);

  // LED Config: Red LED turned off, IR LED 24mA current
  max_init_data = 0x07;
  port_i2c_transmit(MAX30100_ADDR, MAX_REG_LED_CONFIG, 1, &max_init_data, 1, 100);

  // FIFO memory logic and reading
  max_init_data = 0x00;
  port_i2c_transmit(MAX30100_ADDR, MAX_REG_FIFO_WR_PTR, 1, &max_init_data, 1, 100);
  port_i2c_transmit(MAX30100_ADDR, MAX_REG_OVF_COUNTER, 1, &max_init_data, 1, 100);
  port_i2c_transmit(MAX30100_ADDR, MAX_REG_FIFO_RD_PTR, 1, &max_init_data, 1, 100);
}



void read_ir_sensor(uint16_t* ir_raw, uint8_t* is_finger_on)
{
  uint8_t max_buf[4];

  // Reading 4 bytes from the FIFO data (bytes 3 and 4 are always 0, as the red LED is turned off)
  if (port_i2c_receive(MAX30100_ADDR, MAX_REG_FIFO_DATA, 1, max_buf, 4, 100)
      == 0)
  {
    // Combining the 1st and 2nd bytes
    *ir_raw = (uint16_t) ((max_buf[0] << 8) | max_buf[1]);
  }

  // Finger detection
  if (*ir_raw > IR_REFLECTION_THRESHOLD)
  {
    *is_finger_on = 1;
  }
  else
  {
    *is_finger_on = 0;
  }
}
