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

#include "stdpch.h"

#include "property_accessor.h"
#include "dmc.h"
#include "game_share/object.h"

#include <assert.h>
using namespace R2;



CPropertyAccessor::~CPropertyAccessor()
{
	purgeShadowedValues();
}


CObject* CPropertyAccessor::getPropertyValue(CObject* component, const std::string& attrName)
{
	//H_AUTO(R2_CPropertyAccessor_getPropertyValue)
	return const_cast<CObject*>(getPropertyValue((const CObject *) component, attrName));
}


double CPropertyAccessor::getValueAsNumber(CObject* component, const std::string& attrName) const
{
	//H_AUTO(R2_CPropertyAccessor_getValueAsNumber)
	const CObject* object=getPropertyValue((const CObject *) component, attrName);
	if (!object || !object->isNumber()) { return 0; }
	return object->toNumber();
}

bool CPropertyAccessor::hasValueInBase(CObject *component, const std::string& attrName)
{
	//H_AUTO(R2_CPropertyAccessor_hasValueInBase)
	nlassert(component);
	if (attrName.empty()) return false;
	while (component)
	{
		CObject *base = component->getAttr("Base");
		if (!(base && base->isString())) break;
		std::string strBase = base->toString();
		component = _Client->getPaletteElement(strBase);
		if (!component)
		{
			nlwarning("Can't find base palette element : name = %s", strBase.c_str());
			break;
		}
		if (component->getAttr(attrName)) return true;
	}
	return false;
}

const CObject *CPropertyAccessor::getPropertyValue(const CObject* componentParam, const std::string& attrName) const
{
	//H_AUTO(R2_CPropertyAccessor_getPropertyValue)
	const CObject* component = componentParam;
	nlassert(component);
	const CObject* toRet = 0;
	const CObject* base = 0;
	const CObject* propClass = 0;
	const CObject* baseElement = 0;
	toRet = component->getAttr(attrName);


	// First look in base and in the base of its base and so and so
	baseElement = component;
	while (!toRet )
	{

		base = baseElement->getAttr("Base");

		if (base && base->isString())
		{

			std::string strBase = base->toString();

			baseElement = _Client->getPaletteElement(strBase);
			if (!baseElement)
			{
				nlwarning("Can't find base palette element : name = %s", strBase.c_str());
				return NULL;
			}
			toRet = baseElement->getAttr(attrName);
		}
		else
		{
			break;  // no more base
		}

	}

	// Look in baseClass then in baseClass of its BaseClass

	baseElement = component;
	if (!toRet)
	{
		propClass = baseElement->getAttr("Class");
		if (propClass && propClass->isString())
		{
			std::string str = propClass->toString();


			while (!toRet && str != "")
			{
				CObjectGenerator* generator = _Factory->getGenerator(str);

				if (!generator)
				{
					nlwarning("Can't find type : name = %s", str.c_str());
					return NULL;
				}
				toRet = generator->getDefaultValue(attrName);
				if (!toRet)
				{
					str = generator->getBaseClass();
				}
				else
				{
					str ="";
				}

			}
		}
	}


	if (toRet)
	{
		// see if value is currently shadowed
		for(std::vector<CShadowedValue>::const_iterator it = _ShadowedValues.begin(); it != _ShadowedValues.end(); ++it)
		{
			if (it->ShadowedValue == toRet)
			{
				return it->LocalValue;
			}
		}
	}
	return toRet;
}


void CPropertyAccessor::getPropertyList(CObject* component, std::list<std::string>& propertyList)
{
	//H_AUTO(R2_CPropertyAccessor_getPropertyList)
	std::list<std::string> sublist;

	while (component)
	{
		uint32 first = 0;
		uint32 last = component->getSize();

		for ( ; first != last ; ++first)
		{
			std::list<std::string> props;
			std::string key = component->getKey(first);
			props.push_back(key);
		}
		propertyList.insert(sublist.begin(), sublist.end(), propertyList.begin());

		CObject* base = component->getAttr("base");

		if (base && base->isString())
		{
			component = _Client->getPaletteElement(base->toString());
		}
		else
		{
			component = 0;
		}
	}
}

void CPropertyAccessor::shadowValue(CObject *shadowedValue, CObject *localValue)
{
	//H_AUTO(R2_CPropertyAccessor_shadowValue)
	if (!shadowedValue) return;
	purgeShadowedValues();
	for(uint k = 0; k < _ShadowedValues.size(); ++k)
	{
		if (_ShadowedValues[k].ShadowedValue == shadowedValue)
		{
			// just replacing value for an already shadowed value
			if (localValue != _ShadowedValues[k].LocalValue)
			{
				delete _ShadowedValues[k].LocalValue;
				_ShadowedValues[k].LocalValue = localValue;
			}
			return;
		}
	}
	// ... this is a new shadowed value
	CShadowedValue sv;
	sv.ShadowedValue = shadowedValue;
	sv.LocalValue = localValue;
	_ShadowedValues.push_back(sv);
}

CObject *CPropertyAccessor::getShadowingValue(CObject *shadowedValue)
{
	//H_AUTO(R2_CPropertyAccessor_getShadowingValue)
	if (!shadowedValue) return NULL;
	purgeShadowedValues();
	for(uint k = 0; k < _ShadowedValues.size(); ++k)
	{
		if (_ShadowedValues[k].ShadowedValue == shadowedValue)
		{
			return _ShadowedValues[k].LocalValue;
		}
	}
	return NULL;
}


void CPropertyAccessor::commitValue(CObject *shadowedValue)
{
	//H_AUTO(R2_CPropertyAccessor_commitValue)
	for(uint k = 0; k < _ShadowedValues.size(); ++k)
	{
		if (_ShadowedValues[k].ShadowedValue == shadowedValue)
		{
			// NB Nico : removed the local copy below because of the new 'instant feddback' mechanism
			// plus, this introduced a bug where the server command was not sent (because new value & local
			// value were equal)

			// copy local value to shadowed value
			//_ShadowedValues[k].ShadowedValue->inPlaceCopy(*_ShadowedValues[k].LocalValue);
			_ShadowedValues[k].ShadowedValue = NULL; // delete job done by purgeShadowedValues
			break;
		}
	}
	purgeShadowedValues();
}

void CPropertyAccessor::rollbackValue(CObject *shadowedValue)
{
	//H_AUTO(R2_CPropertyAccessor_rollbackValue)
	for(uint k = 0; k < _ShadowedValues.size(); ++k)
	{
		if (_ShadowedValues[k].ShadowedValue == shadowedValue)
		{
			_ShadowedValues[k].ShadowedValue = NULL; // delete job done by purgeShadowedValues
			break;
		}
	}
	purgeShadowedValues();
}


struct CDeadShadowedObjectReferenceTest
{
	bool operator()(const CPropertyAccessor::CShadowedValue &sw) const { return sw.ShadowedValue == NULL; }
};

void CPropertyAccessor::purgeShadowedValues()
{
	//H_AUTO(R2_CPropertyAccessor_purgeShadowedValues)
	for(uint k = 0; k < _ShadowedValues.size(); ++k)
	{
		if (!_ShadowedValues[k].ShadowedValue)
		{
			delete _ShadowedValues[k].LocalValue;
			_ShadowedValues[k].LocalValue = NULL;
		}
	}
	_ShadowedValues.erase(std::remove_if(_ShadowedValues.begin(), 	_ShadowedValues.end(), CDeadShadowedObjectReferenceTest()), _ShadowedValues.end());
}





