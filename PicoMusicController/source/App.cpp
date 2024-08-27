#include "App.h"
#include <iostream>

#include "Spotify/SpotifyAPI.h"


App::~App()
{
    m_Serial.WriteString("off"); // Tell pico screen to be on
}

void App::Init()
{

    m_Spotify.Login();

    while (!m_Spotify.GetAvaliableDevices())
    {
        std::cout << "No active device, press any key to try again" << std::endl;

        std::string input = "";
        std::cin >> input;
    }

    m_Serial.WriteString("on"); // Tell pico screen to be on

    // TODO make sure app runs in background

    m_Spotify.GetPlaybackState(); // Sets up variables from GetPlaybackState()

    m_Spotify.StartSongUpdateCheck([this](std::string& song, std::string& artists) // Starts a new thread to check for updates 
        {
            SendSongDataToSerial(song, artists);
        }); 
}

void App::Run()
{
    while (true)
    {
        std::string str = m_Serial.ReadLine(); // Waits until recieved 

        if (!str.empty())
        {
            m_Spotify.GetPlaybackState();
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

void App::SendSongDataToSerial(std::string& song, std::string& artists)
{
    std::string data = "s" + song + "|" + artists + "\n"; //  s = songdata code, for the pico to respond

    m_Serial.WriteString(data);
}
