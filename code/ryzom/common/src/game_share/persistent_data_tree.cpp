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

  NOTE: The file format for storing trees to disk is line based. It follows the following rules:

  NOTE too that this file format is intended as an alternative for storage of PDR data and that the PDR
  format doesn't differentiate between single values and array elements

  - Leading spaces are ignored when scanning lines, as are all spaces preceding the '=' or '==' signs
  - Any component of the paths may be quote encapsulated
  - Blank lines + those starting with '//' or with '#' are ignored
  - lines of format <path>'='<value> contain raw values
  - lines of format <path>'=='<value> contain values that will be stripped and unquoted if they were previously quoted
  - lines of format <path> declare the start of a new record for the given path
  - paths take the form: <component>'.'<component>'.'<component>...
  - components take one of the following forms:
	<name>
	<name>'#'<idx>
	<name>':'<map_key>
	<name>'#'<idx>':'<map_key>
	<name>':'<map_key>'#'<idx>
	<name>'#'<idx>':'<map_key>'#'<idx>

  EXAMPLE:
  #	a little example
  # the data format that we are representing looks like this:
  #		class ClassA { vector<int> b; int c; string d; }
  #		class ClassTheWholeFile { vector<ClassA> a; map<string,vector<string>> m; }

  // start a first record
  a
  a.b=1
  a.c=2
  a.d== "hello \" world"

  // create a seond record
  a
  a.b=3
  a.c=4
  a.d=hello world

  // append a new element to the first record
  a#0.b=6

  // change an element from the first record
  a#0.b#0=5

  // add a couple of map entries
  m:abc="def"
  m:bcd=fgh

  // add a value to the array 'm:bcd'
  m:bcd=ghi

*/


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "persistent_data_tree.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// handy utility routines
//-----------------------------------------------------------------------------

static bool needsQuotes(const CSString& src)
{
	for (uint32 i=0;i<src.size();++i)
	{
		char c= src[i];
		if (c<=32 || c=='.' || c==':' || c=='#' || c=='\"' || c=='=')
		{
			return true;
		}
	}
	return false;
}

static CSString cleanQuotes(CSString src)
{
	src= src.strip().unquoteIfQuoted();
	return (needsQuotes(src))? src.quote(): src;
}

//-----------------------------------------------------------------------------
// methods CPersistentDataTreeNode
//-----------------------------------------------------------------------------

CPersistentDataTreeNode::CPersistentDataTreeNode(const NLMISC::CSString& name,CPersistentDataTreeNode* parent)
{
	_Name= name;
	_Parent=NULL;
	_IsMap= false;
	_IsValue= false;
	attachToParent(parent);
}

CPersistentDataTreeNode::CPersistentDataTreeNode(const NLMISC::CSString& name,CPersistentDataTreeNode* parent,uint32 idx)
{
	_Name= name;
	_Parent=NULL;
	_IsMap= false;
	_IsValue= false;
	attachToParent(parent,idx);
}

bool CPersistentDataTreeNode::attachToParent(CPersistentDataTreeNode* parent)
{
	return attachToParent(parent,parent==NULL?std::numeric_limits<uint32>::max():(uint32)parent->_Children.size());
}

