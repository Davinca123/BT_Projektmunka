/*
 * display_module.h
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#ifndef INC_DISPLAY_MODULE_H_
#define INC_DISPLAY_MODULE_H_

#include "ugui.h"

extern void pixel_draw_handler(UG_S16 x, UG_S16 y, UG_COLOR color);
extern void display_graphics_init(void);
extern void render_clock_frame(void);
extern void render_step_counter_frame(uint32_t step_count);
extern void render_sample_screen(void);

#endif /* INC_DISPLAY_MODULE_H_ */
