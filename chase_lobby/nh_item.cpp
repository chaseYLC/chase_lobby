#include "stdafx.h"
#include "nh_item.h"

#include "User.h"
#include "docdbManager.h"
#include "dataTable.h"
#include "postManager.h"
#include "shopManager.h"
#include "nh_account.h"

void nh_item::all(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];
	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;

	std::wstring res_id;

	bool result = false;
	Json::Value itemInfoDoc;
	if (true == user->m_resource_id_item.empty()) {
		result = DOC_MG->getItem(user->m_userIDX, itemInfoDoc, user->m_resource_id_item);
	}
	else {
		result = DOC_MG->getItem(user->m_resource_id_item, itemInfoDoc);
	}

	if (false == result) {
		LOGE << "failed getItem. userIDX : " << std::to_string(user->m_userIDX);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "system error";
		user->SendJsonPacket(url, rsp);
		return;
	}

	// 서버에 설정.


	// 사용자에 응답.
	rsp["itemInfo"] = {};
	rsp["itemInfo"].swap(itemInfoDoc);
	
	LOGD << rsp.toStyledString();

	user->SendJsonPacket(url, rsp);
}

bool nh_item::itemMake(const std::string &url, bob_lobbysvr::User *user, Json::Value &rsp, const Shop &shop)
{
	auto &maxCharError = [&]() {
		LOGE << "max Characters. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "max Characters.";
		user->SendJsonPacket(url, rsp);
	};

	if (MAX_CHARACTER_CNT <= user->m_charData.getCharCnt()) {
		LOGE << "max Characters. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "max Characters.";
		user->SendJsonPacket(url, rsp);
		return false;
	}

	if (1 == shop.type) {	// 패키지 아이템
		LOGE << "package item. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "package item.";
		user->SendJsonPacket(url, rsp);
		return false;
	}
	else if (6 == shop.type) {	// 캐릭터 뽑기권

	//캐릭터카드 뽑기권1	6	601
	//캐릭터카드 뽑기권3	6	602
	//캐릭터카드 뽑기권5	6	603
	//캐릭터카드 뽑기권10	6	604


		// superCardPlusCnt
		int superCntCurrent = user->m_charData.getSuperCnt();
		bool getSuperPlusCard = false;
		CharGotchaRate::Unit gotchaUnit = CharGotchaRate::one;

		const int64_t createdTime = bob_lobbysvr::User::getLocalTimeSec();

		std::vector<web::json::value> newCharacters;
		web::json::value newCharacter;

		auto getCharTypeAndGrade = [](const CharGotchaRate::Unit bunchSize, int &charType, int &charGrade) {
			DataTable::instance()->CharacterGotcha(bunchSize, true, 1, charType, charGrade);
		};

		auto makeCharacter = [&getCharTypeAndGrade, &newCharacter, &createdTime, &newCharacters]
		(const CharGotchaRate::Unit bunchSize, int &charType, int &charGrade, const int cnt, int &madeMaxGrade, bool randomGrade = true)
		{
			madeMaxGrade = 0;

			// cnt 수만큼 캐릭터를 추첨합니다.
			for (int i = 0; i < cnt; ++i) {

				if (true == randomGrade) {
					getCharTypeAndGrade(bunchSize, charType, charGrade);
				}
				
				nh_account::genCharFunc(charType, charGrade, newCharacter, 0, createdTime);
				newCharacters.push_back(newCharacter);

				if (madeMaxGrade < charGrade) {
					madeMaxGrade = charGrade;
				}
			}
		};

		int charType;
		int charGrade;
		int madeMaxGrade;

		switch (shop.itemIDX1) {
		case 601:
			gotchaUnit = CharGotchaRate::one;

			if (MAX_CHARACTER_CNT <= user->m_charData.getCharCnt()) {
				maxCharError();
				return false;
			}
			// 캐릭터 1개 추첨.
			makeCharacter(CharGotchaRate::one, charType, charGrade, 1, madeMaxGrade);
			break;
		case 602:
			gotchaUnit = CharGotchaRate::three;

			if (MAX_CHARACTER_CNT <= (3 - 1 + user->m_charData.getCharCnt())) {
				maxCharError();
				return false;
			}
			// 캐릭터 3개 추첨.
			makeCharacter(CharGotchaRate::three, charType, charGrade, 3, madeMaxGrade);
			break;
		case 603:
			gotchaUnit = CharGotchaRate::five;

			if (MAX_CHARACTER_CNT <= (5 - 1 + user->m_charData.getCharCnt())) {
				maxCharError();
				return false;
			}

			// 캐릭터 (4+1)개 추첨.

			// 캐릭터 (4)개 추첨.
			makeCharacter(CharGotchaRate::five, charType, charGrade, 4, madeMaxGrade);
			
			// 5장 뽑기 시 최소등급
			charGrade = DataTable::instance()->m_rivisionGradeMin_5;

			// 캐릭터 (1)개 추첨.
			if (charGrade <= madeMaxGrade) {
				makeCharacter(CharGotchaRate::five, charType, charGrade, 1, madeMaxGrade);
			}
			else {
				// 5장 뽑기에 최소등급 보장이 안되었음. 보정해줌.
				DataTable::instance()->CharacterGotcha(CharGotchaRate::Unit::five
					, false		// rivision
					, 1			// round. 1 고정.
					, charType
					, charGrade
				);

				makeCharacter(CharGotchaRate::five, charType, charGrade, 1, madeMaxGrade, false);
			}

			break;
		case 604:
			gotchaUnit = CharGotchaRate::ten;

			if (MAX_CHARACTER_CNT <= (10 - 1 + user->m_charData.getCharCnt())) {
				maxCharError();
				return false;
			}

			// 9장은 normal로 추첨.
			makeCharacter(CharGotchaRate::ten, charType, charGrade, 9, madeMaxGrade);

			if ((CHARACTER_GRADE_CNT - 1) <= madeMaxGrade) {
				// 최고등급인 S+ 등급을 추첨하는데 성공.
				getSuperPlusCard = true;
				superCntCurrent = 0;
			}

			// 10장째는 rivision 으로 추첨.
			DataTable::instance()->CharacterGotcha(CharGotchaRate::Unit::ten
				, false		// rivision 으로 추첨.
				, superCntCurrent
				, charType
				, charGrade
			);
			makeCharacter(CharGotchaRate::ten, charType, charGrade, 1, madeMaxGrade, false);

			// 다시한번 슈퍼플러스 카드 추첨했는지 검사.
			if ((CHARACTER_GRADE_CNT - 1) <= madeMaxGrade) {
				// 최고등급인 S+ 등급을 추첨하는데 성공.
				getSuperPlusCard = true;
				superCntCurrent = 0;
			}

			break;
		default:
			LOGE << "Not defined item. userIDX: " << std::to_string(user->m_userIDX) << ", itemIDX : " << std::to_string(shop.itemIDX1);

			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "Not defined item";
			user->SendJsonPacket(url, rsp);
			return false;
		}


		if (CharGotchaRate::ten == gotchaUnit
			&& false == getSuperPlusCard
			) {
			++superCntCurrent;
			user->m_charData.incSuperCnt();
		}

		if (false == DOC_MG->addCharacters(user, newCharacters, superCntCurrent, user->m_resource_id_character)) {
			LOGE << "Failed addCharacter DB-Error";

			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "Failed addCharacter DB-Error";
			user->SendJsonPacket(url, rsp);
			return false;
		}


		// 사용자에게 생성된 캐릭터 통지.
		Json::Value rspCreatedCharacaters;
		auto &charListRsp = rspCreatedCharacaters["charList"] = Json::Value(Json::arrayValue);

		for (const web::json::value & charEnt : newCharacters) {
			utility::string_t payload = charEnt.serialize();
#ifdef _WIN32
			std::string payload8(payload.begin(), payload.end());
#else
			std::string &payload8 = payload;
#endif
			Json::Reader reader;
			Json::Value charInfo = Json::Value(Json::objectValue);
			if (false == reader.parse(payload8, charInfo)) {
				LOGE << "invalid json string";

				rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
				rsp[RSP_RM] = "SystemError. invaild json.";
				user->SendJsonPacket("/character/created", rspCreatedCharacaters);
				return false;
			}

			charListRsp.append(charInfo);
		}//for (const web::json::value & charEnt : newCharacters) {

		rsp[RSP_RC] = 0;
		rsp[RSP_RM] = RSP_OK;;
		user->SendJsonPacket("/character/created", rspCreatedCharacaters);
	}
	else {
		// 해당 아이템을 우편함으로.
		std::vector<std::pair<int, int> > items;
		items.emplace_back(shop.itemIDX1, shop.itemCnt1);
		if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
			// 돈을 차감하고 아이템은 지급하지 못함.
			LOGE << "failed buy Item. userIDX : " << std::to_string(user->m_userIDX);
			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "failed buy Item";
			user->SendJsonPacket(url, rsp);
			return false;
		}
	}

	return true;
}

