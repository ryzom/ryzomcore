#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include <nel/misc/app_context.h>
#include <nel/sound/u_audio_mixer.h>

using std::string;

using ::testing::Contains;
using ::testing::StrEq;

using namespace NLSOUND;

class CSoundOpenALIT : public testing::Test
{
protected:
	NLMISC::CApplicationContext context;
	UAudioMixer	*AudioMixer = nullptr;

	void SetUp() override
	{
		context = NLMISC::CApplicationContext();
		NLMISC::createDebug(nullptr);

		AudioMixer = UAudioMixer::createAudioMixer();
	}

	void TearDown() override
	{
	}
};

TEST_F(CSoundOpenALIT, shouldBeToLoadOpenALDriver)
{
	const auto drivers = UAudioMixer::getDrivers();

	ASSERT_THAT(drivers, Contains(
		Field("Name", &UAudioMixer::TDriverInfo::Name, StrEq("OpenAL"))
	));
}
