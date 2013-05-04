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

#include "stdafx.h"
#include "nel_patch_mesh.h"
#include "nel/misc/time_nl.h"
#include "vertex_neighborhood.h"
#include "../nel_3dsmax_shared/nel_3dsmax_shared.h"
#include "nel/3d/zone_symmetrisation.h"

// For MAX_RELEASE
#include <plugapi.h>

using namespace NL3D;
using namespace NLMISC;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif max

//#ifndef min
//#define min(a,b)            (((a) < (b)) ? (a) : (b))
//#endif min

#define PBLOCK_REF	0

#ifdef USE_CACHE
#ifndef NDEBUG
#define DEBUG_PIPELINE
#endif // NDEBUG
#endif // USE_CACHE

float bindWhere[BIND_COUNT]=
{
	0.25f,
	0.75f,
	0.5f,
	0.5f
};

#define RK_APPDATA_TILEFILE 0
#define RK_APPDATA_LAND 1
#define REGKEY_TILEDIT "Software\\Nevrax\\Ryzom\\Tile_Edit"

//#define CHECK_VALIDITY		// check validity

// Default constructor, allocate the array
CPatchMeshData::CPatchMeshData ()
{
	// Get a global allocator
	CPatchAllocator& _Allocator=GetAllocator ();

	_UIPatch=_Allocator.AllocPatch.allocate();
	_UIVertex=_Allocator.AllocVertex.allocate();
	_MapHitToTileIndex=_Allocator.AllocInt.allocate();
}

// Copy constructor, allocate the array
CPatchMeshData::CPatchMeshData (const CPatchMeshData& src)
{
	this->CPatchMeshData::CPatchMeshData ();
	this->operator= (src);
}

// Destructor
CPatchMeshData::~CPatchMeshData ()
{
	// Get a global allocator
	CPatchAllocator& _Allocator=GetAllocator ();

	_Allocator.AllocPatch.free (_UIPatch);
	_Allocator.AllocVertex.free (_UIVertex);
	_Allocator.AllocInt.free (_MapHitToTileIndex);
}

// Copy
CPatchMeshData& CPatchMeshData::operator= (const CPatchMeshData& src)
{
	*_UIPatch=*src._UIPatch;
	*_UIVertex=*src._UIVertex;
	*_MapHitToTileIndex=*src._MapHitToTileIndex;

	return *this;
}

NL3D::CTileBank bank;

std::string GetBankPathName ()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_TILEDIT, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		char path[256];
		DWORD len=256;
		DWORD type;
		if (RegQueryValueEx(hKey, "Bank Path", 0, &type, (LPBYTE)path, &len)==ERROR_SUCCESS)
			return std::string (path);
		RegCloseKey (hKey);
	}
	return "";
}

int GetBankTileSetSet ()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_TILEDIT, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		int tileSetSet;
		DWORD len=256;
		DWORD type;
		if (RegQueryValueEx(hKey, "Tileset Set", 0, &type, (LPBYTE)&tileSetSet, &len)==ERROR_SUCCESS)
			return tileSetSet;
		RegCloseKey (hKey);
	}
	return -1;
}

void SetBankPathName (const std::string& path)
{
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_TILEDIT, &hKey)==ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, "Bank Path", 0, REG_SZ, (LPBYTE)path.c_str(), path.length()+1);
		RegCloseKey (hKey);
	}
}

void SetBankTileSetSet (int tileSetSet)
{
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_TILEDIT, &hKey)==ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, "Tileset Set", 0, REG_DWORD, (LPBYTE)&tileSetSet, 4);
		RegCloseKey (hKey);
	}
}

// RPatchMesh --------------------------------------------------------------------------------------------------------------------------------------------

// Constructeur
RPatchMesh::RPatchMesh (PatchMesh *pmesh)
{
	// Invalidate
	ValidGeom=NEVER;
	ValidTopo=NEVER;
	ValidTexmap=NEVER;
	ValidSelect=NEVER;
	ValidDisplay=NEVER;
	ValidBindingInfo=FOREVER;
	ValidBindingPos=FOREVER;
	rTess.ModeTile=true;
	rTess.KeepMapping=false;
	rTess.TransitionType=1;
	SetSelLevel (EP_OBJECT);
	rTess.TileTesselLevel=0;
	build=-1;
	paintHack=false;
	
	if (pmesh)
	{
		// Patches
		SetNumPatches (pmesh->getNumPatches());

		// Vertices
		SetNumVerts (pmesh->getNumVerts());
	}

	// Getback the binding information
	for (int v=0; v<pmesh->hooks.Count(); v++)
	{
		int hookPoint=pmesh->hooks[v].hookPoint;
		int hookEdge=pmesh->hooks[v].hookEdge;
		int hookPatch=pmesh->hooks[v].hookPatch;
		AddHook (hookPoint, hookEdge, *pmesh);
	}
}

RPatchMesh::RPatchMesh ()
{
	// Set level
	SetSelLevel (EP_OBJECT);
	build=-1;
	rTess.TransitionType=1;
	paintHack=false;

	// Invalidate
	ValidGeom=NEVER;
	ValidTopo=NEVER;
	ValidTexmap=NEVER;
	ValidSelect=NEVER;
	ValidDisplay=NEVER;
	ValidBindingInfo=FOREVER;
	ValidBindingPos=FOREVER;
	rTess.ModeTile=true;
	rTess.KeepMapping=false;
	rTess.TransitionType=1;
	SetSelLevel (EP_OBJECT);
	rTess.TileTesselLevel=0;
	build=-1;
	paintHack=false;
};

RPatchMesh::~RPatchMesh () 
{
};

// Check if a vertex is in a seg
bool IsVertexInEdge (int nVert, int nSeg, const PatchMesh& patch)
{
	if ((patch.edges[nSeg].v1==nVert)||(patch.edges[nSeg].v2==nVert))
		return true;
	else
		return false;
}

// return other vertex in edge
int GetOtherVertex (int nVert, int nSeg, const PatchMesh& patch)
{
	if (patch.edges[nSeg].v1==nVert)
		return patch.edges[nSeg].v2;
	else
	{
		nlassert (patch.edges[nSeg].v2==nVert);
		return patch.edges[nSeg].v1;
	}
}

// Check if two vertex are joined by a edge
int IsVerticesJoined (int nVert0, int nVert1, const CVertexNeighborhood& tab, const PatchMesh& patch)
{
	// Count of neigbor for vertex 0
	uint listSize=tab.getNeighborCount (nVert0);

	// List of neigbor
	const uint* pList=tab.getNeighborList (nVert0);

	// For each neigbor
	for (uint n=0; n<listSize; n++)
	{
		// If the vertex is on this edge, return the edge number
		if (IsVertexInEdge (nVert1, pList[n], patch))
			return pList[n];
	}
	return -1;
}

// Check if two vertex are joined by a edge
bool IsVerticesJoined2 (int nVert0, int nVert1, int& nVert2, int& nEdge0, int& nEdge1, const CVertexNeighborhood& tab, const PatchMesh& patch)
{
	int nCount=0;

	// 
	if (IsVerticesJoined (nVert0, nVert1, tab, patch)!=-1)
		return false;

	// Count of neigbor for vertex 0
	uint listSize=tab.getNeighborCount (nVert0);

	// List of neigbor
	const uint* pList=tab.getNeighborList (nVert0);

	// For each neigbor
	for (uint n=0; n<listSize; n++)
	{
		// Index of the next vertex of this edge
		int nNextVert=(patch.edges[pList[n]].v1==nVert0)?patch.edges[pList[n]].v2:patch.edges[pList[n]].v1;
		if (nNextVert!=-1)
		{
			// The two vertex are joined ?
			int nEdge=IsVerticesJoined (nNextVert, nVert1, tab, patch);

			// Yes !
			if (nEdge!=-1)
			{
				// One more
				nCount++;
				nVert2=nNextVert;
				nEdge0=nEdge;
				nEdge1=pList[n];
			}
		}
	}
	return (nCount==1);
}

// Check if a vertex is in a patch
bool IsVertexInPatch (int nVert, int nPatch, const PatchMesh& patch)
{
	for (int i=0; i<4; i++)
	{
		if (patch.patches[nPatch].v[i]==nVert)
			return true;
	}
	return false;
}

// Check binding
int CheckBind (int nVert, int nSeg, int& v0, int& v1, int& v2, int& v3, const CVertexNeighborhood& tab, const PatchMesh& patch, bool bAssert, bool bCreate)
{
	int config=-1;

	// Check config 1
	// v0 ******* nVert ******* v1
	//  *************************
	v0=patch.edges[nSeg].v1;
	v1=patch.edges[nSeg].v2;
	
	// Valid edge to bind on
	if (v0==-1)
	{
		if (bAssert)
			nlassert (0);
	}
	else
	{
		if (v1==-1)
		{
			if (bAssert)
				nlassert (0);
		}
		else
		{
#if (MAX_RELEASE < 4000)
			if ((patch.edges[nSeg].patch2!=-1)&&bCreate)
			{
				if (bAssert)
					nlassert (0);
			}
			else
#else // (MAX_RELEASE < 4000)
			if ((patch.edges[nSeg].patches.Count()>1)&&bCreate)
			{
				if (bAssert)
					nlassert (0);
			}
			else if (patch.edges[nSeg].patches.Count() > 0)
#endif // (MAX_RELEASE < 4000)
			{
				// config 1?
				
				int nSeg0=IsVerticesJoined (v0, nVert, tab, patch);
				int nSeg1=IsVerticesJoined (v1, nVert, tab, patch);
				if ((nSeg0!=-1)&&(nSeg1!=-1))
				{
					// additionnal check.
#if (MAX_RELEASE < 4000)
					if ((patch.edges[nSeg0].patch2==-1)&&(patch.edges[nSeg1].patch2==-1))
					{
						if (!IsVertexInPatch (nVert, patch.edges[nSeg].patch1, patch))
							config=0;
					}
#else // (MAX_RELEASE < 4000)
					if ((patch.edges[nSeg0].patches.Count()<2)&&(patch.edges[nSeg1].patches.Count()<2))
					{
						if (!IsVertexInPatch (nVert, patch.edges[nSeg].patches[0], patch))
							config=0;
					}
#endif // (MAX_RELEASE < 4000)
				}
				
				if (config==-1)
				{
					// Check config 2
					// v0 **** v2 **** nVert **** v3 **** v1
					//  ***********************************
					int nEdge0;
					int nEdge1;
					int nEdge2;
					int nEdge3;
					if (IsVerticesJoined2 (v0, nVert, v2, nEdge0, nEdge1, tab, patch)&&
						IsVerticesJoined2 (v1, nVert, v3, nEdge2, nEdge3, tab, patch))
					{
						// additionnal check
						if ((v2!=v3)&&
							(nEdge0!=nEdge1)&&
							(nEdge0!=nEdge2)&&
							(nEdge0!=nEdge3)&&
							(nEdge1!=nEdge2)&&
							(nEdge1!=nEdge3)&&
							(nEdge2!=nEdge3)&&
#if (MAX_RELEASE < 4000)
							(patch.edges[nEdge0].patch2==-1)&&
							(patch.edges[nEdge1].patch2==-1)&&
							(patch.edges[nEdge2].patch2==-1)&&
							(patch.edges[nEdge3].patch2==-1)&&
							(!IsVertexInPatch (nVert, patch.edges[nSeg].patch1, patch))&&
							(!IsVertexInPatch (v2, patch.edges[nSeg].patch1, patch))&&
							(!IsVertexInPatch (v3, patch.edges[nSeg].patch1, patch)))
#else // (MAX_RELEASE < 4000)
							(patch.edges[nEdge0].patches.Count()<2)&&
							(patch.edges[nEdge1].patches.Count()<2)&&
							(patch.edges[nEdge2].patches.Count()<2)&&
							(patch.edges[nEdge3].patches.Count()<2)&&
							(!IsVertexInPatch (nVert, patch.edges[nSeg].patches[0], patch))&&
							(!IsVertexInPatch (v2, patch.edges[nSeg].patches[0], patch))&&
							(!IsVertexInPatch (v3, patch.edges[nSeg].patches[0], patch)))
#endif // (MAX_RELEASE < 4000)
						{
							config=1;
						}
					}
				}

				if (config==-1)
				{
					// No config
					if (bAssert)
						nlassert (0);
				}
			}
		}
	}

	return config;
}

