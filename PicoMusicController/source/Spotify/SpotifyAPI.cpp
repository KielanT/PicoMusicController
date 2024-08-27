#include "SpotifyAPI.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>

#include "crow/utility.h"
#include "AuthServer.h"


void SpotifyAPI::Login()
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
		AuthServer server;
		server.Start(id, secret);

		m_AccessJson = nlohmann::json::parse(server.AccessToken);

		if (m_AccessJson.contains("access_token"))
			m_AccessToken = m_AccessJson["access_token"];
		if (m_AccessJson.contains("refresh_token"))
			m_RefreshToken = m_AccessJson["refresh_token"];

		SaveCredentials();
	}

	StartCountdown(); // Runs GenerateRefreshToken() every hour otherwise
					  // the app stops working
	
}

void SpotifyAPI::GenerateRefreshToken()
{

	std::string id{ "" };
	std::string secret{ "" };
	ReadCredentials(id, secret); 

	CURL* curl = curl_easy_init();


	// Post field required to get a new token
	std::string postFields = "grant_type=refresh_token&refresh_token=" + std::string(curl_easy_escape(curl, m_RefreshToken.c_str(), m_RefreshToken.length())) +
		"&client_id=" + curl_easy_escape(curl, id.c_str(), id.length()) +
		"&client_secret=" + curl_easy_escape(curl, secret.c_str(), secret.length());

	curl_easy_cleanup(curl);

	std::vector<std::string> headers;
	headers.push_back("Content-Type: application/x-www-form-urlencoded");

	std::string response = SpotifyPOST("https://accounts.spotify.com/api/token", headers, postFields); // Sends a post request

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


bool SpotifyAPI::GetAvaliableDevices()
{
	// If there are any active devices returns true
	std::string response = SpotifyGET("https://api.spotify.com/v1/me/player/devices");
	bool isValid = false; 

	if (!response.empty())
	{
		nlohmann::json json = nlohmann::json::parse(response);
		
		if(json.contains("devices"))
		{
			auto devices = json["devices"];
			for (const auto& device : devices)
			{
				m_CurrentDeviceID = device["id"];
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

void SpotifyAPI::PlayPause()
{
	std::string response{ "" };
	
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	if (m_IsPlaying)
	{
		response = SpotifyPUT("https://api.spotify.com/v1/me/player/pause", headers);
	}
	else if(!m_IsPlaying)
	{
		response = SpotifyPUT("https://api.spotify.com/v1/me/player/play", headers);
	}
}

void SpotifyAPI::GetPlaybackState()
{
	std::string response = SpotifyGET("https://api.spotify.com/v1/me/player?market=GB"); // TODO select markets
	 
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

bool SpotifyAPI::GetCurrentTrack()
{
	
	std::string response = SpotifyGET("https://api.spotify.com/v1/me/player/currently-playing?market=GB"); // TODO set markets
	bool isValid = !response.empty(); // Empty = false. Not empty = true

	if (isValid)
	{
		nlohmann::json json = nlohmann::json::parse(response);

		if (json.contains("item") && json["item"].contains("name"))
		{
			CurrentSong = json["item"]["name"];

			Artists.clear();
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


void SpotifyAPI::SetVolume(std::string& val)
{

	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	std::string response = SpotifyPUT("https://api.spotify.com/v1/me/player/volume?volume_percent=" + val, headers);
}

void SpotifyAPI::Shuffle()
{
	// Updating shuffle is to be very slow / sometimes unresponsive
	// Works fine when setting breakpoints on the SpotifyPUT functions calls in this function

	std::string response{ "" };

	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	if (m_ShuffleState)
	{
		response = SpotifyPUT("https://api.spotify.com/v1/me/player/shuffle?state=false", headers);
	}
	else
	{
		response = SpotifyPUT("https://api.spotify.com/v1/me/player/shuffle?state=true", headers);
	}
}

void SpotifyAPI::Next()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");

	std::string response = SpotifyPOST("https://api.spotify.com/v1/me/player/next", headers);
}

void SpotifyAPI::Previous()
{
	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Length: 0");
	std::string response = SpotifyPOST("https://api.spotify.com/v1/me/player/previous", headers);
}


void SpotifyAPI::StartSongUpdateCheck(std::function<void(std::string&, std::string&)> func)
{

	auto CheckUpdateFunc = [this, func]()
		{
			while (true)
			{
				if (GetCurrentTrack()) // Returns false if in active
				{

					if (CurrentSong != PreviousSong)
					{
						func(CurrentSong, Artists);

						PreviousSong = CurrentSong;
					}
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::seconds(30));	// TODO Checks every 30 seconds if the device is no long active
																			// Would be better to pause the thread and unpause when the device is active
					
				}
			}

		};

	std::thread SongUpdateThread(CheckUpdateFunc);
	SongUpdateThread.detach();

}

void SpotifyAPI::StartCountdown()
{
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


void SpotifyAPI::ActivateDevice()
{
	nlohmann::json json_payload;
	json_payload["device_ids"] = { m_CurrentDeviceID };
	json_payload["play"] = true;
	m_IsPlaying = true;
	std::string jsonData = json_payload.dump();

	std::vector<std::string> headers;
	headers.push_back("Authorization: Bearer " + m_AccessToken);
	headers.push_back("Content-Type: application/json");

	std::string response = SpotifyPUT("https://api.spotify.com/v1/me/player", headers, jsonData); // Calls transfer playback

}

void SpotifyAPI::ReadCredentials(auto& id, auto& secret)
{
	std::ifstream file;
	file.open("../../SpotifyDevCredentials.txt");
	getline(file, id);
	getline(file, secret);
	getline(file, m_RefreshToken);

	file.close();
}

void SpotifyAPI::SaveCredentials()
{
	std::ofstream file("../../SpotifyDevCredentials.txt", std::ios::app);
	file << "\n" << m_RefreshToken;

	file.close();
}

std::string SpotifyAPI::SpotifyGET(const std::string& url)
{
	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

	std::string response_data{ "" };
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		std::cerr << "GET request failed: " << curl_easy_strerror(res) << std::endl;
	}
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return response_data;
}

std::string SpotifyAPI::SpotifyPUT(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields)
{
	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	struct curl_slist* headerList = NULL;
	for (const auto& header : headers)
	{
		headerList = curl_slist_append(headerList, header.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	
	if (!postfields.empty())
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());
	}

	std::string response_data{ "" };
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		std::cerr << "PUT request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headerList);
	curl_easy_cleanup(curl);

	return response_data;
}

std::string SpotifyAPI::SpotifyPOST(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields)
{
	CURL* curl = curl_easy_init();


	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());


	struct curl_slist* headerList = NULL;
	for (const auto& header : headers)
	{
		headerList = curl_slist_append(headerList, header.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

	if (!postfields.empty())
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());
	}

	std::string response_data{ "" };
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		std::cerr << "POST request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headerList);
	curl_easy_cleanup(curl);

	return response_data;
}


