/**
 * \file script_api.cpp
 * \brief painterscript — Lua binding over the paint op layer (ui M23)
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * Own TU: NLGUI Lua (CLuaManager/CLuaState) + NLMISC only. Must NOT include
 * patch_eval.h / context_display.h / SCENELIB headers (main.cpp include contract).
 *
 * Binding style: wrapped functions (CLuaState&) registered as __zp_* globals, then a
 * bootstrap chunk builds the `painter` table (Ryzom client Lua conventions: camelCase,
 * namespaced table; MaxScript-familiar verbs). Every op returns either true or
 * (nil, "error string") — standard Lua error-return idiom, no exceptions across the
 * C boundary.
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

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include <nel/gui/lua_helper.h>
#include <nel/gui/lua_manager.h>

#include <cstdio>
#include <string>
#include <vector>

#include "editor_ui.h" // SPaintUIBridge (state get/set when the viewer is live)
#include "script_api.h"

using namespace NLMISC;
using namespace NLGUI;

namespace ZPSCRIPT {

static SScriptHost *s_Host = NULL;
static bool s_Registered = false;
static bool s_Executing = false;
static bool s_Recording = false;
static std::string s_Recorder;
static std::string s_Output;
static std::string s_LastError;

void setHost(SScriptHost *host) { s_Host = host; }
SScriptHost *getHost() { return s_Host; }
bool isExecuting() { return s_Executing; }
void setRecording(bool on) { s_Recording = on; }
bool isRecording() { return s_Recording; }
const std::string &recorderText() { return s_Recorder; }
void clearRecorder() { s_Recorder.clear(); }
const std::string &outputText() { return s_Output; }
void clearOutput() { s_Output.clear(); }
const std::string &lastError() { return s_LastError; }

void record(const std::string &luaCall)
{
	if (!s_Recording || s_Executing)
		return;
	s_Recorder += luaCall;
	s_Recorder += "\n";
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
	return execOpRet(ls, toString("%s %u %u %u %u %u %u", big ? "tile256" : "tile",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], (uint)a[4], (uint)rot));
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
	return execOpRet(ls, toString("%s %u %u %u %u", big ? "fill256" : "fill",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)rot));
}

static int lPaintColor(CLuaState &ls) // painter.paintColor(zone,patch,u,v,"rrggbb")
{
	double a[4]; std::string err, rgb;
	if (!argNumbers(ls, 4, a, "paintColor(zone,patch,u,v,\"rrggbb\")", err)) return retErr(ls, err);
	if (!argString(ls, 5, rgb)) return retErr(ls, "usage: paintColor(zone,patch,u,v,\"rrggbb\")");
	return execOpRet(ls, toString("color %u %u %u %u %s",
		(uint)a[0], (uint)a[1], (uint)a[2], (uint)a[3], rgb.c_str()));
}

static int lFillColor(CLuaState &ls) // painter.fillColor(zone,patch,"rrggbb")
{
	double a[2]; std::string err, rgb;
	if (!argNumbers(ls, 2, a, "fillColor(zone,patch,\"rrggbb\")", err)) return retErr(ls, err);
	if (!argString(ls, 3, rgb)) return retErr(ls, "usage: fillColor(zone,patch,\"rrggbb\")");
	return execOpRet(ls, toString("cfill %u %u %s", (uint)a[0], (uint)a[1], rgb.c_str()));
}

static int lColorBrush(CLuaState &ls) // painter.colorBrush(zone,x,y,radius,"rrggbb",hard,opac[,zWorld])
{
	double a[4]; std::string err, rgb;
	if (!argNumbers(ls, 4, a, "colorBrush(zone,x,y,radius,\"rrggbb\",hardness,opacity[,zWorld])", err))
		return retErr(ls, err);
	if (!argString(ls, 5, rgb)) return retErr(ls, "colorBrush: rgb string expected");
	double hard, opac;
	if (!argNumber(ls, 6, hard) || !argNumber(ls, 7, opac))
		return retErr(ls, "colorBrush: hardness/opacity expected");
	double zw;
	if (argNumber(ls, 8, zw))
		return execOpRet(ls, toString("cbrush %u %.3f %.3f %.3f %s %u %u %.3f",
			(uint)a[0], a[1], a[2], a[3], rgb.c_str(), (uint)hard, (uint)opac, zw));
	return execOpRet(ls, toString("cbrush %u %.3f %.3f %.3f %s %u %u",
		(uint)a[0], a[1], a[2], a[3], rgb.c_str(), (uint)hard, (uint)opac));
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
	std::string s; if (!argString(ls, 1, s)) return retErr(ls, "usage: setBrushMask(\"file.tga\"|\"none\")");
	return execOpRet(ls, "mask " + s);
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
		ls.push(std::string("id"));       ls.push((double)rows[i].Id);   ls.setTable(-3);
		ls.push(std::string("name"));     ls.push(rows[i].Name);         ls.setTable(-3);
		ls.push(std::string("file"));     ls.push(rows[i].File);         ls.setTable(-3);
		ls.push(std::string("editable")); ls.push(rows[i].Editable);     ls.setTable(-3);
		ls.push(std::string("frozen"));   ls.push(rows[i].Frozen);       ls.setTable(-3);
		ls.push(std::string("dirty"));    ls.push(rows[i].Dirty);        ls.setTable(-3);
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

static int lPumpUI(CLuaState &ls) // painter.pumpUI() -> true, or false when cancel was requested
{
	if (s_Host && s_Host->pumpUI) s_Host->pumpUI();
	bool cancelled = s_Host && s_Host->cancelRequested && s_Host->cancelRequested();
	ls.push(!cancelled);
	return 1;
}

// ---------------------------------------------------------------------------------------------
// Viewer state (bridge-backed; graceful headless)

static ZPUI::SPaintUIBridge *bridge() { return s_Host ? s_Host->bridge : NULL; }

static int lSetMode(CLuaState &ls) // painter.setMode(0..3)  0=tile 1=color 2=displace 3=prop
{
	double m; if (!argNumber(ls, 1, m)) return retErr(ls, "usage: setMode(0..3)");
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b || !b->selectMode) return retErr(ls, "setMode: viewer only");
	b->selectMode((int)m);
	return retOk(ls);
}

static int lGetMode(CLuaState &ls)
{
	ZPUI::SPaintUIBridge *b = bridge();
	if (!b) return retErr(ls, "getMode: viewer only");
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

// ---------------------------------------------------------------------------------------------
// Recorder API (window lands in M23b; the API is stable from M23a)

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
	"  paintDisplace = __zp_paintDisplace, fillDisplace = __zp_fillDisplace,\n"
	"  setBrushSize = __zp_setBrushSize, setTileGroup = __zp_setTileGroup,\n"
	"  setBrushMask = __zp_setBrushMask,\n"
	"  undo = __zp_undo, redo = __zp_redo, seed = __zp_seed, checkSeams = __zp_checkSeams,\n"
	"  setZoneProp = __zp_setZoneProp, getZoneProp = __zp_getZoneProp,\n"
	"  zones = __zp_zones, save = __zp_save, saveAll = __zp_saveAll,\n"
	"  screenshot = __zp_screenshot, pumpUI = __zp_pumpUI,\n"
	"  setMode = __zp_setMode, getMode = __zp_getMode,\n"
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
	ls->registerFunc("__zp_paintDisplace", lPaintDisplace);
	ls->registerFunc("__zp_fillDisplace", lFillDisplace);
	ls->registerFunc("__zp_setBrushSize", lSetBrushSize);
	ls->registerFunc("__zp_setTileGroup", lSetTileGroup);
	ls->registerFunc("__zp_setBrushMask", lSetBrushMask);
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
	ls->registerFunc("__zp_pumpUI", lPumpUI);
	ls->registerFunc("__zp_setMode", lSetMode);
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

} // namespace ZPSCRIPT

/* end of file */
