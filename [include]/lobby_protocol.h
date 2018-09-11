#pragma once

#include <type_traits>
#include <stdint.h>

#pragma warning( push )
#pragma warning( disable : 4351 )


#ifndef CODE_VALUE_MACRO
#define CODE_VALUE_MACRO code = std::decay<decltype(*this)>::type::packet_value
#endif

#pragma pack(1)   

using TyUserIDX = uint32_t;
using TyGroupIDX = uint32_t;

using TyPacketCode = uint8_t;

namespace LOBBY_PROTOCOL
{
	struct PT_JSON
	{
		enum { packet_value = 1 };

		enum {
			MAX_BODY_SIZE = 30000,
			MAX_URL_STRING = 32,
			HEAD_SIZE = 40,
		};

		TyPacketCode	code;
		uint32_t	sequenceNo;
		int8_t		isCompressed = 0;
		int8_t		url[MAX_URL_STRING];
		uint16_t	bodySize = 0;
		int8_t		jsonBody[MAX_BODY_SIZE];

		PT_JSON() {
			CODE_VALUE_MACRO;
			memset(url, 0, sizeof(url));
		}
	};

	struct PT_HEARTBEAT
	{
		enum { packet_value = 3 };

		TyPacketCode	code;

		PT_HEARTBEAT() {
			CODE_VALUE_MACRO;
		}
	};

	struct PT_AUTHKEY_REQ
	{
		enum {
			packet_value = 5,
			KEY_SIZE = 20
		};

		TyPacketCode	code;
		int8_t		key[KEY_SIZE];

		PT_AUTHKEY_REQ() {
			CODE_VALUE_MACRO;
		}
	};

	struct PT_AUTHKEY_ACK
	{
		enum {
			packet_value = 6,
			KEY_SIZE = 20
		};

		TyPacketCode	code;
		int8_t		key[KEY_SIZE];

		PT_AUTHKEY_ACK() {
			CODE_VALUE_MACRO;
		}
	};

	struct PT_Test
	{
		enum {
			packet_value = 7,
			string_max = 32,
		};

		TyPacketCode	code;

		int32_t		a;
		int16_t		b;
		int8_t		c;
		int8_t		urlString[string_max];

		PT_Test() {
			CODE_VALUE_MACRO;
		}
	};

};// namespace LOBBY_PROTOCOL




#pragma pack()
#pragma warning ( pop )