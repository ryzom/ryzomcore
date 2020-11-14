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

#include "std3d.h"

#include "nel/3d/instance_lighter.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/3d/visual_collision_manager.h"
#include "nel/3d/visual_collision_entity.h"
#include "nel/3d/ig_surface_light_build.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


// Bad coded: don't set too big else it allocates too much memory.
#define	NL3D_INSTANCE_LIGHTER_CUBE_GRID_SIZE	16


// ***************************************************************************
// ***************************************************************************
// Setup part
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CInstanceLighter::CLightDesc::CLightDesc ()
{
	LightDirection.set (1, 1, -1);
	GridSize=512;
	GridCellSize=4;
	Shadow= true;
	OverSampling= 0;
	DisableSunContribution= false;
}

// ***************************************************************************
CInstanceLighter::CInstanceLighter()
{
	_IGSurfaceLightBuild= NULL;
}

// ***************************************************************************
void CInstanceLighter::init ()
{
}

// ***************************************************************************
void CInstanceLighter::addTriangles (CLandscape &landscape, std::vector<uint> &listZone, uint order, std::vector<CTriangle>& triangleArray)
{
	// Lamed from CZoneLighter.
	// Set all to refine
	excludeAllPatchFromRefineAll (landscape, listZone, false);

	// Setup the landscape
	landscape.setThreshold (0);
	landscape.setTileMaxSubdivision (order);

	// Refine it
	landscape.refineAll (CVector (0, 0, 0));

	// Dump tesselated triangles
	std::vector<const CTessFace*> leaves;
	landscape.getTessellationLeaves(leaves);

	// Number of leaves
	uint leavesCount=(uint)leaves.size();

	// Reserve the array
	triangleArray.reserve (triangleArray.size()+leavesCount);

	// Scan each leaves
	for (uint leave=0; leave<leavesCount; leave++)
	{
		// Leave
		const CTessFace *face=leaves[leave];

		// Add a triangle. -1 because not an instance from an IG
		triangleArray.push_back (CTriangle (NLMISC::CTriangle (face->VBase->EndPos, face->VLeft->EndPos, face->VRight->EndPos), -1 ));
	}

	// Setup the landscape
	landscape.setThreshold (1000);
	landscape.setTileMaxSubdivision (0);

	// Remove all triangles
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));

}

// ***************************************************************************
void CInstanceLighter::addTriangles (const IShape &shape, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray, sint instanceId)
{
	// Lamed from CZoneLighter.

	// Cast to CMesh
	const CMesh *mesh=dynamic_cast<const CMesh*>(&shape);

	// Cast to CMeshMultiLod
	const CMeshMultiLod *meshMulti=dynamic_cast<const CMeshMultiLod*>(&shape);

	// Cast to CMeshMultiLod
	const CMeshMRM *meshMRM=dynamic_cast<const CMeshMRM*>(&shape);

	// It is a mesh ?
	if (mesh)
	{
		// Add its triangles
		addTriangles (mesh->getMeshGeom (), modelMT, triangleArray, instanceId);
	}
	// It is a CMeshMultiLod ?
	else if (meshMulti)
	{
		// Get the first geommesh
		const IMeshGeom *meshGeom=&meshMulti->getMeshGeom (0);

		// Dynamic cast
		const CMeshGeom *geomMesh=dynamic_cast<const CMeshGeom*>(meshGeom);
		if (geomMesh)
		{
			addTriangles (*geomMesh, modelMT, triangleArray, instanceId);
		}

		// Dynamic cast
		const CMeshMRMGeom *mrmGeomMesh=dynamic_cast<const CMeshMRMGeom*>(meshGeom);
		if (mrmGeomMesh)
		{
			addTriangles (*mrmGeomMesh, modelMT, triangleArray, instanceId);
		}
	}
	// It is a CMeshMultiLod ?
	else if (meshMRM)
	{
		// Get the first lod mesh geom
		addTriangles (meshMRM->getMeshGeom (), modelMT, triangleArray, instanceId);
	}
}


// ***************************************************************************

void CInstanceLighter::addTriangles (const CMeshGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray, sint instanceId)
{
	// Get the vertex buffer
	const CVertexBuffer &vb=meshGeom.getVertexBuffer();
	CVertexBufferRead vba;
	vb.lock (vba);

	// For each matrix block
	uint numBlock=meshGeom.getNbMatrixBlock();
	for (uint block=0; block<numBlock; block++)
	{
		// For each render pass
		uint numRenderPass=meshGeom.getNbRdrPass(block);
		for (uint pass=0; pass<numRenderPass; pass++)
		{
			// Get the primitive block
			const CIndexBuffer &primitive=meshGeom.getRdrPassPrimitiveBlock ( block, pass);

			// Dump triangles
			CIndexBufferRead iba;
			primitive.lock (iba);
			uint numTri=primitive.getNumIndexes ()/3;
			uint tri;
			if (primitive.getFormat() == CIndexBuffer::Indices16)
			{
				const uint16* triIndex=(uint16*)iba.getPtr ();
				for (tri=0; tri<numTri; tri++)
				{
					// Vertex
					CVector v0=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3]));
					CVector v1=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+1]));
					CVector v2=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+2]));

					// Make a triangle
					triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2), instanceId));
				}
			}
			else
			{
				const uint32* triIndex=(uint32*)iba.getPtr ();
				for (tri=0; tri<numTri; tri++)
				{
					// Vertex
					CVector v0=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3]));
					CVector v1=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+1]));
					CVector v2=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+2]));

					// Make a triangle
					triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2), instanceId));
				}
			}
		}
	}
}

// ***************************************************************************

void CInstanceLighter::addTriangles (const CMeshMRMGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray, sint instanceId)
{
	// Get the vertex buffer
	const CVertexBuffer &vb=meshGeom.getVertexBuffer();
	CVertexBufferRead vba;
	vb.lock (vba);

	// For each render pass
	uint numRenderPass=meshGeom.getNbRdrPass(0);
	for (uint pass=0; pass<numRenderPass; pass++)
	{
		// Get the primitive block
		const CIndexBuffer &primitive=meshGeom.getRdrPassPrimitiveBlock ( 0, pass);

		// Dump triangles
		CIndexBufferRead iba;
		primitive.lock (iba);
		uint numTri=primitive.getNumIndexes ()/3;
		uint tri;
		if (primitive.getFormat() == CIndexBuffer::Indices16)
		{
			const uint16* triIndex=(uint16*)iba.getPtr ();
			for (tri=0; tri<numTri; tri++)
			{
				// Vertex
				CVector v0=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3]));
				CVector v1=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+1]));
				CVector v2=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+2]));

				// Make a triangle
				triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2), instanceId));
			}
		}
		else
		{
			const uint32* triIndex=(uint32*)iba.getPtr ();
			for (tri=0; tri<numTri; tri++)
			{
				// Vertex
				CVector v0=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3]));
				CVector v1=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+1]));
				CVector v2=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+2]));

				// Make a triangle
				triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2), instanceId));
			}
		}
	}
}

// ***************************************************************************

