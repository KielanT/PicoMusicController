#include "App.h"
#include <iostream>

#include "SpotifyAPI.h"


App::~App()
{
    // TODO send data to the pico when app close
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
        std::string str = m_Serial.ReadLine();

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
                m_Spotify.SetVolume(str);
        }
    }
}

void App::SendSongDataToSerial(std::string& song, std::string& artists)
{
    std::string data = song + "|" + artists + "\n";

    m_Serial.WriteString(data);
}
