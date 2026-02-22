#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ostream>
#include <string>
#include <set>
#include <vector>

#include <nel/web/http_client_curl.h>

using std::set;
using std::string;
using std::vector;

using ::testing::Contains;
using ::testing::EndsWith;
using ::testing::Eq;
using ::testing::Field;
using ::testing::NotNull;
using ::testing::Property;
using ::testing::StrEq;
using ::testing::HasSubstr;

using namespace NLWEB;

class CWebHttpClientIT : public testing::Test
{
protected:
	CCurlHttpClient client;
	string response;
	void SetUp() override
	{
		client.verifyServer(false);
	}

	void TearDown() override
	{
		client.disconnect();
	}
};

TEST_F(CWebHttpClientIT, shouldBeAbleToUseGetMethod)
{
	ASSERT_TRUE(client.connect("https://wiki.ryzom.dev/"));

	ASSERT_TRUE(client.sendGet("https://wiki.ryzom.dev/"));

	ASSERT_TRUE(client.receive(response));
	ASSERT_THAT(response, HasSubstr("HTTP"));
}
