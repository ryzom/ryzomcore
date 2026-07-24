/**
 * \file scene_paint.cpp
 * \brief Paint-zone assembly: eligibility, build, weld, instance placement, dumps.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * Walks the loaded scene, picks the exporter-eligible RklPatch(es) as paint targets,
 * clones display-instance zones for `--place`, welds cross-zone open edges, and lands
 * the built CZones on the landscape. `writeZoneV4` + `dumpZones` sit here too as the
 * headless verification surface.
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

bool writeZoneV4(NL3D::CZone &zone, const std::string &path)
{
	NLMISC::CMemStream mem;
	nlassert(!mem.isReading());
	zone.serial(mem);
	if (mem.length() < 1) return false;
	std::vector<uint8> buf(mem.buffer(), mem.buffer() + mem.length());
	if (buf[0] != 5)
	{
		fprintf(stderr, "WARNING: unexpected CZone serial version %u, version byte left untouched\n", buf[0]);
	}
	else
	{
		buf[0] = 4;
	}
	NLMISC::COFile f;
	if (!f.open(path)) return false;
	f.serialBuffer(nlVectorData(buf), (uint)buf.size());
	return true;
}

// ---------------------------------------------------------------------------------------------
// The painting scene zones: every RklPatch node, evaluated in authored space, zoneId = node
// collection index (the plugin used its vectMesh index the same way).

// Exporter-faithful zone eligibility.
//
// Design: ONE editable paint zone per .max for normal authoring (material / special /
// continent brick). Extra RklPatches in the file (embedded neighbor copies, scratch
// leftovers, [NELLIGO] markers) are display/context only, never simultaneous paint
// targets. That matches the export product (one .zone per protocol brick) and is why
// board footprint also derives from that single eligible zone.
//
// Mirrors pipeline_max_export_zone + ligo/zone maxscript selection:
//   - collectZoneNodes skips [NELLIGO] debug markers (shared patch_eval rule).
//   - zonematerial- / zonespecial-: export requires exactly one. If multiple non-frozen,
//     prefer node name matching the cell token (case-insensitive); otherwise first
//     non-frozen is eligible, rest RO.
//   - zonetransition-: exception: all non-frozen (9-slot transition scheme at export).
//   - otherwise (direct ExportRykolZone / continent .max): first RklPatch that is not
//     DONOTEXPORT and has a findID-parseable name (exportDirectZone loop); rest RO.
//
// File-frozen (0x0976) always remain frozen. --all-zones is an escape hatch only
// (open-everything); not the authoring default.
// Eligibility only affects paint targets; writeBack still covers every unfrozen carrier
// of editable files; null-edit round-trips the whole file.
//
// Returns a same-length bool vector: true = eligible paint target when not forceFrozen.
void computeZoneEligibility(const std::vector<SZoneNode> &nodes,
                                   const std::string &fileBasename,
                                   std::vector<bool> &eligible)
{
	eligible.assign(nodes.size(), false);
	if (nodes.empty())
		return;
	if (g_AllZones)
	{
		for (size_t i = 0; i < nodes.size(); ++i)
			eligible[i] = !nodes[i].Frozen; // still respect 0x0976 under --all-zones? 
		// Today open-everything = non-0x0976 only. Match that.
		return;
	}

	// Tokenize basename on '-'
	std::vector<std::string> tokens;
	{
		std::string::size_type pos = 0;
		const std::string &s = fileBasename;
		while (pos <= s.size())
		{
			std::string::size_type dash = s.find('-', pos);
			if (dash == std::string::npos) { tokens.push_back(s.substr(pos)); break; }
			tokens.push_back(s.substr(pos, dash - pos));
			pos = dash + 1;
		}
	}
	const std::string t0 = tokens.empty() ? std::string() : NLMISC::toLowerAscii(tokens[0]);

	// Gather non-frozen indices
	std::vector<size_t> nonFrozen;
	for (size_t i = 0; i < nodes.size(); ++i)
		if (!nodes[i].Frozen)
			nonFrozen.push_back(i);

	if (t0 == "zonematerial" || t0 == "zonespecial")
	{
		// Ligo single-brick: one non-frozen. Prefer name match to cell token.
		std::string cell;
		if (t0 == "zonematerial" && tokens.size() >= 3)
			cell = tokens.back(); // e.g. 200_dz
		else if (t0 == "zonespecial" && tokens.size() >= 2)
			cell = tokens[1];
		int match = -1;
		if (!cell.empty())
		{
			const std::string cellL = NLMISC::toLowerAscii(cell);
			for (size_t k = 0; k < nonFrozen.size(); ++k)
			{
				std::string n = NLMISC::toLowerAscii(
					ucstring(nodes[nonFrozen[k]].Node->userName()).toUtf8());
				if (n == cellL || n.find(cellL) != std::string::npos)
				{
					match = (int)nonFrozen[k];
					break;
				}
			}
		}
		if (match >= 0)
			eligible[(size_t)match] = true;
		else if (!nonFrozen.empty())
			eligible[nonFrozen[0]] = true;
		for (size_t k = 0; k < nonFrozen.size(); ++k)
			if (!eligible[nonFrozen[k]])
				printf("eligibility: '%s' node '%s' non-export → read-only context\n",
				       fileBasename.c_str(),
				       ucstring(nodes[nonFrozen[k]].Node->userName()).toUtf8().c_str());
		return;
	}
	if (t0 == "zonetransition")
	{
		// All non-frozen (exporter classifies on transition grid)
		for (size_t k = 0; k < nonFrozen.size(); ++k)
			eligible[nonFrozen[k]] = true;
		return;
	}

	// Direct zone process (ExportRykolZone): first non-DONOTEXPORT with findID-able name.
	// findID lives in export_zone; replicate the name parse (NUM_AB...).
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen)
			continue;
		// DONOTEXPORT appdata: the exporter refuses these nodes outright
		// (nel_patch_converter/script.cpp export_zone_cf), so eligibility must too;
		// otherwise a marked first node becomes the sole editable while the node the
		// exporter actually exports is stuck read-only.
		if (APPDATA::getScriptAppDataInt(nodes[i].Node, NEL3D_APPDATA_DONOTEXPORT, 0) != 0)
			continue;
		std::string name = ucstring(nodes[i].Node->userName()).toUtf8();
		// findID: parts[0]=num, parts[1]=two letters
		std::vector<std::string> parts;
		std::string::size_type pos = 0;
		while (pos <= name.size())
		{
			std::string::size_type us = name.find('_', pos);
			if (us == std::string::npos) { parts.push_back(name.substr(pos)); break; }
			parts.push_back(name.substr(pos, us - pos));
			pos = us + 1;
		}
		bool idOk = false;
		if (parts.size() >= 2 && parts[1].size() >= 2)
		{
			char l1 = parts[1][0], l2 = parts[1][1];
			if (l1 >= 'A' && l1 <= 'Z' && l2 >= 'A' && l2 <= 'Z')
			{
				int num = 0;
				if (sscanf(parts[0].c_str(), "%d", &num) == 1)
					idOk = true;
			}
			// also accept lowercase letters
			if (!idOk && parts[1].size() >= 2)
			{
				char l1 = (char)toupper(parts[1][0]), l2 = (char)toupper(parts[1][1]);
				if (l1 >= 'A' && l1 <= 'Z' && l2 >= 'A' && l2 <= 'Z')
				{
					int num = 0;
					if (sscanf(parts[0].c_str(), "%d", &num) == 1)
						idOk = true;
				}
			}
		}
		if (idOk)
		{
			eligible[i] = true;
			// Only the FIRST (export writes one .zone and exits)
			for (size_t j = i + 1; j < nodes.size(); ++j)
			{
				if (nodes[j].Frozen) continue;
				printf("eligibility: '%s' node '%s' not first exportable → read-only context\n",
				       fileBasename.c_str(),
				       ucstring(nodes[j].Node->userName()).toUtf8().c_str());
			}
			return;
		}
	}
	// Fallback: first non-frozen (ligo-like) when no findID match
	if (!nonFrozen.empty())
		eligible[nonFrozen[0]] = true;
}

/** True when board sessions should omit embedded non-eligible/frozen display copies. */
bool boardSkipEmbedded()
{
	return g_BoardSession && !g_EmbeddedContext;
}

