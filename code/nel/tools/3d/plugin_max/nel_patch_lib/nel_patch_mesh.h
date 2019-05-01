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

#ifndef __RYKOL_PATCH_MESH_H
#define __RYKOL_PATCH_MESH_H

#pragma warning (disable : 4786)
#include "nel/misc/types_nl.h"

#include <windows.h>
#include <vector>
#include <set>
#include <string>

#include "nel/misc/debug.h"
#include "nel/3d/tile_bank.h"
#include "nel/misc/file.h"
#include "nel/misc/rgba.h"
#include "path_mesh_alloc.h"
#include "../nel_3dsmax_shared/string_common.h"

//#define USE_CACHE

namespace NL3D
{
class CZone;
class CZoneSymmetrisation;
};

typedef unsigned int uint;

#define RYKOLPATCHOBJ_CLASS_ID	Class_ID(0x368c679f, 0x711c22ee)

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

extern ClassDesc* GetRPODesc();

#define RPATCHMESH_SERIALIZE_VERSION_9 9
#define RPATCHMESH_SERIALIZE_VERSION_8 8
#define RPATCHMESH_SERIALIZE_VERSION_7 7
#define RPATCHMESH_SERIALIZE_VERSION_6 6
#define RPATCHMESH_SERIALIZE_VERSION_5 5
#define RPATCHMESH_SERIALIZE_VERSION_4 4
#define RPATCHMESH_SERIALIZE_VERSION_3 3
#define RPATCHMESH_SERIALIZE_VERSION_2 2
#define RPATCHMESH_SERIALIZE_VERSION_1 1
#define RPATCHMESH_SERIALIZE_VERSION RPATCHMESH_SERIALIZE_VERSION_9

#define EP_OBJECT	0
#define EP_VERTEX	1
#define EP_EDGE		2
#define EP_PATCH	3
#define EP_TILE		4

#define PO_TILE		4

#define PATCH_HIT_TILE (PATCH_HIT_INTERIOR+1)

#define MAX_TILE_IN_PATCH 16
#define NUM_TILE_SEL (MAX_TILE_IN_PATCH*MAX_TILE_IN_PATCH)

#define NEL3D_APPDATA_ZONE_ROTATE		1266703978
#define NEL3D_APPDATA_ZONE_SYMMETRY		1266703979

#pragma warning (disable : 4786)

// ------------------------------------------------------------------------------------------------------------------------------------------------

/*

  Here the user infos (UI) for face, edge, vertex, vertex-face.
  All these infos are stored in one class (RPO_UI) through vectors 


*/

class CVertexNeighborhood;

int CheckBind (int nVert, int nSeg, int& v0, int& v1, int& v2, int& v3, const CVertexNeighborhood& tab, const PatchMesh& patch, bool bAssert, bool bCreate);
std::string GetBankPathName ();
int GetBankTileSetSet ();
void SetBankPathName (const std::string& path);
void SetBankTileSetSet (int);
int WhereIsTheEdge (int nPatch, int nEdge, const PatchMesh& patch);

extern NL3D::CTileBank bank;

#define RPO_DEFAULT_TESSEL 4

enum typeBind { BIND_25=0, BIND_75, BIND_50, BIND_SINGLE, BIND_COUNT, BIND_ALIGN=0xffffffff };

extern float bindWhere[BIND_COUNT];

class bindingDesc
{
public:
	uint8			bBinded;			// true, this vertex is binded, false, is not. default 0
	uint8			nType;				// Type of the vertex
	uint16			nPatch;				// # of the patch on which the vertex is binded. Valid only if bBinded==true.
	uint16			nEdge;				// # of the edge in the patch on which the vertex is binded. Valid only if bBinded==true.
	uint16			nPrimVert;			// # of the primary vertex in this bind
	uint16			nBefore;			// # of the before tangant
	uint16			nBefore2;			// # of the before tangant
	uint16			nAfter;				// # of the after tangant
	uint16			nAfter2;			// # of the after tangant
	uint16			nT;					// # of the tangant of the binded edge
	uint16			fnslmq;
	//float			fWhere;				// Where on the edge the vertex is binded. Value must be 0.25f, 0.5f or 0.75f.
										// Valid only if bBinded==true.
};

class tileIndex
{
public:
	tileIndex ()
	{}
	tileIndex (int tile, int rotate)
	{
		Tile=tile;
		Rotate=rotate;
	}
	uint Tile:16;
	int Rotate:8;
};

