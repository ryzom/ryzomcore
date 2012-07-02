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

#include "nel/3d/tile_element.h"
#include "nel/misc/debug.h"


namespace NL3D
{


void	CTileElement::setTileOrient(sint i, uint8 orient)
{
	nlassert(i>=0 && i<=2);
	nlassert(orient<=3);
	sint	where= NL_TILE_ELM_SIZE_ROTATE*i+NL_TILE_ELM_OFFSET_ROTATE;
	Flags&= ~(NL_TILE_ELM_MASK_ROTATE<<where);
	Flags|= orient<<where;
}


uint8	CTileElement::getTileOrient(sint i) const
{
	nlassert(i>=0 && i<=2);
	sint	where= NL_TILE_ELM_SIZE_ROTATE*i+NL_TILE_ELM_OFFSET_ROTATE;
	return uint8 ((Flags>>where) & NL_TILE_ELM_MASK_ROTATE);
}


void	CTileElement::setTile256Info(bool is256x256, uint8 uvOff)
{
	nlassert(uvOff<=3);
	sint	where= NL_TILE_ELM_OFFSET_UVINFO;
	sint	info= uvOff+(is256x256?4:0);
	Flags&= ~(NL_TILE_ELM_MASK_UVINFO<<where);
	Flags|= info<<where;
}


void	CTileElement::getTile256Info(bool &is256x256, uint8 &uvOff) const
{
	sint	where= NL_TILE_ELM_OFFSET_UVINFO;
	sint	info= ((Flags>>where) & NL_TILE_ELM_MASK_UVINFO);
	uvOff= info&3;
	is256x256= (info&4)?true:false;
}


void	CTileElement::setTileSubNoise(uint8 subNoise)
{
	nlassert(subNoise<=15);
	sint	where= NL_TILE_ELM_OFFSET_SUBNOISE;
	Flags&= ~(NL_TILE_ELM_MASK_SUBNOISE<<where);
	Flags|= subNoise<<where;
}


void	CTileElement::serial(NLMISC::IStream &f)
{
	f.xmlSerial (Flags, "FLAGS");
	f.xmlSerial (Tile[0], Tile[1], Tile[2], "TILES_ID");
}


void	CTileElement::setVegetableState(TVegetableInfo state)
{
	nlassert(state < VegetInfoLast);
	const uint16 mask = NL_TILE_ELM_MASK_VEGETABLE << NL_TILE_ELM_OFFSET_VEGETABLE;
	Flags = (Flags & ~mask) | ((uint16) state << NL_TILE_ELM_OFFSET_VEGETABLE);
}

} // NL3D
