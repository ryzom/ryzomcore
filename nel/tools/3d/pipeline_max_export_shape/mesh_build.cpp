/**
 * \file mesh_build.cpp
 * \brief See mesh_build.h.
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
#include "mesh_build.h"

#include <cstdio>
#include <cstring>
#include <cmath>

#include <nel/misc/plane.h>
#include <nel/3d/mrm_parameters.h>

#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace NLMISC;
using namespace NL3D;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MESHEVAL;
using namespace MATBUILD;

// The Max transform helpers now live in pipeline_max_export_common/max_scene.h; the getLocalMatrix
// public API is re-exported through mesh_build.h (SCENELIB provides getNodeTM / SNodeTMCache /
// readObjectOffset), decompMatrix / convertMatrix resolve here.
using MAXSCENE::decompMatrix;
using MAXSCENE::convertMatrix;

namespace MESHBUILD {

// Morpher modifier (Morpher.dlm) — refs 101+i are the blend-shape target nodes (design §10d).
static const NLMISC::CClassId CLASSID_MORPHER(0x17bb6854, 0xa5cba2a3);

// Rendering-control flag bits (chunk 0x099c, read via the typed CNodeImpl renderFlags overlay).
#define NODE_RENDERFLAG_CASTSHADOW 0x00000200
#define NODE_RENDERFLAG_RCVSHADOW 0x00000400

// ---------------------------------------------------------------------------------------------
// Max buildRenderNormals replication.
//
// Per face: face normal = Normalize(cross(v1 - v0, v2 - v1)) in float.
// Per vertex: an RNormal list; for each adjacent face (in face order) with a non-zero smoothing
// group, the first RNormal sharing a smoothing bit accumulates (normal += faceNormal, smGroup
// |= face.smGroup); otherwise a new RNormal is appended. getLocalNormal picks the first RNormal
// sharing a bit with the face's group (face normal for group 0), then the exporter normalizes.

struct SRNormal
{
	float N[3];
	uint32 SmGroup;
};

struct SRenderNormals
{
	std::vector<float> FaceNormals; // 3 per face
	std::vector<std::vector<SRNormal> > VertexNormals;
};

static void computeFaceNormal(const SEvalMesh &mesh, uint face, float out[3], bool normalize)
{
	const SEvalFace &f = mesh.Faces[face];
	const Point3M &v0 = mesh.Verts[f.V[0]];
	const Point3M &v1 = mesh.Verts[f.V[1]];
	const Point3M &v2 = mesh.Verts[f.V[2]];
	// x87-flavored arithmetic: extended-precision intermediates, float at the Point3 stores
	// (the reference plugin is a 32-bit x87 build; double emulates the 80-bit temporaries).
	long double ax = (long double)v1.x - v0.x, ay = (long double)v1.y - v0.y, az = (long double)v1.z - v0.z;
	long double bx = (long double)v2.x - v1.x, by = (long double)v2.y - v1.y, bz = (long double)v2.z - v1.z;
	float nx = (float)(ay * bz - az * by);
	float ny = (float)(az * bx - ax * bz);
	float nz = (float)(ax * by - ay * bx);
	if (normalize)
	{
		long double len = sqrtl((long double)nx * nx + (long double)ny * ny + (long double)nz * nz);
		if (len > 0.0)
		{
			nx = (float)(nx / len);
			ny = (float)(ny / len);
			nz = (float)(nz / len);
		}
	}
	out[0] = nx;
	out[1] = ny;
	out[2] = nz;
}

// Corner angle of face at corner c: acos of the dot of the two normalized edges leaving the
// corner (Max's angle-weighted render normals; discriminated against the reference exports:
// plain and area-weighted sums both fail, the angle weighting matches).
static long double cornerAngle(const SEvalMesh &mesh, const SEvalFace &f, uint c)
{
	const Point3M &a = mesh.Verts[f.V[c]];
	const Point3M &b = mesh.Verts[f.V[(c + 1) % 3]];
	const Point3M &d = mesh.Verts[f.V[(c + 2) % 3]];
	// Keep the whole edge-angle computation at extended precision: rounding the normalized
	// edges to float before the dot loses ~1e-7, which acos amplifies to ~3e-4 near 0/pi —
	// exactly the error class the reference does NOT show (x87 keeps these in registers).
	long double e1x = (long double)b.x - a.x, e1y = (long double)b.y - a.y, e1z = (long double)b.z - a.z;
	long double e2x = (long double)d.x - a.x, e2y = (long double)d.y - a.y, e2z = (long double)d.z - a.z;
	long double l1 = sqrtl(e1x * e1x + e1y * e1y + e1z * e1z);
	long double l2 = sqrtl(e2x * e2x + e2y * e2y + e2z * e2z);
	if (l1 > 0.0) { e1x /= l1; e1y /= l1; e1z /= l1; }
	if (l2 > 0.0) { e2x /= l2; e2y /= l2; e2z /= l2; }
	long double d12 = e1x * e2x + e1y * e2y + e1z * e2z;
	if (d12 > 1.0) d12 = 1.0;
	if (d12 < -1.0) d12 = -1.0;
	return acosl(d12);
}

static void buildRenderNormals(const SEvalMesh &mesh, SRenderNormals &out)
{
	uint nf = (uint)mesh.Faces.size();
	uint nv = (uint)mesh.Verts.size();
	out.FaceNormals.resize(nf * 3);
	out.VertexNormals.clear();
	out.VertexNormals.resize(nv);
	for (uint f = 0; f < nf; ++f)
		computeFaceNormal(mesh, f, &out.FaceNormals[f * 3], true);
	for (uint f = 0; f < nf; ++f)
	{
		const SEvalFace &face = mesh.Faces[f];
		if (!face.SmGroup) continue;
		const float *fn = &out.FaceNormals[f * 3];
		for (uint c = 0; c < 3; ++c)
		{
			uint32 v = face.V[c];
			if (v >= nv) continue;
			long double w = cornerAngle(mesh, face, c);
			std::vector<SRNormal> &rns = out.VertexNormals[v];
			bool merged = false;
			// PMB_SMOOTH_MODE experiment: 0 = first-match on shared bit, OR groups in (default);
			// 1 = first-match, no OR; 2 = exact-equality match.
			static int smMode = -1;
			if (smMode < 0)
			{
				const char *env = getenv("PMB_SMOOTH_MODE");
				smMode = env ? atoi(env) : 0;
			}
			// The weighted contribution materializes as a FLOAT Point3 before the add (Max's
			// Point3 operator* returns a float struct; operator+= then adds float to float) —
			// exact-opposite doubled faces (a two-sided fin, same triangle both windings) must
			// cancel to EXACTLY zero: fl(w·n) + (-fl(w·n)) == 0, where an unrounded long-double
			// delta leaves a rounding residual that normalizes into a garbage unit normal
			// (clapclap/sagass_selle clod class — the reference lands the (1,0,0) zero-normalize
			// fallback below).
			float dwx = (float)((long double)fn[0] * w);
			float dwy = (float)((long double)fn[1] * w);
			float dwz = (float)((long double)fn[2] * w);
			for (uint r = 0; r < rns.size(); ++r)
			{
				bool match = smMode == 2 ? (rns[r].SmGroup == face.SmGroup)
				                         : ((rns[r].SmGroup & face.SmGroup) != 0);
				if (match)
				{
					rns[r].N[0] = (float)((long double)rns[r].N[0] + dwx);
					rns[r].N[1] = (float)((long double)rns[r].N[1] + dwy);
					rns[r].N[2] = (float)((long double)rns[r].N[2] + dwz);
					if (smMode != 1)
						rns[r].SmGroup |= face.SmGroup;
					merged = true;
					break;
				}
			}
			if (!merged)
			{
				SRNormal rn;
				rn.N[0] = dwx;
				rn.N[1] = dwy;
				rn.N[2] = dwz;
				rn.SmGroup = face.SmGroup;
				rns.push_back(rn);
			}
		}
	}
	// Normalize accumulated normals (Max normalizes RNormals at the end of buildRenderNormals).
	// Zero-length (exact cancellation of a two-sided fin's doubled faces) falls back to
	// (1,0,0) — Max Point3::Normalize's documented zero-length result, corpus-pinned by the
	// clapclap/sagass_selle clod references carrying exactly (1,0,0) on those corners.
	for (uint v = 0; v < nv; ++v)
	{
		for (uint r = 0; r < out.VertexNormals[v].size(); ++r)
		{
			float *n = out.VertexNormals[v][r].N;
			long double len = sqrtl((long double)n[0] * n[0] + (long double)n[1] * n[1] + (long double)n[2] * n[2]);
			if (len > 0.0)
			{
				n[0] = (float)(n[0] / len);
				n[1] = (float)(n[1] / len);
				n[2] = (float)(n[2] / len);
			}
			else
			{
				n[0] = 1.f;
				n[1] = 0.f;
				n[2] = 0.f;
			}
		}
	}
}

// CExportNel::getLocalNormal
static void getLocalNormal(const SEvalMesh &mesh, const SRenderNormals &rn, uint face, uint corner, float out[3])
{
	const SEvalFace &f = mesh.Faces[face];
	uint32 v = f.V[corner];
	if (!f.SmGroup || v >= rn.VertexNormals.size())
	{
		memcpy(out, &rn.FaceNormals[face * 3], 12);
		return;
	}
	const std::vector<SRNormal> &rns = rn.VertexNormals[v];
	if (rns.size() == 1)
	{
		memcpy(out, rns[0].N, 12);
		return;
	}
	for (uint r = 0; r < rns.size(); ++r)
	{
		if (rns[r].SmGroup & f.SmGroup)
		{
			memcpy(out, rns[r].N, 12);
			return;
		}
	}
	memcpy(out, &rn.FaceNormals[face * 3], 12);
}

void buildBaseMeshInterface(CMeshBase::CMeshBaseBuild &buildMesh, SMaxMeshBaseBuild &maxBaseBuild,
                            INode &node, SNodeTMCache &tmCache, const Matrix3M &nodeBasis,
                            bool exportLighting)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);

	// Shadow flags from the rendering-control dword (typed CNodeImpl overlay)
	uint32 renderFlags = 0;
	bool found = n && n->renderFlags(renderFlags);
	if (!found) renderFlags = NODE_RENDERFLAG_CASTSHADOW | NODE_RENDERFLAG_RCVSHADOW;
	buildMesh.bCastShadows = (renderFlags & NODE_RENDERFLAG_CASTSHADOW) != 0;
	buildMesh.bRcvShadows = (renderFlags & NODE_RENDERFLAG_RCVSHADOW) != 0;

	// RealTime lighting info
	NL3D::CMaterial::TShader shader;
	bool needVp = hasMaterialWithShaderForVP(node, shader);
	if (!needVp)
		buildMesh.UseLightingLocalAttenuation = getScriptAppDataInt(n, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, 0) == 1;
	else
		buildMesh.UseLightingLocalAttenuation = false;

	// Camera collision
	sint appDataCameraCol = getScriptAppDataInt(n, NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION, 0);
	if (appDataCameraCol >= 3)
		buildMesh.CollisionMeshGeneration = NL3D::CMeshBase::ForceCameraCol;
	else
		buildMesh.CollisionMeshGeneration = (NL3D::CMeshBase::TCameraCollisionGenerate)appDataCameraCol;

	// Materials
	buildMaterials(buildMesh.Materials, maxBaseBuild, node, exportLighting);

	// WindTree vertex program forces vertex color
	int vpId = getScriptAppDataInt(n, NEL3D_APPDATA_VERTEXPROGRAM_ID, 0);
	if (vpId == 1)
		maxBaseBuild.NeedVertexColor = true;

	// Default transformation
	NLMISC::CVector pos, scale;
	NLMISC::CQuat rot;
	decompMatrix(scale, rot, pos, nodeBasis);
	buildMesh.DefaultScale = scale;
	buildMesh.DefaultRotQuat = rot;
	buildMesh.DefaultRotEuler = NLMISC::CVector(0, 0, 0);
	buildMesh.DefaultPivot = NLMISC::CVector(0, 0, 0);
	buildMesh.DefaultPos = pos;

	// Morpher target names (modifier refs 101+i are the target nodes; reference
	// buildBaseMeshInterface pushes factor 0.0 + the target node's name per channel).
	{
		std::vector<CSceneClass *> mods;
		baseObjectOf(node, &mods, NULL);
		for (uint mi = 0; mi < mods.size(); ++mi)
		{
			if (mods[mi]->classDesc()->classId() != CLASSID_MORPHER) continue;
			CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mods[mi]);
			for (uint i = 0; rm && i < 100; ++i)
			{
				if (101 + i >= rm->nbReferences()) break;
				INode *target = dynamic_cast<INode *>(rm->getReference(101 + i));
				if (!target) continue;
				buildMesh.DefaultBSFactors.push_back(0.0f);
				buildMesh.BSNames.push_back(nodeName(*target));
			}
			break;
		}
	}
	(void)tmCache;
}

void buildMeshInterface(const SEvalMesh &mesh, CMesh::CMeshBuild &buildMesh,
                        const CMeshBase::CMeshBaseBuild &buildBaseMesh,
                        const SMaxMeshBaseBuild &maxBaseBuild,
                        INode &node, SNodeTMCache &tmCache, bool skinned,
                        const NLMISC::CMatrix *morphFinalSpace)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);

	// Normals
	SRenderNormals renderNormals;
	buildRenderNormals(mesh, renderNormals);

	buildMesh.VertexFlags = CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag;

	// Export matrix (reference export_mesh.cpp:671): a SKINNED mesh exports its vertices in
	// WORLD space (ToExportSpace = objectTM — skinning overrides the node transform at
	// runtime, so the bind-pose world positions are what the skin weights deform); a
	// non-skinned mesh exports in the node-offset local space, objectToLocal =
	// objectTM * Inverse(nodeTM). DefaultPos/Rot/Scale stay the node's local transform in
	// both cases (buildBaseMeshInterface is unconditional in the reference too).
	NLMISC::CMatrix toExportSpace;
	NLMISC::CMatrix fromExportSpace;
	{
		Matrix3M nodeTM = getNodeTM(&node, tmCache);
		Point3M opos;
		QuatM orot;
		ScaleValueM oscale;
		readObjectOffset(n, opos, orot, oscale);
		Matrix3M offsetTM = composePRS(opos, orot, oscale);
		Matrix3M objectTM = offsetTM * nodeTM;
		if (skinned)
		{
			convertMatrix(toExportSpace, objectTM);
		}
		else
		{
			Matrix3M objectToLocal = objectTM * inverseM3(nodeTM);
			convertMatrix(toExportSpace, objectToLocal);
			// Morph target: right-multiply the caller's finalSpace (reference
			// export_mesh.cpp:725, `ToExportSpace = newBasis*ToExportSpace*finalSpace` with
			// newBasis = Identity from createMeshBuild) — see the header comment.
			if (morphFinalSpace)
				toExportSpace = toExportSpace * (*morphFinalSpace);
		}
		fromExportSpace = toExportSpace;
		fromExportSpace.invert();
	}

	// RGB vertices
	if (mesh.mapSupport(0) && maxBaseBuild.NeedVertexColor)
		buildMesh.VertexFlags |= CVertexBuffer::PrimaryColorFlag;

	// UV channel routing
	uint mappingChannelUsed = 0;
	uint i;
	for (i = 0; i < MAX_MAX_TEXTURE; i++)
	{
		if (maxBaseBuild.UVRouting[i] == i)
			mappingChannelUsed |= 1 << i;
	}
	buildMesh.VertexFlags |= mappingChannelUsed << CVertexBuffer::TexCoord0;

	const uint count = std::min((uint)MAX_MAX_TEXTURE, (uint)CVertexBuffer::MaxStage);
	for (i = 0; i < count; i++)
	{
		if (maxBaseBuild.UVRouting[i] == 0xff)
			buildMesh.UVRouting[i] = (uint8)i;
		else
			buildMesh.UVRouting[i] = maxBaseBuild.UVRouting[i];
	}
	for (; i < CVertexBuffer::MaxStage; i++)
		buildMesh.UVRouting[i] = (uint8)i;

	// Vertices
	uint nNumVertices = (uint)mesh.Verts.size();
	buildMesh.Vertices.resize(nNumVertices);
	for (uint vertex = 0; vertex < nNumVertices; vertex++)
	{
		NLMISC::CVector v(mesh.Verts[vertex].x, mesh.Verts[vertex].y, mesh.Verts[vertex].z);
		buildMesh.Vertices[vertex] = toExportSpace * v;
	}

	// Does this object use a vertex program that needs vertex colors?
	bool vpColorVertex = false;
	if (getScriptAppDataInt(n, NEL3D_APPDATA_VERTEXPROGRAM_ID, 0) == 1)
		vpColorVertex = true;

	// Faces
	uint nNumFaces = (uint)mesh.Faces.size();
	buildMesh.Faces.resize(nNumFaces);
	for (uint face = 0; face < nNumFaces; face++)
	{
		const SEvalFace &pFace = mesh.Faces[face];

		// Material ID
		sint nMaterialID = (sint)pFace.matID();
		if (maxBaseBuild.NumMaterials > 0)
			nMaterialID %= maxBaseBuild.NumMaterials;
		else
			nMaterialID = 0;
		nMaterialID += maxBaseBuild.FirstMaterial;

		buildMesh.Faces[face].MaterialId = nMaterialID;
		buildMesh.Faces[face].SmoothGroup = pFace.SmGroup;

		const CMaterial &material = buildBaseMesh.Materials[nMaterialID];
		CRGBA diffuse = material.getDiffuse();
		CRGBA color = material.getColor();
		uint8 opacity = material.getOpacity();
		bool isLighted = material.isLighted();

		const SMaterialInfo &matInfo = maxBaseBuild.MaterialInfo[nMaterialID - maxBaseBuild.FirstMaterial];

		for (uint corner = 0; corner < 3; corner++)
		{
			CMesh::CCorner &pCorner = buildMesh.Faces[face].Corner[corner];
			pCorner.Vertex = pFace.V[corner];

			// Normal: transform through the inverted matrix as a plane (see the reference)
			float vNormal[3];
			getLocalNormal(mesh, renderNormals, face, corner, vNormal);
			NLMISC::CPlane plane(vNormal[0], vNormal[1], vNormal[2], 0.f);
			plane = plane * fromExportSpace;
			pCorner.Normal = plane.getNormal();
			pCorner.Normal.normalize();

			// UVs
			uint nNumChannelUsed = (uint)matInfo.RemapChannel.size();
			uint uv;
			for (uv = 0; uv < nNumChannelUsed && uv < CVertexBuffer::MaxStage; uv++)
			{
				pCorner.Uvws[uv].U = 0.f;
				pCorner.Uvws[uv].V = 0.f;
				pCorner.Uvws[uv].W = 0.f;

				sint nMaxChan = matInfo.RemapChannel[uv].IndexInMaxMaterial;
				if (nMaxChan >= 0)
				{
					if (!mesh.mapSupport(nMaxChan))
						nMaxChan = matInfo.RemapChannel[uv].IndexInMaxMaterialAlternative;

					// UV transform matrix: identity when TextureMatrixEnabled (the runtime
					// applies it), else the UVGen matrix (identity until decoded).
					float fCropU = matInfo.RemapChannel[uv].CropU;
					float fCropV = matInfo.RemapChannel[uv].CropV;
					float fCropW = matInfo.RemapChannel[uv].CropW;
					float fCropH = matInfo.RemapChannel[uv].CropH;

					if (nMaxChan >= 0 && mesh.mapSupport(nMaxChan))
					{
						const SMapChannel &mc = mesh.Maps.find(nMaxChan)->second;
						uint32 nMapVert = mc.Faces[face].T[corner];
						if (nMapVert < mc.Verts.size())
						{
							const Point3M &mapVert = mc.Verts[nMapVert];
							// uvTransformed = mapVert * uvMatrix (identity for now)
							float u = mapVert.x;
							float v = mapVert.y;
							u = u * fCropW + fCropU;
							v = (1.f - v) * fCropH + fCropV;
							pCorner.Uvws[uv].U = u;
							pCorner.Uvws[uv].V = v;
							pCorner.Uvws[uv].W = 0;
						}
					}
				}
			}
			for (; uv < nNumChannelUsed && uv < CVertexBuffer::MaxStage; uv++)
			{
				pCorner.Uvws[uv].U = 0.f;
				pCorner.Uvws[uv].V = 0.f;
				pCorner.Uvws[uv].W = 0.f;
			}

			// Alpha
			pCorner.Color.A = 255;
			if (matInfo.AlphaVertex && (buildMesh.VertexFlags & CVertexBuffer::PrimaryColorFlag))
			{
				uint channel = matInfo.AlphaVertexChannel;
				if (mesh.mapSupport(channel))
				{
					const SMapChannel &mc = mesh.Maps.find(channel)->second;
					uint32 nMapVert = mc.Faces[face].T[corner];
					if (nMapVert < mc.Verts.size())
					{
						const Point3M &colorVert = mc.Verts[nMapVert];
						float fR = colorVert.x * 255.f + 0.5f;
						float fG = colorVert.y * 255.f + 0.5f;
						float fB = colorVert.z * 255.f + 0.5f;
						float fA = (fR + fG + fB) / 3;
						clamp(fA, 0.f, 255.f);
						pCorner.Color.A = (uint8)fA;
					}
				}
			}
			pCorner.Color.A = (uint8)((pCorner.Color.A * opacity) / 255);

			// Color
			pCorner.Color.R = 255;
			pCorner.Color.G = 255;
			pCorner.Color.B = 255;
			if ((matInfo.ColorVertex && (buildMesh.VertexFlags & CVertexBuffer::PrimaryColorFlag)) || vpColorVertex)
			{
				if (mesh.mapSupport(0))
				{
					const SMapChannel &mc = mesh.Maps.find(0)->second;
					uint32 nMapVert = mc.Faces[face].T[corner];
					if (nMapVert < mc.Verts.size())
					{
						const Point3M &colorVert = mc.Verts[nMapVert];
						float fR = colorVert.x * 255.f + 0.5f;
						float fG = colorVert.y * 255.f + 0.5f;
						float fB = colorVert.z * 255.f + 0.5f;
						clamp(fR, 0.f, 255.f);
						clamp(fG, 0.f, 255.f);
						clamp(fB, 0.f, 255.f);
						pCorner.Color.R = (uint8)fR;
						pCorner.Color.G = (uint8)fG;
						pCorner.Color.B = (uint8)fB;
					}
				}
			}

			// Modulate the color
			if (!vpColorVertex)
			{
				uint8 alphaBackup = pCorner.Color.A;
				pCorner.Color.modulateFromColor(pCorner.Color, isLighted ? diffuse : color);
				pCorner.Color.A = alphaBackup;
			}
		}
	}

	// Interfaces (MRM normal correction): none until the interface-mesh decode lands.
	buildMesh.Interfaces.clear();
	buildMesh.InterfaceLinks.clear();

	// Vertex program
	buildMesh.MeshVertexProgram = NULL;
	// TODO: CMeshVPWindTree from the VPWT appdata; per-pixel-lighting VP from material shaders.
}

void buildMRMParameters(CSceneClass *node, CMRMParameters &params)
{
	params.NLods = getScriptAppDataInt(node, NEL3D_APPDATA_LOD_NB_LOD, 11);
	params.Divisor = getScriptAppDataInt(node, NEL3D_APPDATA_LOD_DIVISOR, 20);
	switch (getScriptAppDataInt(node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, 1))
	{
	case 0:
		params.SkinReduction = CMRMParameters::SkinReductionMin;
		break;
	case 1:
		params.SkinReduction = CMRMParameters::SkinReductionMax;
		break;
	case 2:
		params.SkinReduction = CMRMParameters::SkinReductionBest;
		break;
	}
	params.DistanceFinest = getScriptAppDataFloat(node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, 5.f);
	params.DistanceMiddle = getScriptAppDataFloat(node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, 30.f);
	params.DistanceCoarsest = getScriptAppDataFloat(node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, 200.f);
}

void buildBSList(INode &node, SNodeTMCache &tmCache,
                 const std::vector<CSceneClass *> &mods,
                 const NL3D::CMesh::CMeshBuild &exportMesh, bool skinned,
                 bool exportLighting,
                 std::vector<NL3D::CMesh::CMeshBuild *> &bsList)
{
	static const NLMISC::CClassId CLASSID_MORPHER(0x17bb6854, 0xa5cba2a3);
	CReferenceMaker *morph = NULL;
	for (uint i = 0; i < mods.size() && !morph; ++i)
		if (mods[i]->classDesc()->classId() == CLASSID_MORPHER)
			morph = dynamic_cast<CReferenceMaker *>(mods[i]);
	if (!morph)
		return;

	NLMISC::CMatrix finalSpace = NLMISC::CMatrix::Identity;
	if (skinned)
		MAXSCENE::convertMatrix(finalSpace, getNodeTM(&node, tmCache));

	for (uint i = 0; i < 100; ++i)
	{
		if (101 + i >= morph->nbReferences())
			break;
		INode *target = dynamic_cast<INode *>(morph->getReference(101 + i));
		if (!target)
			continue;
		SEvalMesh tmesh;
		if (!MESHEVAL::evalNodeMesh(*target, tmesh, NULL))
		{
			fprintf(stderr, "WARNING: morph target '%s' of '%s' failed mesh eval; channel dropped\n",
			        nodeName(*target).c_str(), nodeName(node).c_str());
			continue;
		}
		SMaxMeshBaseBuild tMax;
		NL3D::CMeshBase::CMeshBaseBuild tBase;
		buildBaseMeshInterface(tBase, tMax, *target, tmCache, getLocalMatrix(*target, tmCache),
		                       exportLighting);
		NL3D::CMesh::CMeshBuild *mb = new NL3D::CMesh::CMeshBuild;
		buildMeshInterface(tmesh, *mb, tBase, tMax, *target, tmCache, false, &finalSpace);
		if (mb->Vertices.size() != exportMesh.Vertices.size())
		{
			fprintf(stderr, "WARNING: morph target '%s' of '%s' has %u verts vs base %u; channel dropped\n",
			        nodeName(*target).c_str(), nodeName(node).c_str(),
			        (uint)mb->Vertices.size(), (uint)exportMesh.Vertices.size());
			delete mb;
			continue;
		}
		// Interface-vert corner normals come from the (welded) base.
		if (exportMesh.InterfaceVertexFlag.size() != 0)
		{
			for (uint k = 0; k < mb->Faces.size() && k < exportMesh.Faces.size(); ++k)
				for (uint l = 0; l < 3; ++l)
				{
					uint vert = mb->Faces[k].Corner[l].Vertex;
					if (vert < exportMesh.InterfaceVertexFlag.size() && exportMesh.InterfaceVertexFlag.get(vert))
						mb->Faces[k].Corner[l].Normal = exportMesh.Faces[k].Corner[l].Normal;
				}
		}
		bsList.push_back(mb);
	}
}

} /* namespace MESHBUILD */

/* end of file */
