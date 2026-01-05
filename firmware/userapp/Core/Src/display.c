#include "display.h"
#include "u8g2.h"
#include "postcodes.h"
#include "tombstone.h"

#define OLED_Addr (0x3C << 1)
#define DISPLAY_INIT_FUNC u8g2_Setup_ssd1306_i2c_64x32_1f_f
#define I2CBUS hi2c2

static u8g2_t myDisplay;

static char lcd_buf[20] = {0};
static const char *seg_name = NULL;
static char seg_index_str[] = {'(', 'X', ')' ,'\0'};

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(arg_int);
        break;
    }
    return 1;
}

uint8_t u8x8_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32];		/* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buf_idx;
    uint8_t *data;

    switch(msg)
    {
    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while( arg_int > 0 ) {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        HAL_I2C_Master_Transmit(&I2CBUS, OLED_Addr, buffer, buf_idx, 100);
        break;
    default:
        return 0;
    }
    return 1;
}

void Display_ScrollText(const char *text) {
    /* Scroll `text` across the lower third of the 64x32 display. */
    const int screen_w = 64;
    const int screen_h = 32;
    const int lower_y = 22; /* top Y of lower third area (pixels) */
    const int lower_h = screen_h - lower_y; /* height of lower area */

    u8g2_SetFont(&myDisplay, u8g2_font_7x13_mr);
    int text_w = u8g2_GetStrWidth(&myDisplay, text);
    int y = lower_y + lower_h - 2; /* baseline near bottom */

    /* Start just off the right edge and scroll left until just off the left edge */
    for (int x = screen_w; x > -text_w; x--) {
        /* Clear only the lower third by drawing a background box (draw color 0) */
        u8g2_SetDrawColor(&myDisplay, 0);
        u8g2_DrawBox(&myDisplay, 0, lower_y, screen_w, lower_h);

        /* Draw the text in the foreground */
        u8g2_SetDrawColor(&myDisplay, 1);
        u8g2_DrawStr(&myDisplay, x, y, text);

        u8g2_SendBuffer(&myDisplay);
        HAL_Delay(30);
    }
    /* Leave display with lower area cleared after scrolling */
    u8g2_SetDrawColor(&myDisplay, 0);
    u8g2_DrawBox(&myDisplay, 0, lower_y, screen_w, lower_h);
    u8g2_SendBuffer(&myDisplay);

    u8g2_SetDrawColor(&myDisplay, 1);
}

/**
  * @brief OLED Display SSD1306 Initialization Function
  * @param None
  * @retval None
  */
void Display_Init(void)
{
    DISPLAY_INIT_FUNC(&myDisplay, U8G2_R0, u8x8_i2c, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&myDisplay); // send init sequence to the display, display is in sleep mode after this,
    u8g2_SetPowerSave(&myDisplay, 0); // wake up display
    u8g2_SetContrast(&myDisplay, 50);
    u8g2_ClearDisplay(&myDisplay);

    // Display firmware version
    u8g2_SetFont(&myDisplay, u8g2_font_7x13_mr);
    // FW header
    u8g2_DrawStr(&myDisplay, 26, 10, "FW");

    u8g2_SetFont(&myDisplay, u8g2_font_5x7_mr);
    // Print Bootloader version
    if (validate_iapl_tombstone()) {
        tombstone_t *iapl_ts = (tombstone_t *)(APP_BASE-TOMBSTONE_SIZE);
        snprintf(lcd_buf, sizeof(lcd_buf), "IAPL %i.%i", iapl_ts->ver_major, iapl_ts->ver_minor);
    } else {
        strncat(lcd_buf, "IAPL INVALID", sizeof(lcd_buf)-1);
    }
    u8g2_DrawStr(&myDisplay, 4, 22, lcd_buf);

    // Print userapp version
    if (validate_uapp_tombstone()) {
        tombstone_t *uapp_ts = (tombstone_t *)APP_BASE;
        snprintf(lcd_buf, sizeof(lcd_buf), "UAPP %i.%i", uapp_ts->ver_major, uapp_ts->ver_minor);
    } else {
        strncat(lcd_buf, "UAPP INVALID", sizeof(lcd_buf)-1);
    }
    u8g2_DrawStr(&myDisplay, 4, 32, lcd_buf);
    u8g2_SendBuffer(&myDisplay);
    HAL_Delay(2000);

    // Show splash screen
    u8g2_ClearDisplay(&myDisplay);
    u8g2_SetFont(&myDisplay, u8g2_font_7x13_mr);
    u8g2_DrawStr(&myDisplay, 4, 10, "ASPECT 2");
    u8g2_SetFont(&myDisplay, u8g2_font_5x7_mr);
    u8g2_SendBuffer(&myDisplay);
    Display_ScrollText("xboxresearch.com");

    // Cleanup
    u8g2_ClearDisplay(&myDisplay);
    Display_ShowCode(0x4242, 0x71);
}

void Display_ShowCode(uint16_t code, uint8_t segment) {
    seg_name = get_segment_name(segment);
    // Convert numeric index to ASCII character
    seg_index_str[1] = get_segment_index(segment) + '0';

    // Assemble content
    snprintf(lcd_buf, sizeof(lcd_buf), "%04X", code);

    u8g2_ClearDisplay(&myDisplay);
    u8g2_SetFont(&myDisplay, u8g2_font_luBS14_tn);
    u8g2_DrawStr(&myDisplay, 4, 32, lcd_buf);
    u8g2_SetFont(&myDisplay, u8g2_font_7x13_mr);
    u8g2_DrawStr(&myDisplay, 12, 10, seg_name);
    u8g2_DrawStr(&myDisplay, 40, 10, seg_index_str);
    u8g2_SendBuffer(&myDisplay);
}

/**
  * @brief OLED Display SSD1306 tick function for mainloop 
  * @param None
  * @retval None
  */
void Display_Tick(void)
{
    if (slave_hard_error()) {
        Display_Print("BUS ERROR");
    } else if (slave_is_new_segment_available()) {
        Display_ShowCode(
            read_postcode(),
            read_segment()
        );
    }
}

void Display_Print(const char *message)
{
    u8g2_ClearDisplay(&myDisplay);
    u8g2_SetFont(&myDisplay, u8g2_font_7x13_mr);
    u8g2_DrawStr(&myDisplay, 4, 10, message);
    u8g2_SendBuffer(&myDisplay);
}
