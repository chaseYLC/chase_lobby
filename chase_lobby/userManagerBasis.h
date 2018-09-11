#pragma once

#include <net/Singleton.h>
#include <net/packetEncType.h>

class UserManagerBasis
{
public:
	bool createUser(const MLN::Net::EncType::Type encType, void * headerPtr, MLN::Net::Connection::ptr conn, uint32_t size, MLN::Net::MessageBuffer& msg);

	bool decryptMessage(const MLN::Net::EncType::Type encType, MLN::Net::Connection::ptr conn, uint32_t msgSize, MLN::Net::MessageBuffer& msg, MLN::Net::MessageBuffer& madeMsg);
	static void onClose(MLN::Net::Connection::ptr conn);
};
