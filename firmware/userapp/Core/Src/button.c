#include "button.h"

#define DELAY_TIME_MS 10

static uint32_t btnpress_duration = 0;
bool button_pressed = false;

bool Button_IsPressed(uint32_t *duration_ms) {
    *duration_ms = 0;
    if (HAL_GPIO_ReadPin(ENTER_BL_GPIO_Port, ENTER_BL_Pin) == GPIO_PIN_RESET) {
        // Measure duration of button press
        while (HAL_GPIO_ReadPin(ENTER_BL_GPIO_Port, ENTER_BL_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(DELAY_TIME_MS);
            *duration_ms += DELAY_TIME_MS;
        }
        return true;
    }
    return false;
}

void Button_Tick() {
    if (Button_IsPressed(&btnpress_duration)) {
        // Do something
    }
}