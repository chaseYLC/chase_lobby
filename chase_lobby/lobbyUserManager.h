#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <boost/weak_ptr.hpp>
#include <net/Singleton.h>
#include <map>
#include <functional>
#include "userManagerBasis.h"

namespace chase_lobbysvr {
	class User;
}

#define DUMMY_UM_HOOKER		[](chase_lobbysvr::User *){}

class LobbyUserManager
	: public UserManagerBasis
	, public MLN::Net::SingletonLight<LobbyUserManager>
{
public:
	void addUser(MLN::Net::Connection::ptr conn);
	void deleteUser(MLN::Net::Connection::ptr conn);

	int processUser(const size_t identity, std::function< void(chase_lobbysvr::User *, void *) >callback, void *param1);
	int processUser2(const size_t identity, std::function< int(chase_lobbysvr::User *, uint32_t, uint32_t) >callback, uint32_t param1, uint32_t param2);

	bool userLogined(const size_t identity, const uint32_t userIDX, const std::string & accountID);
	bool userLogout(const size_t identity, const uint32_t userIDX, const std::string & accountID);

	void sendPacketFromBroker(const size_t identity, const std::string &url, Json::Value &jv
		, std::function<void(chase_lobbysvr::User *)> userFunc) const;
	void sendPacketFromBroker_byUserIDX(const uint32_t userIDX, const std::string &url, Json::Value &jv) const;
	void sendPacketFromBroker_All(const std::string &url, Json::Value &jv) const;
	boost::weak_ptr<MLN::Net::Connection> getUser(const size_t identity);

private:
	mutable boost::shared_mutex m_mtx_users;

	std::map< size_t	// connection identity
		, boost::weak_ptr<MLN::Net::Connection> > m_users;

	std::map< uint32_t	// userIDX
		, size_t 	// connection identity 
	> m_nodeToUserMap;
};
