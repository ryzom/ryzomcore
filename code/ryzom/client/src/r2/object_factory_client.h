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

#ifndef R2_OBJECT_FACTORY_CLIENT_H
#define R2_OBJECT_FACTORY_CLIENT_H

#include "game_share/object.h"
#include "editor.h"
#include "nel/gui/lua_object.h"

namespace R2
{


/** Reference Id object on the client.
  * This object will trigger the right events on its parent when its target is created or erased.
  * (creation is watched too. This is possible because we use instance id, not direct pointer on the object)
  */
class CObjectRefIdClient : public CObjectRefId, public CEditor::IInstanceObserver
{
public:
	CObjectRefIdClient(const std::string &value);
	~CObjectRefIdClient();
	// from CObjectString
	virtual bool set(const std::string& key, const std::string & value);
	CObject* clone() const;
	void	 enable(bool enabled);
protected:
	// from CEditor::IInstanceObserver
	virtual void onInstanceCreated(CInstance &instance);
	virtual void onInstanceErased(CInstance &instance);
	virtual void onPreHrcMove(CInstance &instance);
	virtual void onPostHrcMove(CInstance &instance);
	virtual void onInstanceEraseRequest(CInstance &instance);
	virtual void onAttrModified(CInstance &instance, const std::string &attrName, sint32 attrIndex);
private:
	// cache for the 'name' of this property in the parent
	CEditor::TInstanceObserverHandle	_ObserverHandle;
	mutable sint32						_IndexInParent;
	mutable sint32						_IndexInParentArray;
	mutable CInstance::TRefPtr			_ParentInstance;
	bool								_Enabled;
private:
	void updateParentInstancePtr() const;
	void getNameInParent(std::string &name, sint32 &indexInArray) const;
	void addObserverHandle();
	void removeObserverHandle();
};


// CObjectTable on client side
class CObjectTableClient : public CObjectTable
{
public:
	CObjectTableClient();
	void pushOnLuaStack(CLuaState &state, CLuaObject &metatable) const;
	CObject* clone() const;
private:
	mutable CLuaObject _Ref;
};

// the client factory redefine R2::CObjectRefId so that it can perform its observer job on the client
class CObjectFactoryClient : public CObjectFactory
{
public:
	CObjectFactoryClient(const std::string &prefix);
	// from CObjectFactory
	virtual CObject* newBasic(const std::string & type);
};



} // R2




#endif

