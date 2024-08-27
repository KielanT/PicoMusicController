#pragma once
#include <string>

#include "Spotify/SpotifyCalls.h"
#include "Serial.h"

class App
{
public:
	App() = default; // Default constructor
	~App(); // Default destructor

	App(const App&) = delete; // Copy constructor
	App(App&&) = delete; // Move constructor
	App& operator=(const App&) = delete; // Copy assignment operator
	App& operator=(App&&) = delete; // Move assignment operator

	void Init();

	void Run();

private:
	void SendSongDataToSerial(std::string& song, std::string& artists);

private:
	SpotifyCalls m_Spotify;
	Serial m_Serial{ "COM10", 9600 };
};

