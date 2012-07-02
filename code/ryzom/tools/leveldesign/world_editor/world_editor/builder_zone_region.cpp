// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "action.h"
#include "world_editor_doc.h"
#include "builder_zone_region.h"
#include "builder_zone.h"

#include <queue>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ***************************************************************************
// CBuilderZoneRegion
// ***************************************************************************


// ---------------------------------------------------------------------------
CBuilderZoneRegion::CBuilderZoneRegion (uint regionId)
{
	_ZeBank = NULL;
	RegionId = regionId;
}

// ---------------------------------------------------------------------------
bool CBuilderZoneRegion::init (NLLIGO::CZoneBank *pBank, CBuilderZone *pBuilder, string &error)
{
	if (_ZeBank != NULL)
		return true;
	_ZeBank = pBank;
	_Builder = pBuilder;
	// Build the material tree
	uint32 i, j, k;
	SMatNode mn;
	vector<string> AllValues;
	_ZeBank->getCategoryValues ("material", AllValues);
	for (i = 0; i < AllValues.size(); ++i)
	{
		mn.Name = AllValues[i];
		_MatTree.push_back (mn);
	}
	// Link between materials
	AllValues.clear ();
	_ZeBank->getCategoryValues ("transname", AllValues);
	for (i = 0; i < AllValues.size(); ++i)
	{
		// Get the 2 materials linked together
		string matAstr, matBstr;
		for (j = 0; j < AllValues[i].size(); ++j)
		{
			if (AllValues[i][j] == '-')
				break;
			else
				matAstr += AllValues[i][j];
		}
		++j;
		for (; j < AllValues[i].size(); ++j)
			matBstr += AllValues[i][j];

		// Find matA
		for (j = 0; j < _MatTree.size(); ++j)
		if (_MatTree[j].Name == matAstr)
			break;

		if (j < _MatTree.size())
		{
			// Find matB
			for (k = 0; k < _MatTree.size(); ++k)
			if (_MatTree[k].Name == matBstr)
				break;

			if (k < _MatTree.size())
			{
				_MatTree[j].Arcs.push_back (k); // Add a ref to matB in node matA
				_MatTree[k].Arcs.push_back (j); // Add a ref to matA in node matB
			}
		}
	}

	// DEBUG
	// DEBUG
	// DEBUG
/*
	_MatTree.clear();
	_MatTree.resize (6);
	_MatTree[0].Name = "A";
	_MatTree[0].Arcs.resize (2);
	_MatTree[0].Arcs[0] = 1;
	_MatTree[0].Arcs[1] = 2;

	_MatTree[1].Name = "F";
	_MatTree[1].Arcs.resize (3);
	_MatTree[1].Arcs[0] = 0;
	_MatTree[1].Arcs[1] = 4;
	_MatTree[1].Arcs[2] = 5;

	_MatTree[2].Name = "B";
	_MatTree[2].Arcs.resize (3);
	_MatTree[2].Arcs[0] = 0;
	_MatTree[2].Arcs[1] = 3;
	_MatTree[2].Arcs[2] = 5;

	_MatTree[3].Name = "C";
	_MatTree[3].Arcs.resize (1);
	_MatTree[3].Arcs[0] = 2;

	_MatTree[4].Name = "Z";
	_MatTree[4].Arcs.resize (1);
	_MatTree[4].Arcs[0] = 1;

	_MatTree[5].Name = "E";
	_MatTree[5].Arcs.resize (2);
	_MatTree[5].Arcs[0] = 1;
	_MatTree[5].Arcs[1] = 2;

	vector<uint32> vRetPath;
	tryPath(0, 3, vRetPath);

	uint32 ij;
	++ij;
	ij = 0;
*/
	// DEBUG
	// DEBUG
	// DEBUG
	return true;
}

// ---------------------------------------------------------------------------
class CToUpdate
{
	struct SElt
	{
		sint32				x, y;
		string				MatPut; // Material put into the cell to update
		CBuilderZoneRegion	*BZR;
	};

	vector<SElt> Elts;

public:

	void add (CBuilderZoneRegion* pBZR, sint32 x, sint32 y, const string &MatName)
	{
		bool bFound = false;
		for (uint32 m = 0; m < Elts.size(); ++m)
			if ((Elts[m].x == x) && (Elts[m].y == y))
			{
				bFound = true;
				break;
			}
		if (!bFound)
		{
			SElt tuTmp;
			tuTmp.x = x;
			tuTmp.y = y;
			tuTmp.MatPut = MatName;
			tuTmp.BZR = pBZR;
			Elts.push_back (tuTmp);
		}
	}

	void del (sint32 x, sint32 y)
	{
		bool bFound = false;
		uint32 m;
		for (m = 0; m < Elts.size(); ++m)
			if ((Elts[m].x == x) && (Elts[m].y == y))
			{
				bFound = true;
				break;
			}
		if (bFound)
		{
			for (; m < Elts.size()-1; ++m)
				Elts[m] = Elts[m+1];
			Elts.resize (Elts.size()-1);
		}
	}

	uint32 size()
	{
		return Elts.size();
	}

	sint32 getX (uint32 m)
	{
		return Elts[m].x;
	}

	sint32 getY (uint32 m)
	{
		return Elts[m].y;
	}

	CBuilderZoneRegion* getBZR (uint32 m)
	{
		return Elts[m].BZR;
	}

	const string& getMat (uint32 m)
	{
		return Elts[m].MatPut;
	}
};

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::add (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt)
{
	sint32 sizeX = pElt->getSizeX(), sizeY = pElt->getSizeY();
	sint32 i, j;
	SPiece sMask, sPosX, sPosY;
	CToUpdate tUpdate; // Transition to update

	if (!_Builder->getZoneMask (x,y))
		return;

	if (pElt->getCategory("transname") != STRING_NO_CAT_TYPE)
	{
		addTransition (x, y, nRot, nFlip, pElt);
		return;
	}

	// Create the mask in the good rotation and flip
	sMask.Tab.resize (sizeX*sizeY);
	sPosX.Tab.resize (sizeX*sizeY);
	sPosY.Tab.resize (sizeX*sizeY);

	for (j = 0; j < sizeY; ++j)
	for (i = 0; i < sizeX; ++i)
	{
		sPosX.Tab[i+j*sizeX] = (uint8)i;
		sPosY.Tab[i+j*sizeX] = (uint8)j;
		sMask.Tab[i+j*sizeX] = pElt->getMask()[i+j*sizeX];
	}
	sPosX.w = sPosY.w = sMask.w = sizeX;
	sPosX.h = sPosY.h = sMask.h = sizeY;
	sMask.rotFlip (nRot, nFlip);
	sPosX.rotFlip (nRot, nFlip);
	sPosY.rotFlip (nRot, nFlip);

	// Test if the pieces can be put (due to mask)
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		if (_Builder->getZoneMask(x+i, y+j) == false)
			return;
	}

	// Delete all pieces that are under the mask
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		del (x+i, y+j, true, &tUpdate);
	}

	// Delete all around all material that are not from the same as us
	const string &CurMat = pElt->getCategory ("material");

	if (CurMat != STRING_NO_CAT_TYPE)
	{	// This element is a valid material
		// Place the piece
		const string &EltName = pElt->getName ();
		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+i, y+j, sPosX.Tab[i+j*sPosX.w], sPosY.Tab[i+j*sPosY.w], EltName);
			setRot (x+i, y+j, nRot);
			setFlip (x+i, y+j, nFlip);
		}

		// Put all transitions between different materials
		putTransitions (x, y, sMask, CurMat, &tUpdate);

		// Place the piece
		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+i, y+j, sPosX.Tab[i+j*sPosX.w], sPosY.Tab[i+j*sPosY.w], EltName);
			setRot (x+i, y+j, nRot);
			setFlip (x+i, y+j, nFlip);
		}
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::invertCutEdge (sint32 x, sint32 y, uint8 cePos)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	if ((x < pBZR->getMinX ()) || (x > pBZR->getMaxX ()) ||
		(y < pBZR->getMinY ()) || (y > pBZR->getMaxY ()))
		return;
	CZoneBankElement *pElt = _ZeBank->getElementByZoneName (pBZR->getName (x, y));
	if (pElt == NULL)
		return;
	if (pElt->getCategory("transname") == STRING_NO_CAT_TYPE)
		return;

	CDatabaseLocator zonePos (RegionId, x, y);
	CLigoData dataZonePos;
	getDocument ()->getLigoData (dataZonePos, zonePos);
	if (dataZonePos.SharingCutEdges[cePos] != 3 - dataZonePos.SharingCutEdges[cePos])
	{
		dataZonePos.SharingCutEdges[cePos] = 3 - dataZonePos.SharingCutEdges[cePos];
		getDocument ()->addModification (new CActionLigoTile (dataZonePos, zonePos));
	}
	updateTrans (x, y);

	// If the transition number is not the same propagate the change
	// Propagate where the edge is cut (1/3 or 2/3) and update the transition
	if (cePos == 2)
	if (dataZonePos.SharingMatNames[0] != dataZonePos.SharingMatNames[2])
	{
		if (x > pBZR->getMinX ())
		{	// [x-1][y].right = [x][y].left
			
			// _Zones[(x-1-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[3] = dataZonePos.SharingCutEdges[2];
			CDatabaseLocator zoneTemp (RegionId, x-1, y);
			CLigoData dataZoneTemp;
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[3] != dataZonePos.SharingCutEdges[2])
			{
				dataZoneTemp.SharingCutEdges[3] = dataZonePos.SharingCutEdges[2];
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}
		updateTrans (x-1, y);
	}
	if (cePos == 3)
	if (dataZonePos.SharingMatNames[1] != dataZonePos.SharingMatNames[3])
	{
		if (x < pBZR->getMaxX ())
		{	// [x+1][y].left = [x][y].right

			CDatabaseLocator zoneTemp (RegionId, x+1, y);
			CLigoData dataZoneTemp;
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[2] != dataZonePos.SharingCutEdges[3])
			{
				dataZoneTemp.SharingCutEdges[2] = dataZonePos.SharingCutEdges[3];
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}
		updateTrans (x+1, y);
	}
	if (cePos == 1)
	if (dataZonePos.SharingMatNames[0] != dataZonePos.SharingMatNames[1])
	{
		if (y > pBZR->getMinY ())
		{	// [x][y-1].up = [x][y].down

			CDatabaseLocator zoneTemp (RegionId, x, y-1);
			CLigoData dataZoneTemp;
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[0] != dataZonePos.SharingCutEdges[1])
			{
				dataZoneTemp.SharingCutEdges[0] = dataZonePos.SharingCutEdges[1];
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}
		updateTrans (x, y-1);
	}
	if (cePos == 0)
	if (dataZonePos.SharingMatNames[2] != dataZonePos.SharingMatNames[3])
	{
		if (y < pBZR->getMaxY ())
		{	// [x][y+1].down = [x][y].up

			CDatabaseLocator zoneTemp (RegionId, x, y+1);
			CLigoData dataZoneTemp;
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[1] != dataZonePos.SharingCutEdges[0])
			{
				dataZoneTemp.SharingCutEdges[1] = dataZonePos.SharingCutEdges[0];
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}
		updateTrans (x, y+1);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::cycleTransition (sint32 x, sint32 y)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	if ((x < pBZR->getMinX ()) || (x > pBZR->getMaxX ()) ||
		(y < pBZR->getMinY ()) || (y > pBZR->getMaxY ()))
		return;
	/* todo  remove
	sint32 stride = (1 + pBZR->getMaxX () - pBZR->getMinX ());
	sint32 zonePos = (x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride;
	*/
	CZoneBankElement *pElt = _ZeBank->getElementByZoneName (pBZR->getName (x, y));
	if (pElt == NULL)
		return;
	if (pElt->getCategory("transname") == STRING_NO_CAT_TYPE)
		return;

	// \todo trap -> choose the good transition in function of the transition under the current location
	// Choose the next possible transition if not the same as the first one
	// Choose among all transition of the same number
	
	updateTrans (x, y);
}

