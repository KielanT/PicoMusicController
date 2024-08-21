#include "SpotifyAPI.h"
#include <windows.h>
#include <iostream>

#include "Serial.h"


int main()
{
    SpotifyAPI spotify;
    spotify.Login();

    while (!spotify.GetAvaliableDevices())
    {
        std::cout << "No active device, press any key to try again" << std::endl;
        
        std::string input = "";
        std::cin >> input;
    }

    // TODO make sure app runs in background

    spotify.GetPlaybackState(); // Sets up variables

    // TODO check com port exists
    Serial serial("COM10", 9600);   
    // TODO Send song to pico
    while (true)
    {
        std::string str = serial.ReadLine();
        
        if (!str.empty())
        {
            spotify.GetPlaybackState();
            if (str == "p")
                spotify.Previous();
            else if (str == "pp")
                spotify.PlayPause();
            else if (str == "n")
                spotify.Next();
            else if (str == "s")
                spotify.Shuffle();
            else if (str[0] == 'v')
                spotify.SetVolume(str);
        }
    }
    return 0;
}
