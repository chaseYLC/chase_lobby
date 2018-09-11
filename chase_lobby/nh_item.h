#pragma once

#include <net/connection.h>
#include <OpenLib/jsoncpp/include/json/json.h>
#include "dataTable.h"

namespace bob_lobbysvr {
	class User;
}

class nh_item
{
public:
	static void all(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void buyRubyGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void buyGoldGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void buyCashGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void buyMarinaGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void beg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);
	static void testPurchaseAcc(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);


	static void shopList(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv);

	static void buyProcessor(const std::string &url, bob_lobbysvr::User *user, uint32_t sequenceNo, std::vector<Shop> &shopList, const int ruby);

private:
	static void addAccountMoney(bob_lobbysvr::User *user, const int money, Json::Value &rsp, const std::string &url);
	static void buyProcessor(const std::string &url, bob_lobbysvr::User *user, Json::Value &rsp, Shop &shop, int64_t ruby, int64_t gold);
	
	static void cashProcessor(const std::string &url, bob_lobbysvr::User *user, Json::Value &rsp, Shop &shop);
	static bool itemMake(const std::string &url, bob_lobbysvr::User *user, Json::Value &rsp, const Shop &shop);
	
	
	
};