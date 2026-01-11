// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
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


/*
 * This sample shows how to initialize the audio mixer (UAudioMixer),
 * how to create a source (USource), and how to move the listener
 * (UListener).
 */


#include "nel/misc/debug.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/path.h"
#include "nel/misc/vector.h"
using namespace NLMISC;

#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_listener.h"
using namespace NLSOUND;

#include <stdio.h>

#ifndef NL_SOUND_DATA
#define NL_SOUND_DATA "."
#endif // NL_SOUND_DATA

// Pointer to the audio mixer object
UAudioMixer	*AudioMixer = NULL;


/*
 * Initialization
 */
void Init()
{
	try
	{

		CPath::addSearchPath(NL_SOUND_DATA"/data", true, false);

		/*
		 * 1. Create the audio mixer object and init it.
		 * If the sound driver cannot be loaded, an exception is thrown.
		 */
		AudioMixer = UAudioMixer::createAudioMixer();

		// Set the sample path before init, this allow the mixer to build the sample banks
		AudioMixer->setSamplePath("data/samplebank");
		// Packed sheet option, this mean we want packed sheet generated in 'data' folder
		AudioMixer->setPackedSheetOption("data", true);

		printf("Select NLSOUND Driver:\n");
		printf(" [1] FMod\n");
		printf(" [2] OpenAl\n");
		printf(" [3] DSound\n");
		printf(" [4] XAudio2\n");
		printf("> ");
		int selection = getchar();
		printf("\n");

		// init with 32 tracks, EAX enabled, no ADPCM, and activate automatic sample bank loading
		AudioMixer->init(32, true, false, NULL, true, (UAudioMixer::TDriver)(selection - '0')/*UAudioMixer::DriverFMod*/);

		/*
		 * 2. Initialize listener's position and orientation (in NeL coordinate system).
		 */
		CVector initpos ( 0.0f, 0.0f, 0.0f );
		CVector frontvec( 0.0f, 1.0f, 0.0f );
		CVector upvec( 0.0f, 0.0f, 1.0f );
		AudioMixer->getListener()->setPos( initpos );
		AudioMixer->getListener()->setOrientation( frontvec, upvec );

	}
	catch(const Exception &e)
	{
		nlerror( "Error: %s", e.what() );
	}
}


/*
 * Adding a source
 */
USource *OnAddSource( const char *name, float x, float y, float z )
{
	/*
	 * Create a source with sound 'name', and set some of its initial properties, if successful
	 */
	USource *source = AudioMixer->createSource( CStringMapper::map(name) );
	if ( source != NULL )
	{
		source->setPos( CVector(x,y,z) );

		/* The initial gain, pitch and looping state are stored
		 * in the "source sounds file".
		 */
		source->setLooping(true);
		source->play(); // start playing immediately
	}
	else
	{
		nlwarning( "Sound '%s' not found", name );
	}
	return source;
}


/*
 * When moving the listener, wait for a short delay
 */
void OnMove( const CVector& listenerpos )
{
	// Move forward
	AudioMixer->getListener()->setPos( listenerpos );

	// Wait 20 ms
	TTime time = CTime::getLocalTime();
	while ( CTime::getLocalTime() < time+20 );

	/* If we used spawned sources or "envsounds" or if we had
	 * a large number of sources, we should call:
	 * AudioMixer->update();
	 */
}

/*
 * main
 *
 * Note: The NeL vector coordinate system is described as follows:
 * \verbatim
 *     (top)
 *       z
 *       |  y (front)
 *       | /
 *       -----x (right)
 * \endverbatim
 */
int main()
{
	CApplicationContext *appContext = new CApplicationContext(); // crash at end if on stack ...

	// Initialization
	Init();

	// First step: we create two sources
	printf( "Press ENTER to start playing the two sources\n" );
	printf( "One is 20 meters ahead, on the right\n" );
	printf( "The other is 5 meters ahead, on the left\n" );
	getchar();
	USource *src1 = OnAddSource( "beep", 1.0f, 20.0f, 0.0f );  // Beep on the right, 20 meters ahead
	USource *src2 = OnAddSource( "tuut", -2.0f, 5.0f, 0.0f ); // Tuut on the left, 5 meters ahead

	// Second step: we will move the listener ahead
	printf( "Press ENTER again to start moving the listener\n" );
	getchar();

	// Listener orientation is constant in this example (see initialization)

	// Move forward, then backward, twice
	CVector listenervel;
	CVector listenerpos ( 0.0f, 0.0f, 0.0f );
	for ( uint i=0; i!=2; i++ )
	{
		printf( "%u of 2\n", i+1 );

		// Forward velocity
		listenervel.set( 0.0f, 0.5f, 0.0f );
		AudioMixer->getListener()->setVelocity( listenervel );

		// Move forward: set position every frame
		printf( "Moving forward, going past the sources...\n" );
		for ( listenerpos.y=0.0f; listenerpos.y<30.0f; listenerpos.y+=0.1f )
		{
			OnMove( listenerpos );
			AudioMixer->update();
		}

		// Backward velocity
		listenervel.set( 0.0f, -0.5f, 0.0f );
		AudioMixer->getListener()->setVelocity( listenervel );

		// Move backward: set position every frame
		printf( "Moving backward, going back to the start position...\n" );
		for ( listenerpos.y=30.0f; listenerpos.y>0.0f; listenerpos.y-=0.1f )
		{
			OnMove( listenerpos );
			AudioMixer->update();
		}
	}

	// Finalization
	printf( "Press ENTER again to exit\n" );
	getchar();

	delete src1; delete src2;
	delete AudioMixer;

	delete appContext;

	return 0;
}
