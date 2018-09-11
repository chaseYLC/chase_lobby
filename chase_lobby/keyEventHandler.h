#pragma once

#include <net/Singleton.h>

class keyEventHandler
	: public MLN::Net::SingletonLight<keyEventHandler>
{
public:
	void registCallback(std::shared_ptr<boost::asio::io_service> ios);
	void processKey(const unsigned int vKey);

private:
	std::weak_ptr<boost::asio::io_service> m_ios;

};