#include "stdafx.h"
#include "packetParserJson.h"
#include <net/packetEncType.h>
#include <net/eventReceiver.h>
#include "lobbyAcceptorReceiver.h"
#include "brokerConnector.h"
#include "lobby_protocol.h"

static std::once_flag message_header_json_packet_manipulator_flag;
static std::once_flag message_header_json_packet_manipulator_flagTest;

MLN::Net::MsgUserManip* JSON_PACKET_PARSING_SUPPORT::getMsgManipulator()
{
	static MLN::Net::MsgUserManip msgMainp;

	std::call_once(message_header_json_packet_manipulator_flag
		, [&] {

		msgMainp = std::make_tuple(
			message_header_manipulator_jsonParser::getHeaderSize
			, message_header_manipulator_jsonParser::facilitate
			);
	});

	return &msgMainp;
}

MLN::Net::MsgUserManip* JSON_PACKET_PARSING_SUPPORT::getMsgManipulatorTest()
{
	static MLN::Net::MsgUserManip msgMainp;

	std::call_once(message_header_json_packet_manipulator_flagTest
		, [&] {

		msgMainp = std::make_tuple(
			message_header_manipulator_jsonParser_Test::getHeaderSize
			, message_header_manipulator_jsonParser_Test::facilitate
		);
	});

	return &msgMainp;
}


bool JSON_PACKET_PARSING_SUPPORT::packetParser(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
	, MLN::Net::MessageProcedure &msg_proc
	, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
	, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap)
{
	using namespace MLN::Net;

	static const uint8_t defaultCipherType = MLN::Net::EncType::AES_128_CBC;

	bool readStartFromMainKey = true;

	message_header_manipulator_jsonParser::HEADER header;
	const size_t sizeHeader = sizeof(header);

	uint32_t total_size = 0;

	do {
		if (true == readStartFromMainKey) {
			if (false == msg->readable((char*)&header, sizeHeader)) {
				break;
			}

			// mainKey check.
			if (JSON_PARSER_KEY_GETKEY(header.mainKey) != 0xffff0000) {
				msg->readAll();
				return false;
			}

			total_size = JSON_PARSER_KEY_GETLENGTH(header.mainKey) + sizeof(header.mainKey);
			if (false == msg->readable(total_size)) {
				break;
			}
			msg->read(sizeof(header.mainKey));
		}
		else {

		}

		msg->read((char*)&header.partKey, sizeof(header.partKey));
		// partKey Check
		if (JSON_PARSER_KEY_GETKEY(header.partKey) != 0xfff30000)
		{
			// err
			msg->readAll();
			return false;
		}
		uint32_t part_body_size = JSON_PARSER_KEY_GETLENGTH(header.partKey);
		uint32_t part_size = part_body_size + sizeof(header.partKey);
		if (false == msg->readable(part_body_size)) {
			// err
			msg->readAll();
			return false;
		}

		total_size -= part_size;

		readStartFromMainKey = (total_size == sizeof(header.partKey));

		MessageBuffer procMsg;

		auto ciperData = (MessageProcedure::NOT_SET_CIPHER_VALUE == msg_proc.getUserCipherType())
			? msg_proc.getCipherHandler(defaultCipherType)
			: msg_proc.getCipherHandler((uint8_t)msg_proc.getUserCipherType());

		if (std::get<1>(ciperData))
		{
			auto ciperType = std::get<0>(ciperData);
			auto cipherInitHandler = std::get<1>(ciperData);
			auto cipherDecHandler = std::get<2>(ciperData);

			if (false == spConn->isSyncEncryptionKey(ciperType)) {
				if (false == cipherInitHandler(ciperType, nullptr, spConn, part_body_size, *msg.get())) {
					msg->readAll();
					return false;
				}

				if (false == msg->readable(part_body_size)) {
					msg->readAll();
					return false;
				}
				msg->read(part_body_size);
				continue;
			}
			else {
				if (MLN::Net::EncType::GREETING == ciperType) {
					msg->read(procMsg.enableBuffer(), part_body_size);
					procMsg.write(part_body_size);
				}
				else {
					assert(NULL != cipherDecHandler);
					cipherDecHandler(ciperType, spConn, part_body_size, *msg.get(), procMsg);
					msg->read(part_body_size);

					if (0 >= procMsg.size()) {
						break;
					}
				}
			}
		}//if (nullptr != std::get<1>(ciperData))
		else {
			msg->read(procMsg.enableBuffer(), part_body_size);
			procMsg.write(part_body_size);
		}

		int32_t packetCode = 0;
		procMsg.readable((char*)&packetCode, sizeof(packetCode));

		auto it = memberFuncMap.find(packetCode);
		if (memberFuncMap.end() != it) {
			it->second(spConn, part_body_size, procMsg);
		}
		else {
			auto it = staticFuncMap.find(packetCode);
			if (staticFuncMap.end() != it) {
				it->second(spConn, part_body_size, procMsg);
			}
			else {
				spConn->getReceiver()->noHandler(spConn, procMsg);
				msg->readAll();
				return false;
			}
		}

	} while (true);

	return true;
}

