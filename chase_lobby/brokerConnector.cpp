#include "stdafx.h"
#include "brokerConnector.h"
#include "Environment.h"
#include "broker_protocol.h"
#include <OpenLib/jsoncpp/include/json/json.h>
#include "connectorIndex.h"
#include <net/NetServiceConnector.h>
#include "connectorUser.h"
#include "connectorHandler.h"

std::map<std::string
	, std::function< void(MLN::Net::Connection::ptr, const std::string&, Json::Value &) > > BrokerConnector::m_URLs;

ConnectorUser * BrokerConnector::m_user = nullptr;

using namespace BROKER_PROTOCOL;


#define LOGIN_MSG_HANDLER_REG(PT)		msgProc->registMessage(PT##::packet_value, &BrokerConnector::h##PT)

void BrokerConnector::onConnect(MLN::Net::Connection::ptr spConn)
{
	m_user = new ConnectorUser(spConn);
	spConn->setTag(m_user);

	Json::Value req;
	req["test"] = "test";
	m_user->SendJsonPacket("/auth", req);
}

void BrokerConnector::onClose(MLN::Net::Connection::ptr spConn)
{
	delete m_user;
	spConn->setTag(nullptr);

	tryConnect();
}

void BrokerConnector::noHandler(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer& msg)
{
	LOGD << "no Handler";
	spConn->closeReserve(0);
}

void BrokerConnector::onConnectFailed(MLN::Net::Connection::ptr spConn)
{
	LOGD << "onConnectFailed";

	//// 계속적으로 접속을 시도합니다.
	tryConnect();
}

void BrokerConnector::onCloseFailed(MLN::Net::Connection::ptr spConn)
{
	LOGD << "onCloseFailed";
}

void BrokerConnector::onUpdate(uint64_t /*elapse*/)
{
}

void BrokerConnector::onExpiredSession(MLN::Net::Connection::ptr spConn)
{
	spConn->closeReserve(0);
}

void BrokerConnector::initHandler(MLN::Net::MessageProcedure *msgProc)
{
	// account
	m_URLs["/auth"] = ConnectorHandler::auth;
	m_URLs["/loginedUser"] = ConnectorHandler::loginedUser;
	m_URLs["/isOnline"] = ConnectorHandler::isOnline;

	// chat
	m_URLs["/chatSendMsg"] = ConnectorHandler::chatSendMsg;
	m_URLs["/chatReceiveMsg"] = ConnectorHandler::chatReceiveMsg;
}

void BrokerConnector::readJsonPacket(MLN::Net::Connection::ptr conn, unsigned int size, MLN::Net::MessageBuffer& msg)
{
	PT_JSON req;
	msg.read((char*)&req, BROKER_PROTOCOL::PT_JSON::HEAD_SIZE);

	if (PT_JSON::MAX_BODY_SIZE < req.bodySize
		|| 0 >= req.bodySize) {
		LOGE << "body size error in broker"
			<< ", size : " << std::to_string(req.bodySize);

		return;
	}

	try {
		msg.read((char*)req.jsonBody, req.bodySize);
	}
	catch (std::exception &e) {
		LOGE << "body pop error in broker" 
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

void BrokerConnector::dispatch(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	auto exceptionHandler = [&url, &jv]() {
		LOGE << "invalid json parsing."
			<< std::endl << " - url : " << url
			<< std::endl << " - requestBody : " << jv.toStyledString();
	};

	auto it = m_URLs.find(url);

	if (m_URLs.end() != it) {
		try {
			it->second(conn, url, jv);
		}
		catch (Json::Exception &) {
			exceptionHandler();
		}
		catch (...) {
			exceptionHandler();
		}
		
		return;
	}

	LOGE << "broker invalid URL : " << url;
}




void BrokerConnector::tryConnect(const size_t sessionCnt /*= 1*/, const size_t additionalWorkingThreadsCnt /*= 0*/)
{
	auto svcConnector = MLN::Net::NetService::getConnector(CONNECTOR_INDEX::BROKER_CONNECTOR);

	svcConnector->connectWait(
		sessionCnt
		, additionalWorkingThreadsCnt
		);
}

int BrokerConnector::sendJsonPacket(const std::string &url, Json::Value &jv)
{
	if (nullptr == m_user) { return 0; }

	return m_user->SendJsonPacket(url, jv);
}
