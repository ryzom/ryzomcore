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



#ifndef _ALIAS_TREE_OWNER_
#define _ALIAS_TREE_OWNER_

#include "ai_share/ai_types.h"
#include "ai_share/ai_alias_description_node.h"
#include "nel/misc/smart_ptr.h"

extern NLLIGO::CLigoConfig LigoConfig;
class CAliasTreeOwner;

//////////////////////////////////////////////////////////////////////////////
// IAliasCont                                                               //
//////////////////////////////////////////////////////////////////////////////

class IAliasCont
{
public:
	virtual uint32				size() const = 0;
	virtual	void				removeChildByAlias	(uint32 alias) = 0;
	virtual	void				removeChildByIndex	(uint32 index) = 0;
	virtual	CAliasTreeOwner*	getAliasChildByAlias(uint32 alias) const = 0;
	virtual	CAliasTreeOwner*	addAliasChild		(CAliasTreeOwner* child) = 0;
	virtual	CAliasTreeOwner*	addAliasChild		(CAliasTreeOwner* child, uint32 index) = 0;		
	
	virtual ~IAliasCont() { }
};

//////////////////////////////////////////////////////////////////////////////
// CAliasTreeOwner                                                          //
//////////////////////////////////////////////////////////////////////////////

class CAliasTreeOwner;

class CAliasTreeOwnerLocator
{
public:
	static CAliasTreeOwnerLocator* getInstance();
private:
	CAliasTreeOwnerLocator() {}
	static CAliasTreeOwnerLocator* _Instance;
	
public:
	CAliasTreeOwner* getEntity(uint32 const alias) const;
	CAliasTreeOwner* getEntity(std::string const& name) const;
	void addEntity(uint32 const alias, std::string const& name, CAliasTreeOwner* entity);
	void delEntity(uint32 const alias, std::string const& name, CAliasTreeOwner* entity);
private:
	std::map<uint32, CAliasTreeOwner*> _EntitiesByAlias;
	std::map<std::string, CAliasTreeOwner*> _EntitiesByName;
};

class CAliasTreeOwner
: public NLMISC::CDbgRefCount<CAliasTreeOwner>
{
public:
	class CAliasDiff
	{
	public:
		CAliasDiff(uint32 alias);
		virtual	~CAliasDiff() { }
		bool operator ()(CAliasTreeOwner const* other) const;
		uint32 _Alias;
	};
	
public:
	explicit CAliasTreeOwner(CAIAliasDescriptionNode* aliasTree);
	explicit CAliasTreeOwner(uint32	alias, std::string const& name);
	virtual ~CAliasTreeOwner();
	
	/// @name Virtual interface
	//@{
	// obtain the container associated with this type.
	virtual IAliasCont* getAliasCont(AITYPES::TAIType type);
	// create a child with the specified alias node.
	virtual CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	// done to allow postprocess dependencies updates.
	virtual void updateDependencies(CAIAliasDescriptionNode const& aliasTree, CAliasTreeOwner* aliasTreeOwner);
	//@}
	
	CAIAliasDescriptionNode* getAliasNode() const;
	
	uint32 getAlias() const;
	std::string getAliasString() const;
	
	std::string const& getName() const;
	
	void setName(std::string const& name);
	
	std::string getAliasFullName() const;
	
	void updateAliasTree(CAIAliasDescriptionNode const& newTree);
	bool getCont(CAliasTreeOwner*& childOwner, IAliasCont*& cont, AITYPES::TAIType _type);

	void pushCurrentOwnerList() { _CurrentOwnerList.push_back(this); }
	void popCurrentOwnerList() { _CurrentOwnerList.pop_back(); }
	
private:
	uint32		_Alias;
	std::string	_Name;
	
	NLMISC::CSmartPtr<CAIAliasDescriptionNode> _AliasTree;
	
	static std::vector<NLMISC::CDbgPtr<CAliasTreeOwner> > _CurrentOwnerList;
};

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CAliasTreeOwner                                                          //
//////////////////////////////////////////////////////////////////////////////

inline
CAliasTreeOwner::CAliasDiff::CAliasDiff(uint32 alias)
: _Alias(alias)
{
}

inline
bool CAliasTreeOwner::CAliasDiff::operator()(CAliasTreeOwner const* other) const
{
	return other->getAlias()!=_Alias;
}

inline
CAliasTreeOwner::CAliasTreeOwner(CAIAliasDescriptionNode* aliasTree)
: _Alias(0)
, _Name(std::string())
, _AliasTree(aliasTree)
{
	if (aliasTree)
	{
		CAliasTreeOwnerLocator::getInstance()->addEntity(aliasTree->getAlias(), aliasTree->getName(), this);
	}
	else
	{
		//DEBUG_STOP;
	}
}

inline
CAliasTreeOwner::CAliasTreeOwner(uint32	alias, std::string const& name)
: _Alias(alias)
, _Name(name)
, _AliasTree(NULL)
{
	CAliasTreeOwnerLocator::getInstance()->addEntity(_Alias, _Name, this);
}

inline
CAliasTreeOwner::~CAliasTreeOwner()
{
	CAliasTreeOwnerLocator::getInstance()->delEntity(getAlias(), getName(), this);
}

inline
CAIAliasDescriptionNode* CAliasTreeOwner::getAliasNode() const
{
	return _AliasTree;
}

inline
uint32 CAliasTreeOwner::getAlias() const
{
	if (_AliasTree)
		return _AliasTree->getAlias();
	
	return _Alias;
}

inline
std::string CAliasTreeOwner::getAliasString() const
{
	if (_AliasTree)
		return LigoConfig.aliasToString(_AliasTree->getAlias());
	
	return LigoConfig.aliasToString(_Alias);
}

inline
std::string const& CAliasTreeOwner::getName() const
{
	if (_AliasTree)
		return _AliasTree->getName();
	
	return _Name;
}

inline
void CAliasTreeOwner::setName(std::string const& name)
{
	// should be able to change the alias tree node name?
	if (_AliasTree)
	{
		DEBUG_STOP;
		return;
	}

	CAliasTreeOwnerLocator::getInstance()->delEntity(_Alias, _Name, this);
	_Name = name;
	CAliasTreeOwnerLocator::getInstance()->addEntity(_Alias, _Name, this);
}

inline
std::string CAliasTreeOwner::getAliasFullName() const
{
	if (_AliasTree)
		return _AliasTree->fullName();
	
	return getName(); // to upgrade...
}

inline
IAliasCont* CAliasTreeOwner::getAliasCont(AITYPES::TAIType type)
{
	return NULL;
}

inline
CAliasTreeOwner* CAliasTreeOwner::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	return NULL;
}

inline
void CAliasTreeOwner::updateDependencies(CAIAliasDescriptionNode const& aliasTree, CAliasTreeOwner* aliasTreeOwner)
{
}

#endif
