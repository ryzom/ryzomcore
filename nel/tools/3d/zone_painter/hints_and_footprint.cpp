/**
 * \file hints_and_footprint.cpp
 * \brief Neighbor-hint appdata codec + footprint mask math + place spec parsing.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * Neighbor hints (NEL3D_APPDATA_PAINTER_NEIGHBOR_HINTS) parse/encode/read-from-scene/
 * write-to-scene and the resolveHintToZone / collectNeighborHints board-session helpers.
 * Footprint mask derivation (exporter-identical, CZoneTemplate::getMask mirror), the
 * rot/mirror transforms, occupancy predicates, and CLI place-spec parsing.
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

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/cmd_args.h>
#include <nel/misc/common.h>
#include <nel/misc/config_file.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/event_server.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/camera.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/dru.h>
#include <nel/3d/event_mouse_listener.h>
#include <nel/3d/font_manager.h>
#include <nel/3d/landscape.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/scene.h>
#include <nel/3d/scene_user.h>
#include <nel/3d/text_context.h>
#include <nel/3d/texture_mem.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/viewport.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_corner_smoother.h>
#include <nel/3d/zone_symmetrisation.h>

#include <nel/ligo/ligo_config.h>
#include <nel/ligo/ligo_error.h>
#include <nel/ligo/zone_template.h>
#include <nel/ligo/zone_region.h>
#include <nel/ligo/zone_bank.h>

#include "../pipeline_max/storage_ole.h"
#include "max_thumbnail.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
#include <process.h>
#define ZP_GETPID _getpid
#else
#include <unistd.h>
#define ZP_GETPID getpid
#endif

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/storage_value.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/control_transform.h"

#include "../pipeline_max_export_common/max_math.h"
#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/max_load.h"
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::NELPATCH;
using namespace MAXMATH;

#include "../pipeline_max_export_common/patch_eval.h"

#include "paint_core.h"
#include "context_display.h"
#include "editor_ui.h"

#include <nel/gui/ctrl_base.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/widget_manager.h>

#include "workspace_discovery.h"
#include "startup_ui.h"
#include "script_api.h"

#include "zp_state.h"

// ---------------------------------------------------------------------------------------------
// Neighbor hints: versioned appdata on the eligible node + embedded-copy fallback.
//
// Format: single string value  v1|dx,dy:basename|dx,dy:basename|...
//   dx,dy  = integer cell offsets relative to the eligible zone's footprint origin
//   basename = file basename without .max
// Unknown future versions: parser ignores the entry (tolerates, does not fail open).

/** Parse appdata payload; returns false if absent/empty/unknown version. */
bool parseNeighborHintsString(const std::string &raw, std::vector<SNeighborHint> &out)
{
	out.clear();
	if (raw.empty()) return false;
	// Split on '|'
	std::vector<std::string> parts;
	{
		std::string::size_type pos = 0;
		while (pos <= raw.size())
		{
			std::string::size_type bar = raw.find('|', pos);
			if (bar == std::string::npos) { parts.push_back(raw.substr(pos)); break; }
			parts.push_back(raw.substr(pos, bar - pos));
			pos = bar + 1;
		}
	}
	if (parts.empty()) return false;
	const std::string &ver = parts[0];
	if (ver != "v1")
	{
		// Unknown future version: ignore the entry (tolerate)
		printf("neighbor-hints: ignore unknown version '%s'\n", ver.c_str());
		return false;
	}
	for (size_t i = 1; i < parts.size(); ++i)
	{
		if (parts[i].empty()) continue;
		// dx,dy:basename
		std::string::size_type colon = parts[i].find(':');
		if (colon == std::string::npos) continue;
		std::string coords = parts[i].substr(0, colon);
		std::string base = parts[i].substr(colon + 1);
		if (base.empty()) continue;
		std::vector<std::string> cf;
		{
			std::string cur;
			for (std::string::size_type ci = 0; ci <= coords.size(); ++ci)
			{
				char c = ci < coords.size() ? coords[ci] : ',';
				if (c == ',') { cf.push_back(cur); cur.clear(); }
				else cur += c;
			}
		}
		if (cf.size() < 2) continue;
		int dx = 0, dy = 0;
		if (!NLMISC::fromString(cf[0], dx)) continue;
		if (!NLMISC::fromString(cf[1], dy)) continue;
		SNeighborHint nh(dx, dy, base);
		// Optional ",r,m" (context transform)
		if (cf.size() >= 3 && !cf[2].empty())
		{
			uint r = 0;
			if (NLMISC::fromString(cf[2], r)) nh.Rot = r & 3;
		}
		if (cf.size() >= 4 && !cf[3].empty())
			nh.Mirror = cf[3] == "1" || NLMISC::toLowerAscii(cf[3]) == "m";
		// Strip accidental .max
		std::string::size_type dot = base.rfind('.');
		if (dot != std::string::npos && NLMISC::toLowerAscii(base.substr(dot)) == ".max")
			base = base.substr(0, dot);
		nh.Basename = base;
		out.push_back(nh);
	}
	return !out.empty();
}

bool neighborHintLess(const SNeighborHint &a, const SNeighborHint &b)
{
	if (a.Dy != b.Dy) return a.Dy < b.Dy;
	if (a.Dx != b.Dx) return a.Dx < b.Dx;
	const std::string al = NLMISC::toLowerAscii(a.Basename), bl = NLMISC::toLowerAscii(b.Basename);
	if (al != bl) return al < bl;
	if (a.Rot != b.Rot) return a.Rot < b.Rot;
	return (int)a.Mirror < (int)b.Mirror;
}

/** Deterministic encode: sort by (dy, dx, basename) then emit v1|... */
std::string encodeNeighborHintsString(std::vector<SNeighborHint> hints)
{
	std::sort(hints.begin(), hints.end(), neighborHintLess);
	// Dedup identical (dx,dy,basename)
	std::vector<SNeighborHint> uniq;
	for (size_t i = 0; i < hints.size(); ++i)
	{
		if (!uniq.empty()
		    && uniq.back().Dx == hints[i].Dx && uniq.back().Dy == hints[i].Dy
		    && NLMISC::toLowerAscii(uniq.back().Basename) == NLMISC::toLowerAscii(hints[i].Basename))
			continue;
		uniq.push_back(hints[i]);
	}
	std::string s = "v1";
	for (size_t i = 0; i < uniq.size(); ++i)
	{
		if (uniq[i].Rot != 0 || uniq[i].Mirror)
			s += NLMISC::toString("|%d,%d,%u,%d:%s", uniq[i].Dx, uniq[i].Dy,
			                      uniq[i].Rot & 3, uniq[i].Mirror ? 1 : 0,
			                      uniq[i].Basename.c_str());
		else
			s += NLMISC::toString("|%d,%d:%s", uniq[i].Dx, uniq[i].Dy, uniq[i].Basename.c_str());
	}
	return s;
}

/** Read appdata neighbor hints from a node's script AppData; false if absent/unusable. */
bool readNeighborHintsFromNode(CNodeImpl *node, std::vector<SNeighborHint> &out,
                                      std::string *rawOut)
{
	out.clear();
	if (rawOut) rawOut->clear();
	if (!node) return false;
	std::string raw;
	if (!APPDATA::getScriptAppData(node, NEL3D_APPDATA_PAINTER_NEIGHBOR_HINTS, raw))
		return false;
	if (rawOut) *rawOut = raw;
	return parseNeighborHintsString(raw, out);
}