/**
 * Append paint zones from one Max scene.
 * zoneIdOffset: first zone id for this file (must not collide with existing zones).
 * forceFrozen: true for neighbor/context files (landscape + weld + metaTile, never paint,
 *   carriers never rewritten because AnyUnfrozen stays false).
 * fileBasename: used for exporter-faithful eligibility.
 *
 * Board authority: when boardSkipEmbedded(), non-eligible and 0x0976 nodes are NOT
 * displayed (logged and skipped). Neighbor files under the same rule only surface their
 * eligible zone(s) as RO. Legacy direct-.max and --embedded-context keep display of
 * embedded RO copies. Save still round-trips every carrier byte-faithfully (load-time
 * display filter only; nodes remain in the scene graph).
 */
bool buildPaintZones(CScene &scene, std::vector<SPaintZone> &zones,
                            uint zoneIdOffset, bool forceFrozen,
                            const std::string &fileBasename)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	std::vector<bool> eligible;
	// Always compute eligibility when we may skip or when not force-frozen.
	// Neighbor files under board authority need eligibility to pick the real brick zone.
	if (forceFrozen && !boardSkipEmbedded())
		eligible.assign(nodes.size(), false); // legacy neighbor: all RO display
	else
		computeZoneEligibility(nodes, fileBasename, eligible);

	const bool skipEmb = boardSkipEmbedded();
	SNodeTMCache tmCache;
	bool any = false;
	uint nEligible = 0, nRo = 0, nSkipped = 0;
	std::string skippedNames;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		std::string name = ucstring(node->userName()).toUtf8();
		const bool fileFrozen = nodes[i].Frozen;
		const bool isEligible = (i < eligible.size() && eligible[i]);

		// Board authority: never display embedded 0x0976 or non-eligible nodes.
		// Neighbor load (forceFrozen) under board: only eligible zone(s) as RO.
		if (skipEmb)
		{
			if (fileFrozen || !isEligible)
			{
				if (nSkipped++) skippedNames += ", ";
				skippedNames += name;
				if (fileFrozen) skippedNames += "[0x0976]";
				else skippedNames += "[ineligible]";
				continue;
			}
		}

		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			// The plugin showed a message box and kept going; keep the surviving zones displayed.
			fprintf(stderr, "WARNING: node '%s': %s (zone skipped)\n", name.c_str(), err.c_str());
			continue;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		SPaintZone pz;
		pz.Node = node;
		// Neighbor files / non-eligible / 0x0976 → read-only context (when displayed).
		const bool ineligible = !forceFrozen && !isEligible;
		pz.Frozen = forceFrozen || fileFrozen || ineligible;
		pz.Name = name;
		pz.ZoneId = zoneIdOffset + (uint)i;
		if (!buildPatchInfo(ep, objectTM, (int)pz.ZoneId, pz.Patches, err))
		{
			fprintf(stderr, "WARNING: node '%s': %s (zone skipped)\n", name.c_str(), err.c_str());
			continue;
		}
		pz.Ep = ep;
		zones.push_back(pz);
		any = true;
		if (pz.Frozen) ++nRo; else ++nEligible;
		if (g_verbose)
			printf("zone %u '%s'%s: %u patches\n", pz.ZoneId, pz.Name.c_str(),
			       pz.Frozen ? " FROZEN" : "", (uint)pz.Patches.size());
	}
	if (skipEmb && nSkipped)
		printf("board authority '%s': skipped %u embedded/non-eligible zone(s): %s\n",
		       fileBasename.c_str(), nSkipped, skippedNames.c_str());
	if (!forceFrozen && any)
		printf("eligibility '%s': %u editable, %u read-only context (all-zones=%s board-skip=%s)\n",
		       fileBasename.c_str(), nEligible, nRo, g_AllZones ? "on" : "off",
		       skipEmb ? "on" : "off");
	return any;
}