bool CPersistentDataTreeNode::attachToParent(CPersistentDataTreeNode* parent,uint32 idx)
{
	// check value of 'idx'
	BOMB_IF(parent != NULL && idx>1024 * 1024, "Implausibly high number of children requested (" + NLMISC::toString("%d", idx) + ") for persistent data tree node: " + parent->getNodeName().c_str(), return false);

	// check for attachment to previous parent
	BOMB_IF(_Parent != NULL, "Attempting to attach a persistent data node to parent '" + (parent == NULL ? "NULL" : parent->getNodeName()) + "' when it's already attached to another parent as: " + getNodeName().c_str(), return false);

	// split the name into its component parts
	CSString mapIndex= _Name;
	CSString nameBase= cleanQuotes(mapIndex.splitToSeparator('#',true,false,true,true,false));
	// get rid of any odd spaces floating about the map index
	bool hasExplicitIndex= false;
	if (!mapIndex.empty())
	{
		hasExplicitIndex= true;
		mapIndex= mapIndex.leftCrop(1).strip();
	}

	// add self to parent's children
	if (parent!=NULL)
	{
		// check parent isn't a value
		BOMB_IF(parent->_IsValue, "Attempting to attach a persistent data node to parent that has a value '" + parent->getNodeName() + "' = " + parent->_Value.c_str(), return false);

		if (hasExplicitIndex)
		{
			// there is an explicit '#' number so ensure that it really is followed by a valid number
			sint32 num= mapIndex.atoi();
			if(NLMISC::toString("%u",num)!=mapIndex)
			{
				WARN("WARNING: Strange node name detected after last '#' - truncating and replacing with a number: "+getNodeName());
				hasExplicitIndex=false;
			}
			else
			{
				// if this is the highest '#' number for this root node name then ajust the _NextPoundValue value
				if (num>=parent->_NextPoundValue[nameBase])
				{
					parent->_NextPoundValue[nameBase]= num+1;
				}

				// ensure that there isn't already a node with the same '#' value attached to the same parent
				DROP_IF(parent->_ChildIndex.find(nameBase + '#' + mapIndex.c_str()) != parent->_ChildIndex.end(), "Failed to add child '" + _Name + "' to parent '" + parent->getNodeName().c_str() + "' because another child of same name already exists", return false);
			}
		}
		if (!hasExplicitIndex)
		{
			// we don't have an explicit '#' number in the name, so we need to generate one
			uint32 number= parent->_NextPoundValue[nameBase]++;
			mapIndex=NLMISC::toString("%u",number);
		}

		// construct our cleaned up name from its constituent parts
		_Name = nameBase + '#' + mapIndex.c_str();

		// setup own _Parent property and ensure that there are no trailing spaces round the _Name
		_Parent= parent;

		_Parent->_ChildIndex[_Name]=this;
		_Parent->_ChildIndex[nameBase]=this;

		// grow the parent's _Children vector if needed
		if (_Parent->_Children.size()<=idx)
		{
			_Parent->_Children.resize(idx+1);
		}

		// ensure that there isn't another child already assigned to this parent slot
		BOMB_IF(_Parent->_Children[idx] != NULL, "Ignoring attempt to add second child to same slot (" + NLMISC::toString("%d", idx) + ") in persistent data tree node's children: " + _Parent->getNodeName().c_str(), return false);

		// write own pointer into parent's _Children vector
		_Parent->_Children[idx]= this;
	}

	return true;
}

bool CPersistentDataTreeNode::readFromPdr(CPersistentDataRecord& pdr)
{
	static uint16 mapKeyToken= std::numeric_limits<uint16>::max(); pdr.addString("__Key__",mapKeyToken);
	static uint16 mapValToken= std::numeric_limits<uint16>::max(); pdr.addString("__Val__",mapValToken);

	while (!pdr.isEndOfData())
	{
		if (pdr.isStartOfStruct())
		{
			// get the token corresponding to this start of block
			uint16 token=pdr.peekNextToken();
			pdr.popStructBegin(token);

			// create a new sub-node for the block in question
			CSString structName= pdr.lookupString(token);
			if (needsQuotes(structName))
				structName= structName.quote();
			TNodePtr newNode= new CPersistentDataTreeNode(structName,this);

			// recurse into new block
			bool ok= newNode->readFromPdr(pdr);
			if (!ok || pdr.isEndOfData())
				return false;

			// pop the end of block token for the block we just finished processing
			DROP_IF(pdr.peekNextToken() != token, "ERROR: End of " + pdr.lookupString(token) + " block expected but not found at: " + getNodeName().c_str(), return false);
			pdr.popStructEnd(token);
		}
		else if (pdr.isEndOfStruct())
		{
			return true;
		}
		else
		{
			uint16 nextToken=pdr.peekNextToken();
			if (nextToken==mapKeyToken)
			{
				// we've matched a map key so we need to extract both key and related value (or child block)

				// ensure the node is either a map or struct but not mixed
				if (!flagAsMap()) return false;

				// extract the map key and ensure that it's followed by a valid __Val__ entry
				CSString mapKey;
				pdr.pop(mapKeyToken,mapKey);
				DROP_IF(pdr.isEndOfData() || pdr.peekNextToken() != mapValToken, "ERROR: Ignoring map key (__Key__) because __Val__ token expected but not found at: " + getNodeName() + ":" + mapKey.c_str(), continue);
				if (needsQuotes(mapKey))
					mapKey=mapKey.quote();

				// create the new node for the map entry
				TNodePtr newNode= new CPersistentDataTreeNode(mapKey,this);

				// see whether this is a struct entry or a simple value
				if (pdr.isStartOfStruct())
				{
					// struct entry - pop the start of struct marker
					pdr.popStructBegin(mapValToken);

					// recurse into the new block
					bool ok= newNode->readFromPdr(pdr);
					if (!ok || pdr.isEndOfData())
						return false;

					// pop the end of struct marker
					DROP_IF(pdr.peekNextToken() != mapValToken, "ERROR: End of __Val__ block expected but not found at: " + getNodeName() + ":" + mapKey.c_str(), return false);
					pdr.popStructEnd(mapValToken);
				}
				else
				{
					pdr.pop(mapValToken,newNode->_Value);
					newNode->_IsValue=true;
				}
			}
			else
			{
				// we've got a classic value so create a new node and set it's value
				CSString valName= pdr.lookupString(nextToken);
				if (needsQuotes(valName))
					valName= valName.quote();

				TNodePtr newNode= new CPersistentDataTreeNode(valName,this);
				pdr.pop(nextToken,newNode->_Value);
				newNode->_IsValue=true;
			}
		}
	}

	return true;
}