bool JSON_PACKET_PARSING_SUPPORT::packetParser_notUseEnc(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
	, MLN::Net::MessageProcedure &msg_proc
	, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
	, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap)
{
	using namespace MLN::Net;
	bool readStartFromMainKey = true;

	message_header_manipulator_jsonParser::HEADER header;
	const size_t sizeHeader = sizeof(header);

	uint32_t total_size = 0;

	do {
		if (true == readStartFromMainKey) {
			if (false == msg->readable((char*)&header, sizeHeader)) {
				break;
			}

			// mainKey check.
			if (JSON_PARSER_KEY_GETKEY(header.mainKey) != 0xffff0000) {
				/*LOG_SERVICE_ERR("instance", "Session[%d] %s - %u - current = %X", ident, __FUNCTION__, __LINE__, recv_buffer.GetCursor());*/
				msg->readAll();
				return false;
			}

			total_size = JSON_PARSER_KEY_GETLENGTH(header.mainKey) + sizeof(header.mainKey);
			if (false == msg->readable(total_size)) {
				break;
			}
			msg->read(sizeof(header.mainKey));
		}
		else {

		}

		msg->read((char*)&header.partKey, sizeof(header.partKey));
		// partKey Check
		if (JSON_PARSER_KEY_GETKEY(header.partKey) != 0xfff30000)
		{
			// err
			msg->readAll();
			return false;
		}
		uint32_t part_body_size = JSON_PARSER_KEY_GETLENGTH(header.partKey);
		uint32_t part_size = part_body_size + sizeof(header.partKey);
		if (false == msg->readable(part_body_size)) {
			// err
			msg->readAll();
			return false;
		}

		total_size -= part_size;

		readStartFromMainKey = (total_size == sizeof(header.partKey));

		MLN::Net::MessageBuffer procMsg;
		msg->read(procMsg.enableBuffer(), part_body_size);
		procMsg.write(part_body_size);

		int32_t packetCode = 0;
		procMsg.readable((char*)&packetCode, sizeof(packetCode));

		switch (packetCode) {
		case LOBBY_PROTOCOL::PT_JSON::packet_value:
			LobbyAcceptorReceiver::readJsonPacket(spConn, part_body_size, procMsg);
			break;
		case LOBBY_PROTOCOL::PT_HEARTBEAT::packet_value:
			// 엔진에서 처리하므로.
			//LobbyAcceptorReceiver::recvHeartBeat(spConn, part_body_size, procMsg);
			break;
		default:
			spConn->getReceiver()->noHandler(spConn, procMsg);
			msg->readAll();
			return false;
		}

	} while (true);

	return true;
}

