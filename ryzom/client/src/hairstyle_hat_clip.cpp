// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2018  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdpch.h"
#include "hairstyle_hat_clip.h"

#include <map>
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/matrix.h"
#include "nel/3d/scene.h"
#include "nel/3d/u_skeleton.h"
#include "nel/3d/shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/bone.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace
{
	// ---------------------------------------------------------------------
	// Geometry: generalized winding number, used both to classify a point as
	// inside/outside the hat's volume, and (via bisection) to find exactly
	// where a hairstyle edge crosses the hat's surface -- this gives us the
	// "cut" edge without needing explicit triangle-vs-mesh intersection math,
	// and stays robust even if the hat mesh isn't perfectly watertight.
	// ---------------------------------------------------------------------

	double signedSolidAngle(const CVector &a, const CVector &b, const CVector &c)
	{
		double la = (double)a.norm();
		double lb = (double)b.norm();
		double lc = (double)c.norm();
		double numer = (double)(a * (b ^ c));
		double denom = la * lb * lc + (double)(a * b) * lc + (double)(b * c) * la + (double)(c * a) * lb;
		if (numer == 0.0 && denom == 0.0)
			return 0.0;
		return 2.0 * atan2(numer, denom);
	}

	double windingNumber(const CVector &p, const vector<CVector> &hatVerts, const vector<uint32> &hatIndices)
	{
		double sum = 0.0;
		for (uint i = 0; i + 2 < hatIndices.size(); i += 3)
		{
			sum += signedSolidAngle(hatVerts[hatIndices[i]] - p, hatVerts[hatIndices[i + 1]] - p, hatVerts[hatIndices[i + 2]] - p);
		}
		return sum / (4.0 * Pi);
	}

	bool isInsideHat(const CVector &p, const vector<CVector> &hatVerts, const vector<uint32> &hatIndices)
	{
		return windingNumber(p, hatVerts, hatIndices) > 0.5;
	}

	// Bisects [a, b] (endpoints on opposite sides) for where the winding number field
	// crosses the inside/outside threshold. Returns t in [0, 1], a at t=0, b at t=1.
	float bisectCrossing(const CVector &a, const CVector &b, bool aInside,
		const vector<CVector> &hatVerts, const vector<uint32> &hatIndices)
	{
		float lo = 0.f, hi = 1.f;
		bool loInside = aInside;
		for (uint iter = 0; iter < 24; ++iter)
		{
			float mid = (lo + hi) * 0.5f;
			bool midInside = isInsideHat(a + (b - a) * mid, hatVerts, hatIndices);
			if (midInside == loInside)
				lo = mid;
			else
				hi = mid;
		}
		return (lo + hi) * 0.5f;
	}

	struct SVertex
	{
		CVector Pos;
		CVector Normal;
		CUV Uv0;
		CMesh::CSkinWeight Skin;
	};

	struct SFace
	{
		uint32 V[3];
		sint32 MaterialId;
	};

	struct SMeshData
	{
		vector<SVertex> Verts;
		vector<SFace> Faces;
		vector<CMaterial> Materials;
		vector<string> BonesNames;
		bool Skinned;
	};

	// ---------------------------------------------------------------------
	// Coordinate spaces: the hairstyle's and the hat's raw vertex data are each in their
	// OWN local space (a hat's is a small space near its own object pivot; a hairstyle's
	// is bind-pose/reference space) -- comparing them directly is comparing unrelated
	// spaces (confirmed empirically: a real pair had wildly different, non-overlapping
	// extents). Both need to be brought into a COMMON space for containment testing.
	// Rather than hand-derive a bind-pose transform (tried first, wrong: an unintended
	// ~90-degree rotation, this engine's bone/mesh axis conventions are non-obvious),
	// this uses the skeleton's own CURRENT, already-computed world matrices -- the exact
	// values USkeleton::stickObject() and real vertex skinning use for actual rendering,
	// so there is no room for a convention mismatch. The hat, being a rigid stick object,
	// only needs the target bone's world matrix. The hairstyle needs real per-vertex
	// weighted skinning (CBone::getWorldMatrix() * CBoneBase::InvBindPos per weight),
	// since different vertices can be weighted to different bones. This world-space
	// classification is used ONLY to decide what's inside/outside and to find edge
	// crossings (as a blend parameter t, reused to interpolate the ORIGINAL bind-pose
	// attributes for the output mesh) -- the mesh that actually gets built and bound
	// stays in bind-pose space throughout, only ever skinned normally at render time.
	// ---------------------------------------------------------------------

	bool boneWorldMatrix(USkeleton &skeleton, const string &boneName, CMatrix &out)
	{
		CSkeletonModel *skelModel = skeleton.getObjectPtr();
		if (!skelModel)
			return false;
		sint boneId = skeleton.getBoneIdByName(boneName);
		if (boneId < 0 || (uint)boneId >= skelModel->Bones.size())
			return false;
		// This runs from updateVisualPropertyVpa, early enough that the skeleton may not
		// have computed this bone yet this session (its world matrix would still be the
		// default identity) -- force it, same as USkeleton::forceComputeBone() is meant for.
		skeleton.forceComputeBone((uint)boneId);
		out = skelModel->Bones[(uint)boneId].getWorldMatrix();
		return true;
	}

	// Maps a mesh's own local bone-name list (SMeshData::BonesNames, what SkinWeight's
	// MatrixId indexes into) to actual skeleton bone indices, -1 if a name isn't found.
	vector<sint> resolveSkeletonBoneIds(USkeleton &skeleton, const vector<string> &meshBoneNames)
	{
		vector<sint> ids(meshBoneNames.size());
		for (uint i = 0; i < meshBoneNames.size(); ++i)
			ids[i] = skeleton.getBoneIdByName(meshBoneNames[i]);
		return ids;
	}

	// Real (current pose) weighted skinning of one bind-pose vertex, for classification
	// only -- see this section's own comment above.
	CVector skinToCurrentWorld(const SVertex &v, const vector<sint> &localToSkeletonBoneId, CSkeletonModel *skelModel)
	{
		CVector result(0.f, 0.f, 0.f);
		float totalWeight = 0.f;
		for (uint w = 0; w < NL3D_MESH_SKINNING_MAX_MATRIX; ++w)
		{
			float weight = v.Skin.Weights[w];
			if (weight <= 0.f)
				continue;
			uint32 localId = v.Skin.MatrixId[w];
			if (localId >= localToSkeletonBoneId.size())
				continue;
			sint boneId = localToSkeletonBoneId[localId];
			if (boneId < 0 || (uint)boneId >= skelModel->Bones.size())
				continue;
			const CBone &bone = skelModel->Bones[(uint)boneId];
			CVector p = bone.getWorldMatrix() * (bone.getBoneBase().InvBindPos * v.Pos);
			result += p * weight;
			totalWeight += weight;
		}
		return totalWeight > 0.f ? result * (1.f / totalWeight) : v.Pos;
	}

	// A hat's real, rendered geometry is often thin/open (e.g. a flat brim ring plus a
	// small dome, no solid "shell" over the sides) -- a hairstyle poking out doesn't
	// necessarily cross that geometry's own surface at all, it just clears the flat brim
	// and continues outward past it, so testing containment against the visible hat mesh
	// itself under-detects what should be hidden. An artist can provide a separate,
	// generously-oversized "<hatShapeName>_mask.shape" purpose-built as the clipping
	// volume (never rendered) -- if present, that's what's used for containment; the
	// actual displayed hat is completely unaffected either way.
	string deriveMaskShapeName(const string &hatShapeName)
	{
		const string suffix = "_mask.shape";
		const string ext = ".shape";
		if (hatShapeName.size() >= ext.size() && hatShapeName.compare(hatShapeName.size() - ext.size(), ext.size(), ext) == 0)
			return hatShapeName.substr(0, hatShapeName.size() - ext.size()) + suffix;
		return hatShapeName + suffix;
	}

	// ---------------------------------------------------------------------
	// Private (non-shape-bank) mesh loading and raw geometry extraction.
	// ---------------------------------------------------------------------

	// Loads shapeName as a brand new, never-registered-in-CShapeBank IShape, mirroring
	// CShapeBank::load()'s own file-loading code (shape_bank.cpp) but skipping the
	// cache/add() step -- so the returned mesh's vertex/index buffers are never
	// "resident" (never claimed by the driver), which is required to read them back.
	IShape *loadPrivateShape(const string &shapeName)
	{
		// CPath's lookup table is keyed in lowercase (see CShapeBank::load, which this
		// mirrors); shape names arrive with their original casing.
		string path = CPath::lookup(toLowerAscii(shapeName), false, false);
		if (path.empty())
		{
			nlwarning("buildClippedHairstyleInstance: CPath::lookup found nothing for '%s'", shapeName.c_str());
			return NULL;
		}

		CShapeStream stream;
		CIFile file;
		if (!file.open(path))
		{
			nlwarning("buildClippedHairstyleInstance: CIFile::open failed for '%s' (%s)", shapeName.c_str(), path.c_str());
			return NULL;
		}
		file.serial(stream);
		file.close();

		if (!stream.getShapePointer())
			nlwarning("buildClippedHairstyleInstance: stream deserialized no shape for '%s'", shapeName.c_str());

		return stream.getShapePointer();
	}

	bool extractMesh(CMesh *mesh, SMeshData &out)
	{
		const CMeshGeom &geom = mesh->getMeshGeom();
		const CVertexBuffer &vb = geom.getVertexBuffer();
		if (vb.isResident())
			return false;

		out.Skinned = geom.isSkinned();
		out.BonesNames = geom.getBonesName();

		uint16 fmt = vb.getVertexFormat();
		bool hasNormal = (fmt & CVertexBuffer::NormalFlag) != 0;
		bool hasUv0 = (fmt & CVertexBuffer::TexCoord0Flag) != 0;
		bool hasSkin = out.Skinned && (fmt & CVertexBuffer::PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag;

		uint numVerts = vb.getNumVertices();
		out.Verts.resize(numVerts);
		{
			CVertexBufferRead vba;
			vb.lock(vba);
			for (uint i = 0; i < numVerts; ++i)
			{
				SVertex &v = out.Verts[i];
				v.Pos = *vba.getVertexCoordPointer(i);
				v.Normal = hasNormal ? *vba.getNormalCoordPointer(i) : CVector(0.f, 0.f, 1.f);
				v.Uv0 = hasUv0 ? *vba.getTexCoordPointer(i, 0) : CUV(0.f, 0.f);
				if (hasSkin)
				{
					const CPaletteSkin *pal = vba.getPaletteSkinPointer(i);
					for (uint w = 0; w < NL3D_MESH_SKINNING_MAX_MATRIX; ++w)
					{
						v.Skin.MatrixId[w] = pal->MatrixId[w];
						v.Skin.Weights[w] = *vba.getWeightPointer(i, w);
					}
				}
			}
		}

		for (uint mb = 0; mb < geom.getNbMatrixBlock(); ++mb)
		{
			for (uint rp = 0; rp < geom.getNbRdrPass(mb); ++rp)
			{
				const CIndexBuffer &pb = geom.getRdrPassPrimitiveBlock(mb, rp);
				if (pb.isResident())
					return false;
				sint32 matId = (sint32)geom.getRdrPassMaterial(mb, rp);

				CIndexBufferRead iba;
				pb.lock(iba);
				uint numIdx = pb.getNumIndexes();
				if (pb.getFormat() == CIndexBuffer::Indices32)
				{
					const uint32 *idx = (const uint32 *)iba.getPtr();
					for (uint t = 0; t + 2 < numIdx; t += 3)
					{
						SFace f;
						f.V[0] = idx[t]; f.V[1] = idx[t + 1]; f.V[2] = idx[t + 2];
						f.MaterialId = matId;
						out.Faces.push_back(f);
					}
				}
				else
				{
					const uint16 *idx = (const uint16 *)iba.getPtr();
					for (uint t = 0; t + 2 < numIdx; t += 3)
					{
						SFace f;
						f.V[0] = idx[t]; f.V[1] = idx[t + 1]; f.V[2] = idx[t + 2];
						f.MaterialId = matId;
						out.Faces.push_back(f);
					}
				}
			}
		}

		out.Materials.resize(mesh->getNbMaterial());
		for (uint m = 0; m < mesh->getNbMaterial(); ++m)
			out.Materials[m] = mesh->getMaterial(m);

		return true;
	}

	// Hairstyles are CMeshMRMSkinned, not plain CMesh -- a completely separate sibling
	// class (both just derive from CMeshBase), with its own geometry API since it stores
	// progressive LOD deltas internally rather than a flat vertex/index buffer. We only
	// ever need the finest level (lod 0) here: our clipped output is a one-off, per-
	// character CMesh with no LOD of its own (see buildMesh) -- an accepted simplification
	// for this private, always-close-up combo, not a general MRM->CMesh converter.
	bool extractMeshMRMSkinned(CMeshMRMSkinned *mesh, SMeshData &out)
	{
		out.Skinned = true;
		out.BonesNames = mesh->getMeshGeom().getBonesName();

		vector<CMesh::CSkinWeight> skinWeights;
		mesh->getMeshGeom().getSkinWeights(skinWeights);

		CVertexBuffer vb;
		mesh->getVertexBuffer(vb);

		uint16 fmt = vb.getVertexFormat();
		bool hasNormal = (fmt & CVertexBuffer::NormalFlag) != 0;
		bool hasUv0 = (fmt & CVertexBuffer::TexCoord0Flag) != 0;

		uint numVerts = vb.getNumVertices();
		if (skinWeights.size() != numVerts)
		{
			nlwarning("extractMeshMRMSkinned: skin weight count (%u) != vertex count (%u)", (uint)skinWeights.size(), numVerts);
			return false;
		}

		if (mesh->getNbLod() == 0)
		{
			nlwarning("extractMeshMRMSkinned: MRM mesh has no lod");
			return false;
		}
		// Lod 0 is the COARSEST level, not the finest: chooseLod() (mesh_mrm_skinned.cpp:557-581)
		// maps alphaMRM=0 (far away) to numLod=0, and alphaMRM=1 (close up, max detail) to the
		// LAST lod index. We want full detail for this close-up private instance.
		const uint lod = mesh->getNbLod() - 1;

		// getVertexBuffer()/getSkinWeights() return RAW packed vertex data: wedge index i,
		// for i < geomorphs.size(), is only a *blend placeholder* for this lod (meant to be
		// interpolated with wedge geomorphs[i].End at render time for smooth LOD transitions)
		// -- using it as-is gives garbage attributes (this is what produced the deformed,
		// stretched-to-origin geometry). For a static render of this lod, resolve those
		// wedges to their End target first, same as Forgery's finest_skinned_lod().
		const vector<CMRMWedgeGeom> &geomorphs = mesh->getMeshGeom().getLodGeomorphs(lod);

		out.Verts.resize(numVerts);
		{
			CVertexBufferRead vba;
			vb.lock(vba);
			for (uint i = 0; i < numVerts; ++i)
			{
				uint srcIdx = (i < geomorphs.size() && geomorphs[i].End < numVerts) ? geomorphs[i].End : i;
				SVertex &v = out.Verts[i];
				v.Pos = *vba.getVertexCoordPointer(srcIdx);
				v.Normal = hasNormal ? *vba.getNormalCoordPointer(srcIdx) : CVector(0.f, 0.f, 1.f);
				v.Uv0 = hasUv0 ? *vba.getTexCoordPointer(srcIdx, 0) : CUV(0.f, 0.f);
				v.Skin = skinWeights[srcIdx];
			}
		}
		for (uint rp = 0; rp < mesh->getNbRdrPass(lod); ++rp)
		{
			sint32 matId = (sint32)mesh->getRdrPassMaterial(lod, rp);

			CIndexBuffer pb;
			mesh->getRdrPassPrimitiveBlock(lod, rp, pb);
			CIndexBufferRead iba;
			pb.lock(iba);
			uint numIdx = pb.getNumIndexes();
			if (pb.getFormat() == CIndexBuffer::Indices32)
			{
				const uint32 *idx = (const uint32 *)iba.getPtr();
				for (uint t = 0; t + 2 < numIdx; t += 3)
				{
					SFace f;
					f.V[0] = idx[t]; f.V[1] = idx[t + 1]; f.V[2] = idx[t + 2];
					f.MaterialId = matId;
					out.Faces.push_back(f);
				}
			}
			else
			{
				const uint16 *idx = (const uint16 *)iba.getPtr();
				for (uint t = 0; t + 2 < numIdx; t += 3)
				{
					SFace f;
					f.V[0] = idx[t]; f.V[1] = idx[t + 1]; f.V[2] = idx[t + 2];
					f.MaterialId = matId;
					out.Faces.push_back(f);
				}
			}
		}

		out.Materials.resize(mesh->getNbMaterial());
		for (uint m = 0; m < mesh->getNbMaterial(); ++m)
			out.Materials[m] = mesh->getMaterial(m);

		return true;
	}

	// Merges two vertices' (matrixId, weight) skin pairs, weighting each side by (1-t)/t,
	// keeping at most NL3D_MESH_SKINNING_MAX_MATRIX bones, renormalized to sum to 1. Used
	// for a newly created cut vertex interpolated between two original ones.
	CMesh::CSkinWeight lerpSkin(const CMesh::CSkinWeight &a, const CMesh::CSkinWeight &b, float t)
	{
		map<uint32, float> merged;
		for (uint i = 0; i < NL3D_MESH_SKINNING_MAX_MATRIX; ++i)
			if (a.Weights[i] > 0.f)
				merged[a.MatrixId[i]] += a.Weights[i] * (1.f - t);
		for (uint i = 0; i < NL3D_MESH_SKINNING_MAX_MATRIX; ++i)
			if (b.Weights[i] > 0.f)
				merged[b.MatrixId[i]] += b.Weights[i] * t;

		CMesh::CSkinWeight res;
		// keep the NL3D_MESH_SKINNING_MAX_MATRIX heaviest contributions
		vector<pair<float, uint32> > sorted;
		for (map<uint32, float>::const_iterator it = merged.begin(); it != merged.end(); ++it)
			sorted.push_back(make_pair(it->second, it->first));
		sort(sorted.rbegin(), sorted.rend());
		if (sorted.size() > NL3D_MESH_SKINNING_MAX_MATRIX)
			sorted.resize(NL3D_MESH_SKINNING_MAX_MATRIX);

		float total = 0.f;
		for (uint i = 0; i < sorted.size(); ++i)
			total += sorted[i].first;
		for (uint i = 0; i < NL3D_MESH_SKINNING_MAX_MATRIX; ++i)
		{
			if (i < sorted.size() && total > 0.f)
			{
				res.MatrixId[i] = sorted[i].second;
				res.Weights[i] = sorted[i].first / total;
			}
			else
			{
				res.MatrixId[i] = 0;
				res.Weights[i] = 0.f;
			}
		}
		return res;
	}

	// ---------------------------------------------------------------------
	// Clipping: cuts away, from the hairstyle geometry, whatever falls inside
	// the hat's volume, creating new vertices/edges exactly on the crossing
	// (found by bisection on the winding number field, see above). Assumes
	// the hat's surface crosses a given hair triangle at most once (a single
	// entering + exiting edge pair) -- true for the common case of a hat
	// silhouette cutting across an unfolded hairstyle triangle; a triangle
	// crossed by a more convoluted intersection would only be approximated.
	// ---------------------------------------------------------------------

	uint32 appendVertex(SMeshData &out, const SVertex &v)
	{
		out.Verts.push_back(v);
		return (uint32)out.Verts.size() - 1;
	}

	SVertex lerpVertex(const SVertex &a, const SVertex &b, float t)
	{
		SVertex v;
		v.Pos = a.Pos + (b.Pos - a.Pos) * t;
		v.Normal = a.Normal + (b.Normal - a.Normal) * t;
		v.Normal.normalize();
		v.Uv0 = CUV(a.Uv0.U + (b.Uv0.U - a.Uv0.U) * t, a.Uv0.V + (b.Uv0.V - a.Uv0.V) * t);
		v.Skin = lerpSkin(a.Skin, b.Skin, t);
		return v;
	}

	// hairWorldPos: same size/indexing as hair.Verts, the CURRENT-world-space skinned
	// position of each vertex (see skinToCurrentWorld) -- used only for the inside/outside
	// test and edge-crossing search, never for the output geometry (which stays in
	// hair.Verts' original bind-pose space, only interpolated by the found blend t).
	void clipHairstyleAgainstHat(const SMeshData &hair, const vector<CVector> &hairWorldPos,
		const vector<CVector> &hatVerts, const vector<uint32> &hatIndices, SMeshData &out)
	{
		out.Materials = hair.Materials;
		out.BonesNames = hair.BonesNames;
		out.Skinned = hair.Skinned;
		out.Verts = hair.Verts; // original vertices kept at their original indices; cut points appended after

		for (uint fi = 0; fi < hair.Faces.size(); ++fi)
		{
			const SFace &f = hair.Faces[fi];
			bool inside[3];
			for (uint k = 0; k < 3; ++k)
				inside[k] = isInsideHat(hairWorldPos[f.V[k]], hatVerts, hatIndices);

			// Walk the triangle's boundary in its original order, keeping outside corners
			// and inserting a new vertex wherever the edge crosses the hat surface. This
			// naturally preserves winding order (hence face normal / backface culling)
			// for every resulting sub-triangle, and degenerates correctly to "keep as is"
			// (all outside) or "drop" (all inside) without needing separate cases.
			vector<uint32> poly;
			for (uint i = 0; i < 3; ++i)
			{
				uint j = (i + 1) % 3;
				if (!inside[i])
					poly.push_back(f.V[i]);
				if (inside[i] != inside[j])
				{
					float t = bisectCrossing(hairWorldPos[f.V[i]], hairWorldPos[f.V[j]], inside[i], hatVerts, hatIndices);
					SVertex nv = lerpVertex(out.Verts[f.V[i]], out.Verts[f.V[j]], t);
					poly.push_back(appendVertex(out, nv));
				}
			}

			if (poly.size() == 3)
			{
				SFace nf; nf.V[0] = poly[0]; nf.V[1] = poly[1]; nf.V[2] = poly[2]; nf.MaterialId = f.MaterialId;
				out.Faces.push_back(nf);
			}
			else if (poly.size() == 4)
			{
				SFace nf0; nf0.V[0] = poly[0]; nf0.V[1] = poly[1]; nf0.V[2] = poly[2]; nf0.MaterialId = f.MaterialId;
				SFace nf1; nf1.V[0] = poly[0]; nf1.V[1] = poly[2]; nf1.V[2] = poly[3]; nf1.MaterialId = f.MaterialId;
				out.Faces.push_back(nf0);
				out.Faces.push_back(nf1);
			}
			// poly.size() == 0: fully inside the hat, dropped.
		}
	}

	// ---------------------------------------------------------------------
	// Rebuild a private CMesh from the clipped geometry, and instantiate it
	// directly (IShape::createInstance), bypassing CShapeBank entirely so it
	// is never shared with any other character.
	// ---------------------------------------------------------------------

	CMesh *buildMesh(const SMeshData &data)
	{
		CMesh::CMeshBuild mbuild;
		mbuild.VertexFlags = CVertexBuffer::NormalFlag | CVertexBuffer::TexCoord0Flag;
		if (data.Skinned)
			mbuild.VertexFlags |= CVertexBuffer::PaletteSkinFlag;
		mbuild.NumCoords[0] = 2;
		mbuild.Vertices = vector<CVector>(data.Verts.size());
		mbuild.BonesNames = data.BonesNames;
		if (data.Skinned)
			mbuild.SkinWeights.resize(data.Verts.size());

		for (uint i = 0; i < data.Verts.size(); ++i)
		{
			mbuild.Vertices[i] = data.Verts[i].Pos;
			if (data.Skinned)
				mbuild.SkinWeights[i] = data.Verts[i].Skin;
		}

		mbuild.Faces.resize(data.Faces.size());
		for (uint i = 0; i < data.Faces.size(); ++i)
		{
			CMesh::CFace &face = mbuild.Faces[i];
			face.MaterialId = data.Faces[i].MaterialId;
			for (uint k = 0; k < 3; ++k)
			{
				const SVertex &v = data.Verts[data.Faces[i].V[k]];
				face.Corner[k].Vertex = data.Faces[i].V[k];
				face.Corner[k].Normal = v.Normal;
				face.Corner[k].Uvws[0] = CUVW(v.Uv0.U, v.Uv0.V, 0.f);
			}
		}

		CMeshBase::CMeshBaseBuild mbase;
		mbase.Materials = data.Materials;
		mbase.DefaultPos = CVector(0.f, 0.f, 0.f);
		mbase.DefaultPivot = CVector(0.f, 0.f, 0.f);
		mbase.DefaultRotEuler = CVector(0.f, 0.f, 0.f);
		mbase.DefaultRotQuat = CQuat::Identity;
		mbase.DefaultScale = CVector(1.f, 1.f, 1.f);

		CMesh *mesh = new CMesh;
		mesh->build(mbase, mbuild);
		return mesh;
	}

	// Dispatches to the right extraction path for whichever concrete shape type was
	// loaded -- hairstyles are CMeshMRMSkinned, hats are plain CMesh (stuck rigidly,
	// no LOD/skinning needed for a simple item), but this stays generic either way.
	bool extractHairMeshData(IShape *shape, SMeshData &out)
	{
		if (CMesh *mesh = dynamic_cast<CMesh *>(shape))
			return extractMesh(mesh, out);
		if (CMeshMRMSkinned *mrm = dynamic_cast<CMeshMRMSkinned *>(shape))
			return extractMeshMRMSkinned(mrm, out);
		nlwarning("extractHairMeshData: unsupported hairstyle shape type (not CMesh or CMeshMRMSkinned)");
		return false;
	}

	// Only positions/triangles are needed for the hat (it's only ever used as the
	// clipping volume, never rebuilt), so this stays independent of full attribute/skin
	// extraction and supports the same two shape types.
	bool extractPositionsAndTriangles(IShape *shape, vector<CVector> &verts, vector<uint32> &indices)
	{
		if (CMesh *mesh = dynamic_cast<CMesh *>(shape))
			return mesh->getMeshGeom().retrieveVertices(verts) && mesh->getMeshGeom().retrieveTriangles(indices);
		if (CMeshMRMSkinned *mrm = dynamic_cast<CMeshMRMSkinned *>(shape))
		{
			SMeshData tmp;
			if (!extractMeshMRMSkinned(mrm, tmp))
				return false;
			verts.resize(tmp.Verts.size());
			for (uint i = 0; i < tmp.Verts.size(); ++i)
				verts[i] = tmp.Verts[i].Pos;
			indices.resize(tmp.Faces.size() * 3);
			for (uint i = 0; i < tmp.Faces.size(); ++i)
			{
				indices[i * 3] = tmp.Faces[i].V[0];
				indices[i * 3 + 1] = tmp.Faces[i].V[1];
				indices[i * 3 + 2] = tmp.Faces[i].V[2];
			}
			return true;
		}
		nlwarning("extractPositionsAndTriangles: unsupported hat shape type (not CMesh or CMeshMRMSkinned)");
		return false;
	}
}

// ---------------------------------------------------------------------
// Public entry point.
// ---------------------------------------------------------------------

NL3D::UInstance buildClippedHairstyleInstance(NL3D::CScene &scene, NL3D::USkeleton &skeleton,
	const std::string &hairstyleShapeName, const std::string &hatShapeName)
{
	IShape *hairSrc = loadPrivateShape(hairstyleShapeName);

	string maskShapeName = deriveMaskShapeName(hatShapeName);
	IShape *hatSrc = loadPrivateShape(maskShapeName);
	if (!hatSrc)
		hatSrc = loadPrivateShape(hatShapeName);

	if (!hairSrc || !hatSrc)
	{
		nlwarning("buildClippedHairstyleInstance: private load failed for '%s' or '%s'",
			hairstyleShapeName.c_str(), hatShapeName.c_str());
		delete hairSrc;
		delete hatSrc;
		return UInstance();
	}

	SMeshData hairData;
	vector<CVector> hatVerts;
	vector<uint32> hatIndices;
	bool ok = extractHairMeshData(hairSrc, hairData) && extractPositionsAndTriangles(hatSrc, hatVerts, hatIndices);
	delete hatSrc;

	// Bring hat AND hairstyle into the skeleton's CURRENT world space -- see this file's
	// "Coordinate spaces" comment above skinToCurrentWorld for why (real bone world
	// matrices, not a hand-derived bind-pose transform).
	vector<CVector> hairWorldPos;
	if (ok)
	{
		CMatrix headWorld;
		bool haveHeadWorld = boneWorldMatrix(skeleton, "Bip01 Head", headWorld);
		if (haveHeadWorld)
		{
			for (uint i = 0; i < hatVerts.size(); ++i)
				hatVerts[i] = headWorld * hatVerts[i];
		}
		else
		{
			nlwarning("buildClippedHairstyleInstance: could not resolve 'Bip01 Head' world matrix, hat left in its own local space (will misclassify)");
		}

		CSkeletonModel *skelModel = skeleton.getObjectPtr();
		vector<sint> localToSkeletonBoneId = resolveSkeletonBoneIds(skeleton, hairData.BonesNames);
		for (uint i = 0; i < localToSkeletonBoneId.size(); ++i)
			if (localToSkeletonBoneId[i] >= 0)
				skeleton.forceComputeBone((uint)localToSkeletonBoneId[i]);
		hairWorldPos.resize(hairData.Verts.size());
		for (uint i = 0; i < hairData.Verts.size(); ++i)
			hairWorldPos[i] = skelModel ? skinToCurrentWorld(hairData.Verts[i], localToSkeletonBoneId, skelModel) : hairData.Verts[i].Pos;

		// Character world positions (e.g. ~10000+ on a large continent) blow the precision
		// budget of a 32-bit float right when it matters most: the winding-number math below
		// needs sub-centimeter precision on DIFFERENCES between vertex positions, but at
		// ~10000 magnitude a float only has ~2-3 significant fractional digits left. Recenter
		// everything on the head bone's own world position first -- the classification only
		// ever needs relative geometry, never true world coordinates.
		if (haveHeadWorld)
		{
			CVector origin = headWorld.getPos();
			for (uint i = 0; i < hatVerts.size(); ++i)
				hatVerts[i] -= origin;
			for (uint i = 0; i < hairWorldPos.size(); ++i)
				hairWorldPos[i] -= origin;
		}
	}

	if (!ok)
	{
		nlwarning("buildClippedHairstyleInstance: geometry extraction failed for '%s' / '%s'",
			hairstyleShapeName.c_str(), hatShapeName.c_str());
		delete hairSrc;
		return UInstance();
	}
	delete hairSrc;

	SMeshData clipped;
	clipHairstyleAgainstHat(hairData, hairWorldPos, hatVerts, hatIndices, clipped);

	CMesh *privateMesh = buildMesh(clipped);
	CTransformShape *transform = privateMesh->createInstance(scene);
	if (!transform)
	{
		delete privateMesh;
		return UInstance();
	}

	UInstance instance(transform);
	if (!skeleton.bindSkin(instance))
		nlwarning("buildClippedHairstyleInstance: bindSkin failed for clipped '%s'", hairstyleShapeName.c_str());

	return instance;
}
