// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2012-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <nel/misc/types_nl.h>

// STL includes
#include <stdio.h>
#ifdef NL_OS_WINDOWS
#	include <conio.h>
#endif

// NeL includes
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/path.h>
#include <nel/misc/vector.h>
#include <nel/misc/i18n.h>
#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>
#include <nel/sound/u_stream_source.h>
#include <nel/sound/u_group_controller.h>
#include <nel/sound/audio_decoder.h>
#include <nel/misc/hierarchical_timer.h>

// Project includes

#ifndef NL_SOUND_DATA
#define NL_SOUND_DATA "."
#endif // NL_SOUND_DATA

#define SAMPLE_OGG "D:/source/kaetemi/toverhex/src/samples/music_stream/data/aeon_1_10_mystic_river.ogg"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace NLSAMPLE {

static UAudioMixer *s_AudioMixer = NULL;
static UStreamSource *s_StreamSource = NULL;
static IAudioDecoder *s_AudioDecoder = NULL;
static UGroupController *s_GroupController = NULL;

static void initSample()
{
	if (!INelContext::isContextInitialised())
		new CApplicationContext();
	CPath::addSearchPath(NL_SOUND_DATA"/database/build/", true, false);
	
	printf("Sample demonstrating OGG playback using UStreamSource.");
	printf("\n\n");
	
	s_AudioMixer = UAudioMixer::createAudioMixer();
	
	printf("Select NLSOUND Driver:\n");
	printf(" [1] FMod\n");
	printf(" [2] OpenAl\n");
	printf(" [3] DSound\n");
	printf(" [4] XAudio2\n");
	printf("> ");
	int selection = getchar(); getchar();
	printf("\n");
	
	// init with 128 tracks, EAX enabled, no ADPCM, and automatic sample bank loading
	s_AudioMixer->init(8, true, false, NULL, true, (UAudioMixer::TDriver)(selection - '0'));
	s_AudioMixer->setLowWaterMark(1);

	CVector initpos(0.0f, 0.0f, 0.0f);
	CVector frontvec(0.0f, 1.0f, 0.0f);
	CVector upvec(0.0f, 0.0f, 1.0f);
	s_AudioMixer->getListener()->setPos(initpos);
	s_AudioMixer->getListener()->setOrientation(frontvec, upvec);
	
	//NLMISC::CHTimer::startBench();

	USource *source = s_AudioMixer->createSource(CStringMapper::map("default_stream"));
	nlassert(source);
	s_StreamSource = dynamic_cast<UStreamSource *>(source);
	nlassert(s_StreamSource);
	source->setSourceRelativeMode(true);

	string sample = SAMPLE_OGG;
	s_AudioDecoder = IAudioDecoder::createAudioDecoder(sample, false, false);
	s_StreamSource->setFormat(s_AudioDecoder->getChannels(), s_AudioDecoder->getBitsPerSample(), (uint32)s_AudioDecoder->getSamplesPerSec());
	//s_StreamSource->setPitch(2.0f);

	s_GroupController = s_AudioMixer->getGroupController("sound:dialog");
}

//CMutex *s_Mutex = NULL;
//
//class CTestThread : public IRunnable
//{
//	virtual void run()
//	{
//		for (int i = 0; i < 100000000; ++i)
//		{
//			s_Mutex->enter();
//			s_Mutex->leave();
//		}
//	}
//};

static void bufferMore(uint bytes) // buffer from bytes to bytes * 2
{
	uint8 *buffer = s_StreamSource->lock(bytes * 2);
	if (buffer)
	{
		uint32 result = s_AudioDecoder->getNextBytes(buffer, bytes, bytes * 2);
		s_StreamSource->unlock(result);
		// printf("|");
	}
}

static void runSample()
{
	uint samples, bytes;
	s_StreamSource->getRecommendedBufferSize(samples, bytes);

	bufferMore(bytes);
	//bufferMore(bytes);

	s_StreamSource->play();
	
	printf("Change volume with - and +\n");
	printf("Press ANY other key to exit\n");
	while (!s_AudioDecoder->isMusicEnded())
	{
#ifdef NL_OS_WINDOWS
		if (_kbhit())
		{
			switch (_getch())
#else
		char ch;
		if (read(0, &ch, 1))
		{
			switch (ch)
#endif
			{
			case '+':
				s_GroupController->setGain(s_GroupController->getGain() + 0.1f);
				break;
			case '-':
				s_GroupController->setGain(s_GroupController->getGain() - 0.1f);
				break;
			default:
				return;
			}
		}
		bufferMore(bytes);
		s_AudioMixer->update();
		//if (!s_StreamSource->asUSource()->isst)
		//{
		//	printf("*!playing!*");
		//	s_StreamSource->asUSource()->play();
		//}
		nlSleep(s_StreamSource->getRecommendedSleepTime());
	}
	printf("Last buffer streamed\n");
	while (s_StreamSource->hasFilledBuffersAvailable())
	{
		nlSleep(40);
		s_AudioMixer->update();
	}
	
	s_StreamSource->stop();
	
	printf("End of song\n");
	printf("Press ANY key to exit\n");
#ifdef NL_OS_WINDOWS
	while (!_kbhit()) 
#else
	char ch;
	while (!read(0, &ch, 1))
#endif
	{ s_AudioMixer->update(); nlSleep(10); }
	return;
}

static void releaseSample()
{
	//NLMISC::CHTimer::clear();
	s_GroupController = NULL;
	delete s_AudioDecoder; s_AudioDecoder = NULL;
	delete s_StreamSource; s_StreamSource = NULL;
	delete s_AudioMixer; s_AudioMixer = NULL;
}



} /* namespace NLSAMPLE */

int main()
{
	NLSAMPLE::initSample();
	NLSAMPLE::runSample();
	NLSAMPLE::releaseSample();
	return 0;
}

/* end of file */
