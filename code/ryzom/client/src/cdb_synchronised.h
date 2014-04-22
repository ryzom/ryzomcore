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


#include "nel/misc/cdb.h"
#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_manager.h"

/**
 * Class to manage a database of properties
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CCDBSynchronised : public NLMISC::CCDBManager
{
	/// string associations
	std::map<uint32,std::string> _Strings;

	/// True while the first database packet has not been completely processed (including branch observers)
	bool					_InitInProgress;

	/// The number of "init database packet" received
	uint8					_InitDeltaReceived;

public:

	/// exception thrown when database is not initialized
	struct EDBNotInit : public NLMISC::Exception
	{
		EDBNotInit() : Exception ("Property Database not initialized") {}
	};

	/**
	 * Default constructor
	 */
	CCDBSynchronised();

	/**
	 * Return a ptr on the node
	 * \return ptr on the node
	 */
	NLMISC::CCDBNodeBranch * getNodePtr() { return _Database; }

	/**
	 *	Build the structure of the database from a file
	 * \param fileName is the name of file containing the database structure
	 */
	void init( const std::string &fileName, class NLMISC::IProgressCallback &progressCallBack );

	/**
	 * Load a backup of the database
	 * \param fileName is the name of the backup file
	 */
	void read( const std::string &fileName );

	/**
	 * Save a backup of the database
	 * \param fileName is the name of the backup file
	 */
	void write( const std::string &fileName );

	/**
	 * Update the database from a stream coming from the FE
	 * \param f the stream
	 */
	void readDelta( NLMISC::TGameCycle gc, NLMISC::CBitMemStream& s, uint bank );

	/**
	 * Return the value of a property (the update flag is set to false)
	 * \param name is the name of the property
	 * \return the value of the property
	 */
	sint64 getProp( const std::string &name );

	/**
	 * Set the value of a property (the update flag is set to true)
	 * \param name is the name of the property
	 * \param value is the value of the property
	 * \return bool : 'true' if the property was found.
	 */
	bool setProp(const std::string &name, sint64 value);

	/**
	 * Return the string associated with id
	 * \param id is the string id
	 * \return the string
	 */
	std::string getString( uint32 id );

	/**
	 * Set a new string association
	 * \param id is the string id
	 * \param str is the new string
	 */
	void setString( uint32 id, const std::string &);

	/**
	 * Clear the database
	 */
	void clear();

	/**
	 * Destructor
	 */
	~CCDBSynchronised() { clear(); }

	/// Return true while the first database packet has not been completely received
	bool initInProgress() const { return _InitInProgress; }

	/// tests
	void test();

	/// Reset the init state (if you relauch the game from scratch)
	void resetInitState() { _InitDeltaReceived = 0; _InitInProgress = true; writeInitInProgressIntoUIDB(); }

	/// Called after flushObserversCalls() as it calls the observers for branches
	void setChangesProcessed()
	{
		if ( allInitPacketReceived() )
		{
			_InitInProgress = false;
			 writeInitInProgressIntoUIDB(); // replaced by DECLARE_INTERFACE_USER_FCT(isDBInitInProgress)
		}
	}

private:

	friend void impulseDatabaseInitPlayer( NLMISC::CBitMemStream &impulse );
	friend void impulseInitInventory( NLMISC::CBitMemStream &impulse );

	void setInitPacketReceived() { ++_InitDeltaReceived; }
	bool allInitPacketReceived() const { return _InitDeltaReceived == 2; } // Classic database + inventory

	void writeInitInProgressIntoUIDB();

	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> m_CDBInitInProgressDB;
};


#endif // CDB_SYNCHRONISED_H






