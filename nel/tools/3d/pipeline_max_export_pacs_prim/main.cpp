/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 */
// Pacs_prim export: .max -> .pacs_prim, replicating the NelExportPACSPrimitives path of the
// 3ds Max plugin (build_gamedata processes/pacs_prim) without 3ds Max.
//
// The pacs_prim maxscript (processes/pacs_prim/maxscript/pacs_prim_export.ms) selects every
// `geometry` node whose class is the scripted "PACS Box" (nel_pacs_box, ClassId
// {0x7f374277,0x5d3971df}, extends:Box) or "PACS Cyl" (nel_pacs_cylinder, ClassId
// {0x62a56810,0x4b3d601c}, extends:Cylinder) plugin and calls NelExportPACSPrimitives once over
// the whole array -> ONE output file named after the SOURCE .max (not grouped by ig name, unlike
// ig/cmb). NelExportPACSPrimitives (nel_export/nel_export_collision.cpp) calls
// CExportNel::buildPrimitiveBlock (nel_mesh_lib/export_collision.cpp), which per node:
//   - reads 12 named ParamBlock2 params (Reaction, Obstacle, EnterTrigger, ExitTrigger,
//     OverlapTrigger, CollisionMask, OcclusionMask, UserData0-3, Absorbtion) — these plugins are
//     "extends" scripted plugins: reference 0 is the delegate Box/Cylinder GeomObject (a real Max
//     primitive with its own old-style ParamBlock, superclass 0x8), reference 1 is a ParamBlock2
//     with the 12 custom params above in that MAXScript declaration order (param id == index,
//     no explicit `id:` in nel_pacs_box.ms/nel_pacs_cylinder.ms) — corpus-confirmed byte-exact
//     against ~/pipeline_export/*/pacs_prim references (2026-07-08 session).
//   - reads the delegate's own dimension params via the old ParamBlock (Box: index 0=length,
//     1=width, 2=height; Cylinder: index 0=radius, 1=height — same index scheme already
//     established by pipeline_max_export_ig's buildParametricMesh).
//   - Position = the node's world transform (GetNodeTM) translation; Orientation = getZRot of
//     the world I-axis for boxes (the signed angle to global X, ignoring Z), always 0 for
//     cylinders (2D-oriented primitives only rotate about Z, and a cylinder's cross-section is
//     rotationally symmetric so the plugin never bothers to compute it).
// If the source .max has zero PACS-primitive nodes, no output file is written at all (matching
// the maxscript's `if (arrayNode.count != 0) then (Export...) else (WARNING, tag anyway)` — the
// "tag anyway" means the source is still marked done, just with no artifact).
//
// Output format: the real NLPACS::CPrimitiveBlock/CPrimitiveDesc (nel/pacs/primitive_block.h) —
// built and serialized exactly like the reference plugin (CPrimitiveBlock::serial over a real
// NLMISC::COXml stream), so the XML is exact by construction rather than a hand-reproduced
// format. Verified byte-identical against every ~/pipeline_export/*/pacs_prim reference.

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
#include <nel/misc/o_xml.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/pacs/primitive_block.h>
#include <nel/pacs/u_move_primitive.h>

#include "../pipeline_max/storage_ole.h"
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
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/param_block_2.h"

#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/old_param_block.h"
#include "../pipeline_max_export_common/export_ids.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

namespace {

using PMAX_EXPORT_IDS::CLASSID_PACS_BOX;
using PMAX_EXPORT_IDS::CLASSID_PACS_CYL;
const TSClassId SCLASS_GEOMOBJECT = 0x00000010;
const TSClassId SCLASS_PARAMBLOCK2 = 0x00000082;

// CExportNel::getZRot (nel_mesh_lib/export_collision.cpp): the signed angle in [0, 2pi) from
// global X to the (Z-flattened, normalized) I axis.
float getZRot(const NLMISC::CVector &i)
{
	NLMISC::CVector n = i;
	n.z = 0;
	n.normalize();
	float cosa = n * NLMISC::CVector::I;
	float sina = (NLMISC::CVector::I ^ n) * NLMISC::CVector::K;
	return (sina > 0) ? (float)acos(cosa) : (float)(2.0 * NLMISC::Pi - acos(cosa));
}

// Find the reference among obj's own references whose classDesc superclass matches; returns
// NULL when absent. Mirrors the "extends" scripted-plugin routing already established for
// nel_ps/nel_flare (pipeline_max_design.md §7): the delegate GeomObject and the custom
// ParamBlock2 are just references on the node's object, found by superclass, not by fixed index
// (robust against declaration-order differences across plugin versions).
CSceneClass *findReferenceBySuperClass(CSceneClass *obj, TSClassId superClassId, CSceneClass *exclude)
{
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	if (!rm) return NULL;
	for (uint i = 0; i < rm->nbReferences(); ++i)
	{
		CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
		if (!r || r == exclude) continue;
		if (r->classDesc()->superClassId() == superClassId) return r;
	}
	return NULL;
}

// Decode one PACS primitive node. Returns false (logging why) on any missing piece — the
// reference exporter's buildPrimitiveBlock clears the WHOLE output block if any single node
// fails this decode (§ design), so a hard failure here should abort the whole file, not just
// skip the node.
bool decodePrimitive(INode &node, MAXSCENE::SNodeTMCache &tmCache, NLPACS::CPrimitiveDesc &desc, std::string &err)
{
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node.getReference(1));
	if (!obj) { err = "node has no object reference"; return false; }
	NLMISC::CClassId cid = obj->classDesc()->classId();
	bool isBox = (cid == CLASSID_PACS_BOX);
	bool isCyl = (cid == CLASSID_PACS_CYL);
	if (!isBox && !isCyl) { err = "not a PACS primitive"; return false; }

