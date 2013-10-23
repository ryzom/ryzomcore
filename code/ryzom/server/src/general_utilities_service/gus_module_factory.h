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

#ifndef GUS_MODULE_FACTORY_H
#define GUS_MODULE_FACTORY_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/smart_ptr.h"

#include "gus_module.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// class IModuleBuilder - class used by module factory
	//-----------------------------------------------------------------------------

	class IModuleBuilder: public NLMISC::CRefCount
	{
	public:
		virtual ~IModuleBuilder() {}
		virtual const NLMISC::CSString& getName() const=0;
		virtual const NLMISC::CSString& getArgs() const=0;
		virtual const NLMISC::CSString& getDescription() const=0;
		virtual NLMISC::CSmartPtr<IModule> buildNewModule(const NLMISC::CSString& rawArgs) const=0;
	};


	//-----------------------------------------------------------------------------
	// class CModuleBuilder - template specialisation of IModuleBuilder
	//-----------------------------------------------------------------------------

	template <class MODULE_CLASS>
	class CModuleBuilder: public IModuleBuilder
	{
	public:
		// ctor
		CModuleBuilder(const NLMISC::CSString& name,const NLMISC::CSString& args,const NLMISC::CSString& description);

		// accessor for 
		const NLMISC::CSString& getName() const;
		const NLMISC::CSString& getArgs() const;
		const NLMISC::CSString& getDescription() const;
		NLMISC::CSmartPtr<IModule> buildNewModule(const NLMISC::CSString& rawArgs) const;

	private:
		NLMISC::CSString _Name;
		NLMISC::CSString _Args;
		NLMISC::CSString _Description;
	};


	//-----------------------------------------------------------------------------
	// class CSingletonModuleBuilder - template specialisation of IModuleBuilder for singletons
	//-----------------------------------------------------------------------------

	template <class MODULE_CLASS>
	class CSingletonModuleBuilder: public IModuleBuilder
	{
	public:
		// ctor
		CSingletonModuleBuilder(const NLMISC::CSString& name,const NLMISC::CSString& args,const NLMISC::CSString& description);

		// accessor for 
		const NLMISC::CSString& getName() const;
		const NLMISC::CSString& getArgs() const;
		const NLMISC::CSString& getDescription() const;
		NLMISC::CSmartPtr<IModule> buildNewModule(const NLMISC::CSString& rawArgs) const;

	private:
		NLMISC::CSString _Name;
		NLMISC::CSString _Args;
		NLMISC::CSString _Description;
	};


	//-----------------------------------------------------------------------------
	// class CModuleFactory
	//-----------------------------------------------------------------------------

	class CModuleFactory
	{
	private:
		// singleton so ctor is private
		CModuleFactory();

	public:
		// singleton implementation
		static CModuleFactory* getInstance();

	public:
		// public types
		typedef NLMISC::CSmartPtr<IModuleBuilder> TModuleBuilderPtr;

		// api
		void registerModuleBuilder(TModuleBuilderPtr moduleBuilderPtr);
		NLMISC::CSmartPtr<IModule> buildNewModule(const NLMISC::CSString& name,const NLMISC::CSString& rawArgs);
		void display();

	private:
		// set of builder objects for module factory
		typedef std::vector<TModuleBuilderPtr> TModuleBuilders;
		TModuleBuilders _ModuleBuilders;
	};


	//-----------------------------------------------------------------------------
	// REGISTER_GUS_MODULE module builder registration MACRO and associated routine
	//-----------------------------------------------------------------------------

	template <class MODULE_CLASS> void registerModuleBuilder(const NLMISC::CSString& moduleName,const NLMISC::CSString& moduleArgs,const NLMISC::CSString& moduleDescription,MODULE_CLASS*)
	{
		static bool firstTime= true;
		if (!firstTime) return;
		firstTime=false;

		IModuleBuilder* moduleBuilder= new CModuleBuilder<MODULE_CLASS>(moduleName,moduleArgs,moduleDescription);
		CModuleFactory::getInstance()->registerModuleBuilder(moduleBuilder);
	}

	// Macro to register an IModule class
	#define REGISTER_GUS_MODULE(module_class,module_name,module_args,module_description)\
		class CGusModuleRegisterer__##module_class\
		{\
		public:\
			CGusModuleRegisterer__##module_class()\
			{\
				GUS::registerModuleBuilder(module_name,module_args,module_description,(module_class*)NULL);\
			}\
		}\
		GusModuleRegisterer__##module_class;


	//-----------------------------------------------------------------------------
	// REGISTER_GUS_SINGLETON_MODULE module builder registration MACRO and associated routine
	//-----------------------------------------------------------------------------

	template <class MODULE_CLASS> void registerSingletonModuleBuilder(const NLMISC::CSString& moduleName,const NLMISC::CSString& moduleArgs,const NLMISC::CSString& moduleDescription,MODULE_CLASS*)
	{
		static bool firstTime= true;
		if (!firstTime) return;
		firstTime=false;

		IModuleBuilder* moduleBuilder= new CSingletonModuleBuilder<MODULE_CLASS>(moduleName,moduleArgs,moduleDescription);
		CModuleFactory::getInstance()->registerModuleBuilder(moduleBuilder);
	}

	// Macro to register an IModule class
	#define REGISTER_GUS_SINGLETON_MODULE(module_class,module_name,module_args,module_description)\
		class CGusModuleRegisterer__##module_class\
		{\
		public:\
			CGusModuleRegisterer__##module_class()\
			{\
				module_class* theInstance= module_class::getInstance();\
				GUS::registerSingletonModuleBuilder(module_name,module_args,module_description,theInstance);\
			}\
		}\
		GusModuleRegisterer__##module_class;


	//-----------------------------------------------------------------------------
	// methods CModuleBuilder
	//-----------------------------------------------------------------------------

	template <class MODULE_CLASS>
	CModuleBuilder<MODULE_CLASS>::CModuleBuilder(const NLMISC::CSString& name,const NLMISC::CSString& args,const NLMISC::CSString& description)
	{
		_Name= name;
		_Args= args;
		_Description= description;
	}

	template <class MODULE_CLASS>
	const NLMISC::CSString& CModuleBuilder<MODULE_CLASS>::getName() const
	{
		return _Name;
	}

	template <class MODULE_CLASS>
	const NLMISC::CSString& CModuleBuilder<MODULE_CLASS>::getArgs() const
	{
		return _Args;
	}

	template <class MODULE_CLASS>
	const NLMISC::CSString& CModuleBuilder<MODULE_CLASS>::getDescription() const
	{
		return _Description;
	}

	template <class MODULE_CLASS>
	NLMISC::CSmartPtr<IModule> CModuleBuilder<MODULE_CLASS>::buildNewModule(const NLMISC::CSString& rawArgs) const
	{
		NLMISC::CSmartPtr<IModule> theModule= new MODULE_CLASS;
		bool isOK= theModule->initialiseModule(rawArgs);
		if (!isOK)
			return NULL;
		return theModule;
	}


	//-----------------------------------------------------------------------------
	// methods CSingletonModuleBuilder
	//-----------------------------------------------------------------------------

	template <class MODULE_CLASS>
	CSingletonModuleBuilder<MODULE_CLASS>::CSingletonModuleBuilder(const NLMISC::CSString& name,const NLMISC::CSString& args,const NLMISC::CSString& description)
	{
		_Name= name;
		_Args= args;
		_Description= description;
	}

	template <class MODULE_CLASS>
	const NLMISC::CSString& CSingletonModuleBuilder<MODULE_CLASS>::getName() const
	{
		return _Name;
	}

	template <class MODULE_CLASS>
	const NLMISC::CSString& CSingletonModuleBuilder<MODULE_CLASS>::getArgs() const
	{
		return _Args;
	}

	template <class MODULE_CLASS>
	const NLMISC::CSString& CSingletonModuleBuilder<MODULE_CLASS>::getDescription() const
	{
		return _Description;
	}

	template <class MODULE_CLASS>
	NLMISC::CSmartPtr<IModule> CSingletonModuleBuilder<MODULE_CLASS>::buildNewModule(const NLMISC::CSString& rawArgs) const
	{
		TModulePtr theModule= MODULE_CLASS::getInstance();

		bool isOK= theModule->initialiseModule(rawArgs);
		if (!isOK)
			return NULL;

		return theModule;
	}
}

//-----------------------------------------------------------------------------
#endif
