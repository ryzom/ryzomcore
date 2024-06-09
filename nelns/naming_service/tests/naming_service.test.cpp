#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nelns/naming_service/naming_service.h>

using ::testing::NotNull;


TEST(CNamingService, ShouldInstantiateSingleton) {
	CNamingService instance;

	EXPECT_THAT(CNamingService::getInstance(), NotNull());
}
