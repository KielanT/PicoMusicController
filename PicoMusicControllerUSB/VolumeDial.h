#include <string>
#include "hardware/adc.h"

const uint THRESHOLD = 4;   

void InitVolumeDial(uint pin)
{
    adc_init();
    adc_gpio_init(pin);
    adc_select_input(1);
}

bool VolumeChange(uint16_t& volume)
{
    static uint16_t prevVal{1000};
    const uint bar_width = 100;
    const uint adc_max = (1 << 12) - 1;
    uint16_t val = adc_read() * bar_width / adc_max;

    if(abs(val - prevVal) >= THRESHOLD)
    {   
        volume = val;
        prevVal = val;
        return true;  
    }

        return false;
}