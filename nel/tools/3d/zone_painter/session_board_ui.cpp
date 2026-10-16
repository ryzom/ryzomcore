/**
 * \file session_board_ui.cpp
 * \brief NeL-GUI session board hub over the live viewer (continent working set / eco scratch)
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * The zone_browser widget doubles as a live board mid-session. Continent worlds show
 * the working set (open/close/save/toggle per cell); ecosystem worlds show a scratch
 * board (place/rotate/mirror/remove instances, open editable / RO context bricks,
 * drag move/copy). SStartupSession + NLGUI helpers come from startup_ui_internal.h.
 *
 * Include contract: NLMISC + NL3D UDriver + NLGUI + editor_ui + workspace_discovery.
 * No patch_eval / SCENELIB / context_display.
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

#include "startup_ui.h"
#include "startup_ui_internal.h"
#include "script_api.h"
#include "editor_ui.h"
#include "max_thumbnail.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <utility>

#include <nel/misc/algo.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include <nel/3d/driver_user.h>
#include <nel/3d/u_driver.h>

#include <nel/gui/action_handler.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_button.h>
#include <nel/gui/ctrl_scroll.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/group_container.h>
#include <nel/gui/group_editbox.h>
#include <nel/gui/group_list.h>
#include <nel/gui/group_menu.h>
#include <nel/gui/interface_group.h>
#include <nel/gui/interface_parser.h>
#include <nel/gui/view_bitmap.h>
#include <nel/gui/view_text.h>
#include <nel/gui/widget_manager.h>

using namespace NLMISC;
using namespace NL3D;
using namespace NLGUI;

namespace ZPUI {

// File-static state: only referenced from within this TU. Startup checks the mirror
// s_Sess.SessionMode instead so it doesn't need to link this symbol.
static bool s_SessionBoardVisible = false;

// Forward declarations for TU-local helpers used before their definition below.
static void populateScratchBoard();
static void openContextBrickPicker(int cx, int cy, int mode);

// ---------------------------------------------------------------------------------------------
// Session board hub

void setSessionBoardBridge(SSessionBoardBridge *bridge)
{
	s_SessionBridge = bridge;
}

SSessionBoardBridge *getSessionBoardBridge()
{
	return s_SessionBridge;
}

void openCellActionPopup(const std::string &basename)
{
	s_Sess.PendingActionBasename = basename;
	// Enable/disable buttons based on state
	const bool dirty = s_SessionBridge && s_SessionBridge->isDirty
		&& s_SessionBridge->isDirty(basename);
	const bool editable = s_SessionBridge && s_SessionBridge->isEditable
		&& s_SessionBridge->isEditable(basename);
	if (CViewText *t = findText("ui:zp:cell_action:hdr"))
		t->setHardText(basename + (dirty ? " *" : ""));
	if (CViewText *t = findText("ui:zp:cell_action:sub"))
	{
		if (editable && dirty)
			t->setHardText("open-editable, dirty");
		else if (editable)
			t->setHardText("open-editable");
		else
			t->setHardText("open read-only");
	}
	if (CViewTextMenu *line = findMenuLine("ui:zp:cell_action:save"))
		line->setGrayed(!dirty || !editable);
	// Save as… needs an editable file (clean files can still be copied elsewhere)
	if (CViewTextMenu *line = findMenuLine("ui:zp:cell_action:saveas"))
		line->setGrayed(!editable);
	if (CViewTextMenu *line = findMenuLine("ui:zp:cell_action:toggle"))
		line->setText(editable ? "Make read-only" : "Make editable");
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:cell_action");
}

// close_confirm purpose: false = close, true = demote-to-RO (the modal is reused for
// both). A dedicated flag, not a StatusMsg tag: a string latch would survive board close
// (ESC/O bypass the handlers) and a later close-confirm on another cell would silently
// run toggleEditable instead of closeZone.
static bool s_CloseConfirmToggle = false;

// Pending hint-name for the empty-cell popup's one-click hint rows. Dedicated slot for
// the same reason as s_CloseConfirmToggle: piggy-backing on s_Sess.StatusMsg (which any
// setStatus() writer touches) risked clobber-between-open-and-click.
static std::string s_PendingHintName;

static void openCloseConfirmModal(const std::string &basename, const std::string &purpose)
{
	s_CloseConfirmToggle = false;
	s_Sess.PendingActionBasename = basename;
	if (CViewText *t = findText("ui:zp:close_confirm:content:title"))
		t->setHardText(purpose.empty() ? "Close dirty zone?" : purpose);
	if (CViewText *t = findText("ui:zp:close_confirm:content:status"))
		t->setHardText(basename + " has unsaved paint changes");
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:close_confirm");
}

void forceShowCloseConfirmForShot(const std::string &basename)
{
	std::string b = basename;
	if (b.empty() && !s_Sess.Zones.empty())
		b = s_Sess.Zones[0].Basename;
	if (b.empty())
		b = "zone";
	openCloseConfirmModal(b, "Close dirty zone?");
}

void forceShowCellActionForShot(const std::string &basename)
{
	std::string b = basename;
	if (b.empty() && !s_Sess.Zones.empty())
		b = s_Sess.Zones[0].Basename;
	if (b.empty())
		b = "zone";
	openCellActionPopup(b);
}

void openInstanceActionPopup(const std::string &basename)
{
	s_Sess.PendingActionBasename = basename;
	char sk = 0;
	int cx = 0, cy = 0;
	uint rot = 0;
	bool mir = false;
	parseScratchBasename(basename, sk, cx, cy);
	int ox = cx, oy = cy;
	if (s_SessionBridge && s_SessionBridge->scratchGetInstanceOrigin)
		s_SessionBridge->scratchGetInstanceOrigin(cx, cy, ox, oy, rot, mir);
	else if (s_SessionBridge && s_SessionBridge->scratchGetInstance)
		s_SessionBridge->scratchGetInstance(cx, cy, rot, mir);
	if (CViewText *t = findText("ui:zp:instance_action:hdr"))
		t->setHardText(NLMISC::toString("Instance %d,%d", ox, oy));
	if (CViewText *t = findText("ui:zp:instance_action:sub"))
	{
		std::string st = NLMISC::toString("rot %u°", rot * 90);
		if (mir) st += " · mirror";
		t->setHardText(st);
	}
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:instance_action");
}

void forceShowInstanceActionForShot(const std::string &basename)
{
	std::string b = basename;
	if (b.empty())
		b = "I:1,0";
	openInstanceActionPopup(b);
}

void forceShowEmptyCellForShot(const std::string &basename)
{
	std::string b = basename;
	if (b.empty())
		b = "E:1,0";
	openEmptyCellPopup(b);
}

void refreshSessionBoardStates()
{
	if (!s_SessionBoardVisible || !s_Sess.SessionMode)
		return;
	if (!s_SessionBridge || !s_SessionBridge->getCellState)
		return;
	// Ecosystem scratch: instance set may change (E↔I); rebuild the grid
	if (s_SessionBridge->World && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
	{
		populateScratchBoard();
		refreshBoardSelectionUI();
		return;
	}
	for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
	{
		int r = 0, c = 0;
		if (!ZPWS::parseContinentZoneName(s_Sess.Zones[i].Basename, r, c))
			continue;
		char idbuf[96];
		snprintf(idbuf, sizeof(idbuf), "ui:zp:zone_browser:content:board_host:board:gc%d_%d", r, c);
		if (CInterfaceGroup *cell = findGroup(idbuf))
		{
			ESessionCellState st = CellClosed;
			s_SessionBridge->getCellState(s_Sess.Zones[i].Basename, st);
			applySessionCellState(cell, s_Sess.Zones[i].Basename, st);
		}
	}
	refreshBoardSelectionUI();
}

/**
 * Ecosystem scratch board: fine-cell grid covering every open file + instances +
 * contexts + margin.
 *
 * Convention: each board cell is one --cellsize unit. Every open file (the first-opened
 * included) claims only its MASKED cells at its board cell (L-shapes/holes are
 * first-class; unmasked cells are empty). Each instance claims its rotFlip-transformed
 * mask cells. Placement legality refuses overlapping masked cells (interlocking L-shapes
 * allowed).
 *
 * Basenames: F:ox,oy[:name] | I:ox,oy | E:cx,cy | L:cx,cy | C:cx,cy:name.
 * Bridge owns the place list. No H: tokens; the first-opened file is an F: open-file cell.
 */