void nh_item::buyProcessor(const std::string &url, bob_lobbysvr::User *user, Json::Value &rsp, Shop &shop, int64_t ruby, int64_t gold)
{
	// 금액 차감.
	if (false == DOC_MG->addItemList(user, user->m_resource_id_item
		, std::vector<std::pair<int, int>>(), ruby, gold, 0)) {
		LOGE << "failed update Item. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed addMoney";
		user->SendJsonPacket(url, rsp);
		return;
	}

	// 아이템 생성.
	if (false == itemMake(url, user, rsp, shop)) {
		LOGE << "failed itemMake. userIDX: " << std::to_string(user->m_userIDX);
		return;
	}

	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;
	
	user->SendJsonPacket(url, rsp);
}

void nh_item::buyProcessor(const std::string &url, bob_lobbysvr::User *user, uint32_t sequenceNo, std::vector<Shop> &shopList, const int ruby)
{
	Json::Value rsp;
	rsp[RSP_SEQ] = sequenceNo;

	if (false == DOC_MG->addItemList(user, user->m_resource_id_item
		, std::vector<std::pair<int, int>>(), ruby, 0, 0)) {
		LOGE << "failed update Item. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed addMoney";
		user->SendJsonPacket(url, rsp);
		return;
	}

	for (const Shop & shop : shopList) {
		if (false == itemMake(url, user, rsp, shop)) {
			LOGE << "failed itemMake. userIDX: " << std::to_string(user->m_userIDX);
			return;
		}
	}

	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;

	user->SendJsonPacket(url, rsp);
}

