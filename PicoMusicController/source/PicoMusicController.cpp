#include "SpotifyAPI.h"
#include <windows.h>
#include <iostream>
#include <conio.h> // Not standard, will not work for all compilers

#include "Serial.h"


int main()
{


    SpotifyAPI spotify;
    spotify.Login();

    while (!spotify.GetAvaliableDevices())
    {
        std::cout << "No active device, press any key to try again" << std::endl;
        _getch();
    }

   
    spotify.GetPlaybackState();

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
        }
    }
    return 0;
}



/* SpotifyAPI spotify;

  spotify.Login();

  while(!spotify.GetAvaliableDevices())
  {
      std::cout << "No active device, press any key to try again" << std::endl;
      _getch();
  }

  spotify.GetCurrentTrack();
  while (true)
  {
      std::string input{ "" };
      std::cout << " ____              _   _  __          ____                      _      " << std::endl;
      std::cout << "/ ___| _ __   ___ | |_(_)/ _|_   _   / ___|___  _ __  ___  ___ | | ___ " << std::endl;
      std::cout << "\\___ \\| '_ \\ / _ \\| __| | |_| | | | | |   / _ \\| '_ \\/ __|/ _ \\| |/ _ \\" << std::endl;
      std::cout << " ___) | |_) | (_) | |_| |  _| |_| | | |__| (_) | | | \\__ \\ (_) | |  __/" << std::endl;
      std::cout << "|____/| .__/ \\___/ \\__|_|_|  \\__, |  \\____\\___/|_| |_|___/\\___/|_|\\___|" << std::endl;
      std::cout << "      |_|                    |___/                                     " << std::endl;

      std::cout << "Now Playing: " << std::endl;
      std::cout << spotify.CurrentSong << std::endl;
      std::cout << spotify.Artists << std::endl;

      std::cout << std::endl;
      std::cout << "p: Pause/Resume" << std::endl;
      std::cout << "d: Next Song" << std::endl;
      std::cout << "a: Prev Song" << std::endl;
      std::cout << "s: Shuffle" << std::endl;

      std::cin >> input;
      spotify.GetCurrentTrack();
      if (input == "p")
      {
          spotify.PlayPause();
      }
      if (input == "d")
      {
          spotify.Next();
      }
      if (input == "a")
      {
          spotify.Previous();
      }
      if (input == "s")
      {
          spotify.Shuffle();
      }

      system("cls");
  }*/