/** Coded basename when (cx,cy) is OCCUPIED (open file/instance/context); else empty. */
static std::string scratchOccupiedCellName(int cx, int cy, int fw, int fh)
{
	(void)fw;
	(void)fh;
	// Open-file block (editable or demoted RO) placed on the board; includes the
	// first-opened file, which occupies through the same per-file mask fields.
	int fox = 0, foy = 0;
	std::string fname;
	bool fedit = true;
	if (s_SessionBridge && s_SessionBridge->scratchGetEditableAt
	    && s_SessionBridge->scratchGetEditableAt(cx, cy, fox, foy, fname, fedit))
	{
		if (cx == fox && cy == foy)
			return NLMISC::toString("F:%d,%d:%s", fox, foy, fname.c_str());
		return NLMISC::toString("F:%d,%d@%d,%d:%s", fox, foy, cx, cy, fname.c_str());
	}
	if (s_SessionBridge && s_SessionBridge->scratchGetInstanceOrigin)
	{
		int ox = 0, oy = 0;
		uint rot = 0;
		bool mir = false;
		if (s_SessionBridge->scratchGetInstanceOrigin(cx, cy, ox, oy, rot, mir))
		{
			// Origin cell gets "I:ox,oy" (label); other block cells get "@cx,cy" (tint only)
			if (cx == ox && cy == oy)
				return NLMISC::toString("I:%d,%d", ox, oy);
			return NLMISC::toString("I:%d,%d@%d,%d", ox, oy, cx, cy);
		}
	}
	else if (s_SessionBridge && s_SessionBridge->scratchGetInstance)
	{
		uint rot = 0;
		bool mir = false;
		if (s_SessionBridge->scratchGetInstance(cx, cy, rot, mir))
			return NLMISC::toString("I:%d,%d", cx, cy);
	}
	// Context brick at this cell?
	std::string cname;
	if (s_SessionBridge && s_SessionBridge->scratchGetContext
	    && s_SessionBridge->scratchGetContext(cx, cy, cname))
		return NLMISC::toString("C:%d,%d:%s", cx, cy, cname.c_str());
	return std::string();
}

