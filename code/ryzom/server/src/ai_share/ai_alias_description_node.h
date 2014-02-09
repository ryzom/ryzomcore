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




#ifndef RYAI_AI_ALIAS_DESCRIPTION_NODE_H
#define RYAI_AI_ALIAS_DESCRIPTION_NODE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/smart_ptr.h"
#include "nel/net/message.h"
#include "nel/ligo/ligo_config.h"

#include "game_share/persistent_data.h"

#include "ai_types.h"

#include <string>
#include <vector>


extern NLLIGO::CLigoConfig LigoConfig;
// The following serve for controling verbose nature of logging - LOG is undefined at end of file
extern bool VerboseAliasDescriptionNodeLog;
#define LOG if (!VerboseAliasDescriptionNodeLog) {} else nlinfo

/*
  -----------------------------------------------------------------------------

  CAIAliasDescriptionNode is the class used for representing the tree of
	valid alias values with their coresponding object names and types

  -----------------------------------------------------------------------------
*/

class	CAIAliasDescriptionNode;

//	TODO...
class CAIAliasDescriptionNode : public NLMISC::CRefCount
{
public:

	// construction and destruction
	CAIAliasDescriptionNode(std::string name, uint32 alias, AITYPES::TAIType type, CAIAliasDescriptionNode *parent):
		_name(name), _alias(alias), _type(type), _parent(parent)
	{
		// add self to parent's vector of children
		if	(!_parent.isNull())
		{
			_parent->_children.push_back(this);
		}
		_aliasDescriptionList.push_back(this);
	}

	static	std::vector<NLMISC::CSmartPtr<CAIAliasDescriptionNode> >	_aliasDescriptionList;
	static	void	flushUnusedAliasDescription	()
	{
		_aliasDescriptionList.clear	();
	}
		
	virtual ~CAIAliasDescriptionNode()
	{
		// remove my own children
		#ifdef NL_DEBUG
		nlassert(_children.empty());
		#endif

		// remove self from parent's vector of children
		if	(!_parent.isNull())
		{
			_parent->removeChild	(this);
		}

	}

	void	removeChild	(CAIAliasDescriptionNode	*child)
	{
		for (uint i=0;i<_children.size();++i)
		{
			if (_children[i]==child)
			{
				_children[i]=_children[_children.size()-1];
				_children.pop_back();
				return;
			}
		}
		#ifdef NL_DEBUG
		nlassert(false);
		#endif
	}
	
	// read accessors

	const std::string &					getName		()	const { static std::string null="NULL"; return (this!=NULL)?_name:null; }
	uint32								getAlias	()	const { return _alias; }
	std::string							getAliasString	()	const { return LigoConfig.aliasToString(_alias); }
	AITYPES::TAIType					getType		()	const { return _type; }
	const	NLMISC::CSmartPtr<CAIAliasDescriptionNode>	&getParent	()	const { return _parent; }

	// tree parse and search routines
	uint32				getChildCount		()	const	{ nlassert(this!=NULL);	return	(uint32)_children.size();	}
	CAIAliasDescriptionNode		* const &getChild	(uint32 idx)	const	{ return	_children[idx];		}
	const	CAIAliasDescriptionNode	*lookupAlias	(uint32 alias)	const
	{
		// see if the alias searched for is the one in this object
		if (_alias==alias)
			return	this;

		// look in the child objects
		for (uint i=0;i<_children.size();++i)
		{
			const CAIAliasDescriptionNode *node=_children[i]->lookupAlias(alias);
			if (node!=NULL)
				return node;
		}

		// not found so return a NULL pointer
		return NULL;
	}

	// Debug tools (may not be fast at all!!!)
	std::string	fullName() const
	{
		static std::string null="NULL";
		if (!this)
			return null;

		std::string result=_name;
		for (CAIAliasDescriptionNode *node=_parent; node; node=node->_parent)
			result=node->_name+':'+result;
		return result;
	}

//	std::string	treeToString() const
//	{
//		static std::string null="NULL";
//		if (!this)
//			return null;
//
//		std::string result=NLMISC::toString("[%08x:%s:%s]",_alias,AITYPES::getName(_type),_name.c_str());
//		if (!_children.empty())
//		{
//			result+='(';
//			for (uint i=0;i<_children.size();++i)
//				result+=_children[i]->treeToString();
//			result+=')';
//		}
//		return result;
//	}

	//-------------------------------------------------------------------------------------------
	// 	handy utilities
	//-------------------------------------------------------------------------------------------

	const	CAIAliasDescriptionNode	*getChildByAlias	(uint32	alias)	const
	{
		for	(int i=getChildCount()-1;i>=0;i--)
			if (getChild(i)->getAlias()==alias)
				return	getChild(i);
		return	NULL;
	}


