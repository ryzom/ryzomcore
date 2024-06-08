#include <gtest/gtest.h>

#include <nelns/naming_service/service_instance_manager.h>

TEST(CServiceInstanceManager, ShouldWork) {
	EXPECT_STRNE("hello", "world");
	EXPECT_EQ(7 * 6, 42);
	CServiceInstanceManager instance;
}
