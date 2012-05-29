#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_vcolor.h"
#include "paint_undo.h"
#include "paint_to_nel.h"
#include "nel/3d/landscape.h"
#include "nel/3d/nelu.h"

// User color 1 and 2
extern COLORREF color1;
extern COLORREF color2;
extern float opa1;
extern float opa2;
extern float hard1;
extern float hard2;
CRGBA maxToNel (COLORREF ref);

/*-------------------------------------------------------------------*/

float distance (PaintPatchMod *pobj)
{
	return COLOR_BRUSH_MIN+(COLOR_BRUSH_MAX-COLOR_BRUSH_MIN)*(float)pobj->ColorBushSize/(float)COLOR_BRUSH_STEP;
}
 
 /*-------------------------------------------------------------------*/

void CPaintColor::paint (int mesh, int tile, const CVector& hit, const CVector& topVector, std::vector<EPM_Mesh> &vectMesh)
{
	// Set the brush base vector
	if (fabs (topVector * CVector::K) > fabs (topVector * CVector::J))
	{
		_PaintBaseX = CVector::J ^ topVector;
		_PaintBaseX.normalize ();
		_PaintBaseY = topVector ^ _PaintBaseX;
		_PaintBaseY.normalize ();
	}
	else
	{
		_PaintBaseX = topVector ^ CVector::K;
		_PaintBaseX.normalize ();
		_PaintBaseY = topVector ^ _PaintBaseX;
		_PaintBaseY.normalize ();
	}
	
	// Only if the landscape is valid
	if (_Landscape)
	{
		// Set of visited tiles
		std::set<EPM_PaintTile*> visited;

		// Nel patch changement manager
		CNelPatchChanger nelPatchChg (_Landscape);

		// Frozen vertex color
		static std::vector<CVertexColorFrozed> frozenVertices;
		frozenVertices.reserve (100);
		frozenVertices.clear ();

		// Start at the first tile
		paintATile (&_MouseProc->metaTile[mesh][tile], visited, hit, vectMesh, nelPatchChg, frozenVertices);

		// Force frozen color vertex
		forceFrozen (frozenVertices, nelPatchChg, vectMesh);

		// Flush nel chgt
		nelPatchChg.applyChanges (false);
	}
}

/*-------------------------------------------------------------------*/

bool CPaintColor::getVertexInNeighbor (EPM_PaintTile *pTile, int curU, int curV, int neighbor, int &finalMesh, int &finalPatch, int &finalS, int &finalT)
{
	// Get the vertex id
	/*
	0     3
	 x***x
	 *   *
	 x***x
	1     2
	*/
	static const int remap[4] = { 0, 3, 1, 2};
	int vertexId = (curU - pTile->u) + (curV - pTile->v) * 2;
	nlassert (vertexId<4);
	vertexId = remap[vertexId];

	// Check neighbor asked is valid
	/*
	   3
	 x***x
	0*   *2
	 x***x
	   1
	*/
	nlassert (( vertexId == neighbor ) || ( vertexId == ((neighbor+1)&3) ));

	// Neighbor empty ?
	if (pTile->voisins[neighbor] == NULL)
		return false;

	// Index of the vertex in the neighbor
	int neighborVertexId = (((vertexId == neighbor)?vertexId-1:vertexId+1) + pTile->rotate[neighbor])&3;

	// ** Fill the result

	// Final mesh
	finalMesh = pTile->voisins[neighbor]->Mesh;

	// Final patch
	finalPatch = pTile->voisins[neighbor]->patch;

	// Final s coordinate
	finalS = pTile->voisins[neighbor]->u + ( ( (neighborVertexId==2) || (neighborVertexId==3) )?1:0 );

	// Final t coordinate
	finalT = pTile->voisins[neighbor]->v + ( ( (neighborVertexId==1) || (neighborVertexId==2) )?1:0 );

	// Ok
	return true;
}

