#include "stdafx.h"
#include "Environment.h"

#include <net/xmlParsingSupport.h>
#include <net/packetEncType.h>


bool Environment::Initialize(const char* szFileName)
{
	XML_PARSING_BEGIN(szFileName);

	// read server
	XML_PARSING_GET(m_title, "config.title");

	auto loadSvrSetting = [&](MLN_SvrSetting &svr, const std::string &svrRootString) {
		XML_PARSING_GET(svr.activate, (svrRootString + ".activate"));
		XML_PARSING_GET(svr.addr, (svrRootString + ".addr"));
		XML_PARSING_GET(svr.listenPort, (svrRootString + ".listenPort") );
		XML_PARSING_GET(svr.keep_alive_ms, (svrRootString + ".keep_alive_ms"));
		XML_PARSING_GET(svr.session_timeout_ms, (svrRootString + ".session_timeout_ms"));
		XML_PARSING_GET(svr.workers, (svrRootString + ".workers"));
		XML_PARSING_GET(svr.enc_type, (svrRootString + ".enc_type"));
		XML_PARSING_GET(svr.tick_time_ms, (svrRootString + ".tick_time_ms"));
		
	};

	loadSvrSetting(m_svrBroker, "config.servers.broker");
	loadSvrSetting(m_svrLobby, "config.servers.lobby");
	loadSvrSetting(m_svrRoom, "config.servers.room");

	XML_PARSING_GET(m_clientVersion, "config.misc.clientVersion");

	XML_PARSING_GET(m_testInterfacePort, "config.test_interface.listenPort");

	XML_PARSING_END
	return true;
}
