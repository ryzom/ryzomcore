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
#include "nel/gui/ctrl_scroll_base.h"

namespace NLGUI
{

	CCtrlScrollBase::CCtrlScrollBase( const TCtorParam &param ) :
	CCtrlBase( param )
	{
		_Target = NULL;
	}

	CCtrlScrollBase::~CCtrlScrollBase()
	{
	}

	void CCtrlScrollBase::setTarget( CInterfaceGroup *pIG )
	{
		// Necessary because it's supposed to be an abstract class,
		// however reflection requires the class to be instantiated.
		nlassert( false );
	}

	sint32 CCtrlScrollBase::moveTrackX( sint32 dx )
	{
		// Necessary because it's supposed to be an abstract class,
		// however reflection requires the class to be instantiated.
		nlassert( false );

		return 0;
	}

	sint32 CCtrlScrollBase::moveTrackY( sint32 dy )
	{
		// Necessary because it's supposed to be an abstract class,
		// however reflection requires the class to be instantiated.
		nlassert( false );

		return 0;
	}

	void CCtrlScrollBase::moveTargetX( sint32 dx )
	{
		// Necessary because it's supposed to be an abstract class,
		// however reflection requires the class to be instantiated.
		nlassert( false );
	}

	void CCtrlScrollBase::moveTargetY( sint32 dy )
	{
		// Necessary because it's supposed to be an abstract class,
		// however reflection requires the class to be instantiated.
		nlassert( false );
	}

}

