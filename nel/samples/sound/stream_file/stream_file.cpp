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
#include <nel/sound/u_group_controller.h>
#include <nel/misc/hierarchical_timer.h>

// For direct play/pause control.
// You should never include this!
#include <nel/sound/stream_file_source.h>

// Project includes

#ifndef NL_SOUND_DATA
#define NL_SOUND_DATA "."
#endif // NL_SOUND_DATA

#define RYZOM_DATA "P:/data"
#define SAMPLE_OGG "Pyr Entrance.ogg"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace NLSAMPLE {

static UAudioMixer *s_AudioMixer = NULL;
static USource *s_Source = NULL;
static CStreamFileSource *s_StreamFileSource = NULL;
static UGroupController *s_GroupController = NULL;

static void initSample()
{
	if (!INelContext::isContextInitialised())
		new CApplicationContext();
	CPath::addSearchPath(NL_SOUND_DATA"/data", true, false);
	
	printf("Sample demonstrating OGG playback using stream file .sound sheets.");
	printf("\n\n");
	
	s_AudioMixer = UAudioMixer::createAudioMixer();
	
	// Set the sample path before init, this allow the mixer to build the sample banks
	s_AudioMixer->setSamplePath(NL_SOUND_DATA"/data/samplebank");
	// Packed sheet option, this mean we want packed sheet generated in 'data' folder
	s_AudioMixer->setPackedSheetOption(NL_SOUND_DATA"/data", true);
	
	printf("Select NLSOUND Driver:\n");
	printf(" [1] FMod\n");
	printf(" [2] OpenAl\n");
	printf(" [3] DSound\n");
	printf(" [4] XAudio2\n");
	printf("> ");
	int selection = getchar(); getchar();
	printf("\n");
	
	// init with 8 tracks, EAX enabled, no ADPCM, and automatic sample bank loading
	s_AudioMixer->init(8, true, false, NULL, true, (UAudioMixer::TDriver)(selection - '0'));
	s_AudioMixer->setLowWaterMark(1);
	
	CVector initpos(0.0f, 0.0f, 0.0f);
	CVector frontvec(0.0f, 1.0f, 0.0f);
	CVector upvec(0.0f, 0.0f, 1.0f);
	s_AudioMixer->getListener()->setPos(initpos);
	s_AudioMixer->getListener()->setOrientation(frontvec, upvec);

	CPath::addSearchPath(RYZOM_DATA, true, false);
	
	//NLMISC::CHTimer::startBench();

	s_Source = s_AudioMixer->createSource(CStringMapper::map("stream_file"));
	nlassert(s_Source);
	s_StreamFileSource = dynamic_cast<CStreamFileSource *>(s_Source);
	nlassert(s_StreamFileSource);
	// s_Source->setSourceRelativeMode(true);	
	// s_Source->setPitch(2.0f);

	s_GroupController = s_AudioMixer->getGroupController("sound:dialog");
}

static void runSample()
{
	s_Source->play();
	
	printf("Change volume with - and +\n");
	printf("Press ANY other key to exit\n");
	for (; ; )
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
			case 'x':
				s_Source->stop();
				break;
			case 's':
				s_Source->play();
				break;
			case 'p':
				s_StreamFileSource->pause();
				break;
			case 'r':
				s_StreamFileSource->resume();
				break;
			case 'e':
				s_AudioMixer->playMusic(SAMPLE_OGG, 1000, true, false);
				break;
			default:
				return;
			}
		}

		s_AudioMixer->update();
		
		nlSleep(40);
	}
}

static void releaseSample()
{
	//NLMISC::CHTimer::clear();
	s_GroupController = NULL;
	s_StreamFileSource = NULL;
	delete s_Source; s_Source = NULL;
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
