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

using testing::Eq;
using testing::IsTrue;

using CVar = NLMISC::CConfigFile::CVar;
using NLMISC::IStream;
using NLMISC::nlSleep;
using NLNET::CMessage;
using NLNET::CCallbackClient;
using NLNET::CCallbackNetBase;
using NLNET::CInetHost;
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
	sint32 shardCount = 0;
	void serial(IStream &stream)
	{
		stream.serial(reason);
		stream.serial(shardCount);
	}

};

void insertConfigVariable(NLMISC::CConfigFile& configFile, const std::string& name, const std::string& value)
{
	CVar var;
	var.Type = NLMISC::CConfigFile::CVar::T_STRING;
	var.setAsString(value);
	configFile.insertVar(name, var);
}

void insertConfigVariable(NLMISC::CConfigFile& configFile, const std::string& name, const bool& value)
{
	const std::string stringValue = value ? "true" : "false";
	insertConfigVariable(configFile, name, stringValue);
}

void insertConfigVariable(NLMISC::CConfigFile& configFile, const std::string& name, const int& value)
{
	CVar var;
	var.Type = NLMISC::CConfigFile::CVar::T_INT;
	var.setAsInt(value);
	configFile.insertVar(name, var);
}

class MockPersistence : public IPersistence
{
public:
	void init() override {}

	std::pair<std::optional<LoginUserProjection>, std::string> findUserByLogin(const std::string &login) override
	{
		return std::make_pair(std::nullopt, "mock error");
	}
};

class CLoginServiceIT : public testing::Test
{
protected:
	std::shared_ptr<MockPersistence> persistence = std::make_shared<MockPersistence>();
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
	CMessage msgout("VLP");
	msgout.serial(request);

	std::promise<void> response_promise;
	std::future<void> response = response_promise.get_future();
	TCallbackItem callbackArray[] = {
		{ "VLP", [&response_promise](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
			response_promise.set_value();
		} }
	};
	client.addCallbackArray(callbackArray, std::size(callbackArray));

	client.send(msgout);
	auto updateClient = std::async(std::launch::async, [=, &response]() {
		while (response.valid() && running)
		{
			client.update();
			nlSleep(1);
		}
	});
	auto updateNamingService = std::async(std::launch::async, [=, &response]() {
		while (response.valid() && running)
		{
			loginService.update();
			nlSleep(1);
		}
	});

	auto state = response.wait_for(defaultTimeout);
	running = false;
	ASSERT_THAT(state, Eq(std::future_status::ready));
}
