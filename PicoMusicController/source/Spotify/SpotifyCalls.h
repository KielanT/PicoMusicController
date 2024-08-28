#pragma once
#include <string>
#include <functional>

#include "json.hpp"


class SpotifyCalls
{
public:
	SpotifyCalls() = default; // Default constructor
	~SpotifyCalls() = default; // Default destructor

	SpotifyCalls(const SpotifyCalls&) = delete; // Copy constructor
	SpotifyCalls(SpotifyCalls&&) = delete; // Move constructor
	SpotifyCalls& operator=(const SpotifyCalls&) = delete; // Copy assignment operator
	SpotifyCalls& operator=(SpotifyCalls&&) = delete; // Move assignment operator

	void Login();

	bool GetAvaliableDevices();

	void PlayPause();

	void GetPlaybackState();

	[[maybe_unused]] bool GetCurrentTrack();

	void SetVolume(std::string& val);

	void Shuffle();

	void Next();

	void Previous();

	// func allows the user to run code when the songs change
	void StartSongUpdateCheck(std::function<void(std::string&, std::string&)> func);

	void ActivateDevice();

private:
	void ReadCredentials(auto& id, auto& secret);
	void SaveCredentials();

	void GenerateRefreshToken();

	// Calls generate refresh token every hour which is required for a long 
	// running app
	void StartCountdown();

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

