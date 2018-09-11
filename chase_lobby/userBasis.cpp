#include "stdafx.h"
#include "UserBasis.h"

#include <net/logger.h>
#include <net/messageBuffer.h>
#include <net/packetParserSimple.h>

namespace chase_lobbysvr {

	MLN::Net::EncType::Type UserBasis::m_encType = MLN::Net::EncType::GREETING;


	UserBasis::UserBasis(MLN::Net::Connection::ptr conn)
	{
		_conn = conn;

		//if (MLN::Net::EncType::AES_128_CBC == m_encType) {
		//	_encBlockAES_128_CBC.init(SERVER_SIDE_AES_128_CBC_INIT_FLAG);
		//}
	}


	bool UserBasis::initEncKey(const MLN::Net::EncType::Type encType, void *, char *buffer, const int bufferSize)
	{
		switch (encType)
		{
		case MLN::Net::EncType::GREETING:
			// send
			_conn->sendPacket("hello", 5);
			break;

			//case MLN::Net::EncType::AES_128_CBC:
			//{
			//	// write secret key
			//	char keyBuffer[MLN::Net::MessageBuffer::MAX_BUFFER_SIZE];

			//	IN OUT size_t keyBufferSize = sizeof(keyBuffer);

			//	if (false == _encBlockAES_128_CBC.getSecretKey(buffer, bufferSize, keyBuffer, keyBufferSize)) {
			//		return false;
			//	}

			//	// send
			//	_conn->sendPacket(keyBuffer, keyBufferSize);

			//}break;

			//case MLN::Net::EncType::AES_128_CBC:
			//{
			//	std::string serialized;
			//	_encBlockAES.genSerializedJsonKeys(serialized);

			//	MLN::Net::MessageBuffer msg;

			//	// write header
			//	msg.write((char*)&header, sizeof(header));
			//	msg.setManipulator(JSONMSG_PARSING_SUPPORT::getMsgManipulator_jsonMsg());

			//	// write body
			//	msg.write((char*)serialized.c_str(), serialized.size());

			//	// send
			//	_conn->send(msg);
			//}break;

			//case MLN::Net::EncType::AES_128_CBC_keR:
			//{
			//	std::string serialized;
			//	_encBlockAES_keR.genSerializedJsonKeys(buffer, bufferSize, serialized);

			//	MLN::Net::MessageBuffer msg;

			//	// write header
			//	msg.write((char*)&header, sizeof(header));
			//	msg.setManipulator(JSONMSG_PARSING_SUPPORT::getMsgManipulator_jsonMsg());

			//	// write body
			//	msg.write((char*)serialized.c_str(), serialized.size());

			//	// send
			//	_conn->send(msg);
			//}break;

		}//switch (encType)

		return true;
	}


	int32_t UserBasis::Send(char* data, uint32_t size)
	{
		switch (m_encType)
		{
		case MLN::Net::EncType::GREETING:
		{
			_conn->sendPacket(data, size);
			return size;
		}break;
		//case MLN::Net::EncType::AES_128_CBC:
		//{
		//	char msgBuffer[MLN::Net::MessageBuffer::MAX_BUFFER_SIZE];
		//	const int encryptedSize = _encBlockAES_128_CBC.Enc(data, size, msgBuffer);
		//	if (0 >= encryptedSize) {
		//		LOGI << "failed AES_128_CBCCryption.";
		//		return 0;
		//	}
		//	_conn->sendPacket(msgBuffer, encryptedSize);
		//	return encryptedSize;

		//	//MLN::Net::MessageBuffer msg(_conn->getMsgManip(), 0);

		//	//// write
		//	//msg.setManipulator(SIMPLE_PACKET_PARSING_SUPPORT::getMsgManipulator_dev3());

		//	//const int encryptedSize = _encBlockAES_128_CBC.Enc(data, size, msg.enableBuffer());
		//	//if (0 >= encryptedSize) {
		//	//	LOGI << "failed AES_128_CBCCryption.";
		//	//	return 0;
		//	//}
		//	//msg.write(encryptedSize);

		//	//_conn->send(msg);

		//	//return msg.size();

		//}break;
		}//switch (header.encType)


		return 0;
	}


	int32_t UserBasis::Decrypt(
		const MLN::Net::EncType::Type encType
		, char *entryptedData, const uint32_t entryptedDataSize
		, OUT char* decryptedBuffer, const uint32_t decryptedBufferSize)
	{
		switch (m_encType)
		{
			//case MLN::Net::EncType::AES_128_CBC:
			//{
			//	const int decryptedSize = _encBlockAES_128_CBC.Dec(entryptedData, entryptedDataSize, decryptedBuffer, decryptedBufferSize);
			//	if (-1 == decryptedSize) {
			//		LOGI << "failed AES_128_CBCDec.";
			//		return 0;
			//	}

			//	return decryptedSize;

			//}break;//case MLN::Net::EncType::AES_128_CBC:

		case MLN::Net::EncType::GREETING:
			break;

		}//switch (m_encType)

		return 0;
	}
}