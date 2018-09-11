#include "stdafx.h"
#include "lobbyUserManager.h"

#include <net/logger.h>
#include "brokerConnector.h"
#include "lobbyUser.h"

void LobbyUserManager::addUser(MLN::Net::Connection::ptr conn)
{
	if (conn->getTag() != nullptr)
	{
		LOGE << "created userobj already. ident: " << conn->getIdentity();
		return;
	}

	chase_lobbysvr::User *user = new chase_lobbysvr::User(conn);
	conn->setTag(user);

	boost::unique_lock<boost::shared_mutex> wlock(m_mtx_users);
	if (false == m_users.emplace(std::make_pair(conn->getIdentity(), conn)).second) {
		LOGE << "failed insert user. identity : " << std::to_string(conn->getIdentity());
		return;
	}

	Json::Value rsp;
	rsp[RSP_SEQ] = 0;
	rsp[RSP_RC] = 0;
	rsp[RSP_RM] = RSP_OK;
	user->SendJsonPacket("/account/connected", rsp);

	//boost::upgrade_lock<boost::shared_mutex> lock(m_mtx_users);	
	//boost::upgrade_to_unique_lock<boost::shared_mutex> wlock(lock);
}

void LobbyUserManager::deleteUser(MLN::Net::Connection::ptr conn)
{
	chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
	if (nullptr == user) {
		return;
	}

	user->logoutProcess();


	// 브로커로 로그아웃 통지.
	Json::Value brokerReq;
	brokerReq["userIDX"] = user->m_userIDX;
	brokerReq["nick"] = user->m_userID;

	BrokerConnector::sendJsonPacket("/userLogout", brokerReq);
	
	// 유저 리스트에서 해제.
	boost::unique_lock<boost::shared_mutex> wlock(m_mtx_users);

	
	// 유저맵(아이디기반)에서 제거
	m_nodeToUserMap.erase(user->m_userIDX);


	auto it = m_users.find(user->getConn()->getIdentity());
	if (m_users.end() == it) {
		LOGE << "none user. userIDX : " << std::to_string(user->m_userIDX);
		delete user;
		conn->setTag(nullptr);
		return;
	}

	m_users.erase(it);

	delete user;
	conn->setTag(nullptr);

}

int LobbyUserManager::processUser(const size_t identity, std::function< void(chase_lobbysvr::User *, void *) >callback, void *param1)
{
	boost::shared_lock<boost::shared_mutex> rlock(m_mtx_users);

	auto it = m_users.find(identity);
	if (m_users.end() == it) {
		return -1;
	}

	MLN::Net::Connection::ptr targetConn = it->second.lock();
	if (!targetConn) {
		return -2;
	}

	chase_lobbysvr::User *user = (chase_lobbysvr::User *)targetConn->getTag();
	if (nullptr == user) {
		return -3;
	}

	callback(user, param1);

	return 0;
}

int LobbyUserManager::processUser2(const size_t identity, std::function< int(chase_lobbysvr::User *, uint32_t, uint32_t) >callback, uint32_t param1, uint32_t param2)
{
	boost::shared_lock<boost::shared_mutex> rlock(m_mtx_users);

	auto it = m_users.find(identity);
	if (m_users.end() == it) {
		return -1;
	}

	MLN::Net::Connection::ptr targetConn = it->second.lock();
	if (!targetConn) {
		return -2;
	}

	chase_lobbysvr::User *user = (chase_lobbysvr::User *)targetConn->getTag();
	if (nullptr == user) {
		return -3;
	}

	return callback(user, param1, param2);
}

bool LobbyUserManager::userLogined(const size_t identity, const uint32_t userIDX, const std::string & accountID)
{
	boost::unique_lock<boost::shared_mutex> wlock(m_mtx_users);

	if (false == m_nodeToUserMap.emplace(userIDX, identity).second) {
		LOGE << "failed m_nodeToUserMap.emplace. userIDX : " << std::to_string(userIDX);
	}

	return true;
}

bool LobbyUserManager::userLogout(const size_t identity, const uint32_t userIDX, const std::string & accountID)
{
	boost::unique_lock<boost::shared_mutex> wlock(m_mtx_users);

	m_nodeToUserMap.erase(userIDX);

	return true;
}

boost::weak_ptr<MLN::Net::Connection> LobbyUserManager::getUser(const size_t identity)
{
	boost::shared_lock<boost::shared_mutex> rlock(m_mtx_users);

	auto pair = m_users.find(identity);
	if (m_users.end() == pair) {
		return boost::weak_ptr<MLN::Net::Connection>();
	}

	return pair->second;
}

void LobbyUserManager::sendPacketFromBroker(const size_t identity, const std::string &url, Json::Value &jv
	, std::function<void(chase_lobbysvr::User *)> userFunc) const
{
	boost::weak_ptr<MLN::Net::Connection> connWeak;

	{
		boost::shared_lock<boost::shared_mutex> rlock(m_mtx_users);

		auto pair = m_users.find(identity);
		if (m_users.end() == pair) {
			return;
		}

		connWeak = pair->second;
	}
	
	auto conn = connWeak.lock();
	if (conn && (nullptr != conn->getTag()) )
	{
		chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();

		userFunc(user);

		user->SendJsonPacket(url, jv);
	}
}

void LobbyUserManager::sendPacketFromBroker_byUserIDX(const uint32_t userIDX, const std::string &url, Json::Value &jv) const
{
	boost::weak_ptr<MLN::Net::Connection> connWeak;

	{
		boost::shared_lock<boost::shared_mutex> rlock(m_mtx_users);

		auto pairConn = m_nodeToUserMap.find(userIDX);
		if (m_nodeToUserMap.end() == pairConn) {
			return;
		}
		auto identity = pairConn->second;

		auto pair = m_users.find(identity);
		if (m_users.end() == pair) {
			return;
		}

		connWeak = pair->second;
	}

	auto conn = connWeak.lock();
	if (conn && (nullptr != conn->getTag()))
	{
		chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
		user->SendJsonPacket(url, jv);
	}
}

void LobbyUserManager::sendPacketFromBroker_All(const std::string &url, Json::Value &jv) const
{
	boost::shared_lock<boost::shared_mutex> rlock(m_mtx_users);

	for (auto &pair : m_users)
	{
		boost::weak_ptr<MLN::Net::Connection> connWeak = pair.second;

		auto conn = connWeak.lock();
		if (conn && (nullptr != conn->getTag()))
		{
			chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
			user->SendJsonPacket(url, jv);
		}
	}
}