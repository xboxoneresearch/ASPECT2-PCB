#include "display.h"
#include "slave.h"

static uint16_t code = 0;
static uint8_t seg = 0;
static char lcd_buf[10] = {0};
static const char *seg_name = NULL;
/**
  * @brief OLED Display SSD1306 Initialization Function
  * @param None
  * @retval None
  */
void Display_Init(void)
{
    ssd1306_Init();
    ssd1306_SetContrast(50);
}

const char *get_segment_name(uint8_t seg) {
  switch (seg & 0xF0) {
    case 0x70:
      return "OS";
    case 0x30:
      return "SP";
    case 0x10:
      return "SMC";
    default:
      return "???";
  }
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
      code = 
              (slave_reg_read(0x20) & 0x0F)
              | ((slave_reg_read(0x21) & 0x0F) << 4)
              | ((slave_reg_read(0x22) & 0x0F) << 8)
              | ((slave_reg_read(0x23) & 0x0F) << 12);

      seg = slave_reg_read(0x24);
      seg_name = get_segment_name(seg);

      /* Assemble content */
      memset(lcd_buf, 0, 10);
      snprintf(lcd_buf, 10, "%04X", code, seg);

      /* Draw to screen */
      ssd1306_Fill(Black);
      ssd1306_SetCursor(0, 0);
      ssd1306_WriteString(seg_name, Font_7x10, White);
      ssd1306_SetCursor(30, 0);
      ssd1306_WriteString(lcd_buf, Font_16x15, White);
      ssd1306_SetCursor(0, 24);
      ssd1306_WriteString("xboxresearch.com", Font_6x8, White);

      /* Flush changes */
      ssd1306_UpdateScreen();
    }
}

void Display_Print(char *message)
{
      ssd1306_Fill(Black);
      ssd1306_SetCursor(0, 0);
      ssd1306_WriteString(message, Font_16x24, White);
      ssd1306_UpdateScreen();
}