/**
 * Shared appdata neighbor-hint read (session open + --dump-neighbor-hints).
 * Scans eligible non-frozen zone nodes first (write target), then any zone node
 * that carries the key (defensive if eligibility order differs after a resave).
 * rawOut: optional raw payload string when found.
 */
bool readNeighborHintsFromScene(CScene &scene, const std::string &fileBasename,
                                       std::vector<SNeighborHint> &out, std::string *rawOut)
{
	out.clear();
	if (rawOut) rawOut->clear();
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	std::vector<bool> eligible;
	computeZoneEligibility(nodes, fileBasename, eligible);
	// (1) eligible non-frozen (same node writeNeighborHintsToScene targets)
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (i < eligible.size() && eligible[i] && !nodes[i].Frozen)
		{
			if (readNeighborHintsFromNode(nodes[i].Node, out, rawOut))
				return true;
		}
	}
	// (2) any zone node (covers edge cases: eligibility drift / frozen write host)
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (readNeighborHintsFromNode(nodes[i].Node, out, rawOut))
			return true;
	}
	return false;
}

/**
 * Write neighbor-hints appdata on the eligible node of `scene` (board-session save only).
 * Shape matches Max setAppData / every other NEL3D_APPDATA_* entry:
 *   AppData chunk 0x2150 on the node (CAnimatable)
 *   entry key = (ScriptClassId 0x04d64858/0x16d1751d, SuperClassId 4128, subId)
 *   value = StorageRaw null-terminated string
 * Deterministic encode (same neighbor set → byte-identical resave).
 */
bool writeNeighborHintsToScene(CScene &scene, const std::string &fileBasename,
                                      const std::vector<SNeighborHint> &hints)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	std::vector<bool> eligible;
	computeZoneEligibility(nodes, fileBasename, eligible);
	CNodeImpl *target = NULL;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (i < eligible.size() && eligible[i] && !nodes[i].Frozen)
		{
			target = nodes[i].Node;
			break;
		}
	}
	if (!target)
	{
		fprintf(stderr, "WARNING: neighbor-hints write: no eligible node in '%s'\n",
		        fileBasename.c_str());
		return false;
	}
	// Authoring accessor creates AppData container when absent
	STORAGE::CAppData *ad = target->appData();
	if (!ad)
	{
		fprintf(stderr, "WARNING: neighbor-hints write: appData() failed on '%s'\n",
		        fileBasename.c_str());
		return false;
	}
	const std::string payload = encodeNeighborHintsString(hints);
	if (!ad->setScriptString(NEL3D_APPDATA_PAINTER_NEIGHBOR_HINTS, payload))
	{
		fprintf(stderr, "WARNING: neighbor-hints write: setScriptString failed\n");
		return false;
	}
	printf("neighbor-hints write '%s': %s\n", fileBasename.c_str(), payload.c_str());
	return true;
}

/**
 * Collect current RO context files as hints for the given editable (board save).
 * Offsets = SContextFile::CellX/Y recorded at load / place-context (fine cells relative
 * to the primary footprint origin). These are the same offsets placement consumes, so
 * write → reopen round-trips: for freestanding bricks they are board cells; for
 * converted absolute-authored bricks they equal authored origin deltas.
 */
void collectHintsFromLoadedContext(std::vector<SNeighborHint> &out)
{
	out.clear();
	for (size_t i = 0; i < g_ContextFiles.size(); ++i)
	{
		const SContextFile &cf = g_ContextFiles[i];
		if (cf.Basename.empty()) continue;
		SNeighborHint nh(cf.CellX, cf.CellY, cf.Basename);
		nh.Rot = cf.Rot;
		nh.Mirror = cf.Mirror;
		out.push_back(nh);
	}
}

// Forward: defined with multi-file save helpers
PIPELINE::MAX::CScene *editableScene(const SEditableFileInfo &efi);

/** Board-session only: stamp neighbor hints onto every editable scene before save. */
// Hint stamping is separable from board authority: byte-pure session saves
// (--no-hint-stamp) and synthesized direct-file sessions (never stamp arbitrary
// pipeline files' appdata) keep the board WITHOUT the save-side Scene mutation.
// g_HintStampEnabled is defined in main.cpp (extern in zp_state.h).

void writeNeighborHintsIfBoardSession()
{
	if (!g_BoardSession || !g_HintStampEnabled) return;
	std::vector<SNeighborHint> baseHints;
	collectHintsFromLoadedContext(baseHints);
	// Spec: record every loaded read-only neighbor file (g_ContextFiles). Offsets are
	// PER-FILE (relative to each editable's own board cell; zero for continents/primary),
	// and eco saves also record the OTHER open files as hints: a zone reopens with the
	// full working set it was painted against, loaded read-only. Continents skip that
	// (the grid names already derive neighbors).
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (!g_EditableFiles[i].Editable) continue;
		PIPELINE::MAX::CScene *sc = editableScene(g_EditableFiles[i]);
		if (!sc) continue;
		const int selfX = g_EditableFiles[i].CellX;
		const int selfY = g_EditableFiles[i].CellY;
		std::vector<SNeighborHint> hints;
		for (size_t h = 0; h < baseHints.size(); ++h)
		{
			SNeighborHint nh = baseHints[h];
			nh.Dx -= selfX;
			nh.Dy -= selfY;
			hints.push_back(nh);
		}
		if (g_StartupWorld.Kind == ZPWS::Ecosystem)
		{
			for (size_t j = 0; j < g_EditableFiles.size(); ++j)
			{
				if (j == i) continue;
				hints.push_back(SNeighborHint(g_EditableFiles[j].CellX - selfX,
				                              g_EditableFiles[j].CellY - selfY,
				                              g_EditableFiles[j].Basename));
			}
		}
		writeNeighborHintsToScene(*sc, g_EditableFiles[i].Basename, hints);
	}
}

/**
 * Unsnapped AABB min of patch geometry after object TM. Placement still floors via
 * zoneNodeAuthoredFootprintOrigin / computeFootprintRect; hint offsets quantize raw deltas.
 */
bool zoneNodeAuthoredFootprintRawMin(CNodeImpl *node, SNodeTMCache &tmCache,
                                            float &minX, float &minY)
{
	minX = minY = 0.f;
	if (!node) return false;
	SEvalPatch ep;
	std::string err;
	if (!evalNodePatch(node, ep, err)) return false;
	Matrix3M objectTM = getObjectTM(node, tmCache);
	std::vector<NL3D::CPatchInfo> patches;
	if (!buildPatchInfo(ep, objectTM, 0, patches, err) || patches.empty()) return false;
	NLMISC::CAABBox bbox;
	bool init = false;
	for (size_t p = 0; p < patches.size(); ++p)
	{
		const NL3D::CBezierPatch &bp = patches[p].Patch;
		for (uint v = 0; v < 4; ++v)
		{
			if (!init) { bbox.setCenter(bp.Vertices[v]); bbox.setHalfSize(NLMISC::CVector::Null); init = true; }
			else bbox.extend(bp.Vertices[v]);
		}
		for (uint v = 0; v < 8; ++v) bbox.extend(bp.Tangents[v]);
		for (uint v = 0; v < 4; ++v) bbox.extend(bp.Interiors[v]);
	}
	if (!init) return false;
	const NLMISC::CVector mn = bbox.getMin();
	minX = mn.x;
	minY = mn.y;
	return true;
}

