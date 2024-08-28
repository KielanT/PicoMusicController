#include "SpotifyCalls.h"
#include <fstream>
#include <thread>

#include "AuthServer.h"

#include "CPPify.h"

void SpotifyCalls::Login()
{
	// Gets the client ID and Secret
	// See the spotify api Create an app in the link below
	// https://developer.spotify.com/documentation/web-api/tutorials/getting-started
	std::string id{ "" };
	std::string secret{ "" };
	ReadCredentials(id, secret);

	// If ReadCredentials() does not have a refresh token then 
	// the user must login to spotify to authenticate
	// if the the ReadCredentials() does then generate a new token
	// generating a new refresh token stops the need to login every hour 
	// and every time the app starts
	if (!m_RefreshToken.empty())
	{
		GenerateRefreshToken();
	}
	else
	{
		// Starts a simple server to login in to spotify and recieve
		// an access token and refresh token
		AuthServer server; 
		server.Start(id, secret);

		m_AccessJson = nlohmann::json::parse(server.AccessToken);

		if (m_AccessJson.contains("access_token"))
			m_AccessToken = m_AccessJson["access_token"];
		if (m_AccessJson.contains("refresh_token"))
			m_RefreshToken = m_AccessJson["refresh_token"];

		// Saves the refresh token to obtain another one 
		// every hour or when loading this again
		SaveCredentials();
	}

	StartCountdown(); // Runs GenerateRefreshToken() every hour otherwise
	// the app stops working

}

bool SpotifyCalls::GetAvaliableDevices()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);

	std::string response = CPPify::SpotifyGET("https://api.spotify.com/v1/me/player/devices", headers);
	bool isValid = false;

	if (!response.empty()) // If there is a response
	{ 
		// Converts the response string to a properly formatted json file
		nlohmann::json json = nlohmann::json::parse(response);

		if (json.contains("devices")) // Search for active devices
		{
			auto devices = json["devices"];
			for (const auto& device : devices)
			{
				m_CurrentDeviceID = device["id"]; // Only sets one, you should store all in an array
												  // but this is fine for my use case
				if (device["is_active"] == true)
				{
					isValid = true;
				}
				else
					isValid = false;
			}
		}
	}

	return isValid;
}

void SpotifyCalls::PlayPause()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	// If playing set false
	// if not playing set to true

	if (m_IsPlaying)
	{
		CPPify::SpotifyPUT("https://api.spotify.com/v1/me/player/pause", headers);
	}
	else if (!m_IsPlaying)
	{
		CPPify::SpotifyPUT("https://api.spotify.com/v1/me/player/play", headers);
	}
}

void SpotifyCalls::GetPlaybackState()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);

	std::string response = CPPify::SpotifyGET("https://api.spotify.com/v1/me/player?market=GB", headers); // TODO select markets

	// If there is not a response and there has been an active device
	// reactivate the device
	// then return out the function since we cannot do any of the following code
	// with an empty string
	if (!m_CurrentDeviceID.empty() && response.empty())
	{
		ActivateDevice();

		return;
	}

	nlohmann::json json = nlohmann::json::parse(response);

	if (json.contains("device") && json["device"].contains("id"))
	{
		m_CurrentDeviceID = json["device"]["id"];
	}

	if (json.contains("shuffle_state"))
		m_ShuffleState = json["shuffle_state"];

	if (json.contains("is_playing"))
		m_IsPlaying = json["is_playing"];
}

bool SpotifyCalls::GetCurrentTrack()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);

	std::string response = CPPify::SpotifyGET("https://api.spotify.com/v1/me/player/currently-playing?market=GB", headers); // TODO set markets
	bool isValid = !response.empty(); // Empty = false. Not empty = true

	if (isValid)
	{
		nlohmann::json json = nlohmann::json::parse(response);

		if (json.contains("item") && json["item"].contains("name"))
		{
			CurrentSong = json["item"]["name"]; // Set the current track name

			Artists.clear(); // Clears the previous artist and repopulates 
			auto artists = json["item"]["artists"];
			for (const auto& artist : artists)
			{
				if (!Artists.empty())
					Artists += ", ";

				Artists += artist["name"];
			}
		}
	}

	return isValid;
}

void SpotifyCalls::SetVolume(std::string& val)
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	// Sets the volume by passing in the new volume to the url
	CPPify::SpotifyPUT("https://api.spotify.com/v1/me/player/volume?volume_percent=" + val, headers);
}

