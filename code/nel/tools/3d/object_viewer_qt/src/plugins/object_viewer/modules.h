/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MODULES_H
#define MODULES_H

#include "object_viewer.h"
#include "particle_editor.h"
#include "main_window.h"
#include "sound_system.h"
#include "vegetable_editor.h"

/**
@class Modules
@brief Main modules aggregated all parts of the program.
*/
class Modules
{
public:
	static void init();
	static void release();

	static NLQT::CObjectViewer &objView()
	{
		return *_objectViewer;
	}
	static NLQT::CMainWindow &mainWin()
	{
		return *_mainWindow;
	}
	static NLQT::CParticleEditor &psEdit()
	{
		return *_particleEditor;
	}
	static NLQT::CSoundSystem &sound()
	{
		return *_soundSystem;
	}
	static NLQT::CVegetableEditor &veget()
	{
		return *_vegetableEditor;
	}

private:
	static NLQT::CObjectViewer *_objectViewer;
	static NLQT::CMainWindow *_mainWindow;
	static NLQT::CParticleEditor *_particleEditor;
	static NLQT::CSoundSystem *_soundSystem;
	static NLQT::CVegetableEditor *_vegetableEditor;
};

#endif // MODULES_H
