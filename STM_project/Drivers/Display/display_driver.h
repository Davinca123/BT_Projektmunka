/*
 * display.h
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include <stdint.h>

#define GME12864_WIDTH 128
#define GME12864_HEIGHT 64

/*
 * @brief Return codes for the display driver
 */
typedef enum
{
  DISP_OK       = 0x00,
  DISP_COMM_ERROR    = 0x01, //I2C communication error
  DISP_INVALID_ARGUMENT = 0x02,
} e_disp_return;


extern e_disp_return gme12864_init(void);
extern void gme12864_clear(void);
extern e_disp_return gme12864_draw_pixel(uint8_t x, uint8_t y, uint8_t color);
extern e_disp_return gme12864_update(void);

#endif /* INC_DISPLAY_H_ */
