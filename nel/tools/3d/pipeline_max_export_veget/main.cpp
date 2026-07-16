/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
 */
// Vegetable export: .max -> .veget, replicating the NelExportVegetable path of the 3ds Max
// plugin (build_gamedata processes/veget) without 3ds Max.
//
// The veget maxscript (processes/veget/maxscript/veget_export.ms::isToBeExported) iterates
// $geometry, keeps only scene-root nodes (parent == undefined) that aren't "Bip01", filters out
// nel_ps and DONOTEXPORT/CHARACTER_LOD flagged nodes, and emits one .veget per node whose
// NEL3D_APPDATA_VEGETABLE appdata == "1". Per node: NelExportVegetable
// (nel_export/nel_export_export.cpp) -> CExportNel::buildVegetableShape
// (nel_mesh_lib/export_vegetable.cpp): convert to TriObject, build the CMeshBase / CMesh via the
// shared mesh_build/material_build/mesh_eval paths (same infrastructure the shape exporter uses),
// verify the mesh has UV1 + exactly one matrix block + one render pass, then copy the VB + PB
// into a CVegetableShapeBuild along with a fixed set of vegetable appdata flags
// (VEGETABLE_ALPHA_BLEND / VEGETABLE_ALPHA_BLEND_ON_LIGHTED / VEGETABLE_ALPHA_BLEND_OFF_LIGHTED /
// VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED / VEGETABLE_FORCE_BEST_SIDED_LIGHTING, plus BEND_FACTOR
// and BEND_CENTER), and CVegetableShape::build. Serialize via `CVegetableShape::serial`.

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
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/mesh.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/vegetable_shape.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../pipeline_max_export_shape/scene_lib.h"
#include "../pipeline_max_export_shape/mesh_eval.h"
#include "../pipeline_max_export_shape/material_build.h"
#include "../pipeline_max_export_shape/mesh_build.h"
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_common/max_scene.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MESHEVAL;
using namespace MATBUILD;
using namespace MESHBUILD;

// NeL vegetable appdata sub-ids (plugin_max/nel_mesh_lib/export_appdata.h)
#define NEL3D_APPDATA_VEGETABLE 1423062580
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND 1423062581
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED 1423062582
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED 1423062583
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED 1423062584
#define NEL3D_APPDATA_BEND_CENTER 1423062585
#define NEL3D_APPDATA_BEND_FACTOR 1423062586
#define NEL3D_APPDATA_VEGETABLE_FORCE_BEST_SIDED_LIGHTING 1423062616
#define NEL3D_APPDATA_DONOTEXPORT 1423062565
#define NEL3D_APPDATA_CHARACTER_LOD 1423062634

#define NEL3D_APPDATA_BEND_FACTOR_DEFAULT 1.0f

// Windows tri-state checkbox constants (BST_UNCHECKED=0, BST_CHECKED=1) — the reference impl
// mixes 0/1/2 semantics via getScriptAppData integer comparisons; reproduce the same values.
#define VEG_BST_UNCHECKED 0
#define VEG_BST_CHECKED 1

// PS class part-A id (skip nel_ps nodes)
#define CLASSID_PARTA_NEL_PS 0x58ce2893

static bool g_verbose = false;

// ---------------------------------------------------------------------------------------------

// Is this node a scene-root geometry node eligible for vegetable export?
static bool isToBeExported(INode &node)
{
	// Root only: parent must be the scene root (not a CNodeImpl)
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (!n) return false;
	INode *p = n->parent();
	if (p && dynamic_cast<CNodeImpl *>(p))
		return false; // has a non-root parent

	std::string name = nodeName(node);
	if (name == "Bip01") return false;

	CSceneClass *base = baseObjectOf(node, NULL, NULL);
	if (!base) return false;

	// Skip nel_ps
	NLMISC::CClassId cid = base->classDesc()->classId();
	if (cid.a() == CLASSID_PARTA_NEL_PS)
		return false;

	// Reference gates on VEGETABLE appdata == "1"
	std::string veg = getScriptAppDataStr(n, NEL3D_APPDATA_VEGETABLE, "");
	if (veg != "1") return false;

	// Reference does not explicitly skip DONOTEXPORT in isToBeExported (the maxscript checks it
	// before the VEGETABLE check), so honor it here for parity.
	if (getScriptAppDataStr(n, NEL3D_APPDATA_DONOTEXPORT, "") == "1")
		return false;

	return true;
}