static bool isFloat(double d,float f)
{
	return (double(f)==d);
}

static void pdrPushCompactValue(CPersistentDataRecord& pdr,uint16 token,const NLMISC::CSString& value)
{
	// if this is an empty string or doesn't begin with a valid numeric character then just push it to the pdr and return
	if ( value.empty() || ( ((uint32)(value[0]-'0')>9) && (value[0]!='-') ) )
	{
		// not numeric so try storing as an eid
		NLMISC::CEntityId eid;
		eid.fromString(value.c_str());
		if (eid!=NLMISC::CEntityId::Unknown && eid.toString()==value)
		{
			pdr.push(token,eid);
			return;
		}

		pdr.push(token,value);
		return;
	}

	if (value[0]=='-')
	{
		// see if we can treat this value as a signed int
		sint64 si64= value.atosi64();
		if (si64!=0)
		{
			// we can store this value as a signed int so decide how big an int we need
			sint32 si= (sint32)si64;
			if (si==si64)	pdr.push(token,si);
			else			pdr.push(token,si64);
			return;
		}
	}
	else
	{
		// not a signed int so try storing as an unsigned int
		uint64 ui64= value.atoui64();
		if (ui64!=0 || value=="0")
		{
			// we can store this value as a signed int so decide how big an int we need
			uint32 ui= (uint32)ui64;
			if (ui==ui64)	pdr.push(token,ui);
			else			pdr.push(token,ui64);
			return;
		}
	}

	// failed to store as an int so try a float
	double d = (double)value.atof();
	if (NLMISC::toString(d)==value)
	{
		float f= (float)d;
		if (isFloat(d,f))	pdr.push(token, f);
		else				pdr.push(token, d);
		return;
	}

	// default to save as a string
	pdr.push(token,value);
}

bool CPersistentDataTreeNode::writeToPdr(CPersistentDataRecord& pdr) const
{
	// extract the sub-part of the name that precedes the '#' token
	CSString name= _Name.splitToSeparator('#').unquoteIfQuoted();

	uint16 token= pdr.addString(name);

	// if this is a map entry then split into map key and value
	if (isMapEntry())
	{
		static uint16 mapKeyToken= std::numeric_limits<uint16>::max(); pdr.addString("__Key__",mapKeyToken);
		static uint16 mapValToken= std::numeric_limits<uint16>::max(); pdr.addString("__Val__",mapValToken);

		// write a value - try to match a reasonably compact format if one exists
		pdrPushCompactValue(pdr,mapKeyToken,name);

		// setup the valToken for writing value using common code...
		token= mapValToken;
	}

	if (_IsValue)
	{
		// write a value - try to match a reasonably compact format if one exists
		pdrPushCompactValue(pdr,token,_Value);
	}
	else
	{
		// open a new block in the pdr
		pdr.pushStructBegin(token);

		// iterate over children recursing...
		bool ok=true;
		for (uint32 i=0;i<_Children.size() && ok;++i)
		{
			BOMB_IF(_Children[i]==NULL,"ERROR: NULL pointer in persistent data tree node children",return false);
			ok= _Children[i]->writeToPdr(pdr);
		}

		// close the block inthe pdr
		pdr.pushStructEnd(token);
		BOMB_IF(!ok,"Giving up writeToPdr() "+_Name,return false);
	}
	return true;
}

