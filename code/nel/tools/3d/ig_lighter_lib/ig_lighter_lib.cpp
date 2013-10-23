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

#include "ig_lighter_lib.h"
#include "nel/misc/path.h"
#include "nel/pacs/retriever_bank.h"
#include "nel/pacs/global_retriever.h"
#include "nel/3d/scene_group.h"



using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;


// ***************************************************************************
void	CIgLighterLib::overSampleCell(CIGSurfaceLightBuild::CCellCorner &cell, uint nSampleReq, 
	const CLocalRetriever &localRetriever, CGlobalRetriever &globalRetriever, 
	sint retrieverInstanceId, const ULocalPosition &localPos, float cellSize,
	float cellRaytraceDeltaZ)
{
	uint	sample;

	nlassert(nSampleReq==2 || nSampleReq==4 || nSampleReq==8 || nSampleReq==16);
	nlassert(nSampleReq<=CInstanceLighter::MaxOverSamples);

	// Compute all localPosition according to overSampleGrid.
	//----------------
	float	s2= cellSize/2;
	float	s4= cellSize/4;
	float	s8= cellSize/8;
	ULocalPosition		localSamplePos[CInstanceLighter::MaxOverSamples];

	// copy from localPos. Surface and estimation.
	for(sample= 0; sample<nSampleReq; sample++)
		localSamplePos[sample]= localPos;

		// Special cases.
	if(nSampleReq==2)
	{
		localSamplePos[0].Estimation+= CVector(-s4, -s4, 0);
		localSamplePos[1].Estimation+= CVector(+s4, +s4, 0);
	}
	else if(nSampleReq==4)
	{
		localSamplePos[0].Estimation+= CVector(-s4, -s4, 0);
		localSamplePos[1].Estimation+= CVector(+s4, -s4, 0);
		localSamplePos[2].Estimation+= CVector(-s4, +s4, 0);
		localSamplePos[3].Estimation+= CVector(+s4, +s4, 0);
	}
	else if(nSampleReq==8)
	{
		/*
			For all 16 samples, but skip ones such that we have this pattern (X == sample).
			X . X .
			. X . X
			X . X .
			. X . X
		*/
		for(sample= 0; sample<16; sample++)
		{
			// Compute pos of the sample.
			uint	x= sample%4;
			uint	y= sample/4;
			// Skip to have only 8 samples like above
			if( (x+y) & 1 )
				continue;
			// Start at BL corner (-s2), get middle of sample (+s8) and there is 4 sample by line/column (*s4)
			// NB: index= sample/2 because only 8 samples are computed.
			localSamplePos[sample/2].Estimation.x+= -s2 + s8 + x*s4;
			localSamplePos[sample/2].Estimation.y+= -s2 + s8 + y*s4;
		}
	}
	else if(nSampleReq==16)
	{
		// For all 16 samples
		for(sample= 0; sample<16; sample++)
		{
			// Compute pos of the sample.
			uint	x= sample%4;
			uint	y= sample/4;
			// Start at BL corner (-s2), get middle of sample (+s8) and there is 4 sample by line/column (*s4)
			localSamplePos[sample].Estimation.x+= -s2 + s8 + x*s4;
			localSamplePos[sample].Estimation.y+= -s2 + s8 + y*s4;
		}
	}


	// For each sample, verify if inSurface, and store into samples.
	//----------------
	cell.NumOverSamples= 0;
	for(sample= 0; sample<nSampleReq; sample++)
	{
		bool	snapped;
		localRetriever.snapToInteriorGround(localSamplePos[sample], snapped);

		// If inSurface, add the sample to the list.
		if(snapped)
		{
			// build the globalPosition.
			UGlobalPosition	globalPos;
			globalPos.InstanceId= retrieverInstanceId;
			globalPos.LocalPosition= localSamplePos[sample];
			// Get the result global Position
			CVector		pos= globalRetriever.getGlobalPosition(globalPos);
			pos.z+= cellRaytraceDeltaZ;
			cell.OverSamples[cell.NumOverSamples++]= pos;
		}
	}

}


