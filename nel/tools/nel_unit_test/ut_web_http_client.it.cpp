#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include <nel/web/http_client_curl.h>

using std::string;

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
