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

/**
  *  This file contains an extension of the 'persistent data record' system
  *  It represents the contents of arbitrary CPersistentDataRecord records in a tree structure
  *  that can be interrogated or written to / read from easy to read text files
  *
  **/

#ifndef PERSISTENT_DATA_TREE_H
#define	PERSISTENT_DATA_TREE_H

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "persistent_data.h"


//-----------------------------------------------------------------------------
// class CPersistentDataTreeNode
//-----------------------------------------------------------------------------

class CPersistentDataTreeNode: public NLMISC::CRefCount
{
public:
	// typedefs
	typedef NLMISC::CSString TValue;
	typedef NLMISC::CSmartPtr<CPersistentDataTreeNode>	TNodePtr;
	typedef NLMISC::CRefPtr<CPersistentDataTreeNode>	TNodeRefPtr;
	typedef std::vector<TNodePtr> TChildren;
	typedef std::map<NLMISC::CSString,TNodePtr> TChildIndex;
	typedef std::map<NLMISC::CSString,int> TNextPoundValue;

	// ctor
	CPersistentDataTreeNode(const NLMISC::CSString& name=NLMISC::CSString(),CPersistentDataTreeNode* parent=NULL);
	CPersistentDataTreeNode(const NLMISC::CSString& name,CPersistentDataTreeNode* parent,uint32 idx);

	// attaching a node to a parent node
	// - note: it is not possible to detach a node once attached
	bool attachToParent(CPersistentDataTreeNode* parent);
	bool attachToParent(CPersistentDataTreeNode* parent,uint32 idx);

	// reading / writing pdr objects
	bool readFromPdr(CPersistentDataRecord& pdr);
	bool writeToPdr(CPersistentDataRecord& pdr) const;

	// writing to a text buffer
	bool writeToBuffer(NLMISC::CSString& buffer) const;

	// 'name' accessors
	const NLMISC::CSString& getName() const;
	NLMISC::CSString getNodeName() const;

	// accessor to find out whether this is a standard property or a map entry
	bool isMapEntry() const;
	// flag a branch as being a map (children are map entries)
	// returns true on success, false if the branch
	// contains a value or if the branch contains children that are not map entries
	bool flagAsMap();

	// 'value' accessors
	void	setValue(const TValue& value);
	const	TValue& getValue() const;
	const	TValue& getValue(const NLMISC::CSString& nameList) const;

	// 'child' accessors
	const CPersistentDataTreeNode* getChild(const NLMISC::CSString& name) const;
	CPersistentDataTreeNode* getChild(const NLMISC::CSString& name);
	const TChildren& getChildren() const;
	CPersistentDataTreeNode* getDescendant(const NLMISC::CSString& nameList,bool createIfNotExist=false);

	// compare two tree nodes (verify that their contents are eqivalent)
	bool operator==(const CPersistentDataTreeNode& other) const;

private:
	// private data common to all nodes
	NLMISC::CSString	_Name;
	TNodeRefPtr			_Parent;
	bool				_IsValue;

	// value for leaf nodes
	TValue				_Value;

	// containers for children and associated indexes etc for branch nodes
	TChildren			_Children;
	TChildIndex			_ChildIndex;
	TNextPoundValue		_NextPoundValue;
	bool				_IsMap;
};


//-----------------------------------------------------------------------------
// class CPersistentDataTree
//-----------------------------------------------------------------------------

class CPersistentDataTree
{
public:
	// typedefs
	typedef CPersistentDataTreeNode::TValue TValue;
	typedef CPersistentDataTreeNode::TNodePtr TNodePtr;

	// ctor
	CPersistentDataTree();

	// reading from different sources
	bool readFromBuffer(const NLMISC::CSString& buffer);
	bool readFromFile(const NLMISC::CSString& fileName);
	bool readFromPdr(CPersistentDataRecord& pdr);

	// writing to different sources
	bool writeToBuffer(NLMISC::CSString& buffer) const;
	bool writeToFile(const NLMISC::CSString& fileName) const;
	bool writeToPdr(CPersistentDataRecord& pdr) const;

	// 'value' accessors
	void   setValue(const TValue& nodeName,const NLMISC::CSString& value);
	const  TValue&  getValue(const NLMISC::CSString& nodeName) const;

	// accessors for navigaiting in the tree
	CPersistentDataTreeNode* getNode(const NLMISC::CSString& nodeName);

	// compare two data trees (verify that their contents are eqivalent)
	bool operator==(const CPersistentDataTree& other) const;

private:
	// private data
	TNodePtr _Child;
};


//-----------------------------------------------------------------------------
#endif
