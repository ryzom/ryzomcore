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
class CTileBank;
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
	/** Absolute tile-set select (palette cell / digit keys); shared path. */
	void (*selectTileSetAbs)(int idx);
	void (*toggleTileSize)();           // 128 <-> 256
	void (*brushSizeDelta)(int d);      // -1 / +1 (0..2 for tile/disp)
	void (*groupDelta)(int d);          // -1 / +1 (mod 13)
	void (*toggleLockBorders)();
	void (*undo)();
	void (*redo)();
	void (*fill)(int rot);              // 0..3
	void (*save)();                     // panel Save: modal (interactive) or direct (--save)
	/** Write-back + whole-file save to an arbitrary path (modal "Save copy", test hook). */
	bool (*saveTo)(const std::string &target);
	/** In-place overwrite: temp → optional one-time .bak → rename (modal "Overwrite"). */
	bool (*saveOverwrite)();
	/** Cycle landscape season textures (ui M6a); no-op when <2 seasons available. */
	void (*seasonNext)();
	// Color / displace (ui M7a) — same paths as keyboard Home/End/Ins/Del/S/Q/[ ]
	/** Color brush radius ± (×1.5 / ÷1.5, clamp 2..32); panel always; keys via brushSizeDelta in Color mode. */
	void (*colorRadiusDelta)(int d);
	void (*hardnessDelta)(int d);     // ± step (keys use ±51 on 0..255)
	void (*opacityDelta)(int d);
	void (*cycleBrushMask)();         // none → mask1 → … → none (S key)
	void (*toggleMaskMode)();         // Q key
	void (*displaceIndexDelta)(int d); // ±1 mod 16 ([ ] keys)
	/** Absolute displace index 0-15 (palette cell); shared path with [ ] / panel stepper. */
	void (*displaceIndexAbs)(int idx);
	/** Show/hide the Tiles palette window (ui M8; key TogglePalette / panel button). */
	void (*togglePalette)();
	/** Set brush color RGB 0-255 (color picker / panel); same field as --color. */
	void (*setBrushColor)(int r, int g, int b);

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
	/** When true, Save opens the overwrite/copy modal (startup interactive, no --save). */
	bool InteractiveSave;
	char EditableBasename[128]; // zone basename for default copy name
	char InputDir[512];         // directory of the opened .max (copy targets)
	/** Self-instance count (1 = off); panel shows INSTANCED xN when > 1 (ui M4b). */
	uint InstanceCount;
	/** M5c: modal "Update thumbnail" checkbox (default true for interactive save). */
	bool UpdateThumbnail;
	/** Current season label for the panel ("spring" / "auto" / ...). */
	char SeasonLabel[32];
	/** Number of discovered season variants (0 = no seasonal tiles; button frozen). */
	uint SeasonCount;
	/** Editable file count + dirty count (M6b multi-session; 1/0 for single). */
	uint EditableFileCount;
	uint DirtyFileCount;
	// Color brush snapshot (M7a)
	float ColorRadius;              // meters, 2..32
	uint ColorHardness;             // 0..255
	uint ColorOpacity;              // 0..255
	uint ColorR, ColorG, ColorB;    // swatch
	char BrushMaskLabel[64];        // basename or "none"
	bool BrushMaskMode;             // Q-key mask-mode toggle
	uint DisplaceIndex;             // 0..15

	SPaintUIBridge()
		: selectMode(NULL), selectTileSetDelta(NULL), selectTileSetAbs(NULL),
		  toggleTileSize(NULL),
		  brushSizeDelta(NULL), groupDelta(NULL), toggleLockBorders(NULL),
		  undo(NULL), redo(NULL), fill(NULL), save(NULL), saveTo(NULL), saveOverwrite(NULL),
		  seasonNext(NULL), colorRadiusDelta(NULL), hardnessDelta(NULL), opacityDelta(NULL),
		  cycleBrushMask(NULL), toggleMaskMode(NULL), displaceIndexDelta(NULL),
		  displaceIndexAbs(NULL), togglePalette(NULL), setBrushColor(NULL),
		  HaveCore(false), Mode(0), CurTileSet(0), TileSetCount(0), Mode256(false),
		  BrushSize(0), TileGroup(0), LockBorders(false), UndoDepth(0), CanSave(false),
		  InteractiveSave(false), InstanceCount(1), UpdateThumbnail(true), SeasonCount(0),
		  EditableFileCount(1), DirtyFileCount(0),
		  ColorRadius(8.f), ColorHardness(128), ColorOpacity(255),
		  ColorR(255), ColorG(255), ColorB(255), BrushMaskMode(false), DisplaceIndex(0)
	{
		TileSetName[0] = 0;
		EditableBasename[0] = 0;
		InputDir[0] = 0;
		SeasonLabel[0] = 0;
		BrushMaskLabel[0] = 'n';
		BrushMaskLabel[1] = 'o';
		BrushMaskLabel[2] = 'n';
		BrushMaskLabel[3] = 'e';
		BrushMaskLabel[4] = 0;
	}
};

/** Open the Save modal (Overwrite / Save copy / Cancel). Prefills copy name. */
void openSaveDialog();

/** Dev/test: force the Save modal open for one screenshot frame (env/guard callers). */
void forceShowSaveDialogForShot();

/**
 * Rebuild the Tiles palette from the loaded bank (ui M8 + M9a).
 * Fills the tileset grid (64px diffuse previews under thumbcache/tileset/) and the
 * Displace section (indices 0-15, noise-map previews under thumbcache/displace/).
 * tilesetForDisplace selects which set's _DisplacementBitmap maps sub-index → file
 * (current painter tileset; falls back to 0). bank may be NULL (clears both grids).
 */
void rebuildTilesetPalette(NL3D::CTileBank *bank, const std::string &bankPath,
                           const std::string &seasonKey, int tilesetForDisplace = 0);

/** Show/hide / toggle the Tiles palette window (ui M8). */
void setTilesetPaletteVisible(bool visible);
void toggleTilesetPalette();
bool isTilesetPaletteVisible();

/** Dev/test: scroll the palette body so the Displace section is in view (M9a shots). */
void scrollPaletteToDisplaceSection();

/** Show/hide the brush color picker window (ui M9b). */
void setColorPickerVisible(bool visible);
void toggleColorPicker();
bool isColorPickerVisible();
/** Dev/test: force the color picker open for one screenshot frame. */
void forceShowColorPickerForShot();

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
