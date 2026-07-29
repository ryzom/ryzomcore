/**
 * \file script_api.cpp
 * \brief painterscript: Lua binding over the paint op layer
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Grok 4.5
 *
 * Own TU: NLGUI Lua (CLuaManager/CLuaState) + NLMISC only. Must NOT include
 * patch_eval.h / context_display.h / SCENELIB headers (main.cpp include contract).
 *
 * Binding style: wrapped functions (CLuaState&) registered as __zp_* globals, then a
 * bootstrap chunk builds the `painter` table (Ryzom client Lua conventions: camelCase,
 * namespaced table; MaxScript-familiar verbs). Every op returns either true or
 * (nil, "error string"): standard Lua error-return idiom, no exceptions across the
 * C boundary.
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

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>

#include <cstring>

#include <nel/gui/lua_helper.h>
#include <nel/gui/lua_manager.h>

#include <cstdio>
#include <string>
#include <vector>

#include "editor_ui.h" // SPaintUIBridge (state get/set when the viewer is live)
#include <nel/misc/vector.h>

#include "script_api.h"

// Patch-edit selection, declared here rather than by including zp_state.h: that header pulls
// in patch_eval.h, which needs pipeline_max scene types this TU deliberately does not carry.
// Index-based readback keeps the container type out of the interface entirely.
void zpPatchVertSelect(uint zoneId, uint vertIdx, int op);
void zpPatchVertClear();
uint zpPatchVertSelCount();
uint zpApplyPatchMove(const NLMISC::CVector &worldDelta, std::string &msg);
bool zpPatchVertSelAt(uint index, uint &zoneOut, uint &vertOut);
bool zpPatchVertWorld(uint zoneId, uint vertIdx, float outPos[3]);
void zpPatchEdgeSelect(uint zoneId, uint vertA, uint vertB, int op);
void zpPatchFaceSelect(uint zoneId, uint patchIdx, int op);
uint zpPatchEdgeSelCount();
uint zpPatchFaceSelCount();
bool zpPatchEdgeSelAt(uint index, uint &zoneOut, uint &aOut, uint &bOut);
bool zpPatchFaceSelAt(uint index, uint &zoneOut, uint &patchOut);
bool zpPatchClickAt(float x, float y, uint buttons);
void zpPatchTangentSelect(uint zoneId, uint vecIdx, int op);
uint zpPatchTangentSelCount();
bool zpPatchTangentSelAt(uint index, uint &zoneOut, uint &vecOut);
bool zpPatchTangentWorld(uint zoneId, uint vecIdx, float outPos[3]);
void zpSetPivotMode(int mode);
bool zpSetUserPivotToSelection();
bool zpTransformPivotXYZ(float outPos[3]);
uint zpApplyPatchRotate(int axis, float degrees, std::string &msg);
uint zpApplyPatchRotateAxis(float ax, float ay, float az, float degrees, std::string &msg);
uint zpApplyPatchScale(float sx, float sy, float sz, std::string &msg);
void zpSetXformKind(int kind);
uint zpUnbindPatchSelection();
uint zpBindPatchSelection();
bool zpBindPatchVertexToEdge(uint zoneId, uint vertIdx, uint patchIdx, uint edgeSlot,
                             std::string &msg);
uint zpSetEdgeNoSmooth(bool noSmooth);
int zpEdgeNoSmoothQuery(uint zoneId, uint vertA, uint vertB);
bool zpVertexBindQuery(uint zoneId, uint vertIdx, int &bindedOut, int &typeOut,
                       int &patchOut, int &edgeOut, int &primOut);
bool zpPatchEdgeCornerPair(uint zoneId, uint patchIdx, uint edgeSlot, uint &aOut, uint &bOut);
bool zpPatchVecIndex(uint zoneId, uint patchIdx, uint ringSlot, uint &vecOut);
uint zpDeletePatchSelection();
uint zpTurnPatchSelection(bool ccw);
uint zpSubdividePatchSelection();
uint zpWeldPatchSelection(float threshold);
uint zpAddQuadPatchSelection();
bool zpZonePatchCount(uint zoneId, uint &countOut);
bool zpZoneVertCount(uint zoneId, uint &countOut);
bool zpTileQuery(uint zoneId, uint patchIdx, uint u, uint v, int &tileOut, int &rotOut,
                 int &numOut);

using namespace NLMISC;
using namespace NLGUI;

namespace ZPSCRIPT {

static SScriptHost *s_Host = NULL;
static bool s_Registered = false;
static bool s_Executing = false;
static bool s_Recording = false;
static bool s_CancelReq = false; // Script-window CANCEL button; ESC rides the host
static std::string s_Recorder;
static std::string s_Output;
static std::string s_LastError;
// Mutation counters: the Script window's panes sync on these instead of text length;
// a same-length content change (CLEAR + same-length line in one frame) left stale text.
static uint s_RecorderGen = 0;
static uint s_OutputGen = 0;

void setHost(SScriptHost *host) { s_Host = host; }
SScriptHost *getHost() { return s_Host; }

/** Quote a string as a Lua literal (same rules as main.cpp's recorder-side luaQuote). */
static std::string luaQuoteLocal(const std::string &s)
{
	std::string r = "\"";
	for (size_t i = 0; i < s.size(); ++i)
	{
		char c = s[i];
		if (c == '"' || c == '\\') { r += '\\'; r += c; }
		else if (c == '\n') r += "\\n";
		else r += c;
	}
	r += "\"";
	return r;
}
bool isExecuting() { return s_Executing; }
void requestCancel() { if (s_Executing) s_CancelReq = true; }
void setRecording(bool on)
{
	const bool starting = on && !s_Recording;
	s_Recording = on;
	if (!starting)
		return;
	// REC preamble (replay fidelity): a recording started mid-session replays against a
	// fresh process (default RNG stream and default painter state), so tile-variant
	// picks and stroke results diverged even when every action was captured. Reseed BOTH
	// the live session and the recording with the same value, then snapshot the paint-
	// relevant state as abs painter.* lines (states recorded on *change* only miss the
	// starting values).
	if (s_Host && s_Host->execOp)
	{
		uint seed = (uint)(NLMISC::CTime::getLocalTime() & 0x7fffffff);
		if (!seed) seed = 1;
		std::string err;
		if (s_Host->execOp(NLMISC::toString("seed %u", seed), err))
		{
			s_Recorder += NLMISC::toString("painter.seed(%u)\n", seed);
			++s_RecorderGen;
		}
	}
	if (s_Host && s_Host->refreshBridge)
		s_Host->refreshBridge();
	ZPUI::SPaintUIBridge *b = s_Host ? s_Host->bridge : NULL;
	if (b && b->HaveCore)
	{
		std::string pre;
		pre += NLMISC::toString("painter.setMode(%d)\n", b->Mode);
		// After setMode: entering patch mode resets the level to Object, so replaying the
		// level first would lose it.
		pre += NLMISC::toString("painter.setSubObject(%d)\n", b->SubObj);
		// ... and the sub-object selection with it. setSubObject clears the set, so these
		// must come after it; a recorded move would otherwise run against an empty selection.
		//
		// Snapshot the LEVEL'S OWN set. At edge or patch level the vertex set is only a
		// projection of that set, so replaying vertices there would restore nothing the level
		// can address. SubEdge = 2, SubPatch = 3 (TSubObject).
		if (b->SubObj == 2)
		{
			const uint nSel = zpPatchEdgeSelCount();
			if (nSel) pre += "painter.clearPatchVertexSelection()\n";
			for (uint i = 0; i < nSel; ++i)
			{
				uint zid = 0, va = 0, vb = 0;
				if (zpPatchEdgeSelAt(i, zid, va, vb))
					pre += NLMISC::toString("painter.selectPatchEdge(%u, %u, %u, 1)\n", zid, va, vb);
			}
		}
		else if (b->SubObj == 3)
		{
			const uint nSel = zpPatchFaceSelCount();
			if (nSel) pre += "painter.clearPatchVertexSelection()\n";
			for (uint i = 0; i < nSel; ++i)
			{
				uint zid = 0, pid = 0;
				if (zpPatchFaceSelAt(i, zid, pid))
					pre += NLMISC::toString("painter.selectPatchFace(%u, %u, 1)\n", zid, pid);
			}
		}
		else
		{
			const uint nSel = zpPatchVertSelCount();
			if (nSel) pre += "painter.clearPatchVertexSelection()\n";
			for (uint i = 0; i < nSel; ++i)
			{
				uint zid = 0, vid = 0;
				if (zpPatchVertSelAt(i, zid, vid))
					pre += NLMISC::toString("painter.selectPatchVertex(%u, %u, 1)\n", zid, vid);
			}
			// Handles sit ON the corner selection (they are only visible - and only the
			// transform target - while their corner is selected), so they come after it. A
			// recording started with a handle selected would otherwise replay the handle move
			// against nothing.
			const uint nTan = zpPatchTangentSelCount();
			for (uint i = 0; i < nTan; ++i)
			{
				uint zid = 0, vec = 0;
				if (zpPatchTangentSelAt(i, zid, vec))
					pre += NLMISC::toString("painter.selectPatchTangent(%u, %u, 1)\n", zid, vec);
			}
		}
		pre += NLMISC::toString("painter.setTileSet(%d)\n", b->CurTileSet);
		pre += NLMISC::toString("painter.set256(%s)\n", b->Mode256 ? "true" : "false");
		pre += NLMISC::toString("painter.setBrushSize(%u)\n", b->BrushSize);
		pre += NLMISC::toString("painter.setTileGroup(%u)\n", b->TileGroup);
		pre += NLMISC::toString("painter.setLockBorders(%s)\n", b->LockBorders ? "true" : "false");
		pre += NLMISC::toString("painter.setHardness(%u)\n", b->ColorHardness);
		pre += NLMISC::toString("painter.setOpacity(%u)\n", b->ColorOpacity);
		pre += NLMISC::toString("painter.setRadius(%.9g)\n", b->ColorRadius);
		pre += NLMISC::toString("painter.setBrushColor(%u, %u, %u)\n", b->ColorR, b->ColorG, b->ColorB);
		pre += NLMISC::toString("painter.setDisplaceIndex(%u)\n", b->DisplaceIndex);
		if (strcmp(b->BrushMaskLabel, "none") != 0 && b->BrushMaskLabel[0])
			pre += NLMISC::toString("painter.setBrushMask(%s)\n",
				luaQuoteLocal(b->BrushMaskLabel).c_str());
		pre += NLMISC::toString("painter.setMaskMode(%s)\n", b->BrushMaskMode ? "true" : "false");
		s_Recorder += pre;
		++s_RecorderGen;
	}
}
bool isRecording() { return s_Recording; }
const std::string &recorderText() { return s_Recorder; }
void clearRecorder() { s_Recorder.clear(); ++s_RecorderGen; }
const std::string &outputText() { return s_Output; }
void clearOutput() { s_Output.clear(); ++s_OutputGen; }
const std::string &lastError() { return s_LastError; }
uint recorderGeneration() { return s_RecorderGen; }
uint outputGeneration() { return s_OutputGen; }

