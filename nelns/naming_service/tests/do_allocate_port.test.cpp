#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <nel/net/buf_sock.h>
#include <nel/net/inet_address.h>
#include <nel/net/unified_network.h>

#include <nelns/naming_service/functions.h>
#include <nelns/naming_service/variables.h>

using ::std::string;
using ::std::vector;

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;

using ::NLNET::CInetAddress;
using ::NLNET::InvalidSockId;
using ::NLNET::TServiceId;

class doAllocatePortTest : public testing::Test
{
protected:
	void SetUp() override
	{
		MinBasePort = 10;
		MaxBasePort = 15;
		RegisteredServices.clear();
	}

	void TearDown() override
	{
		RegisteredServices.clear();
	}
};

TEST_F(doAllocatePortTest, shouldStartAtMinBaseport)
{
	CInetAddress address("localhost:12345");

	EXPECT_THAT(doAllocatePort(address), Eq(10));
}

TEST_F(doAllocatePortTest, shouldIncreasePorts)
{
	CInetAddress address("localhost:12345");

	EXPECT_THAT(doAllocatePort(address), Eq(11));
	EXPECT_THAT(doAllocatePort(address), Eq(12));
	EXPECT_THAT(doAllocatePort(address), Eq(13));
}

TEST_F(doAllocatePortTest, shouldWrapArroundMaxBasePort)
{
	CInetAddress address("localhost:12345");

	EXPECT_THAT(doAllocatePort(address), Eq(14));
	EXPECT_THAT(doAllocatePort(address), Eq(10));
}

TEST_F(doAllocatePortTest, shouldSkipPortsUsedByRegisteredServices)
{
	CInetAddress address("10.0.0.1:11");
	TServiceId serviceId(123);
	vector<CInetAddress> addresses { address };
	string serviceName = "test-service-name";
	RegisteredServices.emplace_back(InvalidSockId, addresses, serviceName, serviceId);

	EXPECT_THAT(doAllocatePort(address), Eq(12));
}

TEST_F(doAllocatePortTest, shouldNotSkipPortIfNotUsedByRegisteredServices)
{
	CInetAddress address("10.0.0.1:10");
	TServiceId serviceId(123);
	vector<CInetAddress> addresses { address };
	string serviceName = "test-service-name";
	RegisteredServices.emplace_back(InvalidSockId, addresses, serviceName, serviceId);

	EXPECT_THAT(doAllocatePort(address), Eq(13));
}
