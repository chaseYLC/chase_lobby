#pragma once

#include <thread>
#include <mutex>
#include <net/connection.h>
#include <OpenLib/jsoncpp/include/json/json.h>

class ConnectorUser
{
public:
	ConnectorUser(MLN::Net::Connection::ptr conn);

	int32_t Send(char* data, uint32_t size);

	template <typename T>
	int32_t Send(T &data)
	{
		return Send((char*)&data, sizeof(data));
	}

	MLN::Net::Connection::ptr getConn() { return _conn; }

public:
	int SendJsonPacket(const std::string &url, Json::Value &jv);

private:
	MLN::Net::Connection::ptr _conn;

	bool m_connected = false;
};