void record(const std::string &luaCall)
{
	if (!s_Recording || s_Executing)
		return;
	s_Recorder += luaCall;
	s_Recorder += "\n";
	++s_RecorderGen;
}

// ---------------------------------------------------------------------------------------------
// Stack helpers (defensive: scripts feed these, never trust arity/types)

static bool argNumber(CLuaState &ls, int idx, double &out)
{
	if (ls.getTop() < idx || !ls.isNumber(idx)) return false;
	out = (double)ls.toNumber(idx);
	return true;
}

static bool argString(CLuaState &ls, int idx, std::string &out)
{
	if (ls.getTop() < idx || !ls.isString(idx)) return false;
	ls.toString(idx, out);
	return true;
}

/** String arg destined for an op line: op lines are space-tokenized, so embedded
 *	whitespace would silently smuggle extra tokens (a "rrggbb blend" color string)
 *	or truncate ("my mask.tga" loses its tail); refuse instead. */
static bool argToken(CLuaState &ls, int idx, std::string &out)
{
	if (!argString(ls, idx, out)) return false;
	if (out.empty() || out.find_first_of(" \t\r\n") != std::string::npos) return false;
	return true;
}

static bool argBoolOpt(CLuaState &ls, int idx, bool def)
{
	if (ls.getTop() < idx) return def;
	if (ls.isBoolean(idx)) return ls.toBoolean(idx);
	if (ls.isNumber(idx)) return ls.toNumber(idx) != 0;
	return def;
}

/** Standard error return: nil, "message". */
static int retErr(CLuaState &ls, const std::string &err)
{
	ls.pushNil();
	ls.push(err);
	return 2;
}

static int retOk(CLuaState &ls)
{
	ls.push(true);
	return 1;
}

/** Execute a canonical op line through the host; standard (true) / (nil, err) return. */
static int execOpRet(CLuaState &ls, const std::string &line)
{
	if (!s_Host || !s_Host->execOp)
		return retErr(ls, "painter: no active session");
	std::string err;
	if (!s_Host->execOp(line, err))
		return retErr(ls, err.empty() ? std::string("op failed") : err);
	return retOk(ls);
}

/** Read N numeric args starting at 1 into out[]; on failure returns the usage error. */
static bool argNumbers(CLuaState &ls, int count, double *out, const char *usage, std::string &err)
{
	for (int i = 0; i < count; ++i)
	{
		if (!argNumber(ls, i + 1, out[i]))
		{
			err = std::string("usage: ") + usage;
			return false;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Ops (thin veneer over the --paint-script vocabulary; one shared executor)

static int lPaintTile(CLuaState &ls) // painter.paintTile(zone,patch,u,v,set[,rot[,big]])
{
	double a[5]; std::string err;
	if (!argNumbers(ls, 5, a, "paintTile(zone,patch,u,v,set[,rot[,big]])", err)) return retErr(ls, err);
	double rot = 0; argNumber(ls, 6, rot);
	bool big = argBoolOpt(ls, 7, false);
	// set is signed: -1 is the clear sentinel (paint_core opTile); %u would turn it into
	// UINT_MAX and fail the bank range check instead of clearing.
	return execOpRet(ls, toString("%s %u %u %u %u %d %u", big ? "tile256" : "tile",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], (int)a[4], (uint)rot));
}

static int lRotateTile(CLuaState &ls) // painter.rotateTile(zone,patch,u,v,rot)
{
	double a[5]; std::string err;
	if (!argNumbers(ls, 5, a, "rotateTile(zone,patch,u,v,rot)", err)) return retErr(ls, err);
	return execOpRet(ls, toString("rot %u %u %u %u %u",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], (uint)a[4]));
}

static int lClearTile(CLuaState &ls) // painter.clearTile(zone,patch,u,v[,big])
{
	double a[4]; std::string err;
	if (!argNumbers(ls, 4, a, "clearTile(zone,patch,u,v[,big])", err)) return retErr(ls, err);
	bool big = argBoolOpt(ls, 5, false);
	return execOpRet(ls, toString("%s %u %u %u %u", big ? "clear256" : "clear",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3]));
}

static int lFillTile(CLuaState &ls) // painter.fillTile(zone,patch,set[,rot[,big]])
{
	double a[3]; std::string err;
	if (!argNumbers(ls, 3, a, "fillTile(zone,patch,set[,rot[,big]])", err)) return retErr(ls, err);
	double rot = 0; argNumber(ls, 4, rot);
	bool big = argBoolOpt(ls, 5, false);
	// set is signed: -1 clears the patch (no alternate Lua fill-clear path).
	return execOpRet(ls, toString("%s %u %u %d %u", big ? "fill256" : "fill",
		(uint)a[0], (uint)a[1], (int)a[2], (uint)rot));
}

static int lPaintColor(CLuaState &ls) // painter.paintColor(zone,patch,u,v,"rrggbb"[,blend 0-256])
{
	double a[4]; std::string err, rgb;
	if (!argNumbers(ls, 4, a, "paintColor(zone,patch,u,v,\"rrggbb\"[,blend])", err)) return retErr(ls, err);
	if (!argToken(ls, 5, rgb)) return retErr(ls, "usage: paintColor(zone,patch,u,v,\"rrggbb\"[,blend])");
	double blend;
	if (argNumber(ls, 6, blend))
		return execOpRet(ls, toString("color %u %u %u %u %s %u",
			(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], rgb.c_str(), (uint)blend));
	return execOpRet(ls, toString("color %u %u %u %u %s",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], rgb.c_str()));
}

static int lFillColor(CLuaState &ls) // painter.fillColor(zone,patch,"rrggbb"[,blend 0-256])
{
	double a[2]; std::string err, rgb;
	if (!argNumbers(ls, 2, a, "fillColor(zone,patch,\"rrggbb\"[,blend])", err)) return retErr(ls, err);
	if (!argToken(ls, 3, rgb)) return retErr(ls, "usage: fillColor(zone,patch,\"rrggbb\"[,blend])");
	double blend;
	if (argNumber(ls, 4, blend))
		return execOpRet(ls, toString("cfill %u %u %s %u",
			(uint)a[0], (uint)a[1], rgb.c_str(), (uint)blend));
	return execOpRet(ls, toString("cfill %u %u %s", (uint)a[0], (uint)a[1], rgb.c_str()));
}

