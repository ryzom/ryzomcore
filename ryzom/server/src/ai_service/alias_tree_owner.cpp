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
#include "alias_tree_owner.h"
#include "nel/misc/variable.h"

using namespace	AITYPES;

extern NLMISC::CVariable<bool>	LogAliasTreeOwner;

CAliasTreeOwnerLocator* CAliasTreeOwnerLocator::_Instance = NULL;
CAliasTreeOwnerLocator* CAliasTreeOwnerLocator::getInstance()
{
	if (!_Instance)
		_Instance = new CAliasTreeOwnerLocator();
	return _Instance;
}

CAliasTreeOwner* CAliasTreeOwnerLocator::getEntity(uint32 const alias) const
{
	std::map<uint32, CAliasTreeOwner*>::const_iterator it = _EntitiesByAlias.find(alias);
	if (it!=_EntitiesByAlias.end())
		return it->second;
	else
		return NULL;
}

CAliasTreeOwner* CAliasTreeOwnerLocator::getEntity(std::string const& name) const
{
	std::map<std::string, CAliasTreeOwner*>::const_iterator it = _EntitiesByName.find(name);
	if (it!=_EntitiesByName.end())
		return it->second;
	else
		return NULL;
}

void CAliasTreeOwnerLocator::addEntity(uint32 const alias, std::string const& name, CAliasTreeOwner* entity)
{
	_EntitiesByAlias.insert(std::make_pair(alias, entity));
	_EntitiesByName.insert(std::make_pair(name, entity));
}

void CAliasTreeOwnerLocator::delEntity(uint32 const alias, std::string const& name, CAliasTreeOwner* entity)
{
	_EntitiesByName.erase(name);
	_EntitiesByAlias.erase(alias);
}

std::vector<NLMISC::CDbgPtr<CAliasTreeOwner> >	CAliasTreeOwner::_CurrentOwnerList;

void	CAliasTreeOwner::updateAliasTree(const	CAIAliasDescriptionNode	&newTree)
{
	_CurrentOwnerList.push_back(this);
	//	check if this objet already have an associated Tree :)
	if	(getAliasNode())
	{
		CAIAliasDescriptionNode	&oldTree	=	*getAliasNode();
		
		//	scan my own alias tree and make sure that everything listed still exists in the new tree
		//	Check for no more existing Alias Nodes ..
		//	scan destruction is reverse for coherence with construction.
		for	(int i=oldTree.getChildCount()-1;i>=0;i--)
		{
			CAIAliasDescriptionNode*const	oldChild=oldTree.getChild(i);
			
			// search new alias tree for a record corresponding to the one we're on in the old tree
			if	(!newTree.getChildByAlias (oldChild->getAlias()))
			{
				//nlstop("Have to parse the rest of child because a cchild may have been added in the upper hierarchy");

				//	LOG("updateAliasTreeDelete(): deleting: %s:%u (%s)",	getName(oldChild->getType()),	oldChild->getAlias(),	oldChild->fullName().c_str());
				//				if	(!updateAliasTreeDelete(oldChild))
				//					nlwarning("updateAliasTreeDelete(): Don't know how to deal with node: %s:%u (%s)",	getName(oldChild->getType()),	oldChild->getAlias(),	oldChild->fullName().c_str());
				IAliasCont*const	cont	=	getAliasCont(oldChild->getType());
				if	(cont)
					cont->removeChildByAlias(oldChild->getAlias());	//	smartPtr will do the job if necessary.
			}
			
		}
		
	}

	// scan the new alias tree for entries that aren't in the old tree
	//	Check for new existing Alias Nodes ..
	for	(uint j=0;j<newTree.getChildCount();j++)
	{
		CAIAliasDescriptionNode*const	newAliasChild	=	newTree.getChild(j);
		CAliasTreeOwner*	currentDeeperChild=this;

		breakable
		{

			//	special case, the node represents a folder. Have to parse him normally on the current object.
			if	(newAliasChild->getType()==AITypeFolder)
				break;
			
			CAliasTreeOwner*	childOwner	=	NULL;
			IAliasCont*			cont		=	NULL;
			
			//	we try to get a valid IAliasCont for this type.
			if	(!getCont(childOwner,cont,newAliasChild->getType()))
			{
				//	kick hack to prevent special parsing case.
				if (	newAliasChild->getType()!=AITypeEventAction
					&&	newAliasChild->getType()!=AITypeFaunaSpawnAtom	)
				{
					if (LogAliasTreeOwner)
						nlwarning("ATO: '%s' not found in '%s', Skeeping ..", std::string(AITYPES::getName(newAliasChild->getType())).c_str(), getName().c_str()	);
				}
				break;
			}
			
			CAliasTreeOwner*	child	=	cont->getAliasChildByAlias(newAliasChild->getAlias());
			if	(!child)	//	a child related to this alias not yet exists ..
			{
				//	so we ask to create one objet of this type ( implementation is specialized :) )
				//	giving it the so precious CAIAliasDescriptionNode.
				//	assumes that it adds the child to its parent ( check by the next assert ).
				child=childOwner->createChild(cont,newAliasChild);
				nlassert(cont->getAliasChildByAlias(newAliasChild->getAlias())!=NULL);
				
				if (LogAliasTreeOwner)
					nldebug("ATO: In '%s' @ %p, created child '%s' @ %p", this->getName().c_str(), this, child->getName().c_str(), child);
			}
			
			if (!child)
			{
				nlwarning("ATO: Cannot create child '%s' in '%s', Skeeping ..", std::string(AITYPES::getName(newAliasChild->getType())).c_str(), getName().c_str()	);
				break;
			}
			currentDeeperChild=child;

		}
		currentDeeperChild->updateAliasTree(*newAliasChild);
		updateDependencies(*newAliasChild, currentDeeperChild);
	}
	_CurrentOwnerList.pop_back();
	
}


bool	CAliasTreeOwner::getCont(CAliasTreeOwner	*&childOwner,	IAliasCont	*&cont,	TAIType	_type)
{
	std::vector<NLMISC::CDbgPtr<CAliasTreeOwner> >::reverse_iterator first(_CurrentOwnerList.rbegin()), last(_CurrentOwnerList.rend());
	for	(; first != last; ++first)
	{
		cont=(*first)->getAliasCont(_type);
		if	(!cont)
			continue;
		childOwner	=	*first;
		return	true;
	}
	return false;
}
