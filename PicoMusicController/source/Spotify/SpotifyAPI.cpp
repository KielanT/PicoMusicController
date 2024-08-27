#include "SpotifyAPI.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include "crow/utility.h"
#include "AuthServer.h"


std::string SpotifyAPI::SpotifyGET(const std::string& url, const std::vector<std::string>& headers)
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

	std::string response_data{ "" };
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		std::cerr << "GET request failed: " << curl_easy_strerror(res) << std::endl;
	}
	curl_slist_free_all(headerList);
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


