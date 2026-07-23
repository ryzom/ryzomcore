/**
 * \file script_api.h
 * \brief painterscript — MaxScript-like Lua scripting over the paint op layer (ui M23)
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * Include contract: NLMISC/std only in this header. The .cpp binds the embedded NLGUI
 * Lua state (CLuaManager) — no patch_eval.h, no SCENELIB, no NLGUI widget headers.
 *
 * Design (wiki drafts/zone_painter_ui_stories.md "Scripting — painterscript"): a
 * `painter` Lua namespace whose calls route through the SAME op layer as the keys, UI
 * and --paint-script (host.execOp formats/forwards canonical op lines; state setters
 * ride the SPaintUIBridge handlers when a viewer is live). Scripting is an additional
 * option, never the primary route. The recorder captures user actions as runnable
 * painter.* lines; script-originated calls are never recorded (re-entrance guard).
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

#ifndef ZONE_PAINTER_SCRIPT_API_H
#define ZONE_PAINTER_SCRIPT_API_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

namespace ZPUI {
struct SPaintUIBridge;
}

namespace ZPSCRIPT {

/** painter.zones() row. */
struct SZoneInfo
{
	uint Id;
	std::string Name;
	std::string File;      ///< source .max basename (empty when unknown)
	bool Editable;         ///< paint target (eligible, not frozen)
	bool Frozen;
	bool Dirty;

	SZoneInfo() : Id(0), Editable(false), Frozen(false), Dirty(false) { }
};

/**
 * Host callbacks the tool wires before scripts run. Function pointers may be NULL when
 * a capability is absent in the current mode (e.g. screenshot/pump headless, bridge
 * outside the viewer) — the binding returns (nil, "not available") for those.
 */
struct SScriptHost
{
	/** Execute ONE canonical op line through the same executor as --paint-script. */
	bool (*execOp)(const std::string &line, std::string &err);
	/** Fill the zones() rows for the current session. */
	void (*zonesInfo)(std::vector<SZoneInfo> &out);
	/** Read a zone export property (rotate|symmetry|passable|usebbox). */
	bool (*getZoneProp)(uint zoneId, const std::string &which, int &value, std::string &err);
	/** Write-back + whole-file save to a target path (never in place). */
	bool (*saveTo)(const std::string &target);
	/** Interactive save-all path (per-file overwrite + .bak); NULL headless. */
	bool (*saveAll)();
	/** Capture the current frame to a .tga; NULL headless. */
	bool (*screenshot)(const std::string &path, std::string &err);
	/** Render/pump gated at >100ms since the last actual pump; NULL headless (no-op). */
	void (*pumpUI)();
	/** True once the user requested cancel during a pumped script (ESC / Cancel). */
	bool (*cancelRequested)();
	/** Clear the host-side cancel latch — called at every script-run start (an ESC cancel
	 *	must not poison every later pumped script of the session). */
	void (*resetCancel)();
	/** Board-session working-set ops (ui M23c); NULL outside board sessions. */
	bool (*openZone)(const std::string &basename, std::string &err);
	bool (*closeZone)(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err);
	/** Live viewer bridge for state get/set; NULL headless. */
	ZPUI::SPaintUIBridge *bridge;

	SScriptHost()
		: execOp(NULL), zonesInfo(NULL), getZoneProp(NULL), saveTo(NULL), saveAll(NULL),
		  screenshot(NULL), pumpUI(NULL), cancelRequested(NULL), resetCancel(NULL),
		  openZone(NULL), closeZone(NULL), bridge(NULL)
	{
	}
};

/** Install the host (call whenever capabilities change: headless setup, viewer entry/exit). */
void setHost(SScriptHost *host);
SScriptHost *getHost();

/** Ensure the Lua state exists and the painter namespace is registered (idempotent). */
bool ensureLua();

/** Run a script file. Returns 0 on success, 1 on load/runtime error (message on stderr
 *	and in lastError()). */
int runFile(const std::string &path);
/** Run a script string (the Script window's Run). */
int runString(const std::string &code);
const std::string &lastError();

/** True while a painter script is executing (recorder re-entrance guard). */
bool isExecuting();

/** Cancel the executing script at its next pumpUI() (Script-window CANCEL / ESC-equivalent).
 *	No-op when no script is executing. */
void requestCancel();

// Recorder (M23b window feeds from this; API available from M23a)
void setRecording(bool on);
bool isRecording();
/** Append one runnable line if recording and not executing a script. */
void record(const std::string &luaCall);
const std::string &recorderText();
void clearRecorder();

/** print() redirection sink for the Script window (empty = stdout only). */
const std::string &outputText();
void clearOutput();

} // namespace ZPSCRIPT

#endif // ZONE_PAINTER_SCRIPT_API_H

/* end of file */
