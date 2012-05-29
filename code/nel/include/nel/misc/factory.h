// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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







#ifndef FACTORY_H
#define FACTORY_H

#include "types_nl.h"
#include "debug.h"
#include <map>

namespace NLMISC
{

/** Interface class for object registered in the factory.
 *	This is for factory internal use, you should not need to use this class directly.
 */
template <class BaseClass>
class IFactoryRegister
{
public:
	virtual ~IFactoryRegister() {}
	/// This method is called to create an instance of the factored object.
	virtual BaseClass *createObject(const typename BaseClass::TCtorParam &ctorParam) = 0;
};


/** Factory implementation class.
 *	the class take 2 template argument :
 *		* BaseClass : the common base class for all factored object of one factory.
 *		* KeyType : the type of the key that identify the factorable object (string by default).
 *
 *	The factory conforms to singleton design pattern.
 *
 *	BaseClass must provide a typedef for TCTorParam corresponding to the parameter
 *	required by the constructor of factored object.
 */
template <class BaseClass, class KeyType = std::string>
class CFactory
{
	typedef std::map<KeyType, IFactoryRegister<BaseClass>*> TRegisterCont;

public:

	/// Get the singleton instance reference.
	static CFactory &instance()
	{
		// Singleton instance pointer.
		static CFactory	*instance = NULL;
		if (!instance)
		{
			instance = new CFactory();
		}
		return *instance;
	}

	/** Register a factorable object in the factory.
	 *	The method receive the key for this factorable object and
	 *	a pointer on the interface of a factory register object.
	 */
	void registerClass(const KeyType &key, IFactoryRegister<BaseClass> *factoryRegister)
	{
		nlassert(_FactoryRegisters.find(key) == _FactoryRegisters.end());
		_FactoryRegisters.insert(std::make_pair(key, factoryRegister));
	}

	/** Create a new instance of a factorable object.
	 *	\param key the identifier of the object to create.
	 *	\param ctorParam the parameter for the constructor.
	 *	\return	a pointer on the newly created object.
	 */
	BaseClass *createObject(const KeyType &key, const typename BaseClass::TCtorParam &ctorParam)
	{
		typename TRegisterCont::iterator it (_FactoryRegisters.find(key));
		if (it == _FactoryRegisters.end())
			return NULL;
		else
			return it->second->createObject(ctorParam);
	}

	// Add some introspection
	void fillFactoryList(std::vector<KeyType> &classList)
	{
		typename TRegisterCont::iterator first(_FactoryRegisters.begin()), last(_FactoryRegisters.end());
		for (; first != last; ++first)
		{
			classList.push_back(first->first);
		}
	}
protected:
	/// Singleton instance pointer.
//	static CFactory	*_Instance;

	/// Container of all registered factorable object.
	TRegisterCont	_FactoryRegisters;
};

/** This class is responsible for creating the factorable object and to register
 *	them in the factory instance.
 *	You must declare an instance of this class for each factorable object.
 **/
template <class FactoryClass, class BaseClass, class FactoredClass, class KeyType>
class CFactoryRegister : public IFactoryRegister<BaseClass>
{
public:
	/** Constructor.
	 *	Register the factorable object in the factory.
	 *	/param key The identifier for this factorable object.
	 */
	CFactoryRegister(const KeyType &key)
	{
		FactoryClass::instance().registerClass(key, this);
	}

