#pragma once

#include <stdint.h>
#include <Sqltypes.h>

namespace DBStruct
{
	static const int MAX_userid = 50;
	static const int MAX_username = 50;
	static const int MAX_email = 50;
	static const int MAX_pass = 50;
	
	typedef struct _tag_userinfo {
		int nID = 0;
		std::string userid;
		std::string username;
		std::string email;
		std::string pass;
		int64_t gamemoney = 0;
		TIMESTAMP_STRUCT join_date;
	}userinfo;
}

#define DB_STRING_BIND(db, prefix, member)	db.Bind(prefix.##member, DBStruct::MAX_##member)