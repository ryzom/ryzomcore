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

#include "continent_manager.h"
#include "continent.h"
#include "ig_callback.h"
#include "pacs_client.h"
#include "continent_manager.h"
#include "world_database_manager.h"

#include "ig_enum.h"

//===========================================================
bool enumAllIGs(IIGEnum *callback)
{
	nlassert(callback);
	if (IGCallbacks)
	{
		if (!IGCallbacks->enumIGs(callback))
			return false;
	}

	if ((ContinentMngr.cur() == 0) || !ContinentMngr.cur()->enumIGs(callback))
		return false;
	return true;
}

// ***************************************************************************
// CIGAddedNotifier
// ***************************************************************************

//===========================================================
void CIGNotifier::registerObserver(IIGObserver *obs)
{
	nlassert(obs);
	_Observers.push_back(obs);
}

//===========================================================
void CIGNotifier::removeObserver(IIGObserver *obs)
{
	_Observers.erase(std::remove(_Observers.begin(), _Observers.end(), obs), _Observers.end());
}

//===========================================================
bool CIGNotifier::isObserver(IIGObserver *obs) const
{
	TObservers::const_iterator it = std::find(_Observers.begin(), _Observers.end(), obs);
	return it != _Observers.end();
}


//===========================================================
void CIGNotifier::notifyIGLoaded(NL3D::UInstanceGroup *ig)
{
	for(TObservers::iterator it = _Observers.begin(); it != _Observers.end(); ++it)
	{
		(*it)->instanceGroupLoaded(ig);
	}
}

//===========================================================
void CIGNotifier::notifyIGAdded(NL3D::UInstanceGroup *ig)
{
	for(TObservers::iterator it = _Observers.begin(); it != _Observers.end(); ++it)
	{
		(*it)->instanceGroupAdded(ig);
	}
}

//===========================================================
void CIGNotifier::notifyIGRemoved(NL3D::UInstanceGroup *ig)
{
	for(TObservers::iterator it = _Observers.begin(); it != _Observers.end(); ++it)
	{
		(*it)->instanceGroupRemoved(ig);
	}
}