static void populateScratchBoard()
{
	const int kCell = 52;
	const int fw = (s_SessionBridge && s_SessionBridge->FootprintCellsW > 0)
	                   ? s_SessionBridge->FootprintCellsW : 1;
	const int fh = (s_SessionBridge && s_SessionBridge->FootprintCellsH > 0)
	                   ? s_SessionBridge->FootprintCellsH : 1;

	// Bounds: every occupied cell (open files incl. the first-opened, instances,
	// contexts) found by the probe below + margin; a default window when nothing is
	// occupied yet.
	int minC = 0, maxC = fw - 1, minR = 0, maxR = fh - 1;

	// Probe a generous window for instances/contexts (bridge lookup is O(places))
	const int kProbe = 24;
	for (int cy = -kProbe; cy <= kProbe + fh; ++cy)
	for (int cx = -kProbe; cx <= kProbe + fw; ++cx)
	{
		bool occ = false;
		if (s_SessionBridge && s_SessionBridge->scratchGetInstanceOrigin)
		{
			int ox = 0, oy = 0;
			uint rot = 0;
			bool mir = false;
			if (s_SessionBridge->scratchGetInstanceOrigin(cx, cy, ox, oy, rot, mir))
				occ = true;
		}
		else if (s_SessionBridge && s_SessionBridge->scratchGetInstance)
		{
			uint rot = 0;
			bool mir = false;
			if (s_SessionBridge->scratchGetInstance(cx, cy, rot, mir))
				occ = true;
		}
		if (!occ && s_SessionBridge && s_SessionBridge->scratchGetContext)
		{
			std::string cname;
			if (s_SessionBridge->scratchGetContext(cx, cy, cname))
				occ = true;
		}
		if (!occ && s_SessionBridge && s_SessionBridge->scratchGetEditableAt)
		{
			int ox = 0, oy = 0;
			std::string fname;
			bool fedit = true;
			if (s_SessionBridge->scratchGetEditableAt(cx, cy, ox, oy, fname, fedit))
				occ = true;
		}
		if (!occ) continue;
		if (cx < minC) minC = cx;
		if (cx > maxC) maxC = cx;
		if (cy < minR) minR = cy;
		if (cy > maxR) maxR = cy;
	}
	// Margin of empty wells around occupied (edge-adjacent fringe of the silhouette)
	minC -= 1; maxC += 1; minR -= 1; maxR += 1;
	// Keep a usable minimum for 1×1 bricks (roughly 7×7)
	if (maxC - minC < 6) { const int mid = (minC + maxC) / 2; minC = mid - 3; maxC = mid + 3; }
	if (maxR - minR < 6) { const int mid = (minR + maxR) / 2; minR = mid - 3; maxR = mid + 3; }

	// Build synthetic zone list for click indices
	s_Sess.Zones.clear();
	std::map<std::pair<int, int>, int> used; // (cy,cx) as row,col
	for (int cy = minR; cy <= maxR; ++cy)
	for (int cx = minC; cx <= maxC; ++cx)
	{
		ZPWS::SZoneEntry z;
		z.Basename = scratchOccupiedCellName(cx, cy, fw, fh);
		used[std::make_pair(cy, cx)] = (int)s_Sess.Zones.size();
		s_Sess.Zones.push_back(z);
	}

	// Unlock model: placing a block unlocks its horizontal/vertical neighbors.
	// Empty cells edge-adjacent to any occupied cell, or named by a saved-neighbor hint,
	// are open wells ("E:"); every other empty cell renders LOCKED ("L:", dim, no menu,
	// still a drag target). Diagonal contact alone does not unlock (ligo border rule).
	// Occupancy snapshot FIRST: pass 2 rewrites empties to E:/L:, and testing rewritten
	// neighbors would cascade the unlock across the whole board.
	std::set<std::pair<int, int> > occ;
	for (int cy = minR; cy <= maxR; ++cy)
	for (int cx = minC; cx <= maxC; ++cx)
	{
		std::map<std::pair<int, int>, int>::iterator it = used.find(std::make_pair(cy, cx));
		if (it != used.end() && !s_Sess.Zones[(size_t)it->second].Basename.empty())
			occ.insert(std::make_pair(cy, cx));
	}
	for (int cy = minR; cy <= maxR; ++cy)
	for (int cx = minC; cx <= maxC; ++cx)
	{
		std::map<std::pair<int, int>, int>::iterator it = used.find(std::make_pair(cy, cx));
		if (it == used.end()) continue;
		ZPWS::SZoneEntry &z = s_Sess.Zones[(size_t)it->second];
		if (!z.Basename.empty()) continue; // occupied
		bool unlocked = false;
		static const int kAdj[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int a = 0; a < 4 && !unlocked; ++a)
		{
			const int nx = cx + kAdj[a][0], ny = cy + kAdj[a][1];
			if (occ.count(std::make_pair(ny, nx)))
				unlocked = true;
			else if ((nx < minC || nx > maxC || ny < minR || ny > maxR)
			         && !scratchOccupiedCellName(nx, ny, fw, fh).empty())
				unlocked = true; // occupied just outside the window bounds
		}
		if (!unlocked && s_SessionBridge && s_SessionBridge->scratchGetHintAt)
		{
			std::string hintName;
			unlocked = s_SessionBridge->scratchGetHintAt(cx, cy, hintName);
		}
		z.Basename = NLMISC::toString(unlocked ? "E:%d,%d" : "L:%d,%d", cx, cy);
	}

	const int nRows = maxR - minR + 1;
	const int nCols = maxC - minC + 1;
	const int boardW = nCols * kCell;
	const int boardH = nRows * kCell;

	setZoneBrowserMode(true);
	clearBoard();
	CInterfaceGroup *board = findGroup("ui:zp:zone_browser:content:board_host:board");
	if (!board) return;
	board->setW(boardW);
	board->setH(boardH);
	board->setMaxW(730);
	board->setMaxH(330);

	// Scroll so board cell (0,0), where the first-opened file starts, is in view.
	// Board cells: x=(c-minC)*kCell, y=-((maxR-r)*kCell). Top-left of content is (0,0)
	// local; positive ofs shifts content to reveal lower/right cells (palette idiom).
	{
		const sint32 kViewW = 730, kViewH = 330;
		const sint32 originX = (0 - minC) * kCell;
		const sint32 originY = -((maxR - 0) * kCell);
		sint32 ofsX = originX - 4;
		sint32 ofsY = -originY - 4;
		const sint32 maxOfsX = boardW > kViewW ? (boardW - kViewW) : 0;
		const sint32 maxOfsY = boardH > kViewH ? (boardH - kViewH) : 0;
		if (ofsX < 0) ofsX = 0;
		if (ofsY < 0) ofsY = 0;
		if (ofsX > maxOfsX) ofsX = maxOfsX;
		if (ofsY > maxOfsY) ofsY = maxOfsY;
		board->setOfsX(ofsX);
		board->setOfsY(ofsY);
	}

	for (std::map<std::pair<int, int>, int>::const_iterator it = used.begin(); it != used.end(); ++it)
	{
		const int r = it->first.first;  // cy
		const int c = it->first.second; // cx
		const int zi = it->second;
		const ZPWS::SZoneEntry &z = s_Sess.Zones[zi];
		const sint32 x = (c - minC) * kCell;
		const sint32 y = -((maxR - r) * kCell);
		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[48], idxbuf[32];
		snprintf(idbuf, sizeof(idbuf), "gc%d_%d", r, c);
		snprintf(idxbuf, sizeof(idxbuf), "%d", zi);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		p.push_back(std::make_pair(std::string("title"), z.Basename));
		p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
		p.push_back(std::make_pair(std::string("thumb"), std::string("w_box_blank.tga")));
		CInterfaceGroup *cell = spawnUnder(board, "zp_board_cell", p, x, y, kCell, kCell);
		if (!cell) continue;
		if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(cell->getView("thumb")))
			thumb->setActive(false);
		if (CInterfaceGroup *fr = cell->getGroup("thumb_frame"))
			fr->setActive(false);
		ESessionCellState st = CellScratchEmpty;
		if (s_SessionBridge && s_SessionBridge->getCellState)
			s_SessionBridge->getCellState(z.Basename, st);
		if (CCtrlButton *btn = dynamic_cast<CCtrlButton *>(cell->getCtrl("btn")))
		{
			btn->setScale(true);
			// EMPTY/LOCKED wells render inset so the backdrop shows as grid lines;
			// occupied blocks stay full-size contiguous.
			const sint32 inset = (st == CellScratchEmpty || st == CellScratchLocked)
			                         ? kCell - 6 : kCell;
			btn->setW(inset);
			btn->setH(inset);
		}
		applySessionCellState(cell, z.Basename, st);
	}

	if (CCtrlScroll *sv = dynamic_cast<CCtrlScroll *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:board_host:sv")))
		sv->setTarget(board);
	if (CCtrlScroll *sh = dynamic_cast<CCtrlScroll *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:board_host:sh")))
		sh->setTarget(board);
	board->invalidateCoords();
	if (CInterfaceGroup *host = findGroup("ui:zp:zone_browser:content:board_host"))
		host->invalidateCoords();
}

// ---------------------------------------------------------------------------------------------
// Board drag move/copy (drag = move, Ctrl/Shift-drag = copy; the file-manager idiom).
// The pointer listener (editor_ui) feeds raw left-button transitions; cells resolve via the
// NLGUI under-pointer stack, so no board geometry math and button clicks (down+up on the
// SAME cell) stay untouched.

static bool s_BoardDragArmed = false;
static int s_BoardDragFromX = 0, s_BoardDragFromY = 0;

/** Board cell under the pointer (id ...:board_host:board:gc<r>_<c>...) → (cx,cy). */
static bool boardCellUnderPointer(int &cx, int &cy)
{
	const std::vector<CCtrlBase *> &ctrls = CWidgetManager::getInstance()->getCtrlsUnderPointer();
	for (size_t i = 0; i < ctrls.size(); ++i)
	{
		if (!ctrls[i]) continue;
		const std::string &id = ctrls[i]->getId();
		std::string::size_type pos = id.find(":board_host:board:gc");
		if (pos == std::string::npos) continue;
		std::string rest = id.substr(pos + strlen(":board_host:board:gc"));
		std::string::size_type us = rest.find('_');
		if (us == std::string::npos) continue;
		std::string::size_type end = rest.find(':', us);
		std::string rowS = rest.substr(0, us);
		std::string colS = end == std::string::npos ? rest.substr(us + 1)
		                                            : rest.substr(us + 1, end - us - 1);
		int r = 0, c = 0;
		if (!NLMISC::fromString(rowS, r) || !NLMISC::fromString(colS, c)) continue;
		// spawn ids are gc<row=cy>_<col=cx>
		cy = r;
		cx = c;
		return true;
	}
	return false;
}

void sessionBoardDragBegin()
{
	// Disarm FIRST, executing or not: the caller invokes unconditionally so a pumped
	// script can never leave a stale arm behind.
	s_BoardDragArmed = false;
	if (ZPSCRIPT::isExecuting())
		return;
	// No arming while a modal/menu is up: CGroupMenu forces ExitClickOut, and
	// getCtrlsUnder still reports the board cells BENEATH the menu, so the click that
	// dismisses a menu (or a press-slide-release across its lines) would arm a drag
	// between whatever cells sit under the pointer (phantom moves/copies).
	if (CWidgetManager::getInstance()->hasModal())
		return;
	if (!s_SessionBoardVisible || !s_SessionBridge || !s_SessionBridge->World
	    || s_SessionBridge->World->Kind != ZPWS::Ecosystem
	    || !s_SessionBridge->scratchDragDrop)
		return;
	if (boardCellUnderPointer(s_BoardDragFromX, s_BoardDragFromY))
		s_BoardDragArmed = true;
}

/** Focus loss mid-drag (alt-tab) never delivers the up event; a later stray up would
 *  pair with the stale arm and fire a phantom drag from the pre-focus-loss cell. */
void sessionBoardDragCancel()
{
	s_BoardDragArmed = false;
}

void sessionBoardDragEnd(bool copyModifier)
{
	if (!s_BoardDragArmed)
		return;
	s_BoardDragArmed = false;
	if (ZPSCRIPT::isExecuting())
		return;
	// A modal opened between arm and release (or the drop lands on one): ignore.
	if (CWidgetManager::getInstance()->hasModal())
		return;
	if (!s_SessionBoardVisible || !s_SessionBridge || !s_SessionBridge->scratchDragDrop)
		return;
	int tx = 0, ty = 0;
	if (!boardCellUnderPointer(tx, ty))
		return;
	if (tx == s_BoardDragFromX && ty == s_BoardDragFromY)
		return; // plain click; the cell button handles it
	std::string err;
	if (!s_SessionBridge->scratchDragDrop(s_BoardDragFromX, s_BoardDragFromY, tx, ty,
	                                      copyModifier, err))
	{
		fprintf(stderr, "board drag (%d,%d)->(%d,%d)%s: %s\n",
		        s_BoardDragFromX, s_BoardDragFromY, tx, ty,
		        copyModifier ? " copy" : "", err.c_str());
		if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
			t->setHardText(err);
		return;
	}
	populateScratchBoard();
	refreshSessionBoardStates();
}

void setSessionBoardVisible(bool visible)
{
	if (!s_SessionBridge || !s_SessionBridge->World)
	{
		if (visible)
			fprintf(stderr, "session board: no bridge (open a continent or ecosystem session first)\n");
		s_SessionBoardVisible = false;
		return;
	}
	const bool isCont = s_SessionBridge->World->Kind == ZPWS::Continent;
	const bool isEco = s_SessionBridge->World->Kind == ZPWS::Ecosystem;
	if (!isCont && !isEco)
	{
		s_SessionBoardVisible = false;
		return;
	}

	s_SessionBoardVisible = visible;
	s_Sess.SessionMode = visible;
	if (visible)
	{
		s_Sess.Active = true;
		s_Sess.Worlds = NULL;
		s_Sess.SelectedWorld = 0;
		setLayoutSelectorVisible(false);
		if (isCont)
		{
			ZPWS::listZones(*s_SessionBridge->World, s_Sess.Zones);
			if (CViewText *t = findText("ui:zp:zone_browser:content:world_title"))
				t->setHardText(s_SessionBridge->World->WorldName + "  (session board)");
			populateContinentGrid(*s_SessionBridge->World);
			// AFTER populateContinentGrid: it overwrites world_sub with the grid extent,
			// so a session hint set before it never displayed (dead store).
			if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
				t->setHardText("working set - O / BACK TO PAINTING returns");
		}
		else
		{
			// The board is the WORLD's scratch layout, not the home brick's; with
			// multiple editables a home-named title misread as "editing that one zone".
			if (CViewText *t = findText("ui:zp:zone_browser:content:world_title"))
				t->setHardText(s_SessionBridge->World->WorldName + "  (scratch board)");
			if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
				t->setHardText("place / rotate / mirror instances - O / BACK TO PAINTING");
			populateScratchBoard();
		}
		if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser"))
			el->setActive(true);
		refreshSessionBoardStates();
		refreshBoardSelectionUI();
		printf("session board: open (%u cells, %s)\n",
		       (uint)s_Sess.Zones.size(), isEco ? "scratch" : "continent");
	}
	else
	{
		if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser"))
			el->setActive(false);
		CWidgetManager::getInstance()->disableModalWindow();
		s_Sess.SessionMode = false;
		s_Sess.Active = false;
		s_Sess.PendingActionBasename.clear();
		s_CloseConfirmToggle = false; // ESC/O close bypasses the modal handlers
		s_PendingHintName.clear();
		printf("session board: closed (back to painting)\n");
	}
}

void toggleSessionBoard()
{
	// Refuse under any active modal - matches the drag-arm/end guard (line 537/562).
	// Closing the board unconditionally disables the modal window, which would silently
	// dismiss unrelated dialogs (save_dialog with a typed filename, close_confirm, the
	// context picker) and drop user-entered state with no warning. Board's own menu
	// popups (cell/context/empty/instance) are also modals; refusing here lets them
	// auto-dismiss via ExitClickOut on their next click instead.
	if (CWidgetManager::getInstance()->hasModal())
		return;
	setSessionBoardVisible(!s_SessionBoardVisible);
}

bool isSessionBoardVisible()
{
	return s_SessionBoardVisible;
}

// Cell-action popup handlers
/** Per-file board ops change eco occupancy (blocks appear/disappear); repopulate. */
void refreshBoardAfterSessionOp()
{
	if (s_SessionBridge && s_SessionBridge->World
	    && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
		populateScratchBoard();
	refreshSessionBoardStates();
}

class CAHZpCellClose : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge || !s_SessionBridge->closeZone)
			return;
		const bool dirty = s_SessionBridge->isDirty && s_SessionBridge->isDirty(base);
		const bool editable = s_SessionBridge->isEditable && s_SessionBridge->isEditable(base);
		if (dirty && editable)
		{
			openCloseConfirmModal(base, "Close dirty zone?");
			return;
		}
		std::string err;
		if (!s_SessionBridge->closeZone(base, false, false, err))
			fprintf(stderr, "session board close '%s': %s\n", base.c_str(), err.c_str());
		refreshBoardAfterSessionOp();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellClose, "zp_cell_close");