/**
 * Authored footprint origin (snapped AABB min of patch geometry after object TM) for one
 * zone node. Same reference computeFootprintRect uses for placement. False if eval fails.
 */
bool zoneNodeAuthoredFootprintOrigin(CNodeImpl *node, SNodeTMCache &tmCache,
                                            float cellSize, float &originX, float &originY)
{
	originX = originY = 0.f;
	if (cellSize <= 0.f) cellSize = 160.f;
	float minX = 0.f, minY = 0.f;
	if (!zoneNodeAuthoredFootprintRawMin(node, tmCache, minX, minY)) return false;
	originX = (float)(std::floor((double)minX / (double)cellSize) * (double)cellSize);
	originY = (float)(std::floor((double)minY / (double)cellSize) * (double)cellSize);
	return true;
}

/**
 * Extract neighbor hints from embedded frozen/ineligible zone nodes in a scene.
 *
 * Offsets are fine-cell deltas of authored footprint AABB mins relative to the eligible
 * zone. Flooring each min independently before subtracting collapses multi-cell /
 * spillover neighbors whose mins land in the same floor cell (e.g. 201_DY and 201_DZ
 * both (0,-1) on 200_dz). Use lround(rawΔ / cellSize) so sub-cell separation is
 * preserved; v1 integer fine-cell format unchanged.
 *
 * Placement still uses floor-snapped origins (computeFootprintRect). When the primary
 * scene still carries the named embedded copy, freestanding neighbors are placed by
 * matching that copy's floor origin so weld acceptance stays at island quality even
 * when recorded (dx,dy) differ from floor-only keys.
 *
 * Falls back to object-TM deltas when geometry eval fails. Basenames keep the authored
 * node name (e.g. "199_DY") for resolveHintToZone. Continent name-grid deltas are NOT
 * used for placement offsets.
 */
bool extractEmbeddedNeighborHints(CScene &scene, const std::string &fileBasename,
                                         float cellSize, std::vector<SNeighborHint> &out)
{
	out.clear();
	if (cellSize <= 0.f) cellSize = 160.f;
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	if (nodes.empty()) return false;
	std::vector<bool> eligible;
	computeZoneEligibility(nodes, fileBasename, eligible);

	// Eligible anchor: raw AABB min (geometry), TM fallback
	SNodeTMCache tmCache;
	float rawOx = 0.f, rawOy = 0.f;
	bool haveOrig = false;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (!(i < eligible.size() && eligible[i] && !nodes[i].Frozen))
			continue;
		if (zoneNodeAuthoredFootprintRawMin(nodes[i].Node, tmCache, rawOx, rawOy))
			haveOrig = true;
		else
		{
			Matrix3M tm = getObjectTM(nodes[i].Node, tmCache);
			rawOx = tm.m[3][0];
			rawOy = tm.m[3][1];
			haveOrig = true;
		}
		break;
	}
	if (!haveOrig)
	{
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (nodes[i].Frozen) continue;
			if (zoneNodeAuthoredFootprintRawMin(nodes[i].Node, tmCache, rawOx, rawOy))
				haveOrig = true;
			else
			{
				Matrix3M tm = getObjectTM(nodes[i].Node, tmCache);
				rawOx = tm.m[3][0];
				rawOy = tm.m[3][1];
				haveOrig = true;
			}
			break;
		}
	}
	if (!haveOrig) return false;

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const bool isEligible = (i < eligible.size() && eligible[i]);
		if (isEligible && !nodes[i].Frozen) continue; // self
		std::string name = ucstring(nodes[i].Node->userName()).toUtf8();
		if (name.empty()) continue;

		float rawNx = 0.f, rawNy = 0.f;
		int dx = 0, dy = 0;
		if (zoneNodeAuthoredFootprintRawMin(nodes[i].Node, tmCache, rawNx, rawNy))
		{
			// Delta-then-round (not floor-each-then-subtract)
			dx = (int)std::lround((double)(rawNx - rawOx) / (double)cellSize);
			dy = (int)std::lround((double)(rawNy - rawOy) / (double)cellSize);
		}
		else
		{
			Matrix3M tm = getObjectTM(nodes[i].Node, tmCache);
			dx = (int)std::lround((double)(tm.m[3][0] - rawOx) / (double)cellSize);
			dy = (int)std::lround((double)(tm.m[3][1] - rawOy) / (double)cellSize);
		}
		if (dx == 0 && dy == 0) continue;
		out.push_back(SNeighborHint(dx, dy, name));
	}
	return !out.empty();
}

/** Resolve a hint basename to a real .max in the world (exact, case-insens, suffix match). */
bool resolveHintToZone(const ZPWS::SWorldEntry &world, const std::string &hintBase,
                              ZPWS::SZoneEntry &out)
{
	if (world.MaxDir.empty() || hintBase.empty()) return false;
	static std::vector<ZPWS::SZoneEntry> s_listed;
	static std::string s_dir;
	if (s_dir != world.MaxDir)
	{
		ZPWS::listZones(world, s_listed);
		s_dir = world.MaxDir;
	}
	const std::string want = NLMISC::toLowerAscii(hintBase);
	// Two passes: on a miss, re-list once and retry. A brick saved DURING the session
	// (Save As into the world dir, then hinted/placed) is invisible to the first listing
	// (same fix class as findWorldZone's re-list-on-miss).
	for (int attempt = 0; attempt < 2; ++attempt)
	{
		if (attempt == 1)
			ZPWS::listZones(world, s_listed);
		// 1) exact basename
		for (size_t i = 0; i < s_listed.size(); ++i)
		{
			if (NLMISC::toLowerAscii(s_listed[i].Basename) == want)
			{
				out = s_listed[i];
				return true;
			}
		}
		// 2) basename ends with -hint or _hint token (zonematerial-converted-199_dy vs
		//    199_DY); also ends-with the hint after a final dash
		for (size_t i = 0; i < s_listed.size(); ++i)
		{
			const std::string b = NLMISC::toLowerAscii(s_listed[i].Basename);
			if (b.size() >= want.size()
			    && b.compare(b.size() - want.size(), want.size(), want) == 0)
			{
				// require boundary: start or non-alnum before match
				if (b.size() == want.size()
				    || b[b.size() - want.size() - 1] == '-'
				    || b[b.size() - want.size() - 1] == '_')
				{
					out = s_listed[i];
					return true;
				}
			}
		}
		// 3) continent grid: parse hint as row_col and find by coords
		{
			int row = 0, col = 0;
			if (ZPWS::parseContinentZoneName(hintBase, row, col))
			{
				for (size_t i = 0; i < s_listed.size(); ++i)
				{
					int r = 0, c = 0;
					if (ZPWS::parseContinentZoneName(s_listed[i].Basename, r, c)
					    && r == row && c == col)
					{
						out = s_listed[i];
						return true;
					}
				}
			}
		}
	}
	return false;
}

/**
 * Neighbor suggestion priority:
 *   (1) appdata neighbor hints on the eligible node
 *   (2) names + cell offsets from embedded frozen/ineligible copies
 *   (3) continent grid-name 8-ring (empty for ecosystems)
 * Fills out with resolved real files; reports unresolved and skips them.
 * sourceOut: "appdata" | "embedded" | "grid" | "none"
 */
