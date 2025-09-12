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
using testing::Not;
using testing::NotNull;
using testing::NiceMock;
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

class MockPersistence : public IPersistence
{
public:
	MOCK_METHOD(void, init, (), (override));

	MOCK_METHOD((std::pair<std::optional<LoginUserProjection>, std::string>), findUserByLogin, (const std::string &login), (override));

	MOCK_METHOD(std::string, authorizeUser, (sint32 uid, const NLNET::CLoginCookie &cookie), (override));

	MOCK_METHOD((std::pair<std::vector<OnlineShardProjection>, std::string>), findOnlineShardsByApplication, (const std::string &application), (override));

	MOCK_METHOD(std::string, createUser, (const std::string& login, const std::string& cpassword), (override));
};

class CLoginServiceIT : public testing::Test
{
protected:
	std::shared_ptr<NiceMock<MockPersistence>> persistence = std::make_shared<NiceMock<MockPersistence>>();
	CLoginService loginService = CLoginService(persistence);
	CCallbackClient client;
	CInetHost host = CInetHost("localhost");
	int port = 51000;
	std::chrono::seconds defaultTimeout = std::chrono::seconds(10);
	bool running = true;

	void SetUp() override
	{
		insertConfigVariable(loginService.ConfigFile, "ClientsPort", port);
		insertConfigVariable(loginService.ConfigFile, "UseDirectClient", true);
		insertConfigVariable(loginService.ConfigFile, "AcceptUnknownUsers", 0);

		loginService.init(); // requires database to start

		host.setPort(port);
		client.connect(host);
		ASSERT_THAT(client.connected(), IsTrue());
	}

	void TearDown() override
	{
		running = false;
		loginService.release();
	}
};

TEST_F(CLoginServiceIT, shouldAnswerToVerifyLoginPassword)
{
	VLPRequest request {
		.login = ucstring::makeFromUtf8("test-login"),
		.cpassword = "test-password",
		.application = "test-application"
	};
	LoginUserProjection user {
		.uid = 123,
		.password = request.cpassword,
		.state = "Offline"
	};
	OnlineShardProjection shard {
		.sid = 456,
		.name = ucstring::makeFromUtf8("test shard"),
		.nbplayers = 111
	};
	EXPECT_CALL(*persistence, findUserByLogin)
		.WillRepeatedly(Return(std::make_pair(std::make_optional(user), "")));
	EXPECT_CALL(*persistence, findOnlineShardsByApplication)
		.WillRepeatedly(Return(std::make_pair(std::vector{shard}, "")));
	CMessage msgout("VLP");
	msgout.serial(request);

	std::promise<VLPResponse> response_promise;
	std::future<VLPResponse> response = response_promise.get_future();
	TCallbackItem callbackArray[] = {
		{ "VLP", [&response_promise](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
		     VLPResponse response;
		     msgin.serial(response);
		     response_promise.set_value(response);
		 } }
	};
	client.addCallbackArray(callbackArray, std::size(callbackArray));

	client.send(msgout);
	auto update = std::async(std::launch::async, [=, &response]() {
		while (response.valid() && running)
		{
			client.update();
			loginService.update();
			nlSleep(1);
		}
	});

	auto state = response.wait_for(defaultTimeout);
	running = false;
	ASSERT_THAT(state, Eq(std::future_status::ready));
	EXPECT_THAT(response.get(),
	    AllOf(
	        Field("reason", &VLPResponse::reason, IsEmpty()),
	        Field("shards", &VLPResponse::shards, ElementsAre(
	        	AllOf(
	        		Field(&OnlineShardProjection::sid, Eq(shard.sid)),
					Field(&OnlineShardProjection::name, Eq(shard.name)),
					Field(&OnlineShardProjection::nbplayers, Eq(shard.nbplayers))
	        	)
	        ))
	    )
	);
}

TEST_F(CLoginServiceIT, shouldReturrnErrorWhenUserDoesNotExist)
{
	VLPRequest request {
		.login = ucstring::makeFromUtf8("test-login"),
		.cpassword = "test password",
		.application = "test-application"
	};
	EXPECT_CALL(*persistence, findUserByLogin)
		.WillRepeatedly(Return(std::make_pair(std::nullopt, "")));
	CMessage msgout("VLP");
	msgout.serial(request);

	std::promise<VLPResponse> response_promise;
	std::future<VLPResponse> response = response_promise.get_future();
	TCallbackItem callbackArray[] = {
		{ "VLP", [&response_promise](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
		     VLPResponse response;
		     msgin.serial(response);
		     response_promise.set_value(response);
		 } }
	};
	client.addCallbackArray(callbackArray, std::size(callbackArray));

	client.send(msgout);
	auto update = std::async(std::launch::async, [=, &response]() {
		while (response.valid() && running)
		{
			client.update();
			loginService.update();
			nlSleep(1);
		}
	});

	auto state = response.wait_for(defaultTimeout);
	running = false;
	ASSERT_THAT(state, Eq(std::future_status::ready));
	EXPECT_THAT(response.get(), Field("reason", &VLPResponse::reason, StrEq("Login 'test-login' doesn't exist")));
}

TEST_F(CLoginServiceIT, shouldAcceptUnknownUsersIfEnabled)
{
	loginService.ConfigFile.getVar("AcceptUnknownUsers").setAsInt(1);
	VLPRequest request {
		.login = ucstring::makeFromUtf8("test-login"),
		.cpassword = "test password",
		.application = "test-application"
	};
	LoginUserProjection user {
		.uid = 123,
		.password = request.cpassword,
		.state = "Offline"
	};
	EXPECT_CALL(*persistence, findUserByLogin)
		.WillOnce(Return(std::make_pair(std::nullopt, "")));
	Expectation userCreated = EXPECT_CALL(*persistence, createUser)
		.WillOnce(Return(""))
		.WillRepeatedly(Return("mock: user already created"));
	EXPECT_CALL(*persistence, findUserByLogin)
		.After(userCreated)
		.WillRepeatedly(Return(std::make_pair(std::make_optional(user), "")));
	CMessage msgout("VLP");
	msgout.serial(request);

	std::promise<VLPResponse> response_promise;
	std::future<VLPResponse> response = response_promise.get_future();
	TCallbackItem callbackArray[] = {
		{ "VLP", [&response_promise](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
		     VLPResponse response;
		     msgin.serial(response);
		     response_promise.set_value(response);
		 } }
	};
	client.addCallbackArray(callbackArray, std::size(callbackArray));

	client.send(msgout);
	auto update = std::async(std::launch::async, [=, &response]() {
		while (response.valid() && running)
		{
			client.update();
			loginService.update();
			nlSleep(1);
		}
	});

	auto state = response.wait_for(defaultTimeout);
	running = false;
	ASSERT_THAT(state, Eq(std::future_status::ready));
	EXPECT_THAT(response.get(), Field("reason", &VLPResponse::reason, IsEmpty()));
}