class tileDesc
{
#define CASE_MASK 0x0007
#define DISPLACE_SHIFT 3
#define DISPLACE_COUNT 16
#define DISPLACE_MASK ((DISPLACE_COUNT-1)<<DISPLACE_SHIFT)
	friend class RPatchMesh;
public:
	tileDesc ()
	{
		setEmpty ();
	}
	void setTile (int num, int ncase, int displace, tileIndex tile0, tileIndex tile1, tileIndex tile2)
	{
		_Num=num;
		_MatIDTab[0]=tile0;
		_MatIDTab[1]=tile1;
		_MatIDTab[2]=tile2;
		setCase (ncase);
		setDisplace (displace);
	}
	tileIndex& getLayer (int num)
	{
		return _MatIDTab[num];
	}
	const tileIndex& getLayer (int num) const
	{
		return _MatIDTab[num];
	}
	int getNumLayer () const
	{
		return _Num;
	}
	void setEmpty ()
	{
		_Num=0;
		_Flags=0;
	}
	bool isEmpty ()
	{
		return _Num==0;
	}
	void rotate (int nRotate)
	{
		for (int i=0; i<3; i++)
		{
			_MatIDTab[i].Rotate+=nRotate;
			_MatIDTab[i].Rotate&=3;
		}
	}
	int getCase() const
	{
		return _Flags&CASE_MASK;
	}
	void setCase(int nCase)
	{
		nlassert ((nCase>=0)&&(nCase<5));
		_Flags&=~CASE_MASK;
		_Flags|=nCase;
	}
	uint8 getDisplace () const
	{
		return (uint8)((_Flags&DISPLACE_MASK)>>DISPLACE_SHIFT);
	}
	void setDisplace (uint8 nDisplace)
	{
		nlassert ((nDisplace>=0)&&(nDisplace<DISPLACE_COUNT));
		_Flags&=~DISPLACE_MASK;
		_Flags|=(nDisplace<<DISPLACE_SHIFT);
	}
private:
	tileIndex _MatIDTab[3];
	USHORT	_Num;
	USHORT	_Flags;
};

class UI_VERTEX
{
public:
	bindingDesc		Binding;			// Binding struct for the vertex
	void Init ()
	{
		Binding.bBinded=false;
	}
};

// Edge flags for no smoothing
#define UI_EDGE_FLAGS_NO_SMOOTH_MASK	0x1

// User info for edge
class CEdgeInfo
{
public:
	// Default Ctor
	CEdgeInfo ()
	{
		// No flags 
		Flags=0;
	}

	// Flags for a edge
	uint32	Flags;
};

class UI_PATCH
{
public:
	int					NbTilesU;		// Default = 3 (2^3 = 8)
	int					NbTilesV;		// Default = 3 (2^3 = 8)

private:
	// Tabl for tile number ( size: (2^NbTilesU) * (2^NbTilesV) ) default, all 0
	tileDesc			Tile[16*16];

	// Tabl for color on tile ( size: ((2^NbTilesU)+1) * ((2^NbTilesV)+1) ) default, all 0xffffffff. Color in 32 bits ARGB.
	uint				Colors[17*17];

	// Info by edges
	CEdgeInfo			_Edges[4];
public:
	// Return  a tileDesc ref
	tileDesc& getTileDesc (uint iD)
	{
		// Debug
		nlassert (iD<getTileSize ());

		// Return the ref
		return Tile[iD];
	}

	// Return a const tileDesc ref
	const tileDesc& getTileDesc (uint iD) const
	{
		// Debug
		nlassert (iD<getTileSize ());

		// Return the ref
		return Tile[iD];
	}

	// Return  a color ref
	uint getColor (uint iD) const
	{
		// Debug
		nlassert (iD<getColorSize ());

		// Return the ref
		return Colors[iD];
	}

	// Return a const tileDesc ref
	void setColor (uint iD, uint newColor)
	{
		// Debug
		nlassert (iD<getColorSize ());

		// Return the ref
		Colors[iD]=newColor;
	}

	// Return the size of the Tile array
	uint getTileSize () const
	{
		return (1<<NbTilesU)*(1<<NbTilesV);
	}

	// Return the size of the Color array
	uint getColorSize () const
	{
		return ((1<<NbTilesU)+1)*((1<<NbTilesV)+1);
	}

	// Return edge flags
	bool getEdgeFlag (uint edge)
	{
		nlassert ((edge>=0)&&(edge<4));
		return (_Edges[edge].Flags&UI_EDGE_FLAGS_NO_SMOOTH_MASK)!=0;
	}

	// Set edge flags
	void setEdgeFlag (uint edge, bool flags)
	{
		nlassert ((edge>=0)&&(edge<4));
		// Erase and set the flag
		_Edges[edge].Flags&=~UI_EDGE_FLAGS_NO_SMOOTH_MASK;
		_Edges[edge].Flags|=(uint32)flags;
	}