// ---------------------------------------------------------------------------
bool CBuilderZoneRegion::addNotPropagate (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	sint32 sizeX = pElt->getSizeX(), sizeY = pElt->getSizeY();
	sint32 i, j;
	SPiece sMask, sPosX, sPosY;
	CToUpdate tUpdate; // Transition to update

	if (!_Builder->getZoneMask (x,y))
		return false;

	if (pElt->getCategory("transname") != STRING_NO_CAT_TYPE)
	{
		addTransition (x, y, nRot, nFlip, pElt);
		return true;
	}

	// Create the mask in the good rotation and flip
	sMask.Tab.resize (sizeX*sizeY);
	sPosX.Tab.resize (sizeX*sizeY);
	sPosY.Tab.resize (sizeX*sizeY);

	for (j = 0; j < sizeY; ++j)
	for (i = 0; i < sizeX; ++i)
	{
		sPosX.Tab[i+j*sizeX] = (uint8)i;
		sPosY.Tab[i+j*sizeX] = (uint8)j;
		sMask.Tab[i+j*sizeX] = pElt->getMask()[i+j*sizeX];
	}
	sPosX.w = sPosY.w = sMask.w = sizeX;
	sPosX.h = sPosY.h = sMask.h = sizeY;
	sMask.rotFlip (nRot, nFlip);
	sPosX.rotFlip (nRot, nFlip);
	sPosY.rotFlip (nRot, nFlip);

	// Test if the pieces can be put (due to mask)
	sint32 stride = (1 + pBZR->getMaxX () - pBZR->getMinX ());
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		if (_Builder->getZoneMask(x+i, y+j) == false)
			return false;
		if (((x+i) < pBZR->getMinX ()) || ((x+i) > pBZR->getMaxX ()) ||
			((y+j) < pBZR->getMinY ()) || ((y+j) > pBZR->getMaxY ()))
			return false;
		CZoneBankElement *pEltUnder = _ZeBank->getElementByZoneName(pBZR->getName (x+i, y+j));
		if (pEltUnder == NULL)
			return false;
		if (pEltUnder->getCategory("material") != pElt->getCategory("material"))
			return false;
	}

	// Delete all pieces that are under the mask
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		del (x+i, y+j, true, &tUpdate);
	}

	const string &CurMat = pElt->getCategory ("material");

	if (CurMat != STRING_NO_CAT_TYPE)
	{	// This element is a valid material
		// Place the piece
		const string &EltName = pElt->getName ();
		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+i, y+j, sPosX.Tab[i+j*sPosX.w], sPosY.Tab[i+j*sPosY.w], EltName);
			setRot (x+i, y+j, nRot);
			setFlip (x+i, y+j, nFlip);
		}
	}
	
	return true;
}

// ---------------------------------------------------------------------------
// Brutal adding a zone over empty space do not propagate in any way -> can result 
// in inconsistency when trying the propagation mode
void CBuilderZoneRegion::addForce (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	sint32 sizeX = pElt->getSizeX(), sizeY = pElt->getSizeY();
	sint32 i, j;
	SPiece sMask, sPosX, sPosY;
	CToUpdate tUpdate; // Transition to update

	if (!_Builder->getZoneMask (x,y))
		return;

	/*
	if (pElt->getCategory("transname") != STRING_NO_CAT_TYPE)
	{
		addTransition (x, y, nRot, nFlip, pElt);
		return;
	}*/

	// Create the mask in the good rotation and flip
	sMask.Tab.resize (sizeX*sizeY);
	sPosX.Tab.resize (sizeX*sizeY);
	sPosY.Tab.resize (sizeX*sizeY);

	for (j = 0; j < sizeY; ++j)
	for (i = 0; i < sizeX; ++i)
	{
		sPosX.Tab[i+j*sizeX] = (uint8)i;
		sPosY.Tab[i+j*sizeX] = (uint8)j;
		sMask.Tab[i+j*sizeX] = pElt->getMask()[i+j*sizeX];
	}
	sPosX.w = sPosY.w = sMask.w = sizeX;
	sPosX.h = sPosY.h = sMask.h = sizeY;
	sMask.rotFlip (nRot, nFlip);
	sPosX.rotFlip (nRot, nFlip);
	sPosY.rotFlip (nRot, nFlip);

	// Test if the pieces can be put (due to mask)
	// All space under the mask must be empty
	sint32 stride = (1 + pBZR->getMaxX () - pBZR->getMinX ());
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		if (_Builder->getZoneMask(x+i, y+j) == false)
			return;
		if (((x+i) < pBZR->getMinX ()) || ((x+i) > pBZR->getMaxX ()) ||
			((y+j) < pBZR->getMinY ()) || ((y+j) > pBZR->getMaxY ()))
			return;
		CZoneBankElement *pEltUnder = _ZeBank->getElementByZoneName(pBZR->getName (x+i, y+i));
		if (pEltUnder != NULL)
			return;
	}

	// Delete all pieces that are under the mask
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		del (x+i, y+j, true, &tUpdate);
	}

	const string &CurMat = pElt->getCategory ("material");
	const bool transition = pElt->getCategory("transname") != STRING_NO_CAT_TYPE;

	if (CurMat != STRING_NO_CAT_TYPE || transition)
	{	// This element is a valid material
		// Place the piece
		const string &EltName = pElt->getName ();
		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+i, y+j, sPosX.Tab[i+j*sPosX.w], sPosY.Tab[i+j*sPosY.w], EltName, transition);
			setRot (x+i, y+j, nRot);
			setFlip (x+i, y+j, nFlip);
		}
	}
}

// ---------------------------------------------------------------------------