static int lColorBrush(CLuaState &ls) // painter.colorBrush(zone,x,y,radius,"rrggbb",hard,opac[,zWorld[,cont]])
{
	// %.9g: float32 round-trips exactly; the recorder's old %.3f quantized the hit
	// position, and distance-based blend weights made replays only approximate.
	double a[4]; std::string err, rgb;
	if (!argNumbers(ls, 4, a, "colorBrush(zone,x,y,radius,\"rrggbb\",hardness,opacity[,zWorld[,cont]])", err))
		return retErr(ls, err);
	if (!argToken(ls, 5, rgb)) return retErr(ls, "colorBrush: rgb string expected (no spaces)");
	double hard, opac;
	if (!argNumber(ls, 6, hard) || !argNumber(ls, 7, opac))
		return retErr(ls, "colorBrush: hardness/opacity expected");
	double zw;
	if (argNumber(ls, 8, zw))
	{
		// cont (bool, 9th): stroke-aware form; commit comes from painter.endStroke()
		if (ls.getTop() >= 9)
		{
			bool cont = argBoolOpt(ls, 9, false);
			return execOpRet(ls, toString("cbrush %u %.9g %.9g %.9g %s %u %u %.9g %u",
				(uint)a[0], a[1], a[2], a[3], rgb.c_str(), (uint)hard, (uint)opac, zw,
				cont ? 1u : 0u));
		}
		return execOpRet(ls, toString("cbrush %u %.9g %.9g %.9g %s %u %u %.9g",
			(uint)a[0], a[1], a[2], a[3], rgb.c_str(), (uint)hard, (uint)opac, zw));
	}
	return execOpRet(ls, toString("cbrush %u %.9g %.9g %.9g %s %u %u",
		(uint)a[0], a[1], a[2], a[3], rgb.c_str(), (uint)hard, (uint)opac));
}

static int lTileStroke(CLuaState &ls) // painter.tileStroke(zone,patch,u,v,set[,big[,cont]]); mouse-path stroke (brush size applies)
{
	double a[5]; std::string err;
	if (!argNumbers(ls, 5, a, "tileStroke(zone,patch,u,v,set[,big[,cont]])", err)) return retErr(ls, err);
	bool big = argBoolOpt(ls, 6, false);
	// cont (bool, 7th): stroke-aware form; first line of a drag passes false, moves pass
	// true, and the commit comes from painter.endStroke(); without it each line is a
	// self-contained stroke (legacy scripts keep their behavior).
	// set is signed (-1 = clear sentinel).
	if (ls.getTop() >= 7)
	{
		bool cont = argBoolOpt(ls, 7, false);
		return execOpRet(ls, toString("tstroke %u %u %u %u %d %u %u",
			(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], (int)a[4], big ? 1u : 0u,
			cont ? 1u : 0u));
	}
	return execOpRet(ls, toString("tstroke %u %u %u %u %d %u",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], (int)a[4], big ? 1u : 0u));
}

static int lEndStroke(CLuaState &ls) // painter.endStroke(); commit the open stroke (mouse-up)
{
	return execOpRet(ls, "endstroke");
}

static int lPaintDisplace(CLuaState &ls) // painter.paintDisplace(zone,patch,u,v,index)
{
	double a[5]; std::string err;
	if (!argNumbers(ls, 5, a, "paintDisplace(zone,patch,u,v,index)", err)) return retErr(ls, err);
	return execOpRet(ls, toString("displace %u %u %u %u %u",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], (uint)a[4]));
}

static int lFillDisplace(CLuaState &ls) // painter.fillDisplace(zone,patch,index)
{
	double a[3]; std::string err;
	if (!argNumbers(ls, 3, a, "fillDisplace(zone,patch,index)", err)) return retErr(ls, err);
	return execOpRet(ls, toString("dfill %u %u %u", (uint)a[0], (uint)a[1], (uint)a[2]));
}

static int lSetBrushSize(CLuaState &ls) // painter.setBrushSize(0..2)
{
	double n; if (!argNumber(ls, 1, n)) return retErr(ls, "usage: setBrushSize(0..2)");
	return execOpRet(ls, toString("brush %u", (uint)n));
}

static int lSetTileGroup(CLuaState &ls) // painter.setTileGroup(0..12)
{
	double n; if (!argNumber(ls, 1, n)) return retErr(ls, "usage: setTileGroup(0..12)");
	return execOpRet(ls, toString("group %u", (uint)n));
}

static int lSetBrushMask(CLuaState &ls) // painter.setBrushMask("file.tga"|"none")
{
	std::string s; if (!argToken(ls, 1, s)) return retErr(ls, "usage: setBrushMask(\"file.tga\"|\"none\"); no whitespace in the name");
	return execOpRet(ls, "mask " + s);
}

static int lSetLockBorders(CLuaState &ls) // painter.setLockBorders(bool); core state, headless-capable
{
	bool on = argBoolOpt(ls, 1, true);
	return execOpRet(ls, on ? "lockborders 1" : "lockborders 0");
}

static int lSetMaskMode(CLuaState &ls) // painter.setMaskMode(bool); core state, headless-capable
{
	bool on = argBoolOpt(ls, 1, true);
	return execOpRet(ls, on ? "maskmode 1" : "maskmode 0");
}

static int lUndo(CLuaState &ls) { return execOpRet(ls, "undo"); }
static int lRedo(CLuaState &ls) { return execOpRet(ls, "redo"); }

static int lSeed(CLuaState &ls) // painter.seed(n)
{
	double n; if (!argNumber(ls, 1, n)) return retErr(ls, "usage: seed(n)");
	return execOpRet(ls, toString("seed %u", (uint)n));
}

static int lCheckSeams(CLuaState &ls) // painter.checkSeams(zone) -> true | nil,err
{
	double z; if (!argNumber(ls, 1, z)) return retErr(ls, "usage: checkSeams(zone)");
	return execOpRet(ls, toString("checkseams %u", (uint)z));
}

// ---------------------------------------------------------------------------------------------
// Zone properties

static int lSetZoneProp(CLuaState &ls) // painter.setZoneProp(zone,"rotate|symmetry|passable|usebbox",value)
{
	double z; std::string which; double v;
	if (!argNumber(ls, 1, z) || !argString(ls, 2, which))
		return retErr(ls, "usage: setZoneProp(zone,\"rotate|symmetry|passable|usebbox\",value)");
	if (!argNumber(ls, 3, v))
	{
		if (ls.getTop() >= 3 && ls.isBoolean(3)) v = ls.toBoolean(3) ? 1 : 0;
		else return retErr(ls, "setZoneProp: numeric or boolean value expected");
	}
	return execOpRet(ls, toString("prop %u %s %d", (uint)z, which.c_str(), (int)v));
}

static int lGetZoneProp(CLuaState &ls) // painter.getZoneProp(zone,which) -> number | nil,err
{
	double z; std::string which;
	if (!argNumber(ls, 1, z) || !argString(ls, 2, which))
		return retErr(ls, "usage: getZoneProp(zone,\"rotate|symmetry|passable|usebbox\")");
	if (!s_Host || !s_Host->getZoneProp) return retErr(ls, "painter: no active session");
	int value = 0; std::string err;
	if (!s_Host->getZoneProp((uint)z, which, value, err)) return retErr(ls, err);
	ls.push((double)value);
	return 1;
}

// ---------------------------------------------------------------------------------------------
// Session

static int lZones(CLuaState &ls) // painter.zones() -> { {id=,name=,file=,editable=,frozen=,dirty=}, ... }
{
	if (!s_Host || !s_Host->zonesInfo) return retErr(ls, "painter: no active session");
	std::vector<SZoneInfo> rows;
	s_Host->zonesInfo(rows);
	ls.newTable();
	for (size_t i = 0; i < rows.size(); ++i)
	{
		ls.newTable();
		ls.push(std::string("id")); ls.push((double)rows[i].Id); ls.setTable(-3);
		ls.push(std::string("name")); ls.push(rows[i].Name); ls.setTable(-3);
		ls.push(std::string("file")); ls.push(rows[i].File); ls.setTable(-3);
		ls.push(std::string("editable")); ls.push(rows[i].Editable); ls.setTable(-3);
		ls.push(std::string("frozen")); ls.push(rows[i].Frozen); ls.setTable(-3);
		ls.push(std::string("dirty")); ls.push(rows[i].Dirty); ls.setTable(-3);
		ls.rawSetI(-2, (int)(i + 1));
	}
	return 1;
}

static int lSave(CLuaState &ls) // painter.save("out.max")
{
	std::string target;
	if (!argString(ls, 1, target)) return retErr(ls, "usage: save(\"out.max\")");
	if (!s_Host || !s_Host->saveTo) return retErr(ls, "painter: no active session");
	if (!s_Host->saveTo(target)) return retErr(ls, "save failed (see log)");
	return retOk(ls);
}

static int lSaveAll(CLuaState &ls) // painter.saveAll()
{
	if (!s_Host || !s_Host->saveAll) return retErr(ls, "saveAll: not available in this mode");
	if (!s_Host->saveAll()) return retErr(ls, "saveAll failed (see log)");
	return retOk(ls);
}

