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

#ifndef RY_MODULE_H
#define RY_MODULE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "module_utils.h"
#include "module_parent.h"

class IModuleCore;

/**
 * A gameplay module. Implements the actions done by an element of a system ( e.g. : a player ) on an element of another system ( e.g. : a guild )
 * A module is responsible of cleaning himself. 
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IModule : public NLMISC::CRefCount
{
public:
	/// ctor. Is in charge of registering the module on the parent and referenced core
	IModule( CModuleParent* parent, IModuleCore* referencedCore);
	virtual ~IModule(){}
	/// callback called when parent is deleted. It calls the onParentDestructionHandler virtual method
	void onParentDestruction();
	/// reference is destroyed. It destroys this module and remove it from parent
	void onReferencedDestruction();
	/// retrieve a "proxy" on the parent owner ( e.g. : the player )
	template <class T>
	void getProxy(T & proxy)const
	{
		MODULE_AST( _Parent );
		MODULE_AST( _Parent->getOwner() );
		// build the proxy on the fly : it only contains a pointer
		proxy = T(_Parent->getOwner());
	}
	
protected:
	/// pointer on the container containing this module
	NLMISC::CRefPtr<CModuleParent>	_Parent;
	/// virtual handler that you have to implement in your module. 
	/// e.g. :in case of a pet module, destroy the pet. 
	/// e. g. : in case of a guild call CGuild::removeReferencingModule(this);
	virtual void onParentDestructionHandler() = 0;
};

//#include "module_parent.h"
//#include "module_core.h"
//#include "module_inline.h"

#endif // RY_MODULE_H

/* End of module.h */