bool CPersistentDataTreeNode::writeToBuffer(NLMISC::CSString& buffer) const
{
	if (_IsValue)
	{
		// write a value
		if (needsQuotes(_Value))
			buffer += getNodeName() + "==" + _Value.quote().c_str() + "\n";
		else
			buffer += getNodeName() + "=" + _Value.c_str() + "\n";
	}
	else
	{
		if (_Children.empty())
		{
			// write an empty block
			buffer+= getNodeName()+"\n";
		}

		// iterate over children recursing...
		for (uint32 i=0;i<_Children.size();++i)
		{
			BOMB_IF(_Children[i]==NULL,"ERROR: NULL pointer in persistent data tree node children",return false);
			bool ok= _Children[i]->writeToBuffer(buffer);
			BOMB_IF(!ok,"Giving up writeToBuffer() "+_Name,return false);
		}
	}
	return true;
}

const CSString& CPersistentDataTreeNode::getName() const
{
	return _Name;
}

CSString CPersistentDataTreeNode::getNodeName() const
{
	// extract the sub-part of the name that precedes the '#' token
	CSString name=_Name.splitToSeparator('#');

	// if we have no parent then our name is obviously just the 'name' value
	if (_Parent==NULL)
		return name;

	// check to see whether there are more then one child in our parent that match our base name
	if (_Parent->_NextPoundValue[name]>1)
	{
		// other siblings exist so we need to retain th full name including the '#' segment
		name=_Name;
	}

	// recurse trhough parents to build the name root
	CSString parentName= _Parent->getNodeName();

	// return one of name, parentName.name and parentName:name
	if (parentName.empty())	return name;
	if (isMapEntry())		return parentName + ":" + name.c_str();
	else					return parentName + "." + name.c_str();
}

void   CPersistentDataTreeNode::setValue(const TValue& value)
{
	DROP_IF(!_Children.empty(),"Ignoring attempt to set value of a node that has children: "+getNodeName(),return);
	_Value= value;
	_IsValue= true;
}

const  CPersistentDataTreeNode::TValue& CPersistentDataTreeNode::getValue() const
{
	WARN_IF(!_IsValue,"Attempting to get value of a node that isn't flagged as being one: "+getNodeName());
	return _Value;
}

const  CPersistentDataTreeNode::TValue&  CPersistentDataTreeNode::getValue(const CSString& nameList) const
{
	// get hold of the named node, or NULL if it doesn't exist
	CPersistentDataTreeNode* node= const_cast<CPersistentDataTreeNode*>(this)->getDescendant(nameList,false);

	// check whether the node exists
	static TValue nullValue;
//	DROP_IF(node==NULL,"Failed to locate node in getValue("+nameList+")",return nullValue);
	if(node==NULL)
		return nullValue;

	// return the node's value
	return node->getValue();
}

bool CPersistentDataTreeNode::isMapEntry() const
{
	return (_Parent!=NULL) && (_Parent->_IsMap);
}

bool CPersistentDataTreeNode::flagAsMap()
{
	DROP_IF(_IsValue, "ERROR: Failed to flag persistent data tree node '" + getNodeName() + "' as a map as it already has a value: " + _Value.c_str(), return false);

	if (_IsMap)
		return true;

	DROP_IF(!_Children.empty(),"ERROR: Failed to flag persistent data tree node '"+getNodeName()+"' as a map as it already has children: ",return false);

	_IsMap= true;
	return true;
}

const  CPersistentDataTreeNode* CPersistentDataTreeNode::getChild(const NLMISC::CSString& name) const
{
	return const_cast<CPersistentDataTreeNode*>(this)->getChild(name);
}

CPersistentDataTreeNode* CPersistentDataTreeNode::getChild(const NLMISC::CSString& name)
{
	TChildIndex::iterator it= _ChildIndex.find(name);
	return (it==_ChildIndex.end())? NULL: (*it).second;
}

const  CPersistentDataTreeNode::TChildren& CPersistentDataTreeNode::getChildren() const
{
	WARN_IF(!_Value.empty(),"Attempting to access children of a node that has a value: "+getNodeName());
	return _Children;
}

