#pragma once

#include <net/connection.h>
#include <net/MessageProcedure.h>
#include <stdint.h>
#include <string>
#include <map>

class ConnectorUser;

namespace MLN {
	class NetServiceConnector;
}

class BrokerConnector
{
public:
	static void onConnect(MLN::Net::Connection::ptr spConn);
	static void onConnectFailed(MLN::Net::Connection::ptr spConn);

	static void onClose(MLN::Net::Connection::ptr spConn);
	static void onCloseFailed(MLN::Net::Connection::ptr spConn);

	static void onUpdate(uint64_t elapse);

	static void onExpiredSession(MLN::Net::Connection::ptr spConn);

	static void noHandler(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer& msg);
	//////////////////////////////////////////////////////////////////
public:
	static void initHandler(MLN::Net::MessageProcedure *msgProc);
	//////////////////////////////////////////////////////////////////

	static void tryConnect(const size_t sessionCnt = 1, const size_t additionalWorkingThreadsCnt = 0);

	static void readJsonPacket(MLN::Net::Connection::ptr conn, unsigned int size, MLN::Net::MessageBuffer& msg);
	static void dispatch(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);

	static int sendJsonPacket(const std::string &url, Json::Value &jv);

private:
	static std::map<std::string
		, std::function< void(MLN::Net::Connection::ptr, const std::string&, Json::Value &) > > m_URLs; 

public:
	static ConnectorUser *m_user;
};