/*-------------------------------------------------------------------*/

void CPaintColor::paintATile (EPM_PaintTile *pTile, std::set<EPM_PaintTile*>& visited, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, 
							  CNelPatchChanger& nelPatchChg, std::vector<CVertexColorFrozed> &frozenVertices)
{
	// Check some args
	nlassert (_PObj->brushSize>=0);
	nlassert (_PObj->brushSize<3);

	// Already visited ?
	if (visited.find (pTile)!=visited.end())
		return;

	// Check if we are in patch subobject and if this patch is selected
	if ((vectMesh[pTile->Mesh].PMesh->selLevel==EP_PATCH)&&(!vectMesh[pTile->Mesh].PMesh->patchSel[pTile->patch]))
		return;

	// Check if this tile is in the range of the brush
	if ((pTile->Center-hit).norm()>distance (_PObj)+pTile->Radius)
		return;

	// Is locked on left or top ?
	//if (isLocked (_PObj, pTile, (1<<0)|(1<<3)))

	// Is frozen ?
	bool frozen = pTile->frozen;

	// Paint the upper left vertex
	if (!frozen)
	{
		// Is left or top frozen ?
		bool leftFrozen = (pTile->voisins[0]!=NULL) && pTile->voisins[0]->frozen;
		bool bottomFrozen = (pTile->voisins[1]!=NULL) && pTile->voisins[1]->frozen;
		bool rightFrozen = (pTile->voisins[2]!=NULL) && pTile->voisins[2]->frozen;
		bool topFrozen = (pTile->voisins[3]!=NULL) && pTile->voisins[3]->frozen;

		// Is left or top locked ?
		bool leftLocked = ((pTile->voisins[0]==NULL) || leftFrozen) && _PObj->lockBorders;
		bool bottomLocked = ((pTile->voisins[1]==NULL) || bottomFrozen) && _PObj->lockBorders;
		bool rightLocked = ((pTile->voisins[2]==NULL) || rightFrozen) && _PObj->lockBorders;
		bool topLocked = ((pTile->voisins[3]==NULL) || topFrozen) && _PObj->lockBorders;

		// Color to copy
		CRGBA colorCopy;

		// Not locked ?
		if (!leftLocked && !topLocked)
		{
			// Is this vertex frozen ?
			if (leftFrozen || topFrozen)
				frozenVertices.push_back (CVertexColorFrozed (pTile, pTile->u, pTile->v));

			// Set the vertex color
			paintAVertex (pTile->Mesh, pTile->patch, pTile->u, pTile->v, hit, vectMesh, nelPatchChg);
		}
	
		// Tile on a right, bottom or right bottom border ?
		bool borderRight=pTile->u == (1<<vectMesh[pTile->Mesh].RMesh->getUIPatch (pTile->patch).NbTilesU)-1;
		bool borderBottom=pTile->v == (1<<vectMesh[pTile->Mesh].RMesh->getUIPatch (pTile->patch).NbTilesV)-1;
		if (borderRight)
		{
			// Not locked ?
			if (!rightLocked && !topLocked)
			{
				// Is this vertex frozen ?
				if (rightFrozen || topFrozen)
					frozenVertices.push_back (CVertexColorFrozed (pTile, pTile->u+1, pTile->v));
				
				// Set the vertex color
				paintAVertex (pTile->Mesh, pTile->patch, pTile->u+1, pTile->v, hit, vectMesh, nelPatchChg);
			}
		}
		if (borderBottom)
		{
			// Not locked ?
			if (!leftLocked && !bottomLocked)
			{
				// Is this vertex frozen ?
				if (leftFrozen || bottomFrozen)
					frozenVertices.push_back (CVertexColorFrozed (pTile, pTile->u, pTile->v+1));

				// Set the vertex color
				paintAVertex (pTile->Mesh, pTile->patch, pTile->u, pTile->v+1, hit, vectMesh, nelPatchChg);
			}
		}
		if (borderRight&&borderBottom)
		{
			// Not locked ?
			if (!rightLocked && !bottomLocked)
			{
				// Is this vertex frozen ?
				if (rightFrozen || bottomFrozen)
					frozenVertices.push_back (CVertexColorFrozed (pTile, pTile->u+1, pTile->v+1));

				// Set the vertex color
				paintAVertex (pTile->Mesh, pTile->patch, pTile->u+1, pTile->v+1, hit, vectMesh, nelPatchChg);
			}
		}
	}

	// Visited
	visited.insert (pTile);

	// Visite the neighborhood
	for (int neightbor=0; neightbor<4; neightbor++)
	{
		// Is there a neighbor ?
		if (pTile->voisins[neightbor])
			// Ok, recusive call
			paintATile (pTile->voisins[neightbor], visited, hit, vectMesh, nelPatchChg, frozenVertices);
	}
}