// Build a CVegetableShape from a node — the mesh path stays common with the shape exporter
// (same evalNodeMesh + buildMeshInterface); vegetables just add appdata flags on top and enforce
// the 1-matblock/1-rdrpass/UV1 constraint that CVegetableShapeBuild requires.
static bool buildVegetableShape(INode &node, SNodeTMCache &tmCache, NL3D::CVegetableShape &out,
                                bool exportLighting)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	std::string name = nodeName(node);

	// Base mesh (materials, transform)
	Matrix3M localTM = getLocalMatrix(node, tmCache);
	SMaxMeshBaseBuild maxBaseBuild;
	NL3D::CMeshBase::CMeshBaseBuild buildBaseMesh;
	buildBaseMeshInterface(buildBaseMesh, maxBaseBuild, node, tmCache, localTM, exportLighting);

	// Mesh eval
	SEvalMesh evalMesh;
	std::vector<std::string> warnings;
	if (!evalNodeMesh(node, evalMesh, &warnings))
	{
		fprintf(stderr, "ERROR: veget '%s' mesh eval failed\n", name.c_str());
		return false;
	}
	NL3D::CMesh::CMeshBuild buildMesh;
	buildMeshInterface(evalMesh, buildMesh, buildBaseMesh, maxBaseBuild, node, tmCache);

	// Verify UV1 present (reference outputs error message + returns false without a UV1)
	if ((buildMesh.VertexFlags & NL3D::CVertexBuffer::TexCoord0Flag) == 0)
	{
		fprintf(stderr, "ERROR: veget '%s' needs UV1 coordinates\n", name.c_str());
		return false;
	}

	// Build a plain CMesh so we can inspect matrix blocks / rdrpass counts / VB / PB
	NL3D::CMesh mesh;
	mesh.build(buildBaseMesh, buildMesh);

	if (mesh.getNbMatrixBlock() != 1)
	{
		fprintf(stderr, "ERROR: veget '%s' must have exactly 1 matrix block (has %u)\n",
		        name.c_str(), mesh.getNbMatrixBlock());
		return false;
	}
	if (mesh.getNbRdrPass(0) != 1)
	{
		fprintf(stderr, "ERROR: veget '%s' must have exactly 1 render pass (has %u)\n",
		        name.c_str(), mesh.getNbRdrPass(0));
		return false;
	}

	// Fill the veget build
	NL3D::CVegetableShapeBuild vegetableBuild;
	vegetableBuild.VB = mesh.getVertexBuffer();
	vegetableBuild.PB = mesh.getRdrPassPrimitiveBlock(0, 0);

	// AlphaBlend — reference: `getScriptAppData(node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0) == 0`
	// means AlphaBlend TRUE. The default when the appdata is missing is 0 (== 0 → true).
	vegetableBuild.AlphaBlend = (getScriptAppDataInt(n, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0) == 0);

	if (vegetableBuild.AlphaBlend)
	{
		vegetableBuild.PreComputeLighting = true;
		vegetableBuild.DoubleSided = true;
		vegetableBuild.Lighted = (getScriptAppDataInt(n, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, 0) == 0);
	}
	else
	{
		// Lighted ?
		vegetableBuild.Lighted = (getScriptAppDataInt(n, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, 0) != 2);
		// PreComputeLighting ?
		vegetableBuild.PreComputeLighting = (getScriptAppDataInt(n, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, 0) == 0);
		// DoubleSided ?
		vegetableBuild.DoubleSided = (getScriptAppDataInt(n, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, 0) != VEG_BST_UNCHECKED);
	}

	if (vegetableBuild.PreComputeLighting)
	{
		vegetableBuild.BestSidedPreComputeLighting = (getScriptAppDataInt(n, NEL3D_APPDATA_VEGETABLE_FORCE_BEST_SIDED_LIGHTING, 0) != VEG_BST_UNCHECKED);
	}

	// Max bend weight — default 1.0f. Reference reads as int-into-float via getScriptAppData;
	// the appdata value is stored as a float on the node.
	vegetableBuild.MaxBendWeight = getScriptAppDataFloat(n, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT);

	// BendCenter mode
	int bcm = getScriptAppDataInt(n, NEL3D_APPDATA_BEND_CENTER, 0);
	vegetableBuild.BendCenterMode = (NL3D::CVegetableShapeBuild::TBendCenterMode)bcm;

	out.build(vegetableBuild);
	return true;
}

