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



#ifndef CDB_H
#define CDB_H

#include "cdb_struct_banks.h"

namespace NLMISC
{
	class IProgressCallback;
	class CBitMemStream;
}


class CCDBStructNodeLeaf;
class CCDBStructNodeBranch;
struct _xmlNode;
typedef struct _xmlNode xmlNode;
class CCDBSynchronised;


/**
 * Interface to manage a database node, can contain a unique property or a set of property
 * \author Stephane Coutelas, Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class ICDBStructNode : public NLMISC::CRefCount
{
	//-----------------------------------------------------------------------
	// end of IDBNode interface
	// start of CDB sub-class definitions

public:
	enum EPropType
	{
		UNKNOWN=0,
		I1,  I2,  I3,  I4,  I5,  I6,  I7,  I8,  I9, I10, I11, I12, I13, I14, I15, I16,
		I17, I18, I19, I20, I21, I22, I23, I24, I25, I26, I27, I28, I29, I30, I31, I32,
		I33, I34, I35, I36, I37, I38, I39, I40, I41, I42, I43, I44, I45, I46, I47, I48,
		I49, I50, I51, I52, I53, I54, I55, I56, I57, I58, I59, I60, I61, I62, I63, I64,
		TEXT, Nb_Prop_Type
	};


	/// Prototype for node change callback
	typedef void (*TNodeChangeCallback)(CCDBSynchronised *syncDb, ICDBStructNode *node);

	/**
	 * Text id
	 * \author Stephane Coutelas
	 * \author Nevrax France
	 * \date 2002
	 */
	class CTextId
	{
	public:
		/**
		 *	Default constructor
		 */
		CTextId(): _Idx(0) {}

		/**
		 * Init this text id from a string
		 */
		CTextId( const std::string& str): _Idx(0)
		{
			std::string s=str;
			uint32 i, j;
			for (i=0,j=0; i+j<s.size(); j++)
				if (s[i+j]==':')
				{
					_Ids.push_back(s.substr(i,j));
					i+=j+1;	// +1 to skip the ':'
					j=0;
				}
			// deal with the last id in the string (terminated by a '\x0' and not a ':')
			_Ids.push_back(s.substr(i,j));
		}

		/**
		 *	Build a string from this text id
		 */
		std::string toString() const
		{
			if (_Ids.size()==0) return std::string("");
			std::string str=_Ids[0];
			for (unsigned i=1; i<_Ids.size(); i++)
				str +=std::string(":")+ _Ids[i];
			return str;
		}

		/**
		 * Push back a sub name id to this id
		 */
		void push( std::string str ) {	_Ids.push_back( str ); }

		/**
		 * Remove the last sub name id to this id
		 */
		void pop() { _Ids.pop_back(); }
		
		/**
		 * Return the next sub id
		 */
		const std::string &readNext() const
		{
			nlassert( _Idx < _Ids.size() );
			return _Ids[_Idx++];
		}

		/** return true if a call to readNext can be performed
		  */
		bool hasElements() const { return _Idx < _Ids.size(); }

		/**
		 * Get the current index in Id
		 */
		uint getCurrentIndex() const { return _Idx; }

		/**
		 *	Return the count of strings composing this id
		 */
		uint size() const { return (uint)_Ids.size(); }

	private:
		std::vector<std::string> _Ids;
		mutable uint _Idx;
	};


	/**
	 * Binary id
	 * \author Stephane Coutelas, Olivier Cado
	 * \author Nevrax France
	 * \date 2002
	 */
	class CBinId
	{
	public :
		void push( uint32 idx, sint bits )	{ Ids.push_back( std::make_pair(idx, bits) ); }
		void pop()							{ Ids.pop_back(); }
		std::string toString() const;

		/// Write the id to a bit stream for server-client comms, and return the size of data written
		sint writeToBitMemStream( NLMISC::CBitMemStream& f ) const;

		std::vector< std::pair <uint32, sint> > Ids; // first int is the idx the second is the number of bits
	};

//-----------------------------------------------------------------------
// end of CDB sub-class definitions


//-----------------------------------------------------------------------
// IDBNode interface definition

