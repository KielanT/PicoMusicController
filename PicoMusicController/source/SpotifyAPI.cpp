#include "SpotifyAPI.h"

#include <iostream>
#include <fstream>
#include <filesystem>

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

	// TODO only do on first time
	AuthServer server;
	server.Start(id, secret); // TODO don't hardcode 

	m_AccessJson = nlohmann::json::parse(server.AccessToken);
	m_AccessToken = m_AccessJson["access_token"];

	system("cls");
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
		
		auto devices = json["devices"];
		for (const auto& device : devices)
		{
			if (device["is_active"] == true)
			{
				isValid = true;
			}
			else
				isValid = false; 
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

void SpotifyAPI::GetCurrentTrack()
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

	if (res == CURLE_OK && !response_data.empty())
	{
		nlohmann::json json = nlohmann::json::parse(response_data);
		m_ShuffleState = json["shuffle_state"]; 
		m_IsPlaying = json["is_playing"];
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
	else
	{
		std::cerr << "request failed: " << curl_easy_strerror(res) << std::endl;
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
		std::cerr << "Pause request failed: " << curl_easy_strerror(res) << std::endl;
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

void SpotifyAPI::ReadCredentials(auto& id, auto& secret)
{
	std::ifstream file("../../SpotifyDevCredentials.txt");
	getline(file, id);
	getline(file, secret);

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
