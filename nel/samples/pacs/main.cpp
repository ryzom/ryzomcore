// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

LEFT/RIGHT/UP/DOWN : move the selected primitive
TAB : select the next primitive
CTRL+MOUSE : move the camera
WHEEL: zoom in/out
PRIOR/NEXT: move on the z axis the selected primitive
INSERT: add primitives
SPACE: start chaos (move all primitive in random manner)
DEL: remove a primitive
*/


// Misc includes
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>

// Pacs includes
#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_move_primitive.h>
#include <nel/pacs/u_collision_desc.h>

// 3d includes
#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_3d_mouse_listener.h>

#include "object.h"

#ifdef NL_OS_WINDOWS
// Just for the main function and ::MessageBox
#	include <windows.h>
#endif // NL_OS_WINDOWS

#ifndef NL_PACS_DATA
#define NL_PACS_DATA "."
#endif // NL_PACS_DATA

using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

// Some defines
#define ARENA_SIZE 50
#define NUM_CELL 10

#define SIZE_PRIMITIVE_MIN (1)
#define SIZE_PRIMITIVE_MAX 2
#define KEYBOARD_ACCEL (5.0)	// m.s-2
#define SHOCK_ABSORB (0.9)		//
#define DELTA_TIME (0.1)
#define GO (10)
#define BORDER_SIZE SIZE_PRIMITIVE_MAX
#define INIT_SPEED 9
#define MAX_WORLD_IMAGE 31

volatile bool synchro;

// Get a random size
double randomSize ()
{
	return (SIZE_PRIMITIVE_MAX - SIZE_PRIMITIVE_MIN)*((double)rand() / (double)RAND_MAX) + SIZE_PRIMITIVE_MIN;
}

// Get a random rotation
double randomRot ()
{
	return 2*(double)Pi*((double)rand() / (double)RAND_MAX);
}

// Get a random speed
CVectorD randomSpeed ()
{
	return CVectorD (	2*(double)INIT_SPEED*((double)rand() / (double)RAND_MAX) - INIT_SPEED,
						2*(double)INIT_SPEED*((double)rand() / (double)RAND_MAX) - INIT_SPEED, 0);
}

// Get a random pos
CVectorD randomPos ()
{
	return CVectorD (	((double)ARENA_SIZE - 2*BORDER_SIZE - SIZE_PRIMITIVE_MAX)*((double)rand() / (double)RAND_MAX) - ARENA_SIZE / 2.0 + BORDER_SIZE+ (SIZE_PRIMITIVE_MAX) / 2,
						((double)ARENA_SIZE - 2*BORDER_SIZE - SIZE_PRIMITIVE_MAX)*((double)rand() / (double)RAND_MAX) - ARENA_SIZE / 2.0 + BORDER_SIZE+ (SIZE_PRIMITIVE_MAX) / 2,
						0);
}

// Setup speed with keyboard
void keyboard (UDriver	*pDriver, double deltaTime, CObjectDyn &obj)
{
	// Add speed to selected object
	CVectorD speed (0,0,0);
	if (pDriver->AsyncListener.isKeyDown (KeyLEFT))
		speed.x+=KEYBOARD_ACCEL*deltaTime;
	if (pDriver->AsyncListener.isKeyDown (KeyRIGHT))
		speed.x-=KEYBOARD_ACCEL*deltaTime;
	if (pDriver->AsyncListener.isKeyDown (KeyUP))
		speed.y-=KEYBOARD_ACCEL*deltaTime;
	if (pDriver->AsyncListener.isKeyDown (KeyDOWN))
		speed.y+=KEYBOARD_ACCEL*deltaTime;
	if (pDriver->AsyncListener.isKeyDown (KeyPRIOR))
		speed.z+=KEYBOARD_ACCEL*deltaTime;
	if (pDriver->AsyncListener.isKeyDown (KeyNEXT))
		speed.z-=KEYBOARD_ACCEL*deltaTime;

	// Set new speed
	if (speed.norm() > 0)
		obj.setSpeed(obj.getSpeed ()+speed);
}

// ** Main entry

