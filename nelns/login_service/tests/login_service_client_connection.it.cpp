#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <future>

#include <nel/misc/config_file.h>
#include <nel/misc/stream.h>
#include <nel/net/message.h>
#include <nel/net/callback_net_base.h>

#include <nelns/login_service/login_service.h>

#include "mock_persistence.h"

using testing::AllOf;
using testing::ElementsAre;
using testing::Eq;
using testing::Expectation;
using testing::Field;
using testing::FieldsAre;
using testing::IsEmpty;
using testing::IsFalse;
using testing::IsNull;
using testing::IsTrue;
using testing::NiceMock;
using testing::Not;
using testing::NotNull;
using testing::Optional;
using testing::Property;
using testing::Return;
using testing::SizeIs;
using testing::StrEq;

using CVar = NLMISC::CConfigFile::CVar;
using NLMISC::IStream;
using NLMISC::nlSleep;
using NLNET::CCallbackClient;
using NLNET::CCallbackNetBase;
using NLNET::CInetHost;
using NLNET::CMessage;
using NLNET::TCallbackItem;
using NLNET::TSockId;

struct VLPRequest
{
	ucstring login;
	std::string cpassword;
	std::string application;
	void serial(IStream &stream)
	{
		stream.serial(login);
		stream.serial(cpassword);
		stream.serial(application);
	}
};

struct VLPResponse
{
	std::string reason;
	std::vector<OnlineShardProjection> shards;
	void serial(IStream &stream)
	{
		stream.serial(reason);
		if (reason.empty())
		{
			sint32 readShardCount;
			stream.serial(readShardCount);
			for (auto i = 0; i < readShardCount; ++i)
			{
				OnlineShardProjection shard;
				stream.serial(shard.name, shard.nbplayers, shard.sid);
				shards.push_back(shard);
			}
		}
	}
};

struct CSRequest
{
	sint32 shardid;
	void serial(IStream &stream)
	{
		stream.serial(shardid);
	}
};

struct SCSResponse
{
	std::string reason;
	void serial(IStream &stream)
	{
		stream.serial(reason);
		if (reason.empty())
		{
		}
	}
};

void insertConfigVariable(NLMISC::CConfigFile &configFile, const std::string &name, const std::string &value)
{
	CVar var;
	var.Type = NLMISC::CConfigFile::CVar::T_STRING;
	var.setAsString(value);
	configFile.insertVar(name, var);
}

void insertConfigVariable(NLMISC::CConfigFile &configFile, const std::string &name, const bool &value)
{
	const std::string stringValue = value ? "true" : "false";
	insertConfigVariable(configFile, name, stringValue);
}

void insertConfigVariable(NLMISC::CConfigFile &configFile, const std::string &name, const int &value)
{
	CVar var;
	var.Type = NLMISC::CConfigFile::CVar::T_INT;
	var.setAsInt(value);
	configFile.insertVar(name, var);
}

class CLoginService_ClientConnectionIT : public testing::Test
{
protected:
	std::shared_ptr<NiceMock<MockPersistence>> persistence = std::make_shared<NiceMock<MockPersistence>>();
	CLoginService loginService = CLoginService(persistence);
	CCallbackClient client;
	CInetHost host = CInetHost("localhost");
	int port = 51000;
	std::chrono::seconds defaultTimeout = std::chrono::seconds(10);

	VLPRequest verifyLogin {
		.login = ucstring::makeFromUtf8("test-login"),
		.cpassword = "test password",
		.application = "test-application"
	};
	LoginUserProjection user {
		.uid = 123,
		.password = verifyLogin.cpassword,
		.state = "Offline"
	};
	OnlineShardProjection shard {
		.sid = 456,
		.name = ucstring::makeFromUtf8("test shard"),
		.nbplayers = 111
	};
	CSRequest chooseShard {
		.shardid = 456
	};
	AuthorizedUserProjection authorizedUser {
		.uid = "test user id",
		.cookie = NLNET::CLoginCookie(),
		.privilege = "test privilege",
		.extendedPrivilege = "test extended privilege",
	};

	void SetUp() override
	{
		insertConfigVariable(loginService.ConfigFile, "ClientsPort", port);
		insertConfigVariable(loginService.ConfigFile, "UseDirectClient", true);
		insertConfigVariable(loginService.ConfigFile, "AcceptUnknownUsers", 0);

		loginService.init();

		host.setPort(port);
		client.connect(host);
		ASSERT_THAT(client.connected(), IsTrue());
	}

	void TearDown() override
	{
		loginService.release();
	}

	template <typename ResponseType, typename RequestType>
	ResponseType sendMessage(const std::string &key, RequestType &messageRequest)
	{
		auto response = std::async(std::launch::async, [=, &messageRequest]() {
			CMessage msgout(key);
			msgout.serial(messageRequest);

			bool pending = true;
			ResponseType messageResponse;
			TCallbackItem callbackArray[] = {
				{ key.c_str(), [&messageResponse, &pending](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
				     msgin.serial(messageResponse);
				     pending = false;
				 } }
			};
			client.addCallbackArray(callbackArray, std::size(callbackArray));
			client.send(msgout);

			while (pending)
			{
				client.update();
				loginService.update();
				nlSleep(1);
			}

			return messageResponse;
		});
		auto state = response.wait_for(defaultTimeout);

		if (state != std::future_status::ready)
		{
			throw std::runtime_error("timeout waiting for response");
		}

		return response.get();
	}

