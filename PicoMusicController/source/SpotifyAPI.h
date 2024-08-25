#pragma once
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
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

	void GetCurrentTrack(CURL* curl);

	void SetVolume(std::string& val); // TODO with pico

	void Shuffle();

	void Next();
	void Previous();

	void StartSongUpdateCheck(std::function<void(std::string&, std::string&)> func); // func allows the user to run code when the songs change
	

	void ActivateDevice();
private:
	void StartCountdown();

	void ReadCredentials(auto& id, auto& secret);
	void SaveCredentials();

	[[nodiscard]] std::string SpotifyGet(const std::string& url, const char* header = "");
	//void SpotifyPut(const std::string& url, const char* header = "");
	//void SpotifyPost(const std::string& url, const char* header = "");
public:

	std::string CurrentSong{ "" };
	std::string PreviousSong{ "" };
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
