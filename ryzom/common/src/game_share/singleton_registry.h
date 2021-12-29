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

  NOTE: The following extension would be intelligent:
  ---------------------------------------------------
  - add 'preInit', 'preServiceUpdate', 'preTickUpdate' and 'preRelease' methods
  - add 'postInit', 'postServiceUpdate', 'postTickUpdate' and 'postRelease' methods
  - call all 'preXXX' methods followed by all XXX methods followed by all postXXX methods
  => This allows one to open log files etc in pre-init, start counters in pre-update, stop counters in post-update, etc...

*/

#ifndef SINGLETON_REGISTRY_H
#define SINGLETON_REGISTRY_H


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include <set>

//-------------------------------------------------------------------------------------------------
// class IServiceSingleton
//-------------------------------------------------------------------------------------------------

class IServiceSingleton
{
public:
	// overloadable method called at service initialisation
	virtual void init()				{}

	// overloadable method called in the service update
	virtual void serviceUpdate()	{}

	// overloadable method called in the tick update
	virtual void tickUpdate()		{}

	// overloadable method called at service release
	virtual void release()			{}

protected:
	// protect from untrolled instantiation
	// this method registers the singleton with the singleton registry
	IServiceSingleton();
	virtual ~IServiceSingleton() {}

private:
	// prohibit copy
	IServiceSingleton(const IServiceSingleton&);
};


//-------------------------------------------------------------------------------------------------
// class CSingletonRegistry
//-------------------------------------------------------------------------------------------------

class CSingletonRegistry
{
public:
	// public interface for getting hold of the singleton instance
	static CSingletonRegistry* getInstance();

	// registration of an IServiceSingleton object with the singleton
	void registerSingleton(IServiceSingleton*);

	// methods called from the service loop
	void init();
	void serviceUpdate();
	void tickUpdate();
	void release();

private:
	// prohibit uncontrolled instantiation
	CSingletonRegistry() {}
	CSingletonRegistry(const CSingletonRegistry&);

	typedef std::set<IServiceSingleton*> TSingletons;
	TSingletons _Singletons;
};


//-------------------------------------------------------------------------------------------------
// inlines IServiceSingleton
//-------------------------------------------------------------------------------------------------

inline IServiceSingleton::IServiceSingleton()
{
	CSingletonRegistry::getInstance()->registerSingleton(this);
}


//-------------------------------------------------------------------------------------------------
// inlines CSingletonRegistry
//-------------------------------------------------------------------------------------------------

inline CSingletonRegistry* CSingletonRegistry::getInstance()
{
	static CSingletonRegistry* instance= NULL;
	if (instance==NULL)
	{
		instance=new CSingletonRegistry;
	}
	return instance;
}

inline void CSingletonRegistry::registerSingleton(IServiceSingleton* singleton)
{
	_Singletons.insert(singleton);
}

inline void CSingletonRegistry::init()
{
	for (TSingletons::iterator it=_Singletons.begin(); it!=_Singletons.end();++it)
		(*it)->init();
}

inline void CSingletonRegistry::tickUpdate()
{
	for (TSingletons::iterator it=_Singletons.begin(); it!=_Singletons.end();++it)
		(*it)->tickUpdate();
}

inline void CSingletonRegistry::serviceUpdate()
{
	for (TSingletons::iterator it=_Singletons.begin(); it!=_Singletons.end();++it)
		(*it)->serviceUpdate();
}

inline void CSingletonRegistry::release()
{
	for (TSingletons::iterator it=_Singletons.begin(); it!=_Singletons.end();++it)
		(*it)->release();
}


//-------------------------------------------------------------------------------------------------
#endif