// Convert (transNum,flip,rot) to cutEdge(up,down,left,right)
uint8 TransToEdge[72][4] =
{	
	{ 0, 0, 1, 1 }, // TransNum = 0, Flip = 0, Rot = 0
	{ 2, 2, 0, 0 }, // TransNum = 0, Flip = 0, Rot = 1
	{ 0, 0, 2, 2 }, // TransNum = 0, Flip = 0, Rot = 2
	{ 1, 1, 0, 0 }, // TransNum = 0, Flip = 0, Rot = 3
	{ 0, 0, 1, 1 }, // TransNum = 0, Flip = 1, Rot = 0
	{ 2, 2, 0, 0 }, // TransNum = 0, Flip = 1, Rot = 1
	{ 0, 0, 2, 2 }, // TransNum = 0, Flip = 1, Rot = 2
	{ 1, 1, 0, 0 }, // TransNum = 0, Flip = 1, Rot = 3

	{ 0, 0, 1, 2 }, // TransNum = 1, Flip = 0, Rot = 0
	{ 1, 2, 0, 0 }, // TransNum = 1, Flip = 0, Rot = 1
	{ 0, 0, 1, 2 }, // TransNum = 1, Flip = 0, Rot = 2
	{ 1, 2, 0, 0 }, // TransNum = 1, Flip = 0, Rot = 3
	{ 0, 0, 2, 1 }, // TransNum = 1, Flip = 1, Rot = 0
	{ 2, 1, 0, 0 }, // TransNum = 1, Flip = 1, Rot = 1
	{ 0, 0, 2, 1 }, // TransNum = 1, Flip = 1, Rot = 2
	{ 2, 1, 0, 0 }, // TransNum = 1, Flip = 1, Rot = 3

	{ 0, 0, 2, 2 }, // TransNum = 2, Flip = 0, Rot = 0
	{ 1, 1, 0, 0 }, // TransNum = 2, Flip = 0, Rot = 1
	{ 0, 0, 1, 1 }, // TransNum = 2, Flip = 0, Rot = 2
	{ 2, 2, 0, 0 }, // TransNum = 2, Flip = 0, Rot = 3
	{ 0, 0, 2, 2 }, // TransNum = 2, Flip = 1, Rot = 0
	{ 1, 1, 0, 0 }, // TransNum = 2, Flip = 1, Rot = 1
	{ 0, 0, 1, 1 }, // TransNum = 2, Flip = 1, Rot = 2
	{ 2, 2, 0, 0 }, // TransNum = 2, Flip = 1, Rot = 3

	{ 0, 1, 1, 0 }, // TransNum = 3, Flip = 0, Rot = 0
	{ 0, 2, 0, 1 }, // TransNum = 3, Flip = 0, Rot = 1
	{ 2, 0, 0, 2 }, // TransNum = 3, Flip = 0, Rot = 2
	{ 1, 0, 2, 0 }, // TransNum = 3, Flip = 0, Rot = 3
	{ 0, 2, 0, 1 }, // TransNum = 3, Flip = 1, Rot = 0
	{ 2, 0, 0, 2 }, // TransNum = 3, Flip = 1, Rot = 1
	{ 1, 0, 2, 0 }, // TransNum = 3, Flip = 1, Rot = 2
	{ 0, 1, 1, 0 }, // TransNum = 3, Flip = 1, Rot = 3

	{ 0, 2, 1, 0 }, // TransNum = 4, Flip = 0, Rot = 0
	{ 0, 2, 0, 2 }, // TransNum = 4, Flip = 0, Rot = 1
	{ 1, 0, 0, 2 }, // TransNum = 4, Flip = 0, Rot = 2
	{ 1, 0, 1, 0 }, // TransNum = 4, Flip = 0, Rot = 3
	{ 0, 1, 0, 1 }, // TransNum = 4, Flip = 1, Rot = 0
	{ 2, 0, 0, 1 }, // TransNum = 4, Flip = 1, Rot = 1
	{ 2, 0, 2, 0 }, // TransNum = 4, Flip = 1, Rot = 2
	{ 0, 1, 2, 0 }, // TransNum = 4, Flip = 1, Rot = 3

	{ 0, 2, 2, 0 }, // TransNum = 5, Flip = 0, Rot = 0
	{ 0, 1, 0, 2 }, // TransNum = 5, Flip = 0, Rot = 1
	{ 1, 0, 0, 1 }, // TransNum = 5, Flip = 0, Rot = 2
	{ 2, 0, 1, 0 }, // TransNum = 5, Flip = 0, Rot = 3
	{ 0, 1, 0, 2 }, // TransNum = 5, Flip = 1, Rot = 0
	{ 1, 0, 0, 1 }, // TransNum = 5, Flip = 1, Rot = 1
	{ 2, 0, 1, 0 }, // TransNum = 5, Flip = 1, Rot = 2
	{ 0, 2, 2, 0 }, // TransNum = 5, Flip = 1, Rot = 3

	{ 0, 1, 1, 0 }, // TransNum = 6, Flip = 0, Rot = 0
	{ 0, 2, 0, 1 }, // TransNum = 6, Flip = 0, Rot = 1
	{ 2, 0, 0, 2 }, // TransNum = 6, Flip = 0, Rot = 2
	{ 1, 0, 2, 0 }, // TransNum = 6, Flip = 0, Rot = 3
	{ 0, 2, 0, 1 }, // TransNum = 6, Flip = 1, Rot = 0
	{ 2, 0, 0, 2 }, // TransNum = 6, Flip = 1, Rot = 1
	{ 1, 0, 2, 0 }, // TransNum = 6, Flip = 1, Rot = 2
	{ 0, 1, 1, 0 }, // TransNum = 6, Flip = 1, Rot = 3

	{ 0, 2, 1, 0 }, // TransNum = 7, Flip = 0, Rot = 0
	{ 0, 2, 0, 2 }, // TransNum = 7, Flip = 0, Rot = 1
	{ 1, 0, 0, 2 }, // TransNum = 7, Flip = 0, Rot = 2
	{ 1, 0, 1, 0 }, // TransNum = 7, Flip = 0, Rot = 3
	{ 0, 1, 0, 1 }, // TransNum = 7, Flip = 1, Rot = 0
	{ 2, 0, 0, 1 }, // TransNum = 7, Flip = 1, Rot = 1
	{ 2, 0, 2, 0 }, // TransNum = 7, Flip = 1, Rot = 2
	{ 0, 1, 2, 0 }, // TransNum = 7, Flip = 1, Rot = 3

	{ 0, 2, 2, 0 }, // TransNum = 8, Flip = 0, Rot = 0
	{ 0, 1, 0, 2 }, // TransNum = 8, Flip = 0, Rot = 1
	{ 1, 0, 0, 1 }, // TransNum = 8, Flip = 0, Rot = 2
	{ 2, 0, 1, 0 }, // TransNum = 8, Flip = 0, Rot = 3
	{ 0, 1, 0, 2 }, // TransNum = 8, Flip = 1, Rot = 0
	{ 1, 0, 0, 1 }, // TransNum = 8, Flip = 1, Rot = 1
	{ 2, 0, 1, 0 }, // TransNum = 8, Flip = 1, Rot = 2
	{ 0, 2, 2, 0 }, // TransNum = 8, Flip = 1, Rot = 3
};
void CBuilderZoneRegion::addTransition (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	uint32 i;
	// Check that we write in an already defined place
	if ((x < pBZR->getMinX ()) || (x > pBZR->getMaxX ()) ||
		(y < pBZR->getMinY ()) || (y > pBZR->getMaxY ()))
		return;

	// Check size
	if ((pElt->getSizeX() != 1) || (pElt->getSizeY() != 1))
		return;

	// Check that an element already exist at position we want put the transition
	CZoneBankElement *pEltUnder = _ZeBank->getElementByZoneName(pBZR->getName (x, y));
	if (pEltUnder == NULL)
		return;

	// And check that this element is also a transition and the same transition
	if (pEltUnder->getCategory ("transname") == STRING_NO_CAT_TYPE)
		return;
	if (pEltUnder->getCategory ("transname") != pElt->getCategory ("transname"))
		return;

	string underType = pEltUnder->getCategory ("transtype");
	string overType = pElt->getCategory ("transtype");
	string underNum = pEltUnder->getCategory ("transnum");
	string overNum = pElt->getCategory ("transnum");

	CDatabaseLocator zoneTemp (RegionId, x, y);
	CLigoData dataZoneTemp;
	getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
	CLigoData dataZoneOriginal = dataZoneTemp;

	bool bMustPropagate = false;
	// Same type of transition ?
	if (pEltUnder->getCategory ("transtype") != pElt->getCategory ("transtype"))
	{
		// No so random the cutEdges
		for (i = 0; i < 4; ++i)
		{
			uint8 nCut = (uint8)(1.0f+NLMISC::frand(2.0f));
			NLMISC::clamp (nCut, (uint8)1, (uint8)2);
			
			dataZoneTemp.SharingCutEdges[i] = nCut;
		}
		pElt = NULL;
		bMustPropagate = true;
	}
	else
	{
		// Put exactly the transition as given
		sint32 transnum = atoi (pElt->getCategory ("transnum").c_str());
		sint32 flip = pBZR->getFlip (x, y);
		sint32 rot = pBZR->getRot (x, y);
		sint32 pos1 = -1, pos2 = -1;

		for (i = 0; i < 4; ++i)
		{
			if ((TransToEdge[transnum*8+flip*4+rot][i] != 0) && 
				(TransToEdge[transnum*8+flip*4+rot][i] != dataZoneTemp.SharingCutEdges[i]))
				bMustPropagate = true;
			
			dataZoneTemp.SharingCutEdges[i] = TransToEdge[transnum*8+flip*4+rot][i];

			if ((pos1 != -1) && (dataZoneTemp.SharingCutEdges[i] != 0))
				pos2 = i;
			if ((pos1 == -1) && (dataZoneTemp.SharingCutEdges[i] != 0))
				pos1 = i;
		}
		// Exchange cutedges != 0 one time /2 to permit all positions
		if ((transnum == 1) || (transnum == 4) || (transnum == 7))
		if (pElt->getName() == pEltUnder->getName())
		{
			bMustPropagate = true;

			dataZoneTemp.SharingCutEdges[pos1] = 3 - dataZoneTemp.SharingCutEdges[pos1];
			dataZoneTemp.SharingCutEdges[pos2] = 3 - dataZoneTemp.SharingCutEdges[pos2];
		}
	}
	if (dataZoneTemp != dataZoneOriginal)
	{
		getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
	}
	updateTrans (x, y, pElt);

	// If the transition number is not the same propagate the change
	if (bMustPropagate)
	{
		// Propagate where the edge is cut (1/3 or 2/3) and update the transition
		if (dataZoneTemp.SharingMatNames[0] != dataZoneTemp.SharingMatNames[2])
		{
			if (x > pBZR->getMinX ())
			{	// [x-1][y].right = [x][y].left
				// _Zones[(x-1-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[3] = dataZoneTemp.SharingCutEdges[2];
				CDatabaseLocator zoneTemp2 (RegionId, x-1, y);
				CLigoData dataZoneTemp2;
				getDocument ()->getLigoData (dataZoneTemp2, zoneTemp2);
				if (dataZoneTemp2.SharingCutEdges[3] != dataZoneTemp.SharingCutEdges[2])
				{
					dataZoneTemp2.SharingCutEdges[3] = dataZoneTemp.SharingCutEdges[2];
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp2, zoneTemp2));
				}
			}
			updateTrans (x-1, y);
		}
		if (dataZoneTemp.SharingMatNames[1] != dataZoneTemp.SharingMatNames[3])
		{
			if (x < pBZR->getMaxX ())
			{	// [x+1][y].left = [x][y].right
				//_Zones[(x+1-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[2] = dataZoneTemp.SharingCutEdges[3]; 
				CDatabaseLocator zoneTemp2 (RegionId, x+1, y);
				CLigoData dataZoneTemp2;
				getDocument ()->getLigoData (dataZoneTemp2, zoneTemp2);
				if (dataZoneTemp2.SharingCutEdges[2] != dataZoneTemp.SharingCutEdges[3])
				{
					dataZoneTemp2.SharingCutEdges[2] = dataZoneTemp.SharingCutEdges[3];
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp2, zoneTemp2));
				}
			}
			updateTrans (x+1, y);
		}
		if (dataZoneTemp.SharingMatNames[0] != dataZoneTemp.SharingMatNames[1])
		{
			if (y > pBZR->getMinY ())
			{	// [x][y-1].up = [x][y].down
				//_Zones[(x-pBZR->getMinX ())+(y-1-pBZR->getMinY ())*stride].SharingCutEdges[0] = dataZoneTemp.SharingCutEdges[1]; 
				CDatabaseLocator zoneTemp2 (RegionId, x, y-1);
				CLigoData dataZoneTemp2;
				getDocument ()->getLigoData (dataZoneTemp2, zoneTemp2);
				if (dataZoneTemp2.SharingCutEdges[0] != dataZoneTemp.SharingCutEdges[1])
				{
					dataZoneTemp2.SharingCutEdges[0] = dataZoneTemp.SharingCutEdges[1];
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp2, zoneTemp2));
				}
			}
			updateTrans (x, y-1);
		}
		if (dataZoneTemp.SharingMatNames[2] != dataZoneTemp.SharingMatNames[3])
		{
			if (y < pBZR->getMaxY ())
			{	// [x][y+1].down = [x][y].up
				//_Zones[(x-pBZR->getMinX ())+(y+1-pBZR->getMinY ())*stride].SharingCutEdges[1] = dataZoneTemp.SharingCutEdges[0]; 
				CDatabaseLocator zoneTemp2 (RegionId, x, y+1);
				CLigoData dataZoneTemp2;
				getDocument ()->getLigoData (dataZoneTemp2, zoneTemp2);
				if (dataZoneTemp2.SharingCutEdges[1] != dataZoneTemp.SharingCutEdges[0])
				{
					dataZoneTemp2.SharingCutEdges[1] = dataZoneTemp.SharingCutEdges[0];
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp2, zoneTemp2));
				}
			}
			updateTrans (x, y+1);
		}
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::addToUpdateAndCreate (CBuilderZoneRegion *pBZRfrom, sint32 sharePos, sint32 x, sint32 y, const string &sNewMat, void *pInt1, void *pInt2)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	CToUpdate *ptCreate = (CToUpdate*)pInt1;
	CToUpdate *ptUpdate = (CToUpdate*)pInt2;
	sint32 stride = (1+pBZR->getMaxX ()-pBZR->getMinX ());

	CDatabaseLocator locator;
	if (getDocument ()->getZoneAmongRegions (locator, pBZRfrom, x, y))
	{
		CLigoData data;
		getDocument ()->getLigoData (data, locator);
		if (data.SharingMatNames[sharePos] != sNewMat)
		{
			data.SharingMatNames[sharePos] = sNewMat;
			getDocument ()->addModification (new CActionLigoTile (data, locator));
		}
		pBZRfrom->del (x, y, true, ptUpdate);
		ptCreate->add (pBZRfrom, x, y, sNewMat);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::putTransitions (sint32 inX, sint32 inY, const SPiece &rMask, const string &MatName,
										 void *pInternal)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	CToUpdate tCreate; // Transition to create
	CToUpdate *ptUpdate = (CToUpdate*)pInternal; // Transition to update

	sint32 i, j, k, l, m;
	sint32 x = inX, y = inY;
	for (j = 0; j < rMask.h; ++j)
	for (i = 0; i < rMask.w; ++i)
	if (rMask.Tab[i+j*rMask.w])
	{
		for (k = -1; k <= 1; ++k)
		for (l = -1; l <= 1; ++l)
		{
			CBuilderZoneRegion *pBZR2 = this;
			CDatabaseLocator locator;
			if (getDocument ()->getZoneAmongRegions (locator, pBZR2, inX+i+l, inY+j+k))
				tCreate.add (pBZR2, inX+i+l, inY+j+k, MatName);
		}
	}
	
	// Check coherency of the transition to update
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		CBuilderZoneRegion *pBZR2 = tCreate.getBZR(m);
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		string putMat = tCreate.getMat(m);

		//if ((x < pBZR->getMinX ())||(x > pBZR->getMaxX ())||(y < pBZR->getMinY ())||(y > pBZR->getMaxY ()))
		//	continue;
		
		CDatabaseLocator zoneTemp (pBZR2->RegionId, x, y);
		CLigoData dataZoneTemp;
		getDocument ()->getLigoData (dataZoneTemp, zoneTemp);

		if (!((dataZoneTemp.SharingMatNames[0] == dataZoneTemp.SharingMatNames[1])&&
			(dataZoneTemp.SharingMatNames[1] == dataZoneTemp.SharingMatNames[2])&&
			(dataZoneTemp.SharingMatNames[2] == dataZoneTemp.SharingMatNames[3])))
			pBZR2->del (x, y, true, ptUpdate);

		// Check to see material can be posed
		uint corner;
		for (corner = 0; corner < 4; corner++)
		{
			string newMat = getNextMatInTree (putMat, dataZoneTemp.SharingMatNames[corner]);

			// Can't be posed ?
			if (newMat == STRING_UNUSED)
				break;
		}
		if ( (corner < 4) && (m != 0) )
		{
			// The material can't be paused
			dataZoneTemp.SharingMatNames[0] = STRING_UNUSED;
			dataZoneTemp.SharingMatNames[1] = STRING_UNUSED;
			dataZoneTemp.SharingMatNames[2] = STRING_UNUSED;
			dataZoneTemp.SharingMatNames[3] = STRING_UNUSED;

			// Don't propagate any more
		}
		else
		{
			// Expand material for the 1st quarter
			string newMat = getNextMatInTree (putMat, dataZoneTemp.SharingMatNames[0]);
			if (newMat != dataZoneTemp.SharingMatNames[0])
			{	// Update the quarter
				if (dataZoneTemp.SharingMatNames[0] != newMat)
				{
					dataZoneTemp.SharingMatNames[0] = newMat;
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
				}

				addToUpdateAndCreate (pBZR2, 1, x-1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 3, x-1, y-1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 2, x, y-1, newMat, &tCreate, ptUpdate);
			}

			// Expand material for the 2nd quarter
			newMat = getNextMatInTree (putMat, dataZoneTemp.SharingMatNames[1]);
			if (newMat != dataZoneTemp.SharingMatNames[1])
			{	// Update the quarter
				//if (_Builder->getZoneMask(x,y))
				if (dataZoneTemp.SharingMatNames[1] != newMat)
				{
					dataZoneTemp.SharingMatNames[1] = newMat;
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
				}

				addToUpdateAndCreate (pBZR2, 0, x+1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 2, x+1, y-1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 3, x, y-1, newMat, &tCreate, ptUpdate);
			}

			// Expand material for the 3rd quarter
			newMat = getNextMatInTree (putMat, dataZoneTemp.SharingMatNames[2]);
			if (newMat != dataZoneTemp.SharingMatNames[2])
			{	// Update the quarter
				//if (_Builder->getZoneMask(x,y))
				if (dataZoneTemp.SharingMatNames[2] != newMat)
				{
					dataZoneTemp.SharingMatNames[2] = newMat;
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
				}
					
				addToUpdateAndCreate (pBZR2, 3, x-1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 1, x-1, y+1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 0, x, y+1, newMat, &tCreate, ptUpdate);
			}

			// Expand material for the 4th quarter
			newMat = getNextMatInTree (putMat, dataZoneTemp.SharingMatNames[3]);
			if (newMat != dataZoneTemp.SharingMatNames[3])
			{	// Update the quarter
				//if (_Builder->getZoneMask(x,y))
				if (dataZoneTemp.SharingMatNames[3] != newMat)
				{
					dataZoneTemp.SharingMatNames[3] = newMat;
					getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
				}

				addToUpdateAndCreate (pBZR2, 2, x+1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 0, x+1, y+1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate (pBZR2, 1, x, y+1, newMat, &tCreate, ptUpdate);
			}
		}
		getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
	}

	// Delete transitions that are inside the mask
	for (j = 0; j < rMask.h; ++j)
	for (i = 0; i < rMask.w; ++i)
	if (rMask.Tab[i+j*rMask.w])
	{
		tCreate.del (inX+i, inY+j);
	}

	// For all transition to update choose the cut edge
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		const CZoneRegion *pBZR2 = &(getDocument ()->getZoneRegion (tCreate.getBZR(m)->RegionId));
		x = tCreate.getX(m);
		y = tCreate.getY(m);

		if ((x < pBZR2->getMinX ())||(x > pBZR2->getMaxX ())||(y < pBZR2->getMinY ())||(y > pBZR2->getMaxY ()))
			continue;

		CDatabaseLocator zoneTemp (tCreate.getBZR(m)->RegionId, x, y);
		CLigoData dataZoneTemp;
		getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
		CLigoData dataZoneTempOriginal = dataZoneTemp;

		for (i = 0; i < 4; ++i)
		{
			uint8 nCut = (uint8)(1.0f+NLMISC::frand(2.0f));
			NLMISC::clamp (nCut, (uint8)1, (uint8)2);
			dataZoneTemp.SharingCutEdges[i] = nCut;
		}

		if (dataZoneTempOriginal != dataZoneTemp)
			getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));

		// Propagate
		if (dataZoneTemp.SharingMatNames[0] != dataZoneTemp.SharingMatNames[2])
		{	// [x-1][y].right = [x][y].left
			CBuilderZoneRegion *pBZR3 = tCreate.getBZR(m);
			CDatabaseLocator pZU;
			if (getDocument ()->getZoneAmongRegions (pZU, pBZR3, x-1, y))
			{
				CLigoData data;
				getDocument ()->getLigoData (data, pZU);
				if (data.SharingCutEdges[3] != dataZoneTemp.SharingCutEdges[2])
				{
					data.SharingCutEdges[3] = dataZoneTemp.SharingCutEdges[2];
					getDocument ()->addModification (new CActionLigoTile (data, pZU));
				}
				ptUpdate->add (pBZR3, x-1, y, "");
			}
		}
		else
		{
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[2] != 0)
			{
				dataZoneTemp.SharingCutEdges[2] = 0;
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}

		if (dataZoneTemp.SharingMatNames[0] != dataZoneTemp.SharingMatNames[1])
		{	// [x][y-1].up = [x][y].down
			CBuilderZoneRegion *pBZR3 = tCreate.getBZR(m);
			CDatabaseLocator pZU;
			if (getDocument ()->getZoneAmongRegions (pZU, pBZR3, x, y-1))
			{
				CLigoData data;
				getDocument ()->getLigoData (data, pZU);
				if (data.SharingCutEdges[0] != dataZoneTemp.SharingCutEdges[1])
				{
					data.SharingCutEdges[0] = dataZoneTemp.SharingCutEdges[1];
					getDocument ()->addModification (new CActionLigoTile (data, pZU));
				}
				ptUpdate->add (pBZR3, x, y-1, "");
			}
		}
		else
		{
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[1] != 0)
			{
				dataZoneTemp.SharingCutEdges[1] = 0;
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}

		if (dataZoneTemp.SharingMatNames[3] != dataZoneTemp.SharingMatNames[1])
		{	// [x+1][y].left = [x][y].right
			CBuilderZoneRegion *pBZR3 = tCreate.getBZR(m);
			CDatabaseLocator pZU;
			if (getDocument ()->getZoneAmongRegions (pZU, pBZR3, x+1, y))
			{
				CLigoData data;
				getDocument ()->getLigoData (data, pZU);
				if (data.SharingCutEdges[2] != dataZoneTemp.SharingCutEdges[3])
				{
					data.SharingCutEdges[2] = dataZoneTemp.SharingCutEdges[3]; 
					getDocument ()->addModification (new CActionLigoTile (data, pZU));
				}
				ptUpdate->add (pBZR3, x+1, y, "");
			}
		}
		else
		{
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[3] != 0)
			{
				dataZoneTemp.SharingCutEdges[3] = 0;
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}

		if (dataZoneTemp.SharingMatNames[2] != dataZoneTemp.SharingMatNames[3])
		{	// [x][y+1].down = [x][y].up
			CBuilderZoneRegion *pBZR3 = tCreate.getBZR(m);
			CDatabaseLocator pZU;
			if (getDocument ()->getZoneAmongRegions (pZU, pBZR3, x, y+1))
			{
				CLigoData data;
				getDocument ()->getLigoData (data, pZU);
				if (data.SharingCutEdges[1] = dataZoneTemp.SharingCutEdges[0])
				{
					data.SharingCutEdges[1] = dataZoneTemp.SharingCutEdges[0];
					getDocument ()->addModification (new CActionLigoTile (data, pZU));
				}
				ptUpdate->add (pBZR3, x, y+1, "");
			}
		}
		else
		{
			getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
			if (dataZoneTemp.SharingCutEdges[0] = 0)
			{
				dataZoneTemp.SharingCutEdges[0] = 0;
				getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
			}
		}
	}	

	// Delete in tUpdate each element in common with tCreate
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{		
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		ptUpdate->del (x,y);
	}

	// Finally update all transition
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		const CZoneRegion *pBZR2 = &(getDocument ()->getZoneRegion (tCreate.getBZR(m)->RegionId));
		x = tCreate.getX(m);
		y = tCreate.getY(m);

		if ((x >= pBZR2->getMinX ())&&(x <= pBZR2->getMaxX ())&&(y >= pBZR2->getMinY ())&&(y <= pBZR2->getMaxY ()))
			tCreate.getBZR(m)->updateTrans (x, y);
	}
	for (m = 0; m < (sint32)ptUpdate->size(); ++m)
	{
		const CZoneRegion *pBZR2 = &(getDocument ()->getZoneRegion (tCreate.getBZR(m)->RegionId));
		x = ptUpdate->getX(m);
		y = ptUpdate->getY(m);
		if ((x >= pBZR2->getMinX ())&&(x <= pBZR2->getMaxX ())&&(y >= pBZR2->getMinY ())&&(y <= pBZR2->getMaxY ()))
			tCreate.getBZR(m)->updateTrans (x, y);
	}

	// Cross material
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		const CZoneRegion *pBZR2 = &(getDocument ()->getZoneRegion (tCreate.getBZR(m)->RegionId));
		x = tCreate.getX(m);
		y = tCreate.getY(m);


		CDatabaseLocator zoneTemp (RegionId, x, y);
		CLigoData dataZoneTemp;
		getDocument ()->getLigoData (dataZoneTemp, zoneTemp);

		std::set<string> matNameSet;
		for (i = 0; i < 4; ++i)
			matNameSet.insert (dataZoneTemp.SharingMatNames[i]);

		if (((dataZoneTemp.SharingMatNames[0] == dataZoneTemp.SharingMatNames[3]) &&
			(dataZoneTemp.SharingMatNames[1] == dataZoneTemp.SharingMatNames[2]) &&
			(dataZoneTemp.SharingMatNames[0] != dataZoneTemp.SharingMatNames[1])) 
			|| (matNameSet.size()>2))
		{

			_ZeBank->resetSelection ();
			_ZeBank->addOrSwitch ("material", tCreate.getMat(m));
			_ZeBank->addAndSwitch ("size", "1x1");
			vector<CZoneBankElement*> vElts;
			_ZeBank->getSelection (vElts);
			if (vElts.size() == 0)
				return;
			sint32 nRan = (sint32)(NLMISC::frand((float)vElts.size()));
			NLMISC::clamp (nRan, (sint32)0, (sint32)(vElts.size()-1));
			CZoneBankElement *pZBE = vElts[nRan];
			nRan = (uint32)(NLMISC::frand (1.0) * 4);
			NLMISC::clamp (nRan, (sint32)0, (sint32)3);
			uint8 rot = (uint8)nRan;
			nRan = (uint32)(NLMISC::frand (1.0) * 2);
			NLMISC::clamp (nRan, (sint32)0, (sint32)1);
			uint8 flip = (uint8)nRan;

			tCreate.getBZR(m)->add (x, y, rot, flip, pZBE);
		}
	}

}