	const	CAIAliasDescriptionNode	*findNodeChildByNameAndType(const std::string &name, AITYPES::TAIType type,bool recurse=false) const
	{
		// first look for a child here with the correct properties
		for (uint i=0;i<getChildCount();++i)
		{
			if (type!=getChild(i)->getType()) continue;
			if (NLMISC::nlstricmp(name,getChild(i)->getName())!=0) continue;
			return getChild(i);
		}

		// no child found in this node so try recursing through children
		if (recurse)
			for (uint i=0;i<getChildCount();++i)
			{
				const CAIAliasDescriptionNode *result=getChild(i)->findNodeChildByNameAndType(name,type,true);
				if (result)
					return result;
			}

		// not found so return NULL
		return NULL;
	}

	const	CAIAliasDescriptionNode	*findNodeByFullNameAndType(const std::string &name, AITYPES::TAIType type) const
	{
		// first off - if our type matches, look and see whether current node name ends with the searched for name
		if (getType()==type)
		{
			std::string nodeName=fullName();
			if (nodeName.size()>=name.size() && NLMISC::nlstricmp(name,nodeName.substr(nodeName.size()-name.size()))==0)
				if (nodeName.size()==name.size()||nodeName[nodeName.size()-name.size()-1]==':')
					return this;
		}

		// no luck so recurse
		for (uint i=0;i<getChildCount();++i)
		{
			const CAIAliasDescriptionNode *result=getChild(i)->findNodeByFullNameAndType(name,type);
			if (result)
				return result;
		}

		// not found so return NULL
		return NULL;
	}

	uint32	findAliasByNameAndType(const std::string &name, AITYPES::TAIType type) const
	{
		const CAIAliasDescriptionNode *resultNode;
		// try recursing node's children to find a suitable candidate
		resultNode=findNodeChildByNameAndType(name,type,true);
		if (resultNode)
			return resultNode->getAlias();

		// try recursing node's parents for a suitable candidate
		for (const CAIAliasDescriptionNode *it=getParent();it!=NULL;it=it->getParent())
		{
			resultNode=it->findNodeChildByNameAndType(name,type,false);
			if (resultNode)
				return resultNode->getAlias();
		}

		// not found so try recursing the whole tree out of pure desparation
		const CAIAliasDescriptionNode *rootNode=this;
		while (rootNode->getParent()) 
			rootNode=rootNode->getParent();
		resultNode=rootNode->findNodeByFullNameAndType(name,type);
		if (resultNode)
			return resultNode->getAlias();

		resultNode = rootNode->findNodeChildByNameAndType(name,type, true);
		if (resultNode)
			return resultNode->getAlias();

		return 0;
	}

	void pushToPdr(CPersistentDataRecord& pdr) const
	{
		pdr.push(pdr.addString("name"),_name);
		pdr.push(pdr.addString("alias"),_alias);
		pdr.push(pdr.addString("type"),(uint16)_type);
		if (!_children.empty())
		{
			for (uint32 i=0;i<_children.size();++i)
			{
				pdr.pushStructBegin(pdr.addString("child"));
				_children[i]->pushToPdr(pdr);
				pdr.pushStructEnd(pdr.addString("child"));
			}
		}
	}

	static CAIAliasDescriptionNode *popFromPdr(CPersistentDataRecord& pdr)
	{
		std::string			name;
		uint32				alias=~0u;
		AITYPES::TAIType	type= AITYPES::AITypeBadType;
		std::vector<CAIAliasDescriptionNode *>	children;

		while (!pdr.isEndOfStruct())
		{
			uint16 token= pdr.peekNextToken();
			const std::string& tokenName= pdr.peekNextTokenName();
			if (tokenName=="name")		{ name=						pdr.popNextArg(token).asString();	continue; }
			if (tokenName=="alias")		{ alias=	(uint32)		pdr.popNextArg(token).asUint();		continue; }
			if (tokenName=="type")		{ type=	(AITYPES::TAIType)	pdr.popNextArg(token).asUint();		continue; }
			if (tokenName=="child")
			{
				pdr.popStructBegin(token);
				vectAppend(children)= CAIAliasDescriptionNode::popFromPdr(pdr);
				pdr.popStructEnd(token);
				continue;
			}
			WARN("Unrecognised content found in pdr: "+tokenName);
			if (pdr.isStartOfStruct())
				pdr.skipStruct();
			else
				pdr.skipData();
		}
		CAIAliasDescriptionNode *result= new CAIAliasDescriptionNode(name,alias,type,NULL);
		result->_children= children;
		for (uint32 i=0;i<result->_children.size();++i)
			result->_children[i]->_parent= result;
		return result;
	}

private:
	// data
	const std::string			_name;
	const uint32				_alias;
	const AITYPES::TAIType		_type;
	NLMISC::CSmartPtr<CAIAliasDescriptionNode>	_parent;
	std::vector<CAIAliasDescriptionNode *>		_children;
};

#undef LOG

#endif