void nh_item::cashProcessor(const std::string &url, bob_lobbysvr::User *user, Json::Value &rsp, Shop &shop)
{
	// 1. 아이템 지급

	switch (shop.type) {
	case 1:		// 패키지
	{
		// 구매 가능한지.
		if (false == ShopManager::instance()->isGoblinBuyable(user, shop)) {
			LOGE << "can't buyable item. userIDX : " << user->m_userIDX
				<< ", type : " << std::to_string(shop.type)
				<< ", shopidx : " << std::to_string(shop.idx);

			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "couldn't buy item.";
			user->SendJsonPacket(url, rsp);
			return;
		}

		// 고블린 구매기록 업데이트.
		if (false == ShopManager::instance()->buyGoblin(user, shop)) {
			LOGE << "failed buyGoblin. userIDX : " << user->m_userIDX
				<< ", type : " << std::to_string(shop.type)
				<< ", shopidx : " << std::to_string(shop.idx);

			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "failed buyGoblin.";
			user->SendJsonPacket(url, rsp);
			return;
		}

		switch (shop.idx) {
		case 101:
		{
			//1. 실버 패키지 상품 구성(월 1회)
			//	즉시 보상 내용		 45루비, 4, 500골드 지급
			//	특별 보상1		하루 10루비 지급(30일간)
			//	특별 보상2		하루 1, 000골드 지급(30일간)

			// 해당 아이템을 우편함으로.
			std::vector<std::pair<int, int> > items;
			items.emplace_back(201, 45);
			items.emplace_back(701, 4500);
			if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
				LOGE << "failed buy Item. userIDX : " << std::to_string(user->m_userIDX);
				rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
				rsp[RSP_RM] = "failed buy Item";
				user->SendJsonPacket(url, rsp);
				return;
			}
		}
			break;
		case 102:
		{
			//2. 골드 패키지 상품 구성(월 1회)
			//	특별 보상1		 100루비, 15, 000골드 지급
			//	특별 보상2		스탯 고정화 아이템 1장
			//	특별 보상3		BS 리셋 스크롤 10장
			//	특별 보상4		RS 리셋 스크롤 10장
			//	특별 보상5		생명수 30개

			// 해당 아이템을 우편함으로.
			std::vector<std::pair<int, int> > items;
			items.emplace_back(201, 100);
			items.emplace_back(701, 15000);
			items.emplace_back(903, 1);
			items.emplace_back(902, 10);
			items.emplace_back(901, 10);
			items.emplace_back(801, 30);
			if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
				LOGE << "failed buy Item. userIDX : " << std::to_string(user->m_userIDX);
				rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
				rsp[RSP_RM] = "failed buy Item";
				user->SendJsonPacket(url, rsp);
				return;
			}
		}
			break;
		case 103:
		{
			//3. 스타터 패키지 상품 구성(계정당 3회)
			//	특별 보상1		골든 카드 1장
			//	특별 보상2		BS 리셋 스크롤 5장
			//	특별 보상3		RS 리셋 스크롤 4장

			// 해당 아이템을 우편함으로.
			std::vector<std::pair<int, int> > items;
			items.emplace_back(1001, 1);
			items.emplace_back(902, 5);
			items.emplace_back(901, 4);
			if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
				LOGE << "failed buy Item. userIDX : " << std::to_string(user->m_userIDX);
				rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
				rsp[RSP_RM] = "failed buy Item";
				user->SendJsonPacket(url, rsp);
				return;
			}
		}
			break;
		case 104:
		{
			//4. 캐릭터 강화 패키지 상품 구성(계정당 3회)
			//	특별 보상1		스타 강화 카드 1장(무강에서 스타카드로 한번에 강화시켜 주는 특별한 카드)
			//	특별 보상2		50루비, 5, 000골드 지급

			// 해당 아이템을 우편함으로.
			std::vector<std::pair<int, int> > items;
			items.emplace_back(1001, 1);
			items.emplace_back(201, 50);
			items.emplace_back(701, 5000);
			if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
				LOGE << "failed buy Item. userIDX : " << std::to_string(user->m_userIDX);
				rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
				rsp[RSP_RM] = "failed buy Item";
				user->SendJsonPacket(url, rsp);
				return;
			}
		}
			break;
		default:
			LOGE << "not defined type. userIDX : " << user->m_userIDX
				<< ", type : " << std::to_string(shop.type)
				<< ", shopidx : " << std::to_string(shop.idx);

			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "not defined type";
			user->SendJsonPacket(url, rsp);
			return;
		}//switch (shop.idx)
	}
		break;
	case 2:		// 루비
	case 3:		// 발사체
	case 5:		// 응모권
	{
		// 해당 아이템을 우편함으로.
		std::vector<std::pair<int, int> > items;
		items.emplace_back(shop.itemIDX1, shop.itemCnt1);
		if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
			// 돈을 차감하고 아이템은 지급하지 못함.
			LOGE << "failed buy Item. userIDX : " << std::to_string(user->m_userIDX);
			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "failed buy Item";
			user->SendJsonPacket(url, rsp);
			return;
		}
	}
		break;
	default:
		LOGE << "not defined type. userIDX : " << user->m_userIDX
			<< ", type : " << std::to_string(shop.type)
			<< ", shopidx : " << std::to_string(shop.idx);

		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "not defined type";
		user->SendJsonPacket(url, rsp);
		return;
	}//switch 


	// 구매 OK.
	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp["notReadMailCnt"] = user->m_notReadMailCnt;
	user->SendJsonPacket(url, rsp);
}