// Patchmesh Check validity (debug stuff)
bool RPatchMesh::Validity (const PatchMesh& patch, bool bAssert)
{
#ifdef CHECK_VALIDITY
	bool bRet=true;

	// Check patches count
	int nPatchCount=patch.numPatches;
	int nPatchUICount=getUIPatchSize();
	if (nPatchCount!=nPatchUICount)
	{
		if (bAssert)
			nlassert (0);	// Not the same patch count
		bRet=false;
	}

	// Check vertices count
	int nVertCount=patch.numVerts;
	int nVertUICount=getUIVertexSize();
	if (nVertCount!=nVertUICount)
	{
		if (bAssert)
			nlassert (0);	// Not the same vertices count
		bRet=false;
	}

	// Static table to avoid alloc prb
	CVertexNeighborhood& tab=vertexNeighborhoodGlobal;
	tab.build (patch);

	// Check binding info in vertices
	for (int nVertex=0; nVertex<(int)getUIVertexSize(); nVertex++)
	{
		// Binding on this vertex ?
		if (getUIVertex(nVertex).Binding.bBinded)
		{
			// Check # of the patch
			int nPatchNumber=getUIVertex(nVertex).Binding.nPatch;
			if ((nPatchNumber>=nPatchUICount)||(nPatchNumber<0))
			{
				if (bAssert)
					nlassert (0);	// # patch invalid in binding info
				bRet=false;
			}

			// Check # of the edge
			int nEdgeNumber=getUIVertex(nVertex).Binding.nEdge;
			if ((nEdgeNumber>=4)||(nEdgeNumber<0))
			{
				if (bAssert)
					nlassert (0);	// # edge invalid in binding info
				bRet=false;
			}

			// Check fWhere
			typeBind type = (typeBind)(getUIVertex(nVertex).Binding.nType);
			if ((type!=BIND_25)&&
				(type!=BIND_50)&&
				(type!=BIND_75)&&
				(type!=BIND_SINGLE))
			{
				if (bAssert)
					nlassert (0);	// fWhere value invalid in binding info
				bRet=false;
			}

			// Primary vertex
			int nPrim=getUIVertex(nVertex).Binding.nPrimVert;
			if ((nPrim<0)||(nPrim>=patch.numVerts))
			{
				if (bAssert)
					nlassert(0);
				bRet=false;

				if (nPrim!=nVertex)
				{
					if (!getUIVertex(nPrim).Binding.bBinded)
					{
						if (bAssert)
							nlassert(0);
						bRet=false;
					}

					if (getUIVertex(nPrim).Binding.nEdge!=(uint)nEdgeNumber)
					{
						if (bAssert)
							nlassert(0);
						bRet=false;
					}
					
					if (getUIVertex(nPrim).Binding.nPatch!=(uint)nPatchNumber)
					{
						if (bAssert)
							nlassert(0);
						bRet=false;
					}
					
					if ((getUIVertex(nPrim).Binding.nPrimVert==BIND_50)||(getUIVertex(nPrim).Binding.nPrimVert==BIND_SINGLE))
					{
						if (bAssert)
							nlassert(0);
						bRet=false;
					}
				}
			}

			// Check geom
			int v0, v1, v2, v3;
			switch (CheckBind (nPrim, patch.patches[nPatchNumber].edge[nEdgeNumber], v0, v1, v2, v3, tab, patch, bAssert, false))
			{
			case 0:
				if (type!=BIND_SINGLE)
				{
					if (bAssert)
						nlassert(0);
					bRet=false;
				}
				break;
			case 1:
				if (nVertex!=nPrim)
				{
					if (v2==nVertex)
					{
						if (type!=BIND_25)
						{
							if (bAssert)
								nlassert(0);
							bRet=false;
						}
					}
					else if (v3==nVertex)
					{
						if (type!=BIND_75)
						{
							if (bAssert)
								nlassert(0);
							bRet=false;
						}
					}
					else 
					{
						if (type!=BIND_50)
						{
							if (bAssert)
								nlassert(0);
							bRet=false;
						}
					}
				}
				break;
			}
		}
	}

	// Check mode tile
	int nSelTileSize=tileSel.GetSize();
	int nTileSize=getUIPatchSize()*NUM_TILE_SEL;
	if (nSelTileSize!=nTileSize)
	{
		if (bAssert)
			nlassert(0);
		bRet=false;
	}

	// Check count of it index
	if (rTess.TileTesselLevel>=0)
	{
		int nFaceCount=mesh.numFaces;
		int nHitCount=_Data._MapHitToTileIndex->size();
		if (nFaceCount!=nHitCount)
		{
			if (bAssert)
				nlassert (0);
			bRet=false;
		}
	}

	// Return the bad news...
	return bRet;
#else // CHECK_VALIDITY
	return true;
#endif 
}

static Point3 InterpCenter(Point3 e1, Point3 i1, Point3 i2, Point3 e2, Point3 *v1 = NULL, Point3 *v2 = NULL, Point3 *v3 = NULL, Point3 *v4 = NULL) 
{
	Point3 e1i1 =(e1 + i1) / 2.0f;
	Point3 i1i2 =(i1 + i2) / 2.0f;
	Point3 i2e2 =(i2 + e2) / 2.0f;
	Point3 a =(e1i1 + i1i2) / 2.0f;
	Point3 b =(i1i2 + i2e2) / 2.0f;
	if (v1)
		*v1 = e1i1;
	if (v2)
		*v2 = a;
	if (v3)
		*v3 = b;
	if (v4)
		*v4 = i2e2;
	return (a + b) / 2.0f;
}

// Update binded vertices's position
void RPatchMesh::UpdateBindingPos (PatchMesh& patch)
{
	// Check validity
#ifndef NDEBUG
	Validity (patch, true);
#endif // NDEBUG

	// Vertex count
	int nVertexCount=patch.getNumVerts ();

	// Pointer
	uint vertSize=getUIVertexSize();
	for (uint nV=0; nV<vertSize; nV++)
	{
		// Pointer on the patch
		UI_VERTEX* pVertex=&getUIVertex (nV);

		// Binded?
		if (pVertex->Binding.bBinded)
		{
			static const float fU[4]={0.f, 1.f, 0.f, -1.f};
			static const float fV[4]={1.f, 0.f, -1.f, 0.f};
			static const float fUadd[4]={0.f, 0.f, 1.f, 1.f};
			static const float fVadd[4]={0.f, 1.f, 1.f, 0.f};

			// Calcule new position of the vertex
			int nEdge=patch.patches[pVertex->Binding.nPatch].edge[pVertex->Binding.nEdge];
			PatchEdge &edge=patch.edges[nEdge];

			// Save
			Point3 vOld=patch.verts[nV].p;

			switch (pVertex->Binding.nType)
			{
			case BIND_25:
				{
					Point3 v0, v1, v2, v3, v4;
					v2=InterpCenter(patch.verts[edge.v1].p, patch.vecs[edge.vec12].p, patch.vecs[edge.vec21].p, 
					patch.verts[edge.v2].p, &v0, &v1, &v3, &v4);
					
					// Final interpolation
					patch.verts[nV].p=InterpCenter(patch.verts[edge.v1].p, v0, v1, v2,
						&patch.vecs[pVertex->Binding.nBefore2].p, &patch.vecs[pVertex->Binding.nBefore].p, 
						&patch.vecs[pVertex->Binding.nAfter].p, &patch.vecs[pVertex->Binding.nAfter2].p);
				}
				break;
			case BIND_50:
				patch.verts[nV].p=InterpCenter(patch.verts[edge.v1].p, patch.vecs[edge.vec12].p, patch.vecs[edge.vec21].p, 
					patch.verts[edge.v2].p, NULL, NULL, NULL, NULL);
				break;
			case BIND_75:
				{
					Point3 v0, v1, v2, v3, v4;
					v2=InterpCenter(patch.verts[edge.v1].p, patch.vecs[edge.vec12].p, patch.vecs[edge.vec21].p, 
					patch.verts[edge.v2].p, &v0, &v1, &v3, &v4);
					
					// Final interpolation
					patch.verts[nV].p=InterpCenter(v2, v3, v4, patch.verts[edge.v2].p,
						&patch.vecs[pVertex->Binding.nBefore2].p, &patch.vecs[pVertex->Binding.nBefore].p, 
						&patch.vecs[pVertex->Binding.nAfter].p, &patch.vecs[pVertex->Binding.nAfter2].p);
				}
				break;
			case BIND_SINGLE:
				patch.verts[nV].p=InterpCenter(patch.verts[edge.v1].p, patch.vecs[edge.vec12].p, patch.vecs[edge.vec21].p, 
					patch.verts[edge.v2].p, &patch.vecs[pVertex->Binding.nBefore2].p, &patch.vecs[pVertex->Binding.nBefore].p, 
					&patch.vecs[pVertex->Binding.nAfter].p, &patch.vecs[pVertex->Binding.nAfter2].p);
				break;
			default:
				nlassert (0);
			}
			patch.vecs[pVertex->Binding.nT].p += patch.verts[nV].p-vOld;
		}
	}
	patch.computeInteriors();
	patch.InvalidateGeomCache();
}

// Build internal binding info
void RPatchMesh::UpdateBindingInfo (PatchMesh& patch)
{
	// Make tab vert to edge list

	// Static table to avoid alloc prb
	CVertexNeighborhood& tab=vertexNeighborhoodGlobal;
	tab.build (patch);

	for (int n=0; n<patch.numVerts; n++)
	{
		if (getUIVertex (n).Binding.bBinded)
		{
			int v0=patch.edges[patch.patches[getUIVertex (n).Binding.nPatch].edge[getUIVertex (n).Binding.nEdge]].v1;
			int v1=patch.edges[patch.patches[getUIVertex (n).Binding.nPatch].edge[getUIVertex (n).Binding.nEdge]].v2;
			int vB=-1;
			int vA=-1;
			int vT=-1;

			// Count of neigbor for vertex n
			uint listSize=tab.getNeighborCount (n);

			// List of neigbor
			const uint* pList=tab.getNeighborList (n);

			// For each neigbor
			for (uint nn=0; nn<listSize; nn++)
			{
				if (getUIVertex (n).Binding.nType==BIND_25)
				{
					if (IsVertexInEdge (v0, pList[nn], patch))
					{
						nlassert (vB==-1);
						vB=pList[nn];
					}
					else if (IsVertexInEdge (getUIVertex (n).Binding.nPrimVert, pList[nn], patch))
					{
						nlassert (vA==-1);
						vA=pList[nn];
					}
					else
					{
						vT=pList[nn];
					}
				}
				else if (getUIVertex (n).Binding.nType==BIND_75)
				{
					if (IsVertexInEdge (getUIVertex (n).Binding.nPrimVert, pList[nn], patch))
					{
						nlassert (vB==-1);
						vB=pList[nn];
					}
					else if (IsVertexInEdge (v1, pList[nn], patch))
					{
						nlassert (vA==-1);
						vA=pList[nn];
					}
					else
					{
						vT=pList[nn];
					}
				}
				else if (getUIVertex (n).Binding.nType==BIND_50)
				{
					int nother=GetOtherVertex (n, pList[nn], patch);
					nlassert (nother!=-1);
					if ((getUIVertex (nother).Binding.bBinded&&
						 (getUIVertex (nother).Binding.nType==BIND_25)&&
						 (getUIVertex (nother).Binding.nPrimVert==(uint)n)))
					{
						nlassert (vB==-1);
						vB=pList[nn];
					}
					else if ((getUIVertex (nother).Binding.bBinded&&
						 (getUIVertex (nother).Binding.nType==BIND_75)&&
						 (getUIVertex (nother).Binding.nPrimVert==(uint)n)))
					{
						nlassert (vA==-1);
						vA=pList[nn];
					}
					else
					{
						vT=pList[nn];
					}
				}
				else if (getUIVertex (n).Binding.nType==BIND_SINGLE)
				{
					int nother=GetOtherVertex (n, pList[nn], patch);
					nlassert (nother!=-1);
					if (IsVertexInEdge (v0, pList[nn], patch))
					{
						nlassert (vB==-1);
						vB=pList[nn];
					}
					else if (IsVertexInEdge (v1, pList[nn], patch))

					{
						nlassert (vA==-1);
						vA=pList[nn];
					}
					else
					{
						vT=pList[nn];
					}
				}
			}
			nlassert (vB!=-1);
			nlassert (vA!=-1);
			nlassert (vT!=-1);
			getUIVertex (n).Binding.nBefore=(patch.edges[vB].v1==n)?patch.edges[vB].vec12:patch.edges[vB].vec21;
			getUIVertex (n).Binding.nAfter=(patch.edges[vA].v1==n)?patch.edges[vA].vec12:patch.edges[vA].vec21;
			getUIVertex (n).Binding.nT=(patch.edges[vT].v1==n)?patch.edges[vT].vec12:patch.edges[vT].vec21;

			getUIVertex (n).Binding.nBefore2=(patch.edges[vB].v1==n)?patch.edges[vB].vec21:patch.edges[vB].vec12;
			getUIVertex (n).Binding.nAfter2=(patch.edges[vA].v1==n)?patch.edges[vA].vec21:patch.edges[vA].vec12;
		}
	}
}

// Update binding
void RPatchMesh::UpdateBinding (PatchMesh& patch, TimeValue t)
{
	if (!ValidBindingInfo.InInterval (t))
	{
		ValidBindingInfo=FOREVER;
		UpdateBindingInfo (patch);
	}
	if (!ValidBindingPos.InInterval (t))
	{
		ValidBindingPos=FOREVER;
		UpdateBindingPos (patch);
	}
}

// Fill the binding info for a vertex. Don't forget to call UpdateBindingInfo after
void RPatchMesh::BindingVertex (int nVertex, int nPatch, int nEdge, int nPrimary, typeBind nType)
{
	// Some check
	nlassert (nVertex>=0);
	nlassert (nVertex<(sint)getUIVertexSize());
	nlassert (nPrimary>=0);
	nlassert (nPrimary<(sint)getUIVertexSize());
	nlassert (nPatch>=0);
	nlassert (nPatch<(sint)getUIPatchSize());
	nlassert (nEdge>=0);
	nlassert (nEdge<4);
	nlassert ((nType==BIND_25)||(nType==BIND_50)||(nType==BIND_75)||(nType==BIND_SINGLE));

	// Go...
	getUIVertex (nVertex).Binding.bBinded=true;
	getUIVertex (nVertex).Binding.nPatch=nPatch;
	getUIVertex (nVertex).Binding.nEdge=nEdge;
	getUIVertex (nVertex).Binding.nType=nType;
	getUIVertex (nVertex).Binding.nPrimVert=nPrimary;
	getUIVertex (nVertex).Binding.nBefore=-1;
	getUIVertex (nVertex).Binding.nAfter=-1;
	getUIVertex (nVertex).Binding.nT=-1;
	getUIVertex (nVertex).Binding.nBefore2=-1;
	getUIVertex (nVertex).Binding.nAfter2=-1;
}