void collectNeighborHints(CScene &scene, const std::string &fileBasename,
                                 const ZPWS::SWorldEntry &world, float cellSize,
                                 std::vector<SNeighborHint> &hintsOut,
                                 std::vector<ZPWS::SZoneEntry> &resolvedOut,
                                 std::string &sourceOut)
{
	hintsOut.clear();
	resolvedOut.clear();
	sourceOut = "none";

	// (1) appdata: same helper as --dump-neighbor-hints
	if (readNeighborHintsFromScene(scene, fileBasename, hintsOut))
		sourceOut = "appdata";
	// (2) embedded copies
	if (sourceOut == "none")
	{
		if (extractEmbeddedNeighborHints(scene, fileBasename, cellSize, hintsOut))
			sourceOut = "embedded";
	}
	// (3) continent grid
	if (sourceOut == "none" && world.Kind == ZPWS::Continent)
	{
		ZPWS::SZoneEntry center;
		center.Basename = fileBasename;
		// Path not required for listContinentNeighbors (uses Basename + MaxDir index)
		std::vector<ZPWS::SZoneEntry> neigh;
		ZPWS::listContinentNeighbors(world, center, neigh);
		int crow = 0, ccol = 0;
		const bool haveC = ZPWS::parseContinentZoneName(fileBasename, crow, ccol);
		for (size_t i = 0; i < neigh.size(); ++i)
		{
			int nr = 0, nc = 0;
			int dx = 0, dy = 0;
			if (haveC && ZPWS::parseContinentZoneName(neigh[i].Basename, nr, nc))
			{
				// zone names: row is Y, col is X → dx=col delta, dy=row delta
				dx = nc - ccol;
				dy = nr - crow;
			}
			hintsOut.push_back(SNeighborHint(dx, dy, neigh[i].Basename));
			resolvedOut.push_back(neigh[i]);
		}
		if (!hintsOut.empty())
			sourceOut = "grid";
		return; // already resolved
	}

	// Resolve basenames for appdata/embedded sources
	for (size_t i = 0; i < hintsOut.size(); ++i)
	{
		ZPWS::SZoneEntry ze;
		if (!resolveHintToZone(world, hintsOut[i].Basename, ze))
		{
			fprintf(stderr, "WARNING: neighbor hint unresolved (skipped): %d,%d:%s\n",
			        hintsOut[i].Dx, hintsOut[i].Dy, hintsOut[i].Basename.c_str());
			continue;
		}
		// Prefer the real file basename in the stored hint for later write-back
		hintsOut[i].Basename = ze.Basename;
		resolvedOut.push_back(ze);
	}
	printf("neighbor-hints: source=%s raw=%u resolved=%u\n",
	       sourceOut.c_str(), (uint)hintsOut.size(), (uint)resolvedOut.size());
}


/** Normalize every spec's Basename to the RESOLVED zone basename:
 *  resolveHintToZone accepts short/suffix names ("199_DY" → zonematerial-converted-199_dy)
 *  but every later basename-keyed correlation (the loadOnePlaceContext mask stash-back,
 *  the spec-vs-hint skip lists, contextBasenameHasSpec/hintContextConflicts) compares
 *  against SContextFile.Basename, which stores the resolved name. An unresolved short
 *  name kept the provisional 1x1 mask forever, made the brick self-collide on rotate/
 *  drag, and let a saved hint reload a duplicate copy. Call before ANY skip-list build. */
void normalizePlaceContextSpecBasenames()
{
	if (g_StartupWorld.MaxDir.empty())
		return;
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
	{
		ZPWS::SZoneEntry ze;
		if (resolveHintToZone(g_StartupWorld, g_PlaceContextSpecs[i].Basename, ze)
		    && NLMISC::toLowerAscii(g_PlaceContextSpecs[i].Basename)
		       != NLMISC::toLowerAscii(ze.Basename))
		{
			printf("place-context: spec '%s' resolved to '%s'\n",
			       g_PlaceContextSpecs[i].Basename.c_str(), ze.Basename.c_str());
			g_PlaceContextSpecs[i].Basename = ze.Basename;
		}
	}
}

/** Parse "dx,dy[,rot][,m]": cell offsets, rot 0..3, optional mirror (m|1|true). */
bool parsePlaceSpec(const std::string &sIn, SInstancePlace &out, std::string &err)
{
	out = SInstancePlace();
	// Optional ":basename" suffix: instance of an OPEN brick instead of home
	std::string s = sIn;
	std::string::size_type colon = s.find(':');
	if (colon != std::string::npos)
	{
		out.SourceBasename = s.substr(colon + 1);
		std::string::size_type dot = out.SourceBasename.rfind('.');
		if (dot != std::string::npos
		    && NLMISC::toLowerAscii(out.SourceBasename.substr(dot)) == ".max")
			out.SourceBasename = out.SourceBasename.substr(0, dot);
		s = s.substr(0, colon);
	}
	std::vector<std::string> parts;
	{
		std::string cur;
		for (size_t i = 0; i <= s.size(); ++i)
		{
			char c = (i < s.size()) ? s[i] : ',';
			if (c == ',') { parts.push_back(cur); cur.clear(); }
			else cur += c;
		}
	}
	if (parts.size() < 2 || parts.size() > 4)
	{
		err = "place expects dx,dy[,rot][,m], got '" + s + "'";
		return false;
	}
	if (!NLMISC::fromString(parts[0], out.CellX) || !NLMISC::fromString(parts[1], out.CellY))
	{
		err = "place dx,dy unparseable: '" + s + "'";
		return false;
	}
	if (out.CellX == 0 && out.CellY == 0)
	{
		err = "place 0,0 is the primary (omit it); got '" + s + "'";
		return false;
	}
	if (parts.size() >= 3 && !parts[2].empty())
	{
		if (!NLMISC::fromString(parts[2], out.Rot) || out.Rot > 3)
		{
			err = "place rot must be 0..3, got '" + parts[2] + "'";
			return false;
		}
	}
	if (parts.size() >= 4 && !parts[3].empty())
	{
		std::string m = NLMISC::toLowerAscii(parts[3]);
		if (m == "1" || m == "m" || m == "true" || m == "yes" || m == "flip")
			out.Mirror = true;
		else if (m == "0" || m == "false" || m == "no")
			out.Mirror = false;
		else
		{
			err = "place mirror expects 0|1|m, got '" + parts[3] + "'";
			return false;
		}
	}
	return true;
}

/**
 * Parse "NxM" layout into translation-only places (non-origin cells).
 * Cell coordinates are fine-cell origins: slot (cx,cy) → origin (cx*fw, cy*fh) so
 * adjacent footprint slots abut. fw/fh default 1 when footprint not yet known.
 */
