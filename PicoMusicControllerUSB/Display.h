#include "hardware/gpio.h"
#include "external/pico-ssd1306/ssd1306.h"
#include "external/pico-ssd1306/textRenderer/TextRenderer.h"
#include "external/pico-ssd1306/shapeRenderer/ShapeRenderer.h"

#define I2C_PORTx0 i2c0
#define I2C_SDAx0 0
#define I2C_SCLx0 1

enum class DisplayState : uint8_t
{
    On = 0,
    Off,
    UpdateScreen,
    None

};

static pico_ssd1306::SSD1306 InitDisplay()
{
    i2c_init(I2C_PORTx0, 1000000);

    gpio_set_function(I2C_SDAx0, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCLx0, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDAx0);
    gpio_pull_up(I2C_SCLx0);

    // My screen is 128x32 make sure yours is the correct size
    // The address 0x3C may also be different for you
    return pico_ssd1306::SSD1306(I2C_PORTx0, 0x3C,  pico_ssd1306::Size::W128xH32);
}