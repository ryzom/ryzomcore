#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

#include <nelns/login_service/login_service.h>


class CLoginServiceIT : public testing::Test
{
protected:
	CLoginService loginService;
	std::chrono::seconds defaultTimeout = std::chrono::seconds(10);

	void SetUp() override
	{
	}

	void TearDown() override
	{
	}
};

TEST_F(CLoginServiceIT, shouldAnswerToVerifyLoginPassword)
{
}
