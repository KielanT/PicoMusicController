#include <stdio.h>
#include "pico/stdlib.h"

struct Button
{
    int Pin{-1};
    int32_t LastButtonPressTime{0};
    uint32_t DebounceDelay{250};
};

void InitButton(Button& btn, uint pin)
{
    btn.Pin = pin;

    gpio_init(btn.Pin);
    gpio_set_dir(btn.Pin, GPIO_IN);
    gpio_pull_down(btn.Pin);
}

bool IsButtonPressed(Button& btn)
{
    if(btn.Pin == -1) return false; // always false if pin is not set

    if (gpio_get(btn.Pin)) 
    {
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        if (currentTime - btn.LastButtonPressTime > btn.DebounceDelay) 
        {
            btn.LastButtonPressTime = currentTime;
            return true;
        }
    }
    

    return false;
}