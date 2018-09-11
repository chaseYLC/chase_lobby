#include "stdafx.h"
#include "nh_lobby.h"

#include <net/mssqlConsumer.h>

#include "Environment.h"
#include "lobbyUser.h"
#include "lobbyUserManager.h"
#include "DBStructs.h"
#include "MLN_Utils.h"

using namespace ChaseGameDef;

void nh_lobby::login(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER_LOBBY(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	std::string userID = jv["userID"].asString();
	std::string userPW = jv["userPW"].asString();
	const int clientVersion = jv["clientVersion"].asInt();

	std::string userAddr = conn->socket().remote_endpoint().address().to_string();

	// 유저정보초기화
	user->_playerType = PLAYER_TYPE::NORMAL;
	user->_roomNo = -1;
	user->_roomBaseMoney = 0;

	auto sendDBFailed = [&](const int retCode, const char *retMsg) {
		rsp[RSP_RC] = retCode;
		rsp[RSP_RM] = retMsg;
		user->SendJsonPacket(url, rsp);
		conn->closeReserve(0);
	};

	if (clientVersion != Environment::instance()->m_clientVersion) {
		sendDBFailed(-1, "client version");
		return;
	}

	MLN::MSSQL::DBConsumer db;
	db.Execute("EXEC dbo.UP_LOGIN \"%s\",\"%s\",\"%s\"", userID.c_str(), userPW.c_str(), userAddr.c_str());
	if (0 >= db.m_resultColumns) {
		LOGE << "failed execute UP_LOGIN. userID:" << userID;
		sendDBFailed(-99, "db falied");
		return;
	}

	if (false == db.FetchResultCode()) {
		sendDBFailed(-99, "db falied2");
		return;
	}

	if (0 != db.m_dbRetCode) {
		// 로그인 실패.
		sendDBFailed(db.m_dbRetCode, db.m_dbRetMsg);
		return;
	}

	if (false == db.MoreResults()) {
		sendDBFailed(-99, "db failed more");
		return;
	}
	
	//// 사용자 정보.
	DBStruct::userinfo & userinfo = user->m_userInfo;
	db.Bind(&userinfo.nID);
	DB_STRING_BIND(db, userinfo, userid);
	DB_STRING_BIND(db, userinfo, username);
	DB_STRING_BIND(db, userinfo, email);
	DB_STRING_BIND(db, userinfo, pass);
	db.Bind(&userinfo.gamemoney);
	db.Bind(&userinfo.join_date);

	if (false == db.Fetch()) {
		sendDBFailed(-99, "db falied3");
		return;
	}

	MLN_Utils::toLowerCase(MLN_Utils::trim(userinfo.userid));

	MLN_Utils::trim(userinfo.pass);

	loginSuccessed(user, url);
}

void nh_lobby::loginSuccessed(chase_lobbysvr::User *user, const std::string &url)
{
}
