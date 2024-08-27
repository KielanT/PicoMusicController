#include "App.h"
#include <iostream>

#include "Spotify/SpotifyAPI.h"



App::~App()
{
    m_Serial.WriteString("off"); // Tell pico screen to be on
                                 // This does not work when the app is closed 
                                 // this is because of the infinite loops
                                 // multi threading is required
}

void App::Init()
{
    m_Spotify.Login();

    while (!m_Spotify.GetAvaliableDevices()) // Runs until there is an active device
    {
        std::cout << "No active device, press any key to try again" << std::endl;

        std::string input = "";
        std::cin >> input;
    }

    m_Serial.WriteString("on"); // Tell pico screen to be on

    m_Spotify.GetPlaybackState(); // Sets up variables from GetPlaybackState()

    m_Spotify.StartSongUpdateCheck([this](std::string& song, std::string& artists) // Starts a new thread to check for updates 
        {
            SendSongDataToSerial(song, artists);
        }); 
}


void App::Run()
{
    while (true) // Runs forever
    {
        std::string str = m_Serial.ReadLine(); // Waits until recieved some data

        if (!str.empty())
        {
            m_Spotify.GetPlaybackState(); // Makes sure that variables in SpotifyAPI are correct
                                          // Makes sure that there is an active device, if not it will 
                                          // activate the previous device

            // When recieved input from pico, using the codes below
            // run the correct spotify function
            if (str == "p")
                m_Spotify.Previous();
            else if (str == "pp")
                m_Spotify.PlayPause();
            else if (str == "n")
                m_Spotify.Next();
            else if (str == "s")
                m_Spotify.Shuffle();
            else if (str[0] == 'v')
            {
                str.erase(0, 1); // Removes the v 
                m_Spotify.SetVolume(str);
            }
        }
    }
}

void App::SendSongDataToSerial(std::string& song, std::string& artists) // Function to be called when the song changes
{
    std::string data = "s" + song + "|" + artists + "\n"; //  s = songdata code, for the pico to respond

    m_Serial.WriteString(data);
}