/*-------------------------------------------------------------------*/

void CPaintColor::paintAVertex (int mesh, int patch, int s, int t, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, CNelPatchChanger& nelPatchChg)
{
	// Check some args
	nlassert (_PObj->brushSize>=0);
	nlassert (_PObj->brushSize<3);

	// Get the zone for this mesh
	const CZone* zone=_Landscape->getZone (mesh);
	nlassert (zone);

	// Get the patch pointer
	const CPatch *pPatch=zone->getPatch(patch);

	// Eval the vertex position
	CVector vertexPos=pPatch->computeVertex ((float)s/(float)pPatch->getOrderS(), (float)t/(float)pPatch->getOrderT());

	// Compute dist from the brush
	CVector deltaPos = vertexPos-hit;
	float distBrush = deltaPos.norm();

	// Brush size
	float brushSize = distance (_PObj);

	// Check if it is in distance
	if (distBrush<=brushSize)
	{
		// *** Compute new vertex color

		// Get the old value
		CRGBA old;
		vectMesh[mesh].RMesh->getVertexColor (patch, s, t, old);

		// Blend with distance
		float blendDist=(brushSize-distBrush)/brushSize;

		// Blend the two colors
		float finalFactor=256.f*opa1*((1.f-hard1)*blendDist+hard1);
		uint16 blend=(uint16)(std::max (std::min (finalFactor, 256.f), 0.f) );

		// The color
		CRGBA theColor = maxToNel (color1);

		// Use a brush ?
		if (_bBrush)
		{
			// Compute the projection on the brush plane
			float bitmapX = (1 + (_PaintBaseX * deltaPos) / brushSize) / 2;
			float bitmapY = (1 + (_PaintBaseY * deltaPos) / brushSize) / 2;

			// Read the pixel
			CRGBAF colorF = _BrushBitmap.getColor (bitmapX, bitmapY);
			colorF *= 255.f;
			CRGBA color;
			color.R = (uint8)colorF.R;
			color.G = (uint8)colorF.G;
			color.B = (uint8)colorF.B;
			color.A = (color.R + color.G + color.B ) / 3;

			// Adjust color and blend
			blend = (uint16)(blend * color.A / 255);
		}

		// Set the vertex color
		setVertexColor (mesh, patch, s, t, theColor, blend, vectMesh, nelPatchChg, true);
	}
}

/*-------------------------------------------------------------------*/

void CPaintColor::pickVertexColor (int mesh, int patch, int s, int t, CVector& pos, CRGBA& color, std::vector<EPM_Mesh> &vectMesh)
{
	// Check some args
	nlassert (_PObj->brushSize>=0);
	nlassert (_PObj->brushSize<3);

	// Get the zone for this mesh
	const CZone* zone=_Landscape->getZone (mesh);
	nlassert (zone);

	// Get the patch pointer
	const CPatch *pPatch=zone->getPatch(patch);

	// Eval the vertex position
	pos=pPatch->computeVertex ((float)s/(float)pPatch->getOrderS(), (float)t/(float)pPatch->getOrderT());

	// Get the old value
	vectMesh[mesh].RMesh->getVertexColor (patch, s, t, color);
}

