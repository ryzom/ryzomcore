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

#include "module_core.h"
#include "module.h"
#include "module_parent.h"

//----------------------------------------------------------------------------
IModuleCore::~IModuleCore()
{
	MODULE_INFO( "Module core destructor : inform all referencing module and delete the contained module parent" );
	const uint size = (uint)_ModulesReferencingMe.size();
	for ( uint i = 0; i < size; i++ )
	{
		if ( _ModulesReferencingMe[i] )
			_ModulesReferencingMe[i]->onReferencedDestruction();
	}
	delete _ModulesCont;
}
//----------------------------------------------------------------------------
IModuleCore::IModuleCore()
{
	_ModulesCont = new CModuleParent(this);
}

//----------------------------------------------------------------------------
void IModuleCore::addReferencingModule(IModule * module)
{
	MODULE_INFO( "Module core : simply adding a referencing module" );
	MODULE_AST( module );
#ifdef RY_MODULE_DEBUG
	NLMISC::CRefPtr<IModule> ref(module);
	std::vector< NLMISC::CRefPtr<IModule> >::iterator it( std::find( _ModulesReferencingMe.begin(), _ModulesReferencingMe.end(), ref ) );
	if ( it != _ModulesReferencingMe.end() )
		nlerror("a referencing module was added twice");
#endif
	_ModulesReferencingMe.push_back( module );
}

//----------------------------------------------------------------------------
void IModuleCore::removeReferencingModule(IModule * module)
{
	MODULE_INFO( "Module core : simply removeing a referencing module without deleting it" );
	MODULE_AST( module );
	NLMISC::CRefPtr<IModule> ref(module);
	std::vector< NLMISC::CRefPtr<IModule> >::iterator it( std::find( _ModulesReferencingMe.begin(), _ModulesReferencingMe.end(), ref ) );
	if ( it == _ModulesReferencingMe.end() )
	{
		nlwarning("<MODULE> cant find referencing module");
		return;
	}
	_ModulesReferencingMe.erase(it);
}