bool parseInstanceLayout(const std::string &s, uint &cols, uint &rows,
                                std::vector<SInstancePlace> &places, std::string &err,
                                int fw, int fh)
{
	cols = 1;
	rows = 1;
	places.clear();
	std::string t = NLMISC::toLowerAscii(s);
	std::string::size_type x = t.find('x');
	if (x == std::string::npos || x == 0 || x + 1 >= t.size())
	{
		err = "instances layout expects NxM (e.g. 2x2), got '" + s + "'";
		return false;
	}
	if (!NLMISC::fromString(t.substr(0, x), cols) || !NLMISC::fromString(t.substr(x + 1), rows)
	    || cols < 1 || rows < 1 || cols > 8 || rows > 8)
	{
		err = "instances layout out of range or unparseable: '" + s + "' (use 1..8 per axis)";
		return false;
	}
	const bool ok = (cols == 1 && rows == 1)
		|| (cols == 2 && rows == 1)
		|| (cols == 1 && rows == 2)
		|| (cols == 2 && rows == 2)
		|| (cols == 3 && rows == 3);
	if (!ok)
	{
		err = "unsupported instances layout '" + s + "' (supported: 1x1, 2x1, 1x2, 2x2, 3x3)";
		return false;
	}
	if (fw < 1) fw = (g_FootprintCellsW > 0) ? g_FootprintCellsW : 1;
	if (fh < 1) fh = (g_FootprintCellsH > 0) ? g_FootprintCellsH : 1;
	for (uint cy = 0; cy < rows; ++cy)
	for (uint cx = 0; cx < cols; ++cx)
	{
		if (cx == 0 && cy == 0) continue;
		places.push_back(SInstancePlace((int)cx * fw, (int)cy * fh, 0, false));
	}
	return true;
}

/** CLI --place specs → g_Places (clears layout state first). Both the workspace startup
 *  branch and the synthetic-session direct .max open route through here.
 *  Returns false (with the error printed) on a bad spec. */
bool parseCliPlaces(NLMISC::CCmdArgs &args)
{
	g_Places.clear();
	g_InstanceCols = 1;
	g_InstanceRows = 1;
	g_InstanceCount = 1;
	if (args.haveLongArg("place"))
	{
		const std::vector<std::string> &pv = args.getLongArg("place");
		for (size_t i = 0; i < pv.size(); ++i)
		{
			SInstancePlace pl;
			std::string perr;
			if (!parsePlaceSpec(pv[i], pl, perr))
			{
				fprintf(stderr, "ERROR: %s\n", perr.c_str());
				return false;
			}
			g_Places.push_back(pl);
		}
	}
	return true;
}

/**
 * Footprint rect: AABB of primary zones, origin snapped DOWN to the cell grid,
 * size ceil'd UP to whole cells. Empty primary => 1×1 cell at (0,0).
 *
 * Pivot for rot/mirror is the FOOTPRINT block center (origin + half step), not the raw
 * geometry AABB center, so rotation/mirror map grid cells to grid cells by construction
 * for any integer W×H (square and non-square). Mask-accurate L-shapes are later.
 */
void computeFootprintRect(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                                 size_t primaryEnd, float cellSize,
                                 float &originX, float &originY,
                                 float &stepX, float &stepY,
                                 int &cellsW, int &cellsH)
{
	if (cellSize <= 0.f) cellSize = 100.f;
	NLMISC::CAABBox bbox;
	bool init = false;
	for (size_t i = primaryBegin; i < primaryEnd && i < zones.size(); ++i)
	{
		for (size_t p = 0; p < zones[i].Patches.size(); ++p)
		{
			const NL3D::CBezierPatch &bp = zones[i].Patches[p].Patch;
			for (uint v = 0; v < 4; ++v)
			{
				if (!init) { bbox.setCenter(bp.Vertices[v]); bbox.setHalfSize(NLMISC::CVector::Null); init = true; }
				else bbox.extend(bp.Vertices[v]);
			}
			for (uint v = 0; v < 8; ++v) bbox.extend(bp.Tangents[v]);
			for (uint v = 0; v < 4; ++v) bbox.extend(bp.Interiors[v]);
		}
	}
	if (!init)
	{
		originX = originY = 0.f;
		stepX = stepY = cellSize;
		cellsW = cellsH = 1;
		return;
	}
	const NLMISC::CVector mn = bbox.getMin();
	const NLMISC::CVector mx = bbox.getMax();
	// Snap origin down to cell grid so the footprint rect is a clean cell block
	originX = (float)(std::floor((double)mn.x / (double)cellSize) * (double)cellSize);
	originY = (float)(std::floor((double)mn.y / (double)cellSize) * (double)cellSize);
	const float extX = mx.x - originX;
	const float extY = mx.y - originY;
	cellsW = (int)std::ceil((double)extX / (double)cellSize);
	cellsH = (int)std::ceil((double)extY / (double)cellSize);
	if (cellsW < 1) cellsW = 1;
	if (cellsH < 1) cellsH = 1;
	stepX = (float)cellsW * cellSize;
	stepY = (float)cellsH * cellSize;
}

/** Footprint step only (compat helper). */
void computeFootprintStep(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                                 size_t primaryEnd, float cellSize, float &stepX, float &stepY)
{
	float ox = 0.f, oy = 0.f;
	int cw = 1, ch = 1;
	computeFootprintRect(zones, primaryBegin, primaryEnd, cellSize, ox, oy, stepX, stepY, cw, ch);
}

// ---------------------------------------------------------------------------------------------
// Footprint masks: exporter-identical derivation
// (pipeline_max_export_zone/main.cpp:175-234 buildZoneMask / buildSquareMask)
//
// Reuses NLLIGO::CZoneTemplate::build + getMask and the getSquareMask AABB fallback. The
// exporter helpers are static locals in that TU; mirror them here (same inputs: open edges,
// objectTM, symmetry, USE_BOUNDINGBOX appdata). No pipeline_max source edits; patch_eval.h
// stays untouchable.

/** Open-edge zone template mask (export buildZoneMask @ main.cpp:175). */
bool buildZoneMaskFromEval(const SEvalPatch &ep, const Matrix3M &objectTM, bool symmetry,
                                  const NLLIGO::CLigoConfig &config,
                                  std::vector<bool> &mask, uint &width, uint &height,
                                  std::string &err)
{
	std::vector<NLMISC::CVector> vertices(ep.Pm.Verts.size());
	for (size_t i = 0; i < ep.Pm.Verts.size(); ++i)
	{
		Point3M v = { ep.Pm.Verts[i].Pos[0], ep.Pm.Verts[i].Pos[1], ep.Pm.Verts[i].Pos[2] };
		v = transformPoint(v, objectTM);
		vertices[i].x = v.x;
		vertices[i].y = v.y;
		vertices[i].z = v.z;
	}
	std::vector<std::pair<uint, uint> > indexes;
	for (size_t e = 0; e < ep.Pm.Edges.size(); ++e)
	{
		const SPmEdge &edge = ep.Pm.Edges[e];
		if (edge.Patches.size() < 2)
		{
			if (symmetry)
				indexes.push_back(std::pair<uint, uint>((uint)edge.V2, (uint)edge.V1));
			else
				indexes.push_back(std::pair<uint, uint>((uint)edge.V1, (uint)edge.V2));
		}
	}
	NLLIGO::CZoneTemplate zoneTemplate;
	NLLIGO::CLigoError errors;
	if (!zoneTemplate.build(vertices, indexes, config, errors))
	{
		err = NLMISC::toString("zone template build failed (main error %d)", (int)errors.MainError);
		return false;
	}
	zoneTemplate.getMask(mask, width, height);
	return true;
}