CPersistentDataTreeNode* CPersistentDataTreeNode::getDescendant(const NLMISC::CSString& nameList,bool createIfNotExist)
{
	CSString s=nameList.strip();

	// if the node name is empty then this is the node we're after...
	if (s.empty())
	{
		return this;
	}

	// extract the next chunk from the name list and get a pointer to the identified node
	CSString nextChunk= s.splitToSeparator('.',true,false,true,true,true).strip();

	//  break the chunk into elements and perform a syntax check
	CVectorSString elements;
	nextChunk.splitByOneOfSeparators("#:",elements,false,true,true,true,false);
	switch (elements.size())
	{
	case 7:
		DROP_IF(elements[1]!='#'||elements[3]!=':'||elements[5]!='#',"SYNTAX ERROR in persistent data tree node name: "+nextChunk,return NULL);
		break;
	case 5:
		DROP_IF(elements[3]!='#'&&elements[3]!=':',"SYNTAX ERROR in persistent data tree node name: "+nextChunk,return NULL);
		DROP_IF(elements[1]==':'&&elements[3]==':',"SYNTAX ERROR in persistent data tree node name: "+nextChunk,return NULL);
		// drop through (no break)
	case 3:
		DROP_IF(elements[1]!='#'&&elements[1]!=':',"SYNTAX ERROR in persistent data tree node name: "+nextChunk,return NULL);
		// drop through (no break)
	case 1:
		break;
	default:
		DROP("SYNTAX ERROR in persistent data tree node name: "+nextChunk,return NULL);
	}
	uint32 elementsToSkip=0;

	// if this is a map entry then get the parent element
	CPersistentDataTreeNode* container= this;
	if (elements.size()>=5 || (elements.size()>=3 && elements[1]==":"))
	{
		// compose the map name
		CSString mapName= cleanQuotes(elements[0]);
		if (elements[1]=='#')
		{
			mapName+=elements[1]+elements[2];
			elementsToSkip=4;
		}
		else
		{
			elementsToSkip=2;
		}

		// this is a true map entry so it's composed of 2 parts ... we treat the first part here and drop through to the
		// normal code to treat the second part
		container= getDescendant(mapName.strip(),createIfNotExist);
		if (container==NULL || !container->flagAsMap())
			return NULL;
	}

	// compose the chunk name
	nextChunk.clear();
	for (uint32 i=elementsToSkip;i<elements.size();++i)
	{
		nextChunk+= ((i&1)==1)? elements[i]: cleanQuotes(elements[i]);
	}

	CPersistentDataTreeNode* nextNode= container->getChild(nextChunk);

	// if the node didn't exist...
	if (nextNode==NULL)
	{
		// if we're not allowed to create non-existant nodes then return NULL
		if (createIfNotExist==false)
			return NULL;

		// create a new node...
		nextNode= new CPersistentDataTreeNode(nextChunk,container);
	}

	// recurse...
	return nextNode->getDescendant(s,createIfNotExist);
}

