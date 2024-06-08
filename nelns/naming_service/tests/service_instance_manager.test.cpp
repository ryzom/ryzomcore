#include <gtest/gtest.h>

#include <nelns/naming_service/service_instance_manager.h>

TEST(CServiceInstanceManager, getInstanceShouldThrowIfNotInitialized) {
	ASSERT_DEATH({
	    CServiceInstanceManager::getInstance();
	}, "");
}

TEST(CServiceInstanceManager, ConstructorShouldNotThrow) {
	EXPECT_NO_THROW({
		CServiceInstanceManager instance;
	});
}
