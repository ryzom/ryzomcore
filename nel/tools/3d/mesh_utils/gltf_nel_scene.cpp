/**
 * \file gltf_nel_scene.cpp
 * \brief See gltf_nel_scene.h. The reconstruction inverts the encoding documented in
 * pipeline_max_export_gltf/gltf_build.cpp (and wiki drafts/nel_gltf_extras.md): original
 * vertex array from nel_vertices, faces scattered back into nel_faces order with corners
 * rebuilt from primitive attributes + nel_vertex_ids, materials from the nel_* extras codec.
 * The shape build then replays exactly what pipeline_max_export_shape's buildShapeForNode does
 * for the mesh path (CMesh / CMeshMRM(Skinned) via the same NL3D calls, optimizeMaterialUsage,
 * animated-material names, auto-anim, dist-max, the CMeshBase v10->v9 version-byte patch and
 * the SerialOldPreferredMemory stream versions) — identical inputs through identical code, so
 * the output .shape is byte-identical to the direct route's on the same host.
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
#include "gltf_nel_scene.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/3d/mesh.h>
#include <nel/3d/mesh_mrm.h>
#include <nel/3d/mesh_mrm_skinned.h>
#include <nel/3d/mrm_parameters.h>
#include <nel/3d/shape.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>

#ifdef NL_OS_WINDOWS
#include <process.h>
#define NLGLTF_GETPID _getpid
#else
#include <unistd.h>
#define NLGLTF_GETPID getpid
#endif

#include "../nel_gltf/json_value.h"
#include "../nel_gltf/gltf_material.h"

#include "mesh_utils.h"

using namespace NLMISC;
using namespace NL3D;
using namespace NLGLTF;

namespace {

// ---------------------------------------------------------------------------------------------
// Document + accessor access (strict to what pipeline_max_export_gltf emits: one buffer with a
// uri, one tightly-packed bufferView per accessor, no accessor byteOffset)

struct SGltfDoc
{
	CJsonValue Json;
	std::vector<uint8> Bin;
	std::string Path;

	bool load(const std::string &path, std::string *err)
	{
		Path = path;
		std::string content;
		{
			CIFile f;
			if (!f.open(path))
			{
				if (err) *err = "cannot open " + path;
				return false;
			}
			content.resize(f.getFileSize());
			if (!content.empty())
				f.serialBuffer((uint8 *)&content[0], (uint)content.size());
		}
		if (!Json.parse(content, err))
			return false;
		const CJsonValue *buffers = Json.get("buffers");
		if (buffers && buffers->size())
		{
			const CJsonValue *buf = buffers->at(0);
			std::string uri = buf->getString("uri", "");
			if (uri.empty())
			{
				if (err) *err = "buffer without uri";
				return false;
			}
			// Tolerate an empty buffer (mesh-less scene, e.g. ig-only or all-skip files):
			// CIFile::open refuses zero-byte files.
			if (buf->getInt("byteLength", 0) > 0)
			{
				std::string binPath = CFile::getPath(path) + uri;
				CIFile f;
				if (!f.open(binPath))
				{
					if (err) *err = "cannot open " + binPath;
					return false;
				}
				Bin.resize(f.getFileSize());
				if (!Bin.empty())
					f.serialBuffer(&Bin[0], (uint)Bin.size());
			}
		}
		return true;
	}

	// Raw accessor payload; validates componentType/type and returns element count.
	const uint8 *accessor(sint idx, int componentType, const char *type, size_t elemBytes,
	                      size_t &count, std::string *err) const
	{
		const CJsonValue *accessors = Json.get("accessors");
		const CJsonValue *views = Json.get("bufferViews");
		const CJsonValue *ac = accessors ? accessors->at((size_t)idx) : NULL;
		if (!ac)
		{
			if (err) *err = "missing accessor";
			return NULL;
		}
		if (ac->getInt("componentType", 0) != componentType || ac->getString("type", "") != type
			|| ac->getInt("byteOffset", 0) != 0)
		{
			if (err) *err = "accessor format mismatch";
			return NULL;
		}
		count = (size_t)ac->getInt("count", 0);
		const CJsonValue *bv = views ? views->at((size_t)ac->getInt("bufferView", -1)) : NULL;
		if (!bv)
		{
			if (err) *err = "missing bufferView";
			return NULL;
		}
		if (bv->get("byteStride"))
		{
			if (err) *err = "strided bufferView not supported";
			return NULL;
		}
		size_t off = (size_t)bv->getInt("byteOffset", 0);
		size_t len = (size_t)bv->getInt("byteLength", 0);
		if (len < count * elemBytes || off + len > Bin.size())
		{
			if (err) *err = "bufferView out of range";
			return NULL;
		}
		return &Bin[off];
	}

	const float *floats(sint idx, const char *type, int nComp, size_t &count, std::string *err) const
	{
		return (const float *)accessor(idx, 5126, type, (size_t)nComp * 4, count, err);
	}
	const uint32 *u32s(sint idx, size_t &count, std::string *err) const
	{
		return (const uint32 *)accessor(idx, 5125, "SCALAR", 4, count, err);
	}
	const uint8 *u8vec4(sint idx, size_t &count, std::string *err) const
	{
		return accessor(idx, 5121, "VEC4", 4, count, err);
	}
};

// ---------------------------------------------------------------------------------------------

struct SStats
{
	uint Exported;
	uint Igs;
	uint Skipped;
	std::map<std::string, uint> SkipReasons;
	SStats() : Exported(0), Igs(0), Skipped(0) { }
	void skip(const std::string &r)
	{
		++Skipped;
		++SkipReasons[r];
	}
};

bool reconstructMeshBuild(const SGltfDoc &doc, const CJsonValue &mesh, const CJsonValue &nodeExtras,
                          CMesh::CMeshBuild &mb, std::string *err)
{
	const CJsonValue *mex = mesh.get("extras");
	if (!mex)
	{
		if (err) *err = "mesh without extras";
		return false;
	}

	mb.VertexFlags = (sint32)mex->getInt("nel_vertex_flags", 0);
	{
		const CJsonValue *r = mex->get("nel_uv_routing");
		for (uint i = 0; i < CVertexBuffer::MaxStage; ++i)
		{
			const CJsonValue *v = r ? r->at(i) : NULL;
			mb.UVRouting[i] = v ? (uint8)v->asInt() : (uint8)i;
		}
	}
	std::vector<uint> uvStages;
	{
		const CJsonValue *st = mex->get("nel_uv_stages");
		for (size_t s = 0; st && s < st->size(); ++s)
			uvStages.push_back((uint)st->at(s)->asInt());
	}

	// Original vertex array
	{
		sint acc = (sint)mex->getInt("nel_vertices", -1);
		if (acc >= 0)
		{
			size_t count = 0;
			const float *v = doc.floats(acc, "VEC3", 3, count, err);
			if (!v) return false;
			mb.Vertices.resize(count);
			for (size_t i = 0; i < count; ++i)
				mb.Vertices[i].set(v[i * 3], v[i * 3 + 1], v[i * 3 + 2]);
		}
	}

	// Default transform from the node's bit-exact TRS extras (same values
	// buildBaseMeshInterface derived from the local matrix on the writer side)
	(void)nodeExtras;

	// Faces from the primitives, scattered into original global order
	const CJsonValue *prims = mesh.get("primitives");
	size_t totalFaces = 0;
	for (size_t p = 0; prims && p < prims->size(); ++p)
	{
		const CJsonValue *prim = prims->at(p);
		const CJsonValue *pex = prim->get("extras");
		if (!pex)
		{
			if (err) *err = "primitive without extras";
			return false;
		}
		size_t nf = 0;
		std::string dummy;
		const uint32 *faces = doc.u32s((sint)pex->getInt("nel_faces", -1), nf, err);
		if (!faces) return false;
		totalFaces += nf;
	}
	mb.Faces.resize(totalFaces);
	std::vector<bool> faceSeen(totalFaces, false);

	for (size_t p = 0; prims && p < prims->size(); ++p)
	{
		const CJsonValue *prim = prims->at(p);
		const CJsonValue *pex = prim->get("extras");
		const CJsonValue *attrs = prim->get("attributes");
		if (!attrs)
		{
			if (err) *err = "primitive without attributes";
			return false;
		}
		sint matId = (sint)pex->getInt("nel_material_id", -1);

		size_t nVerts = 0, nIdx = 0, nFaces = 0, nSm = 0, nVid = 0, nColor = 0;
		const float *pos = doc.floats((sint)attrs->getInt("POSITION", -1), "VEC3", 3, nVerts, err);
		if (!pos) return false;
		const float *norm = doc.floats((sint)attrs->getInt("NORMAL", -1), "VEC3", 3, nVerts, err);
		if (!norm) return false;
		std::vector<const float *> uvs(uvStages.size(), (const float *)NULL);
		for (size_t s = 0; s < uvStages.size(); ++s)
		{
			char an[24];
			snprintf(an, sizeof(an), "TEXCOORD_%u", (uint)s);
			uvs[s] = doc.floats((sint)attrs->getInt(an, -1), "VEC2", 2, nVerts, err);
			if (!uvs[s]) return false;
		}
		const uint8 *colors = NULL;
		if ((uint32)mb.VertexFlags & CVertexBuffer::PrimaryColorFlag)
		{
			colors = doc.u8vec4((sint)attrs->getInt("COLOR_0", -1), nColor, err);
			if (!colors) return false;
		}
		const uint32 *indices = doc.u32s((sint)prim->getInt("indices", -1), nIdx, err);
		if (!indices) return false;
		const uint32 *faces = doc.u32s((sint)pex->getInt("nel_faces", -1), nFaces, err);
		if (!faces) return false;
		const uint32 *smGroups = doc.u32s((sint)pex->getInt("nel_smgroups", -1), nSm, err);
		if (!smGroups) return false;
		const uint32 *vertexIds = doc.u32s((sint)pex->getInt("nel_vertex_ids", -1), nVid, err);
		if (!vertexIds) return false;
		if (nIdx != nFaces * 3 || nSm != nFaces || nVid != nVerts)
		{
			if (err) *err = "primitive extras count mismatch";
			return false;
		}

		for (size_t k = 0; k < nFaces; ++k)
		{
			uint32 gf = faces[k];
			if (gf >= totalFaces || faceSeen[gf])
			{
				if (err) *err = "bad nel_faces ordering";
				return false;
			}
			faceSeen[gf] = true;
			CMesh::CFace &face = mb.Faces[gf];
			face.MaterialId = matId;
			face.SmoothGroup = (sint32)smGroups[k];
			for (uint c = 0; c < 3; ++c)
			{
				uint32 gv = indices[k * 3 + c];
				if (gv >= nVerts)
				{
					if (err) *err = "index out of range";
					return false;
				}
				CMesh::CCorner &corner = face.Corner[c];
				corner.Vertex = (sint32)vertexIds[gv];
				if (corner.Vertex < 0 || (size_t)corner.Vertex >= mb.Vertices.size())
				{
					if (err) *err = "nel_vertex_ids out of range";
					return false;
				}
				corner.Normal.set(norm[gv * 3], norm[gv * 3 + 1], norm[gv * 3 + 2]);
				for (size_t s = 0; s < uvStages.size(); ++s)
				{
					corner.Uvws[uvStages[s]].U = uvs[s][gv * 2];
					corner.Uvws[uvStages[s]].V = uvs[s][gv * 2 + 1];
					corner.Uvws[uvStages[s]].W = 0.0f;
				}
				if (colors)
				{
					corner.Color.R = colors[gv * 4];
					corner.Color.G = colors[gv * 4 + 1];
					corner.Color.B = colors[gv * 4 + 2];
					corner.Color.A = colors[gv * 4 + 3];
				}
			}
		}
	}
	for (size_t f = 0; f < totalFaces; ++f)
		if (!faceSeen[f])
		{
			if (err) *err = "nel_faces does not cover the face range";
			return false;
		}

	// Interface weld state
	{
		const CJsonValue *counts = mex->get("nel_iface_counts");
		if (counts && counts->size())
		{
			size_t nPos = 0, nNorm = 0;
			const float *ipos = doc.floats((sint)mex->getInt("nel_iface_pos", -1), "VEC3", 3, nPos, err);
			if (!ipos) return false;
			const float *inorm = doc.floats((sint)mex->getInt("nel_iface_norm", -1), "VEC3", 3, nNorm, err);
			if (!inorm) return false;
			size_t at = 0;
			mb.Interfaces.resize(counts->size());
			for (size_t m = 0; m < counts->size(); ++m)
			{
				size_t n = (size_t)counts->at(m)->asInt();
				if (at + n > nPos || nPos != nNorm)
				{
					if (err) *err = "nel_iface_counts out of range";
					return false;
				}
				mb.Interfaces[m].Vertices.resize(n);
				for (size_t k = 0; k < n; ++k)
				{
					mb.Interfaces[m].Vertices[k].Pos.set(ipos[(at + k) * 3], ipos[(at + k) * 3 + 1], ipos[(at + k) * 3 + 2]);
					mb.Interfaces[m].Vertices[k].Normal.set(inorm[(at + k) * 3], inorm[(at + k) * 3 + 1], inorm[(at + k) * 3 + 2]);
				}
				at += n;
			}
		}
		const CJsonValue *links = mex->get("nel_iface_links");
		if (links && links->size())
		{
			if (links->size() != mb.Vertices.size() * 2)
			{
				if (err) *err = "nel_iface_links count mismatch";
				return false;
			}
			mb.InterfaceLinks.resize(mb.Vertices.size());
			mb.InterfaceVertexFlag.resize((uint)mb.Vertices.size());
			for (size_t k = 0; k < mb.InterfaceLinks.size(); ++k)
			{
				mb.InterfaceLinks[k].InterfaceId = (sint)links->at(k * 2)->asInt();
				mb.InterfaceLinks[k].InterfaceVertexId = (uint)links->at(k * 2 + 1)->asInt();
				if (mb.InterfaceLinks[k].InterfaceId >= 0)
					mb.InterfaceVertexFlag.set((uint)k);
			}
		}
	}

	mb.VertLink.clear();
	mb.MeshVertexProgram = NULL;
	return true;
}

bool reconstructBaseBuild(const SGltfDoc &doc, const CJsonValue &node, const CJsonValue &mesh,
                          CMeshBase::CMeshBaseBuild &bbm, std::vector<std::string> &matNames,
                          std::string *err)
{
	const CJsonValue *extras = node.get("extras");
	const CJsonValue *mex = mesh.get("extras");
	if (!extras || !mex)
	{
		if (err) *err = "missing extras";
		return false;
	}

	bbm.bCastShadows = extras->getBool("nel_cast_shadows", true);
	bbm.bRcvShadows = extras->getBool("nel_rcv_shadows", true);
	bbm.UseLightingLocalAttenuation = extras->getBool("nel_lighting_local_atten", false);
	bbm.CollisionMeshGeneration = (CMeshBase::TCameraCollisionGenerate)extras->getInt("nel_collision_mesh_gen", 0);

	bbm.DefaultPos.set((float)extras->getDouble("nel_tx", 0.0), (float)extras->getDouble("nel_ty", 0.0),
	                   (float)extras->getDouble("nel_tz", 0.0));
	bbm.DefaultRotQuat.set((float)extras->getDouble("nel_rx", 0.0), (float)extras->getDouble("nel_ry", 0.0),
	                       (float)extras->getDouble("nel_rz", 0.0), (float)extras->getDouble("nel_rw", 1.0));
	bbm.DefaultScale.set((float)extras->getDouble("nel_sx", 1.0), (float)extras->getDouble("nel_sy", 1.0),
	                     (float)extras->getDouble("nel_sz", 1.0));
	bbm.DefaultRotEuler.set(0, 0, 0);
	bbm.DefaultPivot.set(0, 0, 0);

	// Morph channel defaults
	{
		const CJsonValue *bsn = extras->get("nel_bs_names");
		const CJsonValue *bsf = extras->get("nel_bs_factors");
		for (size_t i = 0; bsn && i < bsn->size(); ++i)
		{
			bbm.BSNames.push_back(bsn->at(i)->asString());
			bbm.DefaultBSFactors.push_back(bsf && i < bsf->size() ? bsf->at(i)->asFloat() : 0.0f);
		}
	}

	// Materials
	const CJsonValue *mats = mex->get("nel_materials");
	const CJsonValue *gltfMats = doc.Json.get("materials");
	if (!mats)
	{
		if (err) *err = "mesh without nel_materials";
		return false;
	}
	bbm.Materials.resize(mats->size());
	matNames.resize(mats->size());
	for (size_t i = 0; i < mats->size(); ++i)
	{
		sint gi = (sint)mats->at(i)->asInt();
		const CJsonValue *jm = gltfMats ? gltfMats->at((size_t)gi) : NULL;
		const CJsonValue *jex = jm ? jm->get("extras") : NULL;
		if (!jex)
		{
			if (err) *err = "material without nel extras";
			return false;
		}
		if (!materialFromExtras(*jex, bbm.Materials[i], err))
			return false;
		matNames[i] = jm->getString("name", "");
	}

	return true;
}

// Serialize a shape with the export-era stream conventions: temp COFile (CMeshMRMGeom's save
// seeks), then the CMeshBase version byte 10 -> 9 patch — same block as the direct exporter.
bool writeShapeFile(IShape *shape, const std::string &outPath, std::string *err)
{
	char tmpPath[256];
	snprintf(tmpPath, sizeof(tmpPath), "/tmp/mesh_export_gltf.%d.tmp", (int)NLGLTF_GETPID());
	try
	{
		{
			COFile ofile;
			if (!ofile.open(tmpPath))
			{
				if (err) *err = "cannot open temp file";
				return false;
			}
			CShapeStream shapeStream(shape);
			shapeStream.serial(ofile);
			ofile.close();
		}
		std::vector<uint8> buf;
		{
			CIFile ifile;
			if (!ifile.open(tmpPath))
			{
				if (err) *err = "cannot reopen temp file";
				return false;
			}
			buf.resize(ifile.getFileSize());
			if (!buf.empty())
				ifile.serialBuffer(&buf[0], (uint)buf.size());
			ifile.close();
		}
		CFile::deleteFile(tmpPath);
		std::string className = shape->getClassName();
		uint32 len = (uint32)buf.size();
		uint32 meshBaseVerOff = 4 + 8 + 4 + (uint32)className.size() + 1;
		if ((className == "CMesh" || className == "CMeshMRM" || className == "CMeshMRMSkinned")
			&& meshBaseVerOff < len && buf[meshBaseVerOff] == 10)
			buf[meshBaseVerOff] = 9;
		COFile file;
		if (!file.open(outPath))
		{
			if (err) *err = "cannot open " + outPath;
			return false;
		}
		file.serialBuffer(buf.empty() ? NULL : &buf[0], len);
		file.close();
		return true;
	}
	catch (const NLMISC::Exception &e)
	{
		if (err) *err = e.what();
		return false;
	}
}

} // anonymous namespace

bool isNelGltfFile(const std::string &filePath)
{
	std::string ext = NLMISC::toLowerAscii(CFile::getExtension(filePath));
	if (ext != "gltf")
		return false;
	// Cheap sniff: nel_source in the asset extras (full parse; files are small relative to the
	// asset pipeline's budget, and only .gltf inputs reach this).
	SGltfDoc doc;
	std::string err;
	if (!doc.load(filePath, &err))
		return false;
	const CJsonValue *asset = doc.Json.get("asset");
	const CJsonValue *extras = asset ? asset->get("extras") : NULL;
	return extras && extras->get("nel_source");
}

int exportNelGltfScene(const CMeshUtilsSettings &settings)
{
	SGltfDoc doc;
	std::string err;
	if (!doc.load(settings.SourceFilePath, &err))
	{
		fprintf(stderr, "ERROR: %s: %s\n", settings.SourceFilePath.c_str(), err.c_str());
		return EXIT_FAILURE;
	}

	// Export-era stream versions, matching the direct exporter (see pipeline_max_export_shape).
	bool oldVB = CVertexBuffer::SerialOldPreferredMemory;
	bool oldIB = CIndexBuffer::SerialOldPreferredMemory;
	CVertexBuffer::SerialOldPreferredMemory = true;
	CIndexBuffer::SerialOldPreferredMemory = true;

	std::string outDir = CPath::standardizePath(settings.DestinationDirectoryPath, true);
	std::string coarseDir = settings.ShapeCoarseDirectoryPath.empty() ? outDir
		: CPath::standardizePath(settings.ShapeCoarseDirectoryPath, true);
	CFile::createDirectoryTree(outDir);
	if (coarseDir != outDir)
		CFile::createDirectoryTree(coarseDir);

	SStats stats;
	const CJsonValue *nodes = doc.Json.get("nodes");
	const CJsonValue *meshes = doc.Json.get("meshes");
	int ret = EXIT_SUCCESS;

	// Instance groups: the asset-level nel_igs blob list carries each CInstanceGroup's exact
	// serial stream (dual representation — the per-node nel_ig_name tags are the editable
	// view; rebuilding an edited ig from them is future work, the blob is authoritative).
	{
		const CJsonValue *asset = doc.Json.get("asset");
		const CJsonValue *aex = asset ? asset->get("extras") : NULL;
		const CJsonValue *igs = aex ? aex->get("nel_igs") : NULL;
		std::string igDir = settings.IGDirectoryPath.empty() ? outDir
			: CPath::standardizePath(settings.IGDirectoryPath, true);
		if (igs && igs->size())
			CFile::createDirectoryTree(igDir);
		for (size_t i = 0; igs && i < igs->size(); ++i)
		{
			const CJsonValue *e = igs->at(i);
			std::string name = e->getString("name", "");
			std::string hexData = e->getString("data", "");
			if (name.empty() || hexData.empty() || (hexData.size() % 2))
			{
				fprintf(stderr, "ERROR: bad nel_igs entry %u\n", (uint)i);
				ret = EXIT_FAILURE;
				continue;
			}
			std::vector<uint8> bytes(hexData.size() / 2);
			bool ok = true;
			for (size_t k = 0; k < bytes.size() && ok; ++k)
			{
				unsigned x = 0;
				for (int h = 0; h < 2 && ok; ++h)
				{
					char c = hexData[k * 2 + h];
					x <<= 4;
					if (c >= '0' && c <= '9') x |= (unsigned)(c - '0');
					else if (c >= 'a' && c <= 'f') x |= (unsigned)(c - 'a' + 10);
					else ok = false;
				}
				bytes[k] = (uint8)x;
			}
			if (!ok)
			{
				fprintf(stderr, "ERROR: bad nel_igs hex for '%s'\n", name.c_str());
				ret = EXIT_FAILURE;
				continue;
			}
			std::string igPath = igDir + name + ".ig";
			try
			{
				COFile f;
				if (!f.open(igPath))
				{
					fprintf(stderr, "ERROR: cannot open %s\n", igPath.c_str());
					ret = EXIT_FAILURE;
					continue;
				}
				f.serialBuffer(&bytes[0], (uint)bytes.size());
				f.close();
				++stats.Igs;
			}
			catch (const NLMISC::Exception &e2)
			{
				fprintf(stderr, "ERROR: %s: %s\n", igPath.c_str(), e2.what());
				ret = EXIT_FAILURE;
			}
		}
	}

	for (size_t ni = 0; nodes && ni < nodes->size(); ++ni)
	{
		const CJsonValue *node = nodes->at(ni);
		const CJsonValue *extras = node->get("extras");
		if (!extras || !extras->getBool("nel_shape", false))
			continue;
		std::string name = node->getString("name", "");
		std::string shapeName = extras->getString("nel_shape_name", NLMISC::toLowerAscii(name));

		if (extras->getInt("nel_lod_count", 0) > 0)
		{
			// CMeshMultiLod assembly from the slave meshes — next coverage step
			stats.skip("multilod");
			fprintf(stderr, "SKIP gltf-import '%s': multilod not implemented yet\n", name.c_str());
			continue;
		}
		bool wantMrm = extras->getBool("nel_lod_mrm", false);
		if (extras->getBool("nel_has_morpher", false) && wantMrm)
		{
			// The direct route builds morph-target bsList meshes into the MRM here
			stats.skip("morpher-mrm");
			fprintf(stderr, "SKIP gltf-import '%s': MRM morph targets not implemented yet\n", name.c_str());
			continue;
		}

		const CJsonValue *jmesh = node->get("mesh");
		const CJsonValue *mesh = (jmesh && jmesh->isNumber() && meshes) ? meshes->at((size_t)jmesh->asInt()) : NULL;
		if (!mesh)
		{
			stats.skip("no-mesh");
			continue;
		}

		CMeshBase::CMeshBaseBuild bbm;
		std::vector<std::string> matNames;
		if (!reconstructBaseBuild(doc, *node, *mesh, bbm, matNames, &err))
		{
			stats.skip("base-build");
			fprintf(stderr, "SKIP gltf-import '%s': %s\n", name.c_str(), err.c_str());
			continue;
		}
		CMesh::CMeshBuild mb;
		if (!reconstructMeshBuild(doc, *mesh, *extras, mb, &err))
		{
			stats.skip("mesh-build");
			fprintf(stderr, "SKIP gltf-import '%s': %s\n", name.c_str(), err.c_str());
			continue;
		}

		// Replay the shape build (buildShapeForNode's mesh path)
		CMeshBase *meshBase = NULL;
		std::vector<sint> materialRemap;
		try
		{
			if (wantMrm)
			{
				CMRMParameters parameters;
				parameters.NLods = (uint)extras->getInt("nel_mrm_nlods", 11);
				parameters.Divisor = (uint)extras->getInt("nel_mrm_divisor", 20);
				parameters.SkinReduction = (CMRMParameters::TSkinReduction)extras->getInt("nel_mrm_skin_reduction", CMRMParameters::SkinReductionMax);
				parameters.DistanceFinest = (float)extras->getDouble("nel_mrm_dist_finest", 5.0);
				parameters.DistanceMiddle = (float)extras->getDouble("nel_mrm_dist_middle", 30.0);
				parameters.DistanceCoarsest = (float)extras->getDouble("nel_mrm_dist_coarsest", 200.0);
				std::vector<CMesh::CMeshBuild *> bsList;
				if (CMeshMRMSkinned::isCompatible(mb) && bsList.empty())
				{
					CMeshMRMSkinned *meshMRMSkinned = new CMeshMRMSkinned;
					meshMRMSkinned->build(bbm, mb, parameters);
					if (!meshMRMSkinned->isRuntimeCompiled())
					{
						delete meshMRMSkinned;
						stats.skip("skinned-maxverts");
						continue;
					}
					meshMRMSkinned->optimizeMaterialUsage(materialRemap);
					meshBase = meshMRMSkinned;
				}
				else
				{
					CMeshMRM *meshMRM = new CMeshMRM;
					meshMRM->build(bbm, mb, bsList, parameters);
					meshMRM->optimizeMaterialUsage(materialRemap);
					meshBase = meshMRM;
				}
			}
			else
			{
				CMesh *m = new CMesh;
				m->build(bbm, mb);
				m->optimizeMaterialUsage(materialRemap);
				meshBase = m;
			}
		}
		catch (const NLMISC::Exception &e)
		{
			stats.skip("build");
			fprintf(stderr, "SKIP gltf-import '%s': build failed: %s\n", name.c_str(), e.what());
			continue;
		}

		if (extras->getBool("nel_animated_materials", false))
		{
			for (uint i = 0; i < matNames.size(); ++i)
			{
				sint dstMatId = i < materialRemap.size() ? materialRemap[i] : (sint)i;
				if (dstMatId >= 0)
					meshBase->setAnimatedMaterial((uint)dstMatId, matNames[i]);
			}
		}
		if (extras->getBool("nel_auto_anim", false))
			meshBase->setAutoAnim(true);
		meshBase->setDistMax((float)extras->getDouble("nel_lod_dist_max", 1000.0));

		std::string outPath = (extras->getBool("nel_coarse", false) ? coarseDir : outDir)
			+ shapeName + ".shape";
		if (!writeShapeFile(meshBase, outPath, &err))
		{
			fprintf(stderr, "ERROR: gltf-import '%s': %s\n", name.c_str(), err.c_str());
			ret = EXIT_FAILURE;
		}
		else
			++stats.Exported;
		delete meshBase;
	}

	CVertexBuffer::SerialOldPreferredMemory = oldVB;
	CIndexBuffer::SerialOldPreferredMemory = oldIB;

	printf("GLTF-IMPORT %s (%u shapes, %u igs, %u skipped)\n", settings.SourceFilePath.c_str(),
	       stats.Exported, stats.Igs, stats.Skipped);
	for (std::map<std::string, uint>::iterator it = stats.SkipReasons.begin(); it != stats.SkipReasons.end(); ++it)
		printf("SKIPCLASS %s %u\n", it->first.c_str(), it->second);
	return ret;
}

/* end of file */
