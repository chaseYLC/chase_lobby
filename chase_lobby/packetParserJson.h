#pragma once

#include <net/messageProcedure.h>
#include <net/messageBuffer.h>
#include <functional>
#include <mutex>
#include <tuple>

#define JSON_PARSER_MAIN_KEY 0xffff0000
#define JSON_PARSER_PART_KEY 0xfff30000

#define JSON_PARSER_KEY_MAKEKEY(key, len)	((key & 0xffff0000) | (len & 0x0000ffff))
#define JSON_PARSER_KEY_GETKEY(key)			(key & 0xffff0000)
#define JSON_PARSER_KEY_GETLENGTH(key)		(key & 0x0000ffff)

struct JsonParserProposal
{
	enum {
		MAX_PACKET = 2048,
		PARTKEY_SIZE = 4,
		HEADER_SIZE = PARTKEY_SIZE + PARTKEY_SIZE,
	};
};

class message_header_manipulator_jsonParser
{
public:
	struct HEADER
	{
		int32_t mainKey;
		int32_t partKey;
	};

	static size_t getHeaderSize()
	{
		return sizeof(HEADER);
	}

	static void facilitate(size_t currentSize, char *buffer)
	{
		const int32_t  bodySize = (int32_t)(currentSize - sizeof(HEADER));

		((HEADER*)buffer)->mainKey = JSON_PARSER_KEY_MAKEKEY(JSON_PARSER_MAIN_KEY, JsonParserProposal::PARTKEY_SIZE + bodySize);
		((HEADER*)buffer)->partKey = JSON_PARSER_KEY_MAKEKEY(JSON_PARSER_PART_KEY, bodySize);
	}
};

class message_header_manipulator_jsonParser_Test
{
public:
	struct HEADER
	{
		int32_t mainKey;
		int32_t partKey;
	};

	static size_t getHeaderSize()
	{
		return sizeof(HEADER);
	}

	static void facilitate(size_t currentSize, char *buffer)
	{
		const int32_t  bodySize = (int32_t)(currentSize - sizeof(HEADER));

		((HEADER*)buffer)->mainKey = JsonParserProposal::PARTKEY_SIZE + bodySize;
		((HEADER*)buffer)->partKey = bodySize;
	}
};

class JSON_PACKET_PARSING_SUPPORT
{
public:
	static MLN::Net::MsgUserManip* getMsgManipulator();
	static MLN::Net::MsgUserManip* getMsgManipulatorTest();

	static bool packetParser(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);

	static bool packetParser_notUseEnc(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);

	static bool packetParser_xor(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);


	static bool packetParser_broker(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);

	static bool packetParser_brokerTest(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);


	static bool packetParser_notUseEncTest(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);
};