// Unbind a vertex
void RPatchMesh::UnBindingVertex (int nVertex)
{
	getUIVertex (nVertex).Binding.bBinded=false;
	getUIVertex (nVertex).Binding.nAfter=-1;
	getUIVertex (nVertex).Binding.nAfter2=-1;
	getUIVertex (nVertex).Binding.nBefore=-1;
	getUIVertex (nVertex).Binding.nBefore2=-1;
	getUIVertex (nVertex).Binding.nT=-1;
}

// Resize vertex buffer
void RPatchMesh::SetNumVerts (int nVert)
{
	int nOldSize=getUIVertexSize();
	resizeUIVertex (nVert);
	for (int n=nOldSize; n<nVert; n++)
	{
		getUIVertex (n).Init ();
	}
} 

// Resize patch buffer
void RPatchMesh::SetNumPatches (int nPatch)
{
	int nOldSize=getUIPatchSize();
	resizeUIPatch (nPatch);
	for (int n=nOldSize; n<nPatch; n++)
	{
		getUIPatch (n).Init ();
	}

	// Resize sel array
	tileSel.SetSize (NUM_TILE_SEL*nPatch, 1);
}

// Subdivide both way
void RPatchMesh::Subdivide (int nPatch, int nV0, int nV1, int nV2, int nV3, int nCenter, int nFirstPatch, PatchMesh& patch)
{
	// Subdivide patch
	int nOldTileCount=(1<<getUIPatch (nPatch).NbTilesU)*(1<<getUIPatch (nPatch).NbTilesV);
	int nOldVertexCount=((1<<getUIPatch (nPatch).NbTilesU)+1)*((1<<getUIPatch (nPatch).NbTilesV)+1);
	int nTileU=max (0, (int)getUIPatch (nPatch).NbTilesU-1);
	int nTileV=max (0, (int)getUIPatch (nPatch).NbTilesV-1);

	// Copy info
	int nTileCountU=1<<nTileU;
	int nTileCountV=1<<nTileV;

	// * 0	
	// Patch ptr
	UI_PATCH *pPatch=&getUIPatch (nFirstPatch);

	// Init patch info without intialize tabl
	pPatch->Init (nTileU, nTileV, true);

	int v;
	for(v=0; v<nTileCountV; v++)
	for(int u=0; u<nTileCountU; u++)
	{
		// Tile info
		int nTile=u+v*nTileCountU;
		nlassert (nTile<(int)pPatch->getTileSize());
		int nOldTile=u+(2*nTileCountU*v);
		nOldTile%=nOldTileCount;
		nlassert (nOldTile<(int)getUIPatch (nPatch).getTileSize());
		pPatch->getTileDesc (nTile)=getUIPatch (nPatch).getTileDesc (nOldTile);
	}
	for(v=0; v<nTileCountV+1; v++)
	for(int u=0; u<nTileCountU+1; u++)
	{
		// Vertex info
		int nVertex=u+v*(nTileCountU+1);
		nlassert (nVertex<(int)pPatch->getColorSize());
		int nOldVertex=u+((2*nTileCountU+1)*v);
		nOldVertex%=nOldVertexCount;
		nlassert (nOldVertex<(int)getUIPatch (nPatch).getColorSize());
		pPatch->setColor (nVertex, getUIPatch (nPatch).getColor (nOldVertex));
	}

	// Edge info
	pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (0);
	pPatch->getEdge (3)=getUIPatch (nPatch).getEdge (3);

	// * 1
	// Patch ptr
	pPatch++;

	// Init patch info without intialize tabl
	pPatch->Init (nTileV, nTileU, true);

	for(v=0; v<nTileCountV; v++)
	for(int u=0; u<nTileCountU; u++)
	{
		// Tile info
		int nTile=(nTileCountV-1-v)+u*nTileCountV;
		nlassert (nTile<(int)pPatch->getTileSize());
		int nOldTile=u+(2*nTileCountU*(nTileCountV+v));
		nOldTile%=nOldTileCount;
		nlassert (nOldTile<(int)getUIPatch (nPatch).getTileSize());				
		/*pPatch->Tile[nTile]=getUIPatch (nPatch).Tile[nOldTile];
		pPatch->Tile[nTile].Rotate=(getUIPatch (nPatch).Tile[nOldTile].Rotate+3)&3;*/
		pPatch->getTileDesc (nTile)=getUIPatch (nPatch).getTileDesc (nOldTile);
		pPatch->getTileDesc (nTile).rotate (3);
	}
	for(v=0; v<nTileCountV+1; v++)
	for(int u=0; u<nTileCountU+1; u++)
	{
		// Vertex info
		int nVertex=(nTileCountV-v)+u*(nTileCountV+1);
		nlassert (nVertex<(int)pPatch->getColorSize());
		int nOldVertex=u+((2*nTileCountU+1)*(nTileCountV+v));
		nOldVertex%=nOldVertexCount;
		nlassert (nOldVertex<(int)getUIPatch (nPatch).getColorSize());
		pPatch->setColor (nVertex, getUIPatch (nPatch).getColor (nOldVertex));
	}

	// Edge info
	pPatch->getEdge (3)=getUIPatch (nPatch).getEdge (0);
	pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (1);

	// * 2
	// Patch ptr
	pPatch++;

	// Init patch info without intialize tabl
	pPatch->Init (nTileU, nTileV, true);

	for(v=0; v<nTileCountV; v++)
	for(int u=0; u<nTileCountU; u++)
	{
		// Tile info
		int nTile=(nTileCountU-1-u)+(nTileCountV-1-v)*nTileCountU;
		nlassert (nTile<(int)pPatch->getTileSize());
		int nOldTile=nTileCountU+u+(2*nTileCountU*(nTileCountV+v));
		nOldTile%=nOldTileCount;
		nlassert (nOldTile<(int)getUIPatch (nPatch).getTileSize());
		pPatch->getTileDesc (nTile)=getUIPatch (nPatch).getTileDesc (nOldTile);
		//pPatch->Tile[nTile].Rotate=(getUIPatch (nPatch).Tile[nOldTile].Rotate+2)&3;
		pPatch->getTileDesc (nTile).rotate (2);
	}
	for(v=0; v<nTileCountV+1; v++)
	for(int u=0; u<nTileCountU+1; u++)
	{
		// Vertex info
		int nVertex=(nTileCountU-u)+(nTileCountV-v)*(nTileCountU+1);
		nlassert (nVertex<(int)pPatch->getColorSize());
		int nOldVertex=nTileCountU+u+((2*nTileCountU+1)*(nTileCountV+v));
		nOldVertex%=nOldVertexCount;
		nlassert (nOldVertex<(int)getUIPatch (nPatch).getColorSize());
		pPatch->setColor (nVertex, getUIPatch (nPatch).getColor (nOldVertex));
	}

	// Edge info
	pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (2);
	pPatch->getEdge (3)=getUIPatch (nPatch).getEdge (1);

	// * 3
	// Patch ptr
	pPatch++;

	// Init patch info without intialize tabl
	pPatch->Init (nTileV, nTileU, true);

	for(v=0; v<nTileCountV; v++)
	for(int u=0; u<nTileCountU; u++)
	{
		// Tile info
		int nTile=v+(nTileCountU-1-u)*nTileCountV;
		nlassert (nTile<(int)pPatch->getTileSize());
		int nOldTile=nTileCountU+u+(2*nTileCountU*v);
		nOldTile%=nOldTileCount;
		nlassert (nOldTile<(int)getUIPatch (nPatch).getTileSize());				
		pPatch->getTileDesc (nTile)=getUIPatch (nPatch).getTileDesc (nOldTile);
 		//pPatch->Tile[nTile].Rotate=(getUIPatch (nPatch).Tile[nOldTile].Rotate+1)&3;
		pPatch->getTileDesc (nTile).rotate (1);
	}
	for(v=0; v<nTileCountV+1; v++)
	for(int u=0; u<nTileCountU+1; u++)
	{
		// Vertex info
		int nVertex=v+(nTileCountU-u)*(nTileCountV+1);
		nlassert (nVertex<(int)pPatch->getColorSize());
		int nOldVertex=nTileCountU+u+((2*nTileCountU+1)*v);
		nOldVertex%=nOldVertexCount;
		nlassert (nOldVertex<(int)getUIPatch (nPatch).getColorSize());
		pPatch->setColor (nVertex, getUIPatch (nPatch).getColor (nOldVertex));
	}

	// Edge info
	pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (3);
	pPatch->getEdge (3)=getUIPatch (nPatch).getEdge (2);

	// Update binding info
	for (int i=0; i<(int)getUIVertexSize(); i++)
	{
		if (getUIVertex (i).Binding.bBinded)
		{
			if ((int)getUIVertex (i).Binding.nPatch==nPatch)
			{
				UnBindingVertex (i);
			}
		}
	}
	UnbindRelatedPatch (nPatch, patch);
	InvalidateBindingInfo ();
}

// Subdivide edge 1 and 3
void RPatchMesh::SubdivideU (int nPatch, int nV0, int nV1, int nFirstPatch, PatchMesh& patch)
{
	// Subdivide patch
	int nOldTileCount=(1<<getUIPatch (nPatch).NbTilesU)*(1<<getUIPatch (nPatch).NbTilesV);
	int nOldVertexCount=((1<<getUIPatch (nPatch).NbTilesU)+1)*((1<<getUIPatch (nPatch).NbTilesV)+1);
	int nTileU=(int)getUIPatch (nPatch).NbTilesV;
	int nTileV=max (0, (int)getUIPatch (nPatch).NbTilesU-1);
	for (int nNewPatch=nFirstPatch; nNewPatch<nFirstPatch+2; nNewPatch++)
	{
		// Patch ptr
		UI_PATCH *pPatch=&getUIPatch (nNewPatch);

		// New patch number
		int nNewPatchNum=nNewPatch-nFirstPatch;

		// Coord of the subpatch
		int nU=nNewPatchNum;

		// Init patch info without intialize tabl
		pPatch->Init (nTileU, nTileV, true);

		// Copy info
		int nTileCountU=1<<nTileU;
		int nTileCountV=1<<nTileV;
		int v;
		for(v=0; v<nTileCountV; v++)
		for(int u=0; u<nTileCountU; u++)
		{
			// Tile info
			int nTile=u+v*nTileCountU;
			nlassert (nTile<(int)pPatch->getTileSize());
			int nOldTile=v+nTileCountV*nU+(nTileCountU-1-u)*nTileCountV*2;
			nOldTile%=nOldTileCount;
			nlassert (nOldTile<(int)getUIPatch (nPatch).getTileSize());				
			pPatch->getTileDesc (nTile)=getUIPatch (nPatch).getTileDesc (nOldTile);
			//pPatch->Tile[nTile].Rotate=(getUIPatch (nPatch).Tile[nOldTile].Rotate+3)&3;
			pPatch->getTileDesc (nTile).rotate (3);
		}
		for(v=0; v<nTileCountV+1; v++)
		for(int u=0; u<nTileCountU+1; u++)
		{
			// Vertex info
			int nVertex=u+v*(nTileCountU+1);
			nlassert (nVertex<(int)pPatch->getColorSize());
			int nOldVertex=v+nTileCountV*nU+(nTileCountU-u)*(nTileCountV*2+1);
			nOldVertex%=nOldVertexCount;
			nlassert (nOldVertex<(int)getUIPatch (nPatch).getColorSize ());
			pPatch->setColor (nVertex, getUIPatch (nPatch).getColor (nOldVertex));
		}

		// Edge info
		if (nNewPatch==nFirstPatch)
		{
			pPatch->getEdge (3)=getUIPatch (nPatch).getEdge (0);
			pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (1);
			pPatch->getEdge (2)=getUIPatch (nPatch).getEdge (3);
		}
		else
		{
			pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (1);
			pPatch->getEdge (1)=getUIPatch (nPatch).getEdge (2);
			pPatch->getEdge (2)=getUIPatch (nPatch).getEdge (3);
		}
	}

	// Update binding info
	for (int i=0; i<(int)getUIVertexSize(); i++)
	{
		if ((getUIVertex (i).Binding.bBinded)&&((int)getUIVertex (i).Binding.nPatch==nPatch))
		{
			if (getUIVertex (i).Binding.nEdge&1)
			{
				UnBindingVertex (i);
			}
			else
			{
				if (getUIVertex (i).Binding.nEdge==0)
				{
					getUIVertex (i).Binding.nPatch=nFirstPatch;
					getUIVertex (i).Binding.nEdge=3;
				}
				else
				{
					nlassert (getUIVertex (i).Binding.nEdge==2);
					getUIVertex (i).Binding.nPatch=nFirstPatch+1;
					getUIVertex (i).Binding.nEdge=1;
				}
			}
		}
	}
	UnbindRelatedPatch (nPatch, patch);
	InvalidateBindingInfo ();
}

