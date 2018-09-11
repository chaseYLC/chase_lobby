#include "stdafx.h"
#include "lobbyUser.h"
//#include <OpenLib/zlib/zlib.h>
#include "MLN_Utils.h"
#include "GZipCodec.h"

namespace chase_lobbysvr {
	int User::SendJsonPacket(const std::string &url, Json::Value &jv)
	{
		if (false == jv[RSP_RM].isNull()) {
			jv[RSP_RM] = MLN_Utils::MB2UTF8(jv[RSP_RM].asCString());
		}

		uint32_t packetSequenceNo = 0;
		if (false == jv[RSP_SEQ].isNull()) {
			packetSequenceNo = jv[RSP_SEQ].asUInt();
		}
		
		Json::FastWriter fastWritter;
		std::string serialized = fastWritter.write(jv);

		LOGD << "send Json : " << serialized;

		LOBBY_PROTOCOL::PT_JSON pk;

		// set seq.
		pk.sequenceNo = packetSequenceNo;

		// set URL..
		memcpy(pk.url, url.c_str(), url.length());

		//  set Body Size..
		const int rawBodySize = (int)serialized.length() + 1;	// 널 문자 추가.

		if (1000 > rawBodySize) {
			//  set Body String..
			memcpy(pk.jsonBody, serialized.c_str(), serialized.length());
			memset(pk.jsonBody + serialized.length(), 0, 1);	// 널 문자 추가.
			pk.bodySize = rawBodySize;
		}
		else {
			pk.isCompressed = true;

			if (false == GZipCodec::CompressToBuffer(serialized, (unsigned char*)pk.jsonBody, pk.bodySize)) {
				LOGE << "failed compress.";
				pk.isCompressed = false;

				memcpy(pk.jsonBody, serialized.c_str(), serialized.length());
				memset(pk.jsonBody + serialized.length(), 0, 1);	// 널 문자 추가.
				
				pk.bodySize = rawBodySize;
			}
		}

		if (pk.bodySize > pk.MAX_BODY_SIZE) {
			LOGE << "jsonBody overflow. userIDX : " << std::to_string(m_userIDX);
			return 0;
		}

		// send ~!
		return this->Send((char*)&pk, LOBBY_PROTOCOL::PT_JSON::HEAD_SIZE + pk.bodySize);
	}

	void User::recvHeartBeat()
	{
	}

	void User::logoutProcess()
	{
	}

}//namespace chase_lobbysvr
