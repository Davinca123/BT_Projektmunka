/*
 * display_module.c
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#include "stdio.h"
#include <display_driver.h>
#include "ugui.h"

// Gui instance
UG_GUI gui;


/************************************************************************
 ********************** Port Functions **********************************
 ***********************************************************************/
// uGUI pixel callback function
void pixel_draw_handler(UG_S16 x, UG_S16 y, UG_COLOR color) {
    // Cast uGUI coordinates and color to match your driver's expected types
    gme12864_draw_pixel((uint8_t)x, (uint8_t)y, (uint8_t)color);
}

/************************************************************************
 ********************** Private Functions *******************************
 ***********************************************************************/



/************************************************************************
 ********************** Global Functions ********************************
 ***********************************************************************/

void display_graphics_init(void) {
    // Initialize uGUI
    UG_Init(&gui, pixel_draw_handler, GME12864_WIDTH, GME12864_HEIGHT);

    UG_FontSelect(&FONT_8X12);
}

void render_clock_frame(void)
{
  gme12864_clear();

  UG_DrawFrame(0, 0, 127, 63, 1);

  char buf[32];
  snprintf(buf, sizeof(buf), "11:22:33");
  UG_PutString(4, 25, buf);

  gme12864_update();
}

void render_step_counter_frame(uint32_t step_count)
{
  gme12864_clear();

  UG_DrawFrame(0, 0, 127, 63, 1);

  char buf[32];
  snprintf(buf, sizeof(buf), "Step: %u", (unsigned int)step_count);
  UG_PutString(4, 25, buf);

  gme12864_update();
}

void render_pulse_meter_frame(uint8_t bpm)
{
  gme12864_clear();

  UG_DrawFrame(0, 0, 127, 63, 1);

  char buf[32];
  snprintf(buf, sizeof(buf), "Pulse: %u", (unsigned int)bpm);
  UG_PutString(4, 25, buf);

  gme12864_update();
}

void render_sample_screen(void) {
    // 1. Clear the local framebuffer
    gme12864_clear();

    // 2. Draw a geometric border around the screen (x0, y0, x1, y1, color)
    UG_DrawFrame(0, 0, 127, 63, 1);

    // 3. Draw a dividing line
    UG_DrawLine(0, 15, 127, 15, 1);

    // 4. Write text and numbers
    UG_FontSelect(&FONT_8X12);
    UG_PutString(4, 3, "GME12864 OLED");

    // To print numbers or formatted variables, use standard sprintf then UG_PutString:
    char buf[32];
    int sensor_value = 42;
    sprintf(buf, "Sensor: %d", sensor_value);
    UG_PutString(4, 25, buf);

    // 5. Flush the updated framebuffer over I2C to the physical OLED
    gme12864_update();
}
