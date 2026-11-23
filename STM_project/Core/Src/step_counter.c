/*
 * step_counter.c
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */


#include <stdio.h>
#include <math.h>
#include <string.h>
#include "accelerometer_driver.h"
#include "step_counter.h"



#define STEP_THRESHOLD 0.2f // Step sensitivity threshold (g)

// Filter coefficients (2nd order Butterworth, cutoff frequency Fc=4Hz, sampling frequency Fs=10Hz)
// Coefficients were calulated using SciPy
#define FILTER_B0  0.63895f
#define FILTER_B1  1.27790f
#define FILTER_B2  0.63895f
#define FILTER_A1  1.14298f
#define FILTER_A2  0.41280f

#define STEP_COOLDOWN   35 // 40 * 100ms = 400 ms idle time between steps

/*
 * @brief Initialize step counter handle
 */
void init_step_counter(s_step_counter_handle_t* hnd)
{
  if (hnd == NULL) return;
  hnd->step_count = 0;
  hnd->step_cooldown = 0;
  memset(hnd->x_hist, 0, sizeof(hnd->x_hist));
  memset(hnd->y_hist, 0, sizeof(hnd->y_hist));
}

uint32_t get_step_count(s_step_counter_handle_t* hnd)
{
  if (hnd == NULL) return 0;
  return hnd->step_count;
}

void set_step_count(s_step_counter_handle_t* hnd, uint32_t step_value)
{
  if (hnd == NULL) return;
  hnd->step_count = step_value;
}

void calc_step(s_step_counter_handle_t* hnd)
{
  if (hnd == NULL) return;

  float x_g;
  float y_g;
  float z_g;
  read_accelerometer(&x_g, &y_g, &z_g);

  // Calculating ENMO
  float enmo = fmaxf(
      sqrtf((x_g * x_g) + (y_g * y_g) + (z_g * z_g)) - 1.0f,
      0.0f);

  // Saturation
  if (enmo > 2.0f) {
    enmo = 2.0f;
  }

  // Applying filter
  hnd->x_hist[2] = hnd->x_hist[1];
  hnd->x_hist[1] = hnd->x_hist[0];
  hnd->x_hist[0] = enmo;

  hnd->y_hist[2] = hnd->y_hist[1];
  hnd->y_hist[1] = hnd->y_hist[0];


  // New filtered value
  hnd->y_hist[0] = (FILTER_B0 * hnd->x_hist[0]) + (FILTER_B1 * hnd->x_hist[1])
      + (FILTER_B2 * hnd->x_hist[2]) - (FILTER_A1 * hnd->y_hist[1])
      - (FILTER_A2 * hnd->y_hist[2]);

//  float enmo_filtered = hnd->y_hist[0];

  // Decreasing step idle time
  if (hnd->step_cooldown > 0) {
    hnd->step_cooldown--;
  }

  // Peak detection
  // y_hist[1] is a peak if it is bigger than both neighboring values
  uint8_t is_peak = (hnd->y_hist[1] > hnd->y_hist[0])
      && (hnd->y_hist[1] > hnd->y_hist[2]);

  // Check if the peak is above the necessary threshold
  uint8_t is_above_thresh = (hnd->y_hist[1] > STEP_THRESHOLD);

  // If a valid peak is found, increase step_count and enter idle between steps
  if (is_peak && is_above_thresh && (hnd->step_cooldown == 0)) {
    hnd->step_count++;
    hnd->step_cooldown = STEP_COOLDOWN; // STEP_COOLDOWN * 100ms = 400 ms idle time between steps
  }
}

