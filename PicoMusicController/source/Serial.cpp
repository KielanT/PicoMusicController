#include "Serial.h"

Serial::Serial(const std::string& port, const unsigned int& rate)
	:m_IO(), m_Serial(m_IO, port)
{
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
    for (;;)
    {
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

