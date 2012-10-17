// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_MRM_BUILDER_H
#define NL_MRM_BUILDER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/uv.h"
#include "nel/3d/mrm_mesh.h"
#include "nel/3d/mrm_internal.h"
#include "nel/3d/mrm_parameters.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include <map>


namespace NL3D
{



// ***************************************************************************
/**
 * The class for building MRMs.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMRMBuilder
{
public:

	/// Constructor
	CMRMBuilder();



	/** Compile a MRM mesh info.
	 *	\param mbuild the input mesh
	 *	\param params the parameters of MRM process.
	 *	\param mrmMesh the result MRM mesh.
	 */
	void	compileMRM( const CMesh::CMeshBuild &mbuild, std::vector<CMesh::CMeshBuild*> &bsList,
						const CMRMParameters &params, CMeshMRMGeom::CMeshBuildMRM &mrmMesh,
						uint numMaxMaterial);



	/** Compile a MRM skinned mesh info.
	 *	\param mbuild the input mesh
	 *	\param params the parameters of MRM process.
	 *	\param mrmMesh the result MRM mesh.
	 */
	void	compileMRM( const CMesh::CMeshBuild &mbuild, std::vector<CMesh::CMeshBuild*> &bsList,
						const CMRMParameters &params, CMeshMRMSkinnedGeom::CMeshBuildMRM &mrmMesh,
						uint numMaxMaterial);



// ****************************
private:
// Mesh Level Part.

	/// \name Mesh Level Tmp Values.
	// @{
	// The vertices of the MRMMesh.
	std::vector<CMRMVertex>		TmpVertices;
	// The attributes of the MRMMesh.
	std::vector<CMRMAttribute>	TmpAttributes[NL3D_MRM_MAX_ATTRIB];
	// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
	// The faces of the MRMMesh.
	std::vector<CMRMFaceBuild>	TmpFaces;
	// Ordered list of Edge collapse.
	TEdgeMap					EdgeCollapses;

	// Say if the current build must compute skinning information.
	bool						_Skinned;
	/// If the current build is skinned, control the quality of the skinning redcution.
	CMRMParameters::TSkinReduction	_SkinReduction;

	// @}


	/// \name Edge Cost methods.
	// @{
	bool	vertexHasOneWedge(sint numvertex);
	bool	vertexHasOneMaterial(sint numvertex);
	bool	vertexContinue(sint numvertex);
	bool	vertexClosed(sint numvertex);
	float	getDeltaFaceNormals(sint numvertex);	// return a positive value of Sum(|DeltaNormals|) / NNormals.
	bool	edgeContinue(const CMRMEdge &edge);
	bool	edgeNearUniqueMatFace(const CMRMEdge &edge);
	float	computeEdgeCost(const CMRMEdge &edge);
	// @}


	/// \name Collapse methods.
	// @{
	bool	faceShareWedges(CMRMFaceBuild *face, sint attribId, sint numVertex1, sint numVertex2);
	void	insertFaceIntoEdgeList(CMRMFaceBuild &tmpf);
	void	removeFaceFromEdgeList(CMRMFaceBuild &f);
	sint	collapseEdge(const CMRMEdge &edge);	// return num of deleted faces.
	sint	followVertex(sint i);
	sint	followWedge(sint attribId, sint i);
	CMesh::CSkinWeight	collapseSkinWeight(const CMesh::CSkinWeight &sw1, const CMesh::CSkinWeight &sw2, float InterValue) const;
	// @}


	/// \name Mesh Level methods.
	// @{
	void	init(const CMRMMesh &baseMesh);
	void	collapseEdges(sint nWantedFaces);
	void	makeLODMesh(CMRMMeshGeom &lodMesh);
	void	saveCoarserMesh(CMRMMesh &coarserMesh);
	/// this is the root call to compute a single lodMesh and the coarserMesh from a baseMesh.
	void	makeFromMesh(const CMRMMesh &baseMesh, CMRMMeshGeom &lodMesh, CMRMMesh &coarserMesh, sint nWantedFaces);
	// @}


	void	computeBsVerticesAttributes(std::vector<CMRMMesh> &srcBsMeshs, std::vector<CMRMMesh> &srcBsMeshsMod);
	void	makeCoarserBS (std::vector<CMRMBlendShape> &csBsMeshs);


	/// \name Mesh Interfaces computing
	// @{
	bool						_HasMeshInterfaces;
	// the sewing meshes
	std::vector<CMRMSewingMesh>	_SewingMeshes;
	// The current Lod
	uint						_CurrentLodComputed;