#ifdef NL_OS_WINDOWS
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else // NL_OS_WINDOWS
int main ()
#endif // NL_OS_WINDOWS
{
	NLMISC::CApplicationContext myApplicationContext;

	try
	{
		// Init search paths
		CPath::addSearchPath (NL_PACS_DATA"/shapes");
		
		// Create a driver
		UDriver	*pDriver=UDriver::createDriver(0);

		// Setup text context
		pDriver->setDisplay (UDriver::CMode(640, 480, 0));

		// Create a scene
		UScene *pScene = pDriver->createScene(false);
		nlassert(pScene);

		pScene->enableLightingSystem(true);
		pScene->setAmbientGlobal(CRGBA(128,128,128));

		// Create a container
		// This container has 31 world images. The first is a static world image, the others are dynamic.
		// Each frame, the sample eval the collision in the next dynamic world image.
		// This is to show the multiple world images functionality.
		UMoveContainer *container=UMoveContainer::createMoveContainer (-ARENA_SIZE/2.0, -ARENA_SIZE/2.0, ARENA_SIZE/2.0, ARENA_SIZE/2.0,
			NUM_CELL, NUM_CELL, (double)SIZE_PRIMITIVE_MAX*sqrt(2.0), MAX_WORLD_IMAGE);

		// Set the world image 0 as static (for borders)
		container->setAsStatic (0);

		// Array arena
		std::vector<CObjectDyn*>	arrayArena;

		// Create an arena with boxes
		int num_cell=ARENA_SIZE/SIZE_PRIMITIVE_MAX+1;
		for (int cell=0; cell<num_cell; cell++)
		{
			// Create static boxes around the test area
			arrayArena.push_back (new CObjectDyn (SIZE_PRIMITIVE_MAX, SIZE_PRIMITIVE_MAX, 10*SIZE_PRIMITIVE_MAX, 0, 
				CVectorD (SIZE_PRIMITIVE_MAX*cell-(ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, (ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, 0),
				CVectorD(0,0,0), true, *container, *pScene, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, 0, 1, 0));
			arrayArena.push_back (new CObjectDyn (SIZE_PRIMITIVE_MAX, SIZE_PRIMITIVE_MAX, 10*SIZE_PRIMITIVE_MAX, 0, 
				CVectorD (SIZE_PRIMITIVE_MAX*cell-(ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, -(ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, 0),
				CVectorD(0,0,0), true, *container, *pScene, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, 0, 1, 0));
			arrayArena.push_back (new CObjectDyn (SIZE_PRIMITIVE_MAX, SIZE_PRIMITIVE_MAX, 10*SIZE_PRIMITIVE_MAX, 0, 
				CVectorD ((ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, SIZE_PRIMITIVE_MAX*cell-(ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, 0),
				CVectorD(0,0,0), true, *container, *pScene, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, 0, 1, 0));
			arrayArena.push_back (new CObjectDyn (SIZE_PRIMITIVE_MAX, SIZE_PRIMITIVE_MAX, 10*SIZE_PRIMITIVE_MAX, 0, 
				CVectorD (-(ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, SIZE_PRIMITIVE_MAX*cell-(ARENA_SIZE-SIZE_PRIMITIVE_MAX)/2, 0),
				CVectorD(0,0,0), true, *container, *pScene, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, 0, 1, 0));
		}

		// Setup camera
		UCamera	pCam=pScene->getCam();
		pCam.setTransformMode (UTransformable::DirectMatrix);
		pCam.setPerspective ((float)Pi/2.f, 1.33f, 0.1f, 1000);

		// Setup the mouse listener
		U3dMouseListener *plistener=pDriver->create3dMouseListener ();
		plistener->setHotSpot (CVectorD (0,0,0));
		plistener->setFrustrum (pCam.getFrustum());
		plistener->setMatrix (pCam.getMatrix());
		plistener->setMouseMode (U3dMouseListener::edit3d);

		// Array of dynamic objects
		std::vector<CObjectDyn*>	arrayObj;

		// Create one object
		arrayObj.push_back (
			new CObjectDyn (randomSize (), randomSize (), randomPos (), CVectorD(0,0,0), true, *container, *pScene, 
				UMovePrimitive::Reflexion, UMovePrimitive::NotATrigger, 1, 30, 1));

		// Selected object
		uint selected=0;

		// Get time
		TTime lastTime=CTime::getLocalTime ();

		// Current world image
		uint worldImage=1;

		// Color to clear the background
		CRGBA clearColor;

		// Main loop
		while (pDriver->isActive() && (!pDriver->AsyncListener.isKeyPushed (KeyESCAPE)))
		{
			// Get the current time
			TTime newTime=CTime::getLocalTime ();
			double deltaTime=(double)(uint32)(newTime-lastTime)/1000.0;
			lastTime=newTime;

			// Insert objects in press INSERT
			if (pDriver->AsyncListener.isKeyDown (KeyINSERT))
			{
				if (pDriver->AsyncListener.isKeyDown (KeyMENU))
				{
					// Random cylinder and boxes
					if (rand()&1)
						arrayObj.push_back (new CObjectDyn (randomSize(), randomSize(), randomSize(), randomRot(), randomPos (), CVectorD(0,0,0), 
						false, *container, *pScene, UMovePrimitive::DoNothing, (UMovePrimitive::TTrigger)(UMovePrimitive::EnterTrigger|UMovePrimitive::ExitTrigger|UMovePrimitive::OverlapTrigger), 1, 30, worldImage));
					else
						arrayObj.push_back (new CObjectDyn (randomSize (), randomSize (), randomPos (), CVectorD(0,0,0), false, *container, 
						*pScene, UMovePrimitive::DoNothing, (UMovePrimitive::TTrigger)(UMovePrimitive::EnterTrigger|UMovePrimitive::ExitTrigger|UMovePrimitive::OverlapTrigger), 1, 30, worldImage));
				}
				else
				{
					// Random cylinder and boxes
					bool fixed = (rand()&1) != 0;
					if (rand()&1)
						arrayObj.push_back (new CObjectDyn (randomSize(), randomSize(), randomSize(), randomRot(), randomPos (), CVectorD(0,0,0), 
						true, *container, *pScene, fixed?UMovePrimitive::DoNothing:UMovePrimitive::Reflexion, UMovePrimitive::NotATrigger,
						1, 30, worldImage));
					else
						arrayObj.push_back (new CObjectDyn (randomSize (), randomSize (), randomPos (), CVectorD(0,0,0), true, *container, 
						*pScene, fixed?UMovePrimitive::DoNothing:UMovePrimitive::Reflexion, UMovePrimitive::NotATrigger, 1, 30, worldImage));
				}
			}

			// Make chaos if SPACE pressed
			if (pDriver->AsyncListener.isKeyDown (KeySPACE))
			{
				for (uint t=0; t<arrayObj.size(); t++)
				{
					if (!arrayObj[t]->Freezed)
						arrayObj[t]->setSpeed (randomSpeed ());
				}
			}

			// Keyboard
			if (!arrayObj.empty())
			{
				// Manipulate selected primitive
				keyboard (pDriver, deltaTime, *(arrayObj[selected]));

				// Remove a primitive if DELETE pressed
				if (pDriver->AsyncListener.isKeyDown (KeyDELETE))
				{
					// remove all but one
					if (arrayObj.size() > 1)
					{
						arrayObj[arrayObj.size()-1]->remove (*container, *pScene);
						arrayObj.resize (arrayObj.size()-1);
					}
				}

				// Check selected
				if (selected>=arrayObj.size())
					selected=(uint)arrayObj.size()-1;
				//if (selected<0)
				//	selected=0;

				// Change selected object if TAB pressed
				if (pDriver->AsyncListener.isKeyPushed (KeyTAB))
				{
					selected++;
					selected%=arrayObj.size();
				}
			}

			// Move primitives
			for (uint i=0; i<arrayObj.size(); i++)
				arrayObj[i]->tryMove (DELTA_TIME, *container, worldImage);

			// Eval static world image
			container->evalCollision (DELTA_TIME, 0);

			// Eval current dynamic world image
			container->evalCollision (DELTA_TIME, worldImage);
			
			// Simulation new position
			uint i;
			for (i=0; i<arrayObj.size(); i++)
				arrayObj[i]->doMove (DELTA_TIME, worldImage);

			// Check triggers
			clearColor=CRGBA::Black;

			// Setup view matrix
			if (!arrayObj.empty())
			{
				// Setup hotspot for the 3d listener
				plistener->setHotSpot (arrayObj[selected]->getPos());

				// Look at selected primitive
				pCam.lookAt (plistener->getViewMatrix().getPos (), arrayObj[selected]->getPos());
			}

			// Force listener current matrix
			plistener->setMatrix (pCam.getMatrix());

			// Clear
			pDriver->clearBuffers (clearColor);

			// Render
			pScene->render ();

			// Draw some lines
			pDriver->setMatrixMode3D(pCam);

			// Compute triggers
			uint in = 0;
			uint out = 0;
			uint inside = 0;
			for (i=0; i<container->getNumTriggerInfo(); i++)
			{
				uint8 type = container->getTriggerInfo(i).CollisionType;
				in += (type == UTriggerInfo::In)?1:0;
				out += (type == UTriggerInfo::Out)?1:0;
				inside += (type == UTriggerInfo::Inside)?1:0;
				if (type == UTriggerInfo::Out)
					int foobar = 0;
			}

			// Draw the trigger bar
			pDriver->setMatrixMode2D11();
			pDriver->drawQuad(0, 0.10f, 1.f * (float)in / 50.f, 0.15f, CRGBA::Red);
			pDriver->drawQuad(0, 0.05f, 1.f * (float)inside / 50.f, 0.10f, CRGBA::Green);
			pDriver->drawQuad(0, 0.00f, 1.f * (float)out / 50.f, 0.05f, CRGBA::Blue);
			
			// Swap
			pDriver->swapBuffers ();

			// Pump messages
			pDriver->EventServer.pump();

			// Next world image
			int nextImage=worldImage+1;
			if (nextImage>=MAX_WORLD_IMAGE)
				nextImage=1;
			container->duplicateWorldImage (worldImage, nextImage);
			worldImage=nextImage;
		}

		// Remove mouse listener
		pDriver->delete3dMouseListener (plistener);
	}
	catch (const Exception& e)
	{
#ifdef NL_OS_WINDOWS
		::MessageBox (NULL, e.what(), "Test collision move", MB_OK|MB_ICONEXCLAMATION);
#else // NL_OS_WINDOWS
		printf ("%s\n", e.what());
#endif // NL_OS_WINDOWS
	}

	return 0;
}