/** Next free zone id base after the highest already-assigned id (leave a gap of 1). */
uint nextZoneIdBase(const std::vector<SPaintZone> &zones)
{
	uint maxId = 0;
	bool any = false;
	for (size_t i = 0; i < zones.size(); ++i)
	{
		if (!any || zones[i].ZoneId > maxId)
		{
			maxId = zones[i].ZoneId;
			any = true;
		}
	}
	return any ? (maxId + 1) : 0;
}


/** Translate all patch geometry of zones[begin..end) by (dx,dy) world units. */
void translateZonesXY(std::vector<SPaintZone> &zones, size_t begin, size_t end,
                             float dx, float dy)
{
	for (size_t i = begin; i < end && i < zones.size(); ++i)
	{
		for (size_t p = 0; p < zones[i].Patches.size(); ++p)
		{
			NL3D::CPatchInfo &pi = zones[i].Patches[p];
			for (uint v = 0; v < 4; ++v)
			{
				pi.Patch.Vertices[v].x += dx;
				pi.Patch.Vertices[v].y += dy;
			}
			for (uint v = 0; v < 8; ++v)
			{
				pi.Patch.Tangents[v].x += dx;
				pi.Patch.Tangents[v].y += dy;
			}
			for (uint v = 0; v < 4; ++v)
			{
				pi.Patch.Interiors[v].x += dx;
				pi.Patch.Interiors[v].y += dy;
			}
		}
	}
}

/** Scale all patch geometry of zones[begin..end) about the world origin (unit conversion). */
void scaleZonesXYZ(std::vector<SPaintZone> &zones, size_t begin, size_t end, float s)
{
	for (size_t i = begin; i < end && i < zones.size(); ++i)
	{
		for (size_t p = 0; p < zones[i].Patches.size(); ++p)
		{
			NL3D::CPatchInfo &pi = zones[i].Patches[p];
			for (uint v = 0; v < 4; ++v) pi.Patch.Vertices[v] *= s;
			for (uint v = 0; v < 8; ++v) pi.Patch.Tangents[v] *= s;
			for (uint v = 0; v < 4; ++v) pi.Patch.Interiors[v] *= s;
		}
	}
}

/** Authored-space AABB (patch corners) over zones[begin..end); false when empty. */
bool authoredAABB(const std::vector<SPaintZone> &zones, size_t begin, size_t end,
                         NLMISC::CAABBox &out)
{
	bool init = false;
	for (size_t i = begin; i < end && i < zones.size(); ++i)
	for (size_t p = 0; p < zones[i].Patches.size(); ++p)
	for (uint v = 0; v < 4; ++v)
	{
		const NLMISC::CVector &pt = zones[i].Patches[p].Patch.Vertices[v];
		if (!init) { out.setCenter(pt); out.setHalfSize(NLMISC::CVector::Null); init = true; }
		else out.extend(pt);
	}
	return init;
}

