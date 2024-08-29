#pragma once
#include <iostream>

#include "curl/curl.h"

// Spotify web api: https://developer.spotify.com/documentation/web-api

class CPPify
{
public:
	
	/***********************************************************
	SpotifyGET function
	Used to get data in a json format from spotify such as:
	- Get Currently Playing track
	- Get Ablum
	***********************************************************/

	[[nodiscard]] static std::string SpotifyGET(const std::string& url, const std::vector<std::string>& headers)
	{
		CURL* curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		struct curl_slist* headerList = NULL;
		for (const auto& header : headers)
		{
			headerList = curl_slist_append(headerList, header.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

		std::string responseData{ "" };
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			std::cerr << "GET request failed: " << curl_easy_strerror(res) << std::endl;
		}
		curl_slist_free_all(headerList);
		curl_easy_cleanup(curl);

		return responseData;
	}

	/***********************************************************
	SpotifyPUT function
	Used to update and recieve data in spotify such as:
	- Start/Resume Playback
	- Update Playlist item
	***********************************************************/

	[[maybe_unused]] static std::string SpotifyPUT(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields = "")
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

		std::string responseData{ "" };
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			std::cerr << "PUT request failed: " << curl_easy_strerror(res) << std::endl;
		}

		curl_slist_free_all(headerList);
		curl_easy_cleanup(curl);

		return responseData;
	}

	/***********************************************************
	SpotifyPOST function
	Used to send data to spotify such as:
	- Skip To Next (track)
	- Create Playlist
	***********************************************************/

	[[maybe_unused]] static std::string SpotifyPOST(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields = "")
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

		std::string responseData{ "" };
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			std::cerr << "POST request failed: " << curl_easy_strerror(res) << std::endl;
		}

		curl_slist_free_all(headerList);
		curl_easy_cleanup(curl);

		return responseData;
	}
	
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
	{
		size_t total_size = size * nmemb;
		userp->append(static_cast<char*>(contents), total_size);
		return total_size;
	}

};
