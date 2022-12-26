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

/*
//----------------------------------------------------------------------------
void CModuleParent::removeChildModule(IModule* module)
{
	MODULE_INFO( "Module parent : simply removing a child module" );
	MODULE_AST( module );
	MODULE_AST(_Owner);
	NLMISC::CRefPtr<IModule> ref(module);
	std::vector< NLMISC::CRefPtr<IModule> >::iterator it( std::find( _Modules.begin(), _Modules.end(), ref ) );
	if ( it == _Modules.end() )
	{
		nlwarning("<MODULE> cant find the module to remove");
		return;
	}
	_Modules.erase(it);
}

//----------------------------------------------------------------------------
void CModuleParent::addChildModule(IModule* module)
{
	MODULE_INFO( "Module parent : simply adding a child module" );
	MODULE_AST( module );
#ifdef RY_MODULE_DEBUG	
	NLMISC::CRefPtr<IModule> ref(module);
	std::vector< NLMISC::CRefPtr<IModule> >::iterator it( std::find( _Modules.begin(), _Modules.end(), ref ) );
	if ( it != _Modules.end() )
		nlerror("a module was added twice");
#endif
	_Modules.push_back(module);
}
*/