// Subdivide edge 0 and 2
void RPatchMesh::SubdivideV (int nPatch, int nV0, int nV1, int nFirstPatch, PatchMesh& patch)
{
	// Subdivide patch
	int nOldTileCount=(1<<getUIPatch (nPatch).NbTilesU)*(1<<getUIPatch (nPatch).NbTilesV);
	int nOldVertexCount=((1<<getUIPatch (nPatch).NbTilesU)+1)*((1<<getUIPatch (nPatch).NbTilesV)+1);
	int nTileU=getUIPatch (nPatch).NbTilesU;
	int nTileV=max (0, (int)getUIPatch (nPatch).NbTilesV-1);
	for (int nNewPatch=nFirstPatch; nNewPatch<nFirstPatch+2; nNewPatch++)
	{
		// Patch ptr
		UI_PATCH *pPatch=&getUIPatch (nNewPatch);

		// New patch number
		int nNewPatchNum=nNewPatch-nFirstPatch;

		// Coord of the subpatch
		int nV=nNewPatchNum;

		// Init patch info without intialize tabl
		pPatch->Init (nTileU, nTileV, true);

		// Copy info
		int nTileCountU=1<<nTileU;
		int nTileCountV=1<<nTileV;
		int v;
		int u;
		for(v=0; v<nTileCountV; v++)
		for(u=0; u<nTileCountU; u++)
		{
			// Tile info
			int nTile=u+v*nTileCountU;
			nlassert (nTile<(int)pPatch->getTileSize());
			int nOldTile=u+(nTileCountU*(nV*nTileCountV+v));
			nOldTile%=nOldTileCount;
			nlassert (nOldTile<(int)getUIPatch (nPatch).getTileSize());
			pPatch->getTileDesc (nTile)=getUIPatch (nPatch).getTileDesc (nOldTile);
			//pPatch->Tile[nTile].Rotate=getUIPatch (nPatch).Tile[nOldTile].Rotate;
		}
		for(v=0; v<nTileCountV+1; v++)
		for(u=0; u<nTileCountU+1; u++)
		{
			// Vertex info
			int nVertex=u+v*(nTileCountU+1);
			nlassert (nVertex<(int)pPatch->getColorSize());
			int nOldVertex=u+((nTileCountU+1)*(nV*nTileCountV+v));
			nOldVertex%=nOldVertexCount;
			nlassert (nOldVertex<(int)getUIPatch (nPatch).getColorSize());
			pPatch->setColor (nVertex, getUIPatch (nPatch).getColor (nOldVertex));
		}

		// Edge info
		if (nNewPatch==nFirstPatch)
		{
			pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (0);
			pPatch->getEdge (2)=getUIPatch (nPatch).getEdge (2);
			pPatch->getEdge (3)=getUIPatch (nPatch).getEdge (3);
		}
		else
		{
			pPatch->getEdge (0)=getUIPatch (nPatch).getEdge (0);
			pPatch->getEdge (1)=getUIPatch (nPatch).getEdge (1);
			pPatch->getEdge (2)=getUIPatch (nPatch).getEdge (2);
		}
	}

	// Update binding info
	for (int i=0; i<(int)getUIVertexSize (); i++)
	{
		if ((getUIVertex (i).Binding.bBinded)&&((int)getUIVertex (i).Binding.nPatch==nPatch))
		{
			if ((getUIVertex (i).Binding.nEdge&1)==0)
			{
				UnBindingVertex (i);
			}
			else
			{
				if (getUIVertex (i).Binding.nEdge==1)
				{
					getUIVertex (i).Binding.nPatch=nFirstPatch+1;
					getUIVertex (i).Binding.nEdge=1;
				}
				else
				{
					nlassert (getUIVertex (i).Binding.nEdge==3);
					getUIVertex (i).Binding.nPatch=nFirstPatch;
					getUIVertex (i).Binding.nEdge=3;
				}
			}
		}
	}
	UnbindRelatedPatch (nPatch, patch);
	InvalidateBindingInfo ();
}

// return on which edge is 
int WhereIsTheEdge (int nPatch, int nEdge, const PatchMesh& patch)
{
	for (int i=0; i<4; i++)
	{
		if ((patch.patches[nPatch].edge[i])==nEdge)
			return i;
	}
	nlassert (0);
	return -1;
}

// AddHook
void RPatchMesh::AddHook (int nVert, int nSeg, PatchMesh& patch)
{
#if (MAX_RELEASE < 4000)
	// Une side of the edge must be cleared
	nlassert (patch.edges[nSeg].patch2==-1);
	int patch1 = patch.edges[nSeg].patch1;
#else // (MAX_RELEASE < 4000)
	// Une side of the edge must be cleared
	nlassert (patch.edges[nSeg].patches.Count()<2);
	int patch1 = patch.edges[nSeg].patches.Count()>0?patch.edges[nSeg].patches[0]:-1;
#endif // (MAX_RELEASE < 4000)

	int nEdge=WhereIsTheEdge(patch1, nSeg, patch);
	nlassert(patch.patches[patch1].edge[nEdge]==nSeg);

	BindingVertex(nVert, patch1, nEdge, nVert, BIND_SINGLE);

	InvalidateBindingInfo ();
}

// AddHook
void RPatchMesh::AddHook (int nVert0, int nVert1, int nVert2, int nSeg, PatchMesh& patch)
{
#if (MAX_RELEASE < 4000)
	// Une side of the edge must be cleared
	nlassert (patch.edges[nSeg].patch2==-1);
	int patch1 = patch.edges[nSeg].patch1;
#else // (MAX_RELEASE < 4000)
	// Une side of the edge must be cleared
	nlassert (patch.edges[nSeg].patches.Count()<2);
	int patch1 = patch.edges[nSeg].patches.Count()>0?patch.edges[nSeg].patches[0]:-1;
#endif // (MAX_RELEASE < 4000)

	int nEdge = WhereIsTheEdge(patch1, nSeg, patch);

	BindingVertex(nVert0, patch1, nEdge, nVert1, BIND_25);
	BindingVertex(nVert1, patch1, nEdge, nVert1, BIND_50);
	BindingVertex(nVert2, patch1, nEdge, nVert1, BIND_75);

	InvalidateBindingInfo ();
}

// RemoveHook
void RPatchMesh::RemoveHook (PatchMesh& patch)
{
	for (int i=0; i<patch.numVerts; i++)
	{
		if (patch.vertSel[i])
			UnbindRelatedVertex (i, patch);
	}
}

// Attach -> binding safe
void RPatchMesh::Attach(RPatchMesh *rattPatch, PatchMesh& patch)
{
	// Add to the end.

	// Resize buffers
	int nOldVertCount=getUIVertexSize();
	int nOldPolyCount=getUIPatchSize();
	int nNewVertCount=nOldVertCount+rattPatch->getUIVertexSize();
	int nNewPolyCount=nOldPolyCount+rattPatch->getUIPatchSize();
	SetNumVerts (nNewVertCount);
	SetNumPatches (nNewPolyCount);

	// Add vert and patch, binding safe
	for (int nV=0; nV<nNewVertCount-nOldVertCount; nV++)
	{
		getUIVertex (nOldVertCount+nV)=rattPatch->getUIVertex (nV);

		// Bind patch
		if (getUIVertex (nOldVertCount+nV).Binding.bBinded)
		{
			getUIVertex (nOldVertCount+nV).Binding.nPatch+=nOldPolyCount;
			getUIVertex (nOldVertCount+nV).Binding.nPrimVert+=nOldVertCount;
		}
	}
	for (int nP=0; nP<nNewPolyCount-nOldPolyCount; nP++)
		getUIPatch (nOldPolyCount+nP)=rattPatch->getUIPatch (nP);

	InvalidateBindingInfo ();
}

int GetAdjacent (int nMe, int nedge, PatchMesh *pMesh)
{
	int nEdge=pMesh->patches[nMe].edge[nedge];
#if (MAX_RELEASE < 4000)
	if (pMesh->edges[nEdge].patch1==nMe)
		return pMesh->edges[nEdge].patch2;
	else
	{
		nlassert (pMesh->edges[nEdge].patch2==nMe);
		return pMesh->edges[nEdge].patch1;
	}
#else // (MAX_RELEASE < 4000)
	if ((pMesh->edges[nEdge].patches.Count()>0?pMesh->edges[nEdge].patches[0]:-1)==nMe)
		return (pMesh->edges[nEdge].patches.Count()>1?pMesh->edges[nEdge].patches[1]:-1);
	else
	{
		nlassert ((pMesh->edges[nEdge].patches.Count()>1?pMesh->edges[nEdge].patches[1]:-1)==nMe);
		return (pMesh->edges[nEdge].patches.Count()>0?pMesh->edges[nEdge].patches[0]:-1);
	}
#endif // (MAX_RELEASE < 4000)
}

// return on which edge is 
int WhereInMyAdjacent (int nMe, int nAdjacent, PatchMesh *pMesh)
{
	for (int i=0; i<4; i++)
	{
		if (GetAdjacent (nAdjacent, i, pMesh)==nMe)
			return i;
	}
	return -1;
}

// Extrude
void RPatchMesh::CreateExtrusion (PatchMesh *patch)
{
	// Resize buffer
	int nOldVertCount=getUIVertexSize();
	int nOldPolyCount=getUIPatchSize();
	int nNewVertCount=patch->getNumVerts ();
	int nNewPolyCount=patch->getNumPatches ();
	SetNumVerts (nNewVertCount);
	SetNumPatches (nNewPolyCount);

	patch->buildLinkages();

	// update bind data
	for (int i=0; i<nOldPolyCount; i++)
	{
		// Patch selected
		if (patch->patchSel[i])
		{
			/*int nNewFaces[4];
			int nWhichEdge[4];
			int nEdgeCount=(patch->patches[i].type==PATCH_QUAD)?4:3;
			
			// For all edges
			for (int nEdge=0; nEdge<nEdgeCount; nEdge++)
			{
				// Find the edge
				int nE=patch->patches[i].edge[nEdge];

				// Find the new face
				FindPatch (patch, nE, nWhichEdge[nEdge], nNewFaces[nEdge], nOldPolyCount);
			}

			// Look for binded point...
			for (int nV=0; nV<nOldVertCount; nV++)
			{
				if (getUIVertex (nV).Binding.bBinded)
				{
					if ((int)getUIVertex (nV).Binding.nPatch==i)
					{
						int nEd=getUIVertex (nV).Binding.nEdge;

						// Must have been found
						nlassert (nNewFaces[nEd]!=-1);
						nlassert (nWhichEdge[nEd]!=-1);
						
						UnBindingVertex (i);

						getUIVertex (nV).Binding.nPatch=nNewFaces[nEd];
						getUIVertex (nV).Binding.nEdge=(nWhichEdge[nEd]+2)&3;
					}
				}
			}*/

			// Look for binded point...
			for (int nV=0; nV<nOldVertCount; nV++)
			{
				if (getUIVertex (nV).Binding.bBinded)
				{
					if ((int)getUIVertex (nV).Binding.nPatch==i)
					{
						UnbindRelatedVertex (nV, *patch);
					}
				}
			}			
		}
	}
	InvalidateBindingInfo ();

	// Reinit some patches to get the good UV tessel level
	for (int nP=nOldPolyCount; nP<nNewPolyCount; nP++)
	{
		// Tessel
		int nU=RPO_DEFAULT_TESSEL;
		int nV=RPO_DEFAULT_TESSEL;

		// Look for old adjacent patch
		int nAdj=GetAdjacent (nP, 0, patch);
		if ((nAdj==-1)||(nAdj>=nOldPolyCount))
			nAdj=GetAdjacent (nP, 2, patch);
		if ((nAdj!=-1)&&(nAdj<nOldPolyCount))
		{
			// Where in my adjacent patch
			int nInAdj=WhereInMyAdjacent (nP, nAdj, patch);

			// Very chelou!
			nlassert (nInAdj!=-1);

			// Rip U or V?
			if (nInAdj&1)
				nV=getUIPatch (nAdj).NbTilesU;
			else
				nV=getUIPatch (nAdj).NbTilesV;
		}

		// Look for old adjacent patch
		nAdj=GetAdjacent (nP, 1, patch);
		if ((nAdj==-1)||(nAdj>=nOldPolyCount))
			nAdj=GetAdjacent (nP, 3, patch);
		if ((nAdj!=-1)&&(nAdj<nOldPolyCount))
		{
			// Where in my adjacent patch
			int nInAdj=WhereInMyAdjacent (nP, nAdj, patch);

			// Very chelou!
			nlassert (nInAdj!=-1);

			// Rip U or V?
			if (nInAdj&1)
				nU=getUIPatch (nAdj).NbTilesU;
			else
				nU=getUIPatch (nAdj).NbTilesV;
		}

		// Reinit?
		if ((nU!=RPO_DEFAULT_TESSEL)||(nV!=RPO_DEFAULT_TESSEL))
			getUIPatch (nP).Init (nU, nV);
	}
}

// Look for a patch with this edge
void RPatchMesh::FindPatch (PatchMesh *patch, int nEdge, int &WhichEdge, int &nPatch, int nFirstPatch)
{
	// In the new polygones
	int nn;
	for (nn=nFirstPatch; nn<patch->numPatches; nn++)
	{
		int nv;
		for (nv=0; nv<4; nv++)
		{
			if (patch->patches[nn].edge[nv]==nEdge)
			{
				nPatch=nn;
				WhichEdge=nv;
				break;
			}
		}
		if (nv!=4)
			break;
	}

	// Not found ?
	if (nn==patch->numPatches)
	{
		nPatch=-1;
		WhichEdge=-1;
	}
}

