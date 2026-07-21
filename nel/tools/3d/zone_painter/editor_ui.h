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

	/** Per-frame widget clock + coord check (call after EventServer.pump). */
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
