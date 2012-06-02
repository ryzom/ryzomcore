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

#ifndef RY_MODULE_CORE_H
#define RY_MODULE_CORE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

class IModule;
class CModuleParent;


/**
 * A module Core class ( a character, a guild,... )
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IModuleCore : public NLMISC::CRefCount
{
public:
	/// ctor : allocate the internal module parent structure
	IModuleCore();
	/// dtor : delete the module parent structure
	virtual ~IModuleCore();
	/// return the parent module
	CModuleParent & getModuleParent(){ return *_ModulesCont; }
	/// add a module referencing this corer class
	void addReferencingModule( ::IModule * module );
	/// remove a module referencing this corer class
	void removeReferencingModule( ::IModule * module );
	
	template <class ModuleClass> 
	bool getReferencingModule( ModuleClass* & module)const
	{
		module = NULL;
		const uint size = (uint)_ModulesReferencingMe.size();
		for (uint i = 0; i < size; i++ )
		{	
			ModuleClass * moduleChecked = dynamic_cast<ModuleClass*>( (::IModule*)_ModulesReferencingMe[i] );
			if ( moduleChecked )
			{
				module = moduleChecked;
				// additional checks to ensure module uniqueness
#ifdef RY_MODULE_DEBUG
				for (uint j = i + 1; j < size; j++ )
				{
					ModuleClass * moduleTest = dynamic_cast<ModuleClass*>( (::IModule*)_ModulesReferencingMe[j] );
					if ( moduleTest )
						nlerror("you required a unique referencing module but there is more than 1!!!!");
				}
#endif
				return true;
			}
		}
		return false;
	}

	/// get all the referencing module of the specified type
	// I would have coded getModules( std::vector<ModuleClass> & modules ) but VC6 has an internal compiler error
	template <class ModuleClassVector, class ModuleClass>
	void getReferencingModules( ModuleClassVector &  modules,ModuleClass * dummy )const
	{
		const uint size = _ModulesReferencingMe.size();
		for (uint i = 0;i < size; i++ )
		{	
			ModuleClass * module = dynamic_cast<ModuleClass*>( (::IModule*)_ModulesReferencingMe[i]);
			if ( module )
				modules.push_back(module);
		}
	}


	//std::vector< NLMISC::CRefPtr<IModule> > & getReferencingModules(){ return _ModulesReferencingMe; };
protected:
	/// the module parent of this core structure
	NLMISC::CRefPtr<CModuleParent>			_ModulesCont;
	/// the modules referencing this structure
	std::vector< NLMISC::CRefPtr< ::IModule> >	_ModulesReferencingMe;
};

//#include "module_parent.h"
//#include "module_core_inline.h"

#endif // RY_MODULE_CORE_H

/* End of module_core.h */
