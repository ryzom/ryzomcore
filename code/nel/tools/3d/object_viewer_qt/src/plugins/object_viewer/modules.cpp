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

#include "stdpch.h"
#include "modules.h"

NLQT::CObjectViewer *Modules::_objectViewer = NULL;
NLQT::CMainWindow *Modules::_mainWindow = NULL;
NLQT::CParticleEditor *Modules::_particleEditor = NULL;
NLQT::CSoundSystem *Modules::_soundSystem = NULL;
NLQT::CVegetableEditor *Modules::_vegetableEditor = NULL;

void Modules::init()
{
	if (_objectViewer == NULL) _objectViewer = new NLQT::CObjectViewer;
	if (_soundSystem == NULL) _soundSystem = new NLQT::CSoundSystem;
	if (_particleEditor == NULL) _particleEditor = new NLQT::CParticleEditor;
	if (_vegetableEditor == NULL) _vegetableEditor = new NLQT::CVegetableEditor;
	if (_mainWindow == NULL) _mainWindow = new NLQT::CMainWindow;
}

void Modules::release()
{
//	delete _mainWindow;
	_mainWindow = NULL;
	delete _particleEditor;
	_particleEditor = NULL;
	delete _vegetableEditor;
	_vegetableEditor = NULL;
	delete _soundSystem;
	_soundSystem = NULL;
	delete _objectViewer;
	_objectViewer = NULL;
}
