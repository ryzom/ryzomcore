#include "stdafx.h"
#include "nel/3d/texture_file.h"

using namespace NL3D;
using namespace NLMISC;

namespace NL3D
{
	class ITexture;
}

/* 
 * CVertexColorFrozed
 * 
 * Log a frozen vertex color painting
 */
class CVertexColorFrozed
{
public:
	// Constructors
	CVertexColorFrozed () {}
	CVertexColorFrozed (EPM_PaintTile *tile, int s, int t)
	{
		Tile = tile;
		S = s;
		T = t;
	}

	// Members
	EPM_PaintTile	*Tile;
	int				S, T;
};

class CPaintColor
{
public:
	// Go, paint !
	CPaintColor (PaintPatchMod *pobj, CLandscape	*landscape, CTileUndo *undo, EPM_PaintMouseProc *mouseProc)
	{
		_PObj=pobj;
		_Landscape=landscape;
		_Undo=undo;
		_MouseProc=mouseProc;
		_bBrush=false;
	}

	// Go, paint !
	void paint (int mesh, int tile, const CVector& hit, const CVector& topVector, std::vector<EPM_Mesh> &vectMesh);

	// Set a vertex color with handle of the undo
	void setVertexColor (int mesh, int patch, int s, int t, const CRGBA& newColor, uint16 blend, std::vector<EPM_Mesh> &vectMesh, 
		CNelPatchChanger& nelPatchChg, bool undo);

	// picj a vertex of a patch
	void pickVertexColor (int mesh, int patch, int s, int t, CVector& pos, CRGBA& color, std::vector<EPM_Mesh> &vectMesh);

	// Set brush mode
	void setBrushMode (bool brushOn)
	{
		// Brush loaded ?
		if ( ( _BrushBitmap.getWidth()!= 0 ) && ( _BrushBitmap.getHeight()!= 0 ) && brushOn)
		{
			_bBrush = true;
		}
		else
		{
			_bBrush = false;
		}
	}

	// Get brush mode
	bool getBrushMode () const
	{
		return _bBrush;
	}

	// Get brush mode
	NL3D::ITexture& getBrush ()
	{
		return *_BrushTexture;
	}

	// Load a brush
	bool loadBrush (const std::string &brushFileName);

private:

	// Recurcive function to paint a tile
	void paintATile (EPM_PaintTile *pTile, std::set<EPM_PaintTile*>& visited, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, 
		CNelPatchChanger& nelPatchChg, std::vector<CVertexColorFrozed> &frozenVertices);

	// Paint a vertex of a patch
	void paintAVertex (int mesh, int patch, int s, int t, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, CNelPatchChanger& nelPatchChg);

	// Get the vertex id in the neighbor
	static bool getVertexInNeighbor (EPM_PaintTile *pTile, int curU, int curV, int neighbor, int &finalMesh, int &finalPatch, int &finalS, int &finalT);

	// Force frozen reference zone to paint vertex color on the boundaries
	void forceFrozen (const std::vector<CVertexColorFrozed> &frozenVertices, CNelPatchChanger& nelPatchChg, std::vector<EPM_Mesh> &vectMesh);

private:
	PaintPatchMod			*_PObj;
	CLandscape				*_Landscape;
	CTileUndo				*_Undo;
	EPM_PaintMouseProc		*_MouseProc;
	bool					_bBrush;
	CVector					_PaintBaseX;
	CVector					_PaintBaseY;
	CSmartPtr<CTextureFile>	_BrushTexture;
	CBitmap					_BrushBitmap;
};
