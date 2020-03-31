/**
 * @file ili9486.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "ili9486.h"
#include "disp_spi.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*********************
 *      DEFINES
 *********************/
#define TAG "ili9486"

/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ili9486_send_cmd(uint8_t cmd);

static void ili9486_send_data(void *data, uint16_t length);

static void ili9486_send_color(void *data, uint16_t length);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// From https://github.com/notro/fbtft/blob/master/fb_ili9486.c
void ili9486_init(void) {
    lcd_init_cmd_t ili_init_cmds[] = {
            {ili9486_CMD_SLEEP_OUT,                 {0x00},                                                                                     0x80},
            {ili9486_CMD_COLMOD_PIXEL_FORMAT_SET,   {0x66},                                                                                     1},
            {ili9486_CMD_POWER_CONTROL_3,           {0x44},                                                                                     1},
            {ili9486_CMD_VCOM_CONTROL_1,            {0x00, 0x00, 0x00, 0x00},                                                                   4},
            {ili9486_CMD_POSITIVE_GAMMA_CORRECTION, {0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98, 0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00}, 15},
            {ili9486_CMD_NEGATIVE_GAMMA_CORRECTION, {0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75, 0x37, 0x06, 0x10, 0x03, 0x24, 0x30, 0x00}, 15},
            {ili9486_CMD_DISP_INVERSION_OFF,        {0x00},                                                                                     1},
            {ili9486_CMD_MEMORY_ACCESS_CONTROL,     {0x28},                                                                                     1},
            {ili9486_CMD_INTERFACE_MODE_CONTROL,    {0x00},                                                                                     1},
            {ili9486_CMD_FRAME_RATE_CONTROL_NORMAL, {0xA0},                                                                                     1},
            {ili9486_CMD_DISPLAY_INVERSION_CONTROL, {0x02},                                                                                     1},
            {ili9486_CMD_DISPLAY_FUNCTION_CONTROL,  {0x02, 0x02},                                                                               2},
            {ili9486_CMD_WRITE_CTRL_DISPLAY,        {0x28},                                                                                     1},
            {ili9486_CMD_DISPLAY_ON,                {0x00},                                                                                     0x80},
            {0,                                     {0},                                                                                        0xff},
    };

    //Initialize non-SPI GPIOs
    gpio_set_direction(ili9486_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(ili9486_RST, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(ili9486_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(ili9486_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "ili9486 initialization.");

    // Exit sleep
    ili9486_send_cmd(0x01);    /* Software reset */
    vTaskDelay(100 / portTICK_RATE_MS);

    //Send all the commands
    uint16_t cmd = 0;
    while (ili_init_cmds[cmd].databytes != 0xff) {
        ili9486_send_cmd(ili_init_cmds[cmd].cmd);
        ili9486_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes & 0x1F);
        if (ili_init_cmds[cmd].databytes & 0x80) {
            vTaskDelay(200 / portTICK_RATE_MS);
        }
        cmd++;
    }
}

// Flush function based on mvturnho repo
void ili9486_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

    lv_color16_t *buffer_16bit = (lv_color16_t *) color_map;
    uint8_t *mybuf = (uint8_t *) heap_caps_malloc(3 * size * sizeof(uint8_t), MALLOC_CAP_DMA);

    uint32_t LD = 0;
    uint32_t j = 0;

    for (uint32_t i = 0; i < size; i++) {
        LD = buffer_16bit[i].full;
        mybuf[j] = (uint8_t)((LD & 0xF800) >> 8);
        j++;
        mybuf[j] = (uint8_t)((LD & 0x07E0) >> 3);
        j++;
        mybuf[j] = (uint8_t)((LD & 0x001F) << 3);
        j++;
    }

    /* Column addresses  */
    uint8_t xb[] = {
            (uint8_t)(area->x1 >> 8) & 0xFF,
            (uint8_t)(area->x1) & 0xFF,
            (uint8_t)(area->x2 >> 8) & 0xFF,
            (uint8_t)(area->x2) & 0xFF,
    };

    /* Page addresses  */
    uint8_t yb[] = {
            (uint8_t)(area->y1 >> 8) & 0xFF,
            (uint8_t)(area->y1) & 0xFF,
            (uint8_t)(area->y2 >> 8) & 0xFF,
            (uint8_t)(area->y2) & 0xFF,
    };

    /*Column addresses*/
    ili9486_send_cmd(ili9486_CMD_COLUMN_ADDRESS_SET);
    ili9486_send_data(xb, 4);

    /*Page addresses*/
    ili9486_send_cmd(ili9486_CMD_PAGE_ADDRESS_SET);
    ili9486_send_data(yb, 4);

    /*Memory write*/
    ili9486_send_cmd(ili9486_CMD_MEMORY_WRITE);

    ili9486_send_color((void *) mybuf, size * 3);
    heap_caps_free(mybuf);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


static void ili9486_send_cmd(uint8_t cmd) {
    while (disp_spi_is_busy()) {}
    gpio_set_level(ili9486_DC, 0);     /*Command mode*/
    // shifted data so it ends up in the right shift register
    uint8_t cmd_shift[2] = {0x00, cmd};
    disp_spi_send_data(&cmd_shift[0], 2);
}

static void ili9486_send_data(void *data, uint16_t length) {
    while (disp_spi_is_busy()) {}
    gpio_set_level(ili9486_DC, 1);     /*Data mode*/
    uint8_t data_shift[2] = {0x00, 0x00};
    for(int i = 0; i<length; i++){
        // Send the data out one byte at a time, shifted so that it ends up in the right shift register
        data_shift[1] = *((uint8_t*)data+i);
        disp_spi_send_data(&data_shift[0], 2);
    }
}

static void ili9486_send_color(void *data, uint16_t length) {
    while (disp_spi_is_busy()) {}
    gpio_set_level(ili9486_DC, 1);   /*Data mode*/
    disp_spi_send_colors(data, length);
}