bool JSON_PACKET_PARSING_SUPPORT::packetParser_xor(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
	, MLN::Net::MessageProcedure &msg_proc
	, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
	, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap)
{
	using namespace MLN::Net;
	bool readStartFromMainKey = true;

	message_header_manipulator_jsonParser::HEADER header;
	const size_t sizeHeader = sizeof(header);

	uint32_t total_size = 0;

	do {
		if (true == readStartFromMainKey) {
			if (false == msg->readable((char*)&header, sizeHeader)) {
				break;
			}

			// mainKey check.
			if (JSON_PARSER_KEY_GETKEY(header.mainKey) != 0xffff0000) {
				/*LOG_SERVICE_ERR("instance", "Session[%d] %s - %u - current = %X", ident, __FUNCTION__, __LINE__, recv_buffer.GetCursor());*/
				msg->readAll();
				return false;
			}

			total_size = JSON_PARSER_KEY_GETLENGTH(header.mainKey) + sizeof(header.mainKey);
			if (false == msg->readable(total_size)) {
				break;
			}
			msg->read(sizeof(header.mainKey));
		}
		else {

		}

		msg->read((char*)&header.partKey, sizeof(header.partKey));
		// partKey Check
		if (JSON_PARSER_KEY_GETKEY(header.partKey) != 0xfff30000)
		{
			// err
			msg->readAll();
			return false;
		}
		uint32_t part_body_size = JSON_PARSER_KEY_GETLENGTH(header.partKey);
		uint32_t part_size = part_body_size + sizeof(header.partKey);
		if (false == msg->readable(part_body_size)) {
			// err
			msg->readAll();
			return false;
		}

		total_size -= part_size;

		readStartFromMainKey = (total_size == sizeof(header.partKey));

		MLN::Net::MessageBuffer procMsg;

		char *startBufPos = procMsg.enableBuffer();//
		msg->read(procMsg.enableBuffer(), part_body_size);
		procMsg.write(part_body_size);

		static char keys[6] = "IgNiS";
		for (int j = 4; j >= 0; --j) {
			char c = keys[j];
			char *curPos = startBufPos;
			for (int i = 0; i < part_body_size; ++i) {
				*curPos = (*curPos) ^ c;
				++curPos;
			}
		}

		int32_t packetCode = 0;
		procMsg.readable((char*)&packetCode, sizeof(packetCode));

		switch (packetCode) {
		case LOBBY_PROTOCOL::PT_JSON::packet_value:
			LobbyAcceptorReceiver::readJsonPacket(spConn, part_body_size, procMsg);
			break;
		case LOBBY_PROTOCOL::PT_HEARTBEAT::packet_value:
			// 엔진에서 처리하므로.
			//LobbyAcceptorReceiver::recvHeartBeat(spConn, part_body_size, procMsg);
			break;
		default:
			spConn->getReceiver()->noHandler(spConn, procMsg);
			msg->readAll();
			return false;
		}

	} while (true);

	return true;
}


bool JSON_PACKET_PARSING_SUPPORT::packetParser_broker(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
	, MLN::Net::MessageProcedure &msg_proc
	, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
	, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap)
{
	using namespace MLN::Net;
	bool readStartFromMainKey = true;

	message_header_manipulator_jsonParser::HEADER header;
	const size_t sizeHeader = sizeof(header);

	uint32_t total_size = 0;

	do {
		if (true == readStartFromMainKey) {
			if (false == msg->readable((char*)&header, sizeHeader)) {
				break;
			}

			// mainKey check.
			if (JSON_PARSER_KEY_GETKEY(header.mainKey) != 0xffff0000) {
				/*LOG_SERVICE_ERR("instance", "Session[%d] %s - %u - current = %X", ident, __FUNCTION__, __LINE__, recv_buffer.GetCursor());*/
				msg->readAll();
				return false;
			}

			total_size = JSON_PARSER_KEY_GETLENGTH(header.mainKey) + sizeof(header.mainKey);
			if (false == msg->readable(total_size)) {
				break;
			}
			msg->read(sizeof(header.mainKey));
		}
		else {

		}

		msg->read((char*)&header.partKey, sizeof(header.partKey));
		// partKey Check
		if (JSON_PARSER_KEY_GETKEY(header.partKey) != 0xfff30000)
		{
			// err
			msg->readAll();
			return false;
		}
		uint32_t part_body_size = JSON_PARSER_KEY_GETLENGTH(header.partKey);
		uint32_t part_size = part_body_size + sizeof(header.partKey);
		if (false == msg->readable(part_body_size)) {
			// err
			msg->readAll();
			return false;
		}

		total_size -= part_size;

		readStartFromMainKey = (total_size == sizeof(header.partKey));

		MLN::Net::MessageBuffer procMsg;
		msg->read(procMsg.enableBuffer(), part_body_size);
		procMsg.write(part_body_size);

		int32_t packetCode = 0;
		procMsg.readable((char*)&packetCode, sizeof(packetCode));

		if (1 == packetCode) {
			BrokerConnector::readJsonPacket(spConn, part_body_size, procMsg);
		}
		else {
			spConn->getReceiver()->noHandler(spConn, procMsg);
			msg->readAll();
			return false;
		}

	} while (true);

	return true;
}