// Resolve topologie changes -> Bind safe
void RPatchMesh::ResolveTopoChanges(PatchMesh *patch, bool aux1)
{
	// Nombre de patches
	if (patch->numPatches>(int)getUIPatchSize())
		SetNumPatches (patch->numPatches);
	if (patch->numVerts>(int)getUIVertexSize())
		SetNumVerts (patch->numVerts);

	// static array to avoid alloc prob
	static std::vector<UI_PATCH>	cUIPatch;
	static std::vector<UI_VERTEX>	cUIVertex;
	static std::vector<int>			pnRemapPatch;
	static std::vector<int>			pnRemapVertex;
	
	// Reserve static array
	cUIPatch.reserve (300);
	cUIVertex.reserve (300);
	pnRemapPatch.reserve (300);
	pnRemapVertex.reserve (300);

	// Resize arrays
	pnRemapPatch.resize (0);
	pnRemapPatch.resize (getUIPatchSize(), -1);
	pnRemapVertex.resize (0);
	pnRemapVertex.resize (getUIVertexSize(), -1);

	// Fill the patch array
	uint size=getUIPatchSize();
	cUIPatch.resize (size);
	uint n;
	for (n=0; n<size; n++)
		cUIPatch[n]=getUIPatch (n);

	// Fill the vertex array
	size=getUIVertexSize();
	cUIVertex.resize (size);
	for (n=0; n<size; n++)
		cUIVertex[n]=getUIVertex (n);

	// Build remap vertex info
	int i;
	for (i = 0; i < patch->numVerts; ++i)
	{
		int nTag;
		if (aux1)
			nTag=patch->verts[i].aux1;
		else
			nTag=patch->verts[i].aux2;
		if (nTag>=0)
		{
			pnRemapVertex[nTag]=i;
		}
	}

	// Build remap poly info
	for (i = 0; i < patch->numPatches; ++i)
	{
		int nTag;
		if (aux1)
			nTag=patch->patches[i].aux1;
		else
			nTag=patch->patches[i].aux2;
		if (nTag>=0)
		{
			pnRemapPatch[nTag]=i;
		}
	}

	// Unbind vertex from deleted vertex
	for (i=0; i<(int)pnRemapVertex.size(); i++)
	{
		if (pnRemapVertex[i]==-1)
		{
			UnbindRelatedVertex (i, *patch);
		}
	}

	// Unbind vertex from deleted poly
	for (i=0; i<(int)pnRemapPatch.size(); i++)
	{
		if (pnRemapPatch[i]==-1)
		{
			UnbindRelatedPatch (i, *patch);
		}
	}

	// Remap poly info
	for (i = 0; i < patch->numPatches; ++i)
	{
		int nTag;
		if (aux1)
			nTag=patch->patches[i].aux1;
		else
			nTag=patch->patches[i].aux2;
		if (nTag>=0)
		{
			if (nTag!=i)
				getUIPatch (i)=cUIPatch[nTag];
		}
	}

	// Remap vertex info
	for (i = 0; i < patch->numVerts; ++i)
	{
		int nTag;
		if (aux1)
			nTag=patch->verts[i].aux1;
		else
			nTag=patch->verts[i].aux2;
		if ((nTag>=0)&&(nTag!=i))
		{
			getUIVertex (i)=cUIVertex[nTag];
		}
		if (getUIVertex (i).Binding.bBinded)
		{
			int nPatchTag;
			nPatchTag=pnRemapPatch[cUIVertex[nTag].Binding.nPatch];
			int nVertexTag=pnRemapVertex[cUIVertex[nTag].Binding.nPrimVert];
			if ((nPatchTag==-1)||(nVertexTag==-1))
				UnBindingVertex (i);
			else
			{
				getUIVertex (i).Binding.nPrimVert=nVertexTag;
				getUIVertex (i).Binding.nPatch=nPatchTag;
			}
		}
	}

	// Resize buffers
	SetNumPatches (patch->numPatches);
	SetNumVerts (patch->numVerts);

	// Invalidate binding infos
	InvalidateBindingInfo ();
}

// Weld -> Bind safe
void RPatchMesh::Weld (PatchMesh *patch)
{
	ResolveTopoChanges(patch, false);
}

int GetEdgeNumberInPatch (int nPatch, int nEdge, PatchMesh *patch)
{
	for (int i=0; i<4; i++)
	{
		if (patch->patches[nPatch].edge[i]==nEdge)
			return i;
	}
	return -1;
}

// Add a patch
void RPatchMesh::AddPatch (int nEdge, int nFirstPatch, PatchMesh *patch)
{
	// Add a patch
	SetNumPatches (getUIPatchSize()+1);

	// Find the tile resolution...
	int nV=RPO_DEFAULT_TESSEL;
	int nedge=GetEdgeNumberInPatch (nFirstPatch, nEdge, patch);
	nlassert (nedge!=-1);

	if (nedge&1)
		nV=getUIPatch (nFirstPatch).NbTilesU;
	else
		nV=getUIPatch (nFirstPatch).NbTilesV;
	getUIPatch (getUIPatchSize()-1).Init (nV, nV);
}

// Unbind vertex associed to the vertex
void RPatchMesh::UnbindRelatedVertex (int nVertex, PatchMesh& patch)
{
	uint nFace=getUIVertex (nVertex).Binding.nPatch;
	uint nEdge=getUIVertex (nVertex).Binding.nEdge;
	for (int j=0; j<(int)getUIVertexSize(); j++)
	{
		if ((getUIVertex (j).Binding.bBinded)&&
			(getUIVertex (j).Binding.nPatch==nFace)&&
			(getUIVertex (j).Binding.nEdge==nEdge))
		{
			UnBindingVertex (j);
		}
	}
}

// Unbind vertex associed to the patch
void RPatchMesh::UnbindRelatedPatch (int nPatch, PatchMesh& patch)
{
	for (int i=0; i<4; i++)
	{
		int n=patch.patches[nPatch].v[i];
		if (getUIVertex (n).Binding.bBinded)
		{
			UnbindRelatedVertex (n, patch);
		}
	}
}
// Delete patches and vertices
void RPatchMesh::DeleteAndSweep (const BitArray &remapVerts, const BitArray &remapPatches, PatchMesh& patch)
{
	int mapVSize=remapVerts.GetSize();
	int vSize=getUIVertexSize();
	int mapPSize=remapPatches.GetSize();
	int pSize=getUIPatchSize();
	nlassert (mapVSize==vSize);
	nlassert (mapPSize==pSize);

	int i,j;

	// Unbind vertex from deleted vertex
	for (i=0; i<(int)getUIVertexSize(); i++)
	{
		if (remapVerts[i])
		{
			UnbindRelatedVertex (i, patch);
		}
	}

	// Unbind vertex from deleted poly
	for (i=0; i<(int)getUIPatchSize(); i++)
	{
		if (remapPatches[i])
		{
			UnbindRelatedPatch (i, patch);
		}
	}

	// Make vertex remap table
	static std::vector<int> newVIndex;
	newVIndex.reserve (1000);

	newVIndex.resize (vSize);
	for (i=0, j=0; i<vSize; i++)
	{
		if (i!=j)
			getUIVertex (j)=getUIVertex (i);
		if (remapVerts[i])
			newVIndex[i]=-1;
		else
			newVIndex[i]=j++;
	}
	SetNumVerts (j);
	//UIVertex.resize (j);

	// Make patch remap table
	static std::vector<int> newPIndex;
	newPIndex.reserve (1000);

	newPIndex.resize (pSize);
	for (i=0, j=0; i<pSize; i++)
	{
		if (i!=j)
			getUIPatch (j)=getUIPatch (i);
		if (remapPatches[i])
			newPIndex[i]=-1;
		else
			newPIndex[i]=j++;
	}
	SetNumPatches (j);
	//getUIPatch )resize (j);

	// Remap bind
	for (i=0; i<(int)getUIVertexSize(); i++)
	{
		if (getUIVertex (i).Binding.bBinded)
		{
			int nPrim=newVIndex[getUIVertex (i).Binding.nPrimVert];
			int nPatch=newPIndex[getUIVertex (i).Binding.nPatch];
			if ((nPatch==-1)||(nPrim==-1))
				UnBindingVertex (i);
			else
			{
				getUIVertex (i).Binding.nPrimVert=nPrim;
				getUIVertex (i).Binding.nPatch=nPatch;
			}
		}
	}

	InvalidateBindingInfo ();
}

// Invalidate channels
void RPatchMesh::InvalidateChannels(ChannelMask channels)
{
	if (channels&TOPO_CHANNEL)
	{
		ValidTopo=NEVER;
		//InvalidateBindingInfo ();
	}
	if (channels&GEOM_CHANNEL)
	{
		ValidGeom=NEVER;
		//InvalidateBindingPos ();
	}
	if (channels&SELECT_CHANNEL)
		ValidSelect=NEVER;
	if (channels&TEXMAP_CHANNEL)
		ValidTexmap=NEVER;
	if (channels&DISP_ATTRIB_CHANNEL)
		ValidDisplay=NEVER;
}

// Load
IOResult RPatchMesh::Load(ILoad *iload)
{
	ULONG nb;

	// Version
	unsigned int nVersion;
	iload->Read(&nVersion, sizeof (nVersion), &nb);

	switch (nVersion)
	{
	case RPATCHMESH_SERIALIZE_VERSION_9:
	case RPATCHMESH_SERIALIZE_VERSION_8:
	case RPATCHMESH_SERIALIZE_VERSION_7:
	case RPATCHMESH_SERIALIZE_VERSION_6:
	case RPATCHMESH_SERIALIZE_VERSION_5:
	case RPATCHMESH_SERIALIZE_VERSION_4:
	case RPATCHMESH_SERIALIZE_VERSION_3:
	case RPATCHMESH_SERIALIZE_VERSION_2:
	case RPATCHMESH_SERIALIZE_VERSION_1:
		{
			// Patch info
			int nSize;
			iload->Read(&nSize, sizeof (nSize), &nb);
			SetNumPatches (nSize);
			int i;
			for (i=0; i<nSize; i++)
			{
				iload->Read(&getUIPatch (i).NbTilesU, sizeof (int), &nb);
				iload->Read(&getUIPatch (i).NbTilesV, sizeof (int), &nb);

				if (nVersion<RPATCHMESH_SERIALIZE_VERSION_3)
				{
					// Tiles
					int nSize2;
					iload->Read(&nSize2, sizeof (int), &nb);
					for (int j=0; j<nSize2; j++)
					{
						int old;
						iload->Read(&old, sizeof (int), &nb);
						iload->Read(&old, sizeof (int), &nb);
						getUIPatch (i).getTileDesc (j).setEmpty ();
					}
				}
				else	// RPATCHMESH_SERIALIZE_VERSION_3
				{
					// Tiles
					int nSize2;
					iload->Read(&nSize2, sizeof (int), &nb);
					for (int j=0; j<nSize2; j++)
					{
						iload->Read(&getUIPatch (i).getTileDesc (j)._Num, sizeof (USHORT), &nb);
						
						// Version 5, number of cell for 256x256 tiles
						if (nVersion>=RPATCHMESH_SERIALIZE_VERSION_5)
							iload->Read(&getUIPatch (i).getTileDesc (j)._Flags, sizeof (USHORT), &nb);
						else
							getUIPatch (i).getTileDesc (j)._Flags=0;
						
						// Clear displace flags in version lower than 8
						if (nVersion<RPATCHMESH_SERIALIZE_VERSION_9)
						{
							// Random a noise tile between 0 and 7
							uint noise=rand ()&0x7;
							getUIPatch (i).getTileDesc (j).setDisplace (noise);
						}
						else
						{
							// Read the noise
							uint8 noise;
							iload->Read(&noise, sizeof (uint8), &nb);
							getUIPatch (i).getTileDesc (j).setDisplace (noise);
						}

						for (int k=0; k<3; k++)
						{	
							bool invert;
							int tile;
							int rotate;
							iload->Read(&invert, sizeof (bool), &nb);
							iload->Read(&tile, sizeof (int), &nb);
							iload->Read(&rotate, sizeof (int), &nb);
							// Not used anymore getUIPatch (i).getTileDesc (j)._MatIDTab[k].Invert=invert;
							getUIPatch (i).getTileDesc (j)._MatIDTab[k].Tile=tile;
							getUIPatch (i).getTileDesc (j)._MatIDTab[k].Rotate=rotate;
						}
					}
				}

				// Colors
				int nSize2;
				iload->Read(&nSize2, sizeof (int), &nb);
				for (int j=0; j<nSize2; j++)
				{
					uint color;
					iload->Read(&color, sizeof (uint), &nb);
					getUIPatch (i).setColor (j, color);

					// Before RPATCHMESH_SERIALIZE_VERSION_6, force color to white
					if (nVersion<RPATCHMESH_SERIALIZE_VERSION_6)
						getUIPatch (i).setColor (j, 0xffffff);
				}

				// Edges info
				if (nVersion>=RPATCHMESH_SERIALIZE_VERSION_7)
				{
					// Save the 4 edges
					for (int e=0; e<4; e++)
					{
						// Get an edge ref
						CEdgeInfo& edge=getUIPatch (i).getEdge (e);

						// Read it
						iload->Read(&edge.Flags, sizeof (uint32), &nb);
					}
				}
			}

			// Vertex info
			iload->Read(&nSize, sizeof (nSize), &nb);
			SetNumVerts (nSize);
			for (i=0; i<nSize; i++)
			{
				bool bBinded;
				typeBind nType;
				uint nEdge;
				uint nPatch;
				uint nBefore;
				uint nBefore2;
				uint nAfter;
				uint nAfter2;
				uint nT;
				uint nPrimVert;
				
				iload->Read(&bBinded, sizeof (bool), &nb);
				iload->Read(&nType, sizeof (typeBind), &nb);
				iload->Read(&nEdge, sizeof (uint), &nb);
				iload->Read(&nPatch, sizeof (uint), &nb);
				iload->Read(&nBefore, sizeof (uint), &nb);
				iload->Read(&nBefore2, sizeof (uint), &nb);
				iload->Read(&nAfter, sizeof (uint), &nb);
				iload->Read(&nAfter2, sizeof (uint), &nb);
				iload->Read(&nT, sizeof (uint), &nb);
				iload->Read(&nType, sizeof (typeBind), &nb);
				iload->Read(&nPrimVert, sizeof (uint), &nb);
				
				getUIVertex (i).Binding.bBinded=bBinded;
				getUIVertex (i).Binding.nType=nType;
				getUIVertex (i).Binding.nEdge=nEdge;
				getUIVertex (i).Binding.nPatch=nPatch;
				getUIVertex (i).Binding.nBefore=nBefore;
				getUIVertex (i).Binding.nBefore2=nBefore2;
				getUIVertex (i).Binding.nAfter=nAfter;
				getUIVertex (i).Binding.nAfter2=nAfter2;
				getUIVertex (i).Binding.nT=nT;
				getUIVertex (i).Binding.nType=nType;
				getUIVertex (i).Binding.nPrimVert=nPrimVert;
			}
			
			// Some info
			iload->Read(&rTess.TileTesselLevel, sizeof (rTess.TileTesselLevel), &nb);
			iload->Read(&rTess.ModeTile, sizeof (rTess.ModeTile), &nb);
			iload->Read(&rTess.KeepMapping, sizeof (rTess.KeepMapping), &nb);
			if (nVersion==RPATCHMESH_SERIALIZE_VERSION_4)
				iload->Read(&rTess.TransitionType, sizeof (rTess.TransitionType), &nb);

			iload->Read(&selLevel, sizeof (selLevel), &nb);
		}
		break;
	default:
		return IO_ERROR;
	}

	ValidGeom.SetEmpty();
	ValidTopo.SetEmpty();
	ValidTexmap.SetEmpty();
	ValidSelect.SetEmpty();
	ValidDisplay.SetEmpty();
	ValidBindingInfo.SetEmpty();
	ValidBindingPos.SetEmpty();

	return IO_OK;
}

