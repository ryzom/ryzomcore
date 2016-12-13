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
#include "nel/gui/group_container_base.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	CGroupContainerBase::CGroupContainerBase( const CViewBase::TCtorParam &param ) :
	CInterfaceGroup( param )
	{
		_CurrentContainerAlpha = 255;
		_CurrentContentAlpha = 255;
		_ContentAlpha = 255;
		_ContainerAlpha = 255;
		_RolloverAlphaContainer = 0;
		_RolloverAlphaContent = 0;
		_Locked = false;
		_UseGlobalAlpha = true;
	}

	CGroupContainerBase::~CGroupContainerBase()
	{
	}

	void CGroupContainerBase::removeAllContainers()
	{
		// Necessary because it's supposed to be an abstract class,
		// however reflection requires the class to be instantiated.
		nlassert( false );
	}

	void CGroupContainerBase::setLocked( bool locked )
	{
		// Necessary because it's supposed to be an abstract class,
		// however reflection requires the class to be instantiated.
		nlassert( false );
	}


	// ***************************************************************************
	void CGroupContainerBase::triggerAlphaSettingsChangedAH()
	{
		if (_AHOnAlphaSettingsChanged != NULL)
			CAHManager::getInstance()->runActionHandler(_AHOnAlphaSettingsChanged, this, _AHOnAlphaSettingsChangedParams);
	}


	// ***************************************************************************
	void CGroupContainerBase::setUseGlobalAlpha(bool use)
	{
		_UseGlobalAlpha = use;
		triggerAlphaSettingsChangedAH();
	}

	// ***************************************************************************
	void CGroupContainerBase::setContainerAlpha(uint8 alpha)
	{
		_ContainerAlpha = alpha;
		triggerAlphaSettingsChangedAH();
	}

	// ***************************************************************************
	void CGroupContainerBase::setContentAlpha(uint8 alpha)
	{
		_ContentAlpha = alpha;
		triggerAlphaSettingsChangedAH();
	}

	// ***************************************************************************
	void CGroupContainerBase::setRolloverAlphaContent(uint8 alpha)
	{
		_RolloverAlphaContent = alpha;
		triggerAlphaSettingsChangedAH();
	}

	// ***************************************************************************
	void CGroupContainerBase::setRolloverAlphaContainer(uint8 alpha)
	{
		_RolloverAlphaContainer = alpha;
		triggerAlphaSettingsChangedAH();
	}

}

