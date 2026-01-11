#include "stdafx.h"

using namespace NL3D;
using namespace NLMISC;

#define MAX_UNDO 10

enum UndoAction { UndoTile, UndoColor, UndoDisplace };

// Undo element
class CUndoStruct
{
	friend class CTileUndo;
public:

	// Constructor for tile
	CUndoStruct (const tileDesc& desc)
	{
		// Build data
		_Desc=desc;
	}
	
	// Constructor for vertex color
	CUndoStruct (uint16 patch, uint8 s, uint8 t, const CRGBA& color, uint16 blend)
	{
		// Build data
		_Patch=patch;
		_S=s;
		_T=t;
		_Color=color;
		_Blend=blend;
	}

private:
	tileDesc	_Desc;
	uint16		_Patch;
	uint8		_S;
	uint8		_T;
	uint16		_Blend;
	CRGBA		_Color;
};

// Undo element
class CUndoElement
{
	friend class CTileUndo;
public:
	// Constructor
	CUndoElement (int mesh, int tile, const CUndoStruct& old,  const CUndoStruct& _new) : _Old (old), _New (_new)
	{
		_Action=UndoTile;
		_Mesh=mesh;
		_Tile=tile;
	};

	// Constructor
	CUndoElement (int mesh, const CUndoStruct& old,  const CUndoStruct& _new) : _Old (old), _New (_new)
	{
		_Action=UndoColor;
		_Mesh=mesh;
	};

private:
	UndoAction		_Action;
	int				_Mesh;
	int				_Tile;
	CUndoStruct		_Old;
	CUndoStruct		_New;
};

// A undo manager for the painter
class CTileUndo
{
public:

	// Allocate arrays
	CTileUndo ()
	{
		_ToUndoList.reserve (500);
		_Result.reserve (500);
		_UndoSize=0;
		_RedoSize=0;
		for (int i=0; i<MAX_UNDO; i++)
		{
			_UndoList[i].reserve (500);
			_RedoList[i].reserve (500);
		}
	}

	// Add undo action
	void toUndo ( const CUndoElement& undoList );

	// Add undo action
	void pushUndo ();

	// Undo
	void undo (EPM_PaintMouseProc& mouseProc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, PaintPatchMod* pobj, CPaintColor& paintColor);

	// Redo
	void redo (EPM_PaintMouseProc& mouseProc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, PaintPatchMod* pobj, CPaintColor& paintColor);

private:
	// Undo action: return action to performe
	void getUndoList ();

	// Redo action: return action to performe
	void getRedoList ();

private:
	std::vector<CUndoElement>					_ToUndoList;
	std::vector<CUndoElement>					_Result;
	uint										_UndoSize;
	std::vector<CUndoElement>					_UndoList[MAX_UNDO];
	uint										_RedoSize;
	std::vector<CUndoElement>					_RedoList[MAX_UNDO];
};