	// true if the build has some Mesh sewing interface setup.
	bool	buildMRMSewingMeshes(const CMesh::CMeshBuild &mbuild, uint nWantedLods, uint divisor);

	// @}


private:
// MRM Level Part.

	/// \name MRM Level Variables.
	// @{
	// Our comparator of CMRMWedgeGeom.
	struct	CGeomPred
	{
		bool	operator()(const CMRMWedgeGeom &a, const CMRMWedgeGeom &b) const
		{
			if(a.Start!=b.Start)
				return a.Start<b.Start;
			return a.End<b.End;
		}
	};

	// The map of geomorph. for one LOD only;
	typedef std::map<CMRMWedgeGeom, sint, CGeomPred>		TGeomMap;
	TGeomMap			_GeomMap;
	// @}


	/// \name MRM Level Methods.
	// @{
	/** build the blend shapes in the same way we constructed the base mesh mrm
	 */
	void buildBlendShapes (CMRMMesh &baseMesh, std::vector<CMesh::CMeshBuild*> &bsList, uint32 VertexFlags);

	/** build all LODs from a baseMesh. NB: the coarsestMesh is stored in lodMeshs[0], and has no geomorph info since it is
	 * the coarsest mesh. nWantedLods are created (including the coarsestMesh).
	 * \param lodMeshs array created by the function (size of nWantedlods).
	 * \param nWantedLods number of LODs wanted.
	 * \param divisor the coarsestMesh will have  baseMesh.Faces.size()/divisor  faces.
	 */
	void	buildAllLods(	const CMRMMesh &baseMesh, std::vector<CMRMMeshGeom> &lodMeshs,
							uint nWantedLods= 10, uint divisor= 50 );

	/** given a list of LODs, compress/reorganize data, and store in finalMRM mesh.
	 *
	 */
	void	buildFinalMRM(std::vector<CMRMMeshGeom> &lodMeshs, CMRMMeshFinal &finalMRM);

	// @}



private:
// Interface to MeshBuild Part.

	/// \name Top Level methods.
	// @{


	/// Temp map Attribute/AttributeId .
	struct	CAttributeKey
	{
		sint		VertexId;
		CVectorH	Attribute;

		bool	operator<(const CAttributeKey &o) const
		{
			if(VertexId!=o.VertexId)
				return VertexId<o.VertexId;
			return Attribute<o.Attribute;
		}
	};
	typedef std::map<CAttributeKey, sint>		TAttributeMap;
	TAttributeMap		_AttributeMap[NL3D_MRM_MAX_ATTRIB];
	sint			findInsertAttributeInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, const CVectorH &att);
	sint			findInsertNormalInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, const CVector &normal);
	sint			findInsertColorInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, CRGBA col);
	sint			findInsertUvwInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, const NLMISC::CUVW &uvw);
	CRGBA			attToColor(const CVectorH &att) const;
	NLMISC::CUVW	attToUvw(const CVectorH &att) const;


	/** from a meshBuild, compute a CMRMMesh. This is the first stage of the algo.
	 *	\return the vertexFormat supported by CMRMBuilder.
	 */
	uint32			buildMrmBaseMesh(const CMesh::CMeshBuild &mbuild, CMRMMesh &baseMesh);

	/** from a final MRM Mesh representation, compute a CMeshBuildMRM. This is the last stage of the algo.
	 *	\param vbFlags the vertex format returned by earlier call too buildMrmBaseMesh().
	 *	\param nbMats the number of materials of original MeshBuild.
	 */
	void			buildMeshBuildMrm(const CMRMMeshFinal &finalMRM, CMeshMRMGeom::CMeshBuildMRM &mbuild, uint32 vbFlags, uint32 nbMats, const CMesh::CMeshBuild &mb);

	/** from a final MRM Mesh representation, compute a CMeshBuildMRM. This is the last stage of the algo.
	 *	\param vbFlags the vertex format returned by earlier call too buildMrmBaseMesh().
	 *	\param nbMats the number of materials of original MeshBuild.
	 */
	void			buildMeshBuildMrm(const CMRMMeshFinal &finalMRM, CMeshMRMSkinnedGeom::CMeshBuildMRM &mbuild, uint32 vbFlags, uint32 nbMats, const CMesh::CMeshBuild &mb);


	void			normalizeBaseMeshSkin(CMRMMesh &baseMesh) const;
	CMesh::CSkinWeight	normalizeSkinWeight(const CMesh::CSkinWeight &sw) const;

	// @}

};


} // NL3D


#endif // NL_MRM_BUILDER_H

/* End of mrm_builder.h */
