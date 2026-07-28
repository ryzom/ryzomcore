/**
 * \file viewer_listener.h
 * \brief Paint mouse + window close listener classes.
 * \author Jan Boon (Kaetemi)
 *
 * Shared between viewer.cpp (owner + method bodies) and script_and_ui.cpp
 * (reads Paint fields from g_PaintCtx).
 */

/*
 * Copyright (C) 2026 by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef ZONE_PAINTER_VIEWER_LISTENER_H
#define ZONE_PAINTER_VIEWER_LISTENER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/events.h>
#include <nel/misc/rgba.h>
#include <nel/3d/camera.h>
#include <nel/3d/event_mouse_listener.h>
#include <nel/3d/viewport.h>

#include "zp_state.h"
#include "editor_ui.h"
#include "paint_core.h"
#include "script_api.h"

class CWindowCloseListener : public NLMISC::IEventListener
{
public:
	bool WindowActive;
	CWindowCloseListener() : WindowActive(true) { }
	virtual void operator()(const NLMISC::CEvent &event) NL_OVERRIDE
	{
		if (event == NLMISC::EventDestroyWindowId || event == NLMISC::EventCloseWindowId)
			WindowActive = false;
	}
};

class CPaintMouseListener : public NLMISC::IEventListener
{
public:
	enum TPaintMode { ModeTile = 0, ModeColor, ModeDisplace, ModeProp, ModePatch };

	/**
	 * Sub-object level inside ModePatch. Values are the legacy plugin's EP_* (rpo.h) so the
	 * level means the same thing here as in the file it came from.
	 *
	 * Deliberately NOT written back: SRPatchMesh carries a SelLevel field, but the pristine
	 * copy is only ever mutated where an edit demands it, and merely looking at a zone in
	 * vertex mode is not an edit. Writing it would dirty every file the artist opened.
	 */
	enum TSubObject { SubObject = 0, SubVertex, SubEdge, SubPatch, SubTile, SubCount };

	ZPPAINT::CPaintCore *Core;
	NL3D::CCamera *Camera; // unwrapped from UScene::getCam()
	ZPUI::CEditorUI *EditorUI;
	NL3D::CViewport Viewport;
	int CurTileSet;
	bool Mode256;
	bool Pressed;
	float MouseX, MouseY;
	bool HaveHover;
	uint HoverZone;
	sint32 HoverTile;
	uint StrokeZone;
	sint32 StrokeTile;
	int Mode;
	int SubObj; // TSubObject; meaningful only while Mode == ModePatch
	NLMISC::CRGBA BrushColor;
	float BrushRadius;
	uint BrushHardness, BrushOpacity;
	uint DisplaceIndex;

	CPaintMouseListener() : Core(NULL), Camera(NULL), EditorUI(NULL), CurTileSet(0), Mode256(false), Pressed(false),
		MouseX(0.5f), MouseY(0.5f), HaveHover(false), HoverZone(0), HoverTile(-1), StrokeZone(0), StrokeTile(-1),
		Mode(ModeTile), SubObj(SubObject), BrushColor(255, 255, 255, 255), BrushRadius(8.f), BrushHardness(128), BrushOpacity(255),
		DisplaceIndex(0) { }

	bool guiWantsMouse() const;
	void paintAtHover(bool cont);
	void updateHover();
	virtual void operator()(const NLMISC::CEvent &event) NL_OVERRIDE;
};

#endif
