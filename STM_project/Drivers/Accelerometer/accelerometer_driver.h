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

// Acceleration registers
#define OUT_X_L     0x28 //Only the first one is needed since the memory addresses are next to each other allowing burst reading
//#define OUT_X_H     0x29
//#define OUT_Y_L     0x2A
//#define OUT_Y_H     0x2B
//#define OUT_Z_L     0x2C
//#define OUT_Z_H     0x2D

// Constants
#define ODR_100_HZ             0b0101 // 100 Hz I2C internal sampling
#define MODE_HIGH_PERF         0b01   // High performance mode on
#define LP_MODE_XX             0b00   // Low-power mode off
#define FULL_SCALE_SELECTION   0b01   // ±4g range

#define ACCEL_SENSITIVITY_4G   0.000488f // Sensitivity at ±4g range (g/LSB)

#define ACCEL_TIMEOUT_MS        100u


/* Global function */
extern void init_accelerometer(void);
extern void read_accelerometer(float* x_g, float* y_g, float* z_g);

#endif /* ACCELEROMETER_ACCELEROMETER_DRIVER_H_ */
