#include "App.h"
#include <iostream>

#include "Spotify/CPPify.h"



App::~App()
{
    m_Serial->WriteString("off"); // Tell pico screen to be on
                                 // This does not work when the app is closed 
                                 // this is because of the infinite loops
                                 // multi threading is required

    m_Serial->Close();

   if (m_SerialThread.joinable())
   {
       m_SerialThread.join();  // Wait for the serial thread to finish
   }
   
}

void App::Init(const std::string& port)
{
    m_IsAppRunning.store(true);
    m_Window = std::make_unique<WindowsWindow>(m_IsAppRunning);


    m_Spotify = std::make_unique<SpotifyCalls>(m_IsAppRunning);
    m_Spotify->Login();

    while (!m_Spotify->GetAvaliableDevices()) // Runs until there is an active device
    {
#ifdef _DEBUG
        std::cout << "No active device, press any key to try again" << std::endl;

        std::string input = "";
        std::cin >> input;
#endif // _DEBUG

#ifdef _RELEASE
        int result = MessageBox(m_Window->Handle, L"Make sure Spotify is playing! Try again?", L"No Active device found!", MB_YESNO | MB_ICONERROR);
        if (result == IDNO)
        {
            SendMessage(m_Window->Handle, WM_DESTROY, 0, 0);
            return;
        }

#endif // _RELEASE


    }

    m_Serial = std::make_unique<Serial>(port, 9600, m_IsAppRunning); // Starts port on COM10, UPDATE for your port

    m_Serial->WriteString("on"); // Tell pico screen to be on

    m_Spotify->GetPlaybackState(); // Sets up variables from GetPlaybackState()

    m_Spotify->StartSongUpdateCheck([this](std::string& song, std::string& artists) // Starts a new thread to check for updates 
        {
            SendSongDataToSerial(song, artists);
        }); 

    m_SerialThread = std::thread(&App::SerialThread, this);
}


void App::Run()
{
    int count = 0;
    while (m_IsAppRunning.load()) // Runs forever
    {
        m_Window->Update();
        count++;
    }
}

void App::SerialThread()
{
    while (m_IsAppRunning.load())
    {
        std::string str = m_Serial->ReadLine(); // Waits until recieved some data

        if (!str.empty() /*&& str[0] != 'v'*/)
        {
            m_Spotify->GetPlaybackState(); // Makes sure that variables in SpotifyAPI are correct
            // Makes sure that there is an active device, if not it will 
            // activate the previous device

            // When recieved input from pico, using the codes below
            // run the correct spotify function
            if (str == "p")
                m_Spotify->Previous();
            else if (str == "pp")
                m_Spotify->PlayPause();
            else if (str == "n")
                m_Spotify->Next();
            else if (str == "s")
                m_Spotify->Shuffle();
            else if (str[0] == 'v')
            {
                str.erase(0, 1); // Removes the v 
                m_Spotify->SetVolume(str);
            }
        }
    }


}

void App::SendSongDataToSerial(std::string& song, std::string& artists) // Function to be called when the song changes
{
    std::string data = "s" + song + "|" + artists + "\n"; //  s = songdata code, for the pico to respond

    m_Serial->WriteString(data);
}
