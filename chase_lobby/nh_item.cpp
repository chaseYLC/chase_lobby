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

	// ������ ����.


	// ����ڿ� ����.
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

	if (1 == shop.type) {	// ��Ű�� ������
		LOGE << "package item. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "package item.";
		user->SendJsonPacket(url, rsp);
		return false;
	}
	else if (6 == shop.type) {	// ĳ���� �̱��

	//ĳ����ī�� �̱��1	6	601
	//ĳ����ī�� �̱��3	6	602
	//ĳ����ī�� �̱��5	6	603
	//ĳ����ī�� �̱��10	6	604


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

			// cnt ����ŭ ĳ���͸� ��÷�մϴ�.
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
			// ĳ���� 1�� ��÷.
			makeCharacter(CharGotchaRate::one, charType, charGrade, 1, madeMaxGrade);
			break;
		case 602:
			gotchaUnit = CharGotchaRate::three;

			if (MAX_CHARACTER_CNT <= (3 - 1 + user->m_charData.getCharCnt())) {
				maxCharError();
				return false;
			}
			// ĳ���� 3�� ��÷.
			makeCharacter(CharGotchaRate::three, charType, charGrade, 3, madeMaxGrade);
			break;
		case 603:
			gotchaUnit = CharGotchaRate::five;

			if (MAX_CHARACTER_CNT <= (5 - 1 + user->m_charData.getCharCnt())) {
				maxCharError();
				return false;
			}

			// ĳ���� (4+1)�� ��÷.

			// ĳ���� (4)�� ��÷.
			makeCharacter(CharGotchaRate::five, charType, charGrade, 4, madeMaxGrade);
			
			// 5�� �̱� �� �ּҵ��
			charGrade = DataTable::instance()->m_rivisionGradeMin_5;

			// ĳ���� (1)�� ��÷.
			if (charGrade <= madeMaxGrade) {
				makeCharacter(CharGotchaRate::five, charType, charGrade, 1, madeMaxGrade);
			}
			else {
				// 5�� �̱⿡ �ּҵ�� ������ �ȵǾ���. ��������.
				DataTable::instance()->CharacterGotcha(CharGotchaRate::Unit::five
					, false		// rivision
					, 1			// round. 1 ����.
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

			// 9���� normal�� ��÷.
			makeCharacter(CharGotchaRate::ten, charType, charGrade, 9, madeMaxGrade);

			if ((CHARACTER_GRADE_CNT - 1) <= madeMaxGrade) {
				// �ְ����� S+ ����� ��÷�ϴµ� ����.
				getSuperPlusCard = true;
				superCntCurrent = 0;
			}

			// 10��°�� rivision ���� ��÷.
			DataTable::instance()->CharacterGotcha(CharGotchaRate::Unit::ten
				, false		// rivision ���� ��÷.
				, superCntCurrent
				, charType
				, charGrade
			);
			makeCharacter(CharGotchaRate::ten, charType, charGrade, 1, madeMaxGrade, false);

			// �ٽ��ѹ� �����÷��� ī�� ��÷�ߴ��� �˻�.
			if ((CHARACTER_GRADE_CNT - 1) <= madeMaxGrade) {
				// �ְ����� S+ ����� ��÷�ϴµ� ����.
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


		// ����ڿ��� ������ ĳ���� ����.
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
		// �ش� �������� ����������.
		std::vector<std::pair<int, int> > items;
		items.emplace_back(shop.itemIDX1, shop.itemCnt1);
		if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
			// ���� �����ϰ� �������� �������� ����.
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
	// �ݾ� ����.
	if (false == DOC_MG->addItemList(user, user->m_resource_id_item
		, std::vector<std::pair<int, int>>(), ruby, gold, 0)) {
		LOGE << "failed update Item. userIDX : " << std::to_string(user->m_userIDX);
		rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
		rsp[RSP_RM] = "failed addMoney";
		user->SendJsonPacket(url, rsp);
		return;
	}

	// ������ ����.
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
	// 1. ������ ����

	switch (shop.type) {
	case 1:		// ��Ű��
	{
		// ���� ��������.
		if (false == ShopManager::instance()->isGoblinBuyable(user, shop)) {
			LOGE << "can't buyable item. userIDX : " << user->m_userIDX
				<< ", type : " << std::to_string(shop.type)
				<< ", shopidx : " << std::to_string(shop.idx);

			rsp[RSP_RC] = RSP_RC_SYSTEM_ERROR;
			rsp[RSP_RM] = "couldn't buy item.";
			user->SendJsonPacket(url, rsp);
			return;
		}

		// ��� ���ű�� ������Ʈ.
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
			//1. �ǹ� ��Ű�� ��ǰ ����(�� 1ȸ)
			//	��� ���� ����		 45���, 4, 500��� ����
			//	Ư�� ����1		�Ϸ� 10��� ����(30�ϰ�)
			//	Ư�� ����2		�Ϸ� 1, 000��� ����(30�ϰ�)

			// �ش� �������� ����������.
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
			//2. ��� ��Ű�� ��ǰ ����(�� 1ȸ)
			//	Ư�� ����1		 100���, 15, 000��� ����
			//	Ư�� ����2		���� ����ȭ ������ 1��
			//	Ư�� ����3		BS ���� ��ũ�� 10��
			//	Ư�� ����4		RS ���� ��ũ�� 10��
			//	Ư�� ����5		����� 30��

			// �ش� �������� ����������.
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
			//3. ��Ÿ�� ��Ű�� ��ǰ ����(������ 3ȸ)
			//	Ư�� ����1		��� ī�� 1��
			//	Ư�� ����2		BS ���� ��ũ�� 5��
			//	Ư�� ����3		RS ���� ��ũ�� 4��

			// �ش� �������� ����������.
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
			//4. ĳ���� ��ȭ ��Ű�� ��ǰ ����(������ 3ȸ)
			//	Ư�� ����1		��Ÿ ��ȭ ī�� 1��(�������� ��Ÿī��� �ѹ��� ��ȭ���� �ִ� Ư���� ī��)
			//	Ư�� ����2		50���, 5, 000��� ����

			// �ش� �������� ����������.
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
	case 2:		// ���
	case 3:		// �߻�ü
	case 5:		// �����
	{
		// �ش� �������� ����������.
		std::vector<std::pair<int, int> > items;
		items.emplace_back(shop.itemIDX1, shop.itemCnt1);
		if (false == PostManager::instance()->buyItemList(user, user->m_resource_id_post, items)) {
			// ���� �����ϰ� �������� �������� ����.
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


	// ���� OK.
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
	//netrsp_shop_buyrubygoods_nf	�ش� ��ǰ�� ã�� �� �����ϴ�.
		rsp[RSP_RC] = 1;
		rsp[RSP_RM] = "none of goods.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (shop.ruby != ruby) {
		//netrsp_data_ver	�������� ������ ������ ������ �ٸ��ϴ�.
		rsp[RSP_RC] = 2;
		rsp[RSP_RM] = "check price.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (user->m_accountData.ruby_ < ruby) {
		// �ݾ� ����.
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
		//netrsp_shop_buyrubygoods_nf	�ش� ��ǰ�� ã�� �� �����ϴ�.
		rsp[RSP_RC] = 1;
		rsp[RSP_RM] = "none of goods.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (shop.gold != gold) {
		//netrsp_data_ver	�������� ������ ������ ������ �ٸ��ϴ�.
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
		//netrsp_shop_buyrubygoods_nf	�ش� ��ǰ�� ã�� �� �����ϴ�.
		rsp[RSP_RC] = 1;
		rsp[RSP_RM] = "none of goods.";
		user->SendJsonPacket(url, rsp);
		return;
	}

	if (shop.cash != cash) {
		//netrsp_data_ver	�������� ������ ������ ������ �ٸ��ϴ�.
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

	// ������
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