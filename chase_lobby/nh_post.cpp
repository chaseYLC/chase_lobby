#include "stdafx.h"
#include "nh_post.h"

#include "User.h"
#include "docdbManager.h"

void nh_post::list(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	unsigned int page = jv["page"].asUInt();
	if (0 == page) {
		page = 1;
	}
	int pageSize = jv["pageSize"].asUInt();
	if (0 == pageSize) {
		pageSize = 10;
	}

	list_process(user, jv[RSP_SEQ].asUInt(), page, pageSize);
}

void nh_post::list_process(bob_lobbysvr::User *user, const uint32_t seq
	, const unsigned int page, const unsigned int pageSize)
{
	Json::Value rsp;
	rsp[RSP_SEQ] = seq;
	const static char *url = "/post/list";

	web::json::value postList;
	if (false == DOC_MG->getPostList(user, postList, user->m_resource_id_post)) {
		LOGE << "none of postBox. userIDX : " << std::to_string(user->m_userIDX);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed getPostList";
		user->SendJsonPacket(url, rsp);
		return;
	}

	int skipCnt = (page - 1) * pageSize;
	unsigned int pickCnt = 0;

	web::json::value docPick;
	docPick[U("postList")] = web::json::value::array();
	auto &arrPick = docPick[U("postList")].as_array();

	auto & arr = postList.as_array();
	auto it = arr.rbegin();
	for (; arr.rend() != it; ) {

		if (0 < skipCnt--) {
			++it;
			continue;
		}

		if (pageSize <= pickCnt) {
			break;
		}

		arrPick[pickCnt++] = *it;
		++it;
	}

	Json::Value postListJv;

	utility::string_t payload = docPick.serialize();
#ifdef _WIN32
	std::string payload8(payload.begin(), payload.end());
#else
	std::string &payload8 = payload;
#endif
	Json::Reader reader;
	if (false == reader.parse(payload8, postListJv)) {
		LOGE << "invalid json string";

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed jsonParsing";
		user->SendJsonPacket(url, rsp);
		return;
	}

	rsp.swap(postListJv);
	rsp["postTotal"] = arr.size();	// 메일 전체 갯수.
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;

	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp[RSP_SEQ] = seq;
	user->SendJsonPacket(url, rsp);

	LOGD << "mail-list : " << rsp.toStyledString();

	return;
}


void nh_post::read(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	unsigned int key = jv["key"].asUInt();
	unsigned int pub = jv["pub"].asUInt();

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	if (!key || !pub) {
		LOGE << "invalid param. idx : " << std::to_string(user->m_userIDX);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "invalid param";
		user->SendJsonPacket(url, rsp);
		return;
	}
	
	if(false == DOC_MG->readMail(user, user->m_resource_id_post, key, pub) ){
		LOGE << "failed readMail. userIDX : " << std::to_string(user->m_userIDX);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed readMail";
		user->SendJsonPacket(url, rsp);
		return;
	}

	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp[RSP_SEQ] = jv[RSP_SEQ];
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;

	user->SendJsonPacket(url, rsp);
	return;
}

//void nh_post::detail(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
//{
//	GET_USER(user);
//
//	auto sendErr = [&jv](const std::string &url, bob_lobbysvr::User *user) {
//		Json::Value rsp;
//		rsp[RSP_SEQ] = jv[RSP_SEQ];
//
//		rsp[RSP_RC] = 1;
//		rsp[RSP_RM] = "not found mail";
//		user->SendJsonPacket(url, rsp);
//	};
//
//	uint32_t key1 = jv["key1"].asUInt();
//	uint32_t key2 = jv["key2"].asUInt();
//
//	Json::Value rsp;
//	rsp[RSP_SEQ] = jv[RSP_SEQ];
//	rsp[RSP_RC] = 0;
//	rsp[RSP_RM] = RSP_OK;
//
//	Json::Value postDoc;
//	std::wstring res_id;
//	if (false == DOC_MG->getPostList(user->m_userIDX, postDoc, res_id)) {
//		// 사용자가 없거나 문서를 찾을 수 없음.
//		sendErr(url, user);
//		return;
//	}
//	user->m_resource_id_post = res_id;
//
//	auto postList = postDoc["postList"];
//
//	for (auto it = postList.begin(); postList.end() != it; ++it) {
//		if ((*it)["key1"].asUInt() != key1) continue;
//		if ((*it)["key2"].asUInt() != key2) continue;
//
//		LOGD << "mail detail : " << it->toStyledString();
//
//		rsp["post"] = *it;
//		user->SendJsonPacket(url, rsp);
//		return;  // succes~
//	}
//
//	sendErr(url, user);
//	return;
//}

