#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <snowballs/position/physics.h>

using ::std::string;
using ::std::vector;

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;

class physicsTest : public testing::Test
{
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}
};

TEST_F(physicsTest, shouldStartAtPosition)
{
	CTrajectory physics;
	NLMISC::CVector position(0, 0, 0);
	NLMISC::CVector target(1, 0, 0);
	float speed(10);
	NLMISC::TTime startTime(0);

	physics.init(position, target, speed, startTime);

	EXPECT_THAT(physics.eval(startTime), Eq(position));
}
