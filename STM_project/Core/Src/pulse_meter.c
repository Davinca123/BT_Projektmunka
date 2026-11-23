/*
 * pulse_meter.c
 *
 *  Created on: Nov 23, 2026
 *      Author: hallgato
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "port_time.h"
#include "pulse_meter.h"
#include "ir_sensor.h"

#define PEAK_DETECTION_THRESHOLD 120.0f


void init_pulse_meter(s_pulse_meter_handle_t* hnd)
{
  if (hnd == NULL) return;

  hnd->ir_avg = 0.0f;          // DC component of heart rate measurement
  hnd->ir_ac_filtered = 0.0f;  // AC component of heart rate measurement

  hnd->ready_for_beat = 1;
  hnd->beat_cooldown = 0;    // Idle time between heartbeats
  hnd->last_beat_time = 0;  // Last heartbeat time
  hnd->bpm = 0;              // Pulse in BPM
}


uint8_t get_bpm(s_pulse_meter_handle_t* hnd)
{
  if (hnd == NULL) return 0;
  return hnd->bpm;
}


void calc_bpm(s_pulse_meter_handle_t* hnd)
{
  if (hnd == NULL) return;

  uint16_t ir_raw = 0;
  uint8_t is_finger_on;
  read_ir_sensor(&ir_raw, &is_finger_on);
  uint32_t system_ticks = get_systick()/10;

  // BPM calculation if finger is on
  if (is_finger_on == 1)
  {
    // If filter value is empty, set it so it does not start from 0
    if (hnd->ir_avg == 0.0f)
    {
      hnd->ir_avg = ir_raw;
    }

    // Filtering the DC component
    hnd->ir_avg = (hnd->ir_avg * 0.98f) + (ir_raw * 0.02f);

    // AC (the pulses) calculation and inversion (so beats are positive peaks)
    float ir_ac = hnd->ir_avg - (float) ir_raw;

    // Exponential moving average filter (80% previous, 20% new)
    hnd->ir_ac_filtered = (hnd->ir_ac_filtered * 0.8f) + (ir_ac * 0.2f);

    // Decreasing beat idle time
    if (hnd->beat_cooldown > 0)
    {
      hnd->beat_cooldown--;
    }

    // Hysteresis: re-arm beat detection only after the signal drops below 0
    if (hnd->ir_ac_filtered < 0.0f)
    {
      hnd->ready_for_beat = 1;
    }

    // Peak detection: crosses threshold, idle time is up, and signal dropped below 0 beforehand
    if ((hnd->ir_ac_filtered > PEAK_DETECTION_THRESHOLD) && (hnd->beat_cooldown == 0)
        && (hnd->ready_for_beat == 1))
    {
      hnd->ready_for_beat = 0; // Block new triggers until signal drops below 0 again

      // Calculate the time between this and the previous peak
      uint32_t delta_ticks = system_ticks - hnd->last_beat_time;
      hnd->last_beat_time = system_ticks;

      // Check if value is reasonable (40 - 180 BPM)
      if (delta_ticks >= 33 && delta_ticks <= 150)
      {
        // BPM calculation: BPM = 60 s / elapsed time
        uint8_t current_bpm = 6000 / delta_ticks;

        if (hnd->bpm == 0)
        {
          hnd->bpm = current_bpm; // First valid reading
        }
        else if (abs((int) current_bpm - (int) hnd->bpm) < 35) // Reject sudden noisy spikes
        {
          // Smooth the displayed BPM (75% previous, 25% new)
          hnd->bpm = (hnd->bpm * 3 + current_bpm) / 4;
        }
      }

      hnd->beat_cooldown = 35; // 350 ms idle time (max ~171 BPM)
    }
  }
  else
  {
    hnd->ir_avg = 0.0f;         // Reset DC filter
    hnd->ir_ac_filtered = 0.0f; // Reset AC filter
    hnd->bpm = 0;               // Reset BPM
    hnd->ready_for_beat = 1;    // Ready for the next measurement
    hnd->last_beat_time = system_ticks; // Avoid huge delta_ticks when finger is placed back
  }
}