/**
 * Deep-free a loaded .max: Scene / ClassDirectory3 / DllDirectory, then the struct.
 * Same teardown order as loadMaxFile's own failure path. Never call on
 * loadMaxFileCached results (cache owns those). Callers must not hold zone Node or
 * carrier pointers into the scene (free only after the working set no longer does).
 */
void freeLoadedMax(PMAXLOAD::SLoadedMax *lm)
{
	if (!lm) return;
	delete lm->Scene;
	delete lm->Cd;
	delete lm->Dll;
	delete lm;
}


/**
 * Display clone of a primary zone at footprint cell placement with rot/mirror.
 * Shares Node (carrier) with source. Geometry is transformed; tiles stay authored-space
 * until applyInstanceDisplayTiles (after bank load) / paint_core transformDesc.
 */
SPaintZone cloneInstanceZone(const SPaintZone &src, uint zoneId, float dx, float dy,
                                    float pivotX, float pivotY, const SInstancePlace &place)
{
	SPaintZone pz = src;
	pz.ZoneId = zoneId;
	pz.Rotate = place.Rot & 3;
	pz.Symmetry = place.Mirror;
	std::string tag = NLMISC::toString(" (inst %d,%d", place.CellX, place.CellY);
	if (pz.Rotate) tag += NLMISC::toString(" R%u", pz.Rotate * 90);
	if (pz.Symmetry) tag += " M";
	tag += ")";
	pz.Name = src.Name + tag;
	pz.BorderVertices.clear();
	// Ep (topology) is a value copy (same binds/orders); Patches are world-space display.
	for (size_t p = 0; p < pz.Patches.size(); ++p)
	{
		NL3D::CPatchInfo &pi = pz.Patches[p];
		for (uint v = 0; v < 4; ++v)
			transformInstanceXY(pi.Patch.Vertices[v].x, pi.Patch.Vertices[v].y,
			                    pivotX, pivotY, dx, dy, pz.Rotate, pz.Symmetry);
		for (uint v = 0; v < 8; ++v)
			transformInstanceXY(pi.Patch.Tangents[v].x, pi.Patch.Tangents[v].y,
			                    pivotX, pivotY, dx, dy, pz.Rotate, pz.Symmetry);
		for (uint v = 0; v < 4; ++v)
			transformInstanceXY(pi.Patch.Interiors[v].x, pi.Patch.Interiors[v].y,
			                    pivotX, pivotY, dx, dy, pz.Rotate, pz.Symmetry);
		// Remap intra-zone bind ZoneIds to this instance (session weld fills cross-zone later)
		for (uint e = 0; e < 4; ++e)
		{
			if (pi.BindEdges[e].NPatchs != 0 && pi.BindEdges[e].ZoneId == (uint16)src.ZoneId)
				pi.BindEdges[e].ZoneId = (uint16)zoneId;
		}
	}
	return pz;
}

/**
 * Append display instances for g_Places (primary already at origin).
 * Place (dx,dy) = min-corner cell of the transformed footprint block.
 * Returns number of zone entries appended. zone ids start at kInstanceZoneIdBase.
 */
// The eco board's coordinate frame: the world position of board cell (0,0).
// Captured ONCE at session assembly from the authored footprint origin of the first
// zone opened through the convenience loading route; that FIRST-OPENED role is the only
// distinction that file has. The frame never re-derives, so moving or closing ANY open
// file (including the first) cannot re-anchor the board.
// g_SessionAnchor* and g_LegacyPlaceSourceName are defined in main.cpp (extern in zp_state.h).
// The file the no-source scratchPlace alias means: pinned at assembly like the
// CLI empty-source --place pins; NEVER re-resolved positionally after a close.

