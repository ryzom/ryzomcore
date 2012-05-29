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

#include "streamable_entity_composite.h"
#include "nel/misc/progress_callback.h"

H_AUTO_DECL(RZ_StremableEntityComposite)

//===============================================================================
CStreamableEntityComposite::~CStreamableEntityComposite()
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	for(TStreambleEntities::iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		delete *it;
	}
}

//===============================================================================
void CStreamableEntityComposite::add(IStreamableEntity *entity)
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	nlassert (entity);
	nlassert(std::find(_Entities.begin(), _Entities.end(), entity) == _Entities.end());
	_Entities.push_back(entity);
}

//===============================================================================
void CStreamableEntityComposite::remove(IStreamableEntity *entity)
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	TStreambleEntities::iterator it = std::find(_Entities.begin(), _Entities.end(), entity);
	nlassert(it != _Entities.end());
	_Entities.erase(it);
	delete entity;
}

//===============================================================================
/*virtual*/ bool CStreamableEntityComposite::needCompleteLoading(const NLMISC::CVector &pos) const
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	for(TStreambleEntities::const_iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		if ((*it)->needCompleteLoading(pos)) return true;
	}
	return false;
}

//===============================================================================
/*virtual*/ void CStreamableEntityComposite::update(const NLMISC::CVector &pos)
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	for(TStreambleEntities::iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		(*it)->update(pos);
	}
}

//===============================================================================
/*virtual*/ void CStreamableEntityComposite::forceUpdate(const NLMISC::CVector &pos, NLMISC::IProgressCallback &progress)
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	const uint size = (uint)_Entities.size();
	uint count = 0;
	for(TStreambleEntities::iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		// Progress
		progress.progress((float)count/(float)size);
		progress.pushCropedValues((float)count/(float)size, (float)(count+1)/(float)size);

		(*it)->forceUpdate(pos, progress);
		count++;

		progress.popCropedValues();
	}
}

//===============================================================================
void CStreamableEntityComposite::reserve(uint size)
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	_Entities.reserve(size);
}

//===============================================================================
void CStreamableEntityComposite::removeAll()
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	for(TStreambleEntities::iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		delete *it;
	}
	NLMISC::contReset(_Entities);
}


//===============================================================================
void CStreamableEntityComposite::forceUnload()
{
	H_AUTO_USE(RZ_StremableEntityComposite)
	for(TStreambleEntities::iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		(*it)->forceUnload();
	}
}


