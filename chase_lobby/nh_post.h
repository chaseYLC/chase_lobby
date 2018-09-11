#pragma once

#include <net/connection.h>
#include <OpenLib/jsoncpp/include/json/json.h>

namespace bob_lobbysvr {
	class User;
}

class nh_post
{
public:
	static void list(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void read(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	/*static void detail(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);*/
	static void get(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void getAll(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void sendNote(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void del(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void devDelAll(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	
private:
	static void list_process(bob_lobbysvr::User *user, const uint32_t seq
		, const unsigned int page, const unsigned int pageSize);
		
};