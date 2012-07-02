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

#include "module.h"
#include "module_core.h"
#include "module_parent.h"

//----------------------------------------------------------------------------
IModule::IModule( CModuleParent* parent, IModuleCore* referencedCore)
:_Parent(parent)
{
	MODULE_AST(parent);
	// we add the module to the parent as soon as it is created
	_Parent->addChildModule( this );
	// also add the module in the referenced core
	if(referencedCore)
	{
		MODULE_INFO( "building a module referencing a core" );
		referencedCore->addReferencingModule( this );
	}
	else
		MODULE_INFO( "building a module with no reference on a core" );
}

//----------------------------------------------------------------------------
void IModule::onParentDestruction()
{
	MODULE_INFO( "the parent of a module was destroyed, calling overriden handler" );
	// get a ref poniter on the module
	NLMISC::CRefPtr<IModule> ptr(this);
	// call the derived class handler
	onParentDestructionHandler();
	// if the operation did not delete the module, do it now
	if ( ptr )
	{
		MODULE_INFO( "The module deletes itself..." );
		delete ptr;
	}
	else
		MODULE_INFO( "the module was deleted by the overriden handler. We dont have to delete it" );
	
}

//----------------------------------------------------------------------------
void IModule::onReferencedDestruction()
{
	MODULE_INFO( "The module reference was deleted. Remove the module from the parent and delete it" );
	MODULE_AST( _Parent != NULL );
	// get a ref poniter on the module
	NLMISC::CRefPtr<IModule> ptr(this);
	// remove it from the parent
	_Parent->removeChildModule( this );
	// delete this module
	delete ptr;
}