	// Get edge
	CEdgeInfo& getEdge (uint edge)
	{
		return _Edges[edge];
	}

	// Get edge
	const CEdgeInfo& getEdge (uint edge) const
	{
		return _Edges[edge];
	}
public:
	void Init (int nU=RPO_DEFAULT_TESSEL, int nV=RPO_DEFAULT_TESSEL, bool bKeep=false)
	{
		// Copy old patch infos
		UI_PATCH old=*this;

		// New size
		int nOldU=old.NbTilesU;
		int nOldV=old.NbTilesV;
		int nNewU=1<<nU;
		int nNewV=1<<nV;
		NbTilesU=nU;
		NbTilesV=nV;
		int nTileCount=(1<<NbTilesU)*(1<<NbTilesV);
		int nVertexCount=((1<<NbTilesU)+1)*((1<<NbTilesV)+1);

		// Keep old infos
		if (bKeep)
		{
			// Copy old coord...
			int i,j;
			int nMinU=std::min (nOldU, nNewU);
			int nMinV=std::min (nOldV, nNewV);
			for (j=0; j<nMinV; j++)
			{
				for (i=0; i<nMinU; i++)
				{
					Tile[i+j*nNewU]=old.getTileDesc (i+j*nOldU);
				}
				for (; i<nNewU; i++)
				{
					Tile[i+j*nNewU].setEmpty ();
				}
			}
			for (; j<nNewV; j++)
			{
				for (i=0; i<nNewU; i++)
				{
					Tile[i+j*nNewU].setEmpty ();
				}
			}
			for (j=0; j<nMinV+1; j++)
			{
				for (i=0; i<nMinU+1; i++)
				{
					Colors[i+j*(nNewU+1)]=old.getColor (i+j*(nOldU+1));
				}
				for (; i<nNewU+1; i++)
				{
					Colors[i+j*nNewU]=0xffffff;
				}
			}
			for (; j<nNewV+1; j++)
			{
				for (i=0; i<nMinU+1; i++)
				{
					Colors[i+j*nNewU]=0xffffff;
				}
			}
		}
		else
		{
			// Init new coord
			int j;
			for(j=0; j<nTileCount; j++)
			{
				Tile[j].setEmpty ();
			}
			for(j=0; j<nVertexCount; j++)
			{
				Colors[j]=0xffffff;
			}
		}
	}
};

class CPatchAllocator
{
public:
	CPatchAllocator () : AllocPatch (100), AllocVertex (100), AllocInt (0) { }
	CPathMeshAlloc<UI_PATCH>	AllocPatch;		// 100 patch by mesh
	CPathMeshAlloc<UI_VERTEX>	AllocVertex;		// 100 vertices by mesh
	CPathMeshAlloc<int>			AllocInt;
};

struct RPOTess
{
	int				TileTesselLevel;
	bool			ModeTile;
	bool			KeepMapping;
	int				TransitionType;
};

class CBankManager
{
public:
	CBankManager ()
	{
	}

	const NL3D::CTileBank& getBank (std::string& path=GetBankPathName ())
	{
		if (path!=_lastPath)
		{
			try
			{
				NLMISC::CIFile file;
				if (file.open (path))
				{
					_bank.clear();
					_bank.serial (file);
				}
			}
			catch (const NLMISC::EStream& excp)
			{
				MessageBox (NULL, MaxTStrFromUtf8(excp.what()).data(), _T("Load error"), MB_OK|MB_ICONEXCLAMATION);
			}
		}
		return _bank;
	}
private:
	NL3D::CTileBank	_bank;
	std::string _lastPath;
};

// Class container of data with copy operator
class CPatchMeshData
{
public:
	// Default constructor, allocate the array
	CPatchMeshData ();

	// Copy constructor, allocate the array
	CPatchMeshData (const CPatchMeshData& src);
	// Destructor
	~CPatchMeshData ();

	// Copy
	CPatchMeshData& operator= (const CPatchMeshData& src);

	// The pointers
	std::vector<UI_PATCH>	*_UIPatch;
	std::vector<UI_VERTEX>	*_UIVertex;
	std::vector<int>		*_MapHitToTileIndex;
};

class RPatchMesh
{
	friend class RPO;
public:
	RPatchMesh ();
	~RPatchMesh ();

	// Info per patch
	
private:
	CPatchMeshData		_Data;
private:
	// Remap the map hit size
	void resizeMapHit (uint size)
	{
		_Data._MapHitToTileIndex->resize (size);
	}

