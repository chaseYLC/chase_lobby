#include "stdafx.h"
#include "connectorHandler.h"
#include "lobbyUser.h"
#include "lobbyUserManager.h"
#include "brokerConnector.h"
#include "nh_lobby.h"

void ConnectorHandler::auth(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	LOGI << "connected to Broker. serverIDX : " << g_idx4Broker;
	g_idx4Broker = jv["serverIDX"].asUInt();

	/*LobbyUserManager::instance()->addUser(conn);
	return;*/
}

void ConnectorHandler::loginedUser(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	boost::weak_ptr<MLN::Net::Connection> user = LobbyUserManager::instance()->getUser(jv["identity"].asUInt64());
	MLN::Net::Connection::ptr targetConn = user.lock();
	if (targetConn) {
		/*nh_lobby::loginPostDupCheck(targetConn, "/lobby/login", jv);*/
	}
}

void ConnectorHandler::isOnline(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	jv[RSP_RC] = 0;
	jv[RSP_RM] = RSP_OK;
	jv[RSP_SEQ] = jv["pkSeq"];

	std::function<void(chase_lobbysvr::User *)> dummyUserFunc = [](chase_lobbysvr::User *){};

	LobbyUserManager::instance()->sendPacketFromBroker(
		jv["identity"].asUInt64()
		, "/friend/list"
		, jv
		, DUMMY_UM_HOOKER
	);
}

void ConnectorHandler::chatSendMsg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	LobbyUserManager::instance()->sendPacketFromBroker(
		jv["identity"].asUInt64()
		, "/chat/sendMsg"
		, jv
		, DUMMY_UM_HOOKER
	);
}

void ConnectorHandler::chatReceiveMsg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	switch (jv["channelType"].asInt())
	{
	case 1:
		LobbyUserManager::instance()->sendPacketFromBroker_All(
			 "/chat/receiveMsg"
			, jv);
		break;
	case 2:
		LobbyUserManager::instance()->sendPacketFromBroker_byUserIDX(
			jv["addition_toIDX"].asUInt()
			, "/chat/receiveMsg"
			, jv);
		break;
	}
}