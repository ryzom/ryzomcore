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

#include "std3d.h"

#include "nel/3d/patchuv_locator.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
void	CPatchUVLocator::build(const CPatch *patchCenter, sint edgeCenter, CPatch::CBindInfo	&bindInfo)
{
	nlassert(bindInfo.Zone);
	// copy basic. NB: NPatchs==0 means patchCenter is binded on a 1/X patch.
	_CenterPatch= const_cast<CPatch*>(patchCenter);
	_CenterPatchEdge= edgeCenter;
	_NPatchs= bindInfo.NPatchs;


	// set it to true. false-d if one of the neighbor patch does not have same number of tile.
	_SameEdgeOrder= true;


	// For all patchs binded to me.
	for(sint i=0; i<_NPatchs; i++)
	{
		// The edge of the neihbor on which we are binded.
		sint	edgeNeighbor= bindInfo.Edge[i];
		CPatch	*paNeighbor= bindInfo.Next[i];
		_NeighborPatch[i]= paNeighbor;


		// Find uvI, uvJ, uvP such that:
		// uvOut= uvIn.x * uvI + uvIn.y * uvJ + uvP.
		CVector2f		&uvI= _NeighborBasis[i].UvI;
		CVector2f		&uvJ= _NeighborBasis[i].UvJ;
		CVector2f		&uvP= _NeighborBasis[i].UvP;


		// Find the basis MeToNeighbor.
		//=============================
		sint	rotation= (edgeCenter - edgeNeighbor + 4) & 3;
		// Find scale to apply.
		float	scX, scY;
		// If our neighbor edge is a vertical edge
		if( (edgeNeighbor&1)==0 )
		{
			scX= 1;

			// Manage difference of Order at the edge.
			scY= (float)paNeighbor->getOrderForEdge(edgeNeighbor) / (float)patchCenter->getOrderForEdge(edgeCenter);
			// Manage bind on the edge.
			// If patchCenter is binded on a bigger
			if(bindInfo.MultipleBindNum!=0)
				scY/= bindInfo.MultipleBindNum;
			if(_NPatchs>1)
				scY*= _NPatchs;
			// same TileOrder on the edge??
			if(scY!=1)
				_SameEdgeOrder= false;
		}
		else
		{
			scY= 1;

			// Manage difference of Order at the edge.
			scX= (float)paNeighbor->getOrderForEdge(edgeNeighbor) / (float)patchCenter->getOrderForEdge(edgeCenter);
			// Manage bind on the edge.
			// If patchCenter is binded on a bigger
			if(bindInfo.MultipleBindNum!=0)
				scX/= bindInfo.MultipleBindNum;
			if(_NPatchs>1)
				scX*= _NPatchs;
			// same TileOrder on the edge??
			if(scX!=1)
				_SameEdgeOrder= false;
		}
		// Find rotation to apply.
		switch(rotation)
		{
		case 0: uvI.set(-scX, 0); uvJ.set(0, -scY);	break;
		case 1: uvI.set(0, -scY); uvJ.set(scX, 0);	break;
		case 2: uvI.set(scX, 0); uvJ.set(0, scY);	break;
		case 3: uvI.set(0, scY); uvJ.set(-scX, 0);	break;
		}


		// Find the position.
		//=============================
		// Find the uv coord at start of the edge, for 2 patchs.
		CVector2f		uvCenter(0.f, 0.f);
		CVector2f		uvNeighbor(0.f, 0.f);
		float	decal;

		// find the uv at start of edgeCenter, + decal due to bind 1/X.
		float	ocS= patchCenter->getOrderS();
		float	ocT= patchCenter->getOrderT();
		// Manage Bind 1/X.
		if(_NPatchs>1)
		{
			// Move uvCenter, so it is near the position at start of edgeNeighbor.
			decal= (float)i / _NPatchs;
		}
		else
			decal= 0;
		// Manage rotation.
		switch(edgeCenter)
		{
		case 0: uvCenter.set(0, decal*ocT);		break;
		case 1: uvCenter.set(decal*ocS, ocT);	break;
		case 2: uvCenter.set(ocS, (1-decal)*ocT);	break;
		case 3: uvCenter.set((1-decal)*ocS, 0);	break;
		};

		// find the uv at start of edgeNeighbor, + decal due to bind X/1.
		float	onS= paNeighbor->getOrderS();
		float	onT= paNeighbor->getOrderT();
		// Manage Bind X/1.
		if(bindInfo.MultipleBindNum!=0)
		{
			// Must invert the id, because of mirror.... (make a draw).
			sint	id= (bindInfo.MultipleBindNum-1) - bindInfo.MultipleBindId;
			// Move uvNeighbor, so it is near the position at start of edgeCenter.
			decal= (float)id / bindInfo.MultipleBindNum;
		}
		else
			decal= 0;
		// Manage rotation.
		switch(edgeNeighbor)
		{
		case 0: uvNeighbor.set(0, (1-decal)*onT);		break;
		case 1: uvNeighbor.set((1-decal)*onS, onT);	break;
		case 2: uvNeighbor.set(onS, decal*onT);		break;
		case 3: uvNeighbor.set(decal*onS, 0);		break;
		};



		// uvOut= uvIn.x * uvI + uvIn.y * uvJ + uvP.
		// So uvP  = uvOut - uvIn.x * uvI - uvIn.y * uvJ
		uvP= uvNeighbor - uvCenter.x * uvI - uvCenter.y * uvJ;

	}
}


// ***************************************************************************
uint	CPatchUVLocator::selectPatch(const CVector2f &uvIn)
{
	if(_NPatchs==1)
		return 0;
	else
	{
		// Choice before on which patch we must go.
		float	selection=0.0;
		uint	os= _CenterPatch->getOrderS();
		uint	ot= _CenterPatch->getOrderT();
		switch(_CenterPatchEdge)
		{
		case 0: selection= uvIn.y / ot; break;
		case 1: selection= uvIn.x / os; break;
		case 2: selection= (ot-uvIn.y) / ot; break;
		case 3: selection= (os-uvIn.x) / os; break;
		}

		sint	sel= (sint)floor(selection*_NPatchs);
		clamp(sel, 0, _NPatchs-1);

		return sel;
	}
}


// ***************************************************************************
void	CPatchUVLocator::locateUV(const CVector2f &uvIn, uint patch, CPatch *&patchOut, CVector2f &uvOut)
{
	if(_NPatchs==1)
	{
		// Change basis and select good patch.
		_NeighborBasis[0].mulPoint(uvIn, uvOut);
		patchOut= _NeighborPatch[0];
	}
	else
	{
		// Change basis and select good patch.
		_NeighborBasis[patch].mulPoint(uvIn, uvOut);
		patchOut= _NeighborPatch[patch];
	}
}



} // NL3D
