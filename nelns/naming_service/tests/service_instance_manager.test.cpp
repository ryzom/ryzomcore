#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nel/misc/log.h>
#include <nel/misc/mem_displayer.h>
#include <nelns/naming_service/service_instance_manager.h>

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::NotNull;
using ::testing::StartsWith;
using ::testing::StrEq;

using ::NLMISC::CLightMemDisplayer;
using ::NLMISC::CLog;

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
	    ElementsAre(
	        StartsWith("Restricted services:"),
	        StartsWith("unique-shard-service-name -> only one per shard"),
	        StartsWith("Online registered services:")));
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