	template <typename RequestType>
	void sendMessage(const std::string &key, RequestType &messageRequest)
	{
		CMessage msgout(key);
		msgout.serial(messageRequest);

		client.send(msgout);
	}

	template <typename ResponseType>
	ResponseType receiveMessage(const std::string &key)
	{
		auto response = std::async(std::launch::async, [=]() {
			bool pending = true;
			ResponseType messageResponse;
			TCallbackItem callbackArray[] = {
				{ key.c_str(), [&messageResponse, &pending](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
				     msgin.serial(messageResponse);
				     pending = false;
				 } }
			};
			client.addCallbackArray(callbackArray, std::size(callbackArray));

			while (pending)
			{
				client.update();
				loginService.update();
				nlSleep(1);
			}

			return messageResponse;
		});
		auto state = response.wait_for(defaultTimeout);

		if (state != std::future_status::ready)
		{
			throw std::runtime_error("timeout waiting for response");
		}

		return response.get();
	}
};

TEST_F(CLoginService_ClientConnectionIT, shouldReturnAvaialbleShardsOnSuccessfulLogin)
{
	EXPECT_CALL(*persistence, findUserByLogin)
	    .WillRepeatedly(Return(std::make_pair(std::make_optional(user), "")));
	EXPECT_CALL(*persistence, findOnlineShardsByApplication)
	    .WillRepeatedly(Return(std::make_pair(std::vector { shard }, "")));

	auto response = sendMessage<VLPResponse>("VLP", verifyLogin);

	EXPECT_THAT(response,
	    AllOf(
	        Field("reason", &VLPResponse::reason, IsEmpty()),
	        Field("shards", &VLPResponse::shards, ElementsAre(AllOf(Field(&OnlineShardProjection::sid, Eq(shard.sid)), Field(&OnlineShardProjection::name, Eq(shard.name)), Field(&OnlineShardProjection::nbplayers, Eq(shard.nbplayers)))))));
}

TEST_F(CLoginService_ClientConnectionIT, shouldReturnErrorWhenUserDoesNotExist)
{
	EXPECT_CALL(*persistence, findUserByLogin)
	    .WillRepeatedly(Return(std::make_pair(std::nullopt, "")));

	auto response = sendMessage<VLPResponse>("VLP", verifyLogin);

	EXPECT_THAT(response, Field("reason", &VLPResponse::reason, StrEq("Login 'test-login' doesn't exist")));
}

TEST_F(CLoginService_ClientConnectionIT, shouldAcceptUnknownUsersIfEnabled)
{
	loginService.ConfigFile.getVar("AcceptUnknownUsers").setAsInt(1);
	EXPECT_CALL(*persistence, findUserByLogin)
	    .Times(1)
	    .WillOnce(Return(std::make_pair(std::nullopt, "")));
	EXPECT_CALL(*persistence, createUser)
	    .WillOnce(Return(std::make_pair(std::make_optional(user), "")))
	    .WillRepeatedly(Return(std::make_pair(std::nullopt, "mock: user already created")));

	auto response = sendMessage<VLPResponse>("VLP", verifyLogin);

	EXPECT_THAT(response, Field("reason", &VLPResponse::reason, IsEmpty()));
}

TEST_F(CLoginService_ClientConnectionIT, shouldReturnErrorWhenNoAuthorizedUserExistsToSelectAShard)
{
	EXPECT_CALL(*persistence, findUserByLogin)
	    .WillRepeatedly(Return(std::make_pair(std::make_optional(user), "")));
	EXPECT_CALL(*persistence, findOnlineShardsByApplication)
	    .WillRepeatedly(Return(std::make_pair(std::vector { shard }, "")));
	Expectation userAuthorized = EXPECT_CALL(*persistence, authorizeUser);
	EXPECT_CALL(*persistence, findAuthorizedUsers)
	    .After(userAuthorized);
	ASSERT_THAT(sendMessage<VLPResponse>("VLP", verifyLogin), Field("reason", &VLPResponse::reason, IsEmpty()));

	sendMessage("CS", chooseShard);
	auto response = receiveMessage<SCSResponse>("SCS");

	EXPECT_THAT(response, Field("reason", &SCSResponse::reason, StrEq("You are not authorized to select a shard")));
}

TEST_F(CLoginService_ClientConnectionIT, shouldReturnErrorWhenUserHasWrongAddressToSelectAShard)
{
	authorizedUser.cookie.setFromString("00112233|44556677|8899aabb");
	EXPECT_CALL(*persistence, findUserByLogin)
	    .WillRepeatedly(Return(std::make_pair(std::make_optional(user), "")));
	EXPECT_CALL(*persistence, findOnlineShardsByApplication)
	    .WillRepeatedly(Return(std::make_pair(std::vector { shard }, "")));
	Expectation userAuthorized = EXPECT_CALL(*persistence, authorizeUser);
	EXPECT_CALL(*persistence, findAuthorizedUsers)
	    .After(userAuthorized)
	    .WillRepeatedly(Return(std::make_pair(std::vector { authorizedUser }, "")));
	ASSERT_THAT(sendMessage<VLPResponse>("VLP", verifyLogin), Field("reason", &VLPResponse::reason, IsEmpty()));

	sendMessage("CS", chooseShard);
	auto response = receiveMessage<SCSResponse>("SCS");

	EXPECT_THAT(response, Field("reason", &SCSResponse::reason, StrEq("You are not authorized to select a shard")));
}