public :

	/**
	 *	Build the structure of the database from a file
	 * \param f is the stream
	 */
	virtual void init( xmlNode *node, NLMISC::IProgressCallback &progressCallBack ) = 0;

	/** 
	 *	Get the type of the node. For non leaf, this is always UNKNOWN, else
	 *	it's the data type of the leaf.
	 */
	virtual EPropType		getType() const		{return UNKNOWN;}

	/**
	 * Get a node
	 * \param ids is the list of property index
	 * \param idx is the property index of the current node(updated by the method)
	 */
	virtual ICDBStructNode * getNode( std::vector<uint16>& ids, uint idx ) = 0;


	/**
	 * Get a node . Create it if it does not exist yet
	 * \param id : the CTextId identifying the node
	 */
	virtual ICDBStructNode * getNode( const CTextId& id, bool bCreate=true )=0 ;

	/**
	 * Get a node
	 * \param idx is the node index
	 */
	virtual ICDBStructNode * getNode( uint16 idx ) = 0;

	/**
	 * Get a node index
	 * \param node is a pointer to the node
	 * \param index is a reference that receive the result
	 * \return true if the node was found
	 */
	virtual bool getNodeIndex( ICDBStructNode* node , uint & index) const = 0;

	/**
	 * Inform a node of its parenthood
	 */
	virtual void setParent(CCDBStructNodeBranch * /* parent */) { nlassertex(0,("setParent() not overloaded for given node type!")); }

	/**
	 * get the parent of a node
	 */
	virtual CCDBStructNodeBranch* getParent() { nlassertex(0,("getParent() not overloaded for given node type!")); return NULL; }

	/**
	 * get the name of a node
	 */
	virtual const std::string * getName() { nlassertex(0,("getName() not overloaded for given node type!")); return NULL; }

	/**
	 * Set the data index into the leaves. The passed index is incremented for each leaf leaf or atomic branch.
	 * The "returned" value of index is the number of indices set.
	 */
	virtual void			initDataIndex( TCDBDataIndex& index ) = 0;

	/**
	 * For each different index (leaf of atomic branch), call the callback. Sets the id in leaves.
	 */
	virtual void			initIdAndCallForEachIndex( CBinId& id, void (*callback)(ICDBStructNode*, void*), void *arg ) = 0;

	/**
	 * Return the data index corresponding to a text id (from the root)
	 */
	virtual TCDBDataIndex	findDataIndex( ICDBStructNode::CTextId& id ) const = 0;

	/*
	 * Browse the tree, and for each atom branch encountered, call the callback passing the argument
	 * provided and the index of the atom branch.
	 */
	virtual void			foreachAtomBranchCall( void (*callback)(void*,TCDBDataIndex), void *arg ) const {}

	/**
	 * Browse the tree, building the text id, and for each leaf encountered, call the callback
	 * passing the argument provided, the index and the text id.
	 */
	virtual void			foreachLeafCall( void (*callback)(void*,TCDBDataIndex,CTextId*), CTextId& id, void *arg ) = 0;

	/**
	 * Browse the tree, and for each leaf encountered, call the callback
	 * passing the argument provided and the leaf, and post-incrementing the counter
	 */
	virtual void			foreachLeafCall( void (*callback)(void*,CCDBStructNodeLeaf*,uint&), uint& counter, void *arg ) = 0;

	/**
	 * Return the first leaf found, and set the number of indirections in siblingLevel
	 */
	virtual const CCDBStructNodeLeaf *findFirstLeaf( uint& siblingLevel ) const = 0;

// Common data

	/// Destructor
	virtual ~ICDBStructNode() { if ( _BankLabels ) delete _BankLabels; _BankLabels = NULL; }

	/// Label the tree branches with the matching bank name
	void			labelBranch( TCDBBank bank );

	/// Label the subnodes
	virtual void	labelSiblings( TCDBBank bank ) {}

	/// Move the siblings that match the bank name to the destination tree
	virtual void	moveBranchesToBank( CCDBStructNodeBranch *destRoot, TCDBBank bank ) = 0;

	/// Set a label while loading from XML (precondition: branch not labelled yet)
	void			setLabel( const std::string& bankName );

	/// Return the data index stored in the node
	TCDBDataIndex	getDataIndex() const { return _DataIndex; }

	/// Set the atomic branch flag (when all the modified nodes of a branch should be tranmitted at the same time)
	void			setAtomic( bool atomicBranch ) { _AtomicFlag = atomicBranch; }

	/// Return true if the branch has the atomic flag
	bool			isAtomic() const { return _AtomicFlag; }

	/** Attach callback. Allow to attach a callback function that some code could call
	 *	when an instance of this node is changed.
	 */
	void attachCallback(TNodeChangeCallback callback)	{	_ChangeCallback = callback; }

	/// Get the attached change callback
	TNodeChangeCallback getAttachedCallback()			{ return _ChangeCallback; }

protected:

	/// Constructor
	ICDBStructNode() 
		: _BankLabels(NULL), 
		  _DataIndex(CDB_INVALID_DATA_INDEX), 
		  _AtomicFlag(false),
		  _ChangeCallback(NULL)
	{}

	/// Check if max index not reached at init
	void			checkIfNotMaxIndex() const
	{
		if ( _DataIndex == CDB_MAX_DATA_INDEX )
			nlerror( "Too many database nodes in bank: %u reached", CDB_MAX_DATA_INDEX );
	}

	/// Indicates in which banks the branch is used when building the tree
	std::set<sint>			*_BankLabels;

	/// Corresponding index in data container (for leaves and atom group branches)
	TCDBDataIndex			_DataIndex;

	/// Atomic flag: is the branch an atomic group, or is the leaf a member of an atomic group
	bool					_AtomicFlag;

	/// Optional callback pointer.
	TNodeChangeCallback		_ChangeCallback;

};



#endif // CDB_H
