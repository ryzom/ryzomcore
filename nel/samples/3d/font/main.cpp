// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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

#include "nel/misc/types_nl.h"

#include "nel/misc/event_emitter.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/path.h"
#include "nel/misc/random.h"

// look at 3dinit example
#include "nel/3d/nelu.h"

// used for font management
#include "nel/3d/font_manager.h"
#include "nel/3d/computed_string.h"
#include "nel/3d/text_context.h"
#include "nel/3d/driver_user.h"

#ifdef NL_OS_WINDOWS
	#ifndef NL_COMP_MINGW
		#define NOMINMAX
	#endif
	#include <windows.h>
#endif // NL_OS_WINDOWS

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

#ifndef FONT_DIR
#	define FONT_DIR "."
#endif

using namespace std;
using namespace NL3D;
using namespace NLMISC;

// Font demo state
static CFontManager *s_FontManager = NULL;
static CTextContext *s_TextContext = NULL;
static CComputedString *s_CsRotation = NULL;
static CComputedString *s_Cs3d = NULL;
static CComputedString *s_CsUnicode = NULL;
static CValueSmoother *s_SmoothFPS = NULL;
static float s_X = 0, s_Y = 0, s_Z = 0;
static float s_Scale = 1.0f, s_Way = 0.05f;
static float s_Angle = 0.0f;
static TTicks s_OldTick = 0;

void renderOneFrame()
{
	if (!CNELU::Driver->isFrameReady())
		return; // GPU busy, skip frame to avoid blocking browser event loop

	// look at 3dinit example
	CNELU::clearBuffers(CRGBA(120,120,0));

	// now, every frame, we have to render the computer string.
	s_X+=0.01f; s_Y+=0.1f, s_Z+=0.001f;
	CMatrix m;
	m.identity();
	m.translate(CVector(0.7f*4.0f/3.0f, 0.5, 0.5));
	m.rotateX(s_X);
	m.rotateY(s_Y);
	m.rotateZ(s_Z);
	s_Cs3d->render3D (*CNELU::Driver, m);

	s_TextContext->setColor (CRGBA (255, 255, 255));
	s_TextContext->setFontSize (40);
	s_TextContext->setHotSpot (CComputedString::BottomLeft);
	s_TextContext->printAt (0.5f, 0.7f, string("printAt"));

	s_TextContext->setColor (CRGBA (0, 0, 255));
	s_TextContext->setFontSize (40);
	s_TextContext->setHotSpot (CComputedString::BottomLeft);
	s_TextContext->printAt (0.0f, 0.0f, string("NeL"));

	s_Scale+=s_Way;
	if (s_Scale>4 || s_Scale < 1) s_Way = -s_Way;

	s_TextContext->setColor (CRGBA (200, 255, 64));
	s_TextContext->setFontSize (20);
	s_TextContext->setHotSpot (CComputedString::BottomLeft);
	s_TextContext->setScaleX (s_Scale);
	s_TextContext->setScaleZ (s_Scale);
	s_TextContext->printAt (0.1f, 0.3f, string("printAt Scale String"));

	s_TextContext->setHotSpot (CComputedString::TopLeft);
	s_TextContext->setScaleX (1.0f);
	s_TextContext->setScaleZ (1.0f);
	s_TextContext->printAt (0.1f, 0.25f, string("printAt NoScale String"));

	s_Angle+=0.01f;
	s_CsRotation->render2D (*CNELU::Driver, 0.2f, 0.7f, CComputedString::MiddleMiddle, 1, 1, s_Angle);

	s_CsUnicode->render2D (*CNELU::Driver, 1.0f, 0.15f, CComputedString::MiddleRight);

	s_TextContext->setColor (CRGBA (32, 64, 127));
	s_TextContext->setFontSize (65);
	s_TextContext->setHotSpot (CComputedString::MiddleRight);
	s_TextContext->printAt (1.0f, 0.85f, ucstring("printAt Unicode String"));

	s_TextContext->setColor (CRGBA (255, 127, 0));
	s_TextContext->setFontSize (20);
	s_TextContext->setHotSpot (CComputedString::BottomRight);
	s_TextContext->printAt (0.99f, 0.01f, string("Press <ESC> to quit"));

	{
		TTicks newTick = CTime::getPerformanceTime();
		double deltaTime = CTime::ticksToSecond (newTick-s_OldTick);
		s_OldTick = newTick;
		s_SmoothFPS->addValue((float)deltaTime);
		deltaTime = s_SmoothFPS->getSmoothValue ();
		if (deltaTime > 0.0)
		{
			s_TextContext->setFontSize(16);
			s_TextContext->setColor(CRGBA::Yellow);
			s_TextContext->setHotSpot(CComputedString::TopLeft);
			s_TextContext->printAt(0.01f, 0.99f, toString("FPS:%.f", 1.0f/deltaTime));
		}
	}

	// look 3dinit example
	CNELU::swapBuffers();
#ifndef __EMSCRIPTEN__
	CNELU::screenshot();
#endif

	// look at event example
	CNELU::EventServer.pump(true);
}

#ifdef NL_OS_WINDOWS
int WINAPI WinMain( HINSTANCE hInstance, 
									 HINSTANCE hPrevInstance, 
									 LPSTR lpCmdLine, 
									 int nCmdShow )
#else
int main(int argc, char **argv)
#endif
{
	// look at 3dinit example
	CNELU::init (800, 600, CViewport(), 32, true, 0, false, false); 

	NLMISC::CPath::addSearchPath(FONT_DIR);

	// create a font manager
	s_FontManager = new CFontManager;
	s_FontManager->setMaxMemory(2000000);

	s_TextContext = new CTextContext;
	s_TextContext->init (CNELU::Driver, s_FontManager);
	s_TextContext->setFontGenerator (NLMISC::CPath::lookup("beteckna.ttf"));

	s_CsRotation = new CComputedString;
	s_FontManager->computeString ("cs Rotation", s_TextContext->getFontGenerator(), CRGBA(255,255,255), 70, false, false, CNELU::Driver, *s_CsRotation);

	s_Cs3d = new CComputedString;
	s_FontManager->computeString ("cs 3d", s_TextContext->getFontGenerator(), CRGBA(255,127,0), 75, false, false, CNELU::Driver, *s_Cs3d);

	ucstring ucs("cs Unicode String");
	s_CsUnicode = new CComputedString;
	s_FontManager->computeString (ucs, s_TextContext->getFontGenerator(), CRGBA(32,64,127), 75, false, false, CNELU::Driver, *s_CsUnicode);

	// look at event example
	CNELU::EventServer.addEmitter(CNELU::Driver->getEventEmitter());
	CNELU::AsyncListener.addToServer(CNELU::EventServer);

	s_SmoothFPS = new CValueSmoother;
	s_OldTick = CTime::getPerformanceTime();

#ifdef __EMSCRIPTEN__
	EM_ASM({ if (window.nlLoadingComplete) window.nlLoadingComplete(); });
	emscripten_set_main_loop(renderOneFrame, 0, 1);
#else
	do
	{
		renderOneFrame();
	}
	while(!CNELU::AsyncListener.isKeyPushed(KeyESCAPE));
#endif

	s_FontManager->dumpCache ("font_cache_dump.tga");

	// look at event example
	CNELU::AsyncListener.removeFromServer(CNELU::EventServer);

	delete s_SmoothFPS;
	delete s_CsUnicode;
	delete s_Cs3d;
	delete s_CsRotation;
	delete s_TextContext;
	delete s_FontManager;

	// look at 3dinit example
	CNELU::release();

	return EXIT_SUCCESS;
}