static int lScreenshot(CLuaState &ls) // painter.screenshot("out.tga")
{
	std::string path;
	if (!argString(ls, 1, path)) return retErr(ls, "usage: screenshot(\"out.tga\")");
	if (!s_Host || !s_Host->screenshot) return retErr(ls, "screenshot: viewer only");
	std::string err;
	if (!s_Host->screenshot(path, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lOpenZone(CLuaState &ls) // painter.openZone("basename"[,cx,cy]); board session
{
	std::string base;
	if (!argString(ls, 1, base)) return retErr(ls, "usage: openZone(\"basename\"[,cx,cy])");
	std::string err;
	// 3-arg form opens at an eco board cell (scratchOpenEditable); 1-arg form is the
	// continent open (eco sessions refuse it; they need a cell).
	double cx = 0, cy = 0;
	if (ls.getTop() >= 2)
	{
		if (!argNumber(ls, 2, cx) || !argNumber(ls, 3, cy))
			return retErr(ls, "usage: openZone(\"basename\"[,cx,cy])");
		if (!s_Host || !s_Host->openZoneAt) return retErr(ls, "openZone(cx,cy): board session only");
		if (!s_Host->openZoneAt(base, (int)cx, (int)cy, err)) return retErr(ls, err);
		return retOk(ls);
	}
	if (!s_Host || !s_Host->openZone) return retErr(ls, "openZone: board session only");
	if (!s_Host->openZone(base, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lPlaceInstance(CLuaState &ls) // painter.placeInstance(cx,cy[,"basename"]); eco board
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "placeInstance(cx,cy[,\"basename\"])", err)) return retErr(ls, err);
	std::string base;
	argString(ls, 3, base); // optional; empty = the assembly-pinned first-file source
	if (!s_Host || !s_Host->placeInstance) return retErr(ls, "placeInstance: board session only");
	if (!s_Host->placeInstance((int)a[0], (int)a[1], base, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lRemoveInstance(CLuaState &ls) // painter.removeInstance(cx,cy); eco board
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "removeInstance(cx,cy)", err)) return retErr(ls, err);
	if (!s_Host || !s_Host->removeInstance) return retErr(ls, "removeInstance: board session only");
	if (!s_Host->removeInstance((int)a[0], (int)a[1], err)) return retErr(ls, err);
	return retOk(ls);
}

static int lRotateInstance(CLuaState &ls) // painter.rotateInstance(cx,cy[,delta=1]); eco board
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "rotateInstance(cx,cy[,delta])", err)) return retErr(ls, err);
	double delta = 1;
	argNumber(ls, 3, delta);
	if (!s_Host || !s_Host->rotateInstance) return retErr(ls, "rotateInstance: board session only");
	if (!s_Host->rotateInstance((int)a[0], (int)a[1], (int)delta, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lMirrorInstance(CLuaState &ls) // painter.mirrorInstance(cx,cy); eco board
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "mirrorInstance(cx,cy)", err)) return retErr(ls, err);
	if (!s_Host || !s_Host->mirrorInstance) return retErr(ls, "mirrorInstance: board session only");
	if (!s_Host->mirrorInstance((int)a[0], (int)a[1], err)) return retErr(ls, err);
	return retOk(ls);
}

static int lCloseZone(CLuaState &ls) // painter.closeZone("basename"[,saveFirst[,forceDiscard]])
{
	std::string base;
	if (!argString(ls, 1, base)) return retErr(ls, "usage: closeZone(\"basename\"[,saveFirst[,forceDiscard]])");
	if (!s_Host || !s_Host->closeZone) return retErr(ls, "closeZone: board session only");
	bool saveFirst = argBoolOpt(ls, 2, false);
	bool forceDiscard = argBoolOpt(ls, 3, false);
	std::string err;
	if (!s_Host->closeZone(base, saveFirst, forceDiscard, err)) return retErr(ls, err);
	return retOk(ls);
}

// Board-op completion: context specs, promote, cell drag, RO toggle, per-file save.
// Same host-wrapper idiom as the instance ops; hosts gate eco/continent themselves.

static int lPlaceContext(CLuaState &ls) // painter.placeContext(cx,cy,"basename"); eco board
{
	double a[2];
	std::string err, base;
	if (!argNumbers(ls, 2, a, "placeContext(cx,cy,\"basename\")", err)) return retErr(ls, err);
	if (!argString(ls, 3, base) || base.empty())
		return retErr(ls, "usage: placeContext(cx,cy,\"basename\")");
	if (!s_Host || !s_Host->placeContext) return retErr(ls, "placeContext: board session only");
	if (!s_Host->placeContext((int)a[0], (int)a[1], base, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lRemoveContext(CLuaState &ls) // painter.removeContext(cx,cy); eco board
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "removeContext(cx,cy)", err)) return retErr(ls, err);
	if (!s_Host || !s_Host->removeContext) return retErr(ls, "removeContext: board session only");
	if (!s_Host->removeContext((int)a[0], (int)a[1], err)) return retErr(ls, err);
	return retOk(ls);
}

static int lRotateContext(CLuaState &ls) // painter.rotateContext(cx,cy[,delta=1]); eco board
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "rotateContext(cx,cy[,delta])", err)) return retErr(ls, err);
	double delta = 1;
	argNumber(ls, 3, delta);
	if (!s_Host || !s_Host->rotateContext) return retErr(ls, "rotateContext: board session only");
	if (!s_Host->rotateContext((int)a[0], (int)a[1], (int)delta, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lMirrorContext(CLuaState &ls) // painter.mirrorContext(cx,cy); eco board
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "mirrorContext(cx,cy)", err)) return retErr(ls, err);
	if (!s_Host || !s_Host->mirrorContext) return retErr(ls, "mirrorContext: board session only");
	if (!s_Host->mirrorContext((int)a[0], (int)a[1], err)) return retErr(ls, err);
	return retOk(ls);
}

static int lMakeEditable(CLuaState &ls) // painter.makeEditable(cx,cy); promote RO context
{
	double a[2];
	std::string err;
	if (!argNumbers(ls, 2, a, "makeEditable(cx,cy)", err)) return retErr(ls, err);
	if (!s_Host || !s_Host->makeEditable) return retErr(ls, "makeEditable: board session only");
	if (!s_Host->makeEditable((int)a[0], (int)a[1], err)) return retErr(ls, err);
	return retOk(ls);
}

static int lMoveCell(CLuaState &ls) // painter.moveCell(fx,fy,tx,ty); board drag move
{
	double a[4];
	std::string err;
	if (!argNumbers(ls, 4, a, "moveCell(fx,fy,tx,ty)", err)) return retErr(ls, err);
	if (!s_Host || !s_Host->dragCell) return retErr(ls, "moveCell: board session only");
	if (!s_Host->dragCell((int)a[0], (int)a[1], (int)a[2], (int)a[3], false, err))
		return retErr(ls, err);
	return retOk(ls);
}

static int lCopyCell(CLuaState &ls) // painter.copyCell(fx,fy,tx,ty); board drag copy
{
	double a[4];
	std::string err;
	if (!argNumbers(ls, 4, a, "copyCell(fx,fy,tx,ty)", err)) return retErr(ls, err);
	if (!s_Host || !s_Host->dragCell) return retErr(ls, "copyCell: board session only");
	if (!s_Host->dragCell((int)a[0], (int)a[1], (int)a[2], (int)a[3], true, err))
		return retErr(ls, err);
	return retOk(ls);
}

