#pragma once

#include <net/connection.h>
#include <OpenLib/jsoncpp/include/json/json.h>

namespace chase_lobbysvr {
	class User;
}

class nh_lobby
{
public:
	static void login(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);

private:
	static void loginSuccessed(chase_lobbysvr::User *user, const std::string &url);

};