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
	
	// HTTP Funcs
	[[nodiscard]] std::string SpotifyGET(const std::string& url, const std::vector<std::string>& headers);
	[[maybe_unused]] std::string SpotifyPUT(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields = "");
	[[maybe_unused]] std::string SpotifyPOST(const std::string& url, const std::vector<std::string>& headers, const std::string& postfields = "");
};
