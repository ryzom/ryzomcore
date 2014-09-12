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



#ifndef CL_MAIN_LOOP_H
#define CL_MAIN_LOOP_H

#include "nel/misc/types_nl.h"

class CChatWindow;

const uint NUM_MISSION_OPTIONS = 8;

// Main loop of the application (displayer, input, ...).
bool mainLoop();

// render all
void doRenderScene(bool wantTraversals, bool keepTraversals);
void renderScene(bool forceFullDetail, bool bloom);
void setDefaultChatWindow(CChatWindow *defaultChatWindow);

// Commit sky scene camera for rendering
void commitCamera();

void updateDayNightCycleHour();


enum TScreenshotRequest
{
	ScreenshotRequestNone = 0,
	ScreenshotRequestTGA,
	ScreenshotRequestJPG,
	ScreenshotRequestPNG
};

extern TScreenshotRequest   ScreenshotRequest;
extern bool					ShowInterface;	// Do the Chat OSD have to be displayed.


void destroyLoadingBitmap ();
void drawLoadingBitmap (float progress);
void displayDebugUIUnderMouse();

// in-game helper : pop messages to signal that the patch isn't completed yet
void inGamePatchUncompleteWarning();


// active/desactive bloom config interface
void initBloomConfigUI();

#endif // CL_MAIN_LOOP_H

/* End of main_loop.h */





















