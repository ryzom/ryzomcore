#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

#include <nel/misc/config_file.h>
#include <nel/misc/stream.h>
#include <nel/net/message.h>

#include <nelns/login_service/login_service.h>

using testing::IsTrue;

using CVar = NLMISC::CConfigFile::CVar;
using NLMISC::IStream;
using NLNET::CMessage;
using NLNET::CCallbackClient;
using NLNET::CInetHost;

struct VLPRequest
{
	void serial(IStream &stream)
	{
	}

};

struct VLPResponse
{
	void serial(IStream &stream)
	{
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
		loginService.release();
	}
};

TEST_F(CLoginServiceIT, shouldAnswerToVerifyLoginPassword)
{
	VLPRequest request {
	};
	CMessage msgout("VLP");
	msgout.serial(request);
}
