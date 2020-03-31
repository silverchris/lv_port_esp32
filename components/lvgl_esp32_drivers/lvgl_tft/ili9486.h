/**
 * @file ili9486.h
 */

#ifndef ILI9844_H
#define ILI9844_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <stdint.h>

#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define DISP_BUF_SIZE (LV_HOR_RES_MAX * 40)
#define ili9486_DC   CONFIG_LVGL_DISP_PIN_DC
#define ili9486_RST  CONFIG_LVGL_DISP_PIN_RST
#define ili9486_BCKL CONFIG_LVGL_DISP_PIN_BCKL

// if text/images are backwards, try setting this to 1
#define ili9486_INVERT_DISPLAY CONFIG_LVGL_INVERT_DISPLAY

/*******************
 * ili9486 REGS
*********************/

/* Level 1 Commands (from the display Datasheet) */
#define ili9486_CMD_NOP                             0x00
#define ili9486_CMD_SOFTWARE_RESET                  0x01
#define ili9486_CMD_READ_DISP_ID                    0x04
#define ili9486_CMD_READ_ERROR_DSI                  0x05
#define ili9486_CMD_READ_DISP_STATUS                0x09
#define ili9486_CMD_READ_DISP_POWER_MODE            0x0A
#define ili9486_CMD_READ_DISP_MADCTRL               0x0B
#define ili9486_CMD_READ_DISP_PIXEL_FORMAT          0x0C
#define ili9486_CMD_READ_DISP_IMAGE_MODE            0x0D
#define ili9486_CMD_READ_DISP_SIGNAL_MODE           0x0E
#define ili9486_CMD_READ_DISP_SELF_DIAGNOSTIC       0x0F
#define ili9486_CMD_ENTER_SLEEP_MODE                0x10
#define ili9486_CMD_SLEEP_OUT                       0x11
#define ili9486_CMD_PARTIAL_MODE_ON                 0x12
#define ili9486_CMD_NORMAL_DISP_MODE_ON             0x13
#define ili9486_CMD_DISP_INVERSION_OFF              0x20
#define ili9486_CMD_DISP_INVERSION_ON               0x21
#define ili9486_CMD_DISPLAY_OFF                     0x28
#define ili9486_CMD_DISPLAY_ON                      0x29
#define ili9486_CMD_COLUMN_ADDRESS_SET              0x2A
#define ili9486_CMD_PAGE_ADDRESS_SET                0x2B
#define ili9486_CMD_MEMORY_WRITE                    0x2C
#define ili9486_CMD_MEMORY_READ                     0x2E
#define ili9486_CMD_PARTIAL_AREA                    0x30
#define ili9486_CMD_VERT_SCROLL_DEFINITION          0x33
#define ili9486_CMD_TEARING_EFFECT_LINE_OFF         0x34
#define ili9486_CMD_TEARING_EFFECT_LINE_ON          0x35
#define ili9486_CMD_MEMORY_ACCESS_CONTROL           0x36
#define ili9486_CMD_VERT_SCROLL_START_ADDRESS       0x37
#define ili9486_CMD_IDLE_MODE_OFF                   0x38
#define ili9486_CMD_IDLE_MODE_ON                    0x39
#define ili9486_CMD_COLMOD_PIXEL_FORMAT_SET         0x3A
#define ili9486_CMD_WRITE_MEMORY_CONTINUE           0x3C
#define ili9486_CMD_READ_MEMORY_CONTINUE            0x3E
#define ili9486_CMD_SET_TEAR_SCANLINE               0x44
#define ili9486_CMD_GET_SCANLINE                    0x45
#define ili9486_CMD_WRITE_DISPLAY_BRIGHTNESS        0x51
#define ili9486_CMD_READ_DISPLAY_BRIGHTNESS         0x52
#define ili9486_CMD_WRITE_CTRL_DISPLAY              0x53
#define ili9486_CMD_READ_CTRL_DISPLAY               0x54
#define ili9486_CMD_WRITE_CONTENT_ADAPT_BRIGHTNESS  0x55
#define ili9486_CMD_READ_CONTENT_ADAPT_BRIGHTNESS   0x56
#define ili9486_CMD_WRITE_MIN_CAB_LEVEL             0x5E
#define ili9486_CMD_READ_MIN_CAB_LEVEL              0x5F
#define ili9486_CMD_READ_FIRST_CHECKSUM             0xAA
#define ili9486_CMD_READ_CONTINUE_CHECKSUM          0xAF
#define ili9486_CMD_READ_ID1                        0xDA
#define ili9486_CMD_READ_ID2                        0xDB
#define ili9486_CMD_READ_ID3                        0xDC

/* Level 2 Commands (from the display Datasheet) */
#define ili9486_CMD_INTERFACE_MODE_CONTROL          0xB0
#define ili9486_CMD_FRAME_RATE_CONTROL_NORMAL       0xB1
#define ili9486_CMD_FRAME_RATE_CONTROL_IDLE_8COLOR  0xB2
#define ili9486_CMD_FRAME_RATE_CONTROL_PARTIAL      0xB3
#define ili9486_CMD_DISPLAY_INVERSION_CONTROL       0xB4
#define ili9486_CMD_BLANKING_PORCH_CONTROL          0xB5
#define ili9486_CMD_DISPLAY_FUNCTION_CONTROL        0xB6
#define ili9486_CMD_ENTRY_MODE_SET                  0xB7
#define ili9486_CMD_POWER_CONTROL_1                 0xC0
#define ili9486_CMD_POWER_CONTROL_2                 0xC1
#define ili9486_CMD_POWER_CONTROL_3                 0xC2
#define ili9486_CMD_POWER_CONTROL_4                 0xC3
#define ili9486_CMD_POWER_CONTROL_5                 0xC4
#define ili9486_CMD_VCOM_CONTROL_1                  0xC5
#define ili9486_CMD_CABC_CONTROL_1                  0xC6
#define ili9486_CMD_CABC_CONTROL_2                  0xC8
#define ili9486_CMD_CABC_CONTROL_3                  0xC9
#define ili9486_CMD_CABC_CONTROL_4                  0xCA
#define ili9486_CMD_CABC_CONTROL_5                  0xCB
#define ili9486_CMD_CABC_CONTROL_6                  0xCC
#define ili9486_CMD_CABC_CONTROL_7                  0xCD
#define ili9486_CMD_CABC_CONTROL_8                  0xCE
#define ili9486_CMD_CABC_CONTROL_9                  0xCF
#define ili9486_CMD_NVMEM_WRITE                     0xD0
#define ili9486_CMD_NVMEM_PROTECTION_KEY            0xD1
#define ili9486_CMD_NVMEM_STATUS_READ               0xD2
#define ili9486_CMD_READ_ID4                        0xD3
#define ili9486_CMD_POSITIVE_GAMMA_CORRECTION       0xE0
#define ili9486_CMD_NEGATIVE_GAMMA_CORRECTION       0xE1
#define ili9486_CMD_DIGITAL_GAMMA_CONTROL_1         0xE2
#define ili9486_CMD_DIGITAL_GAMMA_CONTROL_2         0xE3
#define ili9486_CMD_SPI_READ_SETTINGS               0xFB

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} lv_color_custom_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void ili9486_init(void);
void ili9486_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);
void ili9486_enable_backlight(bool backlight);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ili9486_H*/