class CAHZpCellSave : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge || !s_SessionBridge->saveZone)
			return;
		std::string err;
		if (!s_SessionBridge->saveZone(base, err))
			fprintf(stderr, "session board save '%s': %s\n", base.c_str(), err.c_str());
		else
			printf("session board: saved '%s'\n", base.c_str());
		refreshBoardAfterSessionOp();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellSave, "zp_cell_save");

// Per-file save-as: open the save dialog bound to this cell's file (name,
// overwrite/copy, thumbnail checkbox = the custom save options).
class CAHZpCellSaveAs : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty())
			return;
		openSaveDialogForFile(base);
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellSaveAs, "zp_cell_save_as");

class CAHZpCellToggle : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge || !s_SessionBridge->toggleEditable)
			return;
		const bool dirty = s_SessionBridge->isDirty && s_SessionBridge->isDirty(base);
		const bool editable = s_SessionBridge->isEditable && s_SessionBridge->isEditable(base);
		if (editable && dirty)
		{
			// Confirm: save first or discard before demoting to RO
			s_Sess.PendingActionBasename = base;
			if (CViewText *t = findText("ui:zp:close_confirm:content:title"))
				t->setHardText("Make read-only?");
			if (CViewText *t = findText("ui:zp:close_confirm:content:status"))
				t->setHardText(base + " is dirty - save before making read-only?");
			// Reuse close_confirm; btn labels still say Save first / Close without saving
			// "Close without saving" path demotes without save via forceDiscard
			CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:close_confirm");
			s_CloseConfirmToggle = true;
			return;
		}
		std::string err;
		if (!s_SessionBridge->toggleEditable(base, false, false, err))
			fprintf(stderr, "session board toggle '%s': %s\n", base.c_str(), err.c_str());
		refreshBoardAfterSessionOp();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellToggle, "zp_cell_toggle");