bool JSON_PACKET_PARSING_SUPPORT::packetParser_brokerTest(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
	, MLN::Net::MessageProcedure &msg_proc
	, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
	, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap)
{
	using namespace MLN::Net;
	bool readStartFromMainKey = true;

	message_header_manipulator_jsonParser::HEADER header;
	const size_t sizeHeader = sizeof(header);

	uint32_t total_size = 0;

	do {
		if (true == readStartFromMainKey) {
			if (false == msg->readable((char*)&header, sizeHeader)) {
				break;
			}

			total_size = header.mainKey + sizeof(header.mainKey);

			if (false == msg->readable(total_size)) {
				break;
			}
			msg->read(sizeof(header.mainKey));
		}
		else {

		}

		msg->read((char*)&header.partKey, sizeof(header.partKey));

		uint32_t part_body_size = header.partKey;
		uint32_t part_size = part_body_size + sizeof(header.partKey);

		if (false == msg->readable(part_body_size)) {
			// err
			msg->readAll();
			return false;
		}

		total_size -= part_size;

		readStartFromMainKey = (total_size == sizeof(header.partKey));

		MLN::Net::MessageBuffer procMsg;
		msg->read(procMsg.enableBuffer(), part_body_size);
		procMsg.write(part_body_size);

		int32_t packetCode = 0;
		procMsg.readable((char*)&packetCode, sizeof(packetCode));

		if (1 == packetCode) {
			BrokerConnector::readJsonPacket(spConn, part_body_size, procMsg);
		}
		else {
			spConn->getReceiver()->noHandler(spConn, procMsg);
			msg->readAll();
			return false;
		}

	} while (true);

	return true;
}


bool JSON_PACKET_PARSING_SUPPORT::packetParser_notUseEncTest(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
	, MLN::Net::MessageProcedure &msg_proc
	, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
	, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap)
{
	using namespace MLN::Net;
	bool readStartFromMainKey = true;

	message_header_manipulator_jsonParser::HEADER header;
	const size_t sizeHeader = sizeof(header);

	uint32_t total_size = 0;

	do {
		if (true == readStartFromMainKey) {

			if (false == msg->readable((char*)&header, sizeHeader)) {
				break;
			}

			//// mainKey check.
			//if (JSON_PARSER_KEY_GETKEY(header.mainKey) != 0xffff0000) {
			//	/*LOG_SERVICE_ERR("instance", "Session[%d] %s - %u - current = %X", ident, __FUNCTION__, __LINE__, recv_buffer.GetCursor());*/
			//	msg->readAll();
			//	return false;
			//}

			total_size = header.mainKey + sizeof(header.mainKey);

			LOGI << "Total size : " << std::to_string(total_size);

			if (false == msg->readable(total_size)) {
				break;
			}
			msg->read(sizeof(header.mainKey));
		}
		else {

		}

		msg->read((char*)&header.partKey, sizeof(header.partKey));
		//// partKey Check
		//if (JSON_PARSER_KEY_GETKEY(header.partKey) != 0xfff30000)
		//{
		//	// err
		//	msg->readAll();
		//	return false;
		//}
		uint32_t part_body_size = header.partKey;
		uint32_t part_size = part_body_size + sizeof(header.partKey);
		if (false == msg->readable(part_body_size)) {
			// err
			LOGI << "failed. remain Size : " << std::to_string(msg->size()) << ", but partSize:" << std::to_string(part_size);
			msg->readAll();
			return false;
		}

		total_size -= part_size;

		readStartFromMainKey = (total_size == sizeof(header.partKey));

		MLN::Net::MessageBuffer procMsg;
		msg->read(procMsg.enableBuffer(), part_body_size);
		procMsg.write(part_body_size);

		//
		//int32_t packetCode = 0;
		uint8_t packetCode = 0;
		procMsg.readable((char*)&packetCode, sizeof(packetCode));

		LOGI << "Packet Code : " << std::to_string(packetCode);

		switch (packetCode) {
		case LOBBY_PROTOCOL::PT_JSON::packet_value:
			LobbyAcceptorReceiver::readJsonPacket(spConn, part_body_size, procMsg);
			break;
		case LOBBY_PROTOCOL::PT_HEARTBEAT::packet_value:
			// 엔진에서 처리하므로.
			//LobbyAcceptorReceiver::recvHeartBeat(spConn, part_body_size, procMsg);
			break;
		default:
			spConn->getReceiver()->noHandler(spConn, procMsg);
			msg->readAll();
			return false;
		}

	} while (true);

	return true;
}