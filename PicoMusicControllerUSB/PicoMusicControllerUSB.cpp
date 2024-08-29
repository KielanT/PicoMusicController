
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/multicore.h"

#include <string>

#include "cdc.h"
#include "Buttons.h"
#include "VolumeDial.h"
#include "Display.h"

// Set up the button pins
const uint LED_PIN{25};
const uint PREV_PIN{3};
const uint PAUSE_PLAY_PIN{7};
const uint NEXT_PIN{13};
const uint SHUFFLE_PIN{15};
const uint VOLUME_PIN{27};

bool gOnBoardLED{true};
DisplayState gDisplayState{DisplayState::None}; // Flags for updating the display

std::string gCurrentSong{""};
std::string gCurrentArtists{""};

Button gPrevButton;
Button gPausePlayButton;
Button gNextButton;
Button gShuffleButton;


void Core1Main()
{
    pico_ssd1306::SSD1306 display{InitDisplay()};
    display.clear();

    while(1)
    {
        if(gDisplayState == DisplayState::None) continue; // Do not run rest off the code if there is no flag


        if(gDisplayState == DisplayState::UpdateScreen)
        {
            // Write to display
            display.clear();
            drawText(&display, font_8x8, gCurrentSong.c_str(), 0, 0);
            drawText(&display, font_8x8, gCurrentArtists.c_str(), 0, 20);
            display.sendBuffer();
        
            gDisplayState == DisplayState::None; // Reset, wait for new flag
        }
        else if(gDisplayState == DisplayState::Off)
        {
            display.turnOff();
            gDisplayState == DisplayState::None; // Reset, wait for new flag
        }
        else if(gDisplayState == DisplayState::On)
        {
            display.turnOn();
            gDisplayState == DisplayState::None; // Reset, wait for new flag
        }
    }
}

int main()
{ 
    uint16_t volume{1000}; // Could set the value to the current spotify one but
                            // the audio will simply overwrite the spotify one either way

    stdio_init_all();

    multicore_launch_core1(Core1Main); // Run the display code on the other core

    // Init tiny usb
    board_init();
    tud_init(BOARD_TUD_RHPORT);

    // Init LED pins
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, gOnBoardLED ? 1 : 0);

    // Init voloume idal see VolumeDial.h
    InitVolumeDial(VOLUME_PIN);

    // Init the buttons see Buttons.h
    InitButton(gPrevButton, PREV_PIN);
    InitButton(gPausePlayButton, PAUSE_PLAY_PIN);
    InitButton(gNextButton, NEXT_PIN);
    InitButton(gShuffleButton, SHUFFLE_PIN);
    
    std::string dataRecieved{""};
    while (true) 
    {
        dataRecieved.clear(); // Clear data recieved read to use on next iteration of the loop

        tud_task(); // Run the tinyusb task
        cdc_task(dataRecieved); // Get any data that the desktop app sends

        if(!dataRecieved.empty()) // Only runs the code in the if statement if the any data has recieved
        {
            if(dataRecieved[0] == 's') // Sends the song data to the screen
            {
                dataRecieved.erase(0, 1); // remove s
                size_t pos = dataRecieved.find('|');
                if(pos != std::string::npos)
                {
                    gCurrentSong = dataRecieved.substr(0, pos);
                    gCurrentArtists = dataRecieved.substr(pos + 1);
                    gDisplayState = DisplayState::UpdateScreen;
                }
            }
            else if(dataRecieved == "on") // Sets flag to turn on display
            {
                gDisplayState = DisplayState::On;
            }
            else if(dataRecieved == "off") // Sets the flag to turn off display
            {
                gDisplayState = DisplayState::Off;
            }
        }

        if(gDisplayState == DisplayState::Off) continue;    // When the display is off we do not need to send any data
                                                            // to the app, since this means that the app is not running

        if(VolumeChange(volume))
        {
            std::string message = "v" + std::to_string(volume) + "\n";// Creates a message 
            tud_cdc_write(message.c_str(), message.length()); // Writes the message to be sent over serial
            tud_cdc_write_flush(); // Sends messages over serial
        }

        if (IsButtonPressed(gPrevButton)) 
        {
            const char* message = "p\n";
            tud_cdc_write(message, strlen(message));
            tud_cdc_write_flush();
        }
        else if (IsButtonPressed(gPausePlayButton)) 
        {
            const char* message = "pp\n";
            tud_cdc_write(message, strlen(message));
            tud_cdc_write_flush();
        }
        else if (IsButtonPressed(gNextButton)) 
        {
            const char* message = "n\n";
            tud_cdc_write(message, strlen(message));
            tud_cdc_write_flush();
        }
        else if (IsButtonPressed(gShuffleButton)) 
        {
            const char* message = "s\n";
            tud_cdc_write(message, strlen(message));
            tud_cdc_write_flush();
        }
    }
}