// Save
IOResult RPatchMesh::Save(ISave *isave)
{
	ULONG nb;

	// Version
	unsigned int nVersion=RPATCHMESH_SERIALIZE_VERSION;
	isave->Write(&nVersion, sizeof (nVersion), &nb);

	// Patch info
	int nSize=getUIPatchSize();
	isave->Write(&nSize, sizeof (nSize), &nb);
	int i;
	for (int i=0; i<nSize; i++)
	{
		isave->Write(&getUIPatch (i).NbTilesU, sizeof (int), &nb);
		isave->Write(&getUIPatch (i).NbTilesV, sizeof (int), &nb);

		// Tiles
		int nSize2=getUIPatch (i).getTileSize();
		isave->Write(&nSize2, sizeof (int), &nb);
		int j;
		for (j=0; j<nSize2; j++)
		{
			USHORT num=getUIPatch (i).getTileDesc (j)._Num;
			isave->Write(&num, sizeof (USHORT), &nb);
			num=getUIPatch (i).getTileDesc (j)._Flags;
			isave->Write(&num, sizeof (USHORT), &nb);

			// Save noise
			uint8 noise=getUIPatch (i).getTileDesc (j).getDisplace ();
			isave->Write(&noise, sizeof (uint8), &nb);

			for (int k=0; k<3; k++)
			{								
				bool invert = false;
				int tile;
				int rotate;
				// Not used anymore invert=(getUIPatch (i).getTileDesc (j)._MatIDTab[k].Invert!=0);
				tile=getUIPatch (i).getTileDesc (j)._MatIDTab[k].Tile;
				rotate=getUIPatch (i).getTileDesc (j)._MatIDTab[k].Rotate;
				isave->Write(&invert, sizeof (bool), &nb);
				isave->Write(&tile, sizeof (int), &nb);
				isave->Write(&rotate, sizeof (int), &nb);
			}
		}

		// Colors
		nSize2=getUIPatch (i).getColorSize();
		isave->Write(&nSize2, sizeof (int), &nb);
		for (j=0; j<nSize2; j++)
		{
			uint color=getUIPatch (i).getColor (j);
			isave->Write(&color, sizeof (int), &nb);
		}

		// Edges info (nVersion >= RPATCHMESH_SERIALIZE_VERSION_7)
		// Save the 4 edges
		for (int e=0; e<4; e++)
		{
			// Get an edge ref
			const CEdgeInfo& edge=getUIPatch (i).getEdge (e);

			// Write it
			isave->Write(&edge.Flags, sizeof (uint32), &nb);
		}
	}

	// Vertex info
	nSize=getUIVertexSize();
	isave->Write(&nSize, sizeof (nSize), &nb);
	for (i=0; i<nSize; i++)
	{
		bool bBinded=(getUIVertex (i).Binding.bBinded!=0);
		uint nEdge=getUIVertex (i).Binding.nEdge;
		uint nPatch=getUIVertex (i).Binding.nPatch;
		uint nBefore=getUIVertex (i).Binding.nBefore;
		uint nBefore2=getUIVertex (i).Binding.nBefore2;
		uint nAfter=getUIVertex (i).Binding.nAfter;
		uint nAfter2=getUIVertex (i).Binding.nAfter2;
		uint nT=getUIVertex (i).Binding.nT;
		typeBind nType=(typeBind)getUIVertex (i).Binding.nType;
		uint nPrimVert=getUIVertex (i).Binding.nPrimVert;

		isave->Write(&bBinded, sizeof (bool), &nb);
		isave->Write(&nType, sizeof (typeBind), &nb);
		isave->Write(&nEdge, sizeof (uint), &nb);
		isave->Write(&nPatch, sizeof (uint), &nb);
		isave->Write(&nBefore, sizeof (uint), &nb);
		isave->Write(&nBefore2, sizeof (uint), &nb);
		isave->Write(&nAfter, sizeof (uint), &nb);
		isave->Write(&nAfter2, sizeof (uint), &nb);
		isave->Write(&nT, sizeof (uint), &nb);
		isave->Write(&nType, sizeof (typeBind), &nb);
		isave->Write(&nPrimVert, sizeof (uint), &nb);
	}
	
	// Some info
	isave->Write(&rTess.TileTesselLevel, sizeof (rTess.TileTesselLevel), &nb);
	isave->Write(&rTess.ModeTile, sizeof (rTess.ModeTile), &nb);
	isave->Write(&rTess.KeepMapping, sizeof (rTess.KeepMapping), &nb);
	isave->Write(&rTess.TransitionType, sizeof (rTess.TransitionType), &nb);

	isave->Write(&selLevel, sizeof (selLevel), &nb);

	return IO_OK;
};

// *** Tile Methods

// Get the matrix of the selected tiles
Matrix3 RPatchMesh::GetSelTileTm(PatchMesh& patch, TimeValue t, INode *node, bool& bHasSel) const
{
	Interval valid;
	Box3 box;
	Matrix3 otm = node->GetObjectTM(t, &valid);
	Matrix3 tm = node->GetNodeTM(t, &valid);
	bHasSel = false;

	// For all patchs
	for (int nPatch=0; nPatch<(int)getUIPatchSize(); nPatch++)
	{
		float fV=0;
		int nBU=1<<getUIPatch (nPatch).NbTilesU;
		int nBV=1<<getUIPatch (nPatch).NbTilesV;
		float fDU=1.f/(float)nBU;
		float fDV=1.f/(float)nBV;
		for (int nV=0; nV<nBV; nV++)
		{
			float fU=0;
			for (int nU=0; nU<nBU; nU++)
			{
				int nTileNumber=GetTileNumber(nPatch, nU, nV);
				if (tileSel[nTileNumber])
				{
					bHasSel = true;
					box += patch.patches[nPatch].interp (&patch, fU, fV);
					box += patch.patches[nPatch].interp (&patch, fU+fDU, fV);
					box += patch.patches[nPatch].interp (&patch, fU+fDU, fV+fDV);
					box += patch.patches[nPatch].interp (&patch, fU, fV+fDV);
				}
				fU += fDU;
			}
			fV += fDV;
		}
	}
	tm.SetTrans(otm * box.Center());
	return tm;
}

// Get the center of the selected tiles
Point3 RPatchMesh::GetSelTileCenter(PatchMesh& patch, TimeValue t, INode *node, bool& bHasSel) const
{
	Interval valid;
	Box3 box;
	Matrix3 tm = node->GetObjectTM(t, &valid);
	bHasSel=false;

	// For all patchs
	for (int nPatch=0; nPatch<(int)getUIPatchSize(); nPatch++)
	{
		float fV=0;
		int nBU=1<<getUIPatch (nPatch).NbTilesU;
		int nBV=1<<getUIPatch (nPatch).NbTilesV;
		float fDU=1.f/(float)nBU;
		float fDV=1.f/(float)nBV;
		for (int nV=0; nV<nBV; nV++)
		{
			float fU=0;
			for (int nU=0; nU<nBU; nU++)
			{
				int nTileNumber=GetTileNumber(nPatch, nU, nV);
				if (tileSel[nTileNumber])
				{
					bHasSel=true;
					box += patch.patches[nPatch].interp (&patch, fU, fV) * tm;
					box += patch.patches[nPatch].interp (&patch, fU+fDU, fV) * tm;
					box += patch.patches[nPatch].interp (&patch, fU+fDU, fV+fDV) * tm;
					box += patch.patches[nPatch].interp (&patch, fU, fV+fDV) * tm;
				}
				fU += fDU;
			}
			fV += fDV;
		}
	}
	return box.Center();
}

// Hittest method
BOOL RPatchMesh::SubObjectHitTest (GraphicsWindow *gw, Material *ma, HitRegion *hr, DWORD flags, SubPatchHitList& hitList, 
								  TimeValue t, PatchMesh& patch)
{
	// Return value
	BOOL bRet=FALSE;

	// Build the mesh
	BuildMesh (t, patch);
	if (rTess.TileTesselLevel>=0)
	{
		nlassert (mesh.numFaces==(int)getMapHitSize ());

		for (int nf=0; nf<mesh.numFaces; nf++)
		{
			if (mesh.faceSel[nf])
				int toto=0;
		}

		// Hittest the mesh
		int nFlags=SUBHIT_FACES;
		if (flags&SUBHIT_PATCH_SELONLY)
		{
			nFlags|=SUBHIT_SELONLY;
		}
		if (flags&SUBHIT_PATCH_UNSELONLY)
			nFlags|=SUBHIT_UNSELONLY;
		if (flags&SUBHIT_PATCH_ABORTONHIT)
			nFlags|=SUBHIT_ABORTONHIT;
		if (flags&SUBHIT_PATCH_SELSOLID)
			nFlags|=SUBHIT_SELSOLID;

		// Mesh flags
		mesh.SetDispFlag (DISP_SELFACES);
		mesh.selLevel = MESH_FACE;

		// Hittest
		SubObjHitList list;
		gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		bRet=mesh.SubObjectHitTest (gw, ma, hr, nFlags, list);

		MeshSubHitRec *rec=list.First();
		while (rec)
		{
			if (flags&SUBHIT_PATCH_SELONLY)
			{
				int otot=0;
			}
			if (flags&SUBHIT_PATCH_UNSELONLY)
			{
				int otot=0;
			}
			// Check the hit
			nlassert (rec->index>=0);
			nlassert (rec->index<(int)getMapHitSize ());
			nlassert (rec->index<mesh.numFaces);

			// Remap the index
			int nRemapedIndex=remapTriangle (rec->index);

			// Add a patch hit
			hitList.AddHit (rec->dist, &patch, nRemapedIndex, PATCH_HIT_TILE);

			// Next hit
			rec=rec->Next();
		}
	}

	return bRet;
}

// Return the patch adjacent at nPatch in the edge nEdge. If no patch adjacent, return nPatch
int GetAdjacentPatch (int nPatch, int nEdge, PatchMesh *patch)
{
	PatchEdge *pEdge=patch->edges+patch->patches[nPatch].edge[nEdge];
#if (MAX_RELEASE < 4000)
	nlassert ((pEdge->patch1==nPatch)||(pEdge->patch2==nPatch));
	if (pEdge->patch1==nPatch)
		return (pEdge->patch2!=-1)?pEdge->patch2:nPatch;
	else
		return (pEdge->patch1!=-1)?pEdge->patch1:nPatch;
#else // (MAX_RELEASE < 4000)
	nlassert ((pEdge->patches.Count()>0&&pEdge->patches[0]==nPatch)||(pEdge->patches.Count()>1&&pEdge->patches[1]==nPatch));
	if (pEdge->patches[0]==nPatch)
		return (pEdge->patches.Count()>1&&pEdge->patches[1]!=-1)?pEdge->patches[1]:nPatch;
	else
		return (pEdge->patches.Count()>0&&pEdge->patches[0]!=-1)?pEdge->patches[0]:nPatch;
#endif // (MAX_RELEASE < 4000)
}

