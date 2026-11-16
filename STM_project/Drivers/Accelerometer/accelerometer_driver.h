/*
 * accelerometer_driver.h
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#ifndef ACCELEROMETER_ACCELEROMETER_DRIVER_H_
#define ACCELEROMETER_ACCELEROMETER_DRIVER_H_

#include <stdint.h>

// I2C address and registers
#define DEV_ADDR        0x32  // 7-bit I2C register shifted to the left
//#define WHO_AM_I_REG    0x0F  // WHO_AM_I_REG register
#define CTRL1_ADDR      0x20  // Control1 register
#define CTRL6_ADDR      0x25  // Range setting register

// Constants
#define ODR_25_HZ              0b0011 // 25 Hz internal sampling
#define MODE_HIGH_PERF         0b01   // High performance mode on
#define LP_MODE_XX             0b00   // Low-power mode off
#define FULL_SCALE_SELECTION   0b01   // ±4g range

extern void init_accelerometer(void);

#endif /* ACCELEROMETER_ACCELEROMETER_DRIVER_H_ */