// Close-confirm: Save first / Close without saving / Cancel
class CAHZpCloseConfirmSave : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge)
			return;
		const bool isToggle = s_CloseConfirmToggle;
		s_CloseConfirmToggle = false;
		std::string err;
		if (isToggle)
		{
			if (s_SessionBridge->toggleEditable
			    && !s_SessionBridge->toggleEditable(base, /*saveFirst=*/true, false, err))
				fprintf(stderr, "session board toggle(save) '%s': %s\n", base.c_str(), err.c_str());
		}
		else if (s_SessionBridge->closeZone
		         && !s_SessionBridge->closeZone(base, /*saveFirst=*/true, false, err))
		{
			fprintf(stderr, "session board close(save) '%s': %s\n", base.c_str(), err.c_str());
		}
		refreshBoardAfterSessionOp();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCloseConfirmSave, "zp_close_confirm_save");

class CAHZpCloseConfirmDiscard : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge)
			return;
		const bool isToggle = s_CloseConfirmToggle;
		s_CloseConfirmToggle = false;
		std::string err;
		if (isToggle)
		{
			if (s_SessionBridge->toggleEditable
			    && !s_SessionBridge->toggleEditable(base, false, /*forceDiscard=*/true, err))
				fprintf(stderr, "session board toggle(discard) '%s': %s\n", base.c_str(), err.c_str());
		}
		else if (s_SessionBridge->closeZone
		         && !s_SessionBridge->closeZone(base, false, /*forceDiscard=*/true, err))
		{
			fprintf(stderr, "session board close(discard) '%s': %s\n", base.c_str(), err.c_str());
		}
		refreshBoardAfterSessionOp();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCloseConfirmDiscard, "zp_close_confirm_discard");

class CAHZpCloseConfirmCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		s_CloseConfirmToggle = false;
		s_Sess.PendingActionBasename.clear();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCloseConfirmCancel, "zp_close_confirm_cancel");

// Ecosystem scratch instance popup; any cell in a block resolves via bridge
static bool scratchParsePending(int &cx, int &cy)
{
	char sk = 0;
	if (!parseScratchBasename(s_Sess.PendingActionBasename, sk, cx, cy) || sk != 'I')
		return false;
	// Prefer origin so rotate/mirror operate on the block consistently
	if (s_SessionBridge && s_SessionBridge->scratchGetInstanceOrigin)
	{
		int ox = cx, oy = cy;
		uint rot = 0;
		bool mir = false;
		if (s_SessionBridge->scratchGetInstanceOrigin(cx, cy, ox, oy, rot, mir))
		{
			cx = ox;
			cy = oy;
		}
	}
	return true;
}

