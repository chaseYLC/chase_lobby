#pragma once

#include <thread>
#include <mutex>
#include <net/connection.h>
#include <net/messageBuffer.h>
#include <net/packetEncType.h>

namespace chase_lobbysvr {

	class UserBasis
	{
		friend class UserManagerBasis;

	public:
		UserBasis(MLN::Net::Connection::ptr conn);
		virtual ~UserBasis() = default;

		bool initEncKey(const MLN::Net::EncType::Type encType, void *, char *buffer, const int bufferSize);

		int32_t Send(char* data, uint32_t size);
		template <typename T>
		int32_t Send(T &data)
		{
			return Send((char*)&data, sizeof(data));
		}

		int32_t Decrypt(
			const MLN::Net::EncType::Type encType
			, char *entryptedData, const uint32_t entryptedDataSize
			, OUT char* decryptedBuffer, const uint32_t decryptedBufferSize);


		MLN::Net::Connection::ptr getConn() { return _conn; }

	public:
		static MLN::Net::EncType::Type m_encType;

	private:
		MLN::Net::Connection::ptr _conn;

		bool m_connected = false;
	};

}