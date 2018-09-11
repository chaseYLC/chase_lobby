#pragma once

#include <string>
#include <net/Singleton.h>

class Environment
	: public MLN::Net::SingletonLight< Environment >
{
public:
	Environment() = default;
	virtual ~Environment() = default;

	struct MLN_SvrSetting {
		bool activate;
		std::string addr;
		std::string listenPort;
		uint32_t keep_alive_ms = 0;
		uint32_t session_timeout_ms;
		size_t workers = 1;
		uint8_t enc_type = 1;
		uint32_t tick_time_ms = 1000;
	};

	bool Initialize(const char* szFileName);

public:
	std::string m_title;

	MLN_SvrSetting m_svrBroker;
	MLN_SvrSetting m_svrLobby;
	MLN_SvrSetting m_svrRoom;

	uint16_t m_testInterfacePort;

	int32_t m_clientVersion;
};
