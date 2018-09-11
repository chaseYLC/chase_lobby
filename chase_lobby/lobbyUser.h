#pragma once

#include "userBasis.h"
#include <net/memoryPool.h>
#include <OpenLib/jsoncpp/include/json/json.h>

#include "lobby_protocol.h"
#include "gameDef.h"
#include "DBStructs.h"

class Room;

namespace chase_lobbysvr {

	class User
		: public UserBasis
		, public MLN::Net::MemoryPool< chase_lobbysvr::User >
	{
	public:
		User(MLN::Net::Connection::ptr conn)
			: UserBasis(conn)
		{}

		int SendJsonPacket(const std::string &url, Json::Value &jv);

	public:
		void recvHeartBeat();
		void logoutProcess();

	public:
		ChaseGameDef::PLAYER_TYPE::Ty _playerType;
		int32_t _roomNo;
		int64_t _roomBaseMoney;
		DBStruct::userinfo m_userInfo;

		TyUserIDX m_userIDX = 0;
		std::string m_userID;
	};
};// namespace chase_lobbysvr {


#define GET_USER_LOBBY(user)	chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();	if (nullptr == user){	LOGE << "user is null. ident: " << conn->getIdentity();	return;}