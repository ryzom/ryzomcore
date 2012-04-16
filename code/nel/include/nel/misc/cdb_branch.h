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

#include "cdb.h"

namespace NLMISC{

enum{
	CDB_BANKS_MAX = 3,
	CDB_BANK_INVALID
};

/**
 * Database Node which contains a set of properties
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CCDBNodeBranch : public ICDBNode
{
public:
	/// Triggered when the branch observers are updated
	class IBranchObserverCallFlushObserver : public CRefCount{
	public:
		virtual ~IBranchObserverCallFlushObserver(){}
		virtual void onObserverCallFlush() = 0;
	};

	// default constructor
	CCDBNodeBranch(const std::string &name) : ICDBNode(name)
	{
		_Parent = NULL;
		_IdBits = 0;
		_Sorted = false;
	}

	/**
	 *	Build the structure of the database from a file
	 * \param f is the stream
	 */
	void init( xmlNodePtr node, class IProgressCallback &progressCallBack, bool mapBanks=false );

	/**
	 * Add a new sub node
	 * \param node is the new subnode
	 * \param nodeName is the name of the node
	 */
	void attachChild( ICDBNode * node, std::string nodeName );

	/**
	 * Get a node . Create it if it does not exist yet
	 * \param id : the CTextId identifying the node
	 */
	ICDBNode * getNode (const CTextId& id, bool bCreate=true);

	/**
	 * Get a node. Return NULL if out of bounds (no warning)
	 * \param idx is the node index
	 */
	ICDBNode * getNode( uint16 idx );

	/**
	 * Get a node index
	 * \param node is a pointer to the node
	 */
	virtual bool getNodeIndex( ICDBNode* node , uint& index)
	{
		index=0;
		for ( std::vector<ICDBNode*>::const_iterator it = _Nodes.begin(); it != _Nodes.end(); it++)
		{
			if (*it == node)
				return true;
			index++;
		}
		return false;
	}

	// return the child with the given node id, creating it if requested
	CCDBNodeLeaf *getLeaf( const char *id, bool bCreate );
	CCDBNodeLeaf *getLeaf( const std::string &id, bool bCreate ) { return getLeaf(id.c_str(), bCreate); }

	/**
	 * Save a backup of the database
	 * \param id is the text id of the property/grp
	 * \param f is the stream
	 */
	void write( CTextId& id, FILE * f);

	/// Update the database from the delta, but map the first level with the bank mapping (see _CDBBankToUnifiedIndexMapping)
	void readAndMapDelta( TGameCycle gc, CBitMemStream& s, uint bank );

	/// Update the database from a stream coming from the FE
	void readDelta( TGameCycle gc, CBitMemStream & f );

	/**
	 * Return the value of a property (the update flag is set to false)
	 * \param id is the text id of the property/grp
	 * \param name is the name of the property
	 * \return the value of the property
	 */
	sint64 getProp( CTextId& id );

	/**
	 * Set the value of a property (the update flag is set to true)
	 * \param id is the text id of the property/grp
	 * \param name is the name of the property
	 * \param value is the value of the property
	 * \return bool : 'true' if property found.
	 */
	bool setProp( CTextId& id, sint64 value );

	/// Clear the node and his children
	void clear();

	/// Reset the data corresponding to the bank (works only on top level node)
	void resetBank( TGameCycle gc, uint bank)
	{
		//nlassert( getParent() == NULL );
		for ( uint i=0; i!=_Nodes.size(); ++i )
		{
			if ( _UnifiedIndexToBank[i] == bank )
				_Nodes[i]->resetData(gc);
		}
	}

	/// Reset all leaf data from this point
	void resetData(TGameCycle gc, bool forceReset=false)
	{
		for ( uint i=0; i!=_Nodes.size(); ++i )
		{
			_Nodes[i]->resetData(gc, forceReset);
		}
	}

	/**
	 *	Destructor
	 */
	virtual ~CCDBNodeBranch() { clear(); }

	// the parent node for a branch (NULL by default)
	virtual void setParent(CCDBNodeBranch *parent) { _Parent=parent; }

	virtual CCDBNodeBranch*  getParent()
	{
		return _Parent;
	}

	//get the number of nodes
	uint16 getNbNodes()
	{
		return (uint16)_Nodes.size();
	}

	/// Count the leaves
	virtual uint	countLeaves() const;

	/// Find the leaf which count is specified (if found, the returned value is non-null and count is 0)
	virtual CCDBNodeLeaf	*findLeafAtCount( uint& count );

	virtual void display (const std::string &prefix);

	void removeNode (const CTextId& id);

	/**
	* add an observer to a property
	* \param observer : pointer to an observer
	* \param id text id identifying the property
	* \return false if the node doen t exist
	*/
	virtual bool addObserver(IPropertyObserver* observer, CTextId& id);

	/** remove an obsever
	 * \param observer : pointer to an observer
	 * \return false if the node or observer doesn t exist
	 */
	virtual bool removeObserver(IPropertyObserver* observer, CTextId& id);

	// Add an observer to this branch. It will be notified of any change in the sub-leaves

	/**
	 * Add observer to all sub-leaves, except if a positive filter is set:
	 * If positiveLeafNameFilter is non-empty, only changes to leaves having names found in it
	 * will be notified (this is equivalent to creating a sub-branch containing only the specified leaves
	 * and setting a branch observer on it, except you don't need to change your database paths
	 * and update large amounts of code!).
	 */
	void addBranchObserver(IPropertyObserver* observer, const std::vector<std::string>& positiveLeafNameFilter=std::vector<std::string>());

