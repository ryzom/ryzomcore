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

#ifndef SBCLIENT_SNOWBALLS_CLIENT_H
#define SBCLIENT_SNOWBALLS_CLIENT_H
#include <nel/misc/types_nl.h>
#include "snowballs_config.h"

#include <nel/misc/types_nl.h>

// STL includes
// ...

// NeL includes
#include <nel/misc/rgba.h>
#include <nel/misc/ucstring.h>
#include <nel/misc/time_nl.h>
#include <nel/3d/animation_time.h>

// Project includes
// ...

namespace NLMISC {
	class CConfigFile;
}

namespace NL3D {
	class UDriver;
	class UScene;
	class UTextContext;
	class ULandscape;
	class IStereoDisplay;
	class IStereoHMD;
}

namespace SBCLIENT {
	class C3dMouseListener;

/*******************************************************************
 *                             GLOBALS                             *
 *******************************************************************/

class CGlobals
{
public:
	static void assertNull();
};

extern NL3D::UDriver *Driver;
extern NL3D::IStereoDisplay *StereoDisplay;
extern NL3D::IStereoHMD *StereoHMD;
extern NL3D::UScene *Scene;
extern NL3D::UTextContext *TextContext;
extern NLMISC::CConfigFile *ConfigFile;
extern NL3D::ULandscape *Landscape;
extern C3dMouseListener *MouseListener;

extern NLMISC::TLocalTime LocalTime; // use for delta only
extern NLMISC::TLocalTime LocalTimeDelta;
// extern NLMISC::TLocalTime ServerTime; // use for delta only
// extern NLMISC::TLocalTime ServerTimeDelta;
// extern NLMISC::TGameTime GameTime;
// extern NLMISC::TGameTime GameTimeDelta;
// extern NLMISC::TGameCycle GameCycle;
// extern NLMISC::TGameCycle GameCycleDelta;
extern NL3D::TGlobalAnimationTime AnimationTime;
extern NL3D::TAnimationTime AnimationTimeDelta;
extern float FramesPerSecond;
extern float FramesPerSecondSmooth;

// Stuff for connection
extern ucstring Login;

/*******************************************************************
 *                        SNOWBALLS CLIENT                         *
 *******************************************************************/

class CSnowballsClient
{
public:
	static void init();
	static bool run();
	static void release();
};

} /* namespace SBCLIENT */

#endif /* #ifndef SBCLIENT_SNOWBALLS_CLIENT_H */

/* end of file */
