#include "button.h"

#define DELAY_TIME_MS 10
bool button_pressed = false;

bool Button_IsPressed() {
    return (HAL_GPIO_ReadPin(ENTER_BL_GPIO_Port, ENTER_BL_Pin) == GPIO_PIN_RESET);
}

uint32_t Button_IsPressedDuration() {
    uint32_t duration_ms = 0;
    if (Button_IsPressed()) {
        // Measure duration of button press
        while (Button_IsPressed()) {
            HAL_Delay(DELAY_TIME_MS);
            duration_ms += DELAY_TIME_MS;
        }
    }
    return duration_ms;
}