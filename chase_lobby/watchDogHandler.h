#pragma once

#include <string>
#include <net/Singleton.h>


class watchDogHandler
	: public MLN::Net::SingletonLight<watchDogHandler>
{
public:
	void registCallback(std::shared_ptr<boost::asio::io_service> ios, unsigned short port);
	std::string inputLine(const std::string &inputMsg, boost::asio::ip::tcp::socket &sock);
	void closedConsole();

private:
	std::weak_ptr<boost::asio::io_service> m_ios;
};