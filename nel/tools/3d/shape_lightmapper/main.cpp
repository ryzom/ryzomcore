/**
 * \file main.cpp
 * \brief Standalone shape lightmapper — the 2_build half of the standalone-lightmapper design
 * (pipeline_max_design.md §11-lm). Reads a lightmap scene-graph intermediate (.lmscene, written
 * at 1_export by the headless .max exporter / the in-Max plugin / the assimp exporter), runs
 * the CMeshLightmapper library (the calc_lm port) on every receiver, finishes the shape build
 * from the recipe carried in the file, and emits the lightmapped .shape files plus the packed
 * lightmap textures — the content the export stage used to produce in-Max, feeding the same
 * downstream lightmap_optimizer.
 *
 * Project options (lumel size, oversampling, shadows) come from the command line — they are
 * build configuration, never baked into the scene file.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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
#include <nel/misc/app_context.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/common.h>

#include <nel/3d/register_3d.h>
#include <nel/3d/shape.h>
#include <nel/3d/mesh.h>
#include <nel/3d/mesh_mrm.h>
#include <nel/3d/mesh_multi_lod.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/lightmap_scene.h>
#include <nel/3d/mesh_lightmapper.h>
#include <nel/3d/mrm_parameters.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
#include <process.h>
#define SLM_GETPID _getpid
#else
#include <unistd.h>
#define SLM_GETPID getpid
#endif

using namespace NLMISC;
using namespace NL3D;

// ---------------------------------------------------------------------------------------------
// Shape stream writer with the export-era version patches — the same discipline as
// pipeline_max_export_shape's exportFile (see the comments there): serialize through a real
// temp file (CMeshMRMGeom::save seeks past the CMemStream write position), then patch the
// CMeshBase version byte 10 -> 9 (the reference corpus era).
static bool writeShapeCompat(NL3D::IShape *shape, const std::string &outPath)
{
	char tmpPath[256];
	sprintf(tmpPath, "/tmp/shape_lightmapper.%d.tmp", (int)SLM_GETPID());
	{
		NLMISC::COFile ofile;
		if (!ofile.open(tmpPath))
		{
			fprintf(stderr, "ERROR: cannot open %s for writing\n", tmpPath);
			return false;
		}
		NL3D::CShapeStream shapeStream(shape);
		shapeStream.serial(ofile);
		ofile.close();
	}
	std::vector<uint8> memBuf;
	{
		NLMISC::CIFile ifile;
		if (!ifile.open(tmpPath))
		{
			fprintf(stderr, "ERROR: cannot open %s for reading\n", tmpPath);
			return false;
		}
		memBuf.resize(ifile.getFileSize());
		if (!memBuf.empty())
			ifile.serialBuffer(&memBuf[0], (uint)memBuf.size());
		ifile.close();
	}
	CFile::deleteFile(tmpPath);
	uint8 *buf = memBuf.empty() ? NULL : &memBuf[0];
	uint32 len = (uint32)memBuf.size();
	{
		std::string className = shape->getClassName();
		uint32 meshBaseVerOff = 4 + 8 + 4 + (uint32)className.size() + 1;
		if ((className == "CMesh" || className == "CMeshMRM" || className == "CMeshMRMSkinned"
			|| className == "CMeshMultiLod")
			&& meshBaseVerOff < len && buf[meshBaseVerOff] == 10)
			buf[meshBaseVerOff] = 9;
	}
	NLMISC::COFile file;
	if (!file.open(outPath))
	{
		fprintf(stderr, "ERROR: cannot open %s for writing\n", outPath.c_str());
		return false;
	}
	file.serialBuffer(buf, len);
	file.close();
	return true;
}

// ---------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	NLMISC::CApplicationContext appContext;
	NL3D::registerSerial3d();

	// Export-era stream versions (the reference corpus predates the buffer-usage refactor)
	NL3D::CVertexBuffer::SerialOldPreferredMemory = true;
	NL3D::CIndexBuffer::SerialOldPreferredMemory = true;

	std::string scenePath, outDir, outDirCoarse, lightmapDir;
	CMeshLightmapper::COptions options;
	options.bShadow = true; // ShapeExportOptShadow — the full-quality reference setting
	options.rLumelSize = 0.25f;
	options.nOverSampling = 1;
	bool verbose = false;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "-v" || arg == "--verbose")
			verbose = true;
		else if (arg == "--lumel-size" && i + 1 < argc)
			NLMISC::fromString(std::string(argv[++i]), options.rLumelSize);
		else if (arg == "--oversampling" && i + 1 < argc)
			NLMISC::fromString(std::string(argv[++i]), options.nOverSampling);
		else if (arg == "--no-shadow")
			options.bShadow = false;
		else if (arg == "--lighting-limit" && i + 1 < argc)
			NLMISC::fromString(std::string(argv[++i]), options.nExportLighting);
		else if (arg == "--lightmap-log")
			options.OutputLightmapLog = true;
		else if (arg == "--show-lumel")
			options.bShowLumel = true;
		else if (arg == "--lightmaps" && i + 1 < argc)
			lightmapDir = argv[++i];
		else if (arg == "--coarse-out" && i + 1 < argc)
			outDirCoarse = argv[++i];
		else if (arg == "--texture-path" && i + 1 < argc)
			CPath::addSearchPath(argv[++i], true, false);
		else if (scenePath.empty())
			scenePath = arg;
		else if (outDir.empty())
			outDir = arg;
		else
		{
			fprintf(stderr, "unexpected argument: %s\n", arg.c_str());
			return 2;
		}
	}

	if (scenePath.empty() || outDir.empty())
	{
		fprintf(stderr,
			"usage: shape_lightmapper [options] <scene.lmscene> <out-shapes-dir>\n"
			"  --lightmaps <dir>      lightmap texture output (default: <out-shapes-dir>)\n"
			"  --coarse-out <dir>     with-coarse-mesh shape output (default: <out-shapes-dir>)\n"
			"  --lumel-size <f>       lumel size in world units (default 0.25)\n"
			"  --oversampling <n>     oversampling factor (default 1)\n"
			"  --no-shadow            disable raytraced shadows\n"
			"  --lighting-limit <n>   0 = normal, 1 = soft shadows on directionals\n"
			"  --lightmap-log         write the per-lightmap light list log\n"
			"  --texture-path <dir>   search path for occluder transparency textures\n");
		return 2;
	}
	if (outDirCoarse.empty())
		outDirCoarse = outDir;
	if (lightmapDir.empty())
		lightmapDir = outDir;
	options.sExportLighting = lightmapDir;

	CLightmapScene scene;
	try
	{
		scene.load(scenePath);
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: cannot load %s: %s\n", scenePath.c_str(), e.what());
		return 1;
	}
	if (verbose)
		printf("scene '%s': %u receivers, %u occluders, %u lights\n", scene.ProjectName.c_str(),
		       (uint)scene.Receivers.size(), (uint)scene.Occluders.size(), (uint)scene.Lights.size());

	if (getenv("SLM_DUMP_SCENE"))
	{
		for (uint i = 0; i < scene.Lights.size(); ++i)
		{
			const CLightmapLight &l = scene.Lights[i];
			printf("LIGHT '%s' type=%d grp=%u anim='%s' pos=(%g %g %g) dir=(%g %g %g) rmin=%g rmax=%g"
			       " hot=%g fall=%g mult=%g shadow=%d ambOnly=%d diff=(%u %u %u)\n",
			       l.Name.c_str(), (int)l.Type, l.LightGroup, l.AnimatedLight.c_str(),
			       l.Position.x, l.Position.y, l.Position.z, l.Direction.x, l.Direction.y, l.Direction.z,
			       l.rRadiusMin, l.rRadiusMax, l.rHotspot, l.rFallof, l.rMult,
			       (int)l.bCastShadow, (int)l.bAmbientOnly, l.Diffuse.R, l.Diffuse.G, l.Diffuse.B);
		}
		for (uint i = 0; i < scene.Receivers.size(); ++i)
		{
			const CLightmapReceiver &recv = scene.Receivers[i];
			for (uint g = 0; g < recv.Geoms.size(); ++g)
			{
				const CLightmapReceiverGeom &geom = recv.Geoms[g];
				NLMISC::CMatrix m = CMeshLightmapper::getObjectToWorldMatrix(&geom.MeshBuild, &recv.BaseBuild);
				NLMISC::CAABBox box;
				if (!geom.MeshBuild.Vertices.empty())
				{
					box.setCenter(m * geom.MeshBuild.Vertices[0]);
					for (uint v = 1; v < geom.MeshBuild.Vertices.size(); ++v)
						box.extend(m * geom.MeshBuild.Vertices[v]);
				}
				printf("RECV '%s' geom '%s' verts=%u bbox=(%g %g %g)-(%g %g %g)\n",
				       recv.NodeName.c_str(), geom.NodeName.c_str(), (uint)geom.MeshBuild.Vertices.size(),
				       box.getMin().x, box.getMin().y, box.getMin().z,
				       box.getMax().x, box.getMax().y, box.getMax().z);
			}
		}
		fflush(stdout);
		return 0; // dump-only mode
	}

	uint exported = 0, failed = 0, deferred = 0;

	for (uint r = 0; r < scene.Receivers.size(); ++r)
	{
		CLightmapReceiver &recv = scene.Receivers[r];
		if (recv.Geoms.empty())
			continue;

		if (recv.MultiLod)
		{
			// Multi-lod receivers: the per-slot base-material flow of the original exporter
			// is not replicated yet (each slot ran calc_lm against its own slave base build
			// in-Max; the material interaction with the shared multi-lod base needs its own
			// validated round). Deferred — the receiver record carries everything needed.
			fprintf(stderr, "DEFERRED multi-lod receiver '%s' (%u geoms) — not lightmapped yet\n",
			        recv.NodeName.c_str(), (uint)recv.Geoms.size());
			++deferred;
			continue;
		}

		CLightmapReceiverGeom &geom = recv.Geoms[0];

		bool lit = false;
		try
		{
			lit = CMeshLightmapper::calculateLM(&geom.MeshBuild, &recv.BaseBuild, scene, geom, options);
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: lightmap computation failed for '%s': %s\n",
			        recv.NodeName.c_str(), e.what());
			++failed;
			continue;
		}
		if (verbose)
			printf("%s '%s'\n", lit ? "LIT" : "UNLIT", recv.NodeName.c_str());

		// Finish the shape build from the recipe (the reference flow continues into the
		// ordinary build whether or not calculateLM produced lightmaps).
		NL3D::CMeshBase *meshBase = NULL;
		std::vector<sint> materialRemap;
		try
		{
			if (recv.WantMrm)
			{
				NL3D::CMRMParameters parameters;
				parameters.NLods = recv.MrmNLods;
				parameters.Divisor = recv.MrmDivisor;
				parameters.SkinReduction = (NL3D::CMRMParameters::TSkinReduction)recv.MrmSkinReduction;
				parameters.DistanceFinest = recv.MrmDistanceFinest;
				parameters.DistanceMiddle = recv.MrmDistanceMiddle;
				parameters.DistanceCoarsest = recv.MrmDistanceCoarsest;
				std::vector<NL3D::CMesh::CMeshBuild *> bsList; // morph+MRM+lightmap: zero corpus
				NL3D::CMeshMRM *meshMRM = new NL3D::CMeshMRM;
				meshMRM->build(recv.BaseBuild, geom.MeshBuild, bsList, parameters);
				meshMRM->optimizeMaterialUsage(materialRemap);
				meshBase = meshMRM;
			}
			else
			{
				NL3D::CMesh *m = new NL3D::CMesh;
				m->build(recv.BaseBuild, geom.MeshBuild);
				m->optimizeMaterialUsage(materialRemap);
				meshBase = m;
			}
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: shape build failed for '%s': %s\n", recv.NodeName.c_str(), e.what());
			delete meshBase;
			++failed;
			continue;
		}

		// Animated materials (pre-remap names carried in the recipe)
		if (recv.AnimatedMaterials)
		{
			for (uint i = 0; i < recv.MaterialNames.size() && i < materialRemap.size(); ++i)
			{
				sint dst = materialRemap[i];
				if (dst >= 0)
					meshBase->setAnimatedMaterial(dst, recv.MaterialNames[i]);
			}
		}
		if (recv.AutoAnim)
			meshBase->setAutoAnim(true);
		meshBase->setDistMax(recv.DistMax);

		std::string outPath = (recv.CoarseOutput ? outDirCoarse : outDir) + "/"
			+ NLMISC::toLowerAscii(recv.NodeName) + ".shape";
		try
		{
			if (writeShapeCompat(meshBase, outPath))
			{
				++exported;
				if (verbose)
					printf("OK %s\n", outPath.c_str());
			}
			else
				++failed;
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: shape serialization failed for %s: %s\n", outPath.c_str(), e.what());
			++failed;
		}
		delete meshBase;
	}

	printf("LIGHTMAPPED %u shapes, %u failed, %u deferred (multi-lod)\n", exported, failed, deferred);
	return failed ? 1 : 0;
}

/* end of file */
