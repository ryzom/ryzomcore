/** \file cdb.h
 *
 * $Id: cdb.h,v 1.2 2007/01/31 16:40:36 boucher Exp $
 */



#ifndef CDB_H
#define CDB_H

// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/string_mapper.h"

#include <libxml/parser.h>

namespace NLMISC
{
	class IProgressCallback;
	class CBitMemStream;
}

class CCDBNodeLeaf;
class CCDBNodeBranch;

///global bool, must be set to true if we want to display database modification. See displayDBModifs in commands.cpp
extern bool VerboseDatabase;




/**
 * Interface to manage a database node, can contain a unique property or a set of property
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */

class ICDBNode : public NLMISC::CRefCount
{
	//-----------------------------------------------------------------------
	// end of IDBNode interface
	// start of CDB sub-class definitions

public:
	enum EPropType
	{
		UNKNOWN = 0,
		// Unsigned
		I1,  I2,  I3,  I4,  I5,  I6,  I7,  I8,  I9,  I10, I11, I12, I13, I14, I15, I16,
		I17, I18, I19, I20, I21, I22, I23, I24, I25, I26, I27, I28, I29, I30, I31, I32,
		I33, I34, I35, I36, I37, I38, I39, I40, I41, I42, I43, I44, I45, I46, I47, I48,
		I49, I50, I51, I52, I53, I54, I55, I56, I57, I58, I59, I60, I61, I62, I63, I64,
		// Signed
		S1,  S2,  S3,  S4,  S5,  S6,  S7,  S8,  S9,  S10, S11, S12, S13, S14, S15, S16,
		S17, S18, S19, S20, S21, S22, S23, S24, S25, S26, S27, S28, S29, S30, S31, S32,
		S33, S34, S35, S36, S37, S38, S39, S40, S41, S42, S43, S44, S45, S46, S47, S48,
		S49, S50, S51, S52, S53, S54, S55, S56, S57, S58, S59, S60, S61, S62, S63, S64,
		TEXT, Nb_Prop_Type
	};


	/**
	 * observer interface to a database property
	 * \author Nicolas Brigand
	 * \author Nevrax France
	 * \date 2002
	 */
	class IPropertyObserver : public NLMISC::CRefCount
	{
	public :
		virtual void update(ICDBNode* node ) = 0;
	};	



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
		explicit CTextId( const std::string& str ): _Idx(0)
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
		uint size() const { return _Ids.size(); }

	private:
		std::vector<std::string> _Ids;
		mutable uint _Idx;
	};

//-----------------------------------------------------------------------
// end of CDB sub-class definitions


//-----------------------------------------------------------------------
// IDBNode interface definition

public :

	/**
	 *	destructor
	 */
	virtual ~ICDBNode() {}

	/**
	 *	Build the structure of the database from a file
	 * \param f is the stream
	 */
	virtual void init( xmlNodePtr node, NLMISC::IProgressCallback &progressCallBack, bool mapBanks=false ) = 0;

	/**
	 * Save a backup of the database
	 * \param id is the text id of the property/grp
	 * \param f is the stream
	 */
	//virtual void write( CTextId& id, FILE * f) = 0;

	/**
	 * Update the database from a stream coming from the FE
	 * \param gc the server gameCycle of this update. Any outdated update are aborted
	 * \param f : the stream.
	 */
	virtual void readDelta( NLMISC::TGameCycle gc, NLMISC::CBitMemStream & f ) = 0;

	/**
	 * Get a node
	 * \param ids is the list of property index
	 * \param idx is the property index of the current node(updated by the method)
	 */
	virtual ICDBNode * getNode( std::vector<uint16>& ids, uint idx ) = 0;


	/**
	 * Get a node . Create it if it does not exist yet
	 * \param id : the CTextId identifying the node
	 */
	virtual ICDBNode * getNode( const CTextId& id, bool bCreate = true )=0 ;

	/**
	 * Get a node
	 * \param idx is the node index
	 */
	virtual ICDBNode * getNode( uint16 idx ) = 0;

	/**
	 * Get a node index
	 * \param node is a pointer to the node
	 * \param index is a reference that receive the result
	 * \return true if the node was found
	 */
	virtual bool getNodeIndex( ICDBNode* node , uint & index) = 0;

	/**
	 * Return the value of a property (the update flag is set to false)
	 * \param id is the text id of the property/grp
	 * \param name is the name of the property
	 * \return the value of the property
	 */
	virtual sint64 getProp( CTextId& id ) = 0; 
	
	/**
	 * Set the value of a property (the update flag is set to true)
	 * \param id is the text id of the property/grp
	 * \param name is the name of the property
	 * \param value is the value of the property
	 * \return bool : 'true' if property found.
	 */
	virtual bool setProp( CTextId& id, sint64 value ) = 0; 
	
	/// Reset all leaf data from this point
	virtual void resetData(NLMISC::TGameCycle gc) = 0;

	/**
	 * Clear the node and his children
	 */
	virtual void clear() = 0; 
	
	/**
	* add an observer to a property
	* \param observer : pointer to an observer 
	* \param id text id identifying the property	
	* \return false if the node doesn't exist
	*/
	//virtual bool addObserver(IPropertyObserver* observer, CTextId& id) = 0;

	/** remove an obsever
	 * \param observer : pointer to an observer 
	 * \param id text id identifying the property	
	 * \return false if the node or observer doesn t exist 
	 */
	//virtual bool removeObserver(IPropertyObserver* observer, CTextId& id) = 0;
	
	/**
	 * Inform a node of its parenthood
	 */
	virtual void setParent(CCDBNodeBranch *parent) { nlassertex(0,("setParent() not overloaded for given node type!")); }

	/**
	 * get the parent of a node
	 */
	virtual CCDBNodeBranch* getParent() { nlassertex(0,("getParent() not overloaded for given node type!")); return NULL; }

	/**
	 * get the name of this node
	 */
	const std::string * getName() const { return &_DBSM->localUnmap(_Name); }

	/**
	 * get the full name of this node separator is ':' (ie UI:INTERFACE:REDSTUFF)
	 * This will not return the fullname with the ROOT !
	 */
	std::string getFullName();

	/// Count the leaves
	virtual uint			countLeaves() const = 0;

	/// Find the leaf which count is specified (if found, the returned value is non-null and count is 0)
	virtual CCDBNodeLeaf	*findLeafAtCount( uint& count ) = 0;

	/// Set the atomic branch flag (when all the modified nodes of a branch should be tranmitted at the same time)
	void					setAtomic( bool atomicBranch ) { _Atomic = atomicBranch; }

	/// Return true if the branch has the atomic flag
	bool					isAtomic() const { return _Atomic; }

	// test if the node is a leaf
	virtual bool			isLeaf() const = 0;

	/// Debug purpose
	virtual void display (const std::string &prefix){}

protected:

	/// Constructor
	ICDBNode() : _Atomic(false)
	{
		if (_DBSM == NULL) _DBSM = NLMISC::CStringMapper::createLocalMapper();
		_Name = NLMISC::CStringMapper::emptyId();
	}

	/// Constructor
	ICDBNode (const std::string &name) : _Atomic(false)
	{
		if (_DBSM == NULL) _DBSM = NLMISC::CStringMapper::createLocalMapper();
		_Name = _DBSM->localMap(name);
		//_NameDbg = name;
	}

	/// Atomic flag: is the branch an atomic group, or is the leaf a member of an atomic group
	bool			_Atomic		: 1;

	/// Name of the node
	NLMISC::TStringId	_Name;
	//std::string _NameDbg;
	
	static NLMISC::CStringMapper *_DBSM;
	
};



#endif // CDB_H