// ---------------------------------------------------------------------------
/*void CBuilderZoneRegion::putTransition (sint32 x, sint32 y, const string &MatName)
{
	const string &rSZone = getName (x, y);
	if (rSZone != STRING_UNUSED)
		return;

	sint32 stride = (1+pBZR->getMaxX ()-pBZR->getMinX ());
	sint32 m;
	// Set Random edges
	for (m = 0; m < 4; ++m)
	{
		uint8 nCut = (uint8)(1.0f+NLMISC::frand(2.0f));
		NLMISC::clamp (nCut, (uint8)1, (uint8)2);
		_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[m] = nCut;
	}
	// Propagate
	if (x > pBZR->getMinX ())
	{	// [x-1][y].right = [x][y].left
		_Zones[(x-1-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[3] = 
			_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[2]; 
	}
	if (y > pBZR->getMinY ())
	{	// [x][y-1].up = [x][y].down
		_Zones[(x-pBZR->getMinX ())+(y-1-pBZR->getMinY ())*stride].SharingCutEdges[0] = 
			_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[1]; 
	}
	if (x < pBZR->getMaxX ())
	{	// [x+1][y].left = [x][y].right
		_Zones[(x+1-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[2] = 
			_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[3]; 
	}
	if (y < pBZR->getMaxY ())
	{	// [x][y+1].down = [x][y].up
		_Zones[(x-pBZR->getMinX ())+(y+1-pBZR->getMinY ())*stride].SharingCutEdges[1] = 
			_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[0]; 
	}

	// Update Transitions
	updateTrans (x, y);
	if (_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[0] != _Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[2])
		updateTrans (x-1, y);
	if (_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[1] != _Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[3])
		updateTrans (x+1, y);
	if (_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[0] != _Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[1])
		updateTrans (x, y-1);
	if (_Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[2] != _Zones[(x-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingMatNames[3])
		updateTrans (x, y+1);
}
*/
// ---------------------------------------------------------------------------
struct STrans
{
	uint8 Num;
	uint8 Rot;
	uint8 Flip;
};