static int lToggleZone(CLuaState &ls) // painter.toggleZone("basename"[,saveFirst[,forceDiscard]])
{
	std::string base;
	if (!argString(ls, 1, base))
		return retErr(ls, "usage: toggleZone(\"basename\"[,saveFirst[,forceDiscard]])");
	if (!s_Host || !s_Host->toggleZone) return retErr(ls, "toggleZone: board session only");
	bool saveFirst = argBoolOpt(ls, 2, false);
	bool forceDiscard = argBoolOpt(ls, 3, false);
	std::string err;
	if (!s_Host->toggleZone(base, saveFirst, forceDiscard, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lSaveZone(CLuaState &ls) // painter.saveZone("basename"); per-file board save
{
	std::string base;
	if (!argString(ls, 1, base)) return retErr(ls, "usage: saveZone(\"basename\")");
	if (!s_Host || !s_Host->saveZone) return retErr(ls, "saveZone: board session only");
	std::string err;
	if (!s_Host->saveZone(base, err)) return retErr(ls, err);
	return retOk(ls);
}

static int lPumpUI(CLuaState &ls) // painter.pumpUI() -> true, or false when cancel was requested
{
	if (s_Host && s_Host->pumpUI) s_Host->pumpUI();
	bool cancelled = s_CancelReq
		|| (s_Host && s_Host->cancelRequested && s_Host->cancelRequested());
	ls.push(!cancelled);
	return 1;
}

// ---------------------------------------------------------------------------------------------
// Viewer state (bridge-backed; graceful headless)

static ZPUI::SPaintUIBridge *bridge() { return s_Host ? s_Host->bridge : NULL; }

static int lSetMode(CLuaState &ls) // painter.setMode(0..4) tile/color/displace/prop/patch
{
	double m; if (!argNumber(ls, 1, m)) return retErr(ls, "usage: setMode(0..4)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->selectMode) return retErr(ls, "setMode: viewer only");
	b->selectMode((int)m);
	return retOk(ls);
}

static int lSetSubObject(CLuaState &ls) // painter.setSubObject(0..4) EP_OBJECT..EP_TILE
{
	double m; if (!argNumber(ls, 1, m)) return retErr(ls, "usage: setSubObject(0..4)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->selectSubObject) return retErr(ls, "setSubObject: viewer only");
	b->selectSubObject((int)m);
	return retOk(ls);
}

static int lGetSubObject(CLuaState &ls)
{
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b) return retErr(ls, "getSubObject: viewer only");
	if (s_Host && s_Host->refreshBridge) s_Host->refreshBridge();
	ls.push((double)b->SubObj);
	return 1;
}

static int lSelectPatchVertex(CLuaState &ls) // (zone, vertIdx, op) op: 0 replace 1 add 2 remove
{
	double z, v, o;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, v) || !argNumber(ls, 3, o))
		return retErr(ls, "usage: selectPatchVertex(zone, vertIndex, op)");
	zpPatchVertSelect((uint)z, (uint)v, (int)o);
	return retOk(ls);
}

static int lClearPatchVertexSelection(CLuaState &ls)
{
	zpPatchVertClear();
	return retOk(ls);
}

static int lPatchVertexSelectionCount(CLuaState &ls)
{
	ls.push((double)zpPatchVertSelCount());
	return 1;
}

static int lPatchVertexPos(CLuaState &ls) // (zone, vertIdx) -> x, y, z in DISPLAYED world space
{
	double z, v;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, v))
		return retErr(ls, "usage: patchVertexPos(zone, vertIndex)");
	float p[3];
	if (!zpPatchVertWorld((uint)z, (uint)v, p))
		return retErr(ls, "patchVertexPos: no such zone/vertex");
	ls.push((double)p[0]);
	ls.push((double)p[1]);
	ls.push((double)p[2]);
	return 3;
}

static int lSelectPatchEdge(CLuaState &ls) // (zone, vertA, vertB, op)
{
	double z, a, b, o;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, a) || !argNumber(ls, 3, b) || !argNumber(ls, 4, o))
		return retErr(ls, "usage: selectPatchEdge(zone, vertA, vertB, op)");
	zpPatchEdgeSelect((uint)z, (uint)a, (uint)b, (int)o);
	return retOk(ls);
}

static int lSelectPatchFace(CLuaState &ls) // (zone, patchIndex, op)
{
	double z, p, o;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, p) || !argNumber(ls, 3, o))
		return retErr(ls, "usage: selectPatchFace(zone, patchIndex, op)");
	zpPatchFaceSelect((uint)z, (uint)p, (int)o);
	return retOk(ls);
}

static int lPatchEdgeSelectionCount(CLuaState &ls)
{
	ls.push((double)zpPatchEdgeSelCount());
	return 1;
}

static int lPatchFaceSelectionCount(CLuaState &ls)
{
	ls.push((double)zpPatchFaceSelCount());
	return 1;
}

static int lSelectPatchTangent(CLuaState &ls) // (zone, vecIndex, op)
{
	double z, v, o;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, v) || !argNumber(ls, 3, o))
		return retErr(ls, "usage: selectPatchTangent(zone, vecIndex, op)");
	zpPatchTangentSelect((uint)z, (uint)v, (int)o);
	return retOk(ls);
}

static int lPatchTangentSelectionCount(CLuaState &ls)
{
	ls.push((double)zpPatchTangentSelCount());
	return 1;
}

static int lPatchTangentPos(CLuaState &ls) // (zone, vecIndex) -> x, y, z displayed world
{
	double z, v;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, v))
		return retErr(ls, "usage: patchTangentPos(zone, vecIndex)");
	float p[3];
	if (!zpPatchTangentWorld((uint)z, (uint)v, p))
		return retErr(ls, "patchTangentPos: no such zone/handle");
	ls.push((double)p[0]);
	ls.push((double)p[1]);
	ls.push((double)p[2]);
	return 3;
}

static int lSetPivotMode(CLuaState &ls) // (0..4) TPivotMode
{
	double m;
	if (!argNumber(ls, 1, m)) return retErr(ls, "usage: setPivotMode(0..4)");
	zpSetPivotMode((int)m);
	return retOk(ls);
}

static int lSetUserPivotToSelection(CLuaState &ls)
{
	if (!zpSetUserPivotToSelection())
		return retErr(ls, "setUserPivotToSelection: nothing selected");
	return retOk(ls);
}

static int lPivotPos(CLuaState &ls) // -> x, y, z, or nil when the mode cannot form one
{
	float p[3];
	if (!zpTransformPivotXYZ(p))
		return retErr(ls, "pivotPos: no pivot for the current mode");
	ls.push((double)p[0]);
	ls.push((double)p[1]);
	ls.push((double)p[2]);
	return 3;
}

static int lRotatePatchSelection(CLuaState &ls) // (axis 0..2, degrees)
{
	double a, d;
	if (!argNumber(ls, 1, a) || !argNumber(ls, 2, d))
		return retErr(ls, "usage: rotatePatchSelection(axis, degrees)");
	std::string msg;
	const uint n = zpApplyPatchRotate((int)a, (float)d, msg);
	printf("rotatePatchSelection: %u written (%s)\n", n, msg.c_str());
	fflush(stdout);
	return retOk(ls);
}

static int lRotatePatchSelectionAxis(CLuaState &ls) // (ax, ay, az, degrees)
{
	double x, y, z, d;
	if (!argNumber(ls, 1, x) || !argNumber(ls, 2, y) || !argNumber(ls, 3, z) || !argNumber(ls, 4, d))
		return retErr(ls, "usage: rotatePatchSelectionAxis(ax, ay, az, degrees)");
	std::string msg;
	const uint n = zpApplyPatchRotateAxis((float)x, (float)y, (float)z, (float)d, msg);
	printf("rotatePatchSelectionAxis: %u written (%s)\n", n, msg.c_str());
	fflush(stdout);
	return retOk(ls);
}

static int lScalePatchSelection(CLuaState &ls) // (sx, sy, sz)
{
	double x, y, z;
	if (!argNumber(ls, 1, x) || !argNumber(ls, 2, y) || !argNumber(ls, 3, z))
		return retErr(ls, "usage: scalePatchSelection(sx, sy, sz)");
	std::string msg;
	const uint n = zpApplyPatchScale((float)x, (float)y, (float)z, msg);
	printf("scalePatchSelection: %u written (%s)\n", n, msg.c_str());
	fflush(stdout);
	return retOk(ls);
}

static int lSetXformKind(CLuaState &ls) // (0 move, 1 rotate, 2 scale)
{
	double k;
	if (!argNumber(ls, 1, k)) return retErr(ls, "usage: setXformKind(0..2)");
	zpSetXformKind((int)k);
	return retOk(ls);
}

static int lPatchClick(CLuaState &ls) // (x, y [, buttons]) normalized viewport, 0..1
{
	double x, y, btn = 0;
	if (!argNumber(ls, 1, x) || !argNumber(ls, 2, y))
		return retErr(ls, "usage: patchClick(x, y [, buttons])");
	argNumber(ls, 3, btn);
	if (!zpPatchClickAt((float)x, (float)y, (uint)btn))
		return retErr(ls, "patchClick: viewer only");
	return retOk(ls);
}

static int lMovePatchSelection(CLuaState &ls) // (dx, dy, dz) world units
{
	double x, y, z;
	if (!argNumber(ls, 1, x) || !argNumber(ls, 2, y) || !argNumber(ls, 3, z))
		return retErr(ls, "usage: movePatchSelection(dx, dy, dz)");
	std::string msg;
	const uint n = zpApplyPatchMove(NLMISC::CVector((float)x, (float)y, (float)z), msg);
	printf("movePatchSelection: %u written (%s)\n", n, msg.c_str());
	fflush(stdout);
	return retOk(ls);
}

