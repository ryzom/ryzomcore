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



#ifndef CDB_LEAF_H
#define CDB_LEAF_H

#include "player_manager/cdb.h"
#include "player_manager/cdb_branch.h"


/**
 * Database node which contains a unique property
 * \author Stephane Coutelas, Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CCDBStructNodeLeaf : public ICDBStructNode
{
public:

	/// Return the type of the property.
	inline const EPropType &	type() const {return _Type;}

	/// The overridden version that is usable from ICDBStructNode
	virtual EPropType			getType() const		{return _Type;}


	/// Set the property Type.
	inline void					type(const EPropType &t) {_Type = t;}

	/**
	 * Default constructor
	 */
	CCDBStructNodeLeaf() : ICDBStructNode(), _Parent( NULL ), _Type( UNKNOWN ) {}
	
	/**
	 *	Build the structure of the database from a file
	 * \param f is the stream
	 */
	void init( xmlNode *node, NLMISC::IProgressCallback &progressCallBack );

	/**
	 * Get a node
	 * \param ids is the list of property index
	 * \param idx is the property index of the current node(updated by the method)
	 */
	ICDBStructNode * getNode( std::vector<uint16>& ids, uint idx );

	/**
	 * Get a node
	 * \param idx is the node index
	 */
	ICDBStructNode * getNode( uint16 idx );

	/**
	 * Get a node . Create it if it does not exist yet
	 * \param id : the CTextId identifying the node
	 */
	ICDBStructNode * getNode( const CTextId& id, bool bCreate=true ) ;

	/**
	 * Get a node index
	 * \param node is a pointer to the node
	 */
	bool getNodeIndex( ICDBStructNode* node , uint& index) const
	{
		return false;
	}

	// the parent node for a branch (NULL by default)
	void setParent(CCDBStructNodeBranch* parent) { _Parent=parent; }

	// Get the node parent
	virtual CCDBStructNodeBranch* getParent()
	{
		return _Parent;
	}

	// Get the node parent (const)
	const CCDBStructNodeBranch *getParent() const	{ return _Parent; }

	// get the node name
	const std::string * getName()
	{
		if (_Parent == NULL) return NULL;
		for (uint16 i = 0; i < _Parent->getNbNodes() ;i++)
		if (_Parent->getNode(i) == this)
			return _Parent->getNodeName(i);
		return NULL;
	}

	/**
	 * Browse the tree, building the text id, and for each leaf encountered, call the callback
	 * passing the argument provided, the index and the text id.
	 */
	void			foreachLeafCall( void (*callback)(void*,TCDBDataIndex,CTextId*), CTextId& id, void *arg )
	{
		callback( arg, _DataIndex, &id );
	}

	/*
	 * Browse the tree, and for each leaf encountered, call the callback
	 * passing the argument provided and the leaf, and post-incrementing the counter
	 */
	void			foreachLeafCall( void (*callback)(void*,CCDBStructNodeLeaf*,uint&), uint& counter, void *arg )
	{
		callback( arg, this, counter );
		++counter;
	}

	/**
	 * Return the first leaf found, and set the number of indirections in siblingLevel
	 */
	const CCDBStructNodeLeaf *findFirstLeaf( uint& siblingLevel ) const
	{
		return this;
	}

	/**
	 * Return the data index corresponding to a text id (from the root)
	 */
	TCDBDataIndex	findDataIndex( ICDBStructNode::CTextId& id ) const
	{
#ifdef NL_DEBUG
		nlassert( id.getCurrentIndex() == id.size() ); // assert that there are no lines left in the textid
#endif
		return _DataIndex;
	}
	
	/**
	 * Set the data index into the leaves. The passed index is incremented for each leaf leaf or atomic branch.
	 * The "returned" value of index is the number of indices set.
	 */
	void			initDataIndex( TCDBDataIndex& index )
	{
		_DataIndex = index;
		checkIfNotMaxIndex();
		++index;
	}

	/**
	 * For each different index (leaf of atomic branch), call the callback. Sets the id in leaves.
	 */
	void			initIdAndCallForEachIndex( CBinId& id, void (*callback)(ICDBStructNode*, void*), void *arg )
	{
		callback( this, arg );
		_LeafId = id;
	}

	/// Move the siblings that match the bank name to the destination tree
	void			moveBranchesToBank( CCDBStructNodeBranch *destRoot, TCDBBank bank );

	/// Return the binary leaf id
	const CBinId&	binLeafId() const { return _LeafId; }

	/// Build a textid corresponding to the leaf
	CTextId			buildTextId() const;

private:

	CCDBStructNodeBranch *_Parent;

	/// property type
	EPropType			_Type;

	/// Binary property id of the leaf (precalculated & stored to avoid having to get back in the tree to build it)
	CBinId				_LeafId;
};




#endif // CDB_LEAF_H






