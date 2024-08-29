#pragma once
#include <string>
#include <vector>

class AuthServer
{
public:
	AuthServer() {} ; // Default constructor
	~AuthServer() = default; // Default destructor

	AuthServer(const AuthServer&) = delete; // Copy constructor
	AuthServer(AuthServer&&) = delete; // Move constructor
	AuthServer& operator=(const AuthServer&) = delete; // Copy assignment operator
	AuthServer& operator=(AuthServer&&) = delete; // Move assignment operator

	void Start(const std::string& clientId, const std::string& clientSecret);

private:
	std::string GenerateRandomString(size_t length);

	std::string ExchangeCodeForToken(const std::string& code);

	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
	{
		size_t total_size = size * nmemb;
		userp->append(static_cast<char*>(contents), total_size);
		return total_size;
	}


public:
	std::string AccessToken{ "" };

private:
	std::string m_RedirectURL{ "http://localhost:3000/callback" }; // Use your own web page

	/*The scope defines what our application needs to make calls to the spotify api
	which can be seen on the same page as the references for the calls you would like to make (if they are required)*/
	std::string m_Scope{ "user-read-private user-read-email user-read-playback-state user-modify-playback-state user-read-currently-playing" };

	std::string m_State{};
	std::string m_ID{};
	std::string m_Secret{};
};