	/** Create an instance of the factorable class.
	 *	Implements IFactoryRegister::createObject
	 */
	BaseClass *createObject(const typename BaseClass::TCtorParam &ctorParam)
	{
		return new FactoredClass(ctorParam);
	}
};

/** Macro to declare a factory.
 *	Place this macro in an appropriate cpp file to declare a factory implementation.
 *	You just need to specify the base class type and key type.
 */
//#define NLMISC_IMPLEMENT_FACTORY(baseClass, keyType)	NLMISC::CFactory<baseClass, keyType>	*NLMISC::CFactory<baseClass, keyType>::_Instance = NULL

/** Macro to declare a factorable object.
 *	Place this macro in a cpp file after your factorable class declaration.
 */
#define NLMISC_REGISTER_OBJECT(baseClass, factoredClass, keyType, keyValue)	NLMISC::CFactoryRegister<NLMISC::CFactory<baseClass, keyType>, baseClass, factoredClass, keyType>	Register##factoredClass(keyValue)

#define NLMISC_GET_FACTORY(baseClass, keyType) NLMISC::CFactory<baseClass, keyType>::instance()


/** Interface class for object registered in the indirect factory.
 *	This is for indirect factory internal use, you should not need to use this class directly.
 */
template <class BaseFactoryClass>
class IFactoryIndirectRegister
{
public:
	virtual ~IFactoryIndirectRegister() {}
	/** Return the factory implementation.*/
	virtual BaseFactoryClass *getFactory() = 0;
};



/** Indirect factory implementation class.
 *	the class take 2 template argument :
 *		* BaseFactoryClass : the common base class for all factory object of one indirect factory.
 *		* KeyType : the type of the key that identify the factorable object (string by default).
 *
 *	The indirect factory conforms to singleton design pattern.
 *
 *	In indirect factory, the object returned by the factory are not instance of factored object
 *	but instance of 'sub' factory that do the real job.
 *	This can be useful in some case, like adapting existing code into a factory
 *	or having a complex constructor (or more than one constructor).
 */
template <class BaseFactoryClass, class KeyType = std::string>
class CFactoryIndirect
{
	typedef std::map<KeyType, IFactoryIndirectRegister<BaseFactoryClass>*> TRegisterCont;

public:
	/// Get the singleton instance reference.
	static CFactoryIndirect &instance()
	{
		static CFactoryIndirect	*instance = NULL;
		if (!instance)
		{
			instance = new CFactoryIndirect();
		}
		return *instance;
	}

	void registerClass(const KeyType &key, IFactoryIndirectRegister<BaseFactoryClass> *factoryRegister)
	{
		nlassert(_FactoryRegisters.find(key) == _FactoryRegisters.end());
		_FactoryRegisters.insert(std::make_pair(key, factoryRegister));
	}

	BaseFactoryClass *getFactory(const KeyType &key)
	{
		typename TRegisterCont::const_iterator it (_FactoryRegisters.find(key));
		if (it == _FactoryRegisters.end())
			return NULL;
		else
			return it->second->getFactory();
	}
	// Add some introspection
	void fillFactoryList(std::vector<KeyType> &classList)
	{
		typename TRegisterCont::iterator first(_FactoryRegisters.begin()), last(_FactoryRegisters.end());
		for (; first != last; ++first)
		{
			classList.push_back(first->first);
		}
	}
protected:
//	static CFactoryIndirect	*_Instance;

	TRegisterCont	_FactoryRegisters;
};


template <class IndirectFactoryClass, class BaseFactoryClass, class SpecializedFactoryClass, class KeyType>
class CFactoryIndirectRegister : public IFactoryIndirectRegister<BaseFactoryClass>
{
	SpecializedFactoryClass	_FactoryClass;
public:
	CFactoryIndirectRegister(const KeyType &key)
	{
		IndirectFactoryClass::instance().registerClass(key, this);
	}

	BaseFactoryClass *getFactory()
	{
		return &_FactoryClass;
	}
};

//#define NLMISC_IMPLEMENT_FACTORY_INDIRECT(baseFactoryClass, keyType)	NLMISC::CFactoryIndirect<baseFactoryClass, keyType>	*NLMISC::CFactoryIndirect<baseFactoryClass, keyType>::_Instance = NULL

#define NLMISC_GET_FACTORY_INDIRECT_REGISTRY(baseFactoryClass, keyType)	NLMISC::CFactoryIndirect<baseFactoryClass, keyType>::getInstance()
#define NLMISC_REGISTER_OBJECT_INDIRECT(baseFactoryClass, specializedFactoryClass, keyType, keyValue)	NLMISC::CFactoryIndirectRegister<NLMISC::CFactoryIndirect<baseFactoryClass, keyType>, baseFactoryClass, specializedFactoryClass, keyType>	RegisterIndirect##specializedFactoryClass(keyValue)
#define NLMISC_DECLARE_OBJECT_INDIRECT(baseFactoryClass, specializedFactoryClass, keyType)	extern NLMISC::CFactoryIndirectRegister<NLMISC::CFactoryIndirect<baseFactoryClass, keyType>, baseFactoryClass, specializedFactoryClass, keyType>	RegisterIndirect##specializedFactoryClass

#define NLMISC_GET_FACTORY_INDIRECT(specializedFactoryClass) RegisterIndirect##specializedFactoryClass.getFactory()

} // namespace NLMISC

#endif // FACTORY_H