/*-------------------------------------------------------------------*/

void CPaintColor::setVertexColor (int mesh, int patch, int s, int t, const CRGBA& newColor, uint16 blend, std::vector<EPM_Mesh> &vectMesh, 
								  CNelPatchChanger& nelPatchChg, bool undo)
{
	// Get the old value
	CRGBA old;
	vectMesh[mesh].RMesh->getVertexColor (patch, s, t, old);

	if (undo)
	{
		// Create an undo element
		CUndoElement elmt (mesh, CUndoStruct (patch, s, t, old, 256), CUndoStruct (patch, s, t, newColor, blend));

		// Put an undo entry
		_Undo->toUndo (elmt);
	}

	// Get the Nel patch mesh
	RPatchMesh *pMesh=vectMesh[mesh].RMesh;

	// Blend color
	CRGBA color;
	color.blendFromui ( old, newColor, blend );

	// Get order S of this patch
	int OrderS=(1<<pMesh->getUIPatch (patch).NbTilesU)+1;

	// For each mesh
	for (uint i=0; i<vectMesh.size(); i++)
	{
		// Same mesh ?
		if (vectMesh[i].RMesh == pMesh)
		{
			// Get the patch
			std::vector<CTileColor>& copyZone = *nelPatchChg.getColorArray  (i, patch);

			// Set the color in 
			if (vectMesh[i].Symmetry)
			{
				uint newS = OrderS - s - 1;
				copyZone[newS+t*OrderS].Color565=color.get565();
			}
			else
				copyZone[s+t*OrderS].Color565=color.get565();
		}
	}

	// 3dsmax update
	vectMesh[mesh].RMesh->setVertexColor (patch, s, t, color);
}

/*-------------------------------------------------------------------*/

bool CPaintColor::loadBrush (const char *brushFileName)
{
	// Open the file
	try
	{
		// Open the bitmap
		CIFile inputFile;
		if (inputFile.open (brushFileName))
		{
			// Read it in RGBA
			if (_BrushTexture == NULL)
				_BrushTexture = new CTextureFile();
			_BrushTexture->loadGrayscaleAsAlpha (false);
			_BrushBitmap.loadGrayscaleAsAlpha (false);
			_BrushTexture->setFileName (brushFileName);
			_BrushBitmap.load (inputFile);

			// Convert in RGBA
			_BrushBitmap.convertToType (CBitmap::RGBA);
		}
		else
		{
			// Error message
			char msg[512];
			smprintf (msg, 512, "Can't open the file %s.", brushFileName);
			MessageBox ((HWND)CNELU::Driver->getDisplay(), msg, "NeL Painter", MB_OK|MB_ICONEXCLAMATION);

			// Return false
			return false;
		}
	}
	catch (Exception &e)
	{
		// Error message
		MessageBox ((HWND)CNELU::Driver->getDisplay(), e.what(), "NeL Painter", MB_OK|MB_ICONEXCLAMATION);

		// Return false
		return false;
	}

	// Ok
	return true;
}

/*-------------------------------------------------------------------*/