	/**
	 * Easy version of addBranchObserver() (see above).
	 * Examples of dbPathFromThisNode:
	 * "" -> this node
	 * "FOO:BAR" ->  sub-branch "BAR" of "FOO" which is a sub-branch of this node
	 */
	void addBranchObserver(const char *dbPathFromThisNode, ICDBNode::IPropertyObserver& observer, const char **positiveLeafNameFilter=NULL, uint positiveLeafNameFilterSize=0);

	// Remove observer from all sub-leaves
	bool removeBranchObserver(IPropertyObserver* observer);

	/// Easy version of removeBranchObserver() (see above and see easy version of addBranchObserver())
	void removeBranchObserver(const char *dbPathFromThisNode, ICDBNode::IPropertyObserver& observer);

	virtual bool isLeaf() const { return false; }

	/** Update all observers of branchs that have been modified
	  */
	static void flushObserversCalls();

private:
	static void triggerFlushObservers();

public:
	static void addFlushObserver( IBranchObserverCallFlushObserver *observer );
	static void removeFlushObserver( IBranchObserverCallFlushObserver *observer );

	// mark this branch and parent branch as 'modified'. This is usually called by sub-leaves
	void linkInModifiedNodeList(TStringId modifiedLeafName);

	/// Find a subnode at this level
	ICDBNode * find (const std::string &nodeName);

	/// Main init
	static void resetNodeBankMapping() { _UnifiedIndexToBank.clear(); }

	// reset all static mappings
	static void reset();

	/// Internal use only
	static void mapNodeByBank( ICDBNode *node, const std::string& bankStr, bool clientOnly, uint nodeIndex );

protected:


		/** Struct identifying an observer of a db branch
	  * This struct can be linked in a list so that we can update observers only once per pass.
	  * An observer that watch a whole branch can be updated once and only once after each element of the branch has been modified
	  */
	class CDBBranchObsInfo
	{
		public:
			CRefPtr<IPropertyObserver> Observer;
			// 2 linked list are required : while the observer is notified, it can triger one other observer, so we must link it in another list
			bool               Touched[2];
			CDBBranchObsInfo   *PrevNotifiedObserver[2]; // NULL means this is the head
			CDBBranchObsInfo   *NextNotifiedObserver[2];
			ICDBNode		   *Owner;

			// If non-empty, only a leaf whose name is found here will notify something
			// This is equivalent to creating a sub-branch containing only the specified leaves
			// and setting a branch observer on it, except you don't need to change your database paths
			// and update large amounts of code and script!
			std::vector<TStringId> PositiveLeafNameFilter;

		public:

			/// Constructor. See above for usage of positiveLeafNameFilter.
			CDBBranchObsInfo(IPropertyObserver *obs = NULL, ICDBNode *owner = NULL, const std::vector<std::string>& positiveLeafNameFilter=std::vector<std::string>())
			{
				Owner = owner;
				Observer = obs;
				Touched[0] = Touched[1] = false;
				PrevNotifiedObserver[0] = PrevNotifiedObserver[1] = NULL;
				NextNotifiedObserver[0] = NextNotifiedObserver[1]  = NULL;
				for (std::vector<std::string>::const_iterator ipf=positiveLeafNameFilter.begin(); ipf!=positiveLeafNameFilter.end(); ++ipf)
				{
					PositiveLeafNameFilter.push_back(ICDBNode::getStringId(*ipf)); // the ids are also mapped at database init, we don't need to unmap them in destructor
				}
			}
			~CDBBranchObsInfo()
			{
				// should have been unlinked
				nlassert(Touched[0] == false);
				nlassert(Touched[1] == false);
				nlassert(PrevNotifiedObserver[0] == NULL);
				nlassert(PrevNotifiedObserver[1] == NULL);
				nlassert(NextNotifiedObserver[0] == NULL);
				nlassert(NextNotifiedObserver[1] == NULL);
			}
			// Unlink from the given list. This also clear the '_Touched' flag
			void unlink(uint list);			
			void link(uint list, TStringId modifiedLeafName);
	};

	typedef std::list<CDBBranchObsInfo> TObsList; // must use a list because pointers on CDBObserverInfo instances must remains valids

protected:


	CCDBNodeBranch			*_Parent;

	/// database subnodes not sorted
	std::vector<ICDBNode*>	_Nodes;

	/// subnodes sorted by name
	std::vector<ICDBNode*>	_NodesByName;

	// number of bits required to stock my children's ids
	uint8					_IdBits : 7;
	bool					_Sorted : 1;

	// observers for this node or branch
	TObsList				_Observers;

	friend class CDBBranchObsInfo;
	// Global list of modified nodes
	static CDBBranchObsInfo *_FirstNotifiedObs[2];
	static CDBBranchObsInfo *_LastNotifiedObs[2];
	static uint			   _CurrNotifiedObsList; // 0 or 1 => tell in which list observers of modified values must be added
	// current & next observers being notified : if such observer if removed during notification, pointer will be valids
	static CDBBranchObsInfo *_CurrNotifiedObs;
	static CDBBranchObsInfo *_NextNotifiedObs;

	/// Mapping from server database index to client database index (first-level nodes)
	static std::vector<uint> _CDBBankToUnifiedIndexMapping [CDB_BANKS_MAX];

	// Mapping from client database index to TCDBBank (first-level nodes)
	static std::vector<uint> _UnifiedIndexToBank;

	/// Last index mapped
	static uint				_CDBLastUnifiedIndex;

	/// Number of bits for first-level branches, by bank
	static uint				_FirstLevelIdBitsByBank [CDB_BANKS_MAX];

	/// called by clear
	void			removeAllBranchObserver();
	void			removeBranchInfoIt(TObsList::iterator it);


private:
	static std::vector< IBranchObserverCallFlushObserver* > flushObservers;

};

}

#endif // CDB_BRANCH_H






