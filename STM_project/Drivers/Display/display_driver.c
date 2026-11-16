/*
 * display.c
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */


#include <display_driver.h>
#include <stdint.h>
#include <string.h>
#include "port_i2c.h"
#include "port_time.h"

#define GME12864_I2C_ADDR 0x78 // Default 7-bit I2C address (0x3C) shifted to the left
#define GME12864_BUF_SIZE (GME12864_WIDTH * GME12864_HEIGHT / 8) // 1024 bytes

// Control bytes for SSD1306 I2C communication
#define CTRL_BYTE_CMD 0x00
#define CTRL_BYTE_DATA 0x40

// 1024-byte framebuffer (8 pages x 128 columns)
static uint8_t framebuffer[GME12864_BUF_SIZE];
static uint8_t i2c_data[GME12864_BUF_SIZE+1];



static int ssd1306_write_cmd(uint8_t cmd) {
    uint8_t i2c_data[2];
    i2c_data[0] = CTRL_BYTE_CMD;
    i2c_data[1] = cmd;
    return port_i2c_transmit(GME12864_I2C_ADDR, i2c_data, 2, 1000);
}


// -------------------------------------------------------------------------
// DRIVER FUNCTIONS (Based on Datasheet Section 8 & Section 9.4)
// -------------------------------------------------------------------------
e_disp_return gme12864_init(void) {
    port_time_delay(100); // Power stabilization delay (Section 9.4)

    int err;

    err = ssd1306_write_cmd(0xAE); // Display OFF (Sleep mode)
    if(err != 0)
    {
      return DISP_COMM_ERROR;
    }

    // Clock & Multiplex configuration
    err = ssd1306_write_cmd(0xD5); // Set Display Clock Divide Ratio / Osc Frequency
    err = ssd1306_write_cmd(0x80); // Default ratio
    err = ssd1306_write_cmd(0xA8); // Set Multiplex Ratio
    err = ssd1306_write_cmd(0x3F); // 64MUX (0x3F = 63)
    err = ssd1306_write_cmd(0xD3); // Set Display Offset
    err = ssd1306_write_cmd(0x00); // No offset
    err = ssd1306_write_cmd(0x40); // Set Display Start Line to 0

    // Charge Pump (Required for internal DC/DC boost)
    err = ssd1306_write_cmd(0x8D); // Charge Pump Setting
    err = ssd1306_write_cmd(0x14); // Enable Charge Pump

    // Memory Addressing & Orientation
    err = ssd1306_write_cmd(0x20); // Set Memory Addressing Mode
    err = ssd1306_write_cmd(0x00); // Horizontal Addressing Mode
    err = ssd1306_write_cmd(0xA1); // Set Segment Re-map (Column 127 mapped to SEG0)
    err = ssd1306_write_cmd(0xC8); // Set COM Output Scan Direction (Remapped mode)

    // Hardware & Contrast Configuration
    err = ssd1306_write_cmd(0xDA); // Set COM Pins Hardware Configuration
    err = ssd1306_write_cmd(0x12); // Alternative COM pin config
    err = ssd1306_write_cmd(0x81); // Set Contrast Control
    err = ssd1306_write_cmd(0x66); // Contrast value from datasheet example (or 0xCF for brighter)
    err = ssd1306_write_cmd(0xD9); // Set Pre-charge Period
    err = ssd1306_write_cmd(0xF1); // Phase 1 = 1 DCLK, Phase 2 = 15 DCLKs
    err = ssd1306_write_cmd(0xDB); // Set VCOMH Deselect Level
    err = ssd1306_write_cmd(0x30); // ~0.83 x Vcc

    // Display Turn ON
    err = ssd1306_write_cmd(0xA5); // Entire Display White
    err = ssd1306_write_cmd(0xA6); // Set Normal Display (0 = OFF, 1 = ON)
    err = ssd1306_write_cmd(0xAF); // Display ON

    port_time_delay(1000); // Stabilization delay (Section 9.4)

    //Resume display
    gme12864_clear();
    gme12864_update();
    err = ssd1306_write_cmd(0xA4); // Display Resume to RAM content
    return err == 0 ? DISP_OK : DISP_COMM_ERROR;
}

void gme12864_clear(void) {
    memset(framebuffer, 0x00, GME12864_BUF_SIZE);
}

e_disp_return gme12864_draw_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= GME12864_WIDTH || y >= GME12864_HEIGHT)
    {
        return DISP_INVALID_ARGUMENT;
    }

    if (color) {
        framebuffer[x + (y / 8) * GME12864_WIDTH] |= (1 << (y % 8));
    } else {
        framebuffer[x + (y / 8) * GME12864_WIDTH] &= ~(1 << (y % 8));
    }

    return DISP_OK;
}

e_disp_return gme12864_update(void) {
    int err;
    // Set column start (0) and end (127)
    err = ssd1306_write_cmd(0x21);
    err = ssd1306_write_cmd(0x00);
    err = ssd1306_write_cmd(0x7F);
    if(err != 0)
    {
      return DISP_COMM_ERROR;
    }

    // Set page start (0) and end (7)
    err = ssd1306_write_cmd(0x22);
    err = ssd1306_write_cmd(0x00);
    err = ssd1306_write_cmd(0x07);

    // Send the entire 1024-byte buffer in one burst

    i2c_data[0] = CTRL_BYTE_DATA;
    memcpy(&i2c_data[1], framebuffer, GME12864_BUF_SIZE);
    port_i2c_transmit(GME12864_I2C_ADDR, i2c_data, GME12864_BUF_SIZE+1, 1000);
    return err == 0 ? DISP_OK : DISP_COMM_ERROR;
}