class CAHZpInstRotCW : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		int cx = 0, cy = 0;
		if (!scratchParsePending(cx, cy) || !s_SessionBridge || !s_SessionBridge->scratchRotate)
			return;
		std::string err;
		if (!s_SessionBridge->scratchRotate(cx, cy, +1, err))
			fprintf(stderr, "scratch rotate CW (%d,%d): %s\n", cx, cy, err.c_str());
		refreshBoardAfterSessionOp(); // occupancy changed; repopulate, not just re-tint
	}
};
REGISTER_ACTION_HANDLER(CAHZpInstRotCW, "zp_inst_rot_cw");

class CAHZpInstRotCCW : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		int cx = 0, cy = 0;
		if (!scratchParsePending(cx, cy) || !s_SessionBridge || !s_SessionBridge->scratchRotate)
			return;
		std::string err;
		if (!s_SessionBridge->scratchRotate(cx, cy, -1, err))
			fprintf(stderr, "scratch rotate CCW (%d,%d): %s\n", cx, cy, err.c_str());
		refreshBoardAfterSessionOp(); // occupancy changed; repopulate, not just re-tint
	}
};
REGISTER_ACTION_HANDLER(CAHZpInstRotCCW, "zp_inst_rot_ccw");

class CAHZpInstMirror : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		int cx = 0, cy = 0;
		if (!scratchParsePending(cx, cy) || !s_SessionBridge || !s_SessionBridge->scratchMirror)
			return;
		std::string err;
		if (!s_SessionBridge->scratchMirror(cx, cy, err))
			fprintf(stderr, "scratch mirror (%d,%d): %s\n", cx, cy, err.c_str());
		refreshBoardAfterSessionOp(); // occupancy changed; repopulate, not just re-tint
	}
};
REGISTER_ACTION_HANDLER(CAHZpInstMirror, "zp_inst_mirror");

class CAHZpInstRemove : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		int cx = 0, cy = 0;
		if (!scratchParsePending(cx, cy) || !s_SessionBridge || !s_SessionBridge->scratchRemove)
			return;
		std::string err;
		if (!s_SessionBridge->scratchRemove(cx, cy, err))
			fprintf(stderr, "scratch remove (%d,%d): %s\n", cx, cy, err.c_str());
		refreshBoardAfterSessionOp(); // occupancy changed; repopulate, not just re-tint
	}
};
REGISTER_ACTION_HANDLER(CAHZpInstRemove, "zp_inst_remove");

// ---------------------------------------------------------------------------------------------
// Empty-cell popup + context place/remove + brick picker

// Picker mode: 0 = RO context, 1 = editable open, 2 = instance of an OPEN brick
static int s_ContextPickerMode = 0;
static int s_ContextPickerCx = 0;
static int s_ContextPickerCy = 0;
static std::vector<std::string> s_ContextPickerNames;

void openEmptyCellPopup(const std::string &basename)
{
	s_Sess.PendingActionBasename = basename;
	if (CViewText *t = findText("ui:zp:empty_cell_action:hdr"))
		t->setHardText("Empty cell");
	// Saved-neighbor hint for this cell: one-click open offers at the top
	std::string hintName;
	bool haveHint = false;
	{
		char sk = 0;
		int cx = 0, cy = 0;
		const bool parsed = parseScratchBasename(basename, sk, cx, cy);
		if (CViewText *t = findText("ui:zp:empty_cell_action:sub"))
			t->setHardText(parsed ? NLMISC::toString("Cell %d,%d", cx, cy) : basename);
		if (parsed && sk == 'E' && s_SessionBridge && s_SessionBridge->scratchGetHintAt)
			haveHint = s_SessionBridge->scratchGetHintAt(cx, cy, hintName);
	}
	// The two hint rows are plain menu lines (CGroupList backing the menu already
	// reflows around inactive children; no manual Y-repositioning needed).
	if (CViewTextMenu *line = findMenuLine("ui:zp:empty_cell_action:hint_ro"))
	{
		line->setActive(haveHint);
		if (haveHint) line->setText("Open '" + stripLigoFamilyPrefix(hintName) + "' (read-only)");
	}
	if (CViewTextMenu *line = findMenuLine("ui:zp:empty_cell_action:hint_ed"))
	{
		line->setActive(haveHint);
		if (haveHint) line->setText("Open '" + stripLigoFamilyPrefix(hintName) + "' editable");
	}
	if (CInterfaceGroup *menu = findGroup("ui:zp:empty_cell_action"))
		menu->invalidateCoords();
	s_PendingHintName = haveHint ? hintName : std::string();
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:empty_cell_action");
}

void openContextActionPopup(const std::string &basename)
{
	s_Sess.PendingActionBasename = basename;
	std::string lab = basename;
	if (basename.size() > 2 && basename[0] == 'C')
	{
		std::string::size_type last = basename.rfind(':');
		if (last != std::string::npos)
			lab = basename.substr(last + 1);
	}
	if (CViewText *t = findText("ui:zp:context_action:hdr"))
		t->setHardText("Context (RO)");
	if (CViewText *t = findText("ui:zp:context_action:sub"))
		t->setHardText(lab);
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:context_action");
}

void forceShowContextActionForShot(const std::string &basename)
{
	std::string b = basename;
	if (b.empty())
		b = "C:1,0:demo-context";
	openContextActionPopup(b);
}

void forceShowContextPickerForShot(int mode)
{
	// Dev/test: open the context-brick picker for one screenshot frame.
	openContextBrickPicker(1, 0, mode);
}

