#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <nel/misc/config_file.h>
#include <nel/net/inet_host.h>
#include <nel/net/callback_client.h>

#include <nelns/naming_service/naming_service.h>

using ::std::string;
using ::std::vector;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;

using CVar = ::NLMISC::CConfigFile::CVar;
using ::NLNET::CCallbackClient;
using ::NLNET::CInetHost;

TEST(CNamingService, shouldAcceptConnections)
{
	CCallbackClient client;
	CNamingService instance;
	CVar basePort;
	basePort.Type = NLMISC::CConfigFile::CVar::T_INT;
	basePort.setAsInt(51000);
	CVar uniqueOnShardServices;
	uniqueOnShardServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
	uniqueOnShardServices.setAsString((vector<string>) {});
	CVar uniqueByMachineServices;
	uniqueByMachineServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
	uniqueByMachineServices.setAsString((vector<string>) {});
	CVar nsPort;
	nsPort.Type = NLMISC::CConfigFile::CVar::T_INT;
	nsPort.setAsInt(50000);
	CInetHost host("localhost:50000");

	instance.ConfigFile.insertVar("BasePort", basePort);
	instance.ConfigFile.insertVar("NSPort", nsPort);
	instance.ConfigFile.insertVar("UniqueOnShardServices", uniqueOnShardServices);
	instance.ConfigFile.insertVar("UniqueByMachineServices", uniqueByMachineServices);

	instance.init();

	client.connect(host);
	ASSERT_THAT(client.connected(), IsTrue());
}