	// Remap the map hit size
	void setRemapEntry (uint iD, uint remap)
	{
		(*_Data._MapHitToTileIndex)[iD]=remap;
	}

	// Resize the user info size
	void resizeUIPatch (uint size)
	{
		_Data._UIPatch->resize (size);
	}

	// Resize the user info size
	void resizeUIVertex (uint size)
	{
		_Data._UIVertex->resize (size);
	}
public:
	// Get map hit size
	uint getMapHitSize () const
	{
		return (uint)_Data._MapHitToTileIndex->size ();
	}

	// Remap a triangle
	uint remapTriangle (uint iD) const
	{
		nlassert (iD<getMapHitSize ());
		return (*_Data._MapHitToTileIndex)[iD];
	}

	// Get the patch user info size
	uint getUIPatchSize () const
	{
		return (uint)_Data._UIPatch->size();
	}

	// Get a patch user info
	UI_PATCH& getUIPatch (uint iD)
	{
		// Check
		nlassert (iD<getUIPatchSize ());

		return (*_Data._UIPatch)[iD];
	}

	// Get a const patch user info
	const UI_PATCH& getUIPatch (uint iD) const
	{
		// Check
		nlassert (iD<getUIPatchSize ());

		return (*_Data._UIPatch)[iD];
	}

	// Get vertex user info size
	uint getUIVertexSize () const
	{
		return (uint)_Data._UIVertex->size();
	}

	// Get a vertex user info
	UI_VERTEX& getUIVertex (uint iD)
	{
		// Check
		nlassert (iD<getUIVertexSize ());

		return (*_Data._UIVertex)[iD];
	}

	// Get a const vertex user info
	const UI_VERTEX& getUIVertex (uint iD) const
	{
		// Check
		nlassert (iD<getUIVertexSize ());

		return (*_Data._UIVertex)[iD];
	}

public:
	// Validity of the mesh
	Interval			ValidGeom;
	Interval			ValidTopo;
	Interval			ValidTexmap;
	Interval			ValidSelect;
	Interval			ValidDisplay;
	Interval			ValidBindingPos;
	Interval			ValidBindingInfo;
	BitArray			tileSel;
#pragma warning (disable : 4786)

	// Tessel mode
	RPOTess				rTess;
private:
	// cached Mesh for the ModeRykolPatchMesh mode
	Mesh				mesh;

	int					selLevel;
	int					tileSet;
	int					build;
public:
	bool				paint;
	bool				paintHack;
private:
	static CBankManager	manager;

	// Fill the binding info for a vertex. Don't forget to call UpdateBindingInfo after
	void BindingVertex (int nVertex, int nPatch, int nEdge, int nPrimary, typeBind nType);

	// Unbind a vertex
	void UnBindingVertex (int nVertex);

	// Unbind vertex associed to the patch
	void UnbindRelatedPatch (int nPatch, PatchMesh& patch);

	// Unbind vertex associed to the vertex
	void UnbindRelatedVertex (int nPatch, PatchMesh& patch);

	// Update binded vertices's position BIND SAFE
	void UpdateBindingPos (PatchMesh& patch);

	// Build internal binding info
	void UpdateBindingInfo (PatchMesh& patch);

	// Look for a patch with this edge
	void FindPatch (PatchMesh *patch, int nEdge, int &WhichEdge, int &nPatch, int nFirstPatch);
public:
	// Constructor
	RPatchMesh (PatchMesh *pmesh);		// Patch mesh

	// Invalidate binding infos
	void InvalidateBindingPos () { ValidBindingPos=NEVER; };
	void InvalidateBindingInfo () { ValidBindingInfo=NEVER; InvalidateBindingPos (); };

	// Update binding
	void UpdateBinding (PatchMesh& patch, TimeValue t);

	// Check the validity of the RPatchMesh's data with the RPO's data (debug stuff) BIND SAFE
	bool Validity (const PatchMesh& patch, bool bAssert);

	// Resize vertex buffer BIND SAFE
	void SetNumVerts (int nVert);

	// Resize patches buffer BIND SAFE
	void SetNumPatches (int nPatch);

	// Subdivide both way BIND SAFE
	void Subdivide (int nPatch, int nV0, int nV1, int nV2, int nV3, int nCenter, int nFirstPatch, PatchMesh& patch);

	// Subdivide edge 1 and 3 BIND SAFE
	void SubdivideU (int nPatch, int nV0, int nV1, int nFirstPatch, PatchMesh& patch);

	// Subdivide edge 0 and 2 BIND SAFE
	void SubdivideV (int nPatch, int nV0, int nV1, int nFirstPatch, PatchMesh& patch);