void CInstanceLighter::excludeAllPatchFromRefineAll (CLandscape &landscape, vector<uint> &listZone, bool exclude)
{
	// For each zone
	for (uint zone=0; zone<listZone.size(); zone++)
	{
		// Get num patches
		uint patchCount=landscape.getZone(listZone[zone])->getNumPatchs();

		// For each patches
		for (uint patch=0; patch<patchCount; patch++)
		{
			// Exclude all the patches from refine all
			landscape.excludePatchFromRefineAll (listZone[zone], patch, exclude);
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// light part
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void CInstanceLighter::light (const CInstanceGroup &igIn, CInstanceGroup &igOut, const CLightDesc &lightDesc,
	std::vector<CTriangle>& obstacles, CLandscape *landscape, CIGSurfaceLightBuild *igSurfaceLightBuild)
{
	sint					i;
	CVector					outGlobalPos;
	std::vector<CCluster>	outClusters;
	std::vector<CPortal>	outPortals;
	std::vector<CPointLightNamed>	pointLightList;

	nlassert(lightDesc.OverSampling==0 || lightDesc.OverSampling==2 || lightDesc.OverSampling==4
		|| lightDesc.OverSampling==8 || lightDesc.OverSampling==16);

	// Setup.
	//========

	// Prepare IGSurfaceLight lighting
	//-----------
	// Bkup SurfaceLightBuild to know if must light the surfaces, in differents part of the process.
	_IGSurfaceLightBuild= igSurfaceLightBuild;
	// Prepare _IGRetrieverGridMap.
	_IGRetrieverGridMap.clear();
	if(_IGSurfaceLightBuild)
	{
		_TotalCellNumber= 0;
		CIGSurfaceLightBuild::ItRetrieverGridMap	itSrc;
		itSrc= _IGSurfaceLightBuild->RetrieverGridMap.begin();
		// For all retrievers Infos in _IGSurfaceLightBuild
		while(itSrc!=_IGSurfaceLightBuild->RetrieverGridMap.end())
		{
			uint	numSurfaces= (uint)itSrc->second.Grids.size();
			// If !empty retriever.
			if(numSurfaces>0)
			{
				// Add it to the map,
				CIGSurfaceLight::CRetrieverLightGrid		&rlgDst= _IGRetrieverGridMap[itSrc->first];
				// resize Array of surfaces.
				rlgDst.Grids.resize(numSurfaces);
				// For all surfaces, init them in rlgDst.
				for(uint i=0; i<numSurfaces; i++)
				{
					CIGSurfaceLightBuild::CSurface		&surfSrc= itSrc->second.Grids[i];
					CSurfaceLightGrid					&surfDst= rlgDst.Grids[i];
					// Init Cells with a default CellCorner
					CSurfaceLightGrid::CCellCorner		defaultCellCorner;
					defaultCellCorner.SunContribution= 0;
					defaultCellCorner.Light[0]= 0xFF;
					defaultCellCorner.Light[1]= 0xFF;
					defaultCellCorner.LocalAmbientId= 0xFF;

					// Init the grid.
					surfDst.Origin= surfSrc.Origin;
					surfDst.Width= surfSrc.Width;
					surfDst.Height= surfSrc.Height;
					surfDst.Cells.resize((uint32)surfSrc.Cells.size());
					surfDst.Cells.fill(defaultCellCorner);
					// The grid must be valid an not empty
					nlassert( surfDst.Cells.size() == surfDst.Width*surfDst.Height );
					nlassert( surfDst.Width>= 2 );
					nlassert( surfDst.Height>= 2 );

					_TotalCellNumber+= surfDst.Cells.size();
				}
			}

			// Next localRetriever info.
			itSrc++;
		}
	}
	// Reset cell iteration.
	_IsEndCell= true;


	// Retrieve info from igIn.
	//-----------
	igIn.retrieve (outGlobalPos, _Instances, outClusters, outPortals, pointLightList);


	// set All Instances StaticLightEnabled= true, and Build _InstanceInfos.
	//-----------
	// Map of shape
	std::map<string, IShape*> shapeMap;
	_InstanceInfos.resize(_Instances.size());
	for(i=0; i<(sint)_Instances.size();i++)
	{
		// Avoid StaticLight precomputing??
		if(_Instances[i].AvoidStaticLightPreCompute)
		{
			_Instances[i].StaticLightEnabled= false;
			// Next instance.
			continue;
		}

		// Else let's do it.
		_Instances[i].StaticLightEnabled= true;


		// Get the shape centerPos;
		//------------
		CVector	shapeCenterPos;
		CVector	overSamples[MaxOverSamples];

		// Get the instance shape name
		string name= _Instances[i].Name;
		bool	shapeFound= true;

		if (toLower (CFile::getExtension (name)) == "pacs_prim")
		{
			nlwarning("EXPORT BUG: Can't read %s (not a shape), should not be part of .ig!", name.c_str());
			continue;
		}

		// Try to find the shape in the UseShapeMap.
		std::map<string, IShape*>::const_iterator iteMap= lightDesc.UserShapeMap.find (name);

		// If not found in userShape map, try to load it from the temp loaded ShapeBank.
		if( iteMap == lightDesc.UserShapeMap.end() )
		{
			// Add a .shape at the end ?
			if (name.find('.') == std::string::npos)
				name += ".shape";

			// Get the instance shape name
			string nameLookup = CPath::lookup (name, false, false);
			if (!nameLookup.empty())
				name = nameLookup;

			// Find the shape in the bank
			iteMap= shapeMap.find (name);
			if (iteMap==shapeMap.end())
			{
				// Input file
				CIFile inputFile;

				if (!name.empty() && inputFile.open (name))
				{
					// Load it
					CShapeStream stream;
					stream.serial (inputFile);

					// Get the pointer
					iteMap=shapeMap.insert (std::map<string, IShape*>::value_type (name, stream.getShapePointer ())).first;
				}
				else
				{
					// Error
					nlwarning ("WARNING can't load shape %s\n", name.c_str());
					shapeFound= false;
				}
			}
		}


		// Last chance to skip it: fully LightMapped ??
		//-----------
		if(shapeFound)
		{
			CMeshBase	*mesh= dynamic_cast<CMeshBase*>(iteMap->second);
			if(mesh)
			{
				// If this mesh is not lightable (fully lightMapped)
				if(!mesh->isLightable())
				{
					// Force Avoid StaticLight precomputing
					_Instances[i].AvoidStaticLightPreCompute= true;
					// Disable static lighting.
					_Instances[i].StaticLightEnabled= false;
					// Next instance.
					continue;
				}
			}
		}


		// Compute pos and OverSamples
		//-----------
		{
			// Compute bbox, or default bbox
			CAABBox		bbox;
			if(!shapeFound)
			{
				bbox.setCenter(CVector::Null);
				bbox.setHalfSize(CVector::Null);
			}
			else
			{
				iteMap->second->getAABBox(bbox);
			}
			// get pos
			shapeCenterPos= bbox.getCenter();


			// Compute overSamples
			float	qx= bbox.getHalfSize().x/2;
			float	qy= bbox.getHalfSize().y/2;
			float	qz= bbox.getHalfSize().z/2;
			// No OverSampling => just copy.
			if(lightDesc.OverSampling==0)
				overSamples[0]= shapeCenterPos;
			else if(lightDesc.OverSampling==2)
			{
				// Prefer Z Axis.
				overSamples[0]= shapeCenterPos + CVector(0, 0, qz);
				overSamples[1]= shapeCenterPos - CVector(0, 0, qz);
			}
			else if(lightDesc.OverSampling==4)
			{
				// Apply an overSampling such that we see 4 points if we look on each side of the bbox.
				overSamples[0]= shapeCenterPos + CVector(-qx, -qy, -qz);
				overSamples[1]= shapeCenterPos + CVector(+qx, -qy, +qz);
				overSamples[2]= shapeCenterPos + CVector(-qx, +qy, +qz);
				overSamples[3]= shapeCenterPos + CVector(+qx, +qy, -qz);
			}
			else if(lightDesc.OverSampling==8 || lightDesc.OverSampling==16)
			{
				// 8x is the best overSampling shceme for bbox
				overSamples[0]= shapeCenterPos + CVector(-qx, -qy, -qz);
				overSamples[1]= shapeCenterPos + CVector(+qx, -qy, -qz);
				overSamples[2]= shapeCenterPos + CVector(-qx, +qy, -qz);
				overSamples[3]= shapeCenterPos + CVector(+qx, +qy, -qz);
				overSamples[4]= shapeCenterPos + CVector(-qx, -qy, +qz);
				overSamples[5]= shapeCenterPos + CVector(+qx, -qy, +qz);
				overSamples[6]= shapeCenterPos + CVector(-qx, +qy, +qz);
				overSamples[7]= shapeCenterPos + CVector(+qx, +qy, +qz);

				// 16x => use this setup, and decal from 1/8
				if(lightDesc.OverSampling==16)
				{
					CVector		decal(qx/2, qy/2, qz/2);
					for(uint sample=0; sample<8; sample++)
					{
						// Copy and decal
						overSamples[sample+8]= overSamples[sample] + decal;
						// neg decal me
						overSamples[sample]-= decal;
					}
				}
			}
		}


		// Compute pos of the instance
		//------------
		CMatrix		matInst;
		matInst.setPos(_Instances[i].Pos);
		matInst.setRot(_Instances[i].Rot);
		matInst.scale(_Instances[i].Scale);
		_InstanceInfos[i].CenterPos= matInst * shapeCenterPos;
		// Apply matInst to samples.
		uint	nSamples= max(1U, lightDesc.OverSampling);
		for(uint sample=0; sample<nSamples; sample++)
		{
			_InstanceInfos[i].OverSamples[sample]= matInst * overSamples[sample];
		}
	}

	// Clean Up shapes.
	//-----------
	std::map<string, IShape*>::iterator iteMap;
	iteMap= shapeMap.begin();
	while(iteMap!= shapeMap.end())
	{
		// delte shape
		delete	iteMap->second;
		// delete entry in map
		shapeMap.erase(iteMap);
		// next
		iteMap= shapeMap.begin();
	}

	// Build all obstacles plane.
	//-----------
	for(i=0; i<(sint)obstacles.size();i++)
	{
		CInstanceLighter::CTriangle& triangle=obstacles[i];
		// Calc the plane
		triangle.Plane.make (triangle.Triangle.V0, triangle.Triangle.V1, triangle.Triangle.V2);
	}


	// Lighting
	//========
	// Light With Sun: build the grid, and do it on all _Instances, using _InstanceInfos
	// Compute also Lighting on surface.
	computeSunContribution(lightDesc, obstacles, landscape);

	// Light With PointLights
	// build the cubeGrids
	compilePointLightRT(lightDesc.GridSize, lightDesc.GridCellSize, obstacles, lightDesc.Shadow);
	// kill pointLightList, because will use mine.
	pointLightList.clear();
	// Light for all _Instances, using _InstanceInfos
	// Compute also Lighting on surface.
	processIGPointLightRT(pointLightList);

	// If _IGSurfaceLightBuild, then dilate lighting
	if(_IGSurfaceLightBuild)
	{
		dilateLightingOnSurfaceCells();
	}


	// Build result.
	//========
	if(_IGSurfaceLightBuild)
	{
		// build with IGSurfaceLight lighting
		igOut.build(outGlobalPos, _Instances, outClusters, outPortals, pointLightList,
			&_IGRetrieverGridMap, _IGSurfaceLightBuild->CellSize);
	}
	else
	{
		// build without IGSurfaceLight lighting
		igOut.build(outGlobalPos, _Instances, outClusters, outPortals, pointLightList);
	}

}


// ***************************************************************************
static void NEL3DCalcBase (CVector &direction, CMatrix& matrix)
{
	direction.normalize();
	CVector		I=(fabs(direction*CVector(1.f,0,0))>0.99)?CVector(0.f,1.f,0.f):CVector(1.f,0.f,0.f);
	CVector		K=-direction;
	CVector		J=K^I;
	J.normalize();
	I=J^K;
	I.normalize();
	matrix.identity();
	matrix.setRot(I,J,K, true);
}



// ***************************************************************************
void	CInstanceLighter::computeSunContribution(const CLightDesc &lightDesc, std::vector<CTriangle>& obstacles, CLandscape *landscape)
{
	sint	i;
	// Use precoputed landscape SunContribution
	CVisualCollisionManager		*VCM= NULL;
	CVisualCollisionEntity		*VCE= NULL;
	if(landscape)
	{
		// create a CVisualCollisionManager and a CVisualCollisionEntity
		VCM= new CVisualCollisionManager;
		VCM->setLandscape(landscape);
		VCE= VCM->createEntity();
	}
	std::vector<CPointLightInfluence>	dummyPointLightFromLandscape;
	dummyPointLightFromLandscape.reserve(1024);


	// If DisableSunContribution, easy,
	if(lightDesc.DisableSunContribution)
	{
		// Light all instances.
		//==========
		for(i=0; i<(sint)_Instances.size(); i++)
		{
			// If staticLight not enabled, skip.
			if( !_Instances[i].StaticLightEnabled )
				continue;

			// fill SunContribution to 0
			_Instances[i].SunContribution= 0;
		}

		// Light SurfaceGrid Cells.
		//==========
		if(_IGSurfaceLightBuild)
		{
			// Begin cell iteration
			beginCell();
			// For all surface cell corners
			while( !isEndCell() )
			{
				// get the current cell and cellInfo iterated.
				CIGSurfaceLightBuild::CCellCorner	&cellInfo= getCurrentCellInfo();
				CSurfaceLightGrid::CCellCorner		&cell= getCurrentCell();

				// if the cell corner lies in the polygon surface.
				if(cellInfo.InSurface)
				{
					// fill SunContribution to 0
					cell.SunContribution= 0;
					// copy it to cellInfo
					cellInfo.SunContribution= cell.SunContribution;
				}

				// next cell
				nextCell();
			}
		}
	}
	// If no Raytrace Shadow, easy,
	else if(!lightDesc.Shadow)
	{
		// Light all instances.
		//==========
		for(i=0; i<(sint)_Instances.size(); i++)
		{
			progress ("Compute SunContribution on Instances", i / float(_Instances.size()) );

			// If staticLight not enabled, skip.
			if( !_Instances[i].StaticLightEnabled )
				continue;

			// by default, fill SunContribution to 255
			_Instances[i].SunContribution= 255;
			// Try to get landscape SunContribution (better)
			if(landscape)
			{
				CVector		pos= _InstanceInfos[i].CenterPos;
				uint8	landSunContribution;
				dummyPointLightFromLandscape.clear();
				// If find faces under me
				NLMISC::CRGBA	dummyAmbient;
				if(VCE->getStaticLightSetup(NLMISC::CRGBA::Black, pos, dummyPointLightFromLandscape, landSunContribution, dummyAmbient) )
				{
					_Instances[i].SunContribution= landSunContribution;
				}
			}
		}

		// Light SurfaceGrid Cells.
		//==========
		if(_IGSurfaceLightBuild)
		{
			// Begin cell iteration
			beginCell();
			// For all surface cell corners
			while( !isEndCell() )
			{
				progressCell("Compute SunContribution on Surfaces");

				// get the current cell and cellInfo iterated.
				CIGSurfaceLightBuild::CCellCorner	&cellInfo= getCurrentCellInfo();
				CSurfaceLightGrid::CCellCorner		&cell= getCurrentCell();

				// if the cell corner lies in the polygon surface.
				if(cellInfo.InSurface)
				{
					// Just init SunContribution to 255, since no shadowing.
					cell.SunContribution= 255;
					// copy it to cellInfo
					cellInfo.SunContribution= cell.SunContribution;
				}

				// next cell
				nextCell();
			}
		}
	}
	else
	{
		// Compute rayBasis
		CVector	rayDir= lightDesc.LightDirection;
		CMatrix	rayBasis;
		rayDir.normalize();
		NEL3DCalcBase(rayDir, rayBasis);
		CMatrix	invRayBasis;
		invRayBasis= rayBasis.inverted();

		// Build QuadGrid of obstacles.
		//=========
		// setup quadGrid
		CQuadGrid<const CTriangle*>		quadGrid;
		quadGrid.changeBase (invRayBasis);
		quadGrid.create(lightDesc.GridSize, lightDesc.GridCellSize);
		// Insert all obstacles in quadGrid
		for(i=0; i<(sint)obstacles.size(); i++)
		{
			CAABBox	triBBox;
			// Compute the bbox in rayBasis.
			triBBox.setCenter(invRayBasis * obstacles[i].Triangle.V0);
			triBBox.extend(invRayBasis * obstacles[i].Triangle.V1);
			triBBox.extend(invRayBasis * obstacles[i].Triangle.V2);
			// And set the coord in our world, because will be multiplied with invRayBasis in insert()
			quadGrid.insert(rayBasis * triBBox.getMin(), rayBasis * triBBox.getMax(), &obstacles[i]);
		}

		// For all instances, light them.
		//=========
		for(i=0; i<(sint)_Instances.size(); i++)
		{
			progress ("Compute SunContribution on Instances", i / float(_Instances.size()) );

			// If staticLight not enabled, skip.
			if( !_Instances[i].StaticLightEnabled )
				continue;

			// try to use landscape SunContribution.
			bool	landUsed= false;
			if(landscape)
			{
				CVector		pos= _InstanceInfos[i].CenterPos;
				uint8	landSunContribution;
				dummyPointLightFromLandscape.clear();
				// If find faces under me
				NLMISC::CRGBA	dummyAmbient;
				if(VCE->getStaticLightSetup(NLMISC::CRGBA::Black, pos, dummyPointLightFromLandscape, landSunContribution, dummyAmbient) )
				{
					_Instances[i].SunContribution= landSunContribution;
					landUsed= true;
				}
			}

			// If failed to use landscape SunContribution, rayTrace
			if(!landUsed)
			{
				// number of samples (1 if no overSampling)
				uint	nSamples= max(1U, lightDesc.OverSampling);

				// Default is full lighted.
				uint	sunAccum= 255*nSamples;

				// For all samples
				for(uint sample=0; sample<nSamples; sample++)
				{
					// pos to rayTrace against
					CVector		pos= _InstanceInfos[i].OverSamples[sample];

					// rayTrace from this pos.
					CVector		lightPos= pos-(rayDir*1000.f);
					// Select an element with the X axis as a 3d ray
					quadGrid.select (lightPos, lightPos);
					// For each triangle selected
					CQuadGrid<const CTriangle*>::CIterator	it=quadGrid.begin();
					while (it!=quadGrid.end())
					{
						const CTriangle	*tri= *it;

						// If same instanceId, skip
						if(tri->InstanceId != i)
						{
							CVector		hit;
							// If triangle occlude the ray, no sun Contribution
							if(tri->Triangle.intersect(lightPos, pos, hit, tri->Plane))
							{
								// The sample is not touched by sun. sub his contribution
								sunAccum-= 255;
								// End
								break;
							}
						}

						it++;
					}
				}

				// Average samples
				_Instances[i].SunContribution= sunAccum / nSamples;
			}

		}


		// Light SurfaceGrid Cells.
		//==========
		if(_IGSurfaceLightBuild)
		{
			// No instance currenlty computed, since we compute surface cells.
			_CurrentInstanceComputed= -1;

			// Begin cell iteration
			beginCell();
			// For all surface cell corners
			while( !isEndCell() )
			{
				progressCell("Compute SunContribution on Surfaces");

				// get the current cell and cellInfo iterated.
				CIGSurfaceLightBuild::CCellCorner	&cellInfo= getCurrentCellInfo();
				CSurfaceLightGrid::CCellCorner		&cell= getCurrentCell();

				// if the cell corner lies in the polygon surface.
				if(cellInfo.InSurface)
				{
					// number of samples (at least 1 if no overSampling)
					uint	nSamples= cellInfo.NumOverSamples;
					nlassert(nSamples>=1);

					// Default is full lighted.
					uint	sunAccum= 255*nSamples;

					// For all samples
					for(uint sample=0; sample<nSamples; sample++)
					{
						// Get pos to rayTrace.
						CVector	pos= cellInfo.OverSamples[sample];

						// rayTrace from the pos of this Cell sample.
						CVector		lightPos= pos-(rayDir*1000.f);
						// Select an element with the X axis as a 3d ray
						quadGrid.select (lightPos, lightPos);
						// For each triangle selected
						CQuadGrid<const CTriangle*>::CIterator	it=quadGrid.begin();
						while (it!=quadGrid.end())
						{
							const CTriangle	*tri= *it;

							CVector		hit;
							// If triangle occlude the ray, no sun Contribution
							if(tri->Triangle.intersect(lightPos, pos, hit, tri->Plane))
							{
								// The cell sample is not touched by sun. sub his contribution
								sunAccum-= 255;
								// End
								break;
							}

							it++;
						}
					}

					// Average SunContribution
					cell.SunContribution= sunAccum / nSamples;

					// copy it to cellInfo
					cellInfo.SunContribution= cell.SunContribution;
				}

				// next cell
				nextCell();
			}
		}
	}


	// Clean VCM and VCE
	if(landscape)
	{
		// delete CVisualCollisionManager and CVisualCollisionEntity
		VCM->deleteEntity(VCE);
		delete VCM;
	}

}



// ***************************************************************************
// ***************************************************************************
// PointLights part
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CInstanceLighter::CPointLightRT::CPointLightRT()
{
	RefCount= 0;
}


// ***************************************************************************
bool	CInstanceLighter::CPointLightRT::testRaytrace(const CVector &v, sint instanceComputed)
{
	CVector	dummy;

	if(!BSphere.include(v))
		return false;

	// If Ambient light, just skip
	if(PointLight.getType()== CPointLight::AmbientLight)
		return false;

	// If SpotLight verify in angle radius.
	if(PointLight.getType()== CPointLight::SpotLight)
	{
		float	att= PointLight.computeLinearAttenuation(v);
		if (att==0)
			return false;
	}


	// Select in the cubeGrid
	FaceCubeGrid.select(v);
	// For all faces selected
	while(!FaceCubeGrid.isEndSel())
	{
		const CTriangle	*tri= FaceCubeGrid.getSel();

		// If the triangle is not a triangle of the instance currenlty lighted
		if( instanceComputed<0 || tri->InstanceId != instanceComputed )
		{
			// If intersect, the point is occluded.
			if( tri->Triangle.intersect(BSphere.Center, v, dummy, tri->getPlane()) )
				return false;
		}

		// next
		FaceCubeGrid.nextSel();
	}

	// Ok the point is visilbe from the light
	return true;
}


// ***************************************************************************
void			CInstanceLighter::addStaticPointLight(const CPointLightNamed &pln, const char *igName)
{
	// NB: adding light more than 255 is allowed here, since the important thing is to not overflow really useful lights

	// build the plRT.
	CPointLightRT	plRT;
	plRT.PointLight= pln;
	// compute plRT.OODeltaAttenuation
	plRT.OODeltaAttenuation= pln.getAttenuationEnd() - pln.getAttenuationBegin();
	if(plRT.OODeltaAttenuation <=0 )
		plRT.OODeltaAttenuation= 1e10f;
	else
		plRT.OODeltaAttenuation= 1.0f / plRT.OODeltaAttenuation;
	// compute plRT.BSphere
	plRT.BSphere.Center= pln.getPosition();
	plRT.BSphere.Radius= pln.getAttenuationEnd();
	// NB: FaceCubeGrid will be computed during light()

	// add the plRT
	_StaticPointLights.push_back(plRT);
}


// ***************************************************************************
void			CInstanceLighter::compilePointLightRT(uint gridSize, float gridCellSize, std::vector<CTriangle>& obstacles, bool doShadow)
{
	uint	i;

	// Fill the quadGrid of Lights.
	// ===========
	_StaticPointLightQuadGrid.create(gridSize, gridCellSize);
	for(i=0; i<_StaticPointLights.size();i++)
	{
		CPointLightRT	&plRT= _StaticPointLights[i];

		// Compute the bbox of the light
		CAABBox		bbox;
		bbox.setCenter(plRT.BSphere.Center);
		float	hl= plRT.BSphere.Radius;
		bbox.setHalfSize(CVector(hl,hl,hl));

		// Insert the pointLight in the quadGrid.
		_StaticPointLightQuadGrid.insert(bbox.getMin(), bbox.getMax(), &plRT);
	}


	// Append triangles to cubeGrid ??
	if(doShadow)
	{
		// For all obstacles, Fill a quadGrid.
		// ===========
		CQuadGrid<CTriangle*>	obstacleGrid;
		obstacleGrid.create(gridSize, gridCellSize);
		uint	size= (uint)obstacles.size();
		for(i=0; i<size; i++)
		{
			// bbox of triangle
			CAABBox	bbox;
			bbox.setCenter(obstacles[i].Triangle.V0);
			bbox.extend(obstacles[i].Triangle.V1);
			bbox.extend(obstacles[i].Triangle.V2);
			// insert triangle in quadGrid.
			obstacleGrid.insert(bbox.getMin(), bbox.getMax(), &obstacles[i]);
		}


		// For all PointLights, fill his CubeGrid
		// ===========
		for(i=0; i<_StaticPointLights.size();i++)
		{
			// progress
			progress ("Compute Influences of PointLights 1/2", i / (float)_StaticPointLights.size());

			CPointLightRT	&plRT= _StaticPointLights[i];
			// Create the cubeGrid
			plRT.FaceCubeGrid.create(plRT.PointLight.getPosition(), NL3D_INSTANCE_LIGHTER_CUBE_GRID_SIZE);

			// AmbiantLIghts: do nothing.
			if(plRT.PointLight.getType()!=CPointLight::AmbientLight)
			{
				// Select only obstacle Faces around the light. Other are not useful
				CAABBox	bbox;
				bbox.setCenter(plRT.PointLight.getPosition());
				float	hl= plRT.PointLight.getAttenuationEnd();
				bbox.setHalfSize(CVector(hl,hl,hl));
				obstacleGrid.select(bbox.getMin(), bbox.getMax());

				// For all faces, fill the cubeGrid.
				CQuadGrid<CTriangle*>::CIterator	itObstacle;
				itObstacle= obstacleGrid.begin();
				while( itObstacle!=obstacleGrid.end() )
				{
					CTriangle	&tri= *(*itObstacle);
					/* Don't Test BackFace culling Here (unlike in CZoneLighter !!).
					   For objects:
						AutoOccluding problem is avoided with _CurrentInstanceComputed scheme.
						Also, With pointLights, there is no multiSampling (since no factor stored)
						Hence we are sure that no Object samples will lies under floor, and that the center of the
						object is far away.
					   For IGSurface lighting:
						notice that we already add 20cm in height because of "stairs problem" so
						floor/surface auto_shadowing is not a problem here...
					*/
					// Insert the triangle in the CubeGrid
					plRT.FaceCubeGrid.insert( tri.Triangle, &tri);

					itObstacle++;
				}
			}

			// Compile the CubeGrid.
			plRT.FaceCubeGrid.compile();

			// And Reset RefCount.
			plRT.RefCount= 0;
		}
	}
	// else, just build empty grid
	else
	{
		for(i=0; i<_StaticPointLights.size();i++)
		{
			// progress
			progress ("Compute Influences of PointLights 1/2", i / (float)_StaticPointLights.size());

			CPointLightRT	&plRT= _StaticPointLights[i];
			// Create a dummy empty cubeGrid => no rayTrace :)
			plRT.FaceCubeGrid.create(plRT.PointLight.getPosition(), 4);

			// Compile the CubeGrid.
			plRT.FaceCubeGrid.compile();

			// And Reset RefCount.
			plRT.RefCount= 0;
		}
	}

}


// ***************************************************************************
bool	CInstanceLighter::CPredPointLightToPoint::operator() (CPointLightRT *pla, CPointLightRT *plb) const
{
	float	ra= (pla->BSphere.Center - Point).norm();
	float	rb= (plb->BSphere.Center - Point).norm();
	float	infA= (pla->PointLight.getAttenuationEnd() - ra) * pla->OODeltaAttenuation;
	float	infB= (plb->PointLight.getAttenuationEnd() - rb) * plb->OODeltaAttenuation;
	// It is important to clamp, else strange results...
	clamp(infA, 0.f, 1.f);
	clamp(infB, 0.f, 1.f);
	// return which light impact the most.
	// If same impact
	if(infA==infB)
		// return nearest
		return ra < rb;
	else
		// return better impact
		return  infA > infB;
}


// ***************************************************************************
void			CInstanceLighter::processIGPointLightRT(std::vector<CPointLightNamed> &listPointLight)
{
	uint	i;
	vector<CPointLightRT*>		lightInfs;
	lightInfs.reserve(1024);

	// clear result list
	listPointLight.clear();


	// Compute each Instance
	//===========
	for(i=0; i<_InstanceInfos.size(); i++)
	{
		// If staticLight not enabled, skip.
		if( !_Instances[i].StaticLightEnabled )
			continue;

		CInstanceInfo	&inst= _InstanceInfos[i];
		// Avoid autoShadowing
		_CurrentInstanceComputed= i;

		// progress
		progress ("Compute Influences of PointLights 2/2", i / (float)_InstanceInfos.size());

		// get the point of the instance.
		CVector		pos= inst.CenterPos;

		// Default: takes no LocalAmbientLight;
		inst.LocalAmbientLight= NULL;
		float	furtherAmbLight= 0;

		// Compute Which light influences him.
		//---------
		lightInfs.clear();
		// Search possible lights around the position.
		_StaticPointLightQuadGrid.select(pos, pos);
		// For all of them, get the ones which touch this point.
		CQuadGrid<CPointLightRT*>::CIterator	it= _StaticPointLightQuadGrid.begin();
		while(it != _StaticPointLightQuadGrid.end())
		{
			CPointLightRT	*pl= *it;

			// Test if really in the radius of the light, no occlusion, not an ambient, and in Spot Angle setup
			if( pl->testRaytrace(pos, _CurrentInstanceComputed) )
			{
				// Ok, add the light to the lights which influence the instance
				lightInfs.push_back(pl);
			}

			// Ambient Light ??
			if( pl->PointLight.getType() == CPointLight::AmbientLight )
			{
				// If the instance is in radius of the ambiant light.
				float	dRadius= pl->BSphere.Radius - (pl->BSphere.Center - pos).norm();
				if(dRadius>0)
				{
					// Take the best ambient light: the one which is further from the circumference
					if(dRadius > furtherAmbLight)
					{
						furtherAmbLight= dRadius;
						inst.LocalAmbientLight= pl;
					}
				}
			}

			// next
			it++;
		}

		// If ambientLight chosen, inc Ref count of it
		if(inst.LocalAmbientLight)
			inst.LocalAmbientLight->RefCount++;

		// Choose the Best ones.
		//---------
		CPredPointLightToPoint	predPLTP;
		predPLTP.Point= pos;
		// sort.
		sort(lightInfs.begin(), lightInfs.end(), predPLTP);
		// truncate.
		lightInfs.resize( min((uint)lightInfs.size(), (uint)CInstanceGroup::NumStaticLightPerInstance) );


		// For each of them, fill instance
		//---------
		uint					lightInfId;
		for(lightInfId=0; lightInfId<lightInfs.size(); lightInfId++)
		{
			CPointLightRT	*pl= lightInfs[lightInfId];

			// copy light.
			inst.Light[lightInfId]= pl;

			// Inc RefCount of the light.
			pl->RefCount++;
		}
		// Reset any empty slot to NULL.
		for(; lightInfId<CInstanceGroup::NumStaticLightPerInstance; lightInfId++)
		{
			inst.Light[lightInfId]= NULL;
		}

	}


	// Compute Lighting on SurfaceLightGrid
	//===========
	// Must do it before compression !!
	// NB: big copy/Past from above
	if(_IGSurfaceLightBuild)
	{
		// No instance currenlty computed, since we compute surface cells.
		_CurrentInstanceComputed= -1;

		// Begin cell iteration
		beginCell();
		// For all surface cell corners
		while( !isEndCell() )
		{
			progressCell("Compute PointLights on Surfaces");

			// get the current cellInfo iterated.
			CIGSurfaceLightBuild::CCellCorner	&cellInfo= getCurrentCellInfo();

			// if the cell corner lies in the polygon surface.
			if(cellInfo.InSurface)
			{
				// get the point of the cell.
				CVector		pos= cellInfo.CenterPos;

				// Default: takes no LocalAmbientLight;
				cellInfo.LocalAmbientLight= NULL;
				float	furtherAmbLight= 0;

				// Compute Which light influences him.
				//---------
				lightInfs.clear();
				// Search possible lights around the position.
				_StaticPointLightQuadGrid.select(pos, pos);
				// For all of them, get the ones which touch this point.
				CQuadGrid<CPointLightRT*>::CIterator	it= _StaticPointLightQuadGrid.begin();
				while(it != _StaticPointLightQuadGrid.end())
				{
					CPointLightRT	*pl= *it;

					// Test if really in the radius of the light, no occlusion, not an ambient, and in Spot Angle setup
					if( pl->testRaytrace(pos, _CurrentInstanceComputed) )
					{
						// Ok, add the light to the lights which influence the cell
						lightInfs.push_back(pl);
					}

					// Ambient Light ??
					if( pl->PointLight.getType() == CPointLight::AmbientLight )
					{
						// If the instance is in radius of the ambiant light.
						float	dRadius= pl->BSphere.Radius - (pl->BSphere.Center - pos).norm();
						if(dRadius>0)
						{
							// Take the best ambient light: the one which is further from the circumference
							if(dRadius > furtherAmbLight)
							{
								furtherAmbLight= dRadius;
								cellInfo.LocalAmbientLight= pl;
							}
						}
					}

					// next
					it++;
				}

				// If ambientLight chosen, inc Ref count of it
				if(cellInfo.LocalAmbientLight)
					((CPointLightRT*)cellInfo.LocalAmbientLight)->RefCount++;


				// Choose the Best ones.
				//---------
				CPredPointLightToPoint	predPLTP;
				predPLTP.Point= pos;
				// sort.
				sort(lightInfs.begin(), lightInfs.end(), predPLTP);
				// truncate.
				lightInfs.resize( min((uint)lightInfs.size(), (uint)CSurfaceLightGrid::NumLightPerCorner) );


				// For each of them, fill cellInfo
				//---------
				uint					lightInfId;
				for(lightInfId=0; lightInfId<lightInfs.size(); lightInfId++)
				{
					CPointLightRT	*pl= lightInfs[lightInfId];

					// copy light.
					cellInfo.LightInfo[lightInfId]= pl;

					// Inc RefCount of the light.
					pl->RefCount++;
				}
				// Reset any empty slot to NULL.
				for(; lightInfId<CSurfaceLightGrid::NumLightPerCorner; lightInfId++)
				{
					cellInfo.LightInfo[lightInfId]= NULL;
				}

			}

			// next cell
			nextCell();
		}
	}



	// Compress and setup _Instances with compressed data.
	//===========
	uint	plId= 0;
	// Process each pointLights
	for(i=0; i<_StaticPointLights.size(); i++)
	{
		CPointLightRT	&plRT= _StaticPointLights[i];
		// If this light is used.
		if(plRT.RefCount > 0)
		{
			// Valid light ?
			if (plId <=0xFF)
			{
				// Must Copy it into Ig.
				listPointLight.push_back(plRT.PointLight);
				plRT.DstId= plId++;
				// If index >= 255, too many lights (NB: => because 255 is a NULL code).
			}
			else
			{
				nlwarning("ERROR: Too many Static Point Lights influence the IG!!");
				// Set 0xFF. Special code indicating that the light CAN'T BE USED => any instance using
				// it is buggy (won't be lighted by this light).
				plRT.DstId= plId++;
			}
		}
	}

	// For each instance, compress Point light info
	for(i=0; i<_Instances.size(); i++)
	{
		// If staticLight not enabled, skip.
		if( !_Instances[i].StaticLightEnabled )
			continue;

		CInstanceInfo				&instSrc= _InstanceInfos[i];
		CInstanceGroup::CInstance	&instDst= _Instances[i];

		// Do it for PointLights
		for(uint lightId= 0; lightId<CInstanceGroup::NumStaticLightPerInstance; lightId++)
		{
			if(instSrc.Light[lightId] == NULL)
			{
				// Mark as unused.
				instDst.Light[lightId]= 0xFF;
			}
			else
			{
				// Get index. NB: may still be 0xFF if 'Too many static light' bug.
				instDst.Light[lightId]= instSrc.Light[lightId]->DstId;
			}
		}

		// Ensure that all FF are at end of the list (possible because of the TooManyStaticLight bug).
		// But don't do a full sort, to preserve order due to influence...
		nlctassert(CInstanceGroup::NumStaticLightPerInstance==2);
		if(instDst.Light[0] == 0xFF)	swap(instDst.Light[0], instDst.Light[1]);

		// Do it for Ambientlight
		if(instSrc.LocalAmbientLight == NULL)
			instDst.LocalAmbientId= 0xFF;
		else
			// NB: may still be 0xFF if 'Too many static light' bug.
			instDst.LocalAmbientId= instSrc.LocalAmbientLight->DstId;
	}

	// For each cell, compress Point light info
	if(_IGSurfaceLightBuild)
	{
		// Begin cell iteration
		beginCell();
		// For all surface cell corners
		while( !isEndCell() )
		{
			// get the current cell and cellInfo iterated.
			CIGSurfaceLightBuild::CCellCorner	&cellInfo= getCurrentCellInfo();
			CSurfaceLightGrid::CCellCorner		&cell= getCurrentCell();

			if(cellInfo.InSurface)
			{
				// Do it for PointLights
				for(uint lightId= 0; lightId<CSurfaceLightGrid::NumLightPerCorner; lightId++)
				{
					if(cellInfo.LightInfo[lightId] == NULL)
					{
						// Mark as unused.
						cell.Light[lightId]= 0xFF;
					}
					else
					{
						// Get index. NB: may still be 0xFF if 'Too many static light' bug.
						cell.Light[lightId]= reinterpret_cast<CPointLightRT*>(cellInfo.LightInfo[lightId])->DstId;
					}
				}

				// Ensure that all FF are at end of the list (possible because of the TooManyStaticLight bug).
				// But don't do a full sort, to preserve order due to influence...
				nlctassert(CInstanceGroup::NumStaticLightPerInstance==2);
				if(cell.Light[0] == 0xFF)	swap(cell.Light[0], cell.Light[1]);

				// Do it for Ambientlight
				if(cellInfo.LocalAmbientLight == NULL)
					cell.LocalAmbientId= 0xFF;
				else
					// NB: may still be 0xFF if 'Too many static light' bug.
					cell.LocalAmbientId= ((CPointLightRT*)cellInfo.LocalAmbientLight)->DstId;
			}

			// next cell
			nextCell();
		}
	}


}


// ***************************************************************************
// ***************************************************************************
// lightIgSimple
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CInstanceLighter::lightIgSimple(CInstanceLighter &instLighter, const CInstanceGroup &igIn, CInstanceGroup &igOut, const CLightDesc &lightDesc, const char *igName)
{
	sint				i;


	// Setup.
	//=======
	// Init
	instLighter.init();

	// Add obstacles.
	std::vector<CInstanceLighter::CTriangle>	obstacles;
	// only if Shadowing On.
	if(lightDesc.Shadow)
	{
		// Map of shape to load
		std::map<string, IShape*> shapeMap;

		// For all instances of igIn.
		for(i=0; i<(sint)igIn.getNumInstance();i++)
		{
			// progress
			instLighter.progress("Loading Shapes obstacles", float(i)/igIn.getNumInstance());

			// Skip it??
			if(igIn.getInstance(i).DontCastShadow)
				continue;

			// Get the instance shape name
			string name= igIn.getShapeName(i);
			bool	shapeFound= true;

			// Try to find the shape in the UseShapeMap.
			std::map<string, IShape*>::const_iterator iteMap= lightDesc.UserShapeMap.find (name);

			// If not found in userShape map, try to load it from the temp loaded ShapeBank.
			if( iteMap == lightDesc.UserShapeMap.end() )
			{
				// Add a .shape at the end ?
				if (name.find('.') == std::string::npos)
					name += ".shape";

				// Get the instance shape name
				string nameLookup = CPath::lookup (name, false, false);
				if (!nameLookup.empty())
					name = nameLookup;

				// Find the shape in the bank
				iteMap= shapeMap.find (name);
				if (iteMap==shapeMap.end())
				{
					// Input file
					CIFile inputFile;

					if (!name.empty() && inputFile.open (name))
					{
						// Load it
						CShapeStream stream;
						stream.serial (inputFile);

						// Get the pointer
						iteMap=shapeMap.insert (std::map<string, IShape*>::value_type (name, stream.getShapePointer ())).first;
					}
					else
					{
						// Error
						nlwarning ("WARNING can't load shape %s\n", name.c_str());
						shapeFound= false;
					}
				}
			}

			if(shapeFound)
			{
				CMatrix		matInst;
				matInst.setPos(igIn.getInstancePos(i));
				matInst.setRot(igIn.getInstanceRot(i));
				matInst.scale(igIn.getInstanceScale(i));
				// Add triangles of this shape
				CInstanceLighter::addTriangles(*iteMap->second, matInst, obstacles, i);
			}

		}

		// Clean Up shapes.
		//-----------
		std::map<string, IShape*>::iterator iteMap;
		iteMap= shapeMap.begin();
		while(iteMap!= shapeMap.end())
		{
			// delte shape
			delete	iteMap->second;
			// delete entry in map
			shapeMap.erase(iteMap);
			// next
			iteMap= shapeMap.begin();
		}
	}

	// Add pointLights of the IG.
	for(i=0; i<(sint)igIn.getPointLightList().size();i++)
	{
		instLighter.addStaticPointLight( igIn.getPointLightList()[i], igName );
	}


	// Run.
	//=======
	instLighter.light(igIn, igOut, lightDesc, obstacles);

}


// ***************************************************************************
// ***************************************************************************
// Cell Iteration
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CInstanceLighter::progressCell(const char *message)
{
	float	cp= getCurrentCellNumber() / float(getTotalCellNumber());
	if( cp > _LastCellProgress+0.05f)
	{
		progress(message, cp);
		_LastCellProgress= cp;
	}
}


// ***************************************************************************
void			CInstanceLighter::beginCell()
{
	if(_IGSurfaceLightBuild)
	{
		_ItRetriever= _IGRetrieverGridMap.begin();
		if(_ItRetriever != _IGRetrieverGridMap.end() )
		{
			_ItRetrieverInfo= _IGSurfaceLightBuild->RetrieverGridMap.find(_ItRetriever->first);
			nlassert(_ItRetrieverInfo != _IGSurfaceLightBuild->RetrieverGridMap.end() );
			// We are suze here that the retriever is not empty, and that the grid herself is not empty too
			_ItSurfId= 0;
			_ItCellId= 0;
			_ItCurrentCellNumber= 0;
			_IsEndCell= false;
			_LastCellProgress= 0;
		}
		else
		{
			_IsEndCell= true;
		}
	}
	else
	{
		_IsEndCell= true;
	}
}

// ***************************************************************************
void			CInstanceLighter::nextCell()
{
	nlassert(!isEndCell());

	// Next Cell.
	_ItCellId++;
	_ItCurrentCellNumber++;

	// If end of Cells, next surface.
	if(_ItCellId >= _ItRetriever->second.Grids[_ItSurfId].Cells.size() )
	{
		_ItCellId= 0;
		_ItSurfId ++;
	}

	// If end of surface, next retriever.
	if(_ItSurfId >= _ItRetriever->second.Grids.size() )
	{
		_ItSurfId= 0;
		_ItRetriever++;
		if(_ItRetriever != _IGRetrieverGridMap.end())
		{
			// Get info.
			_ItRetrieverInfo= _IGSurfaceLightBuild->RetrieverGridMap.find(_ItRetriever->first);
			nlassert(_ItRetrieverInfo != _IGSurfaceLightBuild->RetrieverGridMap.end() );
		}
	}

	// If end of retreiver, End.
	if(_ItRetriever == _IGRetrieverGridMap.end())
	{
		_IsEndCell= true;
	}
}

// ***************************************************************************
bool			CInstanceLighter::isEndCell()
{
	return _IsEndCell;
}

// ***************************************************************************
CSurfaceLightGrid::CCellCorner		&CInstanceLighter::getCurrentCell()
{
	nlassert(!isEndCell());

	// return ref on Cell.
	return	_ItRetriever->second.Grids[_ItSurfId].Cells[_ItCellId];
}

// ***************************************************************************
CIGSurfaceLightBuild::CCellCorner	&CInstanceLighter::getCurrentCellInfo()
{
	nlassert(!isEndCell());

	// return ref on CellInfo.
	return _ItRetrieverInfo->second.Grids[_ItSurfId].Cells[_ItCellId];
}


// ***************************************************************************
bool			CInstanceLighter::isCurrentNeighborCellInSurface(sint xnb, sint ynb)
{
	nlassert(!isEndCell());

	// get a ref on the current grid.
	CSurfaceLightGrid	&surfGrid= _ItRetriever->second.Grids[_ItSurfId];
	// copute coordinate of the current cellCorner.
	sint	xCell, yCell;
	xCell= _ItCellId%surfGrid.Width;
	yCell= _ItCellId/surfGrid.Width;
	// compute coordinate of the neighbor cell corner
	xCell+= xnb;
	yCell+= ynb;

	// check if in the surfaceGrid
	if(xCell<0 || xCell>=(sint)surfGrid.Width)
		return false;
	if(yCell<0 || yCell>=(sint)surfGrid.Height)
		return false;

	// compute the neighbor id
	uint	nbId= yCell*surfGrid.Width + xCell;

	// Now check in the cellInfo if this cell is InSurface.
	if( !_ItRetrieverInfo->second.Grids[_ItSurfId].Cells[nbId].InSurface )
		return false;

	// Ok, the neighbor cell is valid.

	return true;
}

// ***************************************************************************
CSurfaceLightGrid::CCellCorner		&CInstanceLighter::getCurrentNeighborCell(sint xnb, sint ynb)
{
	nlassert(isCurrentNeighborCellInSurface(xnb, ynb));

	// get a ref on the current grid.
	CSurfaceLightGrid	&surfGrid= _ItRetriever->second.Grids[_ItSurfId];
	// copute coordinate of the current cellCorner.
	sint	xCell, yCell;
	xCell= _ItCellId%surfGrid.Width;
	yCell= _ItCellId/surfGrid.Width;
	// compute coordinate of the neighbor cell corner
	xCell+= xnb;
	yCell+= ynb;
	// compute the neighbor id
	uint	nbId= yCell*surfGrid.Width + xCell;

	// then return a ref on it
	return surfGrid.Cells[nbId];
}


// ***************************************************************************
CIGSurfaceLightBuild::CCellCorner	&CInstanceLighter::getCurrentNeighborCellInfo(sint xnb, sint ynb)
{
	nlassert(isCurrentNeighborCellInSurface(xnb, ynb));

	// get a ref on the current grid.
	CIGSurfaceLightBuild::CSurface	&surfGrid= _ItRetrieverInfo->second.Grids[_ItSurfId];
	// copute coordinate of the current cellCorner.
	sint	xCell, yCell;
	xCell= _ItCellId%surfGrid.Width;
	yCell= _ItCellId/surfGrid.Width;
	// compute coordinate of the neighbor cell corner
	xCell+= xnb;
	yCell+= ynb;
	// compute the neighbor id
	uint	nbId= yCell*surfGrid.Width + xCell;

	// then return a ref on it
	return surfGrid.Cells[nbId];
}


// ***************************************************************************
void			CInstanceLighter::dilateLightingOnSurfaceCells()
{
	// Begin cell iteration
	beginCell();
	// For all surface cell corners
	while( !isEndCell() )
	{
		progressCell("Dilate Surfaces grids");

		// get the current cell and cellInfo iterated.
		CIGSurfaceLightBuild::CCellCorner	&cellInfo= getCurrentCellInfo();
		CSurfaceLightGrid::CCellCorner		&cell= getCurrentCell();

		// if the cell is not in the polygon surface, try to get from his neighbors.
		if(!cellInfo.InSurface)
		{
			// Add Weighted influence of SunContribution, and get one of the PointLightContribution (random).
			uint	wgtSunContribution= 0;
			uint	wgtSunCount= 0;
			// search if one of 8 neighbors is InSurface.
			for(sint ynb= -1; ynb<= 1; ynb++)
			{
				for(sint xnb= -1; xnb<= 1; xnb++)
				{
					// center => skip.
					if( xnb==0 && ynb==0 )
						continue;
					// If the neighbor point is not out of the grid, and if in Surface.
					if( isCurrentNeighborCellInSurface(xnb, ynb) )
					{
						// get the neighbor cell
						CIGSurfaceLightBuild::CCellCorner	&nbCellInfo= getCurrentNeighborCellInfo(xnb, ynb);
						CSurfaceLightGrid::CCellCorner		&nbCell= getCurrentNeighborCell(xnb, ynb);
						// Add SunContribution.
						wgtSunContribution+= nbCell.SunContribution;
						wgtSunCount++;
						// Just Copy PointLight info.
						for(uint lightId= 0; lightId<CSurfaceLightGrid::NumLightPerCorner; lightId++)
							cell.Light[lightId]= nbCell.Light[lightId];
						// Just Copy AmbientLight info.
						cell.LocalAmbientId= nbCell.LocalAmbientId;


						// For debug mesh only, copy z from nb cellInfo
						cellInfo.CenterPos.z= nbCellInfo.CenterPos.z;
					}
				}
			}
			// average SunContribution.
			if(wgtSunCount>0)
			{
				cell.SunContribution= wgtSunContribution / wgtSunCount;

				// For debug mesh only, copy SunContribution into cellInfo
				cellInfo.SunContribution= cell.SunContribution;
				cellInfo.Dilated= true;
			}
		}

		// next cell
		nextCell();
	}
}



} // NL3D
