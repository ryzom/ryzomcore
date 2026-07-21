/**
 * \file startup_ui.h
 * \brief NeL-GUI startup screens for zone_painter (world select / zone browser) — ui M2
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 *
 * Include contract: NLMISC + NL3D UDriver + NLGUI + editor_ui.h + workspace_discovery.h.
 * Must NOT include patch_eval.h, context_display.h, or SCENELIB headers. Zone .max loading
 * stays in main.cpp; this unit only discovers/selects and returns the chosen paths.
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef ZONE_PAINTER_STARTUP_UI_H
#define ZONE_PAINTER_STARTUP_UI_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

#include "workspace_discovery.h"

namespace NL3D {
class UDriver;
}

namespace ZPUI {

class CEditorUI;

enum EStartupResult
{
	StartupQuit = 0,         ///< ESC / window close
	StartupOpenZone,         ///< user (or auto) picked a zone — fill selection
	StartupScreenshotDone,   ///< --startup-screenshot wrote a frame
	StartupError = -1
};

struct SStartupSelection
{
	ZPWS::SWorldEntry World;
	ZPWS::SZoneEntry Zone;
};

/**
 * Run Screens A/B (and C when available) on an already-created UDriver + CEditorUI.
 * worlds may be empty (Screen C / empty list). On StartupOpenZone, *selection is filled
 * and the same select functions the buttons call were used.
 *
 * screenshotPath: if non-empty, render one frame of the first screen and return
 * StartupScreenshotDone without entering the interactive loop.
 *
 * folderBrowserEnabled: when true (M2c+), empty worlds open Screen C; Browse works.
 */
EStartupResult runStartupFlow(NL3D::UDriver *driver,
                              CEditorUI *editorUI,
                              std::vector<ZPWS::SWorldEntry> &worlds,
                              const std::string &screenshotPath,
                              SStartupSelection &selection,
                              bool folderBrowserEnabled,
                              const std::string &initialBrowsePath = std::string());

/** Same selection path buttons use — for --startup-auto thin wrapper reuse. */
bool startupSelectWorldZone(const std::vector<ZPWS::SWorldEntry> &worlds,
                            const std::string &autoPath,
                            SStartupSelection &selection,
                            std::string &err);

/** Show/hide startup windows + painter panel. */
void startupHideAllScreens();
void startupShowPainter(bool show);

} // namespace ZPUI

#endif // ZONE_PAINTER_STARTUP_UI_H

/* end of file */
