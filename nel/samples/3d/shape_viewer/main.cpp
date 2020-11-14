// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


//
// Includes
//

#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/common.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_light.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_instance.h>

#ifdef NL_OS_WINDOWS
	#ifndef NL_COMP_MINGW
		#define NOMINMAX
	#endif
	#include <windows.h>
#endif // NL_OS_WINDOWS

using namespace NLMISC;
using namespace NL3D;

// function to split a string into several substrings delimited by specified characters
void split(const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters)
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

#ifdef NL_OS_WINDOWS
sint WINAPI WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR cmdline, int /* nCmdShow */)
{
	// we can specify several shapes on command line
	// so we need to process it
	std::vector<std::string> argv;

	// argv[0] is the fullpath to the executable but since it's not used, we can let it empty
	argv.push_back("");

	// split the whole cmdline into args
	split(cmdline, argv, " ");

	// args number
	int argc = (int)argv.size();

#else
sint main(int argc, char **argv)
{
#endif
	NLMISC::CApplicationContext myApplicationContext;

	try
	{
		// create OpenGL driver
		NL3D::UDriver *Driver = UDriver::createDriver();
		if (!Driver) throw 2;

		// create a window in 800x600
		Driver->setDisplay(UDriver::CMode(800, 600, 32, true));

		// set the title
		Driver->setWindowTitle(ucstring("NeL shape viewer"));

		// can use both dds and tga textures for shapes
		CPath::remapExtension ("dds", "tga", true);

		// create the light
		ULight *Light = ULight::createLight();
		if (!Light) throw 3;

		// set mode of the light
		Light->setMode(ULight::DirectionalLight);

		// set position of the light
		Light->setPosition(CVector(-20.f, 30.f, 10.f));

		// white light
		Light->setAmbiant(CRGBA(255, 255, 255));

		// set and enable the light
		Driver->setLight(0, *Light);
		Driver->enableLight(0);

		// Create a scene
		NL3D::UScene *Scene = Driver->createScene(true);
		if (!Scene) throw 4;

		// create the camera
		UCamera Camera = Scene->getCam();
		if (Camera.empty()) throw 5;

		Camera.setTransformMode (UTransformable::DirectMatrix);
		Camera.setPerspective ((float)Pi/2.f, 1.33f, 0.1f, 1000);

		// camera will look at entities
		Camera.lookAt (CVector(-10.f, 10.f, 0.f), CVector(0.f, 0.f, 0.f));

		// create entities
		std::vector<UInstance> Entities;

		// create each entity
		for(int i = 1; i < argc; ++i)
		{
			// use the path of the shape to find its textures
			CPath::addSearchPath(CFile::getPath(argv[i]), true, false);

			// add an entity to the scene
			UInstance Entity = Scene->createInstance(argv[i]);

			// if we can't create entity, skip it
			if (Entity.empty()) continue;

			// we will rotate it later so use Euler rotation transform mode
			Entity.setTransformMode(UTransformable::RotEuler);

			// add entity to the vector
			Entities.push_back(Entity);
		}

		// initial angle
		float angle = 0.f;

		// main loop
		while (Driver->isActive())
		{
			Driver->EventServer.pump();
			
			// the background is black
			Driver->clearBuffers(CRGBA(0, 0, 0));

			// increase the angle
			angle += 0.1f;

			if (angle >= NLMISC::Pi*2) angle = 0.f;

			// rotate all entities
			for(size_t i = 0; i < Entities.size(); ++i)
			{
				Entities[i].setRotEuler(0.f, 0.f, angle);
			}

			// animate the scene
			Scene->animate(NLMISC::CTime::getLocalTime() / 1000.0);

			// render the scene
			Scene->render();

			// show the scene
			Driver->swapBuffers();

			// escape will leave the program
			if (Driver->AsyncListener.isKeyPushed(KeyESCAPE))
			{
				break;
			}
			// F3 will change the render mode
			else if (Driver->AsyncListener.isKeyPushed(KeyF3))
			{
				UDriver::TPolygonMode p = Driver->getPolygonMode();
				p = UDriver::TPolygonMode(((int)p+1)%3);
				Driver->setPolygonMode(p);
			}
			// F12 will take a screenshot
			else if (Driver->AsyncListener.isKeyPushed(KeyF12))
			{
				CBitmap btm;
				Driver->getBuffer(btm);
				COFile fs(CFile::findNewFile("screenshot.png"));
				btm.writePNG(fs,24);
			}
		}

		// we are leaving the program

		// delete all entities
		for(size_t i = 0; i < Entities.size(); ++i)
		{
			Scene->deleteInstance(Entities[i]);
		}

		// delete the scene
		Driver->deleteScene(Scene);

		// delete the light
		delete Light;

		// release all textures and others elements
		Driver->release();

		// delete the driver
		delete Driver;
	}
	catch(int a)
	{
		return a;
	}
	catch(...)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
