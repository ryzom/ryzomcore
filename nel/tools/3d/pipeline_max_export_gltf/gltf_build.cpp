/**
 * \file gltf_build.cpp
 * \brief See gltf_build.h.
 *
 * Mesh encoding (the lossless CMeshBuild carrier — schema in wiki drafts/nel_gltf_extras.md):
 *
 * CMeshGeom::build's vertex-buffer construction dedups corners on (original vertex id, normal,
 * uvws, color) in GLOBAL face order, its bbox spans the FULL original Vertices array (including
 * unreferenced vertices), and its rdrpass index buffers fill in global face order — so a glTF
 * encoding that only carried per-material deduped primitives would lose exactly the three
 * things that determine the final .shape bytes. The encoding therefore carries:
 *
 *  - standard per-material primitives (POSITION/NORMAL/TEXCOORD_n/COLOR_0 + indices), corners
 *    deduped with the same key CCornerTmp uses — the interop view any glTF consumer sees;
 *  - per-primitive extras accessors: nel_vertex_ids (original CMeshBuild vertex id per glTF
 *    vertex), nel_faces (original global face index per triangle), nel_smgroups (smoothing
 *    group dword per triangle);
 *  - per-mesh extras: nel_vertices (accessor with the full original vertex array, verbatim),
 *    nel_vertex_flags, nel_uv_routing, nel_materials, and the interface-weld arrays when the
 *    node carried one (nel_iface_*).
 *
 * The reader reconstructs the exact CMeshBuild: Vertices from nel_vertices, Faces by scattering
 * every primitive's triangles into nel_faces order with corners rebuilt from the primitive
 * attributes + nel_vertex_ids. UVW W components are always zero at this stage (asserted here),
 * so TEXCOORD_n stays VEC2.
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
#include "gltf_build.h"

#include <cstdio>
#include <cstring>

#include <nel/misc/file.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/path.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_multi_file.h>
#include <nel/3d/texture_cube.h>

#include "../nel_gltf/gltf_material.h"

using namespace NLMISC;
using namespace NL3D;
using namespace NLGLTF;

namespace GLTFBUILD {

CGltfBuilder::CGltfBuilder()
{
	CJsonValue *asset = m_Root.setObject("asset");
	asset->setString("version", "2.0");
	asset->setString("generator", "pipeline_max_export_gltf");
	m_Root.setInt("scene", 0);
	m_Scenes = m_Root.setArray("scenes");
	m_Scenes->push(); // scenes[0], filled by setSceneRoots
	m_Nodes = m_Root.setArray("nodes");
	m_Meshes = m_Root.setArray("meshes");
	m_Materials = m_Root.setArray("materials");
	m_Textures = m_Root.setArray("textures");
	m_Images = m_Root.setArray("images");
	m_Accessors = m_Root.setArray("accessors");
	m_BufferViews = m_Root.setArray("bufferViews");
	m_Skins = NULL;
	m_Animations = NULL;
	m_Root.setArray("buffers"); // filled at save
}

CGltfBuilder::~CGltfBuilder()
{
}

NLGLTF::CJsonValue *CGltfBuilder::assetExtras()
{
	return m_Root.getMutable("asset")->ensureObject("extras");
}

// ---------------------------------------------------------------------------------------------
// Low-level buffer/accessor plumbing

sint CGltfBuilder::addBufferView(const void *data, size_t bytes, int target)
{
	while (m_Bin.size() % 4)
		m_Bin.push_back(0);
	size_t offset = m_Bin.size();
	m_Bin.insert(m_Bin.end(), (const uint8 *)data, (const uint8 *)data + bytes);
	CJsonValue *bv = m_BufferViews->push();
	bv->setInt("buffer", 0);
	bv->setInt("byteOffset", (sint64)offset);
	bv->setInt("byteLength", (sint64)bytes);
	if (target)
		bv->setInt("target", target);
	return (sint)m_BufferViews->size() - 1;
}

sint CGltfBuilder::addAccessor(sint bufferView, int componentType, const char *type, size_t count,
                               bool normalized, const float *minv, const float *maxv, int nComp)
{
	CJsonValue *ac = m_Accessors->push();
	ac->setInt("bufferView", bufferView);
	ac->setInt("componentType", componentType);
	ac->setInt("count", (sint64)count);
	ac->setString("type", type);
	if (normalized)
		ac->setBool("normalized", true);
	if (minv && maxv && nComp)
	{
		CJsonValue *mn = ac->setArray("min");
		CJsonValue *mx = ac->setArray("max");
		for (int i = 0; i < nComp; ++i)
		{
			mn->pushDouble(minv[i]);
			mx->pushDouble(maxv[i]);
		}
	}
	return (sint)m_Accessors->size() - 1;
}

static const char *typeForComp(int nComp)
{
	switch (nComp)
	{
	case 1: return "SCALAR";
	case 2: return "VEC2";
	case 3: return "VEC3";
	case 4: return "VEC4";
	default: return "SCALAR";
	}
}

sint CGltfBuilder::addAccessorFloat(const float *data, size_t count, int nComp, int target, bool withMinMax)
{
	sint bv = addBufferView(data, count * nComp * 4, target);
	float minv[4], maxv[4];
	if (withMinMax && count)
	{
		for (int c = 0; c < nComp; ++c)
		{
			minv[c] = maxv[c] = data[c];
		}
		for (size_t i = 1; i < count; ++i)
		{
			for (int c = 0; c < nComp; ++c)
			{
				float v = data[i * nComp + c];
				if (v < minv[c]) minv[c] = v;
				if (v > maxv[c]) maxv[c] = v;
			}
		}
	}
	return addAccessor(bv, COMP_FLOAT, typeForComp(nComp), count, false,
	                   withMinMax && count ? minv : NULL, withMinMax && count ? maxv : NULL, nComp);
}

sint CGltfBuilder::addAccessorU32(const uint32 *data, size_t count, int target)
{
	sint bv = addBufferView(data, count * 4, target);
	return addAccessor(bv, COMP_UINT, "SCALAR", count, false, NULL, NULL, 0);
}

sint CGltfBuilder::addAccessorU8Vec4Norm(const uint8 *data, size_t count4)
{
	sint bv = addBufferView(data, count4 * 4, TARGET_ARRAY);
	return addAccessor(bv, COMP_UBYTE, "VEC4", count4, true, NULL, NULL, 0);
}

sint CGltfBuilder::addAccessorU16Vec4(const uint16 *data, size_t count4)
{
	sint bv = addBufferView(data, count4 * 8, TARGET_ARRAY);
	return addAccessor(bv, COMP_USHORT, "VEC4", count4, false, NULL, NULL, 0);
}

sint CGltfBuilder::addSkin(const std::vector<sint> &joints, const float *ibms)
{
	if (!m_Skins)
		m_Skins = m_Root.setArray("skins");
	CJsonValue *sk = m_Skins->push();
	// IBM accessor: no bufferView target (not vertex data — validators flag ARRAY_BUFFER here)
	sint bv = addBufferView(ibms, joints.size() * 16 * 4, 0);
	sk->setInt("inverseBindMatrices", addAccessor(bv, COMP_FLOAT, "MAT4", joints.size(), false, NULL, NULL, 0));
	CJsonValue *js = sk->setArray("joints");
	for (size_t i = 0; i < joints.size(); ++i)
		js->pushInt(joints[i]);
	return (sint)m_Skins->size() - 1;
}

void CGltfBuilder::setNodeSkin(sint node, sint skin)
{
	m_NodeVals[(size_t)node]->setInt("skin", skin);
}

void CGltfBuilder::addWeightsChannel(const std::string &animName, sint node,
                                     const std::vector<float> &times, const std::vector<float> &weights,
                                     int numTargets)
{
	// Same sampler/channel plumbing as addAnimChannel, but the output is a SCALAR accessor
	// with numTargets values per time sample (glTF "weights" path semantics).
	addAnimChannel(animName, node, "weights", times, weights, 1);
	(void)numTargets;
}

void CGltfBuilder::setMeshMorphMeta(sint mesh, const std::vector<std::string> &names,
                                    const std::vector<float> &defaults01)
{
	CJsonValue *jm = const_cast<CJsonValue *>(m_Meshes->at((size_t)mesh));
	CJsonValue *w = jm->setArray("weights");
	for (size_t i = 0; i < defaults01.size(); ++i)
		w->pushDouble(defaults01[i]);
	CJsonValue *tn = jm->ensureObject("extras")->setArray("targetNames");
	for (size_t i = 0; i < names.size(); ++i)
		tn->pushString(names[i]);
}

void CGltfBuilder::addAnimChannel(const std::string &animName, sint node, const char *path,
                                  const std::vector<float> &times, const std::vector<float> &values,
                                  int nComp)
{
	if (!m_Animations)
		m_Animations = m_Root.setArray("animations");
	CJsonValue *anim = NULL;
	if (m_Animations->size())
		anim = const_cast<CJsonValue *>(m_Animations->at(m_Animations->size() - 1));
	if (!anim || anim->getString("name", "") != animName)
	{
		anim = m_Animations->push();
		anim->setString("name", animName);
		anim->setArray("samplers");
		anim->setArray("channels");
	}
	// Time accessor needs min/max per spec; no bufferView target (not vertex data).
	sint tbv = addBufferView(&times[0], times.size() * 4, 0);
	float tmin = times[0], tmax = times[times.size() - 1];
	sint tacc = addAccessor(tbv, COMP_FLOAT, "SCALAR", times.size(), false, &tmin, &tmax, 1);
	sint vbv = addBufferView(&values[0], values.size() * 4, 0);
	sint vacc = addAccessor(vbv, COMP_FLOAT,
	                        nComp == 4 ? "VEC4" : (nComp == 3 ? "VEC3" : "SCALAR"),
	                        values.size() / nComp, false, NULL, NULL, 0);
	CJsonValue *samplers = anim->getMutable("samplers");
	CJsonValue *sampler = samplers->push();
	sampler->setInt("input", tacc);
	sampler->setString("interpolation", "LINEAR");
	sampler->setInt("output", vacc);
	CJsonValue *channels = anim->getMutable("channels");
	CJsonValue *channel = channels->push();
	channel->setInt("sampler", (sint64)samplers->size() - 1);
	CJsonValue *target = channel->setObject("target");
	target->setInt("node", node);
	target->setString("path", path);
}

// ---------------------------------------------------------------------------------------------
// Nodes

sint CGltfBuilder::addNode(const std::string &name, const CVector &pos, const CQuat &rot,
                           const CVector &scale)
{
	CJsonValue *n = m_Nodes->push();
	n->setString("name", name);
	CJsonValue *t = n->setArray("translation");
	t->pushDouble(pos.x);
	t->pushDouble(pos.y);
	t->pushDouble(pos.z);
	CJsonValue *r = n->setArray("rotation");
	r->pushDouble(rot.x);
	r->pushDouble(rot.y);
	r->pushDouble(rot.z);
	r->pushDouble(rot.w);
	CJsonValue *s = n->setArray("scale");
	s->pushDouble(scale.x);
	s->pushDouble(scale.y);
	s->pushDouble(scale.z);
	CJsonValue *e = n->setObject("extras");
	e->setDouble("nel_tx", pos.x);
	e->setDouble("nel_ty", pos.y);
	e->setDouble("nel_tz", pos.z);
	e->setDouble("nel_rx", rot.x);
	e->setDouble("nel_ry", rot.y);
	e->setDouble("nel_rz", rot.z);
	e->setDouble("nel_rw", rot.w);
	e->setDouble("nel_sx", scale.x);
	e->setDouble("nel_sy", scale.y);
	e->setDouble("nel_sz", scale.z);
	m_NodeVals.push_back(n);
	return (sint)m_NodeVals.size() - 1;
}

void CGltfBuilder::setNodeChildren(sint node, const std::vector<sint> &children)
{
	if (node < 0 || node >= (sint)m_NodeVals.size() || children.empty())
		return;
	CJsonValue *c = m_NodeVals[(size_t)node]->setArray("children");
	for (size_t i = 0; i < children.size(); ++i)
		c->pushInt(children[i]);
}

void CGltfBuilder::setSceneRoots(const std::vector<sint> &roots)
{
	CJsonValue *scene0 = const_cast<CJsonValue *>(m_Scenes->at(0));
	scene0->clear();
	scene0->setString("name", "scene");
	CJsonValue *n = scene0->setArray("nodes");
	for (size_t i = 0; i < roots.size(); ++i)
		n->pushInt(roots[i]);
}

void CGltfBuilder::attachMesh(sint node, sint mesh)
{
	if (node < 0 || node >= (sint)m_NodeVals.size() || mesh < 0)
		return;
	m_NodeVals[(size_t)node]->setInt("mesh", mesh);
}

NLGLTF::CJsonValue *CGltfBuilder::nodeExtras(sint node)
{
	if (node < 0 || node >= (sint)m_NodeVals.size())
		return NULL;
	return m_NodeVals[(size_t)node]->ensureObject("extras");
}

// ---------------------------------------------------------------------------------------------
// Materials

sint CGltfBuilder::addImageTexture(const std::string &uri)
{
	std::map<std::string, sint>::iterator it = m_TextureDedup.find(uri);
	if (it != m_TextureDedup.end())
		return it->second;
	sint image;
	std::map<std::string, sint>::iterator ii = m_ImageDedup.find(uri);
	if (ii != m_ImageDedup.end())
		image = ii->second;
	else
	{
		CJsonValue *img = m_Images->push();
		img->setString("uri", uri);
		image = (sint)m_Images->size() - 1;
		m_ImageDedup[uri] = image;
	}
	CJsonValue *tex = m_Textures->push();
	tex->setInt("source", image);
	sint idx = (sint)m_Textures->size() - 1;
	m_TextureDedup[uri] = idx;
	return idx;
}

// The interop-facing texture file of a built export texture (slot 0's basename).
static std::string interopTextureFile(ITexture *tex)
{
	if (!tex) return std::string();
	if (CTextureFile *tf = dynamic_cast<CTextureFile *>(tex))
		return tf->getFileName();
	if (CTextureMultiFile *tm = dynamic_cast<CTextureMultiFile *>(tex))
		return tm->getNumFileName() ? tm->getFileName(0) : std::string();
	if (CTextureCube *tc = dynamic_cast<CTextureCube *>(tex))
		return interopTextureFile(tc->getTexture((CTextureCube::TFace)0));
	return std::string();
}

sint CGltfBuilder::addMaterial(const CMaterial &mat, const std::string &name, std::string *err)
{
	// Dedup key: name + full serialization (standalone stream; polyptr ids deterministic).
	std::string key;
	{
		CMemStream ms;
		const_cast<CMaterial &>(mat).serial(ms);
		key.reserve(name.size() + 1 + ms.length() * 2);
		key = name;
		key += '\0';
		char buf[4];
		for (uint i = 0; i < ms.length(); ++i)
		{
			snprintf(buf, sizeof(buf), "%02x", ms.buffer()[i]);
			key += buf;
		}
	}
	std::map<std::string, sint>::iterator it = m_MaterialDedup.find(key);
	if (it != m_MaterialDedup.end())
		return it->second;

	CJsonValue *jm = m_Materials->push();
	jm->setString("name", name);

	// Interop PBR view
	CMaterial &m = const_cast<CMaterial &>(mat);
	CRGBA base = m.isLighted() ? m.getDiffuse() : m.getColor();
	CJsonValue *pbr = jm->setObject("pbrMetallicRoughness");
	CJsonValue *bcf = pbr->setArray("baseColorFactor");
	bcf->pushDouble(base.R / 255.0f);
	bcf->pushDouble(base.G / 255.0f);
	bcf->pushDouble(base.B / 255.0f);
	bcf->pushDouble(base.A / 255.0f);
	pbr->setDouble("metallicFactor", 0.0f);
	pbr->setDouble("roughnessFactor", 1.0f);
	std::string texFile = interopTextureFile(m.getTexture(0));
	if (!texFile.empty())
	{
		CJsonValue *bct = pbr->setObject("baseColorTexture");
		bct->setInt("index", addImageTexture(texFile));
		bct->setInt("texCoord", 0);
	}
	if (m.getBlend())
		jm->setString("alphaMode", "BLEND");
	else if (m.getAlphaTest())
	{
		jm->setString("alphaMode", "MASK");
		jm->setDouble("alphaCutoff", m.getAlphaTestThreshold());
	}
	if (m.getDoubleSided())
		jm->setBool("doubleSided", true);

	// Exact NeL view
	CJsonValue *extras = jm->setObject("extras");
	if (!materialToExtras(mat, *extras, err))
	{
		if (err) *err = "material '" + name + "': " + *err;
		return -1;
	}

	sint idx = (sint)m_Materials->size() - 1;
	m_MaterialDedup[key] = idx;
	return idx;
}

// ---------------------------------------------------------------------------------------------
// Meshes

// Corner dedup key — the CCornerTmp::operator< field set under the build's vertex flags:
// original vertex id, normal, used-stage UVWs, color. Encoded bitwise so float -0.0 vs +0.0 and
// NaN payloads never merge values the runtime build would keep distinct.
static void cornerKey(const CMesh::CCorner &c, const std::vector<uint> &uvStages, bool hasColor,
                      std::string &key)
{
	key.clear();
	key.append((const char *)&c.Vertex, 4);
	key.append((const char *)&c.Normal, 12);
	for (size_t i = 0; i < uvStages.size(); ++i)
		key.append((const char *)&c.Uvws[uvStages[i]], 12);
	if (hasColor)
		key.append((const char *)&c.Color, 4);
}

sint CGltfBuilder::addMesh(const std::string &name, const CMesh::CMeshBuild &mb,
                           const std::vector<sint> &materialIdx, std::string *err,
                           const std::vector<NL3D::CMesh::CMeshBuild *> *bsList,
                           bool *skinInterop)
{
	// Carried flag surface: position+normal+uv[0..7]+primary color+palette skin. Anything else
	// means a build class the encoding doesn't carry yet — refuse loudly.
	uint32 supported = CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag
		| CVertexBuffer::PrimaryColorFlag | CVertexBuffer::PaletteSkinFlag;
	for (uint i = 0; i < CVertexBuffer::MaxStage; ++i)
		supported |= CVertexBuffer::TexCoord0Flag << i;
	if ((uint32)mb.VertexFlags & ~supported)
	{
		if (err)
		{
			char buf[64];
			snprintf(buf, sizeof(buf), "unsupported vertex flags 0x%x", mb.VertexFlags);
			*err = buf;
		}
		return -1;
	}
	if (!mb.BlendShapes.empty())
	{
		// CMeshBuild::BlendShapes (the plain-CMesh morph carrier) is never filled by the max
		// route — the MRM morph targets ride `bsList` instead.
		if (err) *err = "CMeshBuild::BlendShapes not carried";
		return -1;
	}
	bool hasSkin = ((uint32)mb.VertexFlags & CVertexBuffer::PaletteSkinFlag) != 0;
	if (hasSkin != !mb.SkinWeights.empty()
		|| (hasSkin && mb.SkinWeights.size() != mb.Vertices.size()))
	{
		if (err) *err = "inconsistent skin weight state";
		return -1;
	}

	// JOINTS_0/WEIGHTS_0 viewing tier: only when the weighted MatrixIds fit the interop form
	// (they index BonesNames, which the caller's skin.joints mirrors; u16 component type).
	// Zero-weight slots are ignored here and emitted as joint 0 — the exporter leaves them
	// uninitialized (the geom build never reads them, so the exact tier carries the garbage
	// verbatim). Falling short disables the interop attributes, never the exact-tier emission.
	bool emitSkinAttrs = skinInterop && *skinInterop && hasSkin;
	if (emitSkinAttrs)
		for (size_t i = 0; i < mb.SkinWeights.size() && emitSkinAttrs; ++i)
			for (uint k = 0; k < NL3D_MESH_SKINNING_MAX_MATRIX; ++k)
				if (mb.SkinWeights[i].Weights[k] != 0.0f
					&& (mb.SkinWeights[i].MatrixId[k] >= mb.BonesNames.size()
						|| mb.SkinWeights[i].MatrixId[k] > 0xffff))
				{
					emitSkinAttrs = false;
					break;
				}
	if (skinInterop)
		*skinInterop = emitSkinAttrs;

	std::vector<uint> uvStages;
	for (uint i = 0; i < CVertexBuffer::MaxStage; ++i)
		if ((uint32)mb.VertexFlags & (CVertexBuffer::TexCoord0Flag << i))
		{
			if (mb.NumCoords[i] != 2)
			{
				if (err) *err = "3-component UV stage not carried yet";
				return -1;
			}
			uvStages.push_back(i);
		}
	bool hasColor = ((uint32)mb.VertexFlags & CVertexBuffer::PrimaryColorFlag) != 0;

	// Group faces per local material id, keeping the original global face index.
	std::map<sint, std::vector<uint32> > facesByMat;
	for (uint32 f = 0; f < mb.Faces.size(); ++f)
	{
		sint matId = mb.Faces[f].MaterialId;
		if (matId < 0 || matId >= (sint)materialIdx.size())
		{
			if (err)
			{
				char buf[64];
				snprintf(buf, sizeof(buf), "face %u material id %d out of range", f, matId);
				*err = buf;
			}
			return -1;
		}
		facesByMat[matId].push_back(f);
		// W components are dropped from the interop TEXCOORD_n (VEC2) — they must be zero at
		// this stage (mesh_build writes 0; CCorner ctor zeroes). If a future build class emits
		// 3-coord UVs, NumCoords catches it above; a stray non-zero W would be silent loss.
		for (uint c = 0; c < 3; ++c)
			for (size_t s = 0; s < uvStages.size(); ++s)
				if (mb.Faces[f].Corner[c].Uvws[uvStages[s]].W != 0.0f)
				{
					if (err) *err = "non-zero UVW W component not carried";
					return -1;
				}
	}

	// Morph-target validation up front (refuse before emitting anything). buildBlendShapes
	// iterates the BASE faces and indexes bsList[i]->Faces[j].Corner[k] blindly — equal counts
	// are its implicit contract; the used-stage W==0 rule matches the base corners.
	if (bsList)
	{
		for (size_t i = 0; i < bsList->size(); ++i)
		{
			const CMesh::CMeshBuild *bs = (*bsList)[i];
			if (bs->Vertices.size() != mb.Vertices.size() || bs->Faces.size() != mb.Faces.size())
			{
				if (err) *err = "morph target vertex/face count mismatch";
				return -1;
			}
			for (size_t f = 0; f < bs->Faces.size(); ++f)
				for (uint c = 0; c < 3; ++c)
					for (uint s = 0; s < CVertexBuffer::MaxStage; ++s)
						if (((uint32)mb.VertexFlags & (CVertexBuffer::TexCoord0Flag << s))
							&& bs->Faces[f].Corner[c].Uvws[s].W != 0.0f)
						{
							if (err) *err = "morph target non-zero UVW W component not carried";
							return -1;
						}
		}
	}

	CJsonValue *jm = m_Meshes->push();
	jm->setString("name", name);
	CJsonValue *prims = jm->setArray("primitives");

	for (std::map<sint, std::vector<uint32> >::iterator mi = facesByMat.begin();
	     mi != facesByMat.end(); ++mi)
	{
		const std::vector<uint32> &faces = mi->second;

		// Dedup corners into the primitive's vertex streams
		std::map<std::string, uint32> dedup;
		std::vector<float> pos, norm;
		std::vector<std::vector<float> > uvs(uvStages.size());
		std::vector<uint8> colors;
		std::vector<uint16> joints16;
		std::vector<float> weights4;
		std::vector<uint32> vertexIds;
		std::vector<std::pair<uint32, uint> > firstCorner; // (global face, corner) per vertex
		std::vector<uint32> indices;
		indices.reserve(faces.size() * 3);
		std::string key;
		for (size_t k = 0; k < faces.size(); ++k)
		{
			const CMesh::CFace &face = mb.Faces[faces[k]];
			for (uint c = 0; c < 3; ++c)
			{
				const CMesh::CCorner &corner = face.Corner[c];
				cornerKey(corner, uvStages, hasColor, key);
				std::map<std::string, uint32>::iterator di = dedup.find(key);
				uint32 vid;
				if (di != dedup.end())
					vid = di->second;
				else
				{
					vid = (uint32)(pos.size() / 3);
					dedup[key] = vid;
					const CVector &v = mb.Vertices[(size_t)corner.Vertex];
					pos.push_back(v.x);
					pos.push_back(v.y);
					pos.push_back(v.z);
					norm.push_back(corner.Normal.x);
					norm.push_back(corner.Normal.y);
					norm.push_back(corner.Normal.z);
					for (size_t s = 0; s < uvStages.size(); ++s)
					{
						uvs[s].push_back(corner.Uvws[uvStages[s]].U);
						uvs[s].push_back(corner.Uvws[uvStages[s]].V);
					}
					if (hasColor)
					{
						colors.push_back(corner.Color.R);
						colors.push_back(corner.Color.G);
						colors.push_back(corner.Color.B);
						colors.push_back(corner.Color.A);
					}
					if (emitSkinAttrs)
					{
						const CMesh::CSkinWeight &sw = mb.SkinWeights[(size_t)corner.Vertex];
						for (uint w = 0; w < NL3D_MESH_SKINNING_MAX_MATRIX; ++w)
						{
							bool used = sw.Weights[w] != 0.0f;
							joints16.push_back(used ? (uint16)sw.MatrixId[w] : 0);
							weights4.push_back(used ? sw.Weights[w] : 0.0f);
						}
					}
					vertexIds.push_back((uint32)corner.Vertex);
					firstCorner.push_back(std::make_pair(faces[k], c));
				}
				indices.push_back(vid);
			}
		}

		size_t nVerts = pos.size() / 3;
		CJsonValue *prim = prims->push();
		CJsonValue *attrs = prim->setObject("attributes");
		attrs->setInt("POSITION", addAccessorFloat(&pos[0], nVerts, 3, TARGET_ARRAY, true));
		attrs->setInt("NORMAL", addAccessorFloat(&norm[0], nVerts, 3, TARGET_ARRAY, false));
		for (size_t s = 0; s < uvStages.size(); ++s)
		{
			char an[24];
			// TEXCOORD set indices are consecutive in the glTF; the NeL stage they map to is
			// recorded in nel_uv_stages so sparse stage usage round-trips.
			snprintf(an, sizeof(an), "TEXCOORD_%u", (uint)s);
			attrs->setInt(an, addAccessorFloat(&uvs[s][0], nVerts, 2, TARGET_ARRAY, false));
		}
		if (hasColor)
			attrs->setInt("COLOR_0", addAccessorU8Vec4Norm(&colors[0], nVerts));
		if (emitSkinAttrs)
		{
			attrs->setInt("JOINTS_0", addAccessorU16Vec4(&joints16[0], nVerts));
			attrs->setInt("WEIGHTS_0", addAccessorFloat(&weights4[0], nVerts, 4, TARGET_ARRAY, false));
		}
		prim->setInt("indices", addAccessorU32(&indices[0], indices.size(), TARGET_ELEMENT));
		if (materialIdx[(size_t)mi->first] >= 0)
			prim->setInt("material", materialIdx[(size_t)mi->first]);

		// Standard glTF morph targets (viewing tier): POSITION deltas per original vertex id,
		// NORMAL deltas from the first-occurrence corner (per-corner target normals collapse
		// to per-vertex here — the exact tier's nel_bs_* corner streams stay authoritative).
		if (bsList && !bsList->empty())
		{
			CJsonValue *targets = prim->setArray("targets");
			for (size_t bi = 0; bi < bsList->size(); ++bi)
			{
				const CMesh::CMeshBuild *bs = (*bsList)[bi];
				std::vector<float> dpos(nVerts * 3), dnorm(nVerts * 3);
				for (size_t v = 0; v < nVerts; ++v)
				{
					uint32 ov = vertexIds[v];
					dpos[v * 3 + 0] = bs->Vertices[ov].x - mb.Vertices[ov].x;
					dpos[v * 3 + 1] = bs->Vertices[ov].y - mb.Vertices[ov].y;
					dpos[v * 3 + 2] = bs->Vertices[ov].z - mb.Vertices[ov].z;
					const CMesh::CCorner &bc = bs->Faces[firstCorner[v].first].Corner[firstCorner[v].second];
					dnorm[v * 3 + 0] = bc.Normal.x - norm[v * 3 + 0];
					dnorm[v * 3 + 1] = bc.Normal.y - norm[v * 3 + 1];
					dnorm[v * 3 + 2] = bc.Normal.z - norm[v * 3 + 2];
				}
				CJsonValue *tgt = targets->push();
				tgt->setInt("POSITION", addAccessorFloat(&dpos[0], nVerts, 3, TARGET_ARRAY, true));
				tgt->setInt("NORMAL", addAccessorFloat(&dnorm[0], nVerts, 3, TARGET_ARRAY, false));
			}
		}

		// NeL reconstruction data
		std::vector<uint32> smGroups(faces.size());
		for (size_t k = 0; k < faces.size(); ++k)
			smGroups[k] = (uint32)mb.Faces[faces[k]].SmoothGroup;
		CJsonValue *pex = prim->setObject("extras");
		pex->setInt("nel_material_id", mi->first);
		pex->setInt("nel_vertex_ids", addAccessorU32(&vertexIds[0], vertexIds.size(), 0));
		pex->setInt("nel_faces", addAccessorU32(&faces[0], faces.size(), 0));
		pex->setInt("nel_smgroups", addAccessorU32(&smGroups[0], smGroups.size(), 0));
	}

	// Mesh-level NeL data
	CJsonValue *mex = jm->setObject("extras");
	{
		// Full original vertex array (CMeshGeom's bbox spans it, unreferenced verts included)
		std::vector<float> verts(mb.Vertices.size() * 3);
		for (size_t i = 0; i < mb.Vertices.size(); ++i)
		{
			verts[i * 3 + 0] = mb.Vertices[i].x;
			verts[i * 3 + 1] = mb.Vertices[i].y;
			verts[i * 3 + 2] = mb.Vertices[i].z;
		}
		mex->setInt("nel_vertices", verts.empty() ? -1
			: addAccessorFloat(&verts[0], mb.Vertices.size(), 3, 0, false));
	}
	mex->setInt("nel_vertex_flags", (sint64)(uint32)mb.VertexFlags);
	{
		CJsonValue *r = mex->setArray("nel_uv_routing");
		for (uint i = 0; i < CVertexBuffer::MaxStage; ++i)
			r->pushInt(mb.UVRouting[i]);
	}
	{
		CJsonValue *st = mex->setArray("nel_uv_stages");
		for (size_t s = 0; s < uvStages.size(); ++s)
			st->pushInt(uvStages[s]);
	}
	{
		CJsonValue *mats = mex->setArray("nel_materials");
		for (size_t i = 0; i < materialIdx.size(); ++i)
			mats->pushInt(materialIdx[i]);
	}

	// Skinning: per-ORIGINAL-vertex weights + joint ids (parallel to nel_vertices — the corner
	// dedup key already covers them, skin data is a function of the original vertex id) and the
	// full bone name list applyPhysiqueSkinning produced (the geom build remaps to the used
	// subset itself).
	if (hasSkin)
	{
		std::vector<float> weights(mb.SkinWeights.size() * 4);
		std::vector<uint32> joints(mb.SkinWeights.size() * 4);
		for (size_t i = 0; i < mb.SkinWeights.size(); ++i)
			for (uint k = 0; k < NL3D_MESH_SKINNING_MAX_MATRIX; ++k)
			{
				weights[i * 4 + k] = mb.SkinWeights[i].Weights[k];
				joints[i * 4 + k] = mb.SkinWeights[i].MatrixId[k];
			}
		mex->setInt("nel_skin_weights",
			addAccessorFloat(&weights[0], mb.SkinWeights.size(), 4, 0, false));
		mex->setInt("nel_skin_joints", addAccessorU32(&joints[0], joints.size(), 0));
		CJsonValue *bones = mex->setArray("nel_bones_names");
		for (size_t i = 0; i < mb.BonesNames.size(); ++i)
			bones->pushString(mb.BonesNames[i]);
	}

	// MRM morph targets: per-target full vertex array + corner attribute streams in GLOBAL face
	// order (index 3*face+corner) — exactly the fields CMRMBuilder::buildBlendShapes reads off
	// the bs meshes (their own face topology is never consulted; the base's is).
	if (bsList && !bsList->empty())
	{
		mex->setInt("nel_bs_geoms", (sint64)bsList->size());
		for (size_t i = 0; i < bsList->size(); ++i)
		{
			const CMesh::CMeshBuild *bs = (*bsList)[i];
			char keyBuf[40];
			{
				std::vector<float> verts(bs->Vertices.size() * 3);
				for (size_t v = 0; v < bs->Vertices.size(); ++v)
				{
					verts[v * 3 + 0] = bs->Vertices[v].x;
					verts[v * 3 + 1] = bs->Vertices[v].y;
					verts[v * 3 + 2] = bs->Vertices[v].z;
				}
				snprintf(keyBuf, sizeof(keyBuf), "nel_bs_%u_vertices", (uint)i);
				mex->setInt(keyBuf, verts.empty() ? -1
					: addAccessorFloat(&verts[0], bs->Vertices.size(), 3, 0, false));
			}
			size_t nCorners = bs->Faces.size() * 3;
			if (((uint32)mb.VertexFlags & CVertexBuffer::NormalFlag) && nCorners)
			{
				std::vector<float> norms(nCorners * 3);
				for (size_t f = 0; f < bs->Faces.size(); ++f)
					for (uint c = 0; c < 3; ++c)
					{
						const CVector &nv = bs->Faces[f].Corner[c].Normal;
						norms[(f * 3 + c) * 3 + 0] = nv.x;
						norms[(f * 3 + c) * 3 + 1] = nv.y;
						norms[(f * 3 + c) * 3 + 2] = nv.z;
					}
				snprintf(keyBuf, sizeof(keyBuf), "nel_bs_%u_normals", (uint)i);
				mex->setInt(keyBuf, addAccessorFloat(&norms[0], nCorners, 3, 0, false));
			}
			if (hasColor && nCorners)
			{
				std::vector<uint8> cols(nCorners * 4);
				for (size_t f = 0; f < bs->Faces.size(); ++f)
					for (uint c = 0; c < 3; ++c)
					{
						const NLMISC::CRGBA &col = bs->Faces[f].Corner[c].Color;
						cols[(f * 3 + c) * 4 + 0] = col.R;
						cols[(f * 3 + c) * 4 + 1] = col.G;
						cols[(f * 3 + c) * 4 + 2] = col.B;
						cols[(f * 3 + c) * 4 + 3] = col.A;
					}
				snprintf(keyBuf, sizeof(keyBuf), "nel_bs_%u_colors", (uint)i);
				mex->setInt(keyBuf, addAccessorU8Vec4Norm(&cols[0], nCorners));
			}
			for (size_t s = 0; s < uvStages.size() && nCorners; ++s)
			{
				std::vector<float> uv(nCorners * 2);
				for (size_t f = 0; f < bs->Faces.size(); ++f)
					for (uint c = 0; c < 3; ++c)
					{
						uv[(f * 3 + c) * 2 + 0] = bs->Faces[f].Corner[c].Uvws[uvStages[s]].U;
						uv[(f * 3 + c) * 2 + 1] = bs->Faces[f].Corner[c].Uvws[uvStages[s]].V;
					}
				snprintf(keyBuf, sizeof(keyBuf), "nel_bs_%u_uvw%u", (uint)i, uvStages[s]);
				mex->setInt(keyBuf, addAccessorFloat(&uv[0], nCorners, 2, 0, false));
			}
		}
	}

	// Interface weld state (INTERFACE_FILE appdata nodes): the border polygons and the per-
	// vertex weld links. Consumed by the MRM builder; the welded normals themselves are already
	// in the corner data.
	if (!mb.Interfaces.empty())
	{
		CJsonValue *counts = mex->setArray("nel_iface_counts");
		std::vector<float> ipos, inorm;
		for (size_t m = 0; m < mb.Interfaces.size(); ++m)
		{
			counts->pushInt((sint64)mb.Interfaces[m].Vertices.size());
			for (size_t k = 0; k < mb.Interfaces[m].Vertices.size(); ++k)
			{
				const CMesh::CInterfaceVertex &v = mb.Interfaces[m].Vertices[k];
				ipos.push_back(v.Pos.x);
				ipos.push_back(v.Pos.y);
				ipos.push_back(v.Pos.z);
				inorm.push_back(v.Normal.x);
				inorm.push_back(v.Normal.y);
				inorm.push_back(v.Normal.z);
			}
		}
		if (!ipos.empty())
		{
			mex->setInt("nel_iface_pos", addAccessorFloat(&ipos[0], ipos.size() / 3, 3, 0, false));
			mex->setInt("nel_iface_norm", addAccessorFloat(&inorm[0], inorm.size() / 3, 3, 0, false));
		}
	}
	if (!mb.InterfaceLinks.empty())
	{
		// Flat [interfaceId, interfaceVertexId] pairs per mesh vertex; InterfaceVertexFlag is
		// derived (bit set where interfaceId != -1).
		CJsonValue *links = mex->setArray("nel_iface_links");
		for (size_t k = 0; k < mb.InterfaceLinks.size(); ++k)
		{
			links->pushInt(mb.InterfaceLinks[k].InterfaceId);
			links->pushInt((sint64)mb.InterfaceLinks[k].InterfaceVertexId);
		}
	}

	return (sint)m_Meshes->size() - 1;
}

sint CGltfBuilder::addProxyMesh(const std::string &name, const std::vector<float> &pos,
                                const std::vector<float> &norm, const std::vector<float> &uv,
                                const std::vector<uint32> &indices)
{
	if (pos.empty() || indices.empty())
		return -1;
	CJsonValue *jm = m_Meshes->push();
	jm->setString("name", name);
	CJsonValue *prims = jm->setArray("primitives");
	CJsonValue *prim = prims->push();
	CJsonValue *attrs = prim->setObject("attributes");
	size_t nVerts = pos.size() / 3;
	attrs->setInt("POSITION", addAccessorFloat(&pos[0], nVerts, 3, TARGET_ARRAY, true));
	if (norm.size() == nVerts * 3)
		attrs->setInt("NORMAL", addAccessorFloat(&norm[0], nVerts, 3, TARGET_ARRAY, false));
	if (uv.size() == nVerts * 2)
		attrs->setInt("TEXCOORD_0", addAccessorFloat(&uv[0], nVerts, 2, TARGET_ARRAY, false));
	prim->setInt("indices", addAccessorU32(&indices[0], indices.size(), TARGET_ELEMENT));
	return (sint)m_Meshes->size() - 1;
}

// ---------------------------------------------------------------------------------------------

bool CGltfBuilder::save(const std::string &gltfPath)
{
	std::string binPath = gltfPath;
	std::string::size_type dot = binPath.rfind('.');
	if (dot != std::string::npos)
		binPath = binPath.substr(0, dot);
	binPath += ".bin";

	// A glTF buffer must have byteLength >= 1, and a zero-byte .bin trips CIFile::open on the
	// reader side — pad the (mesh-less scene) case to one dword.
	if (m_Bin.empty() && m_BufferViews->size() == 0)
		m_Bin.resize(4, 0);
	CJsonValue *buffers = m_Root.getMutable("buffers");
	buffers->clear();
	CJsonValue *buf = buffers->push();
	buf->setInt("byteLength", (sint64)m_Bin.size());
	buf->setString("uri", CFile::getFilename(binPath));

	std::string json;
	m_Root.write(json);

	FILE *fp = fopen(gltfPath.c_str(), "wb");
	if (!fp)
		return false;
	bool ok = fwrite(json.data(), 1, json.size(), fp) == json.size();
	fclose(fp);
	if (!ok)
		return false;

	fp = fopen(binPath.c_str(), "wb");
	if (!fp)
		return false;
	ok = m_Bin.empty() || fwrite(&m_Bin[0], 1, m_Bin.size(), fp) == m_Bin.size();
	fclose(fp);
	return ok;
}

} /* namespace GLTFBUILD */

/* end of file */