static int lUnbindPatchSelection(CLuaState &ls)
{
	const uint n = zpUnbindPatchSelection();
	printf("unbindPatchSelection: %u released\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lBindPatchSelection(CLuaState &ls)
{
	const uint n = zpBindPatchSelection();
	printf("bindPatchSelection: %u bound\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lBindPatchVertex(CLuaState &ls) // (zone, vertIdx, patchIdx, edgeSlot)
{
	double z, v, p, e;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, v) || !argNumber(ls, 3, p) || !argNumber(ls, 4, e))
		return retErr(ls, "usage: bindPatchVertex(zone, vertIndex, patchIndex, edgeSlot)");
	std::string msg;
	const bool ok = zpBindPatchVertexToEdge((uint)z, (uint)v, (uint)p, (uint)e, msg);
	printf("bindPatchVertex: %s\n", msg.c_str());
	fflush(stdout);
	if (!ok)
		return retErr(ls, msg);
	return retOk(ls);
}

static int lSetEdgeNoSmooth(CLuaState &ls) // (noSmooth bool)
{
	if (ls.getTop() < 1 || !ls.isBoolean(1))
		return retErr(ls, "usage: setEdgeNoSmooth(true|false)");
	const uint n = zpSetEdgeNoSmooth(ls.toBoolean(1));
	printf("setEdgeNoSmooth: %u flags written\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lEdgeNoSmooth(CLuaState &ls) // (zone, vertA, vertB) -> 0/1
{
	double z, a, b;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, a) || !argNumber(ls, 3, b))
		return retErr(ls, "usage: edgeNoSmooth(zone, vertA, vertB)");
	const int s = zpEdgeNoSmoothQuery((uint)z, (uint)a, (uint)b);
	if (s < 0)
		return retErr(ls, "edgeNoSmooth: no such edge");
	ls.push((double)s);
	return 1;
}

static int lVertexBindInfo(CLuaState &ls) // (zone, vertIdx) -> binded, type, patch, edge, prim
{
	double z, v;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, v))
		return retErr(ls, "usage: vertexBindInfo(zone, vertIndex)");
	int binded, type, patch, edge, prim;
	if (!zpVertexBindQuery((uint)z, (uint)v, binded, type, patch, edge, prim))
		return retErr(ls, "vertexBindInfo: no such zone/vertex");
	ls.push((double)binded);
	ls.push((double)type);
	ls.push((double)patch);
	ls.push((double)edge);
	ls.push((double)prim);
	return 5;
}

