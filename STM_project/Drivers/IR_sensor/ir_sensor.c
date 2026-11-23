/*
 * pulse_meter.c
 *
 *  Created on: Nov 23, 2026
 *      Author: hallgato
 */

#include <stdbool.h>
#include <stdlib.h>
#include "port_i2c.h"
#include "ir_sensor.h"

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



int read_ir_sensor(uint16_t* ir_raw, uint8_t* is_finger_on, uint8_t max_samples, uint8_t* num_samples)
{
  static uint8_t max_buf[IR_FIFO_DEPTH * 4]; // static: keep it off the (small) task stack
  uint8_t ptr_buf[3];                        // FIFO_WR_PTR, OVF_COUNTER, FIFO_RD_PTR (consecutive registers)
  int ret;

  *num_samples = 0;

  // Check how many samples are waiting in the FIFO
  ret = port_i2c_receive(MAX30100_ADDR, MAX_REG_FIFO_WR_PTR, 1, ptr_buf, 3, 100);
  if (ret != 0) return ret;

  uint8_t count = (uint8_t) ((ptr_buf[0] - ptr_buf[2]) & 0x0F);
  if ((count == 0) && (ptr_buf[1] != 0)) count = IR_FIFO_DEPTH; // WR == RD with overflow -> FIFO is full
  if (count > max_samples) count = max_samples;
  if (count == 0) return 0; // No new sample yet

  // Burst reading 4 bytes per sample from the FIFO data (bytes 3 and 4 are always 0, as the red LED is turned off)
  ret = port_i2c_receive(MAX30100_ADDR, MAX_REG_FIFO_DATA, 1, max_buf, (uint16_t) (count * 4), 100);
  if (ret != 0) return ret;

  for (uint8_t i = 0; i < count; i++)
  {
    // Combining the 1st and 2nd bytes
    ir_raw[i] = (uint16_t) ((max_buf[i * 4] << 8) | max_buf[i * 4 + 1]);

    // Finger detection
    is_finger_on[i] = (ir_raw[i] > IR_REFLECTION_THRESHOLD) ? 1 : 0;
  }

  *num_samples = count;
  return 0;
}
