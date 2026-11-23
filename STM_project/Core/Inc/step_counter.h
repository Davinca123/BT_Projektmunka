/*
 * step_counter.h
 *
 *  Created on: Nov 23, 2026
 *      Author: hallgato
 */

#ifndef INC_STEP_COUNTER_H_
#define INC_STEP_COUNTER_H_

typedef struct {
  uint32_t step_count;    // Total number of steps
  uint8_t step_cooldown;  // Step idle time (100 in ms increments)

  /* Previous values used for the filter*/
  float x_hist[3]; // Raw ENMO values: x[n], x[n-1], x[n-2]
  float y_hist[3]; // Filtered ENMO values: y[n], y[n-1], y[n-2]
}s_step_counter_handle_t;

extern void init_step_counter(s_step_counter_handle_t* hnd);
extern uint32_t get_step_count(s_step_counter_handle_t* hnd);
void set_step_count(s_step_counter_handle_t* hnd, uint32_t step_value);
extern void calc_step(s_step_counter_handle_t* hnd);

#endif /* INC_STEP_COUNTER_H_ */
