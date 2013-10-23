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

#ifndef SAVES_UNIT_H
#define SAVES_UNIT_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"

// game share
#include "game_share/file_description_container.h"

// local
#include "rs_module_messages.h"


//-----------------------------------------------------------------------------
// SAVES namespace
//-----------------------------------------------------------------------------

namespace SAVES
{
	//-----------------------------------------------------------------------------
	// advanced class declarations
	//-----------------------------------------------------------------------------

	class CSavesUnit;
	class ISavesUnitElement;

	typedef NLMISC::CSmartPtr<CSavesUnit> TSavesUnitPtr;
	typedef NLMISC::CSmartPtr<ISavesUnitElement> TSavesUnitElementPtr;


	//-----------------------------------------------------------------------------
	// class ISavesCallbackHandler
	//-----------------------------------------------------------------------------

	class ISavesCallbackHandler
	{
	public:
		virtual void addNew(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size)=0;
		virtual void addChange(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size)=0;
		virtual void addDeleted(const NLMISC::CSString& fileName)=0;
		virtual void addElement(ISavesUnitElement* newChild)=0;
	};

	//-----------------------------------------------------------------------------
	// class CSavesUnit
	//-----------------------------------------------------------------------------

	class CSavesUnit: public ISavesCallbackHandler
	{
	public:
		//-----------------------------------------------------------------------------
		// basic public interface

		// ctor
		CSavesUnit();

		// type enumeration used by inti to select default elements to add to unit
		enum TType { SHARD, WWW, BAK };

		// initialise the saves unit (resets all properties, clears memory of directories scanned, etc)
		// must be called before any of the other methods of this class
		void init(const NLMISC::CSString& directoryName,TType type);

		// update routine performs the next atomic scan operation in the set of elements provided
		void update();

		// flag that is false until sufficient calls to update() have been made to complete an entire scan cycle
		bool ready() const;

		// get the complete file list for this unit (note: often >100k files in this list)
		void getFileList(CFileDescriptionContainer &result) const;

		// add an element to the module (an element is typically a directory that may contain files or other directories)
		void addElement(ISavesUnitElement* newChild);

		// get the current change description message and provoke flush of change set in the saves unit
		TMsgRSUpdatePtr popNextChangeSet();


		//-----------------------------------------------------------------------------
		// Interface for element objects to use during their scan to signal file
		// additions, changes or deletions

		void addNew(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size);
		void addChange(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size);
		void addDeleted(const NLMISC::CSString& fileName);


	private:
		//-----------------------------------------------------------------------------
		// private data

		// a couple of booleans setup during initialisation
		bool _IsInitialised;
		bool _IsFirstScan;

		// the root directory of this unit
		NLMISC::CSString _Path;

		// a vector of element objects that are updated cyclicly
		typedef std::vector<TSavesUnitElementPtr> TChildren;
		TChildren _Children;

		// the iterator used to cycle from one child element to the next
		uint32 _IterationIndex;

		// a message object used to receive the changes detected by the updates
		TMsgRSUpdatePtr _ChangeMsg;
	};


	//-----------------------------------------------------------------------------
	// class ISavesUnitElement
	//-----------------------------------------------------------------------------

	class ISavesUnitElement: public NLMISC::CRefCount
	{
	public:
		// ctor
		ISavesUnitElement();

		// dtor
		virtual ~ISavesUnitElement() {}

		// perform the 'rescan' operation for this element - verify whether anything has changed
		// the parent parameter is used to supply addNew(), addChange() and addDeleted() methods
		// to signal any detected changes
		virtual bool update(ISavesCallbackHandler* parent)=0;

		// add all files refferenced by the element to a file description container
		// the supplied 'path' is assumed to be the common file name base for all of the supplied files
		// this common path is stripped from the entries that are stored in the fdc
		virtual void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const=0;

		// flag set true when an element is attached to a parent object (via CSavesUnit::addElement())
		// false by default and false when the parent object stops using the element
		bool isActive() const;

	private:
		// write accessor for the 'isActive' flag (reserved for use by the CSavesUnit class)
		void setActive(bool value);
		friend class CSavesUnit;

		// private data (this is a counter rather than a boolean because there is no guarantee
		// that some future specialisation of ISavesUnitElement won't be held by more than one unit)
		uint32 _ActivationCounter;
	};
}


//-----------------------------------------------------------------------------
#endif
