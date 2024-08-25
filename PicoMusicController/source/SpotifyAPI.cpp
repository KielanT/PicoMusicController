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
	std::string id{ "" };
	std::string secret{ "" };
	ReadCredentials(id, secret);

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

	StartCountdown();
	
}

void SpotifyAPI::GenerateRefreshToken()
{

	std::string id{ "" };
	std::string secret{ "" };
	ReadCredentials(id, secret);

	CURL* curl = curl_easy_init();


	// Include client_id, client_secret, and refresh_token in POST fields
	std::string post_fields = "grant_type=refresh_token&refresh_token=" + std::string(curl_easy_escape(curl, m_RefreshToken.c_str(), m_RefreshToken.length())) +
		"&client_id=" + curl_easy_escape(curl, id.c_str(), id.length()) +
		"&client_secret=" + curl_easy_escape(curl, secret.c_str(), secret.length());

	curl_easy_cleanup(curl);

	std::vector<std::string> headers;
	headers.push_back("Content-Type: application/x-www-form-urlencoded");

	std::string response = SpotifyPOST("https://accounts.spotify.com/api/token", headers, post_fields);

	if (!response.empty())
	{
		m_AccessJson = nlohmann::json::parse(response);

		if (m_AccessJson.contains("access_token"))
			m_AccessToken = m_AccessJson["access_token"];

		if (m_AccessJson.contains("refresh_token"))
		{
			m_RefreshToken = m_AccessJson["refresh_token"];
			SaveCredentials();
		}
	}

}


bool SpotifyAPI::GetAvaliableDevices()
{
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

void SpotifyAPI::GetCurrentTrack(CURL* curl)
{
	std::string response = SpotifyGET("https://api.spotify.com/v1/me/player/currently-playing?market=GB");


	if (!response.empty())
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
}


void SpotifyAPI::SetVolume(std::string& val)
{
	// TODO send and recieve json instead
	// TODO pico only send once value as stopped changing after a time otherwise we constantly send value on the tinest of changes
	val.erase(0, 1); // Removes the v 

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
	CURL* UpdateThreadCurl = curl_easy_init();

	auto CheckUpdateFunc = [this, UpdateThreadCurl, func]()
		{
			while (true)
			{
				GetCurrentTrack(UpdateThreadCurl);

				if (CurrentSong != PreviousSong)
				{
					func(CurrentSong, Artists);

					PreviousSong = CurrentSong;
				}

				std::this_thread::sleep_for(std::chrono::seconds(2));
			}

			curl_easy_cleanup(UpdateThreadCurl);

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

	std::string response = SpotifyPUT("https://api.spotify.com/v1/me/player", headers, jsonData);

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


