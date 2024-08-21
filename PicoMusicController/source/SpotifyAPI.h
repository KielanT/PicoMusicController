#pragma once
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include "curl/curl.h"
#include "json.hpp"

class SpotifyAPI
{
public:
	SpotifyAPI(); // Default constructor
	~SpotifyAPI(); // Default destructor

	SpotifyAPI(const SpotifyAPI&) = delete; // Copy constructor
	SpotifyAPI(SpotifyAPI&&) = delete; // Move constructor
	SpotifyAPI& operator=(const SpotifyAPI&) = delete; // Copy assignment operator
	SpotifyAPI& operator=(SpotifyAPI&&) = delete; // Move assignment operator

	void Login();
	void GenerateRefreshToken();


	bool GetAvaliableDevices();

	void PlayPause();

	void GetPlaybackState(); // TODO call again with a timer when song is over

	//void SetVolume(); // TODO with pico

	void Shuffle();

	void Next();
	void Previous();

	void ActivateDevice();
private:
	void StartCountdown();


	void ReadCredentials(auto& id, auto& secret);
	void SaveCredentials();

	//void SportifyGet(const std::string& url, const char* header = "");
	//void SportifyPut(const std::string& url, const char* header = "");
	//void SportifyPost(const std::string& url, const char* header = "");
public:

	std::string CurrentSong{ "" };
	std::string Artists{ "" };


private:
	CURL* m_Curl;

	std::string m_AccessToken{ "" };
	std::string m_RefreshToken{ "" };
	nlohmann::json m_AccessJson{};

	std::string m_CurrentDeviceID{ "" };

	bool m_IsPlaying{ false };
	bool m_ShuffleState{ false };
	
};
