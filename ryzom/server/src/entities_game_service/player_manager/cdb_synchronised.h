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



#ifndef CDB_SYNCHRONISED_H
#define CDB_SYNCHRONISED_H


#include "cdb_data_instance_container.h"
#include "cdb_struct_banks.h"

class CCDBStructNodeBranch;
class CCDBStructNodeLeaf;

namespace NLMISC
{
	class CBitMemStream;
}

struct TPushAtomChangeStruct;


const uint CDBChangedPropertyCountBitSize = 16;


/**
 * Class to manage a database of properties
 * \author Stephane Coutelas, Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CCDBSynchronised
{
public:

	/// exception thrown when database is not initialized
	struct EDBNotInit : public NLMISC::Exception
	{
		EDBNotInit() : Exception("CDB: Property Database not initialized") {}
	};

	struct ECDBNotFound : public NLMISC::Exception
	{
		ECDBNotFound() : Exception("CDB: Property not found") {}
	};

	/*struct CPropForClientOnly
	{
		CPropForClientOnly( const std::string& propName, sint64 value )
			: PropName(propName), Value(value) {}

		std::string	PropName;
		sint64		Value;
	};*/

	/**
	 * Default constructor
	 */
	CCDBSynchronised();

	~CCDBSynchronised();

	/**
	 *	Init (the singleton of CCDBStructBanks must have been initialized before)
	 * \param bank Database bank
	 * \param usePermanentTracker True if you want to be able to get delta of all
	 * changes done since the beginning (using writePermanentDelta()), otherwise false.
	 */
	void init( TCDBBank bank, bool usePermanentTracker=false );

	/**
	 * Load a backup of the database
	 * \param fileName is the name of the backup file
	 */
	void read( const std::string& fileName );

	/**
	 * Save a backup of the database
	 * \param fileName is the name of the backup file
	 */
//	void write( const std::string& fileName ); 	
	
	/**
	 * Build the bitstream with new changes to send to the recipient
	 * \param s the stream
	 * \param maxBitSize the maximum number of bits that should be written by this call to writeDelta()
	 * (may be overtaken).
	 * \return True if something was written, false if there was nothing to write.
	 */
	bool writeDelta( NLMISC::CBitMemStream& s, uint32 maxBitSize );

	/**
	 * Build the bitstream with all changes since the beginning to send to the recipient.
	 * Precondition: the CCDBSynchronised object must have been init with usePermanentTracker=true.
	 * Note: at the moment, there is no size control, so the CCDBSynchronised object should
	 * not contain a huge number of leaves/modifications. Providing size control would require
	 * to have some context with something like an iterator.
	 * \param s the stream
	 * \return True if something was written, false if there was nothing to write.
	 */
	bool writePermanentDelta( NLMISC::CBitMemStream& s );

	// Return true if there are pending property changes pushed using setPropIntoClientonlyDB().
	//bool hasClientonlyPropertyChanges() const { return ! _PropsForClientOnly.empty(); }

	/**
	 * Fill the bitstream with the property changes that were pushed using setPropIntoClientonlyDB().
	 * Empty the list of pending property changes.
	 * Call only if hasClientonlyPropertyChanges() returned true.
	 */
	//void writeClientonlyPropertyChanges( NLMISC::CBitMemStream& s );

	// :KLUDGE: ICDBStructNode non-const 'coz getName and getParent are not
	// const methods. See implementation for more info.
	ICDBStructNode * getICDBStructNodeFromName(const std::string& name) const;
	
	/**
	 * Return the value of a property. If not found, throws ECDBNotFound().
	 * \param name is the name of the property
	 * \return the value of the property
	 */
	sint64 x_getProp( const std::string& name ) const;
	
	/**
	 * Return the value of a property
	 * Use getICDBStructNodeFromName() to store the node pointer.
	 * If not found, throws ECDBNotFound().
	 * Precondition: node not null.
	 */
	sint64 x_getProp( ICDBStructNode *node ) const;

	/**
	 * Return the value of a property. If not found, throws ECDBNotFound().
	 * \param name is the name of the property
	 * \return the value of the property
	 */
