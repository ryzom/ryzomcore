/**
 * \file editor_ui.h
 * \brief In-engine NLGUI facade for the standalone zone painter (ui M1)
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 *
 * Embeds NLGUI over the viewer's UDriver (the sample at nel/samples/gui is the
 * embedding contract). Owns atlas/parser init, per-frame update+draw, visibility
 * (F10 / ToggleUI), and wantsMouse() so paint/orbit do not punch through windows.
 *
 * Paint ops never live here: a SPaintUIBridge points at named handlers owned by
 * runViewer so keys and GUI buttons share one implementation.
 *
 * TU hygiene: this unit must NOT include patch_eval.h or context_display.h /
 * SCENELIB headers (see main.cpp include contract).
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

#ifndef ZONE_PAINTER_EDITOR_UI_H
#define ZONE_PAINTER_EDITOR_UI_H

#include <nel/misc/types_nl.h>

#include <set>
#include <string>

namespace NL3D {
class UDriver;
class UTextContext;
}

namespace NLGUI {
class CEventListener;
}

namespace ZPUI {

/** Pointer-button state mirror (NLGUI does not feed button-down into the
 *	view pointer on its own; the client / GUI sample do this). */
class CPointerButtonListener;

/**
 * Bridge between the viewer paint state and the NLGUI action handlers.
 * runViewer fills the state fields each frame and installs the function
 * pointers once; action handlers call the functions (never reimplement ops).
 */
struct SPaintUIBridge
{
	// Named handlers (same paths as the keyboard)
	void (*selectMode)(int mode);       // 0=Tile 1=Color 2=Displace
	void (*selectTileSetDelta)(int d);  // -1 / +1
	void (*toggleTileSize)();           // 128 <-> 256
	void (*brushSizeDelta)(int d);      // -1 / +1 (0..2 for tile/disp)
	void (*groupDelta)(int d);          // -1 / +1 (mod 13)
	void (*toggleLockBorders)();
	void (*undo)();
	void (*redo)();
	void (*fill)(int rot);              // 0..3
	void (*save)();                     // write-back + whole-file; no exit

	// State snapshot for panel sync (filled by runViewer each frame)
	bool HaveCore;
	int Mode;
	int CurTileSet;
	uint TileSetCount;
	char TileSetName[128];
	bool Mode256;
	uint BrushSize;
	uint TileGroup;
	bool LockBorders;
	uint UndoDepth;
	bool CanSave;

	SPaintUIBridge()
		: selectMode(NULL), selectTileSetDelta(NULL), toggleTileSize(NULL),
		  brushSizeDelta(NULL), groupDelta(NULL), toggleLockBorders(NULL),
		  undo(NULL), redo(NULL), fill(NULL), save(NULL),
		  HaveCore(false), Mode(0), CurTileSet(0), TileSetCount(0), Mode256(false),
		  BrushSize(0), TileGroup(0), LockBorders(false), UndoDepth(0), CanSave(false)
	{
		TileSetName[0] = 0;
	}
};

/** Install / clear the process-wide bridge (action handlers look it up). */
void setPaintUIBridge(SPaintUIBridge *bridge);
SPaintUIBridge *getPaintUIBridge();

/**
 * Viewer-side NLGUI shell. init() is optional-fail: if the atlas or interface
 * XML is missing the viewer continues with keyboard+HUD only.
 */
class CEditorUI
{
public:
	CEditorUI();
	~CEditorUI();

	/** Search paths → CViewRenderer → atlas → parse ui/*.xml → activate master
	 *	group. Returns false on soft failure (viewer keeps working without UI). */
	bool init(NL3D::UDriver *driver, const std::string &fontPathHint);

	void shutdown();

	/** Per-frame: push bridge state into widgets, then widget clock + coords. */
	void update();

	/** Draw GUI over the 3D scene (after scene render, before swap). */
	void draw();

	/** True when the cursor is over a GUI window or the GUI has pointer capture.
	 *	Paint strokes and orbit navigation must not consume those events. */
	bool wantsMouse() const;

	bool isReady() const { return _Ready; }
	bool isVisible() const { return _Visible; }
	void setVisible(bool visible);
	void toggleVisible();

private:
	void syncPanelFromBridge();

	bool _Ready;
	bool _Visible;
	NL3D::UDriver *_Driver;
	NL3D::UTextContext *_TextContext;
	NLGUI::CEventListener *_GuiListener;
	CPointerButtonListener *_PointerButtons;
	std::set<std::string> _HwCursors;

	static const char *MASTER_GROUP;
};

} // namespace ZPUI

#endif // ZONE_PAINTER_EDITOR_UI_H

/* end of file */
