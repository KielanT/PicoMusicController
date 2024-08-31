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

    if (m_Serial.is_open())
        m_Serial.close();
}

std::string Serial::ReadLine()
{
    char c;
    std::string result{ "" };
    asio::steady_timer timer(m_IO);

    while (m_AppIsRunning.load())
    {

        if (!m_Serial.is_open())
        {
            break;
        }

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const asio::error_code& error)
            {
                if (!error && m_AppIsRunning.load())
                {
                    m_Serial.cancel();
                }
            });

        asio::error_code ec;
        asio::read(m_Serial, asio::buffer(&c, 1), ec);

        timer.cancel();

        if (ec == asio::error::operation_aborted)
        {
            break;
        }

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

    return result;
}

void Serial::WriteString(std::string s)
{
    asio::write(m_Serial, asio::buffer(s.c_str(), s.size()));
}

void Serial::Close()
{

    if (m_Serial.is_open())
        m_Serial.close();
}