static int lDeletePatchSelection(CLuaState &ls)
{
	const uint n = zpDeletePatchSelection();
	printf("deletePatchSelection: %u removed\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lTurnPatchSelection(CLuaState &ls) // (ccw bool)
{
	if (ls.getTop() < 1 || !ls.isBoolean(1))
		return retErr(ls, "usage: turnPatchSelection(true|false)");
	const uint n = zpTurnPatchSelection(ls.toBoolean(1));
	printf("turnPatchSelection: %u turned\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lWeldPatchSelection(CLuaState &ls) // ([threshold])
{
	double th = 0.1;
	argNumber(ls, 1, th);
	const uint n = zpWeldPatchSelection((float)th);
	printf("weldPatchSelection: %u merged\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lAddQuadPatchSelection(CLuaState &ls)
{
	const uint n = zpAddQuadPatchSelection();
	printf("addQuadPatchSelection: %u added\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lRawTile(CLuaState &ls) // (zone, patch, u, v, tile [, rot]) raw record, no solver
{
	double z, pch, u, v, t, r = 0;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, pch) || !argNumber(ls, 3, u)
	    || !argNumber(ls, 4, v) || !argNumber(ls, 5, t))
		return retErr(ls, "usage: rawTile(zone, patch, u, v, tile [, rot])");
	argNumber(ls, 6, r);
	return execOpRet(ls, toString("rawtile %u %u %u %u %d %u",
		(uint)z, (uint)pch, (uint)u, (uint)v, (int)t, (uint)r));
}

static int lSubdividePatchSelection(CLuaState &ls)
{
	const uint n = zpSubdividePatchSelection();
	printf("subdividePatchSelection: %u subdivided\n", n);
	fflush(stdout);
	ls.push((double)n);
	return 1;
}

static int lPatchCount(CLuaState &ls) // (zone) -> displayed patch count
{
	double z;
	if (!argNumber(ls, 1, z))
		return retErr(ls, "usage: patchCount(zone)");
	uint n;
	if (!zpZonePatchCount((uint)z, n))
		return retErr(ls, "patchCount: no such zone");
	ls.push((double)n);
	return 1;
}

static int lVertexCount(CLuaState &ls) // (zone)
{
	double z;
	if (!argNumber(ls, 1, z))
		return retErr(ls, "usage: vertexCount(zone)");
	uint n;
	if (!zpZoneVertCount((uint)z, n))
		return retErr(ls, "vertexCount: no such zone");
	ls.push((double)n);
	return 1;
}

static int lTileAt(CLuaState &ls) // (zone, patch, u, v) -> layer0 tile, rotate, numLayers
{
	double z, p, u, v;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, p) || !argNumber(ls, 3, u) || !argNumber(ls, 4, v))
		return retErr(ls, "usage: tileAt(zone, patch, u, v)");
	int tile, rot, num;
	if (!zpTileQuery((uint)z, (uint)p, (uint)u, (uint)v, tile, rot, num))
		return retErr(ls, "tileAt: out of range");
	ls.push((double)tile);
	ls.push((double)rot);
	ls.push((double)num);
	return 3;
}

static int lPatchEdgeVerts(CLuaState &ls) // (zone, patchIdx, edgeSlot) -> vertA, vertB
{
	double z, p, e;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, p) || !argNumber(ls, 3, e))
		return retErr(ls, "usage: patchEdgeVerts(zone, patchIndex, edgeSlot)");
	uint a, b;
	if (!zpPatchEdgeCornerPair((uint)z, (uint)p, (uint)e, a, b))
		return retErr(ls, "patchEdgeVerts: no such edge");
	ls.push((double)a);
	ls.push((double)b);
	return 2;
}

static int lPatchVecIndex(CLuaState &ls) // (zone, patchIdx, ringSlot 0..7) -> vec index
{
	double z, p, s;
	if (!argNumber(ls, 1, z) || !argNumber(ls, 2, p) || !argNumber(ls, 3, s))
		return retErr(ls, "usage: patchVecIndex(zone, patchIndex, ringSlot)");
	uint v;
	if (!zpPatchVecIndex((uint)z, (uint)p, (uint)s, v))
		return retErr(ls, "patchVecIndex: no such slot");
	ls.push((double)v);
	return 1;
}

static int lGetMode(CLuaState &ls)
{
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b) return retErr(ls, "getMode: viewer only");
	// Snapshot fields are frame-synced and STALE for the whole script run; refresh first
	if (s_Host && s_Host->refreshBridge) s_Host->refreshBridge();
	ls.push((double)b->Mode);
	return 1;
}

static int lSetTileSet(CLuaState &ls) // painter.setTileSet(index)
{
	double i; if (!argNumber(ls, 1, i)) return retErr(ls, "usage: setTileSet(index)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->selectTileSetAbs) return retErr(ls, "setTileSet: viewer only");
	b->selectTileSetAbs((int)i);
	return retOk(ls);
}

static int lGetTileSet(CLuaState &ls)
{
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b) return retErr(ls, "getTileSet: viewer only");
	// Snapshot fields are frame-synced and STALE for the whole script run; refresh first
	if (s_Host && s_Host->refreshBridge) s_Host->refreshBridge();
	ls.push((double)b->CurTileSet);
	return 1;
}

static int lSetDisplaceIndex(CLuaState &ls) // painter.setDisplaceIndex(0..15)
{
	double i; if (!argNumber(ls, 1, i)) return retErr(ls, "usage: setDisplaceIndex(0..15)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->displaceIndexAbs) return retErr(ls, "setDisplaceIndex: viewer only");
	b->displaceIndexAbs((int)i);
	return retOk(ls);
}

static int lSetBrushColor(CLuaState &ls) // painter.setBrushColor(r,g,b)
{
	double r, g, bb;
	if (!argNumber(ls, 1, r) || !argNumber(ls, 2, g) || !argNumber(ls, 3, bb))
		return retErr(ls, "usage: setBrushColor(r,g,b)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->setBrushColor) return retErr(ls, "setBrushColor: viewer only");
	b->setBrushColor((int)r, (int)g, (int)bb);
	return retOk(ls);
}

static int lSetSeason(CLuaState &ls) // painter.setSeason("sp"|"su"|"au"|"wi")
{
	std::string code; if (!argString(ls, 1, code)) return retErr(ls, "usage: setSeason(\"sp|su|au|wi\")");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->seasonSelect) return retErr(ls, "setSeason: viewer only");
	b->seasonSelect(code);
	return retOk(ls);
}

static int lSet256(CLuaState &ls) // painter.set256(bool); 128/256 mouse-stroke mode
{
	bool on = argBoolOpt(ls, 1, true);
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->setTileSize256) return retErr(ls, "set256: viewer only");
	b->setTileSize256(on);
	return retOk(ls);
}

static int lSetHardness(CLuaState &ls) // painter.setHardness(0..255)
{
	double v; if (!argNumber(ls, 1, v)) return retErr(ls, "usage: setHardness(0..255)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->setHardnessAbs) return retErr(ls, "setHardness: viewer only");
	b->setHardnessAbs((int)v);
	return retOk(ls);
}

static int lSetOpacity(CLuaState &ls) // painter.setOpacity(0..255)
{
	double v; if (!argNumber(ls, 1, v)) return retErr(ls, "usage: setOpacity(0..255)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->setOpacityAbs) return retErr(ls, "setOpacity: viewer only");
	b->setOpacityAbs((int)v);
	return retOk(ls);
}

static int lSetRadius(CLuaState &ls) // painter.setRadius(meters 2..32)
{
	double v; if (!argNumber(ls, 1, v)) return retErr(ls, "usage: setRadius(meters)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->setColorRadiusAbs) return retErr(ls, "setRadius: viewer only");
	b->setColorRadiusAbs((float)v);
	return retOk(ls);
}

// ---------------------------------------------------------------------------------------------
// Recorder API

static int lSetRecording(CLuaState &ls)
{
	setRecording(argBoolOpt(ls, 1, true));
	return retOk(ls);
}

static int lIsRecording(CLuaState &ls)
{
	ls.push(isRecording());
	return 1;
}

static int lRecorderText(CLuaState &ls)
{
	ls.push(recorderText());
	return 1;
}

static int lClearRecorder(CLuaState &ls)
{
	clearRecorder();
	return retOk(ls);
}

// print() replacement: tee to stdout and the Script window output buffer
static int lPrint(CLuaState &ls)
{
	std::string lineOut;
	int n = ls.getTop();
	for (int i = 1; i <= n; ++i)
	{
		std::string part;
		if (ls.isString(i) || ls.isNumber(i)) ls.toString(i, part);
		else if (ls.isBoolean(i)) part = ls.toBoolean(i) ? "true" : "false";
		else if (ls.getTop() >= i) part = "<value>";
		if (i > 1) lineOut += "\t";
		lineOut += part;
	}
	printf("%s\n", lineOut.c_str());
	s_Output += lineOut;
	s_Output += "\n";
	++s_OutputGen;
	// Keep the window buffer bounded (drop oldest lines past ~64 KB)
	if (s_Output.size() > 65536)
		s_Output.erase(0, s_Output.size() - 49152);
	return 0;
}

// ---------------------------------------------------------------------------------------------

/** The painter table, built from the registered __zp_* globals (client-style camelCase). */
static const char *kBootstrap =
	"painter = {\n"
	"  paintTile = __zp_paintTile, rotateTile = __zp_rotateTile, clearTile = __zp_clearTile,\n"
	"  fillTile = __zp_fillTile,\n"
	"  paintColor = __zp_paintColor, fillColor = __zp_fillColor, colorBrush = __zp_colorBrush,\n"
	"  tileStroke = __zp_tileStroke, endStroke = __zp_endStroke,\n"
	"  paintDisplace = __zp_paintDisplace, fillDisplace = __zp_fillDisplace,\n"
	"  setBrushSize = __zp_setBrushSize, setTileGroup = __zp_setTileGroup,\n"
	"  setBrushMask = __zp_setBrushMask,\n"
	"  setLockBorders = __zp_setLockBorders, setMaskMode = __zp_setMaskMode,\n"
	"  set256 = __zp_set256, setHardness = __zp_setHardness,\n"
	"  setOpacity = __zp_setOpacity, setRadius = __zp_setRadius,\n"
	"  undo = __zp_undo, redo = __zp_redo, seed = __zp_seed, checkSeams = __zp_checkSeams,\n"
	"  setZoneProp = __zp_setZoneProp, getZoneProp = __zp_getZoneProp,\n"
	"  zones = __zp_zones, save = __zp_save, saveAll = __zp_saveAll,\n"
	"  screenshot = __zp_screenshot, pumpUI = __zp_pumpUI,\n"
	"  openZone = __zp_openZone, closeZone = __zp_closeZone,\n"
	"  placeInstance = __zp_placeInstance, removeInstance = __zp_removeInstance,\n"
	"  rotateInstance = __zp_rotateInstance, mirrorInstance = __zp_mirrorInstance,\n"
	"  placeContext = __zp_placeContext, removeContext = __zp_removeContext,\n"
	"  rotateContext = __zp_rotateContext, mirrorContext = __zp_mirrorContext,\n"
	"  makeEditable = __zp_makeEditable,\n"
	"  moveCell = __zp_moveCell, copyCell = __zp_copyCell,\n"
	"  toggleZone = __zp_toggleZone, saveZone = __zp_saveZone,\n"
	"  setMode = __zp_setMode, getMode = __zp_getMode,\n"
	"  setSubObject = __zp_setSubObject, getSubObject = __zp_getSubObject,\n"
	"  selectPatchVertex = __zp_selectPatchVertex,\n"
	"  clearPatchVertexSelection = __zp_clearPatchVertexSelection,\n"
	"  patchVertexSelectionCount = __zp_patchVertexSelectionCount,\n"
	"  movePatchSelection = __zp_movePatchSelection,\n"
	"  patchVertexPos = __zp_patchVertexPos,\n"
	"  selectPatchEdge = __zp_selectPatchEdge, selectPatchFace = __zp_selectPatchFace,\n"
	"  patchEdgeSelectionCount = __zp_patchEdgeSelectionCount,\n"
	"  patchFaceSelectionCount = __zp_patchFaceSelectionCount,\n"
	"  patchClick = __zp_patchClick,\n"
	"  selectPatchTangent = __zp_selectPatchTangent,\n"
	"  patchTangentSelectionCount = __zp_patchTangentSelectionCount,\n"
	"  patchTangentPos = __zp_patchTangentPos,\n"
	"  setPivotMode = __zp_setPivotMode, pivotPos = __zp_pivotPos,\n"
	"  setUserPivotToSelection = __zp_setUserPivotToSelection,\n"
	"  rotatePatchSelection = __zp_rotatePatchSelection,\n"
	"  rotatePatchSelectionAxis = __zp_rotatePatchSelectionAxis,\n"
	"  scalePatchSelection = __zp_scalePatchSelection,\n"
	"  setXformKind = __zp_setXformKind,\n"
	"  unbindPatchSelection = __zp_unbindPatchSelection,\n"
	"  bindPatchSelection = __zp_bindPatchSelection,\n"
	"  bindPatchVertex = __zp_bindPatchVertex,\n"
	"  setEdgeNoSmooth = __zp_setEdgeNoSmooth, edgeNoSmooth = __zp_edgeNoSmooth,\n"
	"  vertexBindInfo = __zp_vertexBindInfo, patchEdgeVerts = __zp_patchEdgeVerts,\n"
	"  patchVecIndex = __zp_patchVecIndex,\n"
	"  deletePatchSelection = __zp_deletePatchSelection,\n"
	"  turnPatchSelection = __zp_turnPatchSelection,\n"
	"  subdividePatchSelection = __zp_subdividePatchSelection,\n"
	"  rawTile = __zp_rawTile,\n"
	"  weldPatchSelection = __zp_weldPatchSelection,\n"
	"  addQuadPatchSelection = __zp_addQuadPatchSelection,\n"
	"  patchCount = __zp_patchCount, tileAt = __zp_tileAt, vertexCount = __zp_vertexCount,\n"
	"  setTileSet = __zp_setTileSet, getTileSet = __zp_getTileSet,\n"
	"  setDisplaceIndex = __zp_setDisplaceIndex, setBrushColor = __zp_setBrushColor,\n"
	"  setSeason = __zp_setSeason,\n"
	"  setRecording = __zp_setRecording, isRecording = __zp_isRecording,\n"
	"  recorderText = __zp_recorderText, clearRecorder = __zp_clearRecorder,\n"
	"}\n"
	"print = __zp_print\n";

bool ensureLua()
{
	if (s_Registered)
		return true;
	CLuaState *ls = CLuaManager::getInstance().getLuaState();
	if (!ls)
		return false;
	ls->registerFunc("__zp_paintTile", lPaintTile);
	ls->registerFunc("__zp_rotateTile", lRotateTile);
	ls->registerFunc("__zp_clearTile", lClearTile);
	ls->registerFunc("__zp_fillTile", lFillTile);
	ls->registerFunc("__zp_paintColor", lPaintColor);
	ls->registerFunc("__zp_fillColor", lFillColor);
	ls->registerFunc("__zp_colorBrush", lColorBrush);
	ls->registerFunc("__zp_tileStroke", lTileStroke);
	ls->registerFunc("__zp_endStroke", lEndStroke);
	ls->registerFunc("__zp_paintDisplace", lPaintDisplace);
	ls->registerFunc("__zp_fillDisplace", lFillDisplace);
	ls->registerFunc("__zp_setBrushSize", lSetBrushSize);
	ls->registerFunc("__zp_setTileGroup", lSetTileGroup);
	ls->registerFunc("__zp_setBrushMask", lSetBrushMask);
	ls->registerFunc("__zp_setLockBorders", lSetLockBorders);
	ls->registerFunc("__zp_setMaskMode", lSetMaskMode);
	ls->registerFunc("__zp_set256", lSet256);
	ls->registerFunc("__zp_setHardness", lSetHardness);
	ls->registerFunc("__zp_setOpacity", lSetOpacity);
	ls->registerFunc("__zp_setRadius", lSetRadius);
	ls->registerFunc("__zp_undo", lUndo);
	ls->registerFunc("__zp_redo", lRedo);
	ls->registerFunc("__zp_seed", lSeed);
	ls->registerFunc("__zp_checkSeams", lCheckSeams);
	ls->registerFunc("__zp_setZoneProp", lSetZoneProp);
	ls->registerFunc("__zp_getZoneProp", lGetZoneProp);
	ls->registerFunc("__zp_zones", lZones);
	ls->registerFunc("__zp_save", lSave);
	ls->registerFunc("__zp_saveAll", lSaveAll);
	ls->registerFunc("__zp_screenshot", lScreenshot);
	ls->registerFunc("__zp_openZone", lOpenZone);
	ls->registerFunc("__zp_closeZone", lCloseZone);
	ls->registerFunc("__zp_placeInstance", lPlaceInstance);
	ls->registerFunc("__zp_removeInstance", lRemoveInstance);
	ls->registerFunc("__zp_rotateInstance", lRotateInstance);
	ls->registerFunc("__zp_mirrorInstance", lMirrorInstance);
	ls->registerFunc("__zp_placeContext", lPlaceContext);
	ls->registerFunc("__zp_removeContext", lRemoveContext);
	ls->registerFunc("__zp_rotateContext", lRotateContext);
	ls->registerFunc("__zp_mirrorContext", lMirrorContext);
	ls->registerFunc("__zp_makeEditable", lMakeEditable);
	ls->registerFunc("__zp_moveCell", lMoveCell);
	ls->registerFunc("__zp_copyCell", lCopyCell);
	ls->registerFunc("__zp_toggleZone", lToggleZone);
	ls->registerFunc("__zp_saveZone", lSaveZone);
	ls->registerFunc("__zp_pumpUI", lPumpUI);
	ls->registerFunc("__zp_setMode", lSetMode);
	ls->registerFunc("__zp_setSubObject", lSetSubObject);
	ls->registerFunc("__zp_selectPatchVertex", lSelectPatchVertex);
	ls->registerFunc("__zp_patchVertexPos", lPatchVertexPos);
	ls->registerFunc("__zp_selectPatchEdge", lSelectPatchEdge);
	ls->registerFunc("__zp_selectPatchFace", lSelectPatchFace);
	ls->registerFunc("__zp_patchEdgeSelectionCount", lPatchEdgeSelectionCount);
	ls->registerFunc("__zp_patchFaceSelectionCount", lPatchFaceSelectionCount);
	ls->registerFunc("__zp_patchClick", lPatchClick);
	ls->registerFunc("__zp_selectPatchTangent", lSelectPatchTangent);
	ls->registerFunc("__zp_patchTangentSelectionCount", lPatchTangentSelectionCount);
	ls->registerFunc("__zp_patchTangentPos", lPatchTangentPos);
	ls->registerFunc("__zp_setPivotMode", lSetPivotMode);
	ls->registerFunc("__zp_setUserPivotToSelection", lSetUserPivotToSelection);
	ls->registerFunc("__zp_pivotPos", lPivotPos);
	ls->registerFunc("__zp_rotatePatchSelection", lRotatePatchSelection);
	ls->registerFunc("__zp_rotatePatchSelectionAxis", lRotatePatchSelectionAxis);
	ls->registerFunc("__zp_scalePatchSelection", lScalePatchSelection);
	ls->registerFunc("__zp_setXformKind", lSetXformKind);
	ls->registerFunc("__zp_clearPatchVertexSelection", lClearPatchVertexSelection);
	ls->registerFunc("__zp_patchVertexSelectionCount", lPatchVertexSelectionCount);
	ls->registerFunc("__zp_movePatchSelection", lMovePatchSelection);
	ls->registerFunc("__zp_unbindPatchSelection", lUnbindPatchSelection);
	ls->registerFunc("__zp_bindPatchSelection", lBindPatchSelection);
	ls->registerFunc("__zp_bindPatchVertex", lBindPatchVertex);
	ls->registerFunc("__zp_setEdgeNoSmooth", lSetEdgeNoSmooth);
	ls->registerFunc("__zp_edgeNoSmooth", lEdgeNoSmooth);
	ls->registerFunc("__zp_vertexBindInfo", lVertexBindInfo);
	ls->registerFunc("__zp_patchEdgeVerts", lPatchEdgeVerts);
	ls->registerFunc("__zp_patchVecIndex", lPatchVecIndex);
	ls->registerFunc("__zp_deletePatchSelection", lDeletePatchSelection);
	ls->registerFunc("__zp_turnPatchSelection", lTurnPatchSelection);
	ls->registerFunc("__zp_subdividePatchSelection", lSubdividePatchSelection);
	ls->registerFunc("__zp_rawTile", lRawTile);
	ls->registerFunc("__zp_weldPatchSelection", lWeldPatchSelection);
	ls->registerFunc("__zp_addQuadPatchSelection", lAddQuadPatchSelection);
	ls->registerFunc("__zp_patchCount", lPatchCount);
	ls->registerFunc("__zp_vertexCount", lVertexCount);
	ls->registerFunc("__zp_tileAt", lTileAt);
	ls->registerFunc("__zp_getSubObject", lGetSubObject);
	ls->registerFunc("__zp_getMode", lGetMode);
	ls->registerFunc("__zp_setTileSet", lSetTileSet);
	ls->registerFunc("__zp_getTileSet", lGetTileSet);
	ls->registerFunc("__zp_setDisplaceIndex", lSetDisplaceIndex);
	ls->registerFunc("__zp_setBrushColor", lSetBrushColor);
	ls->registerFunc("__zp_setSeason", lSetSeason);
	ls->registerFunc("__zp_setRecording", lSetRecording);
	ls->registerFunc("__zp_isRecording", lIsRecording);
	ls->registerFunc("__zp_recorderText", lRecorderText);
	ls->registerFunc("__zp_clearRecorder", lClearRecorder);
	ls->registerFunc("__zp_print", lPrint);
	if (!ls->executeScriptNoThrow(kBootstrap))
	{
		fprintf(stderr, "ERROR: painterscript bootstrap failed\n");
		return false;
	}
	s_Registered = true;
	return true;
}

static int runInternal(const std::string &code, const std::string &label)
{
	if (!ensureLua())
	{
		s_LastError = "lua state unavailable";
		return 1;
	}
	CLuaState *ls = CLuaManager::getInstance().getLuaState();
	s_LastError.clear();
	s_CancelReq = false;
	if (s_Host && s_Host->resetCancel)
		s_Host->resetCancel(); // an earlier ESC cancel must not abort this fresh run
	s_Executing = true;
	bool ok = false;
	try
	{
		ok = ls->executeScriptNoThrow(code);
	}
	catch (const std::exception &e)
	{
		s_LastError = e.what();
	}
	s_Executing = false;
	if (!ok)
	{
		if (s_LastError.empty()) s_LastError = "script error (see log)";
		fprintf(stderr, "ERROR: painterscript %s: %s\n", label.c_str(), s_LastError.c_str());
		s_Output += "ERROR: " + s_LastError + "\n";
		++s_OutputGen;
		return 1;
	}
	return 0;
}

int runFile(const std::string &path)
{
	CIFile f;
	if (!f.open(path))
	{
		s_LastError = "cannot open script: " + path;
		fprintf(stderr, "ERROR: %s\n", s_LastError.c_str());
		return 1;
	}
	std::string code;
	code.resize((size_t)f.getFileSize());
	if (!code.empty())
		f.serialBuffer((uint8 *)&code[0], (uint)code.size());
	return runInternal(code, CFile::getFilename(path));
}

int runString(const std::string &code)
{
	return runInternal(code, "chunk");
}

static std::string s_PendingRun;
static bool s_HavePendingRun = false;

void queueRunString(const std::string &code)
{
	if (s_Executing)
		return; // UI is locked during pumped scripts; RUN is guarded, belt only
	s_PendingRun = code;
	s_HavePendingRun = true;
}

void processPendingRun()
{
	if (!s_HavePendingRun || s_Executing)
		return;
	std::string code;
	code.swap(s_PendingRun);
	s_HavePendingRun = false;
	runInternal(code, "chunk");
}

} // namespace ZPSCRIPT

/* end of file */