/** getSquareMask replication (export buildSquareMask @ main.cpp:215): AABB, all cells true. */
void buildSquareMaskFromPatches(const std::vector<NL3D::CPatchInfo> &patchinfo,
                                       float cellSize, std::vector<bool> &mask,
                                       uint &width, uint &height)
{
	sint maxX = 1;
	sint maxY = 1;
	for (size_t i = 0; i < patchinfo.size(); ++i)
	{
		for (uint v = 0; v < 4; ++v)
		{
			sint positionX = (sint)((patchinfo[i].Patch.Vertices[v].x + cellSize / 2) / cellSize);
			sint positionY = (sint)((patchinfo[i].Patch.Vertices[v].y + cellSize / 2) / cellSize);
			if (positionX > maxX) maxX = positionX;
			if (positionY > maxY) maxY = positionY;
		}
	}
	width = (uint)maxX;
	height = (uint)maxY;
	if (width < 1) width = 1;
	if (height < 1) height = 1;
	mask.clear();
	mask.resize(width * height, true);
}

bool maskHasHole(const std::vector<bool> &mask)
{
	for (size_t i = 0; i < mask.size(); ++i)
		if (!mask[i]) return true;
	return false;
}

std::string maskToTFString(const std::vector<bool> &mask, int w, int h)
{
	std::string s;
	s.reserve((size_t)(w * h + h));
	for (int y = 0; y < h; ++y)
	{
		if (y) s += '/';
		for (int x = 0; x < w; ++x)
			s += (x + y * w < (int)mask.size() && mask[(size_t)(x + y * w)]) ? 'T' : 'F';
	}
	return s;
}

/**
 * Derive exporter-identical footprint for one zone (open-edge CZoneTemplate mask, or
 * AABB square / USE_BOUNDINGBOX). This is the authoring source of truth: same inputs
 * ligo export uses when writing a .ligozone; the painter does not read .ligozone.
 * Honors NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX; degenerate template → AABB + log.
 */