	// AddHook BIND SAFE
	void AddHook (int nVert, int nSeg, PatchMesh& patch);

	// AddHook BIND SAFE
	void AddHook (int nVert0, int nVert1, int nVert2, int nSeg, PatchMesh& patch);

	// RemoveHook BIND SAFE
	void RemoveHook (PatchMesh& patch);

	// Attach BIND SAFE
	void Attach(RPatchMesh *rattPatch, PatchMesh& patch);

	// Extrude BIND SAFE
	void CreateExtrusion (PatchMesh *rpatch);

	// Weld BIND SAFE
	void Weld (PatchMesh *patch);

	// Add a patch BIND SAFE
	void AddPatch (int nEdge, int nFirstPatch, PatchMesh *patch);

	// Delete patches and vertices BIND SAFE
	void DeleteAndSweep (const BitArray &remapVerts, const BitArray &remapPatches, PatchMesh& patch);

	// Invalidate channels BIND SAFE
	void InvalidateChannels(ChannelMask channels);

	// Update topo change BIND SAFE
	void ResolveTopoChanges(PatchMesh *patch, bool aux1);

	// Change sel level BIND SAFE
	void SetSelLevel (int sellevel)
	{
		selLevel=sellevel;
		InvalidateChannels(PART_SELECT);
	}

	// Get sel level BIND SAFE
	int GetSelLevel ()
	{
		return selLevel;
	}

	// Load
	IOResult Load(ILoad *iload);

	// Save
	IOResult Save(ISave *isave);

	// *** Tile Methods

	// Get the matrix of the selected tiles
	Matrix3 GetSelTileTm(PatchMesh& patch, TimeValue t, INode *node, bool& bHasSel) const;

	// Get the center of the selected tiles
	Point3 GetSelTileCenter(PatchMesh& patch, TimeValue t, INode *node, bool& bHasSel) const;

	// Hittest method
	BOOL SubObjectHitTest(GraphicsWindow *gw, Material *ma, HitRegion *hr, DWORD flags, SubPatchHitList& hitList, TimeValue t, 
		PatchMesh& patch);

	// Return the tile number
	int GetTileNumber(int nPatch, int nU, int nV) const
	{
		nlassert (nU>=0);
		nlassert (nU<MAX_TILE_IN_PATCH);
		nlassert (nV>=0);
		nlassert (nV<MAX_TILE_IN_PATCH);
		return nV*MAX_TILE_IN_PATCH+nU+nPatch*NUM_TILE_SEL;
	}

	// Build the mesh
	void BuildMesh(TimeValue t, PatchMesh& patch, Mesh *pMesh=NULL);

	// Get tessel level of a patch
	void GetPatchTess (int nPatch, int& nUTess, int& nVTess);

	// Display
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, PatchMesh& patch);

	// Tile access
	tileDesc& getTileDesc (int nTile);

	// Tile access
	void setTileDesc (int nTile, const tileDesc& desc);

	// Rotate tiles
	void RotateTiles (PatchMesh *patch, int rot);

	// Turn selected patch
	void TurnPatch(PatchMesh *patch);

	// Export a zone to NeL format
	bool exportZone(INode* pNode, PatchMesh* pPM, NL3D::CZone& zone, NL3D::CZoneSymmetrisation& sym, int zoneId, float snapCell, float weldThreshold, bool forceBuildZoneSymmetry);

	// Export a zone to NeL format
	void importZone (PatchMesh* pPM, NL3D::CZone& zone, int &zoneId);
	
	// *** Vertex color Methods

	// Get the vertex color of a patch
	void getVertexColor (int patch, int s, int t, NLMISC::CRGBA& dest)
	{
		// Get the color
		uint encodedColor=getUIPatch (patch).getColor (t*((1<<getUIPatch (patch).NbTilesU)+1)+s);

		// Store the color
		dest.A=encodedColor>>24;
		dest.R=(encodedColor>>16)&0xff;
		dest.G=(encodedColor>>8)&0xff;
		dest.B=encodedColor&0xff;
	}

	// Set the vertex color of a patch
	void setVertexColor (int patch, int s, int t, const NLMISC::CRGBA& newColor)
	{
		// Get the color
		uint encodedColor=(newColor.A<<24)|(newColor.R<<16)|(newColor.G<<8)|newColor.B;
		
		// Store the color
		getUIPatch (patch).setColor (t*((1<<getUIPatch (patch).NbTilesU)+1)+s, encodedColor);
	}
};

// ------------------------------------------------------------------------------------------------------------------------------------------------

#endif // __RYKOL_PATCH_MESH_H
