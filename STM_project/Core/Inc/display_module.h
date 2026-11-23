/*
 * display_module.h
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#ifndef INC_DISPLAY_MODULE_H_
#define INC_DISPLAY_MODULE_H_

#include "ugui.h"

/* Number of display modes (clock, step counter, pulse meter) */
#define DISP_MODE_NUMBER  3u

typedef enum {
  DISP_MODE_CLOCK         = 0,
  DISP_MODE_STEP_COUNTER  = 1,
  DISP_MODE_PULSE_METER   = 2
} e_disp_mode_t;

extern void pixel_draw_handler(UG_S16 x, UG_S16 y, UG_COLOR color);
extern void display_graphics_init(void);
extern void render_clock_frame(void);
extern void render_step_counter_frame(uint32_t step_count);
extern void render_pulse_meter_frame(uint8_t bpm);
extern void render_sample_screen(void);

#endif /* INC_DISPLAY_MODULE_H_ */