bool CPersistentDataTreeNode::operator==(const CPersistentDataTreeNode& other) const
{
	// make sure the names match
	if (_Name != other._Name)
		return false;

	// make sure the 'type' booleans match
	if (_IsMap != other._IsMap)
		return false;
	if (_IsValue != other._IsValue)
		return false;

	// are we a node or a value?
	if (_IsValue)
	{
		// check that values match
		if (_Value != other._Value)
			return false;
	}
	else
	{
		// check that children match
		if (_Children.size() != other._Children.size())
			return false;
		for (uint32 i=0;i<_Children.size();++i)
		{
			// Check whether this child matches the other tree node's equivalent child
			if (!(*_Children[i]==*other._Children[i]))
				return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// methods CPersistentDataTree
//-----------------------------------------------------------------------------

CPersistentDataTree::CPersistentDataTree()
{
	_Child= new CPersistentDataTreeNode;
}

bool CPersistentDataTree::readFromBuffer(const CSString& buffer)
{
	// generate a new _Child object and implicitly liberate the previous one
	_Child= new CPersistentDataTreeNode;

	// convert the buffer to lines
	CVectorSString lines;
	buffer.splitLines(lines);

	// iterate over the lines...
	for (uint32 i=0;i<lines.size();++i)
	{
		// make sure this isn't a blank or comment line
		CSString line= lines[i].leftStrip();
		if (line.empty()||line[0]=='#'||line.left(2)=="//")
		{
			continue;
		}

		// split the line into key (s0) and value (s1)
		CSString s1=line;
		CSString s0=s1.splitToSeparator('=',true,false,true,true,false).strip();
		s1=s1.leftStrip();

		// we need to create a new tree node (eg a new struct, array entry or value)

		// split the key string into its component parts
		CVectorSString components;
		s0.splitBySeparator('.',components);

		// extract the child name
		CSString childName= components.back();

		// construct the parent name
		components.pop_back();
		CSString parentName;
		parentName.join(components,'.');

		// if the child name is a map entry then we have a special case to deal with
		CSString mapName= childName.splitToSeparator(':',true,false,true,true,true);
		if (childName.empty())
		{
			childName=mapName;
			mapName.clear();
		}
		else
		{
			if (!parentName.empty())
				parentName+='.';
			parentName+= mapName;
		}

		// get hold of a pointer to the parent node, creating new tree structure if need be
		CPersistentDataTreeNode* parent= _Child->getDescendant(parentName,true);
		DROP_IF(parent==NULL,"ERROR Failed to get handle to node in readFromBuffer: "+parentName,return false);

		// if this is a map entry then try to flag the parent as a map
		if (!mapName.empty())
		{
			if (!parent->flagAsMap())
				return false;
		}

		// create the new child node and attach it to the parent
		// note that by using a smart ptr (TNodePtr) we avoid memory leakage if attach fails
		CPersistentDataTree::TNodePtr child= new CPersistentDataTreeNode(childName,parent);

		// if there is a value then add it
		if (!s1.empty())
		{
			// note that we use the 2 parameter version of setValue here and NOT child->setValue because
			// it is possible that the attempt to attach a child failed and that there is another node
			// in the tree that will accept the 'setValue()' instead
			if (s1.left(2)=="==")
				setValue(s0,s1.leftCrop(2).strip().unquoteIfQuoted());
			else /* in this case we have a single "=" and not an "==" */
				setValue(s0,s1.leftCrop(1).leftStrip());
		}
	}

	return true;
}

bool CPersistentDataTree::readFromFile(const CSString& fileName)
{
	// generate a new _Child object and implicitly liberate the previous one
	_Child= new CPersistentDataTreeNode;

	CSString buffer;
	buffer.readFromFile(fileName);
	DROP_IF(buffer.empty(),"Failed to read input file for persistent data tree: "+fileName,return false);
	return readFromBuffer(buffer);
}

bool CPersistentDataTree::readFromPdr(CPersistentDataRecord& pdr)
{
	// generate a new _Child object and implicitly liberate the previous one
	_Child= new CPersistentDataTreeNode;

	bool ok= _Child->readFromPdr(pdr);
	if (!ok || !pdr.isEndOfData())
	{
		// we've had a problem so give up cleanly

		// delete the child object and create a new empty one
		_Child= new CPersistentDataTreeNode;

		return false;
	}

	return true;
}

bool CPersistentDataTree::writeToBuffer(CSString& buffer) const
{
	// clear out the buffer before starting
	buffer.clear();

	// write the tree to the buffer
	bool ok= _Child->writeToBuffer(buffer);

	// if there was an error then clear out the buffer before returning
	if (!ok)
		buffer.clear();

	return ok;
}

bool CPersistentDataTree::writeToFile(const CSString& fileName) const
{
	// create a new text buffer and call writeToBuffer() to do the work
	CSString buffer;
	bool ok= writeToBuffer(buffer);

	// in the case of an error display a warning and drop throught to write an empty buffer to the output file
	WARN_IF(!ok,"Failed to build output buffer for persistent data tree to write to file: "+fileName);

	// write the output file
	return buffer.writeToFile(fileName);
}

bool CPersistentDataTree::writeToPdr(CPersistentDataRecord& pdr) const
{
	// clear out the pdr before starting
	pdr.clear();

	// iterate over children recursing...
	bool ok=true;
	for (uint32 i=0;i<_Child->getChildren().size() && ok;++i)
	{
		BOMB_IF(_Child->getChildren()[i]==NULL,"ERROR: NULL pointer in persistent data tree node children",return false);
		ok= _Child->getChildren()[i]->writeToPdr(pdr);
	}

	// if an error occured during write then clear out the result pdr and return false
	if (!ok)
		pdr.clear();

	return ok;
}

void   CPersistentDataTree::setValue(const TValue& nodeName,const CSString& value)
{
	// get hold of the named node, creating it if need be
	CPersistentDataTreeNode* node= _Child->getDescendant(nodeName,true);
	DROP_IF(node==NULL,"ERROR: Failed get to handle to node in order to set value: "+nodeName,return);

	// set the node's value
	node->setValue(value);
}

const  CPersistentDataTree::TValue&  CPersistentDataTree::getValue(const CSString& nodeName) const
{
	return _Child->getValue(nodeName);
}

CPersistentDataTreeNode* CPersistentDataTree::getNode(const NLMISC::CSString& nodeName)
{
	return (_Child==NULL)? NULL: _Child->getDescendant(nodeName,false);
}

bool CPersistentDataTree::operator==(const CPersistentDataTree& other) const
{
	return *_Child == *other._Child;
}