	CSceneClass *delegate = findReferenceBySuperClass(obj, SCLASS_GEOMOBJECT, obj);
	CSceneClass *pb2Obj = findReferenceBySuperClass(obj, SCLASS_PARAMBLOCK2, obj);
	if (!delegate) { err = "no delegate Box/Cylinder reference"; return false; }
	if (!pb2Obj) { err = "no ParamBlock2 reference"; return false; }
	CParamBlock2 *pb2 = dynamic_cast<CParamBlock2 *>(pb2Obj);
	if (!pb2) { err = "ParamBlock2 reference did not parse as CParamBlock2"; return false; }

	// The 12 custom params, by declaration index (nel_pacs_box.ms / nel_pacs_cylinder.ms).
	sint32 reaction = 1; bool enterTrigger = false, exitTrigger = false, overlapTrigger = false;
	sint32 collisionMask = 0, occlusionMask = 1; bool obstacle = true; float absorbtion = 1.0f;
	sint32 userData0 = 0, userData1 = 0, userData2 = 0, userData3 = 0;
	bool ok = true;
	ok &= pb2->getInt(0, reaction);
	ok &= pb2->getBool(1, enterTrigger);
	ok &= pb2->getBool(2, exitTrigger);
	ok &= pb2->getBool(3, overlapTrigger);
	ok &= pb2->getInt(4, collisionMask);
	ok &= pb2->getInt(5, occlusionMask);
	ok &= pb2->getBool(6, obstacle);
	ok &= pb2->getFloat(7, absorbtion);
	ok &= pb2->getInt(8, userData0);
	ok &= pb2->getInt(9, userData1);
	ok &= pb2->getInt(10, userData2);
	ok &= pb2->getInt(11, userData3);
	if (!ok) { err = "missing ParamBlock2 param(s)"; return false; }

	// Delegate's own dimensions, via its old-style ParamBlock (reference 0 of the delegate).
	CSceneClass *delegatePBlock = findReferenceBySuperClass(delegate, 0x00000008, NULL);
	if (!delegatePBlock) { err = "delegate has no old ParamBlock"; return false; }
	std::map<sint32, OLDPBLOCK::SParam> dims;
	OLDPBLOCK::readOldParamBlock(delegatePBlock, dims);

	float height, length0, length1, orientation;
	NLMISC::CMatrix mt;
	MAXMATH::Matrix3M nodeTM = MAXSCENE::getNodeTM(&node, tmCache);
	MAXSCENE::convertMatrix(mt, nodeTM);
	if (isBox)
	{
		height = OLDPBLOCK::paramFloat(dims, 2);
		length0 = OLDPBLOCK::paramFloat(dims, 1); // box.width
		length1 = OLDPBLOCK::paramFloat(dims, 0); // box.length
		orientation = getZRot(mt.getI());
	}
	else
	{
		height = OLDPBLOCK::paramFloat(dims, 1);
		length0 = OLDPBLOCK::paramFloat(dims, 0); // cylinder.radius
		length1 = 0.0f;
		orientation = 0.0f;
	}

	desc.Length[0] = length0;
	desc.Length[1] = length1;
	desc.Height = height;
	desc.Attenuation = absorbtion;
	desc.Type = isBox ? NLPACS::UMovePrimitive::_2DOrientedBox : NLPACS::UMovePrimitive::_2DOrientedCylinder;
	// MaxScript radiobuttons are 1-based (1..4); the reference exporter's own cast, reproduced
	// verbatim (note this does NOT reproduce UMovePrimitive::TReaction's own enumerator values
	// for "Stop" — (4-1)<<4 = 0x30, not the named Stop=0x40 — a quirk of the reference tool we
	// match bit-for-bit rather than "fix").
	desc.Reaction = (NLPACS::UMovePrimitive::TReaction)((reaction - 1) << 4);
	desc.Trigger = (NLPACS::UMovePrimitive::TTrigger)(
		(enterTrigger ? NLPACS::UMovePrimitive::EnterTrigger : 0)
		| (exitTrigger ? NLPACS::UMovePrimitive::ExitTrigger : 0)
		| (overlapTrigger ? NLPACS::UMovePrimitive::OverlapTrigger : 0));
	desc.Obstacle = obstacle;
	desc.OcclusionMask = (uint32)occlusionMask;
	desc.CollisionMask = (uint32)collisionMask;
	desc.Position = mt.getPos();
	desc.Orientation = orientation;
	desc.UserData = ((uint64)(uint16)userData0) | (((uint64)(uint16)userData1) << 16)
		| (((uint64)(uint16)userData2) << 32) | (((uint64)(uint16)userData3) << 48);
	return true;
}

} /* anonymous namespace */

