#include "stdafx.h"
#include "BrokerAcceptorReceiver.h"
#include "keyEventHandler.h"
#include "Environment.h"
#include "lobbyUserManager.h"
//#include "relayUserManager.h"
#include <broker_protocol.h>
#include <OpenLib/jsoncpp/include/json/json.h>


std::map<std::string
	, std::function< void(MLN::Net::Connection::ptr, const std::string&, Json::Value &) > > BrokerAcceptorReceiver::m_URLs;

using namespace BROKER_PROTOCOL;

#define BROKER_SERVER_MSG_HANDLER_REG(PT)		msgProc->registMessage(PT##::packet_value, &BrokerAcceptorReceiver::h##PT)

#define POP_MSG(user, msg, req)		try {\
										msg >> req;\
									}catch (std::exception& e) {\
										LOGE << "failed message-pop. user : " << std::to_string(user->m_IDX) << ", " << e.what();\
										return;\
									}

void BrokerAcceptorReceiver::onAccept(MLN::Net::Connection::ptr spConn)
{
	LOGD << "accept - " << spConn->socket().remote_endpoint().address() << " : " << spConn->socket().remote_endpoint().port();
}

void BrokerAcceptorReceiver::onClose(MLN::Net::Connection::ptr spConn)
{
	LOGD << "close - " << spConn->socket().remote_endpoint().address() << " : " << spConn->socket().remote_endpoint().port();

	//bob_brokersvr::User *user = (bob_brokersvr::User *)spConn->getTag();
	//if (nullptr == user) {
	//	return;
	//}

	//if (bob_brokersvr::User::SERVER_TYPE_RELAY == user->m_serverType) {
	//	RelayUserManager::instance()->deleteUser(spConn);
	//}
	//else if (bob_brokersvr::User::SERVER_TYPE_LOBBY == user->m_serverType) {
	//	LobbyUserManager::instance()->deleteUser(spConn);
	//}
}

void BrokerAcceptorReceiver::onUpdate(uint64_t elapse)
{
}

void BrokerAcceptorReceiver::noHandler(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer& msg)
{
	LOGW << "no Handler.";
	spConn->closeReserve(0);
}

void BrokerAcceptorReceiver::onAcceptFailed(MLN::Net::Connection::ptr spConn)
{
	LOGW << "failed accept";
}
void BrokerAcceptorReceiver::onCloseFailed(MLN::Net::Connection::ptr spConn)
{
	LOGW << "failed close";
}

void BrokerAcceptorReceiver::onExpiredSession(MLN::Net::Connection::ptr spConn)
{
	auto & endPoint = spConn->socket().remote_endpoint();
	LOGW << "Expired Session. (addr/port) : " << endPoint.address() << " / " << endPoint.port();

	 spConn->closeReserve(0);
}

void BrokerAcceptorReceiver::readJsonPacket(MLN::Net::Connection::ptr conn, unsigned int size, MLN::Net::MessageBuffer& msg)
{
	PT_JSON req;
	msg.read((char*)&req, BROKER_PROTOCOL::PT_JSON::HEAD_SIZE);

	if (PT_JSON::MAX_BODY_SIZE < req.bodySize
		|| 0 >= req.bodySize) {

		LOGE << "body size error. ip : " << conn->socket().remote_endpoint().address() 
			<< ", port : " << conn->socket().remote_endpoint().port();

		return;
	}

	try {
		msg.read((char*)req.jsonBody, req.bodySize);
	}
	catch (std::exception &e) {
		LOGE << "body pop error. ip : " << conn->socket().remote_endpoint().address()
			<< ", port : " << conn->socket().remote_endpoint().port()
			<< ", msg : " << e.what();
		return;
	}

	
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

void BrokerAcceptorReceiver::initHandler(MLN::Net::MessageProcedure *msgProc)
{
	//// account
	//m_URLs["/auth"] = nh_common::auth;
	//m_URLs["/loginedUser"] = nh_common::loginedUser;
	//m_URLs["/userLogin"] = nh_common::userLogin;
	//m_URLs["/userLogout"] = nh_common::userLogout;

	//// is Login
	//m_URLs["/isOnline"] = nh_common::isOnline;

	//// chat
	//m_URLs["/chatSendMsg"] = nh_common::chatSendMsg;
}

void BrokerAcceptorReceiver::dispatch(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	auto it = m_URLs.find(url);

	if (m_URLs.end() != it) {
		it->second(conn, url, jv);
		return;
	}

	LOGE << "invalid URL : " << url;
}