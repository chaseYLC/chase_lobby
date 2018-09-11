#include "stdafx.h"
#include "connectorUser.h"

#include <net/logger.h>
#include "../../bob_broker/[include]/broker_protocol.h"
//#include <net/messageBuffer.h>

ConnectorUser::ConnectorUser(MLN::Net::Connection::ptr conn)
{
	_conn = conn;
}

int32_t ConnectorUser::Send(char* data, uint32_t size)
{
	_conn->sendPacket(data, size);
	return size;
}

int ConnectorUser::SendJsonPacket(const std::string &url, Json::Value &jv)
{
	Json::FastWriter fastWritter;
	std::string serialized = fastWritter.write(jv);

	LOGD << "send Json : " << serialized;

	BROKER_PROTOCOL::PT_JSON pk;

	// set URL..
	memcpy(pk.url, url.c_str(), url.length());

	//  set Body Size..
	pk.bodySize = (uint16_t)serialized.length();

	//  set Body String..
	memcpy(pk.jsonBody, serialized.c_str(), serialized.length());

	// send ~!
	return this->Send((char*)&pk, BROKER_PROTOCOL::PT_JSON::HEAD_SIZE + pk.bodySize);
}