void nh_post::get(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	uint32_t key = jv["key"].asUInt();
	uint32_t pub = jv["pub"].asUInt();
	/*uint32_t page = jv["page"].asUInt();*/

	Json::Value rsp;
	if (false == DOC_MG->getMail(user, user->m_resource_id_post, user->m_resource_id_item
		, key, pub, rsp)) {
		rsp[RSP_SEQ] = jv[RSP_SEQ];
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed getMail.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	rsp[RSP_SEQ] = jv[RSP_SEQ];
	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;

	user->SendJsonPacket(url, rsp);


	//// 메일 목록 갱신시켜준다.
	//list_process(user, 0, page, 10);
}

void nh_post::getAll(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	const uint32_t page = jv["page"].asUInt();
	const uint32_t pageSize = 10;

	web::json::value postList;
	if (false == DOC_MG->getPostList(user, postList, user->m_resource_id_post)) {
		LOGE << "none of postBox. userIDX : " << std::to_string(user->m_userIDX);

		Json::Value rsp;
		rsp[RSP_SEQ] = jv[RSP_SEQ];
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed getPostList";
		user->SendJsonPacket(url, rsp);
		return;
	}

	int skipCnt = (page - 1) * pageSize;
	int pickCnt = 0;
	int passCnt = 0;

	
	web::json::value docPick;
	docPick[U("postList")] = web::json::value::array();
	auto &arrPick = docPick[U("postList")].as_array();
	std::list<int> posForDelete;
	auto & arr = postList.as_array();

	int deleteMailCnt = 0;

	{// 메일을 수집.
		auto it = arr.rbegin();
		size_t pos = arr.size() - 1;
		for (; arr.rend() != it; ) {

			if (0 < skipCnt--) {
				++it;
				--pos;
				continue;
			}

			if (pageSize <= (pickCnt + passCnt) ) {
				break;
			}

			if (2 == (*it)[U("getType")].as_integer()) {
				++passCnt;
				// 공지사항은 스킵.
				++it;
				--pos;
				continue;
			}

			if (false == (*it)[U("read")].as_bool()) {
				++deleteMailCnt;
			}

			arrPick[pickCnt++] = *it;
			posForDelete.push_back((int) pos--);

			++it;
		}
	}

	// 메일을 삭제.
	user->m_notReadMailCnt -= deleteMailCnt;
	if (false == DOC_MG->deleteMailList(user->m_userIDX, user->m_resource_id_post, posForDelete, user->m_notReadMailCnt)) {
		LOGE << "failed delete mail. userIDX: " << std::to_string(user->m_userIDX);

		// 실패시 메일 갯수 회복.
		user->m_notReadMailCnt += deleteMailCnt;
	}
	

	// 아이템을 지급.
	if (false == DOC_MG->postListToItem(user, user->m_resource_id_item, arrPick)) {
		Json::Value rsp;
		rsp[RSP_SEQ] = jv[RSP_SEQ];
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed postListToItem";
		user->SendJsonPacket(url, rsp);
		return;
	}

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];
	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;
	user->SendJsonPacket(url, rsp);
	

	// 

	//// 메일 목록 갱신시켜준다.
	//list_process(user, 0, page, 10);
}

void nh_post::sendNote(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{

}


void nh_post::del(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	unsigned int key = jv["key"].asUInt();
	unsigned int pub = jv["pub"].asUInt();

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	if (!key || !pub) {
		LOGE << "invalid param. idx : " << std::to_string(user->m_userIDX);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "invalid param";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (false == DOC_MG->deleteMail(user, key, pub, user->m_resource_id_post, user->m_notReadMailCnt)) {
		LOGE << "failed deleteMail. userIDX : " << std::to_string(user->m_userIDX);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed deleteMail";
		user->SendJsonPacket(url, rsp);
		return;
	}

	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp[RSP_SEQ] = jv[RSP_SEQ];
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;
	user->SendJsonPacket(url, rsp);
}


void nh_post::devDelAll(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
#ifdef CBT_SPECIAL_DEFINE
	return;
#endif

	GET_USER(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	if (false == DOC_MG->deleteMailAll(user->m_userIDX, user->m_resource_id_post)) {
		LOGE << "failed deleteMailAll. userIDX : " << std::to_string(user->m_userIDX);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed deleteMailAll";
		user->SendJsonPacket(url, rsp);
		return;
	}

	
	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	user->m_notReadMailCnt = 0;
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;

	user->SendJsonPacket(url, rsp);
}