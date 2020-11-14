// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2011-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdafx.h"

// For MAX_RELEASE
#include <plugapi.h>
#include <maxversion.h>
#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/maxscript.h>
#else
#	include <MaxScrpt/maxscrpt.h>
#endif

#include "rpo.h"
#include "nel/3d/zone.h"
#include "nel/3d/zone_symmetrisation.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


// ***************************************************************************
static int getCommonEdge(PatchMesh* pPM, int edge, Patch& patch2)
{
	for(int e=0 ; e<4 ; e++)
	{
		if (patch2.edge[e]==edge)
			return(e);
	}
	nlassert (0);		// no!
	return(-1);
}

// ***************************************************************************

static int getCommonVertex(PatchMesh* pPM, int ipatch1, int ipatch2, int* pordervtx=NULL)
{
	Patch*	patch1;
	Patch*	patch2;

	patch1=&pPM->patches[ipatch1];
	patch2=&pPM->patches[ipatch2];

	int i;
	for(i=0 ; i<4 ; i++)
	{
		if (patch1->v[i]==patch2->v[0])			
		{
			break;
		}
		if (patch1->v[i]==patch2->v[1])			
		{
			break;
		}
		if (patch1->v[i]==patch2->v[2])			
		{
			break;
		}
		if (patch1->v[i]==patch2->v[3])			
		{
			break;
		}
	}
	if (i==4)
	{
		return(-1);
	}
	if (pordervtx)
	{
		*pordervtx=i;
	}
	return(patch1->v[i]);
}

// ***************************************************************************

static int getOtherBindedVertex(RPatchMesh*	pRPM, PatchMesh* pPM, int ipatch1, int ipatch2, int iOtherVertex)
{
	Patch*		patch1;
	Patch*		patch2;

	patch1=&pPM->patches[ipatch1];
	patch2=&pPM->patches[ipatch2];

	for(int i=0 ; i<4 ; i++)
	{
		UI_VERTEX uiv=pRPM->getUIVertex (patch1->v[i]);
		if (uiv.Binding.bBinded)
		{
			if ((int)uiv.Binding.nPatch==ipatch2 && i!=iOtherVertex)
			{
				return(patch1->v[i]);
			}
		}
	}
	return(-1);
}

// ***************************************************************************

static int getEdge(PatchMesh* pPM, Patch* patch, int iv1, int iv2)
{
	for(int i=0 ; i<4 ; i++)
	{
		PatchEdge edge=pPM->edges[patch->edge[i]];
		if (edge.v1==iv1 && edge.v2==iv2)
		{
			return(i);
		}
		if (edge.v2==iv1 && edge.v1==iv2)
		{
			return(i);
		}

	}
	return(-1);
}

// ***************************************************************************

int getScriptAppData (Animatable *node, uint32 id, int def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	int value;
	if (sscanf ((const char*)ap->data, "%d", &value)==1)
		return value;
	else
		return def;
}

// ***************************************************************************

