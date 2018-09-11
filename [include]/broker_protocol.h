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

namespace BROKER_PROTOCOL
{
	struct PT_JSON
	{
		enum { packet_value = 1 };

		enum {
			MAX_BODY_SIZE = 4000,
			MAX_URL_STRING = 32,
			HEAD_SIZE = 38,
		};

		uint32_t	code;
		int8_t		url[MAX_URL_STRING];
		uint16_t	bodySize = 0;
		int8_t		jsonBody[MAX_BODY_SIZE];

		PT_JSON() {
			CODE_VALUE_MACRO;
			memset(url, 0, sizeof(url));
		}
	};

};// namespace BROKER_PROTOCOL




#pragma pack()
#pragma warning ( pop )