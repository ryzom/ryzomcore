/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Swt export: .max -> .swt, replicating the NelExportSkeletonWeight path of the 3ds Max
// plugin (build_gamedata processes/swt) without 3ds Max.
//
// The swt maxscript does `max select all` and calls the plugin's exportSWT over the whole
// selection; exportSWT (plugin_max/nel_export/nel_export_swt.cpp) keeps every node whose
// NEL3D_APPDATA_EXPORT_SWT appdata is checked, reads one float from
// NEL3D_APPDATA_EXPORT_SWT_WEIGHT, and emits three CSkeletonWeight entries per node —
// "<name>.rotquat", "<name>.pos", "<name>.scale" — all carrying that weight. Node order is
// scene (container) order, matching the Max selection enumeration of `max select all`
// (byte-validated against the reference export of max_top.max).

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
#include <nel/misc/file.h>
#include <nel/3d/skeleton_weight.h>
#include <nel/3d/transformable.h>

#include "../pipeline_max/storage_ole.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"
#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/storage/app_data.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

// NeL export AppData sub-ids (plugin_max/nel_mesh_lib/export_appdata.h)
#define NEL3D_APPDATA_EXPORT_SWT 1423062611
#define NEL3D_APPDATA_EXPORT_SWT_WEIGHT 1423062612

// String-valued NeL AppData script entry (SubId match; same convention as export_anim/skel).
static bool getNodeScriptAppDataString(CNodeImpl *node, uint32 subId, std::string &out)
{
	STORAGE::CAppData *ad = node->appData();
	if (!ad) return false;
	for (STORAGE::CAppData::TMap::const_iterator it = ad->entries().begin(); it != ad->entries().end(); ++it)
	{
		if (it->first.SubId != subId) continue;
		CStorageRaw *raw = it->second->value<CStorageRaw>();
		if (!raw) return false;
		std::string s(raw->Value.begin(), raw->Value.end());
		while (!s.empty() && (s[s.size() - 1] == '\0' || s[s.size() - 1] == '\n' || s[s.size() - 1] == '\r')) s.resize(s.size() - 1);
		out = s;
		return true;
	}
	return false;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cerr << "usage: pipeline_max_export_swt <input.max> <output.swt>\n";
		std::cerr << "exit codes: 0 ok, 1 error, 3 nothing to export\n";
		return 1;
	}

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	CStorageOleIn in;
	if (!in.open(argv[1])) { std::cerr << "ERROR: not an OLE compound file: " << argv[1] << "\n"; return 1; }

	CDllDirectory dll;
	{ std::vector<uint8> b; if (!in.readStream("DllDirectory", b)) { std::cerr << "ERROR: no DllDirectory stream\n"; return 1; } CStorageStream st(b); dll.serial(st); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ std::vector<uint8> b; if (!in.readStream("ClassDirectory3", b)) { std::cerr << "ERROR: no ClassDirectory3 stream\n"; return 1; } CStorageStream st(b); cd.serial(st); }
	cd.parse(VersionUnknown);
	CScene scene(&reg, &dll, &cd);
	{ std::vector<uint8> b; if (!in.readStream("Scene", b)) { std::cerr << "ERROR: no Scene stream\n"; return 1; } CStorageStream st(b); scene.serial(st); }
	scene.parse(VersionUnknown);

	// Walk nodes in scene-container order (the `max select all` enumeration) and collect the
	// flagged ones, three channels each: rotquat, pos, scale (reference emission order).
	NL3D::CSkeletonWeight::TNodeArray nodes;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		// `max select all` skips hidden nodes; the swt maxscript unhides categories but not
		// per-node hidden state. Node flag chunk 0x0963 bit 0x40 = hidden (byte-validated on
		// max_top.max, whose hidden Dummy01..18 carry the SWT appdata but are absent from the
		// reference export).
		{
			CStorageRaw *flags = dynamic_cast<CStorageRaw *>(node->findStorageObject(0x0963));
			bool foundFlags = false;
			uint32 fl = 0;
			if (flags && flags->Value.size() >= 4) { memcpy(&fl, flags->Value.data(), 4); foundFlags = true; }
			if (!foundFlags)
			{
				const CStorageContainer::TStorageObjectContainer &orphans = node->orphanedChunks();
				for (CStorageContainer::TStorageObjectConstIt oit = orphans.begin(); oit != orphans.end(); ++oit)
				{
					if (oit->first != 0x0963) continue;
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(oit->second);
					if (raw && raw->Value.size() >= 4) { memcpy(&fl, raw->Value.data(), 4); }
					break;
				}
			}
			if (fl & 0x40) continue; // hidden
		}
		std::string flag;
		if (!getNodeScriptAppDataString(node, NEL3D_APPDATA_EXPORT_SWT, flag)) continue;
		if (atoi(flag.c_str()) == 0) continue; // BST_UNCHECKED
		std::string weightStr;
		float weight = 0.f;
		if (getNodeScriptAppDataString(node, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, weightStr))
			weight = (float)atof(weightStr.c_str());
		std::string name = ucstring(node->userName()).toUtf8();
		NL3D::CSkeletonWeight::CNode entry;
		entry.Name = name + "." + NL3D::ITransformable::getRotQuatValueName();
		entry.Weight = weight;
		nodes.push_back(entry);
		entry.Name = name + "." + NL3D::ITransformable::getPosValueName();
		nodes.push_back(entry);
		entry.Name = name + "." + NL3D::ITransformable::getScaleValueName();
		nodes.push_back(entry);
	}

	if (nodes.empty())
	{
		std::cerr << "WARNING: no node flagged for swt export in " << argv[1] << "\n";
		return 3;
	}

	try
	{
		NL3D::CSkeletonWeight sw;
		sw.build(nodes);
		NLMISC::COFile file;
		if (!file.open(argv[2])) { std::cerr << "ERROR: cannot open output " << argv[2] << "\n"; return 1; }
		sw.serial(file);
		file.close();
	}
	catch (const NLMISC::Exception &e)
	{
		std::cerr << "ERROR: serial failed: " << e.what() << "\n";
		return 1;
	}

	return 0;
}

/* end of file */
