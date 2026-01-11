// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#include "nel/gui/interface_factory.h"
#include "nel/gui/view_base.h"
#include "nel/misc/factory.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	CViewBase* CInterfaceFactory::createClass( const std::string &name )
	{
		return NLMISC_GET_FACTORY( CViewBase, std::string ).createObject( std::string( name ) , CViewBase::TCtorParam() );
	}
}


