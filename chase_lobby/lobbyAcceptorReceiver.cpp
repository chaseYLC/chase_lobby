#include "stdafx.h"
#include "LobbyAcceptorReceiver.h"
#include "keyEventHandler.h"
#include "Environment.h"
#include "lobbyUserManager.h"
#include <lobby_protocol.h>
#include <OpenLib/jsoncpp/include/json/json.h>
#include "lobbyUser.h"

// handlers..
#include "nh_lobby.h"
#include "nh_chat.h"

std::map<std::string
	, std::function< void(MLN::Net::Connection::ptr, const std::string&, Json::Value &) > > LobbyAcceptorReceiver::m_URLs;

using namespace LOBBY_PROTOCOL;

#define LOBBY_SERVER_MSG_HANDLER_REG(PT)		msgProc->registMessage(PT##::packet_value, &LobbyAcceptorReceiver::h##PT)

#define POP_MSG(user, msg, req)		try {\
										msg >> req;\
									}catch (std::exception& e) {\
										LOGE << "failed message-pop. user : " << std::to_string(user->m_userIDX) << ", " << e.what();\
										return;\
									}

void LobbyAcceptorReceiver::onAccept(MLN::Net::Connection::ptr spConn)
{
	LOGD << "accept - " << spConn->socket().remote_endpoint().address() << " : " << spConn->socket().remote_endpoint().port();

	LobbyUserManager::instance()->addUser(spConn);
}

void LobbyAcceptorReceiver::onClose(MLN::Net::Connection::ptr spConn)
{
	LOGD << "close - " << spConn->socket().remote_endpoint().address() << " : " << spConn->socket().remote_endpoint().port();

	LobbyUserManager::instance()->deleteUser(spConn);
}

void LobbyAcceptorReceiver::onUpdate(uint64_t elapse)
{
}

void LobbyAcceptorReceiver::noHandler(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer& msg)
{
	LOGW << "no Handler.";
	spConn->closeReserve(0);
}

void LobbyAcceptorReceiver::onAcceptFailed(MLN::Net::Connection::ptr spConn)
{
	LOGW << "failed accept";
}
void LobbyAcceptorReceiver::onCloseFailed(MLN::Net::Connection::ptr spConn)
{
	LOGW << "failed close";
}

void LobbyAcceptorReceiver::onExpiredSession(MLN::Net::Connection::ptr spConn)
{
	auto & endPoint = spConn->socket().remote_endpoint();
	LOGW << "Expired Session. (addr/port) : " << endPoint.address() << " / " << endPoint.port();

	 spConn->closeReserve(0);
}

void LobbyAcceptorReceiver::readJsonPacket(MLN::Net::Connection::ptr conn, unsigned int size, MLN::Net::MessageBuffer& msg)
{
	LOGI << "recv json packet";

	GET_USER_LOBBY(user);

	if (LOBBY_PROTOCOL::PT_JSON::HEAD_SIZE > size) {
		LOGE << "invalid packet.";
		conn->closeReserve(0);
		return;
	}

	PT_JSON req;
	msg.read((char*)&req, LOBBY_PROTOCOL::PT_JSON::HEAD_SIZE);

	if (PT_JSON::MAX_BODY_SIZE < req.bodySize
		|| 0 >= req.bodySize) {
		LOGE << "body size error. user : " << std::to_string(user->m_userIDX)
			<< ", size : " << std::to_string(req.bodySize);

		return;
	}

	try {
		msg.read((char*)req.jsonBody, req.bodySize);
	}
	catch (std::exception &e) {
		LOGE << "body pop error. user : " << std::to_string(user->m_userIDX) << ", msg : " << e.what();
		return;
	}

	//////////////////////////////////////////////
	// desc...

	
	// pasing json string.
	Json::Reader reader;
	Json::Value value;
	std::string serializedString((char*)&(req.jsonBody), req.bodySize);

	if (false == reader.parse(serializedString, value)) {
		LOGE << "invalid json string";
		return;
	}

#ifdef _DEBUG
	LOGD << "json : " << value.toStyledString();

#endif
	char urlString[sizeof(req.url)] = { 0, };
	memcpy(urlString, req.url, sizeof(req.url));
	dispatch(conn, urlString, value);

	return;
}

void LobbyAcceptorReceiver::recvHeartBeat(MLN::Net::Connection::ptr conn, unsigned int size, MLN::Net::MessageBuffer& msg)
{
	GET_USER_LOBBY(user);

	if ( sizeof(PT_HEARTBEAT) > size) {
		LOGE << "invalid heartHeat!.";
		conn->closeReserve(0);
		return;
	}

	PT_HEARTBEAT req;
	msg.read((char*)&req, LOBBY_PROTOCOL::PT_JSON::HEAD_SIZE);

	user->recvHeartBeat();
}

void LobbyAcceptorReceiver::initHandler(MLN::Net::MessageProcedure *msgProc)
{
	// lobby
	m_URLs["login"] = nh_lobby::login;

	// chat
	m_URLs["/chat/sendMsg"] = nh_chat::sendMsg;

}

void LobbyAcceptorReceiver::dispatch(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	/*auto it = m_URLs.find(jv["url"].asString());*/

	auto exceptionHandler = [&conn, &url, &jv]() {
		chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
		if (nullptr == user) {
			LOGW << "invalid json parsing.";
		}
		else {
			LOGE << "invalid json parsing. userIDX:" << user->m_userIDX
				<< std::endl << " - url : " << url
				<< std::endl << " - requestBody : " << jv.toStyledString();
		}
	};

	auto it = m_URLs.find(url);

	if (m_URLs.end() != it) {
		try {
			it->second(conn, url, jv);
			return;
		}
		catch(Json::Exception &e){
			LOGE << e.what();
			exceptionHandler();
		}
		catch (...) {
			exceptionHandler();
		}
	}
	LOGE << "invalid URL : " << url;
}