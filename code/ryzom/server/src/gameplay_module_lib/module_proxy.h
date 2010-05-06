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

/* Copyright, 2000-2004 Nevrax Ltd.
 *
 * This file is part of NEVRAX RYZOM.
 * NEVRAX RYZOM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX RYZOM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX RYZOM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef RY_MODULE_PROXY_H
#define RY_MODULE_PROXY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"


class IModuleCore;

/**
 * A proxy to a module core. Usulful to abstract the core completly
 * If you dont have a module but need a proxy ( e.g. : just before creating the module), build it with IModuleProxy( IModuleCore * core)
 * Otherwise, use the getProxy method of your module, and inside a module method
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
template <class T>
class IModuleProxy
{
public:
	/// ctor ( needed by the module class )
	IModuleProxy(){}
	/// ctor to use if you are not in a module ( CAUTION : the only real case is when you have to interact with a core before creating the module ) 
	IModuleProxy( IModuleCore * core)
	{
		_ModuleCore = dynamic_cast<T*>(core);
		MODULE_AST(_ModuleCore);
	}

protected:
	NLMISC::CRefPtr<T>	_ModuleCore;
};


#endif // RY_MODULE_PROXY_H

/* End of module_proxy.h */