uint appendInstanceZones(std::vector<SPaintZone> &zones, size_t primaryCount,
                                const std::vector<SInstancePlace> &places, float cellSize)
{
	if (places.empty()) return 0;
	if (primaryCount == 0 || zones.size() < primaryCount) return 0;

	// Prefer derived mask footprint; refresh if empty
	if (g_FootprintMask.empty() || g_FootprintCellsW < 1 || g_FootprintCellsH < 1)
		derivePrimaryFootprint(zones, 0, primaryCount, cellSize, g_SessionSnap > 0.f ? g_SessionSnap : 1.f);
	float originX = 0.f, originY = 0.f;
	scratchBoardAnchor(originX, originY); // board mapping frame (session-frozen)
	int cellsW = g_FootprintCellsW, cellsH = g_FootprintCellsH;
	float stepX = (float)cellsW * cellSize, stepY = (float)cellsH * cellSize;
	float pivotX = originX + stepX * 0.5f;
	float pivotY = originY + stepY * 0.5f;

	// One-shot synthetic W≠H + mask rotFlip/interlock unit checks
	static bool s_UnitChecked = false;
	if (!s_UnitChecked)
	{
		unitCheckFootprintOccupancy();
		s_UnitChecked = true;
	}

	uint nextId = kInstanceZoneIdBase;
	// Overflow guard on the SUM of each place's actual source zone count (file 0's
	// count only stands in on the legacy no-registry path). A big source instanced
	// many times must fail here, not run nextId past uint16 mid-append.
	uint needIds = 0;
	for (size_t pi = 0; pi < places.size(); ++pi)
	{
		if (g_EditableFiles.empty())
		{
			needIds += (uint)primaryCount;
			continue;
		}
		const std::string nm = places[pi].SourceBasename.empty()
			? g_EditableFiles[0].Basename : places[pi].SourceBasename;
		for (size_t ei = 0; ei < g_EditableFiles.size(); ++ei)
		{
			if (NLMISC::toLowerAscii(g_EditableFiles[ei].Basename) == NLMISC::toLowerAscii(nm))
			{
				needIds += (uint)g_EditableFiles[ei].ZoneIds.size();
				break;
			}
		}
	}
	if (nextId + needIds >= 65535)
	{
		fprintf(stderr, "ERROR: instance zone id range would exceed uint16 (need %u ids from %u)\n",
		        needIds, nextId);
		return 0;
	}

	uint appended = 0;
	for (size_t pi = 0; pi < places.size(); ++pi)
	{
		const SInstancePlace &pl = places[pi];
		// The source resolves UNIFORMLY over every open file: the first-opened
		// file is a source like any other (an empty SourceBasename is the legacy CLI
		// spelling for it). Zone range by the file's id base (index*1000), origin at the
		// file's DISPLAYED cell (board anchor + cell), footprint from its fields.
		const std::string srcName = (pl.SourceBasename.empty() && !g_EditableFiles.empty())
			? g_EditableFiles[0].Basename : pl.SourceBasename;
		size_t srcBegin = 0, srcEnd = 0;
		float srcOriginX = originX, srcOriginY = originY;
		int srcCellsW = cellsW, srcCellsH = cellsH;
		bool found = false;
		if (g_EditableFiles.empty())
		{
			// Legacy direct-.max path (no file registry): the whole primary range
			srcBegin = 0;
			srcEnd = primaryCount;
			found = srcEnd > 0;
		}
		else
		{
			for (size_t ei = 0; ei < g_EditableFiles.size(); ++ei)
			{
				if (NLMISC::toLowerAscii(g_EditableFiles[ei].Basename)
				    != NLMISC::toLowerAscii(srcName))
					continue;
				const uint baseId = (uint)(ei * 1000);
				bool haveBegin = false;
				for (size_t z = 0; z < zones.size(); ++z)
				{
					if (zones[z].ZoneId >= baseId && zones[z].ZoneId < baseId + 1000
					    && zones[z].ZoneId < kInstanceZoneIdBase)
					{
						if (!haveBegin) { srcBegin = z; haveBegin = true; }
						srcEnd = z + 1;
					}
				}
				srcOriginX = originX + (float)g_EditableFiles[ei].CellX * cellSize;
				srcOriginY = originY + (float)g_EditableFiles[ei].CellY * cellSize;
				if (g_EditableFiles[ei].CellsW > 0) srcCellsW = g_EditableFiles[ei].CellsW;
				if (g_EditableFiles[ei].CellsH > 0) srcCellsH = g_EditableFiles[ei].CellsH;
				found = haveBegin;
				break;
			}
		}
		if (!found)
		{
			fprintf(stderr, "WARNING: instance source '%s' not open: place skipped\n",
			        srcName.c_str());
			continue;
		}
		const float srcStepX = (float)srcCellsW * cellSize;
		const float srcStepY = (float)srcCellsH * cellSize;
		const float srcPivotX = srcOriginX + srcStepX * 0.5f;
		const float srcPivotY = srcOriginY + srcStepY * 0.5f;
		float dx = 0.f, dy = 0.f;
		computePlaceTranslationFrom(srcOriginX, srcOriginY, srcStepX, srcStepY,
		                            srcPivotX, srcPivotY, originX, originY, cellSize,
		                            pl.CellX, pl.CellY, pl.Rot & 3, pl.Mirror, dx, dy);
		int bw = 0, bh = 0;
		footprintBlockSize(srcCellsW, srcCellsH, pl.Rot, pl.Mirror, bw, bh);
		for (size_t i = srcBegin; i < srcEnd; ++i)
		{
			zones.push_back(cloneInstanceZone(zones[i], nextId, dx, dy, srcPivotX, srcPivotY, pl));
			++nextId;
			++appended;
		}
		const std::string srcTag = pl.SourceBasename.empty()
			? std::string() : (" '" + pl.SourceBasename + "'");
		printf("  place[%u]%s origin cell (%d,%d) rot %u%s  occupies %dx%d cells [%d,%d)×[%d,%d)\n",
		       (uint)pi, srcTag.c_str(),
		       pl.CellX, pl.CellY, pl.Rot, pl.Mirror ? " mirror" : "",
		       bw, bh, pl.CellX, pl.CellX + bw, pl.CellY, pl.CellY + bh);
	}
	printf("instances: %u place(s)  footprint %dx%d cells step (%.1f, %.1f)  origin (%.1f, %.1f)  "
	       "pivot (%.1f, %.1f)  primary zones %u  display zones +%u (ids from %u)\n",
	       (uint)places.size(), cellsW, cellsH, stepX, stepY, originX, originY,
	       pivotX, pivotY, (uint)primaryCount, appended, kInstanceZoneIdBase);
	printf("  note: place dx,dy = min-corner of transformed block in fine cells; "
	       "first-opened file footprint %dx%d (source=%s filled=%s)\n",
	       cellsW, cellsH,
	       g_FootprintFromTemplate ? "template" : "aabb-square",
	       maskHasHole(g_FootprintMask) ? "no" : "yes");
	// Pivot grid-alignment check for multi-cell square (material-bassin 8×8)
	const float halfCell = cellSize * 0.5f;
	const bool pivotOnHalf = (std::fabs(std::fmod((double)pivotX, (double)halfCell)) < 1e-3
	                          || std::fabs(std::fmod((double)pivotX, (double)halfCell) - halfCell) < 1e-3);
	(void)pivotOnHalf;
	printf("  pivot grid: pivot (%.3f, %.3f) = origin + half-step (%.3f, %.3f)\n",
	       pivotX, pivotY, stepX * 0.5f, stepY * 0.5f);
	return appended;
}

