#include "stdafx.h"
#include "nh_chat.h"
#include "lobbyUser.h"
#include "brokerConnector.h"
#include "MLN_Utils.h"

void nh_chat::sendMsg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER_LOBBY(user);

	/* channelType
	    1 : 그룹채널
		2 : 개인채널
	*/

	// 메세지 길이.
	if (0 >= jv["msg"].asString().length() ) {
		LOGE << "msg size is zero.userIDX : " << std::to_string(user->m_userIDX);
		return;
	}

	// 개인메세지일 경우 대상 nick 길이 점검.
	if (2 == jv["channelType"].asInt()
		&& 2 > jv["to"].asString().length()) {
		LOGE << "invalid target nickname. userIDX : " << std::to_string(user->m_userIDX);
		return;
	}

	// 1. set Additional values.
	jv["addition_fromIDX"] = user->m_userIDX;
	jv["addition_fromSVR"] = g_idx4Broker;
	jv["from"] = user->m_userID;
	jv["seq"] = jv[RSP_SEQ];
	jv["sendTime"] = MLN_Utils::getLocalTimeSec();

	// 브로커로 전달.
	jv["identity"] = user->getConn()->getIdentity();
	BrokerConnector::sendJsonPacket("/chatSendMsg", jv);
}
