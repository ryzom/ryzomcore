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

// object_viewer_exe.cpp : Defines the entry point for the application.
//

#include "std_afx.h"


//#include "nel/3d/register_3d.h"
//#include "nel/3d/scene.h"
#include "../object_viewer/object_viewer_interface.h"
//#include "nel/3d/init_3d.h"

//using namespace NL3D;
using namespace NLMISC;
using namespace std;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// Register 3d
	// vl: the init doesn't seem to be important here
//	registerSerial3d ();
//	CScene::registerBasics ();
//	init3d ();

	// Remove command line ""
	char commandLine[512];
	char *commandLinePtr=commandLine;
	strcpy (commandLine, lpCmdLine);
	if (commandLine[0]=='"')
		commandLinePtr++;
	if (commandLinePtr[strlen (commandLinePtr)-1]=='"')
		commandLinePtr[strlen (commandLinePtr)-1]=0;

 	// Create a object viewer
	IObjectViewer *objectViewer=IObjectViewer::getInterface();

	if (objectViewer)
	{
		// Init ui
		if (objectViewer->initUI ())
		{
			// Argument ?
			if (strcmp (commandLinePtr, "")!=0)
			{
				// Make a string vector
				vector<string> strVector;
				strVector.push_back (commandLinePtr);

				// Try to load a shape
				if (objectViewer->loadMesh (strVector, ""))
				{
					// Reset the camera
					objectViewer->resetCamera ();
				}
				else
					return 1;
			}

			// Go
			objectViewer->go ();

			// Release ui
			objectViewer->releaseUI ();
		}

		// Delete the pointer
		IObjectViewer::releaseInterface (objectViewer);
	}


	return 0;
}