// Build the mesh
void RPatchMesh::BuildMesh(TimeValue t, PatchMesh& patch, Mesh *pMesh)
{

	// Check validity
	Point3		point;
	int			x,y;

	// Mesh pointer
	if (!pMesh)
		pMesh=&mesh;

	// Chech binding info are valid
	UpdateBinding (patch, t);
	nlassert (ValidBindingPos.InInterval (t));
	nlassert (ValidBindingInfo.InInterval (t));

	ChannelMask Build=0;

	if (!ValidTopo.InInterval (t))
	{
		Build|=PART_TOPO;
	}
	if (!ValidGeom.InInterval (t))
	{
		Build|=PART_GEOM;
	}
	if (!ValidTexmap.InInterval (t))
	{
		Build|=PART_TEXMAP;
	}
	if (!ValidSelect.InInterval (t))
	{
		Build|=PART_SELECT|PART_TOPO|PART_GEOM;
	}
	if (!ValidDisplay.InInterval (t))
	{
		Build|=PART_DISPLAY;
	}

	if (!Build)
		return;

	TTicks ticks=CTime::getPerformanceTime ();

#ifndef NDEBUG
	Validity (patch, true);
#endif // NDEBUG

	// Build tracking
	static int nBuildNumber=0;
	build=nBuildNumber++;

	// Textured mode ?
	bool		bTextured=false;
	if (rTess.TileTesselLevel>=0)
		// ok, there is at least two tri for a tile, so we can texture...
		bTextured=true;

	// Count channels
	int nChannelCount=0;
	BitArray pChannelBit (MAX_MESHMAPS);
	pChannelBit.ClearAll ();
	int nChannel;
	for (nChannel=0; nChannel<MAX_MESHMAPS; nChannel++)
	{
		if ((patch.getNumMapVerts(nChannel)>0)||((!rTess.KeepMapping)&&(nChannel==1)))
		{
			pChannelBit.Set (nChannel);
			nChannelCount=nChannel+1;
		}
	}

	// Show interior ?
	bool bShowInter=(patch.GetShowInterior()!=0);

	if (Build&(PART_TOPO|PART_GEOM))
	{
		int	nbdivsU,nbdivsV;
		int nf=0;
		int finalVertexCount=0;
		for(int p=0 ; p<patch.getNumPatches() ; p++)
		{
			if (patch.patches[p].type==PATCH_QUAD)
			{
				// Quads count in U and V
				GetPatchTess (p, nbdivsU, nbdivsV);

				// Quads count in U and V
				nf+=2*nbdivsU*nbdivsV;
				finalVertexCount+=(nbdivsV+1)*(nbdivsU+1);
			}
		}

		// Setup count
		if (Build|=PART_GEOM)
		{
			pMesh->setNumVerts(finalVertexCount,FALSE,TRUE);

			// Rebuild bind info
			//UpdateBindingInfo (patch);
		}
		if (Build|=PART_TOPO)
		{
			// Num of patch
			pMesh->setNumFaces(nf,FALSE,TRUE);
			if (bTextured)
				resizeMapHit (nf);

			// For each channel
			for (nChannel=0; nChannel<nChannelCount; nChannel++)
			{
				if (pChannelBit[nChannel])
				{
					// Activate this channel (mandatory for Max 4.2)
					pMesh->setMapSupport(nChannel, TRUE);

					if (rTess.KeepMapping||!bTextured||(nChannel!=1))
					{
						// Number of vertex
						pMesh->setNumMapVerts (nChannel, finalVertexCount);
					}
					else
					{
						// Number of vertex
						pMesh->setNumMapVerts (nChannel, (nf/2)*4);
					}

					// Num of patch
					pMesh->setNumMapFaces(nChannel, nf);
				}
			}
		}
	}

	if (Build&(PART_GEOM|PART_GEOM|PART_SELECT|PART_TEXMAP))
	{
		// First vertex of the two tri...
		int offset=0;

		// Next face index
		int f=0;

		// Next vertex index
		int nv=0;

		// Next mapping vertex index
		int mp=0;

		// First Edge
		int ne=0;

		// Clear the edges
		if (Build&PART_SELECT)
			pMesh->edgeSel.ClearAll ();

		// Tile level
		int nTileLevel=rTess.TileTesselLevel;

		// Count of vertex by tile
		int nVertexTileCount=(1<<nTileLevel)*2;

		// Corner coord
		Point3 vTt[4][5]=
		{	
			{ Point3 (0,1,0), Point3 (0,1,0), Point3 (0.f,0.5f,0.f), Point3 (0.5f,0.5f,0.f), Point3 (0.5f,1.f,0.f) },
			{ Point3 (0,0,0), Point3 (0.f,0.5f,0.f), Point3 (0,0,0), Point3 (0.5f,0.f,0.f), Point3 (0.5f,0.5f,0.f) },
			{ Point3 (1,0,0), Point3 (0.5f,0.5f,0.f), Point3 (0.5f,0.f,0.f), Point3 (1.f,0.f,0.f), Point3 (1.f,0.5f,0.f) },
			{ Point3 (1,1,0), Point3 (0.5f,1.f,0.f), Point3 (0.5f,0.5f,0.f), Point3 (1.f,0.5f,0.f), Point3 (1.f,1.f,0.f) } 
		};
		for(int p=0 ; p<patch.getNumPatches() ; p++)
		{
			// Check it's a quad patch
			if (patch.patches[p].type==PATCH_QUAD)
			{
				int	nbdivsU, nbdivsV;

				// Tessel in U and V
				GetPatchTess (p, nbdivsU, nbdivsV);

				// Delta for mapping
				float fUd=1.f/(float)nbdivsU;
				float fVd=1.f/(float)nbdivsV;

				// Delta for mapping
				float fTiled=1.f/(float)(1<<nTileLevel);

				// Count of vertex in the patch
				int nUCount=nbdivsU+1;
				int nVCount=nbdivsV+1;

				// Number of tile
				int nTileCountU=1<<getUIPatch (p).NbTilesU;
				int nTileCountV=1<<getUIPatch (p).NbTilesV;

				// Nb face in a tile
				int nTess=1<<rTess.TileTesselLevel;
				
				// Vertices have changed
				if (Build&(PART_GEOM|PART_SELECT))
				{
					int nV;
					if (Build&PART_GEOM)
					{
						float oonbdivsU=1.0f/(float)nbdivsU;
						float oonbdivsV=1.0f/(float)nbdivsV;

						// Compute the vertices
						for(nV=0 ; nV<=nbdivsV; ++nV)
						{
							float v=(float)nV*oonbdivsV;
							for(int nU=0 ; nU<=nbdivsU ; nU++)
							{
								float u=(float)nU*oonbdivsU;
								point=patch.patches[p].interp(&patch,u,v);
								pMesh->setVert(nv, point);
								nv++;
							}
						}
					}

					// Vertices selection
					if (Build&PART_SELECT)
					{
						pMesh->vertSel.Set ( offset, patch.vertSel[patch.patches[p].v[0]]);
						pMesh->vertSel.Set ( offset+nbdivsV*(nbdivsU+1), patch.vertSel[patch.patches[p].v[1]]);
						pMesh->vertSel.Set ( offset+nbdivsV*(nbdivsU+1)+nbdivsU, patch.vertSel[patch.patches[p].v[2]]);
						pMesh->vertSel.Set ( offset+nbdivsU, patch.vertSel[patch.patches[p].v[3]]);
					}
				}

				if (Build&(PART_TOPO|PART_SELECT|PART_TEXMAP))
				{
					if (Build&PART_SELECT)
					{
						// Hide all verts
						int nLastVert=offset+(nbdivsV+1)*(nbdivsU+1);
						for (int tt=offset; tt<nLastVert; tt++)
						{
							pMesh->vertHide.Set (tt);
						}

						// Show some of them
						pMesh->vertHide.Clear (offset);
						pMesh->vertHide.Clear (offset+nbdivsU);
						pMesh->vertHide.Clear (nLastVert-1-nbdivsU);
						pMesh->vertHide.Clear (nLastVert-1);

						// Edge selection
						int ne=f*3;
						if (GetSelLevel()==EP_PATCH)
						{
							int bPatchSel=patch.patchSel[p];
							int nUV=nbdivsU*nbdivsV;
							if (bPatchSel||patch.patchSel[GetAdjacentPatch (p, 3, &patch)])
							{
								// Top
								for (int ee=0; ee<nbdivsU; ee++)
									pMesh->edgeSel.Set (ne+6*ee+2);
							}
							if (bPatchSel||patch.patchSel[GetAdjacentPatch (p, 1, &patch)])
							{
								// Bottom
								for (int ee=nUV-nbdivsU; ee<nUV; ee++)
									pMesh->edgeSel.Set (ne+6*ee+3);
							}
							if (bPatchSel||patch.patchSel[GetAdjacentPatch (p, 0, &patch)])
							{
								// Left
								for (int ee=0; ee<nbdivsV; ee++)
									pMesh->edgeSel.Set (ne+6*nbdivsU*ee);
							}
							if (bPatchSel||patch.patchSel[GetAdjacentPatch (p, 2, &patch)])
							{
								// Right
								for (int ee=0; ee<nbdivsV; ee++)
									pMesh->edgeSel.Set (ne+6*nbdivsU*ee+4+6*(nbdivsU-1));
							}
						}
						else if ((GetSelLevel()==EP_EDGE)||paint)
						{
							// Top
							if (patch.edgeSel[patch.patches[p].edge[3]])
							{
								for (int ee=0; ee<nbdivsU; ee++)
									pMesh->edgeSel.Set(ne+6*ee+2);
							}
							// Bottom
							if (patch.edgeSel[patch.patches[p].edge[1]])
							{
								int nUV=nbdivsU*nbdivsV;
								for (int ee=nUV-nbdivsU; ee<nUV; ee++)
									pMesh->edgeSel.Set(ne+6*ee+3);
							}
							// Left
							if (patch.edgeSel[patch.patches[p].edge[0]])
							{
								for (int ee=0; ee<nbdivsV; ee++)
									pMesh->edgeSel.Set(ne+6*nbdivsU*ee);
							}
							// Right
							if (patch.edgeSel[patch.patches[p].edge[2]])
							{
								for (int ee=0; ee<nbdivsV; ee++)
									pMesh->edgeSel.Set(ne+6*nbdivsU*ee+4+6*(nbdivsU-1));
							}
						}
					}

					// Topology has changed
					int mpcopy=mp;
					int offsetcopy=offset;
					for(y=0 ; y<nbdivsV ; y++)
					{
						for(x=0 ; x<nbdivsU ; x++)
						{
							if (Build&(PART_TOPO|PART_SELECT))
							{
								pMesh->faces[f].v[0]=offset+x;
								pMesh->faces[f].v[1]=offset+x+(nbdivsU+1);
								pMesh->faces[f].v[2]=offset+x+1;
								pMesh->faces[f].flags&=~(EDGE_ALL);
								if (bShowInter)
									pMesh->faces[f].flags|=EDGE_A|EDGE_C;
								if (x==0)
									pMesh->faces[f].flags|=EDGE_A;
								if (y==0)
									pMesh->faces[f].flags|=EDGE_C;
								pMesh->faces[f].smGroup=1;

								pMesh->faces[f+1].v[0]=offset+x+(nbdivsU+1);
								pMesh->faces[f+1].v[1]=offset+x+(nbdivsU+1)+1;
								pMesh->faces[f+1].v[2]=offset+x+1;
								pMesh->faces[f+1].flags&=~(EDGE_ALL);
								if (bShowInter)
									pMesh->faces[f+1].flags|=EDGE_A|EDGE_B;
								if (x==nbdivsU-1)
									pMesh->faces[f+1].flags|=EDGE_B;
								if (y==nbdivsV-1)
									pMesh->faces[f+1].flags|=EDGE_A;
								pMesh->faces[f+1].smGroup=1;

								// MatId
								/*pMesh->faces[f].setMatID((abs(f>>1)%3));
								pMesh->faces[f+1].setMatID((abs(f>>1)%3));*/
								// Tile number

								// Face display
								if (patch.patches[p].flags&PATCH_HIDDEN)
								{
									pMesh->faces[f].flags|=FACE_HIDDEN;
									pMesh->faces[f+1].flags|=FACE_HIDDEN;
								}
								else
								{
									pMesh->faces[f].flags&=~(FACE_HIDDEN);
									pMesh->faces[f+1].flags&=~(FACE_HIDDEN);
								}

								// Texture maps
								int nTileU=x>>nTileLevel;
								int nTileV=y>>nTileLevel;
								for (nChannel=0; nChannel<nChannelCount; nChannel++)
								{
									if (pChannelBit[nChannel])
									{
										TVFace *pTvP=pMesh->mapFaces(nChannel);
										nlassert (pTvP);
										if (pTvP)
										{
											pTvP+=f;
											if (rTess.KeepMapping||!bTextured||(nChannel!=1))
											{
												pTvP->t[0]=offset+x;
												pTvP->t[1]=offset+x+(nbdivsU+1);
												pTvP->t[2]=offset+x+1;
												pTvP++;
												pTvP->t[0]=offset+x+(nbdivsU+1);
												pTvP->t[1]=offset+x+(nbdivsU+1)+1;
												pTvP->t[2]=offset+x+1;
											}
											else
											{
												int nX2=x*2;
												pTvP->t[0]=mp+nX2;
												pTvP->t[1]=mp+nX2+nbdivsU*2;
												pTvP->t[2]=mp+nX2+1;
												pTvP++;
												pTvP->t[0]=mp+nX2+nbdivsU*2;
												pTvP->t[1]=mp+nX2+nbdivsU*2+1;
												pTvP->t[2]=mp+nX2+1;
											}
										}
									}
								}
							}
							
							if (Build&PART_TEXMAP)
							{
								int uTile=x>>nTileLevel;
								int vTile=y>>nTileLevel;

								uint nTileNumber=0;
								tileDesc& desc=getUIPatch (p).getTileDesc (uTile+vTile*nTileCountU);
								if (!desc.isEmpty ())
								{
									nTileNumber=desc.getLayer(std::min (desc.getNumLayer()-1, rTess.TransitionType-1)).Tile+1;
								}
								pMesh->faces[f].setMatID(nTileNumber);
								pMesh->faces[f+1].setMatID(nTileNumber);
							}

							if (Build&PART_SELECT)
							{
								// Remap for hittest
								if (bTextured)
								{
									int nU=x>>rTess.TileTesselLevel;
									int nV=y>>rTess.TileTesselLevel;
									int tileNumber=GetTileNumber(p, nU, nV);
									setRemapEntry (f, tileNumber);
									setRemapEntry (f+1, tileNumber);
									if (GetSelLevel()==EP_TILE)
									{
										int bTileSel=tileSel[tileNumber];
										if (bTileSel)
										{
											// Top
											if ((y&(nTess-1))==0)
												pMesh->edgeSel.Set(3*f+2);
											// Left
											if ((x&(nTess-1))==0)
												pMesh->edgeSel.Set(3*f);
											// Bottom
											if ((y&(nTess-1))==(nTess-1))
												pMesh->edgeSel.Set(3*f+3);
											// Right
											if ((x&(nTess-1))==(nTess-1))
												pMesh->edgeSel.Set(3*f+4);
										}

										// Face selection
										pMesh->faceSel.Set (f, bTileSel);
										pMesh->faceSel.Set (f+1, bTileSel);
									}
								}
							}
							f+=2;
						}
						offset+=nbdivsU+1;
						mp+=nbdivsU*4;
					}
					offset+=nbdivsU+1;

					// Generate texture vertices
					if ((Build&PART_TOPO)|(Build&PART_TEXMAP))
					{
						// Copy mapping
						// For each channel
						for (nChannel=0; nChannel<nChannelCount; nChannel++)
						{
							if (pChannelBit[nChannel])
							{
								if (rTess.KeepMapping||!bTextured||(nChannel!=1))
								{
									Point3 vT[4];
									if (rTess.KeepMapping||(nChannel!=1))
									{
										for (int ii=0; ii<4; ii++)
											vT[ii]=patch.getMapVert (nChannel, patch.tvPatches[nChannel][p].tv[ii]);
									}
									else
									{
										for (int ii=0; ii<4; ii++)
											vT[ii]=Point3 (0,0,0);
									}
									// For each vertex
									float fV=0.f;
									for (int v=0; v<nVCount; v++)
									{
										float fU=0.f;
										for (int u=0; u<nUCount; u++)
										{
											Point3 vRight=(vT[(2)&3]-vT[(3)&3])*fV+vT[(3)&3];
											Point3 vLeft=(vT[(1)&3]-vT[(0)&3])*fV+vT[(0)&3];
											Point3 vPoint=(vRight-vLeft)*fU+vLeft;
											pMesh->setMapVert (nChannel, offsetcopy+u+v*nUCount, vPoint);
											fU+=fUd;
										}
										fV+=fVd;
									}
								}
								else
								{
									// For each tile
									for (int vTile=0; vTile<nTileCountV; vTile++)
									{
										for (int uTile=0; uTile<nTileCountU; uTile++)
										{
											// For each vertices
											for (int v=0; v<nVertexTileCount; v++)
											{
												float fV=(float)((v+1)>>1)/(float)(nVertexTileCount>>1);
												for (int u=0; u<nVertexTileCount; u++)
												{
													// rotate
													tileDesc &desc=getUIPatch (p).getTileDesc (uTile+vTile*nTileCountU);
													int nRotate=4-desc.getLayer(std::min (desc.getNumLayer()-1, rTess.TransitionType-1)).Rotate;
													int nCase=desc.getCase();
													float fU=(float)((u+1)>>1)/(float)(nVertexTileCount>>1);
													int nIndex=mpcopy+vTile*nVertexTileCount*nVertexTileCount*nTileCountU+v*nVertexTileCount*nTileCountU+
														uTile*nVertexTileCount+u;
													Point3 vRight=(vTt[(2+nRotate)&3][nCase]-vTt[(3+nRotate)&3][nCase])*fV+vTt[(3+nRotate)&3][nCase];
													Point3 vLeft=(vTt[(1+nRotate)&3][nCase]-vTt[(0+nRotate)&3][nCase])*fV+vTt[(0+nRotate)&3][nCase];
													Point3 vPoint=(vRight-vLeft)*fU+vLeft;
													pMesh->setMapVert (nChannel, nIndex, vPoint);
												}
											}
										}
									}
								}
							}
						}
					}						
				}
			}
		}
	}
	if (Build&PART_TOPO)
	{
		pMesh->InvalidateTopologyCache();
		//pMesh->BuildStripsAndEdges();
	}
	if (Build&PART_GEOM)
	{
		pMesh->InvalidateGeomCache();
	}
	ValidGeom=FOREVER;
	ValidTopo=FOREVER;
	ValidTexmap=FOREVER;
	ValidSelect=FOREVER;
	ValidDisplay=FOREVER;

	for (int nf=0; nf<mesh.numFaces; nf++)
	{
		if (mesh.faceSel[nf])
			int toto=0;
	}

	ticks=CTime::getPerformanceTime ()-ticks;
	nldebug ("%f", CTime::ticksToSecond(ticks));
}