// Whole-file flow shared by the standalone tool and the max2gltf writer (PMB_PACS_PRIM_NO_MAIN
// + nel_pacs_prim blob): returns 1 with the serialized XML bytes, 3 when the scene has no PACS
// primitives, -1 on error. One code path — the tool's file and the blob cannot drift.
int pmbExportPacsPrimForGltf(const std::string &maxPath, std::vector<uint8> &out)
{
	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);
	NELPATCH::CNelPatch::registerClasses(&reg);

	CStorageOleIn in;
	if (!in.open(maxPath.c_str())) { std::cerr << "ERROR: not an OLE compound file: " << maxPath << "\n"; return -1; }

	CDllDirectory dll;
	{ std::vector<uint8> b; if (!in.readStream("DllDirectory", b)) { std::cerr << "ERROR: no DllDirectory stream\n"; return -1; } CStorageStream st(b); dll.serial(st); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ std::vector<uint8> b; if (!in.readStream("ClassDirectory3", b)) { std::cerr << "ERROR: no ClassDirectory3 stream\n"; return -1; } CStorageStream st(b); cd.serial(st); }
	cd.parse(VersionUnknown);
	CScene scene(&reg, &dll, &cd);
	{ std::vector<uint8> b; if (!in.readStream("Scene", b)) { std::cerr << "ERROR: no Scene stream\n"; return -1; } CStorageStream st(b); scene.serial(st); }
	scene.parse(VersionUnknown);

	// Walk nodes in scene-container order (the `geometry` MaxScript category enumeration,
	// same precedent as pipeline_max_export_ig/_swt), collecting PACS-primitive nodes.
	std::vector<INode *> candidates;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
		if (!obj) continue;
		NLMISC::CClassId cid = obj->classDesc()->classId();
		if (cid == CLASSID_PACS_BOX || cid == CLASSID_PACS_CYL)
			candidates.push_back(node);
	}

	if (candidates.empty())
	{
		std::cerr << "WARNING: no PACS primitives in " << maxPath << "\n";
		return 3;
	}

	MAXSCENE::SNodeTMCache tmCache;
	NLPACS::CPrimitiveBlock primitiveBlock;
	primitiveBlock.Primitives.resize(candidates.size());
	for (size_t i = 0; i < candidates.size(); ++i)
	{
		std::string err;
		if (!decodePrimitive(*candidates[i], tmCache, primitiveBlock.Primitives[i], err))
		{
			std::cerr << "ERROR: \"" << ucstring(candidates[i]->userName()).toUtf8() << "\": " << err << "\n";
			return -1;
		}
	}

	try
	{
		NLMISC::CMemStream ms;
		NLMISC::COXml output;
		if (!output.init(&ms, "1.0")) { std::cerr << "ERROR: cannot init XML stream\n"; return -1; }
		primitiveBlock.serial(output);
		output.flush();
		out.assign(ms.buffer(), ms.buffer() + ms.length());
	}
	catch (const NLMISC::Exception &e)
	{
		std::cerr << "ERROR: serial failed: " << e.what() << "\n";
		return -1;
	}

	return 1;
}

#ifndef PMB_PACS_PRIM_NO_MAIN
int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cerr << "usage: pipeline_max_export_pacs_prim <input.max> <output.pacs_prim>\n";
		std::cerr << "exit codes: 0 ok, 1 error, 3 nothing to export (no output written)\n";
		return 1;
	}
	std::vector<uint8> bytes;
	int rc = pmbExportPacsPrimForGltf(argv[1], bytes);
	if (rc == 3) return 3;
	if (rc != 1) return 1;
	NLMISC::COFile file;
	if (!file.open(argv[2])) { std::cerr << "ERROR: cannot open output " << argv[2] << "\n"; return 1; }
	try
	{
		file.serialBuffer(&bytes[0], (uint)bytes.size());
		file.close();
	}
	catch (const NLMISC::Exception &e)
	{
		std::cerr << "ERROR: write failed: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
#endif /* PMB_PACS_PRIM_NO_MAIN */

/* end of file */
