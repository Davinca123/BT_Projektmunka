/*
 * step_counter.c
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */


#include <stdio.h>
#include <math.h>



// Acceleration registers
#define OUT_X_L     0x28 //Only the first one is needed since the memory addresses are next to each other allowing burst reading
//#define OUT_X_H     0x29
//#define OUT_Y_L     0x2A
//#define OUT_Y_H     0x2B
//#define OUT_Z_L     0x2C
//#define OUT_Z_H     0x2D



#define ACCEL_SENSITIVITY_4G   0.000488f // Sensitivity at ±4g range (g/LSB)
#define STEP_THRESHOLD 0.2f // Step sensitivity threshold (g)

// Filter coefficients (2nd order Butterworth, cutoff frequency Fc=4Hz, sampling frequency Fs=10Hz)
// Coefficients were calulated using SciPy
#define FILTER_B0  0.63895f
#define FILTER_B1  1.27790f
#define FILTER_B2  0.63895f
#define FILTER_A1  1.14298f
#define FILTER_A2  0.41280f



volatile uint8_t timer_flag = 0;

// Previous values used for the filter
float x_hist[3] = { 0.0f, 0.0f, 0.0f }; // Raw ENMO values: x[n], x[n-1], x[n-2]
float y_hist[3] = { 0.0f, 0.0f, 0.0f }; // Filtered ENMO values: y[n], y[n-1], y[n-2]

uint32_t step_count = 0;      // Total number of steps
uint8_t step_cooldown = 0;    // Step idle time (100 in ms increments)



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM6) {
    timer_flag = 1;
  }
}




