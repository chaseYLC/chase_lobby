#include "stdafx.h"
#include "UserManagerBasis.h"
#include <net/logger.h>
#include <net/messageBuffer.h>

bool UserManagerBasis::createUser(const MLN::Net::EncType::Type encType, void * headerPtr, MLN::Net::Connection::ptr conn, uint32_t size, MLN::Net::MessageBuffer& msg)
{
	//if (conn->getTag() != nullptr)
	//{
	//	LOGE << "created userobj already. ident: " << conn->getIdentity();
	//	return false;
	//}

	//chase_lobbysvr::User *user = new chase_lobbysvr::User(conn);
	//conn->setTag(user);


	//switch (encType)
	//{
	//case MLN::Net::EncType::GREETING:
	//case MLN::Net::EncType::AES_128_CBC:
	//case MLN::Net::EncType::AES_128_CBC_keR:
	//{
	//	// send key
	//	if (false == user->initEncKey(encType, nullptr, msg.data(), size)) {
	//		return false;
	//	}
	//}break;

	//default:
	//	return false;
	//}//switch (encType)

	// // sync key.
	//conn->setSyncEncryptionKey(encType, true);



	//// 서버 시간 통지
	////ROOM_PROTOCOL::PT_SERVER_TIME serverTime;
	////typedef std::decay<decltype(ROOM_PROTOCOL::PT_SERVER_TIME::timeOnServer)>::type timeType;
	////serverTime.timeOnServer = (timeType)time(NULL);
	////user->Send(serverTime);

	return true;
}

bool UserManagerBasis::decryptMessage(const MLN::Net::EncType::Type encType, MLN::Net::Connection::ptr conn, uint32_t msgSize, MLN::Net::MessageBuffer& msg, MLN::Net::MessageBuffer& madeMsg)
{
	//chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
	//if (nullptr == user)
	//{
	//	LOGE << "user is null. ident: " << conn->getIdentity();
	//	return false;
	//}

	//madeMsg.clear();

	//const int decryptedSize = user->Decrypt(encType, msg.data(), msgSize, madeMsg.enableBuffer(), (uint32_t) madeMsg.remainWriteSize());
	//if (0 >= decryptedSize) {
	//	LOGE << "failed Decryption. ident: " << conn->getIdentity();
	//	madeMsg.clear();
	//	return false;
	//}
	//madeMsg.write(decryptedSize);

	return true;
}

void UserManagerBasis::onClose(MLN::Net::Connection::ptr conn)
{
	//chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
	//if (nullptr == user) {
	//	return;
	//}

	//delete user;
	//conn->setTag(nullptr);
}
