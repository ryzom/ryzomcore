#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nelns/naming_service/naming_service.h>

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;

TEST(CNamingService, shouldInstantiateSingleton)
{
	CNamingService instance;

	EXPECT_THAT(CNamingService::isServiceInitialized(), IsTrue());
	EXPECT_THAT(CNamingService::getInstance(), NotNull());
	EXPECT_THAT(CNamingService::getInstance(), Eq(&instance));
}

TEST(CNamingService, shouldNotAllowMultipleSimultaneousInstances)
{
	CNamingService first;

	ASSERT_DEATH({ CNamingService second; }, "");
}

TEST(CNamingService, shouldCleanupSingleton)
{
	{
		CNamingService instance;
	};

	EXPECT_THAT(CNamingService::isServiceInitialized(), IsFalse());
	EXPECT_THAT(CNamingService::getInstance(), IsNull());
}

TEST(CNamingService, shouldAllowNewInstanceAfterDestruction)
{
	{
		CNamingService first;
	};

	{
		CNamingService second;

		EXPECT_THAT(CNamingService::isServiceInitialized(), IsTrue());
		EXPECT_THAT(CNamingService::getInstance(), NotNull());
		EXPECT_THAT(CNamingService::getInstance(), Eq(&second));
	};
}