bool RPatchMesh::exportZone(INode* pNode, PatchMesh* pPM, NL3D::CZone& zone, CZoneSymmetrisation &zoneSymmetry, int zoneId, float snapCell, float weldThreshold, bool forceBuildZoneSymmetry)
{
	Matrix3					TM;
	CPatchInfo				pi;
	std::vector<CPatchInfo>	patchinfo;
	sint					i,j;
	Point3					v;
	Patch*					pPatch;
	
	TM=pNode->GetObjectTM(0);

	// --- Get the rotation value and symmetry flags
	bool symmetry = getScriptAppData (pNode, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
	int rotate = getScriptAppData (pNode, NEL3D_APPDATA_ZONE_ROTATE, 0);

	// Need a tile bank ?
	if (symmetry || rotate || forceBuildZoneSymmetry)
	{
		// Bank loaded
		bool loaded = false;

		// Get the bank name
		std::string sName=GetBankPathName ();
		if (!sName.empty())
		{
			// Open the bank
			CIFile file;
			if (file.open (sName))
			{
				try
				{
					// Read it
					bank.clear();
					bank.serial (file);
					bank.computeXRef ();

					// Ok
					loaded = true;
				}
				catch (const EStream& e)
				{
					MessageBox (NULL, MaxTStrFromUtf8(e.what()).data(), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
				}
			}
		}

		// Not loaded ?
		if (loaded == false)
		{
			nlwarning ("Can't load any tile bank. Select on with the tile_utility plug-in");
			mprintf (_T("Can't load any tile bank. Select on with the tile_utility plug-in"));
			return false;
		}
	}

	// ---
	// --- Basic checks
	// ---

	// Map edge count
	map<pair<uint, uint>, uint >	edgeSet;

	// Triple edge Patch error
	set<uint>	patchError;

	// For each patches
	for (uint patch=0; patch<(uint)pPM->numPatches; patch++)
	{
		// For each edges
		for (uint edge=0; edge<4; edge++)
		{
			// Two vertices
			uint v1 = pPM->edges[pPM->patches[patch].edge[edge]].v1;
			uint v2 = pPM->edges[pPM->patches[patch].edge[edge]].v2;

			// Insert in the map
			map<pair<uint, uint>, uint >::iterator	ite;
			ite = edgeSet.find (pair<uint, uint>(min(v1, v2), max(v1, v2)));
			
			// Inserted ?
			if (ite == edgeSet.end())
				ite = edgeSet.insert (pair<pair<uint, uint>, uint>(pair<uint, uint>(min(v1, v2), max(v1, v2)), 1)).first;
			else
			{
				// Add a ref
				ite->second++;

				// Patch error ?
				if (ite->second>=3)
				{
					// Add a patch error
					patchError.insert (patch);
				}
			}
		}
	}

	// Some errors ?
	if (!patchError.empty())
	{
		// Make an error message
		std::string error = "Error: triple edge detected in ";

		// For each error
		set<uint>::iterator ite=patchError.begin();
		while (ite!=patchError.end())
		{
			// Sub error message
			error += toString("patch %d ", (*ite)+1);

			// Next error
			ite++;
		}

		// Show the message
		mprintf(_M("%s\n"), MaxTStrFromUtf8(error).data());
		nlwarning("%s", error.c_str());

		// Error
		return false;
	}

	// ---
	// --- Basic exports
	// ---
	for(i=0 ; i<pPM->numPatches ; i++)
	{
		pPatch=&pPM->patches[i];
		// - Vertices
		for(j=0 ; j<4 ; j++)
		{
			v=pPM->verts[pPatch->v[j]].p;
			v=v*TM;
			pi.Patch.Vertices[j].x=v.x;
			pi.Patch.Vertices[j].y=v.y;
			pi.Patch.Vertices[j].z=v.z;
		}
		// - Tangents
		for(j=0 ; j<8 ; j++)
		{
			v=pPM->vecs[pPatch->vec[j]].p;
			v=v*TM;
			pi.Patch.Tangents[j].x=v.x;
			pi.Patch.Tangents[j].y=v.y;
			pi.Patch.Tangents[j].z=v.z;
		}
		// - Interiors
		for(j=0 ; j<4 ; j++)
		{
			v=pPM->vecs[pPatch->interior[j]].p;
			v=v*TM;
			pi.Patch.Interiors[j].x=v.x;
			pi.Patch.Interiors[j].y=v.y;
			pi.Patch.Interiors[j].z=v.z;
		}
		pi.OrderS=1<<getUIPatch (i).NbTilesU;
		pi.OrderT=1<<getUIPatch (i).NbTilesV;
		pi.BindEdges[0].ZoneId=zoneId;
		pi.BindEdges[1].ZoneId=zoneId;
		pi.BindEdges[2].ZoneId=zoneId;
		pi.BindEdges[3].ZoneId=zoneId;
		pi.BaseVertices[0]=pPatch->v[0];
		pi.BaseVertices[1]=pPatch->v[1];
		pi.BaseVertices[2]=pPatch->v[2];
		pi.BaseVertices[3]=pPatch->v[3];
		pi.Tiles.resize(pi.OrderS*pi.OrderT);

		// ** Export tile colors

		// Resize color table
		pi.TileColors.resize ((pi.OrderS+1)*(pi.OrderT+1));

		// Export it
		int u,v;
		for (v=0; v<pi.OrderT+1; v++)
		for (u=0; u<pi.OrderS+1; u++)
		{
			// Get rgb value at this vertex
			uint color=getUIPatch (i).getColor (u+v*(pi.OrderS+1));

			// Create a rgba value
			CRGBA rgba ( (color&0xff0000)>>16, (color&0x00ff00)>>8, color&0xff );

			// Store it in the tile info
			pi.TileColors[u+v*(pi.OrderS+1)].Color565=rgba.get565();
		}

		// ** Export tile shading

		pi.Lumels.resize ((pi.OrderS*4)*(pi.OrderT*4), 255);

		// ---
		// --- Smooth flags
		// ---

		// Clear smooth flags
		pi.Flags&=~0xf;

		for (int edge=0; edge<4; edge++)
		{
			// Edge smooth ?
			if (!getUIPatch (i).getEdgeFlag (edge))
			{
				// Don't smooth
				pi.Flags|=(1<<edge);
			}
		}

		// Add this tile info
		patchinfo.push_back(pi);

	}
	
	// ---
	// --- Pass 1 :
	// --- Parse each patch and then each vertex.
	// ---
	int isrcpatch;
	Patch* srcpatch;
	for(isrcpatch=0 ; isrcpatch<pPM->numPatches ; isrcpatch++)
	{
		srcpatch=&pPM->patches[isrcpatch];

		for(int nv=0 ; nv<4 ; nv++)
		{
			UI_VERTEX uiv=getUIVertex (srcpatch->v[nv]);

			if (uiv.Binding.bBinded)
			{				
				int isrcedge;
				int n;
				int	icv;
				int idstpatch=uiv.Binding.nPatch;
				int idstedge=uiv.Binding.nEdge;
				int	orderdstvtx;

				n=-1;
				// -
				if (uiv.Binding.nType==BIND_SINGLE)
				{
					icv=getCommonVertex(pPM,idstpatch,isrcpatch,&orderdstvtx);			
					if (icv==-1)
					{
						mprintf(_M("Invalid bind\n"));
						nlwarning("Invalid bind");
						return false;
					}
					if (idstedge==orderdstvtx) 
					{
						n=0;
					}
					else
					{
						n=1;
					}
				}
				// -
				if (uiv.Binding.nType==BIND_25)
				{
					n=1;
					icv=getOtherBindedVertex(this, pPM, isrcpatch,idstpatch,nv);
					if (icv==-1)
					{
						n=0;
						icv=getCommonVertex(pPM,idstpatch,isrcpatch);			
						if (icv==-1)
						{
							mprintf(_M("Invalid bind\n"));
							nlwarning ("Invalid bind");
							return false;
						}
					}
				}
				// -
				if (uiv.Binding.nType==BIND_75)
				{
					n=2;
					icv=getOtherBindedVertex(this, pPM, isrcpatch,idstpatch,nv);
					if (icv==-1)
					{
						n=3;
						icv=getCommonVertex(pPM,idstpatch,isrcpatch);			
						if (icv==-1)
						{
							mprintf(_M("Invalid bind\n"));
							nlwarning ("Invalid bind");
							return false;
						}
					}
				}
				// -
				if (n!=-1)
				{
					isrcedge=getEdge(pPM,srcpatch,srcpatch->v[nv],icv);
					if (isrcedge==-1)
					{
						mprintf(_M("Invalid edge\n"));
						nlwarning("Invalid edge");
						return false;
					}
					// let's fill the dst patch (n is important here... it's the order)
					patchinfo[idstpatch].BindEdges[idstedge].NPatchs++;
					patchinfo[idstpatch].BindEdges[idstedge].Edge[n]=isrcedge;
					patchinfo[idstpatch].BindEdges[idstedge].Next[n]=isrcpatch;

					// let's fill the src patch also...
					patchinfo[isrcpatch].BindEdges[isrcedge].NPatchs=5;
					patchinfo[isrcpatch].BindEdges[isrcedge].Edge[0]=idstedge;
					patchinfo[isrcpatch].BindEdges[isrcedge].Next[0]=idstpatch;
				}
			}
		}
	}
	
	// ---
	// --- Pass 2 :
	// --- Get all one/one cases.
	// --- Parse each patch and each edge
	// ---
	for(i=0 ; i<pPM->numPatches ; i++)
	{
		pPatch=&pPM->patches[i];
		for(int e=0 ; e<4 ; e++)
		{
			PatchEdge edge=pPM->edges[pPatch->edge[e]];

			// One/One binding
#if (MAX_RELEASE < 4000)
			if (edge.patch2>=0)
			{		
				patchinfo[i].BindEdges[e].NPatchs=1;
				// 'coz i don't know wether edge.patch1 or edge.patch2 is
				// the patch that i am parsing
				if (edge.patch2!=i)
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patch2;
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patch2]);
				}
				else
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patch1;
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patch1]);
				}
			}
