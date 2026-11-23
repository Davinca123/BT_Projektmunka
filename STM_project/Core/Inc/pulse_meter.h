/*
 * pulse_meter.h
 *
 *  Created on: Nov 23, 2026
 *      Author: hallgato
 */

#ifndef INC_PULSE_METER_H_
#define INC_PULSE_METER_H_




typedef struct {
  float ir_avg;             // DC component of heart rate measurement
  float ir_ac_filtered;     // AC component of heart rate measurement

  uint8_t ready_for_beat;
  uint8_t beat_cooldown;    // Idle time between heartbeats
  uint32_t last_beat_time;  // Last heartbeat time
  uint8_t bpm;              // Pulse in BPM
}s_pulse_meter_handle_t;

extern void init_pulse_meter(s_pulse_meter_handle_t* hnd);
extern uint8_t get_bpm(s_pulse_meter_handle_t* hnd);
extern void calc_bpm(s_pulse_meter_handle_t* hnd);

#endif /* INC_PULSE_METER_H_ */