/**
 * After bank load: transform display CPatchInfo tile elements on rotated/mirrored instances
 * so the initial landscape matches GetTile display space (plugin exportZone / CPatchInfo::transform
 * tile loop). Geometry is already in place from cloneInstanceZone; only tile/color remounts
 * apply here (control-point index swaps under symmetry are skipped; world XY was already mirrored).
 *
 * Completes the symmetry half: U remount (u' = OrderS-u-1), transformTile/transform256Case
 * with symmetry=true, and tile-color S-flip, so a freshly placed mirrored instance matches
 * painted-then-mirrored before any live setTile refresh.
 */
void applyInstanceDisplayTiles(std::vector<SPaintZone> &zones, NL3D::CTileBank *bank)
{
	if (!bank) return;
	for (size_t zi = 0; zi < zones.size(); ++zi)
	{
		SPaintZone &pz = zones[zi];
		if (pz.Rotate == 0 && !pz.Symmetry) continue;
		const uint rotate = pz.Rotate & 3;
		const bool symmetry = pz.Symmetry;
		for (size_t p = 0; p < pz.Patches.size(); ++p)
		{
			NL3D::CPatchInfo &pi = pz.Patches[p];
			// Tile colors: S-flip under symmetry (CPatchInfo::transform color half)
			if (symmetry && !pi.TileColors.empty() && pi.OrderS > 0)
			{
				const uint countU = (uint)pi.OrderS / 2 + 1;
				const uint countV = (uint)pi.OrderT + 1;
				for (uint v = 0; v < countV; ++v)
				for (uint u = 0; u < countU; ++u)
				{
					const uint index0 = u + v * ((uint)pi.OrderS + 1);
					const uint index1 = ((uint)pi.OrderS - u) + v * ((uint)pi.OrderS + 1);
					if (index0 >= pi.TileColors.size() || index1 >= pi.TileColors.size()) continue;
					if (index0 == index1) continue;
					uint16 tmp = pi.TileColors[index0].Color565;
					pi.TileColors[index0].Color565 = pi.TileColors[index1].Color565;
					pi.TileColors[index1].Color565 = tmp;
				}
			}
			// Tiles: U remount under symmetry + transformTile / transform256Case (plugin loop)
			std::vector<NL3D::CTileElement> tiles = pi.Tiles;
			for (int v = 0; v < pi.OrderT; ++v)
			for (int u = 0; u < pi.OrderS; ++u)
			{
				const int uSymmetry = symmetry ? (pi.OrderS - u - 1) : u;
				NL3D::CTileElement &element = pi.Tiles[u + v * pi.OrderS];
				element = tiles[uSymmetry + v * pi.OrderS];
				for (int l = 0; l < 3; ++l)
				{
					if (element.Tile[l] == 0xffff) continue;
					uint tile = element.Tile[l];
					uint tileRotation = element.getTileOrient(l);
					uint tileRotate = rotate;
					bool tileSymmetry = symmetry;
					// goofy=false: zone Sym is built later in paint_core; initial display uses
					// the non-goofy path (matches rotation remount and live setTile when
					// border state is Nothing).
					if (!NL3D::CPatchInfo::getTileSymmetryRotate(*bank, tile, tileSymmetry, tileRotate))
						continue;
					if (!NL3D::CPatchInfo::transformTile(*bank, tile, tileRotation, tileSymmetry, (4 - tileRotate) & 3, false))
						continue;
					element.Tile[l] = (uint16)tile;
					element.setTileOrient(l, (uint8)(tileRotation & 3));
				}
				if (element.Tile[0] != 0xffff)
				{
					bool is256 = false;
					uint8 uvOff = 0;
					element.getTile256Info(is256, uvOff);
					if (is256)
					{
						uint tileRotate = rotate;
						bool tileSymmetry = symmetry;
						uint tileRotation = tiles[uSymmetry + v * pi.OrderS].getTileOrient(0);
						NL3D::CPatchInfo::getTileSymmetryRotate(*bank, element.Tile[0], tileSymmetry, tileRotate);
						NL3D::CPatchInfo::transform256Case(*bank, uvOff, tileRotation, tileSymmetry, (4 - tileRotate) & 3, false);
						element.setTile256Info(true, uvOff);
					}
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Cross-zone open-edge weld (port of the paint.cpp cross-mesh branch, WELD_THRESOLD=1): two
// open edges of different zones whose world-space endpoints coincide reversed within the
// threshold become a one/one bind across the zones, with CBorderVertex records on both sides
// so the landscape shares the corner tess vertices (and rebinds borders when zones are added
// in sequence). Session-only: this only touches the display CPatchInfo/CZone copies, the .max
// data is never modified by it.

#define WELD_THRESOLD 1.0f

// Both-direction border-vertex record for one matched corner pair, deduplicated (a corner is
// shared by the two consecutive welded edges).
void addBorderVertexPair(std::vector<SPaintZone> &zones, uint zi, uint16 va, uint zj, uint16 vb,
                                std::set<std::pair<std::pair<uint, uint>, std::pair<uint, uint> > > &seen)
{
	std::pair<std::pair<uint, uint>, std::pair<uint, uint> > key(
		std::pair<uint, uint>(zi, va), std::pair<uint, uint>(zj, vb));
	if (key.second < key.first) std::swap(key.first, key.second);
	if (!seen.insert(key).second) return;
	NL3D::CBorderVertex bv;
	bv.CurrentVertex = va;
	bv.NeighborZoneId = (uint16)zones[zj].ZoneId;
	bv.NeighborVertex = vb;
	zones[zi].BorderVertices.push_back(bv);
	bv.CurrentVertex = vb;
	bv.NeighborZoneId = (uint16)zones[zi].ZoneId;
	bv.NeighborVertex = va;
	zones[zj].BorderVertices.push_back(bv);
}

uint weldPaintZones(std::vector<SPaintZone> &zones)
{
	uint welds = 0;
	std::set<std::pair<std::pair<uint, uint>, std::pair<uint, uint> > > seenVerts;
	for (uint zi = 0; zi < zones.size(); ++zi)
	{
		std::vector<NL3D::CPatchInfo> &pa = zones[zi].Patches;
		for (uint p = 0; p < pa.size(); ++p)
		for (uint e = 0; e < 4; ++e)
		{
			if (pa[p].BindEdges[e].NPatchs != 0) continue; // interior or already welded
			// Edge e runs from corner e to corner (e+1)&3 (Max Patch edge convention, same as
			// the paint.cpp scan).
			const NLMISC::CVector &vA1 = pa[p].Patch.Vertices[e];
			const NLMISC::CVector &vB1 = pa[p].Patch.Vertices[(e + 1) & 3];
			bool found = false;
			for (uint zj = 0; zj < zones.size() && !found; ++zj)
			{
				if (zj == zi) continue;
				std::vector<NL3D::CPatchInfo> &pb = zones[zj].Patches;
				for (uint pp = 0; pp < pb.size() && !found; ++pp)
				for (uint ee = 0; ee < 4; ++ee)
				{
					if (pb[pp].BindEdges[ee].NPatchs != 0) continue;
					const NLMISC::CVector &vA2 = pb[pp].Patch.Vertices[ee];
					const NLMISC::CVector &vB2 = pb[pp].Patch.Vertices[(ee + 1) & 3];
					// The same edge, reversed orientation (paint.cpp: vA1~vB2 && vA2~vB1).
					if ((vA1 - vB2).norm() < WELD_THRESOLD && (vA2 - vB1).norm() < WELD_THRESOLD)
					{
						pa[p].BindEdges[e].NPatchs = 1;
						pa[p].BindEdges[e].ZoneId = (uint16)zones[zj].ZoneId;
						pa[p].BindEdges[e].Next[0] = (uint16)pp;
						pa[p].BindEdges[e].Edge[0] = (uint8)ee;
						pb[pp].BindEdges[ee].NPatchs = 1;
						pb[pp].BindEdges[ee].ZoneId = (uint16)zones[zi].ZoneId;
						pb[pp].BindEdges[ee].Next[0] = (uint16)p;
						pb[pp].BindEdges[ee].Edge[0] = (uint8)e;
						// Shared corners: (zi corner e <-> zj corner ee+1), (zi corner e+1 <-> zj
						// corner ee).
						addBorderVertexPair(zones, zi, pa[p].BaseVertices[e],
						                    zj, pb[pp].BaseVertices[(ee + 1) & 3], seenVerts);
						addBorderVertexPair(zones, zi, pa[p].BaseVertices[(e + 1) & 3],
						                    zj, pb[pp].BaseVertices[ee], seenVerts);
						++welds;
						found = true;
						break;
					}
				}
			}
		}
	}
	return welds;
}

// Build one display CZone from a paint zone: CZone::build + the corner smoother, exactly the
// plugin's per-zone sequence (smoother runs on the built, uncompiled zone with no neighbor
// list, like the plugin did).
void buildDisplayZone(const SPaintZone &pz, NL3D::CZone &zone)
{
	zone.build((uint16)pz.ZoneId, pz.Patches, pz.BorderVertices);
	NL3D::CZoneCornerSmoother cornerSmoother;
	std::vector<NL3D::CZone *> emptyVector;
	cornerSmoother.computeAllCornerSmoothFlags(&zone, emptyVector);
}

// ---------------------------------------------------------------------------------------------
// --dump-zones: the headless eval->weld->build proof. Writes each built zone and reports the
// counts that the weld and bind passes produced.

std::string sanitizeName(const std::string &s)
{
	std::string r = s;
	for (size_t i = 0; i < r.size(); ++i)
	{
		char c = r[i];
		bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
		if (!ok) r[i] = '_';
	}
	return r;
}

int dumpZones(std::vector<SPaintZone> &zones, uint welds, const std::string &outDir)
{
	NLMISC::CFile::createDirectoryTree(outDir);
	uint totalPatches = 0, totalBound = 0, totalCross = 0, totalBorderVerts = 0;
	for (size_t i = 0; i < zones.size(); ++i)
	{
		const SPaintZone &pz = zones[i];
		uint bound = 0, cross = 0;
		for (size_t p = 0; p < pz.Patches.size(); ++p)
		for (uint e = 0; e < 4; ++e)
		{
			const NL3D::CPatchInfo::CBindInfo &b = pz.Patches[p].BindEdges[e];
			if (b.NPatchs == 0) continue;
			++bound;
			if (b.ZoneId != pz.ZoneId) ++cross;
		}
		NL3D::CZone zone;
		buildDisplayZone(pz, zone);
		std::string path = outDir + "/" + NLMISC::toString("zone_%u_", pz.ZoneId) + sanitizeName(pz.Name) + ".zone";
		if (!writeZoneV4(zone, path))
		{
			fprintf(stderr, "ERROR: cannot write %s\n", path.c_str());
			return 1;
		}
		NLMISC::CAABBox zb;
		bool zbInit = false;
		for (size_t p = 0; p < pz.Patches.size(); ++p)
		for (uint v = 0; v < 4; ++v)
		{
			if (!zbInit) { zb.setCenter(pz.Patches[p].Patch.Vertices[v]); zb.setHalfSize(NLMISC::CVector::Null); zbInit = true; }
			else zb.extend(pz.Patches[p].Patch.Vertices[v]);
		}
		printf("zone %u '%s'%s: %u patches, %u bound edges (%u cross-zone), %u border verts, "
		       "bbox (%.1f,%.1f)-(%.1f,%.1f) -> %s\n",
		       pz.ZoneId, pz.Name.c_str(), pz.Frozen ? " FROZEN" : "",
		       (uint)pz.Patches.size(), bound, cross, (uint)pz.BorderVertices.size(),
		       zb.getMin().x, zb.getMin().y, zb.getMax().x, zb.getMax().y, path.c_str());
		totalPatches += (uint)pz.Patches.size();
		totalBound += bound;
		totalCross += cross;
		totalBorderVerts += (uint)pz.BorderVertices.size();
	}
	// Name list for eligibility verification (editable vs read-only context)
	{
		std::string editNames, roNames;
		uint nEdit = 0, nRo = 0;
		for (size_t i = 0; i < zones.size(); ++i)
		{
			const SPaintZone &pz = zones[i];
			if (pz.Frozen)
			{
				if (nRo++) roNames += ", ";
				roNames += pz.Name;
			}
			else
			{
				if (nEdit++) editNames += ", ";
				editNames += pz.Name;
			}
		}
		printf("eligibility names: editable(%u)=[%s] read-only(%u)=[%s] all-zones=%s\n",
		       nEdit, editNames.c_str(), nRo, roNames.c_str(), g_AllZones ? "on" : "off");
	}
	printf("OK dump-zones: %u zones, %u patches, %u bound edges (%u cross-zone), %u welds, %u border verts\n",
	       (uint)zones.size(), totalPatches, totalBound, totalCross, welds, totalBorderVerts);
	return 0;
}


// Build the paint core inputs from the assembled zones (pointers into the final zones vector).
void buildPaintInputs(std::vector<SPaintZone> &zones, std::vector<ZPPAINT::SPaintZoneInput> &inputs)
{
	inputs.clear();
	for (size_t i = 0; i < zones.size(); ++i)
	{
		ZPPAINT::SPaintZoneInput in;
		in.Node = zones[i].Node;
		in.Frozen = zones[i].Frozen;
		in.ZoneId = zones[i].ZoneId;
		in.Name = zones[i].Name;
		in.Patches = &zones[i].Patches;
		in.Pm = &zones[i].Ep.Pm;
		in.EvalRp = &zones[i].Ep.Rp;
		in.Rotate = zones[i].Rotate;
		in.Symmetry = zones[i].Symmetry;
		inputs.push_back(in);
	}
}