#else // (MAX_RELEASE < 4000)
			if (edge.patches.Count()>1)
			{		
				patchinfo[i].BindEdges[e].NPatchs=1;
				// 'coz i don't know wether edge.patch1 or edge.patch2 is
				// the patch that i am parsing
				if (edge.patches[1]!=i)
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patches[1];
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patches[1]]);
				}
				else
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patches[0];
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patches[0]]);
				}				
			}
#endif // (MAX_RELEASE < 4000)
		}
	}

	// Fill tile infos with temp data 
	// Tileset of the tile and rotation is important but tile number, rotation and case will be ajusted later
	// For each patches
	for (i=0; i<pPM->numPatches; i++)
	{
		// Ref on the patch info
		CPatchInfo &patchInfo = patchinfo[i];

		int u,v;
		for (v=0; v<patchInfo.OrderT; v++)
		for (u=0; u<patchInfo.OrderS; u++)
		{
			tileDesc &desc=getUIPatch (i).getTileDesc (u+v*patchInfo.OrderS);
			for (int l=0; l<3; l++)
			{
				if (l>=desc.getNumLayer ())
				{
					patchInfo.Tiles[u+v*patchInfo.OrderS].Tile[l]=0xffff;
				}
				else
				{
					// Get the tile index
					uint tile = desc.getLayer (l).Tile;
					uint tileRotation = desc.getLayer (l).Rotate;
					// this check was intended to avoid nel patch paint crash, but it breaks zone export
					//if (tile >= (uint)bank.getTileCount())
					//{
					//	std::string error = NLMISC::toString(
					//		"Incorrect tileset for this zone.\r\n"
					//		"Tile %u does not exist.\r\n"
					//		"There are %u tiles in the tilebank.",
					//		tile, bank.getTileCount());
					//	nlwarning(error.c_str());
					//	MessageBoxA(NULL, error.c_str(), "RPO2NEL", MB_OK | MB_ICONERROR);
					//	return false;
					//}

					// Set the tile
					patchInfo.Tiles[u+v*patchInfo.OrderS].Tile[l] = tile;
					patchInfo.Tiles[u+v*patchInfo.OrderS].setTileOrient (l, (uint8)tileRotation);
				}
			}
			if (patchInfo.Tiles[u+v*patchInfo.OrderS].Tile[0]==0xffff)
				patchInfo.Tiles[u+v*patchInfo.OrderS].setTile256Info (false, 0);
			else
			{
				if (desc.getCase()==0)
					patchInfo.Tiles[u+v*patchInfo.OrderS].setTile256Info (false, 0);
				else
				{
					// Transform 256 case
					uint case256 = desc.getCase()-1;
					patchInfo.Tiles[u+v*patchInfo.OrderS].setTile256Info (true, case256);
				}
			}
			patchInfo.Tiles[u+v*patchInfo.OrderS].setTileSubNoise (desc.getDisplace());

			// Default VegetableState: AboveWater. Important: must not be VegetableDisabled
			patchInfo.Tiles[u+v*patchInfo.OrderS].setVegetableState (CTileElement::AboveWater);
		}
	}

	// Transform the zone (symmetry and rotate)
	if (symmetry || rotate)
	{
		CMatrix sym, rot;
		sym.identity ();
		rot.identity ();
		if (symmetry)
			sym.scale (CVector (1, -1, 1));
		rot.rotateZ ((float) Pi * (float) rotate / 2.f);
		sym *= rot;	
		sym.invert ();
		if (!CPatchInfo::transform (patchinfo, zoneSymmetry, bank, symmetry, rotate, snapCell, weldThreshold, sym))
		{
			mprintf(_M("Can't transform the zone\n"));
			nlwarning("Can't transform the zone");
			return false;
		}
	}
	// Force the build ?
	else if (forceBuildZoneSymmetry)
	{
		// For each patches
		NL3D::CZoneSymmetrisation::CError error;

		// Build the structure
		if (!zoneSymmetry.build (patchinfo, snapCell, weldThreshold, bank, error, CMatrix::Identity))
		{
			uint i;
			for (i=0; i<error.Errors.size (); i++)
			{
				mprintf(_M("%s\n"), MaxTStrFromUtf8(error.Errors[i]));
			}
			return false;
		}
	}

	// Build the zone
	zone.build (zoneId, patchinfo, std::vector<CBorderVertex>());
	return true;
}

