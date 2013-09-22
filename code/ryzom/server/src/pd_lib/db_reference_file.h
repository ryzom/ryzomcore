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

#ifndef NL_DB_REFERENCE_FILE_H
#define NL_DB_REFERENCE_FILE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/stream.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

#include <map>

#include "pd_server_utils.h"


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CDBReferenceFile : public CMixedStreamFile
{
public:

	/// Constructor
	CDBReferenceFile();

	/// Destructor
	~CDBReferenceFile();

	/// Setup file name and path
	void		setup(const std::string& name, const std::string& path, uint32 baseIndex, uint32 overIndex, uint32 rowSize);

	/// Clear initial setup
	void		clear();

	/// Is Initialised?
	bool		initialised() const						{ return _Init; }

	/// close file
	void		close();


	/**
	 * Builds an empty file
	 */
	bool		buildEmptyRef();





	/**
	 * Prewrite reference file.
	 * At least, read file header to known the base and index index in file
	 */
	bool		prewrite(bool failIfNotExist = false);

	/**
	 * Update Start/End Delta Ids
	 */
	bool		updateDeltaIds(uint32 startId, uint32 endId);

	/**
	 * Get Start/End Delta Ids
	 */
	void		getUpdateDeltaIds(uint32& startId, uint32& endId);

	/**
	 * Update a row in the reference file
	 * \param index is the absolute row index to update, not relative to file base index
	 * \param data is the data buffer to store in file
	 */
	bool		update(uint32 index, const uint8* rowdata);

	/**
	 * Postwrite reference file
	 * Mark file as valid, close file, flush anything still alive...
	 */
	bool		postwrite();






	/**
	 * Preload reference file.
	 * At least, read file header to known the base and index index in file
	 */
	bool		preload();

	/**
	 * Read a row in the reference file
	 * \param index is the absolute row index to read, not relative to file base index
	 * \param data is the data buffer to store data read from file
	 */
	bool		read(uint32 index, uint8* rowdata);







	/**
	 * Get standard row header size
	 */
	static uint32	getRowHeaderSize()					{ return sizeof(uint32); }

	/**
	 * Get reference file name
	 */
	static std::string	getRefFileName(uint32 tableId, uint32 refFile)	{ return NLMISC::toString("%04X_%04X.%s", tableId, refFile, getRefFileExt().c_str()); }

	/**
	 * Get reference file name
	 */
	static bool		isRefFile(const std::string& filename, uint32& tableId, uint32& refFile)
	{
		return sscanf(NLMISC::CFile::getFilename(filename).c_str(), "%X_%X.dbref", &tableId, &refFile) == 2;
	}

	/**
	 * Get reference file name extension
	 */
	static std::string	getRefFileExt()					{ return "dbref"; }

	/// Get Base Index
	uint32			getBaseIndex() const				{ return _Header.BaseIndex; }

	/// Get Base Index
	uint32			getEndIndex() const					{ return _Header.EndIndex; }

	/// Get Timestamp
	uint32			getTimestamp() const				{ return _Header.Timestamp; }

	/// Set Delta Ids
	void			setDeltaIds(uint32 startId, uint32 endId)	{ _Header.StartDeltaId = startId; _Header.EndDeltaId = endId; }

	/// Get Delta Ids
	void			getDeltaIds(uint32& startId, uint32& endId)	{ startId = _Header.StartDeltaId; endId = _Header.EndDeltaId; }

private:

	class CRefHeader
	{
	public:

		CRefHeader()
		{
			BaseIndex = 0;
			RowSize = 0;
			FullRowSize = 0;
			Timestamp = 0;

			StartDeltaId = 0;
			EndDeltaId = 0;
		}

		/// Base Index
		uint32				BaseIndex;

		/// End Index, last index in file plus 1
		uint32				EndIndex;

		/// Over Index, last index file can contain plus 1, base index in next reference file
		uint32				OverIndex;

		/// Row size, only declared size
		uint32				RowSize;

		/// Row Size, in byte (all headers included)
		uint32				FullRowSize;

		/// File timestamp
		uint32				Timestamp;

		/// Start Delta Id, used to check delta concatenation
		uint32				StartDeltaId;

		/// End Delta Id, used to check delta concatenation
		uint32				EndDeltaId;

		void				serial(NLMISC::IStream& s)
		{
			s.serialCheck(NELID("RHdr');
			uint	version = s.serialVersion(0);

			s.serial(BaseIndex);
			s.serial(EndIndex);
			s.serial(OverIndex);
			s.serial(RowSize);
			s.serial(FullRowSize);
			s.serial(Timestamp);

			s.serial(StartDeltaId);
			s.serial(EndDeltaId);
		}
	};

	/// Initialised
	bool				_Init;

	/// File base name
	std::string			_Name;

	/// File path
	std::string			_Path;

	/// Header of the reference
	CRefHeader			_Header;

	enum TMode
	{
		Update,
		Read
	};

	/// File Mode
	TMode				_Mode;

	/// Data start position
	uint32				_DataStart;

	/// Serial file header
	bool				serialHeader();

	uint32				getSeekPos(uint32 index)			{ return _DataStart + _Header.FullRowSize*(index-_Header.BaseIndex); }

};



#endif // NL_DB_REFERENCE_FILE_H

/* End of db_reference_file.h */