static void openContextBrickPicker(int cx, int cy, int mode)
{
	s_ContextPickerMode = mode;
	s_ContextPickerCx = cx;
	s_ContextPickerCy = cy;
	s_ContextPickerNames.clear();
	// Screen B list idiom: rows in a scroll_text's text_list (scrollable; not a fixed height box).
	static const char *kPickerList = "ui:zp:context_picker:content:list_scroll:text_list";
	clearList(kPickerList);
	if (!s_SessionBridge || !s_SessionBridge->World)
		return;
	std::vector<ZPWS::SZoneEntry> zones;
	if (s_ContextPickerMode == 2)
	{
		// Instance sources: every OPEN file by cell probe. The first-opened file
		// occupies board cells like the rest, so the probe finds it uniformly.
		if (s_SessionBridge->scratchGetEditableAt)
		{
			std::set<std::string> seen;
			const int kProbe = 24;
			for (int py = -kProbe; py <= kProbe; ++py)
			for (int px = -kProbe; px <= kProbe; ++px)
			{
				int fox = 0, foy = 0;
				std::string fname;
				bool fedit = true;
				if (s_SessionBridge->scratchGetEditableAt(px, py, fox, foy, fname, fedit)
				    && seen.insert(fname).second)
				{
					ZPWS::SZoneEntry fe;
					fe.Basename = fname;
					zones.push_back(fe);
				}
			}
		}
	}
	else
	{
		ZPWS::listZones(*s_SessionBridge->World, zones);
		// Session-hinted bricks sort to the top (stable within both groups)
		if (s_SessionBridge->scratchHintNames)
		{
			std::vector<std::string> hinted;
			s_SessionBridge->scratchHintNames(hinted);
			if (!hinted.empty())
			{
				std::vector<ZPWS::SZoneEntry> pri, rest;
				for (size_t i = 0; i < zones.size(); ++i)
				{
					bool isHinted = false;
					for (size_t h = 0; h < hinted.size() && !isHinted; ++h)
						isHinted = NLMISC::toLowerAscii(hinted[h])
						           == NLMISC::toLowerAscii(zones[i].Basename);
					(isHinted ? pri : rest).push_back(zones[i]);
				}
				zones = pri;
				zones.insert(zones.end(), rest.begin(), rest.end());
			}
		}
	}
	for (size_t i = 0; i < zones.size(); ++i)
	{
		// Context mode skips ALREADY-OPEN files (they cannot load again as RO context);
		// open-editable mode lists them (picking an open zone places a shared-paint
		// INSTANCE at the cell), and instance mode lists the open sources anyway.
		if (s_ContextPickerMode == 0 && s_SessionBridge->isOpen
		    && s_SessionBridge->isOpen(zones[i].Basename))
			continue;
		s_ContextPickerNames.push_back(zones[i].Basename);
		std::vector<std::pair<std::string, std::string> > p;
		p.push_back(std::make_pair(std::string("id"), NLMISC::toString("row_%u", (uint)i)));
		p.push_back(std::make_pair(std::string("title"), zones[i].Basename));
		p.push_back(std::make_pair(std::string("idx"), NLMISC::toString("%u", (uint)(s_ContextPickerNames.size() - 1))));
		p.push_back(std::make_pair(std::string("thumb"), std::string("")));
		CInterfaceGroup *row = spawnRow("zp_zone_row", kPickerList, p);
		if (row)
		{
			// Rebind click to context picker select. Use CCtrlBaseButton, not CCtrlTextButton:
			// zp_zone_row's btn is a plain <ctrl type="button"> (CCtrlButton). A CCtrlTextButton
			// cast returns NULL and the row keeps zp_select_zone/#idx, which would dispatch a
			// board cell action instead of picking the brick.
			if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(row->getCtrl("btn")))
			{
				btn->setActionOnLeftClick("zp_context_pick");
				btn->setParamsOnLeftClick(NLMISC::toString("%u", (uint)(s_ContextPickerNames.size() - 1)));
			}
			row->setH(36);
		}
	}
	if (CGroupList *gl = findList(kPickerList))
		gl->invalidateCoords();
	if (CViewText *t = findText("ui:zp:context_picker:content:title"))
		t->setHardText(NLMISC::toString(s_ContextPickerMode == 2 ? "Place instance @ %d,%d"
		                                : (s_ContextPickerMode == 1 ? "Open editable @ %d,%d"
		                                                            : "Place context @ %d,%d"),
		                                cx, cy));
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:context_picker");
}

class CAHZpEmptyPlaceInstance : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		char sk = 0;
		int cx = 0, cy = 0;
		if (!parseScratchBasename(s_Sess.PendingActionBasename, sk, cx, cy) || sk != 'E')
			return;
		if (!s_SessionBridge || !s_SessionBridge->scratchPlace) return;
		// With more than one open brick, pick the instance source first
		if (s_SessionBridge->scratchPlaceInstanceOf && s_SessionBridge->scratchOpenFileCount
		    && s_SessionBridge->scratchOpenFileCount() > 1)
		{
			openContextBrickPicker(cx, cy, /*mode=*/2);
			return;
		}
		std::string err;
		if (!s_SessionBridge->scratchPlace(cx, cy, err))
			fprintf(stderr, "scratch place (%d,%d): %s\n", cx, cy, err.c_str());
		// Rebuild board cells (instance set changed)
		if (s_SessionBridge->World && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
			populateScratchBoard();
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpEmptyPlaceInstance, "zp_empty_place_instance");

class CAHZpEmptyPlaceContext : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		char sk = 0;
		int cx = 0, cy = 0;
		if (!parseScratchBasename(s_Sess.PendingActionBasename, sk, cx, cy) || sk != 'E')
			return;
		openContextBrickPicker(cx, cy, /*mode=*/0);
	}
};
REGISTER_ACTION_HANDLER(CAHZpEmptyPlaceContext, "zp_empty_place_context");

// Empty cell → open a world brick as EDITABLE at this cell (multi-file eco editing)
class CAHZpEmptyOpenEditable : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		char sk = 0;
		int cx = 0, cy = 0;
		if (!parseScratchBasename(s_Sess.PendingActionBasename, sk, cx, cy) || sk != 'E')
			return;
		openContextBrickPicker(cx, cy, /*mode=*/1);
	}
};
REGISTER_ACTION_HANDLER(CAHZpEmptyOpenEditable, "zp_empty_open_editable");

// One-click open of the cell's saved-neighbor hint (read-only / editable)
static void zpEmptyOpenHint(bool editable)
{
	CWidgetManager::getInstance()->disableModalWindow();
	char sk = 0;
	int cx = 0, cy = 0;
	if (!parseScratchBasename(s_Sess.PendingActionBasename, sk, cx, cy) || sk != 'E')
		return;
	if (s_PendingHintName.empty())
		return;
	const std::string name = s_PendingHintName;
	s_PendingHintName.clear();
	if (!s_SessionBridge) return;
	std::string err;
	bool ok = false;
	if (editable && s_SessionBridge->scratchOpenEditable)
		ok = s_SessionBridge->scratchOpenEditable(cx, cy, name, err);
	else if (!editable && s_SessionBridge->scratchPlaceContext)
		ok = s_SessionBridge->scratchPlaceContext(cx, cy, name, err);
	if (!ok)
		fprintf(stderr, "hint open (%d,%d:%s): %s\n", cx, cy, name.c_str(), err.c_str());
	if (s_SessionBridge->World && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
		populateScratchBoard();
	refreshSessionBoardStates();
}

class CAHZpEmptyHintRo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		zpEmptyOpenHint(false);
	}
};
REGISTER_ACTION_HANDLER(CAHZpEmptyHintRo, "zp_empty_hint_ro");