// ***************************************************************************
void	CIgLighterLib::lightIg(CInstanceLighter &instanceLighter,
	const CInstanceGroup &igIn, CInstanceGroup &igOut, CInstanceLighter::CLightDesc &lightDesc, 
	CSurfaceLightingInfo &slInfo, const char *igName)
{
	sint				i;


	// Setup.
	//=======
	// Init
	instanceLighter.init();

	// For interiors ig, disable Sun contrib according to ig.
	lightDesc.DisableSunContribution= !igIn.getRealTimeSunContribution();
	// Copy it to igOut, just to keep same setup data for in and out.
	igOut.enableRealTimeSunContribution(!lightDesc.DisableSunContribution);


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
			instanceLighter.progress("Loading Shapes obstacles", float(i)/igIn.getNumInstance());

			// Skip it?? IgLighterLib use the DontCastShadowForInterior flag. See doc of this flag
			if(igIn.getInstance(i).DontCastShadow || igIn.getInstance(i).DontCastShadowForInterior)
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

				// Lookup the file
				string nameLookup = CPath::lookup (name, false, false);
				if (!nameLookup.empty())
					name = nameLookup;

				// Find the shape in the bank
				iteMap= shapeMap.find (name);
				if (iteMap==shapeMap.end())
				{
					// Input file
					CIFile inputFile;

					if (inputFile.open (name))
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
		instanceLighter.addStaticPointLight( igIn.getPointLightList()[i], igName );
	}


	// Setup a CIGSurfaceLightBuild if needed.
	//=======
	CIGSurfaceLightBuild	*igSurfaceLightBuild= NULL;
	CGlobalRetriever		*globalRetriever= slInfo.GlobalRetriever;
	CRetrieverBank			*retrieverBank= slInfo.RetrieverBank;
	float	cellSurfaceLightSize= slInfo.CellSurfaceLightSize;
	if(retrieverBank && globalRetriever)
	{
		igSurfaceLightBuild= new CIGSurfaceLightBuild;
		igSurfaceLightBuild->CellSize= cellSurfaceLightSize;
		// col Identifier.
		string	colIdent= slInfo.ColIdentifierPrefix + slInfo.IgFileName + slInfo.ColIdentifierSuffix;

		// For any retreiverInstance with this identifier.
		//----------------
		uint	numInstances= (uint)globalRetriever->getInstances().size();
		for(uint instanceId=0; instanceId<numInstances; instanceId++)
		{
			const CRetrieverInstance	&instance= globalRetriever->getInstance(instanceId);
			// If this instance is an interior
			if ( instance.getType() == CLocalRetriever::Interior )
			{
				uint					localRetrieverId= instance.getRetrieverId();
				const CLocalRetriever	&localRetriever= retrieverBank->getRetriever(localRetrieverId);
				// get the identifer of this localRetriever
				string	retIdent= localRetriever.getIdentifier();

				// Match the ident??
				if( retIdent.find(colIdent)!=string::npos )
				{
					// check CRetrieverLightGrid not already present
					CIGSurfaceLightBuild::ItRetrieverGridMap	itRgm;
					itRgm= igSurfaceLightBuild->RetrieverGridMap.find(localRetrieverId);
					if( itRgm != igSurfaceLightBuild->RetrieverGridMap.end() )
					{
						nlwarning ("ERROR Found 2 different collision retriever with same identifier: '%s'. The 2nd is discared\n", retIdent.c_str());
					}
					else
					{
						// Append CRetrieverLightGrid.
						itRgm= igSurfaceLightBuild->RetrieverGridMap.insert(
							make_pair(localRetrieverId, CIGSurfaceLightBuild::CRetrieverLightGrid() ) ).first;
						CIGSurfaceLightBuild::CRetrieverLightGrid	&rlg= itRgm->second;

						// Resize Grids.
						uint	numSurfaces= (uint)localRetriever.getSurfaces().size();
						rlg.Grids.resize( numSurfaces );

						// Compute the bbox for all surfaces. (NB: local to the localRetriever).
						vector<CAABBox>		surfaceBBoxes;
						localRetriever.buildInteriorSurfaceBBoxes(surfaceBBoxes);

						// For each surface, compute it.
						for(uint surfaceId=0; surfaceId<numSurfaces; surfaceId++)
						{
							// Progress.
							char	stmp[256];
							sprintf(stmp, "Sample surfaces of %s", retIdent.c_str());
							instanceLighter.progress(stmp, surfaceId / float(numSurfaces));

							// Compute surface and size of the grid.
							CIGSurfaceLightBuild::CSurface		&surfDst= rlg.Grids[surfaceId];

							// Snap Origin on cellSize
							surfDst.Origin= surfaceBBoxes[surfaceId].getMin();
							surfDst.Origin.x= floorf(surfDst.Origin.x/cellSurfaceLightSize) * cellSurfaceLightSize;
							surfDst.Origin.y= floorf(surfDst.Origin.y/cellSurfaceLightSize) * cellSurfaceLightSize;

							// Snap Width / Height on cellSize.
							float	sizex= surfaceBBoxes[surfaceId].getMax().x - surfDst.Origin.x;
							float	sizey= surfaceBBoxes[surfaceId].getMax().y - surfDst.Origin.y;
							surfDst.Width= (uint)floorf(sizex/cellSurfaceLightSize) + 2;
							surfDst.Height= (uint)floorf(sizey/cellSurfaceLightSize) + 2;
							// Get Zcenter.
							float	zCenter= surfaceBBoxes[surfaceId].getCenter().z;

							// Allocate elements.
							surfDst.Cells.resize(surfDst.Width * surfDst.Height);

							// For all elements
							for(sint yCell=0; yCell<(sint)surfDst.Height; yCell++)
							{
								for(sint xCell=0; xCell<(sint)surfDst.Width; xCell++)
								{
									// compute pos of the cell.
									ULocalPosition	localPos;
									localPos.Estimation.x= surfDst.Origin.x + xCell*cellSurfaceLightSize;
									localPos.Estimation.y= surfDst.Origin.y + yCell*cellSurfaceLightSize;
									localPos.Estimation.z= zCenter;

									// snap the pos to the surface.
									localPos.Surface= surfaceId;
									bool	snapped;
									localRetriever.snapToInteriorGround(localPos, snapped);

									// if snapped then this point is IN the surface.
									CIGSurfaceLightBuild::CCellCorner	&cell= 
										surfDst.Cells[yCell * surfDst.Width + xCell];
									cell.InSurface= snapped;

									// If ok, retrieve the global (ie world) position
									if(snapped)
									{
										// build a valid globalPosition.
										UGlobalPosition	globalPos;
										globalPos.InstanceId= instanceId;
										globalPos.LocalPosition= localPos;
										// retrieve from globalRetriever.
										cell.CenterPos= globalRetriever->getGlobalPosition(globalPos);
										// Add a delta to simulate entity center
										cell.CenterPos.z+= slInfo.CellRaytraceDeltaZ;

										// OverSample
										if(lightDesc.OverSampling==0)
										{
											// No OverSample, just add CenterPos to the samples.
											cell.NumOverSamples= 1;
											cell.OverSamples[0]= cell.CenterPos;
										}
										else
										{
											// OverSample.
											overSampleCell(cell, lightDesc.OverSampling, localRetriever, 
												*globalRetriever, instanceId, localPos, cellSurfaceLightSize, 
												slInfo.CellRaytraceDeltaZ);
											// it is possible that no samples lies in surfaces (small surface).
											// In this case, just copy CenterPos into samples.
											if(cell.NumOverSamples==0)
											{
												cell.NumOverSamples= 1;
												cell.OverSamples[0]= cell.CenterPos;
											}
										}
									}
									else
									{
										// For debug mesh only, get an approximate pos.
										cell.CenterPos= localPos.Estimation + instance.getOrigin();
										cell.CenterPos.z+= slInfo.CellRaytraceDeltaZ;
									}

									// Init cell defaults
									cell.Dilated= false;
									cell.SunContribution= 0;
								}
							}
						}
					}
				}
			}
		}

	}


	// Run.
	//=======
	instanceLighter.light(igIn, igOut, lightDesc, obstacles, NULL, igSurfaceLightBuild);

	// Output a debug mesh??
	if(igSurfaceLightBuild && slInfo.BuildDebugSurfaceShape && !igSurfaceLightBuild->RetrieverGridMap.empty() )
	{
		// Do it for the sun and point lights.
		for(uint i=0;i<2;i++)
		{
			// compute
			CMesh::CMeshBuild			meshBuild;
			CMeshBase::CMeshBaseBuild	meshBaseBuild;
			CVector	deltaPos= CVector::Null;
			deltaPos.z= - slInfo.CellRaytraceDeltaZ + 0.1f;
			// What kind of debug?
			if( i==0 )
				igSurfaceLightBuild->buildSunDebugMesh(meshBuild, meshBaseBuild, deltaPos);
			else
				igSurfaceLightBuild->buildPLDebugMesh(meshBuild, meshBaseBuild, deltaPos, igOut);

			// build
			CMesh	mesh;
			mesh.build(meshBaseBuild, meshBuild);

			// Save.
			CShapeStream	shapeStream;
			shapeStream.setShapePointer(&mesh);
			COFile		file;
			if( i==0 )
				file.open(slInfo.DebugSunName);
			else
				file.open(slInfo.DebugPLName);
			shapeStream.serial(file);
		}
	}


	// Clean.
	//=======
	if(igSurfaceLightBuild)
		delete igSurfaceLightBuild;
}



