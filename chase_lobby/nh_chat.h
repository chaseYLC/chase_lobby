#pragma once

#include <net/connection.h>
#include <OpenLib/jsoncpp/include/json/json.h>

namespace chase_lobbysvr {
	class User;
}

class nh_chat
{
public:
	static void sendMsg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
};