#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nelns/naming_service/service_instance_manager.h>

using ::testing::Eq;
using ::testing::NotNull;

TEST(CServiceInstanceManager, getInstanceShouldAssertIfNotInitialized) {
	ASSERT_DEATH({
	    CServiceInstanceManager::getInstance();
	}, "");
}

TEST(CServiceInstanceManager, getInstanceShouldReturnSingleton) {
	EXPECT_NO_THROW({
		CServiceInstanceManager instance;

		EXPECT_THAT(CServiceInstanceManager::getInstance(), NotNull());
		EXPECT_THAT(CServiceInstanceManager::getInstance(), Eq(&instance));
	});
}

TEST(CServiceInstanceManager, DestructorShouldCleanupSingletonInstance) {
	{
		CServiceInstanceManager instance;
	}
	ASSERT_DEATH({
		CServiceInstanceManager::getInstance();
	}, "");
}
