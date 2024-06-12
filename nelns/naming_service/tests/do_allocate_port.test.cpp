#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nelns/naming_service/functions.h>
#include <nelns/naming_service/variables.h>

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;

using ::NLNET::CInetAddress;

TEST(doAllocatePort, shouldStartAtMinBaseportAndWrapArroundMaxBasePort)
{
	MinBasePort = 10;
	MaxBasePort = 15;
	CInetAddress addresses( "localhost:12345" );

	EXPECT_THAT(doAllocatePort(addresses), Eq(10));
	EXPECT_THAT(doAllocatePort(addresses), Eq(11));
	EXPECT_THAT(doAllocatePort(addresses), Eq(12));
	EXPECT_THAT(doAllocatePort(addresses), Eq(13));
	EXPECT_THAT(doAllocatePort(addresses), Eq(14));
	EXPECT_THAT(doAllocatePort(addresses), Eq(10));
}