// ---------------------------------------------------------------------------------------------
// Per-file export

static int exportFile(const std::string &maxPath, const std::string &outDir, bool exportLighting,
                      uint &exported, uint &skipped)
{
	SLoadedMax lm;
	if (!loadMaxFile(maxPath, lm))
		return 1;

	CSceneClassContainer *ssc = lm.Scene->container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = NULL;

	std::vector<INode *> allNodes;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		allNodes.push_back(node);
	}

	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode &node = *allNodes[i];
		if (!isToBeExported(node))
			continue;

		std::string name = nodeName(node);
		std::string outPath = outDir + "/" + name + ".veget";

		try
		{
			NL3D::CVegetableShape shape;
			if (!buildVegetableShape(node, tmCache, shape, exportLighting))
			{
				++skipped;
				continue;
			}

			NLMISC::COFile file;
			if (!file.open(outPath))
			{
				fprintf(stderr, "ERROR: cannot open %s for writing\n", outPath.c_str());
				++skipped;
				continue;
			}
			shape.serial(file);
			file.close();
			++exported;
			if (g_verbose)
				printf("OK %s\n", outPath.c_str());
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: veget serialization failed for %s: %s\n", outPath.c_str(), e.what());
			++skipped;
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext appContext;
	NL3D::registerSerial3d();

	NL3D::CVertexBuffer::SerialOldPreferredMemory = true;
	NL3D::CIndexBuffer::SerialOldPreferredMemory = true;

	std::string input, outDir, dbRoot;
	bool exportLighting = true;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--verbose" || arg == "-v")
			g_verbose = true;
		else if (arg == "--db" && i + 1 < argc)
			dbRoot = argv[++i];
		else if (arg == "--path-alias" && i + 1 < argc)
		{
			std::string kv = argv[++i];
			std::string::size_type eq = kv.find('=');
			if (eq == std::string::npos)
				fprintf(stderr, "WARNING: --path-alias expects <prefix>=<root>, got '%s'\n", kv.c_str());
			else
				DBPATH::addAlias(kv.substr(0, eq), kv.substr(eq + 1));
		}
		else if (arg == "--no-lighting")
			exportLighting = false;
		else if (input.empty())
			input = arg;
		else if (outDir.empty())
			outDir = arg;
		else
		{
			fprintf(stderr, "unexpected argument: %s\n", arg.c_str());
			return 2;
		}
	}

	if (input.empty() || outDir.empty())
	{
		fprintf(stderr, "usage: pipeline_max_export_veget [--db <graphics-root>] [--no-lighting] [-v] <input.max> <outdir>\n");
		return 2;
	}

	// Deduce db root from input path when not given
	if (dbRoot.empty())
	{
		std::string abs = NLMISC::CPath::getFullPath(input, false);
		std::string::size_type p = abs.find("/stuff/");
		if (p == std::string::npos) p = abs.find("/landscape/");
		if (p == std::string::npos) p = abs.find("/sky_v2/");
		if (p != std::string::npos)
			dbRoot = abs.substr(0, p);
	}
	setDatabaseRoot(dbRoot);

	uint exported = 0, skipped = 0;
	int ret = exportFile(input, outDir, exportLighting, exported, skipped);
	printf("EXPORTED %u vegets, %u skipped\n", exported, skipped);
	return ret;
}

/* end of file */
