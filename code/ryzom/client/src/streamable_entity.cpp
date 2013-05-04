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



#include "stdpch.h"
#include "streamable_entity.h"


H_AUTO_DECL(RZ_StreamableEntity)

CStreamableEntity::CStreamableEntity()	: _LoadRadius(0), _ForceLoadRadius(0), _UnloadRadius(0)
{

}


void CStreamableEntity::forceUpdate(const NLMISC::CVector &pos, NLMISC::IProgressCallback &progress)
{
	H_AUTO_USE(RZ_StreamableEntity)
	float dist = (pos - _Pos).norm();
	//
	if (dist >= _UnloadRadius)
	{
		unload();
		return;
	}
	//
	if (dist <= _LoadRadius)
	{
		load(progress);
		return;
	}
}


void CStreamableEntity::update(const NLMISC::CVector &pos)
{
	H_AUTO_USE(RZ_StreamableEntity)
	float dist = (pos - _Pos).norm();
	//
	if (dist >= _UnloadRadius)
	{
		unload();
		return;
	}
	// TEMP TEMP
	/*
	if (dist <= _ForceLoadRadius)
	{
		load(season);
		return;
	}
	*/
	//
	if (dist <= _LoadRadius)
	{
		loadAsync();
		return;
	}
}