// ---------------------------------------------------------------------------
STrans TranConvTable[128] =
{
	{ 0,0,0 }, // Quart = 0, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 3, Np = 1 UNUSED

	{ 6,0,0 }, // Quart = 1, CutEdge = 0, Np = 0
	{ 6,3,1 }, // Quart = 1, CutEdge = 0, Np = 1
	{ 7,0,0 }, // Quart = 1, CutEdge = 1, Np = 0
	{ 7,0,0 }, // Quart = 1, CutEdge = 1, Np = 1
	{ 7,3,1 }, // Quart = 1, CutEdge = 2, Np = 0
	{ 7,3,1 }, // Quart = 1, CutEdge = 2, Np = 1
	{ 8,0,0 }, // Quart = 1, CutEdge = 3, Np = 0
	{ 8,3,1 }, // Quart = 1, CutEdge = 3, Np = 1

	{ 7,0,1 }, // Quart = 2, CutEdge = 0, Np = 0
	{ 7,0,1 }, // Quart = 2, CutEdge = 0, Np = 1
	{ 6,0,1 }, // Quart = 2, CutEdge = 1, Np = 0
	{ 6,1,0 }, // Quart = 2, CutEdge = 1, Np = 1
	{ 8,1,0 }, // Quart = 2, CutEdge = 2, Np = 0
	{ 8,0,1 }, // Quart = 2, CutEdge = 2, Np = 1
	{ 7,1,0 }, // Quart = 2, CutEdge = 3, Np = 0
	{ 7,1,0 }, // Quart = 2, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 3, CutEdge = 0, Np = 0
	{ 0,0,1 }, // Quart = 3, CutEdge = 0, Np = 1
	{ 1,0,1 }, // Quart = 3, CutEdge = 1, Np = 0
	{ 1,0,1 }, // Quart = 3, CutEdge = 1, Np = 1
	{ 1,0,0 }, // Quart = 3, CutEdge = 2, Np = 0
	{ 1,0,0 }, // Quart = 3, CutEdge = 2, Np = 1
	{ 2,0,0 }, // Quart = 3, CutEdge = 3, Np = 0
	{ 2,0,1 }, // Quart = 3, CutEdge = 3, Np = 1

	{ 7,3,0 }, // Quart = 4, CutEdge = 0, Np = 0
	{ 7,3,0 }, // Quart = 4, CutEdge = 0, Np = 1
	{ 8,3,0 }, // Quart = 4, CutEdge = 1, Np = 0
	{ 8,2,1 }, // Quart = 4, CutEdge = 1, Np = 1
	{ 6,3,0 }, // Quart = 4, CutEdge = 2, Np = 0
	{ 6,2,1 }, // Quart = 4, CutEdge = 2, Np = 1
	{ 7,2,1 }, // Quart = 4, CutEdge = 3, Np = 0
	{ 7,2,1 }, // Quart = 4, CutEdge = 3, Np = 1

	{ 0,3,0 }, // Quart = 5, CutEdge = 0, Np = 0
	{ 0,3,1 }, // Quart = 5, CutEdge = 0, Np = 1
	{ 1,3,1 }, // Quart = 5, CutEdge = 1, Np = 0
	{ 1,3,1 }, // Quart = 5, CutEdge = 1, Np = 1
	{ 1,3,0 }, // Quart = 5, CutEdge = 2, Np = 0
	{ 1,3,0 }, // Quart = 5, CutEdge = 2, Np = 1
	{ 2,3,0 }, // Quart = 5, CutEdge = 3, Np = 0
	{ 2,3,1 }, // Quart = 5, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 6, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 3, Np = 1 UNUSED

	{ 5,2,0 }, // Quart = 7, CutEdge = 0, Np = 0
	{ 5,1,1 }, // Quart = 7, CutEdge = 0, Np = 1
	{ 4,1,1 }, // Quart = 7, CutEdge = 1, Np = 0
	{ 4,1,1 }, // Quart = 7, CutEdge = 1, Np = 1
	{ 4,2,0 }, // Quart = 7, CutEdge = 2, Np = 0
	{ 4,2,0 }, // Quart = 7, CutEdge = 2, Np = 1
	{ 3,2,0 }, // Quart = 7, CutEdge = 3, Np = 0
	{ 3,1,1 }, // Quart = 7, CutEdge = 3, Np = 1

	{ 8,2,0 }, // Quart = 8, CutEdge = 0, Np = 0
	{ 8,1,1 }, // Quart = 8, CutEdge = 0, Np = 1
	{ 7,1,1 }, // Quart = 8, CutEdge = 1, Np = 0
	{ 7,1,1 }, // Quart = 8, CutEdge = 1, Np = 1
	{ 7,2,0 }, // Quart = 8, CutEdge = 2, Np = 0
	{ 7,2,0 }, // Quart = 8, CutEdge = 2, Np = 1
	{ 6,2,0 }, // Quart = 8, CutEdge = 3, Np = 0
	{ 6,1,1 }, // Quart = 8, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 9, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 3, Np = 1 UNUSED

	{ 2,1,0 }, // Quart = 10, CutEdge = 0, Np = 0
	{ 2,1,1 }, // Quart = 10, CutEdge = 0, Np = 1
	{ 1,1,1 }, // Quart = 10, CutEdge = 1, Np = 0
	{ 1,1,1 }, // Quart = 10, CutEdge = 1, Np = 1
	{ 1,1,0 }, // Quart = 10, CutEdge = 2, Np = 0
	{ 1,1,0 }, // Quart = 10, CutEdge = 2, Np = 1
	{ 0,1,0 }, // Quart = 10, CutEdge = 3, Np = 0
	{ 0,1,1 }, // Quart = 10, CutEdge = 3, Np = 1

	{ 4,3,0 }, // Quart = 11, CutEdge = 0, Np = 0
	{ 4,3,0 }, // Quart = 11, CutEdge = 0, Np = 1
	{ 5,3,0 }, // Quart = 11, CutEdge = 1, Np = 0
	{ 5,2,1 }, // Quart = 11, CutEdge = 1, Np = 1
	{ 3,3,0 }, // Quart = 11, CutEdge = 2, Np = 0
	{ 3,2,1 }, // Quart = 11, CutEdge = 2, Np = 1
	{ 4,2,1 }, // Quart = 11, CutEdge = 3, Np = 0
	{ 4,2,1 }, // Quart = 11, CutEdge = 3, Np = 1

	{ 2,2,0 }, // Quart = 12, CutEdge = 0, Np = 0
	{ 2,2,1 }, // Quart = 12, CutEdge = 0, Np = 1
	{ 1,2,1 }, // Quart = 12, CutEdge = 1, Np = 0
	{ 1,2,1 }, // Quart = 12, CutEdge = 1, Np = 1
	{ 1,2,0 }, // Quart = 12, CutEdge = 2, Np = 0
	{ 1,2,0 }, // Quart = 12, CutEdge = 2, Np = 1
	{ 0,2,0 }, // Quart = 12, CutEdge = 3, Np = 0
	{ 0,2,1 }, // Quart = 12, CutEdge = 3, Np = 1

	{ 4,0,1 }, // Quart = 13, CutEdge = 0, Np = 0
	{ 4,0,1 }, // Quart = 13, CutEdge = 0, Np = 1
	{ 3,1,0 }, // Quart = 13, CutEdge = 1, Np = 0
	{ 3,0,1 }, // Quart = 13, CutEdge = 1, Np = 1
	{ 5,1,0 }, // Quart = 13, CutEdge = 2, Np = 0
	{ 5,0,1 }, // Quart = 13, CutEdge = 2, Np = 1
	{ 4,1,0 }, // Quart = 13, CutEdge = 3, Np = 0
	{ 4,1,0 }, // Quart = 13, CutEdge = 3, Np = 1

	{ 3,0,0 }, // Quart = 14, CutEdge = 0, Np = 0
	{ 3,3,1 }, // Quart = 14, CutEdge = 0, Np = 1
	{ 4,0,0 }, // Quart = 14, CutEdge = 1, Np = 0
	{ 4,0,0 }, // Quart = 14, CutEdge = 1, Np = 1
	{ 4,3,1 }, // Quart = 14, CutEdge = 2, Np = 0
	{ 4,3,1 }, // Quart = 14, CutEdge = 2, Np = 1
	{ 5,0,0 }, // Quart = 14, CutEdge = 3, Np = 0
	{ 5,3,1 }, // Quart = 14, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 15, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }  // Quart = 15, CutEdge = 3, Np = 1 UNUSED
};

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::updateTrans (sint32 x, sint32 y, CZoneBankElement *pElt)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	if ((x < pBZR->getMinX ()) || (x > pBZR->getMaxX ()) || (y < pBZR->getMinY ()) || (y > pBZR->getMaxY ()))
		return;

	//if (!_Builder->getZoneMask(x,y))
	//	return;

	// Interpret the transition info
	x -= pBZR->getMinX ();
	y -= pBZR->getMinY ();
	sint32 m;
	// Calculate the number of material around with transition info
	std::set<string> matNameSet;

	CDatabaseLocator zoneTemp (RegionId, x+pBZR->getMinX (), y+pBZR->getMinY ());
	CLigoData dataZoneTemp;
	getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
	CLigoData dataZoneTempOriginal = dataZoneTemp;

	for (m = 0; m < 4; ++m)
		matNameSet.insert (dataZoneTemp.SharingMatNames[m]);

	if (matNameSet.size() == 1)
	{
		if (dataZoneTemp.SharingMatNames[0] == STRING_UNUSED)
		{
			del (x+pBZR->getMinX (), y+pBZR->getMinY ());
			// set (x+pBZR->getMinX (), y+pBZR->getMinY (), 0, 0, STRING_UNUSED, false);
			return;
		}
		else
		{
			CZoneBankElement *pZBE = _ZeBank->getElementByZoneName (dataZoneTemp.ZoneName);
			if ((pZBE != NULL) && (pZBE->getCategory("material")==dataZoneTemp.SharingMatNames[0]))
				return;
			_ZeBank->resetSelection ();
			_ZeBank->addOrSwitch ("material", dataZoneTemp.SharingMatNames[0]);
			_ZeBank->addAndSwitch ("size", "1x1");
			vector<CZoneBankElement*> vElts;
			_ZeBank->getSelection (vElts);
			if (vElts.size() == 0)
				return;
			sint32 nRan = (sint32)(NLMISC::frand((float)vElts.size()));
			NLMISC::clamp (nRan, (sint32)0, (sint32)(vElts.size()-1));
			pZBE = vElts[nRan];
			nRan = (uint32)(NLMISC::frand (1.0) * 4);
			NLMISC::clamp (nRan, (sint32)0, (sint32)3);
			uint8 rot = (uint8)nRan;
			nRan = (uint32)(NLMISC::frand (1.0) * 2);
			NLMISC::clamp (nRan, (sint32)0, (sint32)1);
			uint8 flip = (uint8)nRan;

			set (x+pBZR->getMinX (), y+pBZR->getMinY (), 0, 0, pZBE->getName(), false);
			setRot (x+pBZR->getMinX (), y+pBZR->getMinY (), rot);
			setFlip (x+pBZR->getMinX (), y+pBZR->getMinY (), flip);
			return;
		}
	}

	// No 2 materials so the transition system dont work
	if (matNameSet.size() != 2)
		return;

	std::set<string>::iterator it = matNameSet.begin();
	string sMatA = *it;
	++it;
	string sMatB = *it;

	_ZeBank->resetSelection ();
	_ZeBank->addOrSwitch ("transname", sMatA + "-" + sMatB);
	vector<CZoneBankElement*> selection;
	_ZeBank->getSelection (selection);
	if (selection.size() == 0)
	{
		string sTmp = sMatA;
		sMatA = sMatB;
		sMatB = sTmp;
		_ZeBank->resetSelection ();
		_ZeBank->addOrSwitch ("transname", sMatA + "-" + sMatB);
		_ZeBank->getSelection (selection);
	}

	if (selection.size() == 0)
		return;

	// Convert the sharingCutEdges and SharingNames to the num and type of transition
	uint8 nQuart = 0; // 0-MatA 1-MatB
	for (m = 0; m < 4; ++m)
		if (dataZoneTemp.SharingMatNames[m] == sMatB)
			nQuart |= (1<<m);

	if ((nQuart == 0)||(nQuart == 6)||(nQuart == 9)||(nQuart == 15))
		return; // No transition for those types

	uint8 nCutEdge = 0;
	uint8 nPosCorner = 0;

	// If up edge is cut write the cut position in nCutEdge bitfield (1->0, 2->1)
	if ((nQuart == 4)||(nQuart == 5)||(nQuart == 7)||(nQuart == 8)||(nQuart == 10)||(nQuart == 11))
	{
		if (dataZoneTemp.SharingCutEdges[0] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.SharingCutEdges[0] = 0;
	}

	// Same for down edge
	if ((nQuart == 1)||(nQuart == 2)||(nQuart == 5)||(nQuart == 10)||(nQuart == 13)||(nQuart == 14))
	{
		if (dataZoneTemp.SharingCutEdges[1] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.SharingCutEdges[1] = 0;
	}

	// Same for left edge
	if ((nQuart == 1)||(nQuart == 3)||(nQuart == 4)||(nQuart == 11)||(nQuart == 12)||(nQuart == 14))
	{
		if (dataZoneTemp.SharingCutEdges[2] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.SharingCutEdges[2] = 0;
	}

	// Same for right edge
	if ((nQuart == 2)||(nQuart == 3)||(nQuart == 7)||(nQuart == 8)||(nQuart == 12)||(nQuart == 13))
	{
		if (dataZoneTemp.SharingCutEdges[3] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.SharingCutEdges[3] = 0;
	}

	nlassert (nPosCorner == 2); // If not this means that more than 2 edges are cut which is not possible

	STrans Trans, TransTmp1, TransTmp2;

	TransTmp1 = TranConvTable[nQuart*8+2*nCutEdge+0];
	TransTmp2 = TranConvTable[nQuart*8+2*nCutEdge+1];

	// Choose one or the two
	sint32 nTrans = (sint32)(NLMISC::frand(2.0f));
	NLMISC::clamp (nTrans, (sint32)0, (sint32)1);
	if (nTrans == 0)
		Trans = TransTmp1;
	else
		Trans = TransTmp2;

	_ZeBank->addAndSwitch ("transnum", NLMISC::toString(Trans.Num));
	_ZeBank->getSelection (selection);

	if (selection.size() > 0)
	{
		if (pElt != NULL)
		{
			dataZoneTemp.ZoneName = pElt->getName();
		}
		else
		{
			nTrans = (uint32)(NLMISC::frand (1.0) * selection.size());
			NLMISC::clamp (nTrans, (sint32)0, (sint32)(selection.size()-1));
			dataZoneTemp.ZoneName = selection[nTrans]->getName();
		}
		dataZoneTemp.PosX = dataZoneTemp.PosY = 0;
		dataZoneTemp.Rot = Trans.Rot;
		dataZoneTemp.Flip = Trans.Flip;
	}
	if (dataZoneTempOriginal != dataZoneTemp)
		getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
}

// ---------------------------------------------------------------------------
string CBuilderZoneRegion::getNextMatInTree (const string &sMatA, const string &sMatB)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	uint32 i, posA = 10000, posB = 10000;

	if (sMatA == sMatB)
		return sMatA;

	for (i = 0; i < _MatTree.size(); ++i)
	{
		if (_MatTree[i].Name == sMatA) 
			posA = i;
		if (_MatTree[i].Name == sMatB) 
			posB = i;
	}
	if ((posA == 10000) || (posB == 10000))
		return STRING_UNUSED;
	
	vector<uint32> vTemp;
	tryPath (posA, posB, vTemp);
	if (vTemp.size() <= 1)
		return STRING_UNUSED;
	else
		return _MatTree[vTemp[1]].Name;
}

// ---------------------------------------------------------------------------
struct SNode
{
	sint32 NodePos, Dist, PrevNodePos;

	SNode()
	{
		NodePos = Dist = PrevNodePos = -1;
	}
};

// ---------------------------------------------------------------------------
// Find the fastest way between posA and posB in the MatTree (Dijkstra)
void CBuilderZoneRegion::tryPath (uint32 posA, uint32 posB, vector<uint32> &vPath)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	// Build the adjascence matrix
	vector<sint32> matAdj;
	sint32 nNumNodes = _MatTree.size();
	sint32 i, j, cost;
	matAdj.resize (nNumNodes*nNumNodes, -1);
	for (i = 0; i < nNumNodes; ++i)
		for (j = 0; j < (sint32)_MatTree[i].Arcs.size(); ++j)
			matAdj[i+_MatTree[i].Arcs[j]*nNumNodes] = 1;

	vector<SNode> vNodes; // NodesPos == index
	vNodes.resize (nNumNodes);
	for (i = 0; i < nNumNodes; ++i)
		vNodes[i].NodePos = i;
	vNodes[posA].Dist = 0;

	queue<SNode> qNodes;
	qNodes.push (vNodes[posA]);

	while (qNodes.size() > 0)
	{
		SNode node = qNodes.front ();
		qNodes.pop ();

		for (i = 0; i < nNumNodes; ++i)
		{
			cost = matAdj[node.NodePos+i*nNumNodes];
			if (cost != -1)	
			{
				if ((vNodes[i].Dist == -1) || (vNodes[i].Dist > (cost+node.Dist)))
				{
					vNodes[i].Dist = cost+node.Dist;
					vNodes[i].PrevNodePos = node.NodePos;
					qNodes.push (vNodes[i]);
				}
			}
		}
	}

	// Get path length
	i = posB;
	j = 0;
	while (i != -1)
	{
		++j;
		i = vNodes[i].PrevNodePos;
	}

	// Write the path in the good order (from posA to posB)
	vPath.resize (j);
	i = posB;
	while (i != -1)
	{
		--j;
		vPath[j] = i;
		i = vNodes[i].PrevNodePos;
	}
}



// ---------------------------------------------------------------------------
void CBuilderZoneRegion::del (sint32 x, sint32 y, bool transition, void *pInternal)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	if (!_Builder->getZoneMask (x,y))
		return;

	const string &rSZone = pBZR->getName (x, y);

	CToUpdate *pUpdate = (CToUpdate *)pInternal;

	CZoneBankElement *pZBE = _ZeBank->getElementByZoneName (rSZone);
	if (pZBE != NULL)
	{
		sint32 sizeX = pZBE->getSizeX(), sizeY = pZBE->getSizeY();
		sint32 posX = pBZR->getPosX (x, y), posY = pBZR->getPosY (x, y);
		uint8 rot = pBZR->getRot (x, y);
		uint8 flip = pBZR->getFlip (x, y);
		sint32 i, j;
		sint32 deltaX, deltaY;

		if (flip == 0)
		{
			switch (rot)
			{
				case 0: deltaX = -posX; deltaY = -posY; break;
				case 1: deltaX = -(sizeY-1-posY); deltaY = -posX; break;
				case 2: deltaX = -(sizeX-1-posX); deltaY = -(sizeY-1-posY); break;
				case 3: deltaX = -posY; deltaY = -(sizeX-1-posX); break;
			}
		}
		else
		{
			switch (rot)
			{
				case 0: deltaX = -(sizeX-1-posX); deltaY = -posY; break;
				case 1: deltaX = -(sizeY-1-posY); deltaY = -(sizeX-1-posX); break;
				case 2: deltaX = -posX; deltaY = -(sizeY-1-posY); break;
				case 3: deltaX = -posY; deltaY = -posX; break;
			}
		}

		SPiece sMask;
		sMask.Tab.resize (sizeX*sizeY);
		for(i = 0; i < sizeX*sizeY; ++i)
			sMask.Tab[i] = pZBE->getMask()[i];
		sMask.w = sizeX;
		sMask.h = sizeY;
		sMask.rotFlip (rot, flip);

		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+deltaX+i, y+deltaY+j, 0, 0, STRING_UNUSED, true);
			setRot (x+deltaX+i, y+deltaY+j, 0);
			setFlip (x+deltaX+i, y+deltaY+j, 0);
			if (pUpdate != NULL)
			{
				pUpdate->add (this, x+deltaX+i, y+deltaY+j, "");
			}
		}
		if (!transition)
			reduceMin ();
	}
	else
	{
		if ((x < pBZR->getMinX ()) || (x > pBZR->getMaxX ()) || (y < pBZR->getMinY ()) || (y > pBZR->getMaxY ()))
			return;

		CDatabaseLocator zoneTemp (RegionId, x, y);
		CLigoData dataZoneTemp;
		getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
		CLigoData dataZoneTempOriginal = dataZoneTemp;

		dataZoneTemp.ZoneName = STRING_UNUSED;
		dataZoneTemp.PosX = 0;
		dataZoneTemp.PosY = 0;

		for (uint32 i = 0; i < 4; ++i)
		{
			dataZoneTemp.SharingMatNames[i] = STRING_UNUSED;
			dataZoneTemp.SharingCutEdges[i] = 0;
		}

		if (dataZoneTempOriginal != dataZoneTemp)
			getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
		
	}
}

