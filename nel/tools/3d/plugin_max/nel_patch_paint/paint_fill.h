#include "stdafx.h"

using namespace NL3D;
using namespace NLMISC;

class CPaintColor;

class CFillPatch
{
public:
	// Go, paint !
	CFillPatch (PaintPatchMod *pobj, CLandscape	*landscape, CTileUndo *undo, EPM_PaintMouseProc *mouseProc)
	{
		_PObj=pobj;
		_Landscape=landscape;
		_Undo=undo;
		_MouseProc=mouseProc;
	}

	// Go, fill !
	void fillTile (int mesh, int patch, std::vector<EPM_Mesh> &vectMesh, int tileSet, int rot, int group, bool _256, 
					const CTileBank& bank);

	// Go, fill !
	void fillColor (int mesh, int patch, std::vector<EPM_Mesh> &vectMesh, const CRGBA& color, uint16 blend, CPaintColor& paintColor);

	// Go, fill !
	void fillDisplace (int mesh, int patch, std::vector<EPM_Mesh> &vectMesh, const CTileBank& bank);


private:
	PaintPatchMod			*_PObj;
	CLandscape				*_Landscape;
	CTileUndo				*_Undo;
	EPM_PaintMouseProc		*_MouseProc;
	static float			_Distance[BRUSH_COUNT];
};
