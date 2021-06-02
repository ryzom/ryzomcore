/**
 * \file NeLDriver.cpp
 * \date November 2004
 * \author Matt Raykowski
 * \author Henri Kuuste
 */

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

#include "globals.h"
#include "resource.h"
#include "NeLDriver.h"

#include <CEGUIExceptions.h>

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	undef min
#	undef max
#endif

int frame = 0;

void NeLDriver::init() {
#ifdef NL_OS_WINDOWS
	HWND hWnd = (HWND )m_Driver->getDisplay();
	SetWindowText(hWnd,"CEGUI NeL Demo");
#endif

	// Create the window with config file values
	if (!m_Driver->setDisplay(NL3D::UDriver::CMode(800, 600, 32, true, 0))) {
		nlwarning ("Can't set display mode %d %d %d %d %d", 800, 600, 32, false, 0);
		return;
	}
	m_Driver->setFontManagerMaxMemory(2000000);
	m_TextContext = m_Driver->createTextContext(NLMISC::CPath::lookup("n019003l.pfb"));
	if(m_TextContext == 0) {
		nlwarning("Can't create text context");
		return;
	}
	m_TextContext->setKeep800x600Ratio(false);
	m_Driver->setAmbientColor(NLMISC::CRGBA(82, 100, 133, 255));
	m_Driver->enableFog(false);
	m_Scene = m_Driver->createScene(false);
	if(m_Scene == 0) {
		nlwarning("Can't create a NeL UScene");
		return;
	}
	m_Scene->getCam().setPerspective(NLMISC::degToRad(90.0f), 1.33f, 1.0f*GScale, 30000.0f*GScale);
	m_Scene->getCam().setTransformMode(NL3D::UTransformable::DirectMatrix);
	m_Scene->enableLightingSystem(true);
	m_Scene->setSunAmbient(NLMISC::CRGBA(82, 100, 133, 255));
	m_Scene->setSunDiffuse(NLMISC::CRGBA(255,255,255));
	m_Scene->setSunSpecular(NLMISC::CRGBA(255,255,255));
	m_Scene->setSunDirection(NLMISC::CVector(-1,0,-1));

	m_Scene->setPolygonBalancingMode(NL3D::UScene::PolygonBalancingOn);
	m_Scene->setGroupLoadMaxPolygon("Fx", 5000);

	// INITIALIZE TIMES
	m_FirstTime = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
	m_OldTime = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
	m_Time = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
	
}

void NeLDriver::update() {
	using namespace NLMISC;
	H_AUTO(NeLDriver_update);
	// UPDATE THE TIME.
	m_OldTime = m_Time;
	double newTime = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
	m_Time = newTime - m_FirstTime;
	m_DeltaTime = m_Time - m_OldTime;
	m_DeltaTimeSmooth.addValue(m_DeltaTime);

	// 3D
	m_Scene->animate(m_Time);

	// INPUT
	m_Driver->EventServer.pump();
}

double NeLDriver::getFps() {
	return m_DeltaTimeSmooth.getSmoothValue() ? 1.0 / m_DeltaTimeSmooth.getSmoothValue() : 0.0;
}

void NeLDriver::render() {
	using namespace NLMISC;
	H_AUTO(NeLDriver_render);
	m_Scene->render();
}

NL3D::UDriver		&NeLDriver::getDriver() const {
	return *m_Driver;
}

NL3D::UScene		&NeLDriver::getScene() const {
	return *m_Scene;
}

NL3D::UTextContext	&NeLDriver::getTextContext() const {
	return *m_TextContext;
}

