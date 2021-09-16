// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef CL_INIT_MAIN_LOOP_H
#define CL_INIT_MAIN_LOOP_H

#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/progress_callback.h"
#include "global.h"

#include "3d_notes.h"

namespace NL3D
{
	class UTextureFile;
};

// active/desactive welcome window
void initWelcomeWindow();

// popup to offer hardware cursor activaction
void initHardwareCursor(bool secondCall = false);

// Initialize the main loop.
void initMainLoop();


// *** Loading sessions

// Start a loading session
void beginLoading (TBackground background);

// End a loading session
void endLoading ();

// Set the loading continent
void setLoadingContinent (class CContinent *continent);

extern CProgress				ProgressBar;
//extern C3DNotes					Notes;

extern uint64 StartInitTime;
extern uint64 StartPlayTime;

#if FINAL_VERSION
#define USE_ESCAPE_DURING_LOADING false
#else // FINAL_VERSION
#define USE_ESCAPE_DURING_LOADING true
#endif // FINAL_VERSION

extern bool					UseEscapeDuringLoading;

void loadBackgroundBitmap (TBackground background);

#endif // CL_INIT_MAIN_LOOP_H

/* End of init_main_loop.h */
