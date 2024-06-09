#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nel/misc/log.h>
#include <nel/misc/mem_displayer.h>
#include <nel/net/buf_sock.h>
#include <nelns/naming_service/service_entry.h>
#include <nelns/naming_service/service_instance_manager.h>
#include <nelns/naming_service/variables.h>

using ::std::string;
using ::std::vector;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsSupersetOf;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::StartsWith;
using ::testing::StrEq;

using ::NLMISC::CLightMemDisplayer;
using ::NLMISC::CLog;
using ::NLNET::CInetAddress;
using ::NLNET::InvalidSockId;
using ::NLNET::TServiceId;

TEST(CServiceInstanceManager, getInstanceShouldAssertIfNotInitialized)
{
	ASSERT_DEATH({ CServiceInstanceManager::getInstance(); }, "");
}

TEST(CServiceInstanceManager, getInstanceShouldReturnSingleton)
{
	EXPECT_NO_THROW({
		CServiceInstanceManager instance;

		EXPECT_THAT(CServiceInstanceManager::getInstance(), NotNull());
		EXPECT_THAT(CServiceInstanceManager::getInstance(), Eq(&instance));
	});
}

TEST(CServiceInstanceManager, DestructorShouldCleanupSingletonInstance)
{
	{
		CServiceInstanceManager instance;
	}
	ASSERT_DEATH({ CServiceInstanceManager::getInstance(); }, "");
}

TEST(CServiceInstanceManager, displayInfoShouldBeEmptyIfNothingRegistered)
{
	CServiceInstanceManager instance;
	CLightMemDisplayer displayer;
	CLog log = { NLMISC::CLog::LOG_ASSERT };
	log.addDisplayer(&displayer);

	instance.displayInfo(&log);

	EXPECT_THAT(
	    displayer.lockStrings(),
	    ElementsAre(
	        StrEq("Restricted services:\n"),
	        StrEq("Online registered services:\n")));
}

TEST(CServiceInstanceManager, addUniqueServiceShouldAddShardUniqueService)
{
	CServiceInstanceManager instance;
	CLightMemDisplayer displayer;
	CLog log = { NLMISC::CLog::LOG_ASSERT };
	log.addDisplayer(&displayer);

	instance.addUniqueService("unique-shard-service-name", true);

	instance.displayInfo(&log);
	EXPECT_THAT(
	    displayer.lockStrings(),
	    IsSupersetOf({ StartsWith("Restricted services:"),
	        StartsWith("unique-shard-service-name -> only one per shard") }));
}

TEST(CServiceInstanceManager, addUniqueServiceShouldAddMachineUniqueService)
{
	CServiceInstanceManager instance;
	CLightMemDisplayer displayer;
	CLog log = { NLMISC::CLog::LOG_ASSERT };
	log.addDisplayer(&displayer);

	instance.addUniqueService("unique-machine-service-name", false);

	instance.displayInfo(&log);
	EXPECT_THAT(
	    displayer.lockStrings(),
	    ElementsAre(
	        StartsWith("Restricted services:"),
	        StartsWith("unique-machine-service-name -> only one per machine"),
	        StartsWith("Online registered services:")));
}

TEST(CServiceInstanceManager, queryStartServiceShouldAddOnlineSercie)
{
	CServiceInstanceManager instance;
	TServiceId serviceId(123);
	vector<CInetAddress> addresses = { "localhost:12345" };
	string reason;
	CLightMemDisplayer displayer;
	CLog log(NLMISC::CLog::LOG_ASSERT);
	log.addDisplayer(&displayer);

	EXPECT_THAT(
	    instance.queryStartService("online-service", serviceId, addresses, reason),
	    IsTrue());

	instance.displayInfo(&log);
	EXPECT_THAT(
	    displayer.lockStrings(),
	    IsSupersetOf({ StartsWith("Online registered services:"),
	        StartsWith(serviceId.toString()) }));
}

