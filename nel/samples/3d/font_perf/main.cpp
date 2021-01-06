// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2020  Winch Gate Property Limited
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

#ifndef FONT_DIR
#	define FONT_DIR "."
#endif

using namespace std;
using namespace NL3D;
using namespace NLMISC;

int main(int argc, char **argv)
{
	// look at 3dinit example
	CNELU::init (800, 600, CViewport(), 32, true, 0, false, false);

	NLMISC::CPath::addSearchPath(FONT_DIR);

	// create a font manager
	CFontManager fontManager;

	// set the font cache to 2 megabytes (default is 1mb)
	fontManager.setMaxMemory(2000000);

	CTextContext tc;

	tc.init (CNELU::Driver, &fontManager);

	// The first param is the font name (could be ttf, pfb, fon, etc...). The
	// second one is optional, it's the font kerning file
	tc.setFontGenerator (NLMISC::CPath::lookup("beteckna.ttf"));

	NLMISC::CRandom rnd;

	uint nbCount = 100000;
	TTicks startTick = CTime::getPerformanceTime();
	std::string txt;
	for(uint i = 0; i < nbCount; ++i)
	{
		uint fontSize = rnd.rand(200);
		bool embolden = rnd.rand(1) == 1;
		bool oblique = rnd.rand(1) == 1;
		txt = toString("Lorem ipsum %03d", fontSize);

		CComputedString cs;
		fontManager.computeString(txt, tc.getFontGenerator(), CRGBA::White, fontSize, embolden, oblique, CNELU::Driver, cs);
	}

	TTicks endTick = CTime::getPerformanceTime();

	double deltaTime = CTime::ticksToSecond(endTick-startTick);
	std::string msg = toString("Generated %d strings in %.2fs\n", nbCount, deltaTime);

	nlinfo("%s", msg.c_str());
	printf("%s", msg.c_str());

	fontManager.dumpCache ("font_pref_cache_dump.tga");

	// look at 3dinit example
	CNELU::release();

	return EXIT_SUCCESS;
}
