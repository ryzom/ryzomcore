#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_undo.h"
#include "paint_to_nel.h"
#include "paint_vcolor.h"

/*-------------------------------------------------------------------*/

void CTileUndo::toUndo ( const CUndoElement& undoList )
{
	// Push in the toundo list
	_ToUndoList.push_back (undoList);
}

/*-------------------------------------------------------------------*/

// Add undo action
void CTileUndo::pushUndo ()
{
	// Resize the array
	if (_UndoSize<MAX_UNDO)
		_UndoSize++;

	// Decal undo list
	for (int l=_UndoSize-1; l>0; l--)
		// Copy array
		_UndoList[l]=_UndoList[l-1];

	// Push it in the undo stack
	_UndoList[0]=_ToUndoList;

	// Clear redo list
	_RedoSize=0;

	// Clear the toundo list
	_ToUndoList.clear();
}

/*-------------------------------------------------------------------*/

// Undo action: return action to performe
void CTileUndo::getUndoList ()
{
	// Clear result
	_Result.clear ();

	// Not empty list
	if (_UndoSize>0)
	{
		// Resize the array
		if (_RedoSize<MAX_UNDO)
			_RedoSize++;

		// Decal redo list
		uint l;
		for (l=_RedoSize-1; l>0; l--)
			// Copy array
			_RedoList[l]=_RedoList[l-1];

		// Put in the redo list
		_RedoList[0]=_UndoList[0];

		// Copy return
		_Result=_UndoList[0];

		// Decal undo list upper
		for (l=0; l<_UndoSize-1; l++)
			// Copy array
			_UndoList[l]=_UndoList[l+1];

		// Resize the undo array
		_UndoSize--;
	}
}

/*-------------------------------------------------------------------*/

// Redo action: return action to performe
void CTileUndo::getRedoList ()
{
	// Clear result
	_Result.clear ();

	// Not empty list
	if (_RedoSize>0)
	{
		// Resize the undo array
		if (_UndoSize<MAX_UNDO)
			_UndoSize++;

		// Decal undo list
		uint l;
		for (l=_UndoSize-1; l>0; l--)
			// Copy array
			_UndoList[l]=_UndoList[l-1];

		// Put in the redo list
		_UndoList[0]=_RedoList[0];

		// Copy return
		_Result=_RedoList[0];

		// Decal undo list upper
		for (l=0; l<_RedoSize-1; l++)
			// Copy array
			_RedoList[l]=_RedoList[l+1];

		// Resize the undo array
		_RedoSize--;
	}
}

/*-------------------------------------------------------------------*/

// Undo
void CTileUndo::undo (EPM_PaintMouseProc& mouseProc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, PaintPatchMod* pobj, CPaintColor& paintColor)
{
	// List to undo
	getUndoList ();

	// Iterator
	std::vector<CUndoElement>::iterator ite=_Result.begin();

	// Set of meshes
	std::set<int> setMeshes;

	// Nel patch changement manager
	CNelPatchChanger nelPatchChg (land);


		// Iterator
	if (_Result.size()!=0)
	{
		std::vector<CUndoElement>::iterator ite=_Result.end();

		// Go undo
		do
		{
			// Previous
			ite--;

			// Add the mesh
			setMeshes.insert (ite->_Mesh);

			// Check the good action
			if (ite->_Action==UndoTile)
				// Set the tile
				mouseProc.SetTile (ite->_Mesh, ite->_Tile, ite->_Old._Desc, vectMesh, land, nelPatchChg, false, false);
			else if (ite->_Action==UndoColor)
				// Set the color
				paintColor.setVertexColor (ite->_Mesh, ite->_Old._Patch, ite->_Old._S, ite->_Old._T, ite->_Old._Color, ite->_Old._Blend, 
											vectMesh, nelPatchChg, false);
		}
		while (ite!=_Result.begin());
	}
	
	// Flush nel chgt
	nelPatchChg.applyChanges (true);

	// Invalid all meshes
	std::set<int>::iterator iteSet=setMeshes.begin();
}

/*-------------------------------------------------------------------*/

// Redo
void CTileUndo::redo (EPM_PaintMouseProc& mouseProc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, PaintPatchMod* pobj, CPaintColor& paintColor)
{
	// List to undo
	getRedoList ();

	// Set of meshes
	std::set<int> setMeshes;

	// Nel patch changement manager
	CNelPatchChanger nelPatchChg (land);

	// First
	std::vector<CUndoElement>::iterator ite=_Result.begin();

	// Go undo
	while (ite!=_Result.end())
	{
		// Check the good action
		if (ite->_Action==UndoTile)
			// Set the tile
			mouseProc.SetTile (ite->_Mesh, ite->_Tile, ite->_New._Desc, vectMesh, land, nelPatchChg, false, false);
		else if (ite->_Action==UndoColor)
			// Set the color
			paintColor.setVertexColor (ite->_Mesh, ite->_New._Patch, ite->_New._S, ite->_New._T, ite->_New._Color, ite->_New._Blend, 
										vectMesh, nelPatchChg, false);

		// Add the mesh
		setMeshes.insert (ite->_Mesh);

		// Next
		ite++;
	}

	// Flush nel chgt
	nelPatchChg.applyChanges (true);

	// Invalid all meshes
	std::set<int>::iterator iteSet=setMeshes.begin();
}
