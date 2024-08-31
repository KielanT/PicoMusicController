#pragma once
#include <string>

#include "Spotify/SpotifyCalls.h"
#include "Serial.h"

#include "Window/WindowsWindow.h"

class App
{
public:
	App() = default; // Default constructor
	~App(); // Default destructor

	App(const App&) = delete; // Copy constructor
	App(App&&) = delete; // Move constructor
	App& operator=(const App&) = delete; // Copy assignment operator
	App& operator=(App&&) = delete; // Move assignment operator

	void Init(const std::string& port);

	void Run();

	void SerialThread();

private:
	void SendSongDataToSerial(std::string& song, std::string& artists);

private:
	std::unique_ptr<SpotifyCalls> m_Spotify;
	std::unique_ptr<Serial> m_Serial; // TODO Move to app contructor

	std::unique_ptr<WindowsWindow> m_Window;
	std::atomic<bool> m_IsAppRunning{ true };

	std::thread m_SerialThread;
};