bool deriveZoneFootprintMask(const SPaintZone &pz, float cellSize, float snap,
                                    std::vector<bool> &mask, int &cellsW, int &cellsH,
                                    float &originX, float &originY, bool &fromTemplate,
                                    std::string &err)
{
	mask.clear();
	cellsW = cellsH = 1;
	originX = originY = 0.f;
	fromTemplate = false;
	if (cellSize <= 0.f) cellSize = 160.f;
	if (snap <= 0.f) snap = 1.f;

	// AABB origin always (geometry placement + board origin reference)
	{
		std::vector<SPaintZone> one;
		one.push_back(pz);
		float sx = 0.f, sy = 0.f;
		int aw = 1, ah = 1;
		computeFootprintRect(one, 0, 1, cellSize, originX, originY, sx, sy, aw, ah);
	}

	if (!pz.Node)
	{
		uint w = 0, h = 0;
		buildSquareMaskFromPatches(pz.Patches, cellSize, mask, w, h);
		cellsW = (int)w;
		cellsH = (int)h;
		err = "no node: square mask";
		return true;
	}

	NLLIGO::CLigoConfig config;
	config.CellSize = cellSize;
	config.Snap = snap;
	config.ZoneSnapShotRes = 128;

	const bool useBB =
		APPDATA::getScriptAppDataInt(pz.Node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
	const bool symmetry =
		APPDATA::getScriptAppDataInt(pz.Node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;

	SNodeTMCache tmCache;
	Matrix3M objectTM = getObjectTM(pz.Node, tmCache);

	uint w = 0, h = 0;
	if (!useBB)
	{
		std::string terr;
		if (buildZoneMaskFromEval(pz.Ep, objectTM, symmetry, config, mask, w, h, terr)
		    && w >= 1 && h >= 1 && mask.size() == (size_t)w * (size_t)h)
		{
			cellsW = (int)w;
			cellsH = (int)h;
			fromTemplate = true;
			return true;
		}
		if (terr.empty()) terr = "empty/degenerate mask";
		err = terr;
		printf("footprint: template mask failed for '%s' (%s): AABB square fallback\n",
		       pz.Name.c_str(), err.c_str());
	}

	buildSquareMaskFromPatches(pz.Patches, cellSize, mask, w, h);
	cellsW = (int)w;
	cellsH = (int)h;
	fromTemplate = false;

	// Align square-mask absolute indexing with snapped AABB when they disagree
	{
		float sx = 0.f, sy = 0.f, ox = 0.f, oy = 0.f;
		int aw = 1, ah = 1;
		std::vector<SPaintZone> one;
		one.push_back(pz);
		computeFootprintRect(one, 0, 1, cellSize, ox, oy, sx, sy, aw, ah);
		if (aw > 0 && ah > 0 && (aw != cellsW || ah != cellsH)
		    && (size_t)aw * (size_t)ah <= (size_t)cellsW * (size_t)cellsH)
		{
			cellsW = aw;
			cellsH = ah;
			mask.assign((size_t)cellsW * (size_t)cellsH, true);
		}
		originX = ox;
		originY = oy;
	}
	return true;
}


/**
 * Derive primary footprint for board occupancy from the file's single editable
 * paint zone (by design: one editable zone per normal .max). First non-frozen
 * in [begin,end) under eligibility. Mask algorithm is exporter-identical
 * (deriveZoneFootprintMask); .ligozone is never read here.
 * Fills g_Footprint* globals.
 */
void derivePrimaryFootprint(const std::vector<SPaintZone> &zones, size_t begin, size_t end,
                                   float cellSize, float snap)
{
	g_FootprintMask.clear();
	g_FootprintCellsW = 1;
	g_FootprintCellsH = 1;
	g_FootprintOriginX = 0.f;
	g_FootprintOriginY = 0.f;
	g_FootprintFromTemplate = false;

	size_t pick = end;
	for (size_t i = begin; i < end && i < zones.size(); ++i)
	{
		if (!zones[i].Frozen) { pick = i; break; }
	}
	if (pick >= end || pick >= zones.size())
	{
		for (size_t i = begin; i < end && i < zones.size(); ++i) { pick = i; break; }
	}
	if (pick >= zones.size() || pick >= end)
	{
		float sx = 0.f, sy = 0.f;
		computeFootprintRect(zones, begin, end, cellSize,
		                     g_FootprintOriginX, g_FootprintOriginY, sx, sy,
		                     g_FootprintCellsW, g_FootprintCellsH);
		g_FootprintMask.assign((size_t)g_FootprintCellsW * (size_t)g_FootprintCellsH, true);
		printf("footprint: %dx%d cells (AABB, no zone) origin (%.1f, %.1f) mask=filled\n",
		       g_FootprintCellsW, g_FootprintCellsH, g_FootprintOriginX, g_FootprintOriginY);
		return;
	}

	std::string err;
	bool fromT = false;
	int cw = 1, ch = 1;
	float ox = 0.f, oy = 0.f;
	std::vector<bool> mask;
	deriveZoneFootprintMask(zones[pick], cellSize, snap, mask, cw, ch, ox, oy, fromT, err);
	g_FootprintMask = mask;
	g_FootprintCellsW = cw;
	g_FootprintCellsH = ch;
	g_FootprintOriginX = ox;
	g_FootprintOriginY = oy;
	g_FootprintFromTemplate = fromT;
	if (g_FootprintMask.empty())
		g_FootprintMask.assign((size_t)cw * (size_t)ch, true);

	const bool hole = maskHasHole(g_FootprintMask);
	const float sx = (float)cw * cellSize;
	const float sy = (float)ch * cellSize;
	printf("footprint: %dx%d cells  step (%.1f, %.1f)  origin (%.1f, %.1f)  pivot (%.1f, %.1f)  "
	       "source=%s  filled=%s  mask=%s\n",
	       cw, ch, sx, sy, ox, oy, ox + sx * 0.5f, oy + sy * 0.5f,
	       fromT ? "template" : "aabb-square",
	       hole ? "no" : "yes",
	       maskToTFString(g_FootprintMask, cw, ch).c_str());

	// One-shot unit checks (even without --place)
	static bool s_UnitChecked = false;
	if (!s_UnitChecked)
	{
		unitCheckFootprintOccupancy();
		s_UnitChecked = true;
	}
}

/**
 * Optional audit: compare the already-derived mask to a pre-existing .ligozone.
 * Test/CI fixture only; authoring never requires or generates .ligozone (that is a
 * ligo *export* build product). Source of truth for board occupancy is always the
 * .max-derived mask above.
 * Prints parity line: size/filled/mask bits.
 */
bool compareFootprintToLigozone(const std::string &ligozonePath)
{
	if (ligozonePath.empty() || !NLMISC::CFile::fileExists(ligozonePath))
	{
		printf("footprint-ligozone: missing %s\n", ligozonePath.c_str());
		return false;
	}
	NLLIGO::CZoneBankElement elm;
	try
	{
		NLMISC::CIFile f;
		if (!f.open(ligozonePath))
		{
			printf("footprint-ligozone: cannot open %s\n", ligozonePath.c_str());
			return false;
		}
		NLMISC::CIXml xml;
		xml.init(f);
		elm.serial(xml);
	}
	catch (const std::exception &e)
	{
		printf("footprint-ligozone: parse failed %s: %s\n", ligozonePath.c_str(), e.what());
		return false;
	}
	catch (...)
	{
		printf("footprint-ligozone: parse failed %s\n", ligozonePath.c_str());
		return false;
	}
	elm.convertSize();
	const int lw = (int)elm.getSizeX();
	const int lh = (int)elm.getSizeY();
	const std::vector<bool> &lm = elm.getMask();
	const int dw = g_FootprintCellsW;
	const int dh = g_FootprintCellsH;
	const std::vector<bool> &dm = g_FootprintMask;
	bool sizeOk = (lw == dw && lh == dh);
	bool bitsOk = sizeOk && lm.size() == dm.size();
	if (bitsOk)
	{
		for (size_t i = 0; i < lm.size(); ++i)
			if (lm[i] != dm[i]) { bitsOk = false; break; }
	}
	const bool lHole = maskHasHole(lm);
	const bool dHole = maskHasHole(dm);
	printf("footprint-ligozone: %s  ref %dx%d filled=%s mask=%s  der %dx%d filled=%s mask=%s  "
	       "size=%s bits=%s\n",
	       NLMISC::CFile::getFilename(ligozonePath).c_str(),
	       lw, lh, lHole ? "no" : "yes", maskToTFString(lm, lw, lh).c_str(),
	       dw, dh, dHole ? "no" : "yes", maskToTFString(dm, dw, dh).c_str(),
	       sizeOk ? "OK" : "DIFF", bitsOk ? "OK" : "DIFF");
	return sizeOk && bitsOk;
}

/**
 * SPiece::rotFlip convention (nel/src/ligo/zone_region.cpp:331): flip mirrors X, then rot
 * 0..3. Transforms an occupancy mask in place; W/H swap on odd rotations.
 */
void maskRotFlip(std::vector<bool> &mask, int &w, int &h, uint rot, bool flip)
{
	if (w < 1) w = 1;
	if (h < 1) h = 1;
	if (mask.size() != (size_t)w * (size_t)h)
		mask.assign((size_t)w * (size_t)h, true);
	NLLIGO::SPiece piece;
	piece.w = w;
	piece.h = h;
	piece.Tab.resize((size_t)w * (size_t)h);
	for (size_t i = 0; i < piece.Tab.size(); ++i)
		piece.Tab[i] = mask[i] ? 1 : 0;
	piece.rotFlip((uint8)(rot & 3), flip ? 1 : 0);
	w = (int)piece.w;
	h = (int)piece.h;
	mask.resize((size_t)w * (size_t)h);
	for (size_t i = 0; i < mask.size(); ++i)
		mask[i] = piece.Tab[i] != 0;
}

/** Occupied block size after rot (mirror alone does not change the axis-aligned cell AABB). */
void footprintBlockSize(int cellsW, int cellsH, uint rot, bool /*mirror*/,
                               int &outW, int &outH)
{
	if (rot & 1) { outW = cellsH; outH = cellsW; }
	else { outW = cellsW; outH = cellsH; }
}

/** Cell occupancy of a transformed mask placed with min-corner at (ox,oy). */
bool maskCellOccupied(const std::vector<bool> &mask, int w, int h,
                             int ox, int oy, uint rot, bool mirror, int cx, int cy)
{
	std::vector<bool> m = mask;
	int mw = w, mh = h;
	if (m.empty()) m.assign((size_t)mw * (size_t)mh, true);
	maskRotFlip(m, mw, mh, rot, mirror);
	const int lx = cx - ox;
	const int ly = cy - oy;
	if (lx < 0 || ly < 0 || lx >= mw || ly >= mh) return false;
	return m[(size_t)(lx + ly * mw)];
}

/** True if two placed masks share any true cell. */
bool masksCollide(const std::vector<bool> &a, int aw, int ah, int aox, int aoy,
                         uint aRot, bool aMir,
                         const std::vector<bool> &b, int bw, int bh, int box, int boy,
                         uint bRot, bool bMir)
{
	std::vector<bool> ma = a, mb = b;
	int maw = aw, mah = ah, mbw = bw, mbh = bh;
	if (ma.empty()) ma.assign((size_t)maw * (size_t)mah, true);
	if (mb.empty()) mb.assign((size_t)mbw * (size_t)mbh, true);
	maskRotFlip(ma, maw, mah, aRot, aMir);
	maskRotFlip(mb, mbw, mbh, bRot, bMir);
	const int x0 = std::max(aox, box);
	const int y0 = std::max(aoy, boy);
	const int x1 = std::min(aox + maw, box + mbw);
	const int y1 = std::min(aoy + mah, boy + mbh);
	for (int y = y0; y < y1; ++y)
	for (int x = x0; x < x1; ++x)
	{
		const int ai = (x - aox) + (y - aoy) * maw;
		const int bi = (x - box) + (y - boy) * mbw;
		if (ma[(size_t)ai] && mb[(size_t)bi]) return true;
	}
	return false;
}

/**
 * After rotating/mirroring a block about its own center, return the new min-corner origin
 * (relative to the old origin) and the new size. Uses continuous center + half-extents so
 * same-parity W×H stay on-grid; different-parity half-cells round with floor (unit-tested).
 */
void footprintBlockAfterTransform(int cellsW, int cellsH, uint rot, bool mirror,
                                         int &originDX, int &originDY, int &outW, int &outH)
{
	// Transform the continuous rect [0,W]×[0,H] about center (W/2,H/2)
	const double W = (double)cellsW, H = (double)cellsH;
	const double cx = W * 0.5, cy = H * 0.5;
	const double corners[4][2] = { { 0, 0 }, { W, 0 }, { W, H }, { 0, H } };
	double minX = 1e300, minY = 1e300, maxX = -1e300, maxY = -1e300;
	for (int i = 0; i < 4; ++i)
	{
		double lx = corners[i][0] - cx;
		double ly = corners[i][1] - cy;
		if (mirror) lx = -lx;
		double rx = lx, ry = ly;
		switch (rot & 3)
		{
		case 0: rx = lx;  ry = ly;  break;
		case 1: rx = -ly; ry = lx;  break;
		case 2: rx = -lx; ry = -ly; break;
		case 3: rx = ly;  ry = -lx; break;
		}
		const double x = rx + cx;
		const double y = ry + cy;
		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (x > maxX) maxX = x;
		if (y > maxY) maxY = y;
	}
	originDX = (int)std::floor(minX + 1e-9);
	originDY = (int)std::floor(minY + 1e-9);
	outW = (int)std::floor(maxX - minX + 0.5);
	outH = (int)std::floor(maxY - minY + 0.5);
	if (outW < 1) outW = 1;
	if (outH < 1) outH = 1;
}

/** Unit-check W≠H block size + SPiece rotFlip mask + interlocking L legality. */
void unitCheckFootprintOccupancy()
{
	int odx = 0, ody = 0, ow = 0, oh = 0;
	footprintBlockAfterTransform(2, 1, 1, false, odx, ody, ow, oh);
	const int w21 = ow, h21 = oh, dx21 = odx, dy21 = ody;
	const bool ok21 = (w21 == 1 && h21 == 2);
	footprintBlockAfterTransform(3, 2, 1, false, odx, ody, ow, oh);
	const int w32 = ow, h32 = oh;
	const bool ok32 = (w32 == 2 && h32 == 3);
	footprintBlockAfterTransform(8, 8, 1, false, odx, ody, ow, oh);
	const bool ok88 = (ow == 8 && oh == 8 && odx == 0 && ody == 0);
	footprintBlockAfterTransform(2, 1, 0, true, odx, ody, ow, oh);
	const bool okM = (ow == 2 && oh == 1);

	// Synthetic L 2×2 {T,T,F,T} (NeL example class) via SPiece::rotFlip
	std::vector<bool> L;
	L.push_back(true); L.push_back(true); L.push_back(false); L.push_back(true);
	int lw = 2, lh = 2;
	maskRotFlip(L, lw, lh, 0, false);
	const bool okL0 = (lw == 2 && lh == 2 && L[0] && L[1] && !L[2] && L[3]);
	std::vector<bool> Lr;
	Lr.push_back(true); Lr.push_back(true); Lr.push_back(false); Lr.push_back(true);
	int lrw = 2, lrh = 2;
	maskRotFlip(Lr, lrw, lrh, 1, false);
	const bool okLR = (lrw == 2 && lrh == 2);

	std::vector<bool> L2;
	L2.push_back(true); L2.push_back(true); L2.push_back(false); L2.push_back(true);
	const bool collSame = masksCollide(L2, 2, 2, 0, 0, 0, false, L2, 2, 2, 0, 0, 0, false);
	std::vector<bool> inv;
	inv.push_back(false); inv.push_back(false); inv.push_back(true); inv.push_back(false);
	const bool collInv = masksCollide(L2, 2, 2, 0, 0, 0, false, inv, 2, 2, 0, 0, 0, false);
	const bool okInter = collSame && !collInv;

	printf("footprint unit-check: 2×1 R1 → %dx%d originΔ (%d,%d) %s; 3×2 R1 → %dx%d %s; "
	       "8×8 R1 origin-stable %s; 2×1 M size-stable %s; "
	       "L-mask R0 %s R1-size %s interlock %s\n",
	       w21, h21, dx21, dy21, ok21 ? "OK" : "FAIL",
	       w32, h32, ok32 ? "OK" : "FAIL",
	       ok88 ? "OK" : "FAIL", okM ? "OK" : "FAIL",
	       okL0 ? "OK" : "FAIL", okLR ? "OK" : "FAIL", okInter ? "OK" : "FAIL");
}

/** XY transform: mirror about Y (X flip, land/maxscript scale[-1,1,1]), then rot 0..3 CCW about pivot, then translate. */
void transformInstanceXY(float &x, float &y, float pivotX, float pivotY,
                                float dx, float dy, uint rot, bool mirror)
{
	float lx = x - pivotX;
	float ly = y - pivotY;
	if (mirror)
		lx = -lx;
	float rx = lx, ry = ly;
	switch (rot & 3)
	{
	case 0: rx = lx;  ry = ly;  break;
	case 1: rx = -ly; ry = lx;  break; // 90° CCW
	case 2: rx = -lx; ry = -ly; break; // 180°
	case 3: rx = ly;  ry = -lx; break; // 270° CCW
	}
	x = rx + pivotX + dx;
	y = ry + pivotY + dy;
}

/**
 * Pivot = footprint block center (origin snapped to cell grid + half step).
 * Matches AABB center for grid-aligned square footprints; for W≠H keeps rot/mirror
 * cell-grid faithful.
 */
void computePrimaryPivot(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                                size_t primaryEnd, float cellSize, float &px, float &py)
{
	float ox = 0.f, oy = 0.f, sx = 0.f, sy = 0.f;
	int cw = 1, ch = 1;
	computeFootprintRect(zones, primaryBegin, primaryEnd, cellSize, ox, oy, sx, sy, cw, ch);
	px = ox + sx * 0.5f;
	py = oy + sy * 0.5f;
}

/**
 * World translation after rot/mirror about pivot so the transformed footprint's min-corner
 * lands at board cell (placeX, placeY); primary footprint origin cell is (0,0).
 */
void computePlaceTranslationFrom(float originX, float originY, float stepX, float stepY,
                                        float pivotX, float pivotY,
                                        float boardOriginX, float boardOriginY, float cellSize,
                                        int placeX, int placeY, uint rot, bool mirror,
                                        float &outDx, float &outDy)
{
	// Transform the four corners of the SOURCE footprint rect about its pivot; the desired
	// min-corner is a BOARD cell (anchored at the primary footprint origin).
	float xs[4] = { originX, originX + stepX, originX + stepX, originX };
	float ys[4] = { originY, originY, originY + stepY, originY + stepY };
	float tminX = 0.f, tminY = 0.f;
	for (int i = 0; i < 4; ++i)
	{
		float x = xs[i], y = ys[i];
		transformInstanceXY(x, y, pivotX, pivotY, 0.f, 0.f, rot, mirror);
		if (i == 0) { tminX = x; tminY = y; }
		else
		{
			if (x < tminX) tminX = x;
			if (y < tminY) tminY = y;
		}
	}
	const float desiredX = boardOriginX + (float)placeX * cellSize;
	const float desiredY = boardOriginY + (float)placeY * cellSize;
	outDx = desiredX - tminX;
	outDy = desiredY - tminY;
}

void computePlaceTranslation(float originX, float originY, float stepX, float stepY,
                                    float pivotX, float pivotY, float cellSize,
                                    int placeX, int placeY, uint rot, bool mirror,
                                    float &outDx, float &outDy)
{
	computePlaceTranslationFrom(originX, originY, stepX, stepY, pivotX, pivotY,
	                            originX, originY, cellSize, placeX, placeY, rot, mirror,
	                            outDx, outDy);
}