// ***************************************************************************

void RPatchMesh::importZone (PatchMesh* pPM, NL3D::CZone& zone, int &zoneId)
{
	// Patch info
	std::vector<CPatchInfo> patchs;
	std::vector<CBorderVertex> borderVertices;

	// Retrieve the geometry
	zone.retrieve (patchs, borderVertices);

	// Get the zone id
	zoneId = zone.getZoneId ();

	// Vertex number
	int vertexNum = 0;

	// Vertex map
	map<pair<uint, uint>, uint> mapEdgeVertex;

	// Number of vertices
	pPM->setNumVerts (4*patchs.size());
	SetNumVerts (0);
	SetNumVerts (4*patchs.size());

	// Number of patches
	pPM->setNumPatches (patchs.size());
	SetNumPatches (0);
	SetNumPatches (patchs.size());

	// Number of tangents
	// Number of interiors
	pPM->setNumVecs (12*patchs.size());

	// Number of edges
	pPM->setNumEdges (4*patchs.size());

	// Fill the vertices and tangents
	for (uint patch=0; patch<patchs.size(); patch++)
	{
		// The vector
		for (uint vert=0; vert<4; vert++)
		{
			// Pos ref
			CVector &pos = patchs[patch].Patch.Vertices[vert];
			CVector &inter = patchs[patch].Patch.Interiors[vert];

			// Dest ref
			PatchVert &destVert = pPM->verts[patch*4+vert];
			PatchVec &destVect = pPM->vecs[patch*12+vert];

			// Set the position
			destVert.p = Point3 (pos.x, pos.y, pos.z);
			destVect.p = Point3 (inter.x, inter.y, inter.z);

			// Set the flag
			destVert.flags = PVERT_CORNER;
			destVect.flags = PVEC_INTERIOR;
		}

		// The tan
		for (uint tang=0; tang<8; tang++)
		{
			// Pos ref
			CVector &pos = patchs[patch].Patch.Tangents[tang];

			// Dest ref
			PatchVec &destVect = pPM->vecs[patch*12+4+tang];

			// Set the position
			destVect.p = Point3 (pos.x, pos.y, pos.z);

			// Set the flag
			destVect.flags = 0;
		}

		// The indexes
		Patch &patchRef = pPM->patches[patch];
		for (uint i=0; i<4; i++)
		{
			patchRef.v[i] = patch*4 + i;
			patchRef.vec[2*i] = patch*12 + 4 + 2*i;
			patchRef.vec[2*i+1] = patch*12 + 4 + 2*i + 1;
			patchRef.interior[i] = patch*12 + i;
			patchRef.edge[i] = patch*4 + i;
			patchRef.smGroup = 1;
			patchRef.flags = 0;
			patchRef.type = PATCH_QUAD;
		}

		// Get the userinfo patch
		UI_PATCH &uiRef = getUIPatch (patch);
		uiRef.Init (getPowerOf2 (patchs[patch].OrderS), getPowerOf2 (patchs[patch].OrderT));

		// Copy tiles
		uint u, v;
		for (v=0; v<patchs[patch].OrderT; v++)
		for (u=0; u<patchs[patch].OrderS; u++)
		{
			// Tile index 
			uint tileindex = u+v*patchs[patch].OrderS;

			// Get the tile des
			tileDesc& desc = uiRef.getTileDesc (tileindex);
			int numLayer = 
				(patchs[patch].Tiles[tileindex].Tile[0]==NL_TILE_ELM_LAYER_EMPTY)?0:
				(patchs[patch].Tiles[tileindex].Tile[1]==NL_TILE_ELM_LAYER_EMPTY)?1:
				(patchs[patch].Tiles[tileindex].Tile[2]==NL_TILE_ELM_LAYER_EMPTY)?2:
				3;

			// Case info
			bool is256x256;
			uint8 uvOff;
			CTileElement &tileElement = patchs[patch].Tiles[tileindex];
			tileElement.getTile256Info(is256x256, uvOff);

			// Set the tile
			desc.setTile (numLayer, is256x256?uvOff+1:0, tileElement.getTileSubNoise(), 
				tileIndex (tileElement.Tile[0], tileElement.getTileOrient (0)), 
				tileIndex (tileElement.Tile[1], tileElement.getTileOrient (1)), 
				tileIndex (tileElement.Tile[2], tileElement.getTileOrient (2)));
		}

		// Tile colors
		for (v=0; v<(uint)(patchs[patch].OrderT+1); v++)
		for (u=0; u<(uint)(patchs[patch].OrderS+1); u++)
		{
			CRGBA color;
			color.set565 (patchs[patch].TileColors[u+v*(patchs[patch].OrderS+1)].Color565);
			uiRef.setColor (u+v*(patchs[patch].OrderS+1), (color.R<<16)|(color.G<<8)|(color.B));
		}

		// Edge flags
		for (uint edge=0; edge<4; edge++)
		{
			uiRef.setEdgeFlag (edge, (patchs[patch].Flags&(1<<edge))==0);
		}
	}

	// Rebuild the patch mesh
	pPM->InvalidateGeomCache();
	nlverify (pPM->buildLinkages ()==TRUE);
	pPM->computeInteriors ();
	pPM->ApplyConstraints ();

	// Invalidate
	InvalidateBindingInfo ();
	UpdateBinding (*pPM, 0);
	Validity (*pPM, true);
}

// ***************************************************************************