void nh_item::buyRubyGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	int goodsIDX = jv["IDX"].asUInt();
	int ruby = jv["ruby"].asUInt();

	Shop shop;
	if (false == DataTable::instance()->getShop(goodsIDX, shop)) {
	//netrsp_shop_buyrubygoods_nf	해당 상품을 찾을 수 없습니다.
		rsp[RSP_RC] = 1;
		rsp[RSP_RM] = "none of goods.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (shop.ruby != ruby) {
		//netrsp_data_ver	실행중인 게임의 버젼이 서버와 다릅니다.
		rsp[RSP_RC] = 2;
		rsp[RSP_RM] = "check price.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (user->m_accountData.ruby_ < ruby) {
		// 금액 부족.
		rsp[RSP_RC] = 3;
		rsp[RSP_RM] = "check money.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	buyProcessor(url, user, rsp, shop, -ruby, 0);
}

void nh_item::buyGoldGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	int goodsIDX = jv["IDX"].asUInt();
	int64_t gold = jv["gold"].asUInt64();

	Shop shop;
	if (false == DataTable::instance()->getShop(goodsIDX, shop)) {
		//netrsp_shop_buyrubygoods_nf	해당 상품을 찾을 수 없습니다.
		rsp[RSP_RC] = 1;
		rsp[RSP_RM] = "none of goods.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (shop.gold != gold) {
		//netrsp_data_ver	실행중인 게임의 버젼이 서버와 다릅니다.
		rsp[RSP_RC] = 2;
		rsp[RSP_RM] = "check price.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (gold > (int64_t)user->m_accountData.gold_) {
		rsp[RSP_RC] = 3;
		rsp[RSP_RM] = "check money.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	buyProcessor(url, user, rsp, shop, 0, -gold);
}

