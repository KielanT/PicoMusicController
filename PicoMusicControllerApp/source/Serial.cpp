#include "Serial.h"
#include "iostream"

Serial::Serial(const std::string& port, const unsigned int& rate, std::atomic<bool>& appRunning)
	:m_IO(), m_Serial(m_IO), m_AppIsRunning(appRunning)
{
    m_Serial.open(port);
	m_Serial.set_option(asio::serial_port_base::baud_rate(rate));
}

Serial::~Serial()
{
    m_Serial.close();
}

std::string Serial::ReadLine()
{
    char c;
    std::string result;
    try
    {
        while (m_AppIsRunning.load()) // infinite loop
        {
            if (!m_Serial.is_open())
            {
                break;
            }
            asio::read(m_Serial, asio::buffer(&c, 1));
            switch (c)
            {
            case '\r':
                break;
            case '\n':
                return result;
            default:
                result += c;
            }
        }
    }
    catch (const std::system_error& e) 
    {
        std::string error = e.what();
        std::cerr << "System error: " << e.what() << std::endl;
        // TODO fix the issue causing the exception: "read: The I/O operation has been aborted because of either a thread exit or an application request."
    }
 
}

void Serial::WriteString(std::string s)
{
    asio::write(m_Serial, asio::buffer(s.c_str(), s.size()));
}

