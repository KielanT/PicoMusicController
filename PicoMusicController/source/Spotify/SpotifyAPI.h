#pragma once
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
#include <vector>
#include <condition_variable>
#include <mutex>
#include "curl/curl.h"
#include "json.hpp"

class SpotifyAPI
{
public:
	SpotifyAPI() = default; // Default constructor
	
	~SpotifyAPI() = default; // Default destructor

	SpotifyAPI(const SpotifyAPI&) = delete; // Copy constructor
	SpotifyAPI(SpotifyAPI&&) = delete; // Move constructor
	SpotifyAPI& operator=(const SpotifyAPI&) = delete; // Copy assignment operator
	SpotifyAPI& operator=(SpotifyAPI&&) = delete; // Move assignment operator

	void Login();
	void GenerateRefreshToken();


	bool GetAvaliableDevices();

	void PlayPause();

	void GetPlaybackState();

	[[maybe_unused]] bool GetCurrentTrack();

	void SetVolume(std::string& val); // TODO with pico

	void Shuffle();

	void Next();
	void Previous();

	void StartSongUpdateCheck(std::function<void(std::string&, std::string&)> func); // func allows the user to run code when the songs change
	
	void ActivateDevice();

	// HTTP Funcs
	[[nodiscard]] std::string SpotifyGET(const std::string& url);
	[[maybe_unused]] std::string SpotifyPUT(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields = "");
	[[maybe_unused]] std::string SpotifyPOST(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields = "");

private:
	void StartCountdown();

	void ReadCredentials(auto& id, auto& secret);
	void SaveCredentials();

	
public:

	std::string CurrentSong{ "" };
	std::string PreviousSong{ "" };
	std::string Artists{ "" };

private:

	std::string m_AccessToken{ "" };
	std::string m_RefreshToken{ "" };
	nlohmann::json m_AccessJson{};

	std::string m_CurrentDeviceID{ "" };

	bool m_IsPlaying{ false };
	bool m_ShuffleState{ false };
	
};
