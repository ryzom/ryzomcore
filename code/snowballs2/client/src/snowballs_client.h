/**
 * \file snowballs_client.h
 * \brief CSnowballsClient
 * \date 2008-11-26 13:13GMT
 * \author NeL Developers
 * CSnowballsClient
 * Snowballs 2 main file
 */

/* 
 * Copyright (C) 2008  by authors
 * 
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

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