//	const ucstring &getPropString( const std::string& name ) const;
	
	/**
	 * Return the value of a property
	 * Use getICDBStructNodeFromName() to store the node pointer.
	 * If not found, throws ECDBNotFound().
	 * Precondition: node not null.
	 */
	ucstring x_getPropUcstring( ICDBStructNode *node ) const;
	const std::string &x_getPropString( ICDBStructNode *node ) const;

	/**
	 * Set the value of a property if it is not the same as the current one (the update flag is set to true)
	 * \param name is the name of the property
	 * \param value is the value of the property
	 * \param forceSending is a flag to force to send the update even if the value has not changed
	 * \return bool : 'true' if the property was found.
	 */
	bool x_setProp( const std::string& name, sint64 value, bool forceSending=false );

	/**
	 * Same as setProp(string,sint64,bool) but much faster version.
	 * Use getICDBStructNodeFromName() to store the node pointer.
	 */
	bool x_setProp( ICDBStructNode *node, sint64 value, bool forceSending=false );

	/**
	 * Same as setProp(ICDBStructNode*,sint64,bool) but one level below.
	 * If the child is not found, returns false.
	 * Use getICDBStructNodeFromName() to store the node pointer.
	 */
	bool x_setProp( ICDBStructNode *node, const char *childName, sint64 value, bool forceSending=false );

	/**
	 * Set the value of a property if it is not the same as the current one (the update flag is set to true)
	 * \param name is the name of the property
	 * \param value is the value of the property
	 * \param forceSending is a flag to force to send the update even if the value has not changed
	 * \return bool : 'true' if the property was found.
	 */
	bool x_setPropString( const std::string& name, const ucstring &value, bool forceSending=false );

	/**
	 * Same as setProp(string,sint64,bool) but much faster version.
	 * Use getICDBStructNodeFromName() to store the node pointer.
	 */
	bool x_setPropString( ICDBStructNode *node, const ucstring &value, bool forceSending=false );
	bool x_setPropString( ICDBStructNode *node, const std::string &value, bool forceSending=false );

	/**
	 * Same as setProp(ICDBStructNode*,sint64,bool) but one level below.
	 * If the child is not found, returns false.
	 * Use getICDBStructNodeFromName() to store the node pointer.
	 */
	bool x_setPropString( ICDBStructNode *node, const char *childName, const ucstring &value, bool forceSending=false );

	/**
	 * Same as setProp(ICDBStructNode*,const char*,sint64,bool) but increment the current value
	 * Precondition: the child MUST be a leaf
	 */
	bool x_incProp( ICDBStructNode *node, const char *childName );

	/**
	 * Set the value of a property (but the update flag is NOT changed)
	 * \param name is the name of the property
	 * \param value is the value of the property
	 * \return bool : 'true' if the property was found.
	 */
	bool x_setPropButDontSend( const std::string& name, sint64 value );

	/**
	 * Set the value of a property that is not in the server database tree.
	 * It will be sent by impulsion message to the client at the end of the current game cycle.
	 * Then it will be applied to the client database.
	 * Do not use too frequently (no bandwidth regulation).
	 */
	//bool setPropIntoClientonlyDB( const std::string& name, sint64 value );

	/**
	 * Return true if the specified property has been modified and will be sent to the client
	 */
	bool isModified(  const std::string& name ) const;

	/**
	 * Return the count of property which have been modified
	 * \param the changed property count
	 */
	uint getChangedPropertyCount() const { return _DataContainer.getChangedPropertyCount(); }

	/// Return true if writeDelta has not been called yet
	bool notSentYet() const { return _NotSentYet; }

	/// Force the notSentYet() flag to false
	void setAsSent() { _NotSentYet = false; }

	/// Return the bank
	TCDBBank	bank() const { return _Bank; }

	/// Number of database changes pushed to packets for the client
	uint32		NbDatabaseChanges;

	/**
	 * Return the string associated with id
	 * \param id is the string id
	 * \return the string
	 */
	//std::string getString( uint32 id );

	/**
	 * Set a new string association
	 * \param id is the string id
	 * \param str is the new string
	 */
	//void setString( uint32 id, std::string );

	/// tests
	void test();

protected:

	/// Push one change to the stream
	void	pushDelta( NLMISC::CBitMemStream& s, CCDBStructNodeLeaf *node, uint32& bitsize );

	/// Push one change to the stream (permanent mode)
	void	pushDeltaPermanent( NLMISC::CBitMemStream& s, CCDBStructNodeLeaf *node, uint32& bitsize );

	/// Push one change in an atom if the leaf needs it
	void	pushDeltaOfLeafInAtomIfChanged( TPushAtomChangeStruct *arg, CCDBStructNodeLeaf *node, uint indexInAtom );

	/// Push one change in an atom if the leaf needs it (permanent mode)
	void	pushDeltaOfLeafInAtomIfChangedPermanent( TPushAtomChangeStruct *arg, CCDBStructNodeLeaf *node, uint indexInAtom );

	friend void cbPushDeltaOfLeafInAtomIfChanged( void* arg, CCDBStructNodeLeaf *node, uint& indexInAtom );
	friend void cbPushDeltaOfLeafInAtomIfChangedPermanent( void* arg, CCDBStructNodeLeaf *node, uint& indexInAtom );

private:

	/// Data instance
	CCDBDataInstanceContainer	_DataContainer;

	/// Pointer on data structure
	CCDBStructNodeBranch		*_DataStructRoot;

	// Property changes to send to client for "client only" database
	//std::vector<CPropForClientOnly> _PropsForClientOnly;

	/// Bank
	TCDBBank					_Bank;

	/// Becomes false at the first time writeDelta() is called
	bool						_NotSentYet;
};


#endif // CDB_SYNCHRONISED_H






