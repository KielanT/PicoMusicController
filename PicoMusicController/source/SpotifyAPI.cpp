#include "SpotifyAPI.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>

#include "crow/utility.h"
#include "AuthServer.h"

SpotifyAPI::SpotifyAPI()
{
	m_Curl = curl_easy_init();
}

SpotifyAPI::~SpotifyAPI()
{
	curl_easy_cleanup(m_Curl);
}

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

	curl_easy_reset(m_Curl);

	// Include client_id, client_secret, and refresh_token in POST fields
	std::string post_fields = "grant_type=refresh_token&refresh_token=" + std::string(curl_easy_escape(m_Curl, m_RefreshToken.c_str(), m_RefreshToken.length())) +
		"&client_id=" + curl_easy_escape(m_Curl, id.c_str(), id.length()) +
		"&client_secret=" + curl_easy_escape(m_Curl, secret.c_str(), secret.length());

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
	curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS, post_fields.c_str());
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);
	if (res == CURLE_OK)
	{
		m_AccessJson = nlohmann::json::parse(response_data);

		if (m_AccessJson.contains("access_token"))
			m_AccessToken = m_AccessJson["access_token"];

		if (m_AccessJson.contains("refresh_token"))
		{
			m_RefreshToken = m_AccessJson["refresh_token"];
			SaveCredentials();
		}
	}
	else
	{
		std::cerr << "Refresh request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headers);


}


bool SpotifyAPI::GetAvaliableDevices()
{
	bool isValid = false;
	curl_easy_reset(m_Curl);

	curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/devices");

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_HTTPGET, 1L);

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);

	if (res == CURLE_OK)
	{
		nlohmann::json json = nlohmann::json::parse(response_data);
		
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
	else
	{
		isValid = false;
		std::cerr << "request failed: " << curl_easy_strerror(res) << std::endl;
	}
	curl_slist_free_all(headers);

	return isValid;
}

void SpotifyAPI::PlayPause()
{
	curl_easy_reset(m_Curl);

	if (m_IsPlaying)
	{
		curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/pause");
	}
	else
	{
		curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/play");
	}
	
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	headers = curl_slist_append(headers, "Content-Length: 0");
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, "PUT");

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);

	if (res != CURLE_OK)
	{
		std::cerr << "Pause request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headers);
}

void SpotifyAPI::GetPlaybackState()
{
	curl_easy_reset(m_Curl);

	curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player?market=GB");  

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_HTTPGET, 1L);

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);

	if (res == CURLE_OK)
	{
		if (!m_CurrentDeviceID.empty() && response_data.empty())
		{
			curl_slist_free_all(headers);
			ActivateDevice();

			// TODO try again with this function?

			return;
		}

		nlohmann::json json = nlohmann::json::parse(response_data);
		
		if (json.contains("device") && json["device"].contains("id"))
		{
			m_CurrentDeviceID = json["device"]["id"];
		}
		

		if(json.contains("shuffle_state"))
			m_ShuffleState = json["shuffle_state"]; 

		if (json.contains("is_playing"))
			m_IsPlaying = json["is_playing"];

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
	else
	{
		std::cerr << "request failed: " << curl_easy_strerror(res) << std::endl;
	}
	curl_slist_free_all(headers);
}

void SpotifyAPI::SetVolume(std::string& val)
{
	// TODO send and recieve json instead
	// TODO pico only send once value as stopped changing after a time otherwise we constantly send value on the tinest of changes
	val.erase(0, 1); // Remove v 

	std::string url = "https://api.spotify.com/v1/me/player/volume?volume_percent=" + val;


	curl_easy_reset(m_Curl);

	curl_easy_setopt(m_Curl, CURLOPT_URL, url.c_str());

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	headers = curl_slist_append(headers, "Content-Length: 0");
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, "PUT");

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);

	if (res != CURLE_OK)
	{
		std::cerr << " request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headers);
}

void SpotifyAPI::Shuffle()
{
	m_ShuffleState = !m_ShuffleState;

	curl_easy_reset(m_Curl);

	if (m_ShuffleState)
	{
		curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/shuffle?state=true");
	}
	else
	{
		curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/shuffle?state=false");
	}

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	headers = curl_slist_append(headers, "Content-Length: 0");
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, "PUT");

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);

	if (res != CURLE_OK)
	{
		std::cerr << " request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headers);
}

void SpotifyAPI::Next()
{
	curl_easy_reset(m_Curl);

	curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/next");


	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	headers = curl_slist_append(headers, "Content-Length: 0");
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, "POST");

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);

	if (res != CURLE_OK)
	{
		std::cerr << "Next request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headers);
}

void SpotifyAPI::Previous()
{
	curl_easy_reset(m_Curl);

	curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/previous");


	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	headers = curl_slist_append(headers, "Content-Length: 0");
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, "POST");

	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);

	if (res != CURLE_OK)
	{
		std::cerr << "Pause request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headers);
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
	curl_easy_reset(m_Curl);

	curl_easy_setopt(m_Curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player");

	nlohmann::json json_payload;
	json_payload["device_ids"] = { m_CurrentDeviceID };
	json_payload["play"] = true;
	m_IsPlaying = true;
	std::string jsonData = json_payload.dump();

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, "PUT");

	curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS, jsonData.c_str());
	std::string response_data;
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(m_Curl);
	if (res != CURLE_OK)
	{
		std::cerr << "Device activation request failed: " << curl_easy_strerror(res) << std::endl;
	}

	curl_slist_free_all(headers);

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


//void SpotifyAPI::SportifyGet(const std::string& url, const char* header = "")
//{
//	curl_easy_reset(m_Curl);
//
//	std::string apiUrl = "https://api.spotify.com" + url;
//
//	struct curl_slist* headers = NULL;
//	headers = curl_slist_append(headers, ("Authorization: Bearer " + m_AccessToken).c_str());
//	if(header != "")
//		headers = curl_slist_append(headers, header);
//
//}
