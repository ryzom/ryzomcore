#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nelns/naming_service/naming_service.h>

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;

TEST(CNamingService, shouldStartService)
{
	CNamingService instance;
	uint16 port = 0;

	auto exitcode = instance.main("short name", "long name", port, "config-dir", "log-dir", "compilation date");

	EXPECT_THAT(exitcode, Eq(0));
}
