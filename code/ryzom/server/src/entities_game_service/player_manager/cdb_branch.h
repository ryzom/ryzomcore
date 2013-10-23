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



#ifndef CDB_BRANCH_H
#define CDB_BRANCH_H

#include "player_manager/cdb.h"



/**
 * Database Node which contains a set of properties
 * \author Stephane Coutelas, Olivier Cado
 * \author Nevrax France
 * \date 2002, 2003
 */
class CCDBStructNodeBranch : public ICDBStructNode
{
	NL_INSTANCE_COUNTER_DECL(CCDBStructNodeBranch);
public:

	// default constructor
	CCDBStructNodeBranch() : ICDBStructNode(), _Parent(0), _IdBits(0) {}

	/**
	 *	Build the structure of the database from a file
	 * \param f is the stream
	 */
	void init( xmlNode *node, class NLMISC::IProgressCallback &progressCallBack );

	// Allocate and return a copy of the branch, but with no node nor label
	//CCDBStructNodeBranch	*cloneBranchWithoutNodes();

	/**
	 * Add a new sub node
	 * \param node is the new subnode
	 * \param nodeName is the name of the node
	 */
	void attachChild( ICDBStructNode *node, const std::string& nodeName );

	/**
	 * Set a node pt to NULL (not deleting)
	 */
	void detachChild( ICDBStructNode *node );

	/**
	 * Get a node
	 * \param ids is the list of property index
	 * \param idx is the property index of the current node(updated by the method,should be 0 at first call)
	 */
	ICDBStructNode * getNode( std::vector<uint16>& ids, uint idx );

	/**
	 * Get a node . Create it if it does not exist yet
	 * \param id : the CTextId identifying the node
	 */
	ICDBStructNode * getNode( const CTextId& id, bool bCreate=true );

	/**
	 * Get a node
	 * \param idx is the node index
	 */
	ICDBStructNode * getNode( uint16 idx );

	/// Get a node (const)
	const ICDBStructNode * getNode( uint16 idx ) const
	{
		nlassert( idx < _Nodes.size() );
		return _Nodes[idx];
	}


	/**
	 * Get a node index
	 * \param node is a pointer to the node
	 */
	virtual bool getNodeIndex( ICDBStructNode* node , uint& index) const
	{
		index=0;
		for ( std::vector<ICDBStructNode*>::const_iterator it = _Nodes.begin(); it != _Nodes.end(); ++it)
		{
			if (*it == node)
				return true;
			index++;
		}
		return false;
	}

	/**
	 *	Destructor
	 */
	virtual ~CCDBStructNodeBranch();

	// the parent node for a branch (NULL by default)
	virtual void setParent(CCDBStructNodeBranch *parent) { _Parent=parent; }

	// Get the node parent
	virtual CCDBStructNodeBranch*  getParent()
	{
		return _Parent;
	}

	// Get the node parent (const)
	const CCDBStructNodeBranch *getParent() const	{ return _Parent; }

	// get the node name
	const std::string * getName()
	{
		if (_Parent == NULL) return NULL;
		for (uint16 i = 0; i < (uint16)_Parent->_Nodes.size() ;i++)
			if( _Parent->_Nodes[i] == this)
				return &(_Parent->_Names[i]);

		return NULL;
	}

	// get a child name
	const std::string * getNodeName(uint16 index) const
	{
		if (index >= _Names.size() )
			return NULL;
		return &_Names[index];
	}

	//get the number of nodes
	uint16 getNbNodes() const
	{
		return (uint16)
			_Nodes.size();
	}

	/**
	 * Return the data index corresponding to a text id (from the root)
	 */
	TCDBDataIndex	findDataIndex( ICDBStructNode::CTextId& id ) const;

	/**
	 * Set the data index into the leaves. The passed index is incremented for each leaf leaf or atomic branch.
	 * The "returned" value of index is the number of indices set.
	 */
	void			initDataIndex( TCDBDataIndex& index );

	/**
	 * For each different index (leaf of atomic branch), call the callback. Sets the id in leaves.
	 */
	void			initIdAndCallForEachIndex( CBinId& id, void (*callback)(ICDBStructNode*, void*), void *arg );

	/**
	 * Browse the tree, and for each atom branch encountered, call the callback passing the argument
	 * provided and the index of the atom branch.
	 */
	void			foreachAtomBranchCall( void (*callback)(void*,TCDBDataIndex), void *arg ) const;

	/**
	 * Browse the tree, building the text id, and for each leaf encountered, call the callback
	 * passing the argument provided, the index and the text id.
	 */
	void			foreachLeafCall( void (*callback)(void*,TCDBDataIndex,CTextId*), CTextId& id, void *arg );

	/**
	 * Browse the tree, and for each leaf encountered, call the callback
	 * passing the argument provided and the leaf, and post-incrementing the counter
	 */
	void			foreachLeafCall( void (*callback)(void*,CCDBStructNodeLeaf*,uint&), uint& counter, void *arg );

	/**
	 * Return the first leaf found, and set the number of indirections in siblingLevel
	 */
	const CCDBStructNodeLeaf *findFirstLeaf( uint& siblingLevel ) const;

	/// Label the subnodes
	void			labelSiblings( TCDBBank bank )
	{
		// Recurse subnodes
		std::vector<ICDBStructNode*>::iterator in;
		for ( in=_Nodes.begin(); in!=_Nodes.end(); ++in )
		{
			(*in)->labelBranch( bank );
		}
	}

	/// Move the siblings that match the bank name to the destination tree
	void			moveBranchesToBank( CCDBStructNodeBranch *destRoot, TCDBBank bank );

	/// Count and store the number of bits required to store the id
	void			calcIdBits();

	/// Return the number of bits required to store the id
	sint			idBits() const { return _IdBits; }

	/// Build the binary id corresponding to the branch (it must have at least one leaf)
	void			buildBinIdFromLeaf( CBinId& binId ) const;

	/// Return a pointer to a node corresponding to a bank and a property name (from the root)
	ICDBStructNode *getICDBStructNodeFromNameFromRoot( const std::string& name );

protected:

	/// Parent node
	CCDBStructNodeBranch			*_Parent;

	/// database subnodes
	std::vector<ICDBStructNode *>	_Nodes;

	///
	std::map<std::string,uint32>	_Index;

	///
	std::vector<std::string>		_Names;

	/// number of bits required to stock my children's ids
	sint							_IdBits;
};


#endif // CDB_BRANCH_H