// ---------------------------------------------------------------------------

void CBuilderZoneRegion::move (sint32 x, sint32 y)
{
	getDocument ()->addModification (new CActionLigoMove (getDocument()->regionIDToDatabaseElementID(RegionId), x, y));
}

// ---------------------------------------------------------------------------
uint32 CBuilderZoneRegion::countZones ()
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	sint32 x, y;

	uint32 counter = 0;
	
	for (y = pBZR->getMinY (); y <= pBZR->getMaxY (); ++y)
	for (x = pBZR->getMinX (); x <= pBZR->getMaxX (); ++x)
	if (pBZR->getName (x, y) != STRING_UNUSED)
		++counter;

	return counter;
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::set (sint32 x, sint32 y, sint32 PosX, sint32 PosY, 
						const std::string &ZoneName, bool transition)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	// Do we need to resize ?
	if ((x < pBZR->getMinX ()) || (x > pBZR->getMaxX ()) ||
		(y < pBZR->getMinY ()) || (y > pBZR->getMaxY ()))
	{
		sint32 newMinX = (x<pBZR->getMinX ()?x:pBZR->getMinX ()), newMinY = (y<pBZR->getMinY ()?y:pBZR->getMinY ());
		sint32 newMaxX = (x>pBZR->getMaxX ()?x:pBZR->getMaxX ()), newMaxY = (y>pBZR->getMaxY ()?y:pBZR->getMaxY ());

		resize (newMinX, newMaxX, newMinY, newMaxY);
	}

	CDatabaseLocator zoneTemp (RegionId, x, y);
	CLigoData dataZoneTemp;
	getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
	CLigoData dataZoneTempOriginal = dataZoneTemp;

	dataZoneTemp.ZoneName = ZoneName;
	dataZoneTemp.PosX = (uint8)PosX;
	dataZoneTemp.PosY = (uint8)PosY;
	
	if (!transition)
	{
		CZoneBankElement *pZBE = _ZeBank->getElementByZoneName (ZoneName);
		if (pZBE == NULL)
			return;
		const string &sMatName = pZBE->getCategory ("material");
		if (sMatName == STRING_NO_CAT_TYPE)
			return;
		for (uint32 i = 0; i < 4; ++i)
		{
			dataZoneTemp.SharingMatNames[i] = sMatName;
			dataZoneTemp.SharingCutEdges[i] = 0;
		}
	
		if (dataZoneTempOriginal != dataZoneTemp)
			getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));

		CBuilderZoneRegion *pBZR = this;
		CDatabaseLocator pZU;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x-1, y-1))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if (data.SharingMatNames[3] != sMatName)
			{
				data.SharingMatNames[3] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
		pBZR = this;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x, y-1))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if ((data.SharingMatNames[2] != sMatName) || (data.SharingMatNames[3] != sMatName))
			{
				data.SharingMatNames[2] = sMatName;
				data.SharingMatNames[3] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
		pBZR = this;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x+1, y-1))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if (data.SharingMatNames[2] != sMatName)
			{
				data.SharingMatNames[2] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
		pBZR = this;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x-1, y))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if ((data.SharingMatNames[1] != sMatName) || (data.SharingMatNames[3] != sMatName))
			{
				data.SharingMatNames[1] = sMatName;
				data.SharingMatNames[3] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
		pBZR = this;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x+1, y))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if ((data.SharingMatNames[0] != sMatName) || (data.SharingMatNames[2] != sMatName))
			{
				data.SharingMatNames[0] = sMatName;
				data.SharingMatNames[2] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
		pBZR = this;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x-1, y+1))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if (data.SharingMatNames[1] != sMatName)
			{
				data.SharingMatNames[1] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
		pBZR = this;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x, y+1))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if ((data.SharingMatNames[0] != sMatName) || (data.SharingMatNames[1] != sMatName))
			{
				data.SharingMatNames[0] = sMatName;
				data.SharingMatNames[1] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
		pBZR = this;
		if (getDocument ()->getZoneAmongRegions (pZU, pBZR, x+1, y+1))
		{
			CLigoData data;
			getDocument ()->getLigoData (data, pZU);
			if (data.SharingMatNames[0] != sMatName)
			{
				data.SharingMatNames[0] = sMatName;
				getDocument ()->addModification (new CActionLigoTile (data, pZU));
			}
		}
	}
	else
	{
		if (dataZoneTempOriginal != dataZoneTemp)
			getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::setRot (sint32 x, sint32 y, uint8 rot)
{
	CDatabaseLocator zoneTemp (RegionId, x, y);
	CLigoData dataZoneTemp;
	getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
	if (dataZoneTemp.Rot != rot)
	{
		dataZoneTemp.Rot = rot;
		getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::setFlip (sint32 x, sint32 y, uint8 flip)
{
	CDatabaseLocator zoneTemp (RegionId, x, y);
	CLigoData dataZoneTemp;
	getDocument ()->getLigoData (dataZoneTemp, zoneTemp);
	if (dataZoneTemp.Flip != flip)
	{
		dataZoneTemp.Flip = flip;
		getDocument ()->addModification (new CActionLigoTile (dataZoneTemp, zoneTemp));
	}
}


// ---------------------------------------------------------------------------
void CBuilderZoneRegion::reduceMin ()
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	sint32 i, j;

	sint32 newMinX = pBZR->getMinX (), newMinY = pBZR->getMinY ();
	sint32 newMaxX = pBZR->getMaxX (), newMaxY = pBZR->getMaxY ();
	bool bCanSuppr;

	// Reduce the MinY
	while (true)
	{
		if (newMinY == newMaxY)
			break;
		j = newMinY;
		bCanSuppr = true;
		for (i = newMinX; i <= newMaxX; ++i)
		{
			string str = pBZR->getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			++newMinY;
		else
			break;
	}

	// Reduce the MaxY
	while (true)
	{
		if (newMinY == newMaxY)
			break;
		j = newMaxY;
		bCanSuppr = true;
		for (i = newMinX; i <= newMaxX; ++i)
		{
			string str = pBZR->getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			--newMaxY;
		else
			break;
	}

	// Reduce the MinX
	while (true)
	{
		if (newMinX == newMaxX)
			break;
		i = newMinX;
		bCanSuppr = true;
		for (j = newMinY; j <= newMaxY; ++j)
		{
			string str = pBZR->getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			++newMinX;
		else
			break;
	}

	// Reduce the MaxX
	while (true)
	{
		if (newMinX == newMaxX)
			break;
		i = newMaxX;
		bCanSuppr = true;
		for (j = newMinY; j <= newMaxY; ++j)
		{
			string str = pBZR->getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			--newMaxX;
		else
			break;
	}

	if ((newMinX != pBZR->getMinX ()) || (newMinY != pBZR->getMinY ()) || (newMaxX != pBZR->getMaxX ()) || (newMaxY != pBZR->getMaxY ()))
	{
		resize (newMinX, newMaxX, newMinY, newMaxY);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (RegionId));
	if ((pBZR->getMinX ()!= newMinX) || (pBZR->getMaxX ()!= newMaxX) || (pBZR->getMinY ()!= newMinY) || (pBZR->getMaxY ()!= newMaxY))
	{
		getDocument ()->addModification (new CActionLigoResize (getDocument()->regionIDToDatabaseElementID(RegionId), newMinX, newMaxX, newMinY, newMaxY));
		
	}
}

// ---------------------------------------------------------------------------