void nh_item::buyCashGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	int goodsIDX = jv["IDX"].asUInt();
	int cash = jv["cash"].asUInt();

	Shop shop;
	if (false == DataTable::instance()->getShop(goodsIDX, shop)) {
		//netrsp_shop_buyrubygoods_nf	해당 상품을 찾을 수 없습니다.
		rsp[RSP_RC] = 1;
		rsp[RSP_RM] = "none of goods.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (shop.cash != cash) {
		//netrsp_data_ver	실행중인 게임의 버젼이 서버와 다릅니다.
		rsp[RSP_RC] = 2;
		rsp[RSP_RM] = "check price.";
		user->SendJsonPacket(url, rsp);
		return;
	}


	cashProcessor(url, user, rsp, shop);
}

void nh_item::buyMarinaGoods(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	uint32_t goodsKey = jv["goodsKey"].asUInt();
	int ruby = jv["ruby_discounted"].asUInt();

	if (ruby > user->m_accountData.ruby_) {
		LOGE << "check money. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "check money";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (false == ShopManager::instance()->buyMarina(url, jv[RSP_SEQ].asUInt(), user, goodsKey, ruby)) {
		LOGE << "failed buyMarina. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed buyMarina";
		user->SendJsonPacket(url, rsp);
	}
}

void nh_item::beg(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
#ifdef CBT_SPECIAL_DEFINE
	return;
#endif

	GET_USER(user);

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	int itemIDX = jv["itemIDX"].asUInt();
	int itemCnt = jv["itemCnt"].asUInt();

	if (0 == itemIDX
		|| 0 == itemCnt) {
		LOGE << "failed beg. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "check parameters.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (false == DOC_MG->beg(user->m_userIDX, itemIDX, itemCnt, user->m_resource_id_item)) {
		LOGE << "failed beg. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed beg";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (201 == itemIDX){
		user->m_accountData.addMoney(itemCnt, 0);
	}
	else if (701 == itemIDX) {
		user->m_accountData.addMoney(0, itemCnt);
	}
	else {
		user->m_accountData.addItem(itemIDX, itemCnt);
	}

	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	user->SendJsonPacket(url, rsp);
}

void nh_item::addAccountMoney(bob_lobbysvr::User *user, const int money, Json::Value &rsp, const std::string &url)
{
	if (false == DOC_MG->addAccountMoney(user->m_resource_id_account, user->m_accountID, money)) {
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed addAccountMoney";
		user->SendJsonPacket(url, rsp);
		return;
	}

	// 성공시
	user->m_accountData.addPurchaseMoney(money);

	std::vector<std::pair<int, int> > rewardItems;
	bool sixDayBonus = false;

	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	rsp["purchaseAcc"] = user->m_accountData.purchaseAcc_;
	user->SendJsonPacket(url, rsp);
}

void nh_item::testPurchaseAcc(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
#ifdef CBT_SPECIAL_DEFINE
	return;
#endif

	GET_USER(user);

	const int money = jv["money"].asUInt();

	Json::Value rsp;
	rsp[RSP_SEQ] = jv[RSP_SEQ];

	addAccountMoney(user, money, rsp, url);
}


void nh_item::shopList(MLN::Net::Connection::ptr conn, const std::string &url, Json::Value &jv)
{
	GET_USER(user);

	if (true == user->m_resource_id_shopData.empty()) {
		web::json::value jv;
		if (true == DOC_MG->getUserShopData(user->m_userIDX, jv, user->m_resource_id_shopData)) {
			user->m_shopInfo.setShopData(jv);
		}
	}

	Json::Value rsp;
	ShopManager::instance()->getDynamicGoods(user, rsp);

	rsp[RSP_SEQ] = jv[RSP_SEQ];
	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;

	user->SendJsonPacket(url, rsp);
}