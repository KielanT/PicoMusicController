#include "AuthServer.h"
#include <random>
#include "crow.h"
#include "curl/curl.h"

/***********************************************************************************************************
C++ version of the code found here https://developer.spotify.com/documentation/web-api/tutorials/code-flow

It is worth noting that we are using port 3000, which gets set on the developer dashboard for app where
Client ID and Client Secret is, the ports/url must match

Ideally we would use our own service for authentication instead of 
local host
************************************************************************************************************/

void AuthServer::Start(const std::string& clientId, const std::string& clientSecret)
{
	m_ID = clientId;
	m_Secret = clientSecret;

	m_State = GenerateRandomString(16);

	crow::SimpleApp app;

	app.route_dynamic("/login") ([&](const crow::request& req)
		{

			std::ostringstream url_stream;
			url_stream << "https://accounts.spotify.com/authorize?"
				<< "response_type=code&"
				<< "client_id=" << clientId << "&"
				<< "scope=" << m_Scope << "&"
				<< "redirect_uri=" << m_RedirectURL << "&"
				<< "state=" << m_State;

			std::string url = url_stream.str();

			crow::response res(302);
			res.add_header("Location", url_stream.str());

			return res;
		});
	
	app.route_dynamic("/callback") ([&](const crow::request& req)
		{
			std::string code = req.url_params.get("code");
			std::string state = req.url_params.get("state");

			if (state.empty() || state != m_State)
			{
				return crow::response(302, "/#error=state_mismatch");
			}

			AccessToken = ExchangeCodeForToken(code);
			std::cout << "Token response: " << AccessToken << std::endl;
			if (AccessToken.find("error") != std::string::npos)
			{
				return crow::response(500, "Failed to authenticate. \n");
			}
			app.stop();
			// Process the token response, extract tokens, and handle as needed
			return crow::response("Successfully authenticated. \n");
		});

	std::string url = "http://localhost:3000/login";
	std::string command = "start " + url;
	system(command.c_str());
	app.port(3000).multithreaded().run();
}

std::string AuthServer::GenerateRandomString(size_t length)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

	std::string str;
	for (size_t i = 0; i < length; ++i)
	{
		str += alphanum[dis(gen)];
	}
	return str;
}

std::string AuthServer::ExchangeCodeForToken(const std::string& code)
{
	CURL* curl;
	CURLcode res;
	std::string response_data;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (!curl)
	{
		return "Failed to initialize curl";
	}

	// Prepare POST data
	std::string post_fields = "code=" + std::string(curl_easy_escape(curl, code.c_str(), code.length())) +
		"&redirect_uri=" + curl_easy_escape(curl, m_RedirectURL.c_str(), m_RedirectURL.length()) +
		"&grant_type=authorization_code";

	std::string auth_string = m_ID + ":" + m_Secret;
	std::string auth_header = "Basic " + crow::utility::base64encode(auth_string, auth_string.size());

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
	headers = curl_slist_append(headers, ("Authorization: " + auth_header).c_str());

	curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		response_data = "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return response_data;
}