class CAHZpEmptyHintEd : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		zpEmptyOpenHint(true);
	}
};
REGISTER_ACTION_HANDLER(CAHZpEmptyHintEd, "zp_empty_hint_ed");

class CAHZpContextRemove : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string &base = s_Sess.PendingActionBasename;
		int cx = 0, cy = 0;
		if (base.size() > 2 && base[0] == 'C' && base[1] == ':')
		{
			std::string rest = base.substr(2);
			std::string::size_type comma = rest.find(',');
			std::string::size_type colon = rest.find(':', comma == std::string::npos ? 0 : comma);
			if (comma != std::string::npos && colon != std::string::npos)
			{
				NLMISC::fromString(rest.substr(0, comma), cx);
				NLMISC::fromString(rest.substr(comma + 1, colon - comma - 1), cy);
			}
		}
		if (!s_SessionBridge || !s_SessionBridge->scratchRemoveContext) return;
		std::string err;
		if (!s_SessionBridge->scratchRemoveContext(cx, cy, err))
			fprintf(stderr, "scratch remove-context (%d,%d): %s\n", cx, cy, err.c_str());
		if (s_SessionBridge->World && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
			populateScratchBoard();
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpContextRemove, "zp_context_remove");

// Context brick → editable open file at the same cell
class CAHZpContextMakeEditable : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string &base = s_Sess.PendingActionBasename;
		int cx = 0, cy = 0;
		if (base.size() > 2 && base[0] == 'C' && base[1] == ':')
		{
			std::string rest = base.substr(2);
			std::string::size_type comma = rest.find(',');
			std::string::size_type colon = rest.find(':', comma == std::string::npos ? 0 : comma);
			if (comma != std::string::npos && colon != std::string::npos)
			{
				NLMISC::fromString(rest.substr(0, comma), cx);
				NLMISC::fromString(rest.substr(comma + 1, colon - comma - 1), cy);
			}
		}
		if (!s_SessionBridge || !s_SessionBridge->scratchContextToEditable) return;
		std::string err;
		if (!s_SessionBridge->scratchContextToEditable(cx, cy, err))
			fprintf(stderr, "scratch context-to-editable (%d,%d): %s\n", cx, cy, err.c_str());
		if (s_SessionBridge->World && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
			populateScratchBoard();
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpContextMakeEditable, "zp_context_make_editable");

// Context brick rotate/mirror (same block-center rules as instances)
static void zpContextTransformAction(int rotDelta, bool mirror)
{
	CWidgetManager::getInstance()->disableModalWindow();
	const std::string &base = s_Sess.PendingActionBasename;
	int cx = 0, cy = 0;
	if (base.size() > 2 && base[0] == 'C' && base[1] == ':')
	{
		std::string rest = base.substr(2);
		std::string::size_type comma = rest.find(',');
		std::string::size_type colon = rest.find(':', comma == std::string::npos ? 0 : comma);
		if (comma != std::string::npos && colon != std::string::npos)
		{
			NLMISC::fromString(rest.substr(0, comma), cx);
			NLMISC::fromString(rest.substr(comma + 1, colon - comma - 1), cy);
		}
	}
	if (!s_SessionBridge) return;
	std::string err;
	bool ok = false;
	if (mirror && s_SessionBridge->scratchMirrorContext)
		ok = s_SessionBridge->scratchMirrorContext(cx, cy, err);
	else if (!mirror && s_SessionBridge->scratchRotateContext)
		ok = s_SessionBridge->scratchRotateContext(cx, cy, rotDelta, err);
	if (!ok)
		fprintf(stderr, "context transform (%d,%d): %s\n", cx, cy, err.c_str());
	if (s_SessionBridge->World && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
		populateScratchBoard();
	refreshSessionBoardStates();
}

class CAHZpContextRotCW : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		zpContextTransformAction(+1, false);
	}
};
REGISTER_ACTION_HANDLER(CAHZpContextRotCW, "zp_context_rot_cw");

class CAHZpContextRotCCW : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		zpContextTransformAction(-1, false);
	}
};
REGISTER_ACTION_HANDLER(CAHZpContextRotCCW, "zp_context_rot_ccw");

class CAHZpContextMirror : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		zpContextTransformAction(0, true);
	}
};
REGISTER_ACTION_HANDLER(CAHZpContextMirror, "zp_context_mirror");

class CAHZpContextPick : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		uint idx = 0;
		if (!NLMISC::fromString(params, idx) || idx >= s_ContextPickerNames.size())
			return;
		std::string err;
		if (s_ContextPickerMode == 2)
		{
			if (!s_SessionBridge || !s_SessionBridge->scratchPlaceInstanceOf) return;
			if (!s_SessionBridge->scratchPlaceInstanceOf(s_ContextPickerCx, s_ContextPickerCy,
			                                             s_ContextPickerNames[idx], err))
				fprintf(stderr, "scratch place-instance (%d,%d:%s): %s\n",
				        s_ContextPickerCx, s_ContextPickerCy,
				        s_ContextPickerNames[idx].c_str(), err.c_str());
		}
		else if (s_ContextPickerMode == 1)
		{
			if (!s_SessionBridge || !s_SessionBridge->scratchOpenEditable) return;
			if (!s_SessionBridge->scratchOpenEditable(s_ContextPickerCx, s_ContextPickerCy,
			                                          s_ContextPickerNames[idx], err))
				fprintf(stderr, "scratch open-editable (%d,%d:%s): %s\n",
				        s_ContextPickerCx, s_ContextPickerCy,
				        s_ContextPickerNames[idx].c_str(), err.c_str());
		}
		else
		{
			if (!s_SessionBridge || !s_SessionBridge->scratchPlaceContext) return;
			if (!s_SessionBridge->scratchPlaceContext(s_ContextPickerCx, s_ContextPickerCy,
			                                         s_ContextPickerNames[idx], err))
				fprintf(stderr, "scratch place-context (%d,%d:%s): %s\n",
				        s_ContextPickerCx, s_ContextPickerCy,
				        s_ContextPickerNames[idx].c_str(), err.c_str());
		}
		if (s_SessionBridge->World && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
			populateScratchBoard();
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpContextPick, "zp_context_pick");

class CAHZpContextPickerCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER(CAHZpContextPickerCancel, "zp_context_picker_cancel");

} // namespace ZPUI

/* end of file */
