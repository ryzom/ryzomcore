#ifndef TESTS_MOCK_PERSISTENCE_H
#define TESTS_MOCK_PERSISTENCE_H

#include <gmock/gmock.h>

#include <nelns/login_service/persistence.h>

class MockPersistence : public IPersistence
{
public:
	MOCK_METHOD(void, init, (), (override));

	// user
	MOCK_METHOD((std::pair<std::optional<LoginUserProjection>, std::string>), findUserByLogin, (const std::string &login), (override));

	MOCK_METHOD(std::string, authorizeUser, (sint32 uid, const NLNET::CLoginCookie &cookie), (override));

	MOCK_METHOD((std::pair<std::optional<LoginUserProjection>, std::string>), createUser, (const std::string &login, const std::string &cpassword), (override));

	MOCK_METHOD((std::pair<std::vector<AuthorizedUserProjection>, std::string>), findAuthorizedUsers, (), (override));

	MOCK_METHOD(std::string, updateUserWaitingOnShard, (const std::string& uid, const sint32& shardid), (override));

	MOCK_METHOD(std::string, logoutUserById, (const std::string& uid), (override));

	MOCK_METHOD(std::string, logoutUserByCookie, (const NLNET::CLoginCookie &cookie), (override));

	MOCK_METHOD((std::pair<std::optional<std::string>, std::string>), findUserLoginById, (const std::string& uid), (override));

	MOCK_METHOD((std::pair<std::vector<NotOfflineUserProjection>, std::string>), findNotOfflineUsers, (), (override));


	// shard
	MOCK_METHOD((std::pair<std::vector<OnlineShardProjection>, std::string>), findOnlineShardsByApplication, (const std::string &application), (override));

	MOCK_METHOD((std::pair<bool, std::string>), existsShardById, (const sint32& shardid), (override));
};

#endif // TESTS_MOCK_PERSISTENCE_H