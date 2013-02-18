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
#include "object_factory_client.h"
#include "editor.h"
#include "instance.h"
#include "nel/gui/lua_ihm.h"
#include "../interface_v3/lua_ihm_ryzom.h"

namespace R2
{



///////////////////////////////////////
// CObjectRefIdClient implementation //
///////////////////////////////////////


// ************************************************************************
CObjectRefIdClient::CObjectRefIdClient(const std::string &value) : CObjectRefId(""), _ObserverHandle(CEditor::BadInstanceObserverHandle),
										   _IndexInParent(-1),
										   _IndexInParentArray(-1),
										   _Enabled(true)
{
	//nlwarning("# Creating CObjectRefIdClient 0x%x", (int) this);
	set("", value);
}

// ************************************************************************
CObjectRefIdClient::~CObjectRefIdClient()
{
	if (_Enabled)
	{
		nlassert(getValue().empty() == (_ObserverHandle == CEditor::BadInstanceObserverHandle));
		set("", ""); // force to release the observer
	}
}

// ************************************************************************
CObject* CObjectRefIdClient::clone() const
{
	//H_AUTO(R2_CObjectRefIdClient_clone)
	return new CObjectRefIdClient(getValue());
}

// ************************************************************************
void CObjectRefIdClient::enable(bool enabled)
{
	//H_AUTO(R2_CObjectRefIdClient_enable)
	if (enabled == _Enabled) return;
	_Enabled = enabled;
	if (_Enabled)
	{
		addObserverHandle();
	}
	else
	{
		removeObserverHandle();
	}
}

// ************************************************************************
void CObjectRefIdClient::removeObserverHandle()
{
	//H_AUTO(R2_CObjectRefIdClient_removeObserverHandle)
	if (_ObserverHandle != CEditor::BadInstanceObserverHandle)
	{
		getEditor().removeInstanceObserver(_ObserverHandle);
		_ObserverHandle = CEditor::BadInstanceObserverHandle;
	}
}

// ************************************************************************
void CObjectRefIdClient::addObserverHandle()
{
	//H_AUTO(R2_CObjectRefIdClient_addObserverHandle)
	if (!getValue().empty())
	{
		_ObserverHandle = getEditor().addInstanceObserver(getValue(), this);
	}
}

// ************************************************************************
bool CObjectRefIdClient::set(const std::string &key, const std::string & value)
{
	//H_AUTO(R2_CObjectRefIdClient_set)
	//nlwarning("# Setting CObjectRefIdClient 0x%x value to %s", (int) this, value.c_str());
	if (_Enabled)
	{
		nlassert(getValue().empty() == (_ObserverHandle == CEditor::BadInstanceObserverHandle));
	}
	if (value == getValue()) return true;
	if (_Enabled)
	{
		removeObserverHandle();
	}
	bool nodeSet = CObjectRefId::set(key, value);
	if (_Enabled)
	{
		if (!value.empty())
		{
			_ObserverHandle = getEditor().addInstanceObserver(value, this);
		}

		nlassert(getValue().empty() == (_ObserverHandle == CEditor::BadInstanceObserverHandle));
	}
	return nodeSet;
}

// ************************************************************************
void CObjectRefIdClient::onInstanceCreated(CInstance &/* instance */)
{
	//H_AUTO(R2_CObjectRefIdClient_onInstanceCreated)
	if (!_Enabled) return;
	std::string nameInParent;
	sint32 indexInArray;
	getNameInParent(nameInParent, indexInArray); // parent ptr will be updated
	if (!_ParentInstance) return;
	_ParentInstance->onTargetInstanceCreated(nameInParent, indexInArray);
}

// ************************************************************************
void CObjectRefIdClient::onInstanceErased(CInstance &/* instance */)
{
	//H_AUTO(R2_CObjectRefIdClient_onInstanceErased)
	if (!_Enabled) return;
	std::string nameInParent;
	sint32 indexInArray;
	getNameInParent(nameInParent, indexInArray); // parent ptr will be updated
	if (!_ParentInstance) return;
	_ParentInstance->onTargetInstanceErased(nameInParent, indexInArray);
}

// ************************************************************************
void CObjectRefIdClient::onPreHrcMove(CInstance &/* instance */)
{
	//H_AUTO(R2_CObjectRefIdClient_onPreHrcMove)
	if (!_Enabled) return;
	std::string nameInParent;
	sint32 indexInArray;
	getNameInParent(nameInParent, indexInArray); // parent ptr will be updated
	if (!_ParentInstance) return;
	_ParentInstance->onTargetInstancePreHrcMove(nameInParent, indexInArray);
}

// ************************************************************************
void CObjectRefIdClient::onPostHrcMove(CInstance &/* instance */)
{
	//H_AUTO(R2_CObjectRefIdClient_onPostHrcMove)
	if (!_Enabled) return;
	std::string nameInParent;
	sint32 indexInArray;
	getNameInParent(nameInParent, indexInArray); // parent ptr will be updated
	if (!_ParentInstance) return;
	_ParentInstance->onTargetInstancePostHrcMove(nameInParent, indexInArray);
}


// ************************************************************************
void CObjectRefIdClient::onInstanceEraseRequest(CInstance &/* instance */)
{
	//H_AUTO(R2_CObjectRefIdClient_onInstanceEraseRequest)
	if (!_Enabled) return;
	std::string nameInParent;
	sint32 indexInArray;
	getNameInParent(nameInParent, indexInArray); // parent ptr will be updated
	if (!_ParentInstance) return;
	_ParentInstance->onTargetInstanceEraseRequested(nameInParent, indexInArray);
}

// ************************************************************************
void CObjectRefIdClient::onAttrModified(CInstance &/* instance */, const std::string &modifiedAttrName, sint32 modifiedAttrIndexInArray)
{
	//H_AUTO(R2_CObjectRefIdClient_onAttrModified)
	if (!_Enabled) return;
	std::string nameInParent;
	sint32 indexInArray;
	getNameInParent(nameInParent, indexInArray); // parent ptr will be updated
	if (!_ParentInstance) return;
	_ParentInstance->onTargetInstanceAttrModified(nameInParent, indexInArray, modifiedAttrName, modifiedAttrIndexInArray);
}

// ************************************************************************
void CObjectRefIdClient::updateParentInstancePtr() const
{
	//H_AUTO(R2_CObjectRefIdClient_updateParentInstancePtr)
	_ParentInstance = NULL;
	CObject *currParent = this->getParent();
	while (currParent)
	{
		_ParentInstance = getEditor().getInstanceFromObject(currParent);
		if (_ParentInstance) break;
		currParent = currParent->getParent();
	}
}

// ************************************************************************
void CObjectRefIdClient::getNameInParent(std::string &name, sint32 &indexInArray) const
{
	//H_AUTO(R2_CObjectRefIdClient_getNameInParent)
	if (_IndexInParent != -1 && _ParentInstance)
	{
		CObjectTable *parentInstanceTable = _ParentInstance->getObjectTable();
		// check that index is still valid (true most of the case unless instance has been moved)
		if (_IndexInParent <= (sint32) parentInstanceTable->getSize())
		{
			if (_IndexInParentArray == -1)
			{
				if (parentInstanceTable->getValue(_IndexInParent) == static_cast<const CObject *>(this))
				{
					name =  parentInstanceTable->getKey(_IndexInParent);
					indexInArray = -1;
					return;
				}
			}
			else
			{
				CObject *subObject = parentInstanceTable->getValue(_IndexInParent);
				if (subObject->isTable())
				{
					CObjectTable *subTable = (CObjectTable *) subObject;
					if (_IndexInParentArray < (sint32) subTable->getSize())
					{
						if (subTable->getValue(_IndexInParentArray) == static_cast<const CObject *>(this))
						{
							name =  parentInstanceTable->getKey(_IndexInParent);
							indexInArray = _IndexInParentArray;
						}
					}
				}
			}
		}
	}
	// must search name in parent (on init or when object is moved)
	updateParentInstancePtr();
	if (!_ParentInstance)
	{
		_IndexInParent = -1;
		_IndexInParentArray = -1;
		name.clear();
		indexInArray = -1;
		return;
	}
	CObjectTable *parentInstanceTable = _ParentInstance->getObjectTable();
	const CObject *ptrInParent = (parentInstanceTable == this->getParent()) ? static_cast<const CObject *>(this) : this->getParent();
	// if instance is the direct parent (e.g object is not in an array of the parent)
	for (uint k = 0; k < parentInstanceTable->getSize(); ++k)
	{
		if (parentInstanceTable->getValue(k) == ptrInParent)
		{
			_IndexInParent = k;
			if (ptrInParent == this)
			{
				_IndexInParentArray = -1;
				indexInArray = -1;
				name = parentInstanceTable->getKey(_IndexInParent);
				return;
			}
			else
			{
				// I'm in an array in my parent, retrieve the index
				for (uint l = 0; l < getParent()->getSize(); ++l)
				{
					if (getParent()->getValue(l) == static_cast<const CObject *>(this))
					{
						name = parentInstanceTable->getKey(_IndexInParent);
						_IndexInParentArray = l;
						return;
					}
				}
			}
		}
	}
	// TMP TMP
	nlwarning("=========================================");
	CLuaIHMRyzom::dumpCallStack();
	nlwarning("=========================================");
	nlwarning("ObservedObject = %s", getValue().c_str());
	CInstance *obsInstance = getEditor().getInstanceFromId(getValue().c_str());
	nlwarning("ObservedObject instance ptr = %p", obsInstance);
	nlwarning("=========================================");
	if (obsInstance)
	{
		obsInstance->getLuaProjection().dump();
		CInstance *parent = obsInstance->getParent();
		nlwarning("ObservedObject parent instance ptr = %p", parent);
		parent->getLuaProjection().dump();
	}
	nlassert(0); // not found in parent
}

///////////////////////////////////////
// CObjectTableClient implementation //
///////////////////////////////////////

// ************************************************************************
void CObjectTableClient::pushOnLuaStack(CLuaState &state, CLuaObject &metatable) const
{
	//H_AUTO(R2_CObjectTableClient_pushOnLuaStack)
	// cache refptr here to avoid costly allocations
	CLuaStackChecker lsc(&state, 1);
	if (!_Ref.isValid())
	{
		nlassert(metatable.isValid());
		// return a new refptr on the sub table
		void *block = state.newUserData(sizeof(TRefPtrConst));
		new (block) CObjectTable::TRefPtrConst(this); // create in place
		metatable.push();
		state.setMetaTable(-2);
		_Ref.pop(state);
	}
	nlassert(_Ref.getLuaState() == &state);
	_Ref.push();
}

// ************************************************************************
CObject* CObjectTableClient::clone() const
{
	//H_AUTO(R2_CObjectTableClient_clone)
	CObjectTableClient *ret = new CObjectTableClient();
	// NB : don't copy the reference because there can be only one CObjectTableClient per instance (other copy are for undo/redo or network)
	TContainer::const_iterator first(_Value.begin()),  last(_Value.end());
	for ( ;first != last; ++first )
	{
		nlassert(first->second);
		ret->add(first->first,  first->second->clone());
	}
	ret->setGhost(getGhost());
	return ret;
}

// ************************************************************************
CObjectTableClient::CObjectTableClient()
{
}

///////////////////////////////////////
// CClientObjectFactory implementation //
///////////////////////////////////////

// ************************************************************************
CObject* CObjectFactoryClient::newBasic(const std::string & type)
{
	//H_AUTO(R2_CObjectFactoryClient_newBasic)
	if (type == "RefId")
	{
		return new CObjectRefIdClient("");
	}
	else if (type == "Table")
	{
		return new CObjectTableClient;
	}
	else
	{
		return CObjectFactory::newBasic(type);
	}
}

// ************************************************************************
CObjectFactoryClient::CObjectFactoryClient(const std::string &prefix) : CObjectFactory(prefix)
{
}

} // R2

