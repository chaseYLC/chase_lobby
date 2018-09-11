#pragma once

#include <net/connection.h>
#include <OpenLib/jsoncpp/include/json/json.h>

class ConnectorHandler
{
public:

// chat
	static void auth(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void loginedUser(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void isOnline(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void chatSendMsg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void chatReceiveMsg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
};