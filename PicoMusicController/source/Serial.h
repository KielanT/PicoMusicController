#pragma once

#include <asio.hpp>

// https://gist.github.com/kaliatech/427d57cb1a8e9a8815894413be337cf9

class Serial
{
public:
	Serial(const std::string& port, const unsigned int& rate, std::atomic<bool>& appRunning); // Default constructor
	~Serial(); // Default destructor

	Serial(const Serial&) = delete; // Copy constructor
	Serial(Serial&&) = delete; // Move constructor
	Serial& operator=(const Serial&) = delete; // Copy assignment operator
	Serial& operator=(Serial&&) = delete; // Move assignment operator

	std::string ReadLine();

	void WriteString(std::string s);

private:
	asio::io_service m_IO;
	asio::serial_port m_Serial;
	std::atomic<bool>& m_AppIsRunning;
};