void CPaintColor::forceFrozen (const std::vector<CVertexColorFrozed> &frozenVertices, CNelPatchChanger& nelPatchChg, std::vector<EPM_Mesh> &vectMesh)
{
	// 1. Referenced frozen vertices will copy their color in non frozen color
	uint vertex;
	for (vertex=0; vertex<frozenVertices.size(); vertex++)
	{
		// Vertex
		const CVertexColorFrozed &vertexFrozen = frozenVertices[vertex];

		// For each mesh
		uint mesh;
		for (mesh=0; mesh<vectMesh.size(); mesh++)
		{
			// Same instancied mesh but not the same node ?
			if ( (vectMesh[mesh].RMesh == vectMesh[vertexFrozen.Tile->Mesh].RMesh) && (mesh != (uint)vertexFrozen.Tile->Mesh) )
			{
				// Reference tile pointer
				EPM_PaintTile *refTile = &_MouseProc->metaTile[mesh][vertexFrozen.Tile->tile];
				nlassert (refTile != vertexFrozen.Tile);

				// Neighbor frozen
				bool neighborNotFrozen[4];
				uint n;
				for (n=0; n<4; n++)
					// Flag
					neighborNotFrozen[n] = (refTile->voisins[n] != NULL) && (!refTile->voisins[n]->frozen);

				// Get the vertex id
				static const int remap[4] = { 0, 3, 1, 2};
				uint vertexId = (vertexFrozen.S - refTile->u) + (vertexFrozen.T - refTile->v) * 2;
				nlassert (vertexId<4);
				vertexId = remap[vertexId];

				// Choose the neighbor
				uint neighbor = 0xffffffff;
				if (neighborNotFrozen[vertexId])
				{
					neighbor = vertexId;
				}
				else if (neighborNotFrozen[(vertexId-1)&3])
				{
					// Should be the other one
					neighbor = (vertexId-1)&3;
				}

				// Find ?
				if (neighbor != 0xffffffff)
				{
					// Get the source color
					CRGBA color;
					vectMesh[mesh].RMesh->getVertexColor (refTile->patch, vertexFrozen.S, vertexFrozen.T, color);

					// Get the corresponding vertex
					int finalMesh, finalPatch, finalS, finalT;
					nlverify (getVertexInNeighbor (refTile, vertexFrozen.S, vertexFrozen.T, neighbor, finalMesh, finalPatch,
						finalS, finalT));

					// Set the vertex color
					setVertexColor (finalMesh, finalPatch, finalS, finalT, color, 255, vectMesh, nelPatchChg, true);
				}
			}
		}
	}

	// 2. Non frozen vertices will get their color from the frozen vertices
	for (vertex=0; vertex<frozenVertices.size(); vertex++)
	{
		// Vertex
		const CVertexColorFrozed &vertexFrozen = frozenVertices[vertex];

		// Neighbor frozen
		bool neighborFrozen[4];
		uint n;
		for (n=0; n<4; n++)
			// Flag
			neighborFrozen[n] = (vertexFrozen.Tile->voisins[n] != NULL) && vertexFrozen.Tile->voisins[n]->frozen;

		// Get the vertex id
		static const int remap[4] = { 0, 3, 1, 2};
		uint vertexId = (vertexFrozen.S - vertexFrozen.Tile->u) + (vertexFrozen.T - vertexFrozen.Tile->v) * 2;
		nlassert (vertexId<4);
		vertexId = remap[vertexId];

		// Choose the neighbor
		uint neighbor;
		if (neighborFrozen[vertexId])
		{
			// This neighbor
			neighbor = vertexId;
		}
		else
		{
			// Should be the other one
			nlassert (neighborFrozen[(vertexId-1)&3]);
			neighbor = (vertexId-1)&3;
		}

		// Get the frozen neighbor
		int sourceMesh, sourcePatch, sourceS, sourceT;
		nlverify (getVertexInNeighbor (vertexFrozen.Tile, vertexFrozen.S, vertexFrozen.T, neighbor, sourceMesh, sourcePatch,
			sourceS, sourceT));

		// Get the source color
		CRGBA color;
		vectMesh[sourceMesh].RMesh->getVertexColor (sourcePatch, sourceS, sourceT, color);

		// Set the vertex color
		setVertexColor (vertexFrozen.Tile->Mesh, vertexFrozen.Tile->patch, vertexFrozen.S, vertexFrozen.T, color, 255, vectMesh, nelPatchChg, true);
	}
}

/*-------------------------------------------------------------------*/