void SpotifyCalls::Shuffle()
{
	// Updating shuffle is to be very slow / sometimes unresponsive
	// Works fine when setting breakpoints

	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	if (m_ShuffleState)
	{
		CPPify::SpotifyPUT("https://api.spotify.com/v1/me/player/shuffle?state=false", headers);
	}
	else
	{
		CPPify::SpotifyPUT("https://api.spotify.com/v1/me/player/shuffle?state=true", headers);
	}
}

void SpotifyCalls::Next()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	CPPify::SpotifyPOST("https://api.spotify.com/v1/me/player/next", headers);
}

void SpotifyCalls::Previous()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	CPPify::SpotifyPOST("https://api.spotify.com/v1/me/player/previous", headers);
}

void SpotifyCalls::StartSongUpdateCheck(std::function<void(std::string&, std::string&)> func)
{
	auto CheckUpdateFunc = [this, func]() // Lambda function
		{
			while (true)
			{
				if (GetCurrentTrack()) // Returns false if in active
				{

					if (CurrentSong != PreviousSong)
					{
						func(CurrentSong, Artists); // Calls the function the user defines for when there is a track change

						PreviousSong = CurrentSong;
					}
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::seconds(30));	// Checks every 30 seconds if the device is no long active
					// Would be better to pause the thread and unpause when the device is active

				}
			}

		};

	// Creates a new thread to check every few seconds if the song as changed
	std::thread SongUpdateThread(CheckUpdateFunc);
	SongUpdateThread.detach(); // Makes sure the thread runs independtly from the main thread
							   // otherwise threads wait for each other, which we do not want in this case
}

void SpotifyCalls::ActivateDevice()
{
	// Data to send to spotify
	// The device_id is the device we want to active
	// We can tell spotify to resume with play
	nlohmann::json json_payload;
	json_payload["device_ids"] = { m_CurrentDeviceID };
	json_payload["play"] = true;
	m_IsPlaying = true;
	std::string jsonData = json_payload.dump();

	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Type: application/json");

	CPPify::SpotifyPUT("https://api.spotify.com/v1/me/player", headers, jsonData); 
}

void SpotifyCalls::ReadCredentials(auto& id, auto& secret)
{
	// File is set outside of the git folder for security
	// Use better methods when needed
	std::ifstream file;
	file.open("../../SpotifyDevCredentials.txt");
	getline(file, id);
	getline(file, secret);
	getline(file, m_RefreshToken);

	file.close();
}

void SpotifyCalls::SaveCredentials()
{
	// We only want to append the last line in the file,
	// we know that is where the refresh token is,
	// this saves us from looping all the way through a file
	std::ofstream file("../../SpotifyDevCredentials.txt", std::ios::app);
	file << "\n" << m_RefreshToken;

	file.close();
}

void SpotifyCalls::GenerateRefreshToken()
{

	std::string id{ "" };
	std::string secret{ "" };
	ReadCredentials(id, secret);

	CURL* curl = curl_easy_init();


	// Post field required to get a new token
	// We use curl here to make sure that the postfields
	// are legal
	std::string postFields = "grant_type=refresh_token&refresh_token=" + std::string(curl_easy_escape(curl, m_RefreshToken.c_str(), m_RefreshToken.length())) +
		"&client_id=" + curl_easy_escape(curl, id.c_str(), id.length()) +
		"&client_secret=" + curl_easy_escape(curl, secret.c_str(), secret.length());

	curl_easy_cleanup(curl);

	std::vector<std::string> headers;
	headers.push_back("Content-Type: application/x-www-form-urlencoded");

	std::string response = CPPify::SpotifyPOST("https://accounts.spotify.com/api/token", headers, postFields); // Sends a post request

	if (!response.empty()) // If response is not empty 
	{
		m_AccessJson = nlohmann::json::parse(response); // Converts response string to a json object

		if (m_AccessJson.contains("access_token")) // If the access token exists set it 
			m_AccessToken = m_AccessJson["access_token"];

		if (m_AccessJson.contains("refresh_token")) // If the refresh token exists set it
		{
			m_RefreshToken = m_AccessJson["refresh_token"];
			SaveCredentials(); // Save refresh token to file, this means that the refresh token
			// can be generated next time the app starts instead of 
			// having to login to spotify every time 
		}
	}

}

void SpotifyCalls::StartCountdown()
{
	// This function call generate refresh token every hour
	// A access token expires every hour, so we need to use refresh token
	// to get a new one
	auto countDownFunc = [this]()
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::hours(1));
				GenerateRefreshToken();
			}

		};
	std::thread countdownThread(countDownFunc);
	countdownThread.detach();
}