TEST(CServiceInstanceManager, queryStartServiceShouldAddSingleShardUnqiueService)
{
	CServiceInstanceManager instance;
	TServiceId serviceId(123);
	vector<CInetAddress> addresses = { "localhost:12345" };
	string reason;
	CLightMemDisplayer displayer;
	CLog log(NLMISC::CLog::LOG_ASSERT);
	log.addDisplayer(&displayer);
	string serviceName = "unique-shard-service-name";
	instance.addUniqueService(serviceName, true);

	EXPECT_THAT(
	    instance.queryStartService(serviceName, serviceId, addresses, reason),
	    IsTrue());

	instance.displayInfo(&log);
	EXPECT_THAT(
	    displayer.lockStrings(),
	    IsSupersetOf({ StartsWith("Online registered services:"),
	        StartsWith(serviceId.toString()) }));
}

TEST(CServiceInstanceManager, queryStartServiceShouldNotAddAdditionalShardUnqiueService)
{
	RegisteredServices.clear();
	CServiceInstanceManager instance;
	TServiceId serviceId(123);
	vector<CInetAddress> addresses = { "localhost:12345" };
	string reason;
	CLightMemDisplayer displayer;
	CLog log(NLMISC::CLog::LOG_ASSERT);
	log.addDisplayer(&displayer);
	string serviceName = "unique-shard-service-name";
	RegisteredServices.push_back(CServiceEntry(InvalidSockId, addresses, serviceName, serviceId));
	instance.addUniqueService(serviceName, true);
	instance.queryStartService(serviceName, serviceId, addresses, reason);

	EXPECT_THAT(
	    instance.queryStartService(serviceName, serviceId, addresses, reason),
	    IsFalse());

	EXPECT_THAT(
	    reason,
	    StrEq("Service unique-shard-service-name already found as 123, must be unique on shard"));
}

TEST(CServiceInstanceManager, queryStartServiceShouldAddAdditionalMachineUnqiueServiceOnDifferentMachine)
{
	RegisteredServices.clear();
	CServiceInstanceManager instance;
	TServiceId serviceId(123);
	vector<CInetAddress> firstAddresses = { "127.0.0.1:12345" };
	vector<CInetAddress> secondAddresses = { "127.0.0.2:12345" };
	string reason;
	CLightMemDisplayer displayer;
	CLog log(NLMISC::CLog::LOG_ASSERT);
	log.addDisplayer(&displayer);
	string serviceName = "unique-machine-service-name";
	RegisteredServices.push_back(CServiceEntry(InvalidSockId, firstAddresses, serviceName, serviceId));
	instance.addUniqueService(serviceName, false);
	instance.queryStartService(serviceName, serviceId, firstAddresses, reason);

	EXPECT_THAT(
	    instance.queryStartService(serviceName, serviceId, secondAddresses, reason),
	    IsTrue());
}

TEST(CServiceInstanceManager, queryStartServiceShouldNotAddAdditionalMachineUnqiueService)
{
	RegisteredServices.clear();
	CServiceInstanceManager instance;
	TServiceId serviceId(123);
	vector<CInetAddress> addresses = { "localhost:12345" };
	string reason;
	CLightMemDisplayer displayer;
	CLog log(NLMISC::CLog::LOG_ASSERT);
	log.addDisplayer(&displayer);
	string serviceName = "unique-machine-service-name";
	RegisteredServices.push_back(CServiceEntry(InvalidSockId, addresses, serviceName, serviceId));
	instance.addUniqueService(serviceName, false);
	instance.queryStartService(serviceName, serviceId, addresses, reason);

	EXPECT_THAT(
	    instance.queryStartService(serviceName, serviceId, addresses, reason),
	    IsFalse());

	EXPECT_THAT(
	    reason,
	    StrEq("Service unique-machine-service-name already found as 123 on same machine"));
}

TEST(CServiceInstanceManager, releaseServiceShouldRemoveOnlineSercie)
{
	CServiceInstanceManager instance;
	TServiceId serviceId(123);
	vector<CInetAddress> addresses = { "localhost:12345" };
	string reason;
	CLightMemDisplayer displayer;
	CLog log(NLMISC::CLog::LOG_ASSERT);
	log.addDisplayer(&displayer);
	instance.queryStartService("online-service", serviceId, addresses, reason);

	instance.releaseService(serviceId);

	instance.displayInfo(&log);
	EXPECT_THAT(
	    displayer.lockStrings(),
	    ElementsAre(
	        StartsWith("Restricted services:"),
	        StartsWith("Online registered services:")));
}