// Get tessel level of a patch
void RPatchMesh::GetPatchTess (int nPatch, int& nUTess, int& nVTess)
{
	// Check size
	nlassert (nPatch<(int)getUIPatchSize());

	// Return tessel level depending tessel of the pathmesh and count of tile in the patch
	nUTess=1<<max (0, rTess.TileTesselLevel+getUIPatch (nPatch).NbTilesU);
	nVTess=1<<max (0, rTess.TileTesselLevel+getUIPatch (nPatch).NbTilesV);
}

// Display
int RPatchMesh::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, PatchMesh& patch)
{
	//TODO: Implement the displaying of the object here

	// Update the mesh
	BuildMesh (t, patch);

	// Draw the mesh
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);
	
	// Becarful with sublevel selection
	mesh.dispFlags=0;
	switch (GetSelLevel())
	{
	case EP_VERTEX:
		mesh.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS);
		mesh.selLevel = MESH_VERTEX;
		break;
	case EP_EDGE:
	case EP_PATCH:
	case EP_TILE:
		mesh.SetDispFlag(DISP_SELEDGES);
		mesh.selLevel = MESH_EDGE;
		break;
	default:
		mesh.SetDispFlag(0);
		mesh.selLevel = MESH_OBJECT;
		break;
	}
	if (paint)
	{
		mesh.SetDispFlag(DISP_SELEDGES);
		mesh.selLevel = MESH_EDGE;
	}

	// Go
	mesh.render( gw, inode->Mtls(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL | ((flags&DISP_SHOWSUBOBJECT)?COMP_OBJSELECTED:0),
		inode->NumMtls());

	// Draw vector
	if ((GetSelLevel()==EP_VERTEX)&&(inode->Selected()))
	{
		UpdateBinding (patch, t);

		// Set new flags
		patch.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS);
		patch.selLevel=PATCH_VERTEX;

		DWORD dw=gw->getRndLimits();
		BitArray bit;
		bit.SetSize (patch.numPatches);
		bit.ClearAll ();
		if ((dw&GW_PICK)==0)
		{
			// Hide all patch
			for (int nP=0; nP<patch.numPatches; nP++)
			{
				if (patch.patches[nP].flags&PATCH_HIDDEN)
					bit.Set (nP);
				patch.patches[nP].flags|=PATCH_HIDDEN;
			}
		}

		patch.render( gw, inode->Mtls(),
			(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL | ((flags&DISP_SHOWSUBOBJECT)?COMP_OBJSELECTED:0),
			inode->NumMtls());

		if ((dw&GW_PICK)==0)
		{
			// Hide all patch
			for (int nP=0; nP<patch.numPatches; nP++)
			{
				if (!bit[nP])
					patch.patches[nP].flags&=~PATCH_HIDDEN;
			}
		}
	}

	// Point binded
	if (inode->Selected())
	{
		gw->setColor(LINE_COLOR, 0, 0, 0);
		for (int i=0; i<patch.numVerts; i++)
		{
			if (getUIVertex (i).Binding.bBinded)
			{
				// draw the point
				gw->marker (&patch.verts[i].p, DOT_MRKR);
			}
		}
	}

	//     A
	//     -
	//   B   C
	//     -
	//     -
	//     D
	// Draw the arrow
	if (GetSelLevel()==EP_PATCH)
	{
		for (int i=0; i<patch.numPatches; i++)
		{
			if (patch.patches[i].type==PATCH_QUAD)
			{
				Point3 a,b,c,d;
				Point3 p[2];

				// 4 points
				a=patch.patches[i].interp(&patch,0.5f,1.f/6.f);
				b=patch.patches[i].interp(&patch,0.5f-1.f/6.f,1.f/3.f);
				c=patch.patches[i].interp(&patch,0.5f+1.f/6.f,1.f/3.f);
				d=patch.patches[i].interp(&patch,0.5f,5.f/6.f);

				// draw the point
				p[0]=a;
				p[1]=b;
				gw->polyline (2, p, NULL, NULL, FALSE, NULL);
				p[1]=c;
				gw->polyline (2, p, NULL, NULL, FALSE, NULL);
				p[1]=d;
				gw->polyline (2, p, NULL, NULL, FALSE, NULL);
			}
		}
	}
	
	return 0;
}

// Return a tile desc
tileDesc& RPatchMesh::getTileDesc (int nTile)
{
	int patch=nTile/NUM_TILE_SEL;
	int tile=nTile%NUM_TILE_SEL;
	int tileY=tile/MAX_TILE_IN_PATCH;
	int tileX=tile%MAX_TILE_IN_PATCH;
	tile=tileY*(1<<getUIPatch (patch).NbTilesU)+tileX;
	return getUIPatch (patch).getTileDesc (tile);
}

// Set a tile desc
void RPatchMesh::setTileDesc (int nTile, const tileDesc& desc)
{
	int patch=nTile/NUM_TILE_SEL;
	int tile=nTile%NUM_TILE_SEL;
	int tileY=tile/MAX_TILE_IN_PATCH;
	int tileX=tile%MAX_TILE_IN_PATCH;
	tile=tileY*(1<<getUIPatch (patch).NbTilesU)+tileX;
	getUIPatch (patch).getTileDesc (tile)=desc;
}

// Turn selected patch
void RPatchMesh::TurnPatch(PatchMesh *patch)
{
	// For each patch
	for (int p=0; p<patch->numPatches; p++)
	{
		// Selected ?
		if (patch->patchSel[p])
		{
			// Tessel U and V
			int	nOldU=1<<(getUIPatch (p).NbTilesU);
			int	nOldV=1<<(getUIPatch (p).NbTilesV);

			// Reverse UV count
			int tmp=getUIPatch (p).NbTilesU;
			getUIPatch (p).NbTilesU=getUIPatch (p).NbTilesV;
			getUIPatch (p).NbTilesV=tmp;

			// Copy old array
			UI_PATCH				old=getUIPatch (p);

			// Turn tile array
			int u, v;
			for (v=0; v<nOldV; v++)
			for (u=0; u<nOldU; u++)
			{
				int newU=nOldV-v-1;
				int newV=u;
				getUIPatch (p).getTileDesc (newU+newV*nOldV)=old.getTileDesc (u+v*nOldU);

				// Rotate each layer
				getUIPatch (p).getTileDesc (newU+newV*nOldV).rotate (3);
			}

			// Turn vertex color

			// Tessel U and V
			nOldU++;
			nOldV++;
			
			// Turn tile array
			for (v=0; v<nOldV; v++)
			for (u=0; u<nOldU; u++)
			{
				int newU=nOldV-v-1;
				int newV=u;
				getUIPatch (p).setColor (newU+newV*nOldV, old.getColor (u+v*nOldU));
			}
		}
	}

	// Check binding infos
	for (int v=0; v<patch->numVerts; v++)
	{
		// Binded ?
		if (getUIVertex (v).Binding.bBinded)
		{
			// On a patch turned ?
			int nPatch=getUIVertex (v).Binding.nPatch;
			if (patch->patchSel[nPatch])
			{
				// Ok, turn the bind info..
				getUIVertex (v).Binding.nEdge--;
				getUIVertex (v).Binding.nEdge&=3;
			}
		}
	}
}

// Turn selected patch
void RPatchMesh::RotateTiles (PatchMesh *patch, int rot)
{
	// For each patch
	for (int p=0; p<patch->numPatches; p++)
	{
		// Tessel U and V
		int	nU=1<<(getUIPatch (p).NbTilesU);
		int	nV=1<<(getUIPatch (p).NbTilesV);

		// Turn tile array
		for (int v=0; v<nV; v++)
		for (int u=0; u<nU; u++)
		{
			// Rotate each layer
			getUIPatch (p).getTileDesc (u+v*nU).rotate (rot);
		}
	}
}

// Turn selected patch
/*void RPatchMesh::flipTilesUpDown (PatchMesh *patch)
{
	// For each patch
	for (int p=0; p<patch->numPatches; p++)
	{
		// Tessel U and V
		int	nU=1<<(getUIPatch (p).NbTilesU);
		int	nV=1<<(getUIPatch (p).NbTilesV);

		// Copy old array
		UI_PATCH				old=getUIPatch (p);

		// Turn tile array
		for (int v=0; v<nV; v++)
		for (int u=0; u<nU; u++)
		{
			// Rotate each layer
			getUIPatch (p).getTileDesc (u+v*nU) = old.getTileDesc (u+(nV-1-v)*nU);
			getUIPatch (p).getTileDesc (u+v*nU).flipUpDown ();
		}
	}
}*/

// Copy operator
#ifdef USE_CACHE
void RPatchMesh::operator= (const RPatchMesh& fromOb)
{
	DeepCopy (const_cast<RPatchMesh*>(&fromOb), PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO|TEXMAP_CHANNEL);
}
#endif // USE_CACHE

CBankManager RPatchMesh::manager;
