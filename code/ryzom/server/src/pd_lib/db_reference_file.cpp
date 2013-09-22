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

#include "db_reference_file.h"
#include "pds_table_buffer.h"

#include <nel/misc/debug.h>

using namespace NLMISC;
using namespace std;

/*
 * Constructor
 */
CDBReferenceFile::CDBReferenceFile()
{
	clear();
}

/*
 * Destructor
 */
CDBReferenceFile::~CDBReferenceFile()
{
	clear();
}


/*
 * Clear initial setup
 */
void	CDBReferenceFile::clear()
{
	_Init = false;

	close();

	_Name.clear();
	_Path.clear();

	_Header.BaseIndex = 0;
	_Header.EndIndex = 0;
	_Header.OverIndex = 0;
	_Header.RowSize = 0;
	_Header.FullRowSize = 0;
	_Header.Timestamp = 0;

	_Mode = Read;

	_DataStart = 0;
}

/*
 * close file
 */
void	CDBReferenceFile::close()
{
	if (_File != NULL)
	{
		PDS_LOG_DEBUG(1)("CDBReferenceFile::clear(): closing file '%s%s' in %s mode", _Path.c_str(), _Name.c_str(), (_Mode == Read ? "Read" : "Update"));

		// in update mode, postwrite to validate file
		if (_Mode == Update)
		{
			postwrite();
		}
	}

	CMixedStreamFile::close();
}


/*
 * Setup file name and path
 */
void	CDBReferenceFile::setup(const string& name, const string& path, uint32 baseIndex, uint32 overIndex, uint32 rowSize)
{
	clear();

	uint32	tableId;
	uint32	refFile;
	nlassert(isRefFile(name, tableId, refFile));

	_Name = name;
	_Path = path;

	_Header.BaseIndex = baseIndex;
	_Header.EndIndex = baseIndex;
	_Header.OverIndex = overIndex;
	_Header.RowSize = rowSize;
	_Header.FullRowSize = rowSize + getRowHeaderSize();
	_Header.Timestamp = CTableBuffer::getCommonStamp(); //CStampHandler::getStamp();

	_Init = true;
}



/*
 * Builds an empty file
 */
bool	CDBReferenceFile::buildEmptyRef()
{
	string	filepath = _Path+_Name;

	if (_File != NULL)
	{
		nlwarning("CDBReferenceFile::buildEmptyRef(): failed, file '%s' already open", filepath.c_str());
		return false;
	}

	// check file doesn't exist yet
	if (CFile::fileExists(filepath))
	{
		nlwarning("CDBReferenceFile::buildEmptyRef(): failed, file '%s' already exists", filepath.c_str());
		return false;
	}

	if (!prewrite())
	{
		nlwarning("CDBReferenceFile::buildEmptyRef(): failed, cannot prewrite file '%s'", filepath.c_str());
		close();
		return false;
	}

	close();

	return true;
}



/*
 * Prewrite reference file.
 * At least, read file header to known the base and index index in file
 */
bool	CDBReferenceFile::prewrite(bool failIfNotExist)
{
	// check file already open
	if (_File != NULL)
		return true;

	string	filepath = _Path+_Name;

	// not?
	// check file exists
	if (!CFile::fileExists(filepath))
	{
		if (failIfNotExist)
		{
			nlwarning("CDBReferenceFile::prewrite(): failed, file '%s' does not exist", filepath.c_str());
			return false;
		}

		// open file
		if (!open(filepath.c_str(), "wb"))
		{
			nlwarning("CDBReferenceFile::prewrite(): failed, cannot open file '%s' for write", filepath.c_str());
			return false;
		}

		// write header, set to output mode
		setInOut(false);
	}
	else
	{
		// open file
		if (!open(filepath.c_str(), "r+b"))
		{
			nlwarning("CDBReferenceFile::prewrite(): failed, cannot open file '%s' for read", filepath.c_str());
			return false;
		}

		if (fseek(_File, 0, SEEK_SET) != 0)
		{
			nlwarning("CDBReferenceFile::prewrite(): failed, cannot seek start of '%s' to read header", filepath.c_str());
			return false;
		}

		// read header, set to input mode
		setInOut(true);
	}

	// serial header
	try
	{
		if (!serialHeader())
		{
			nlwarning("CDBReferenceFile::prewrite(): failed, cannot read file '%s' header", filepath.c_str());
			return false;
		}
	}
	catch (const Exception& e)
	{
		nlwarning("CDBReferenceFile::prewrite(): failed, cannot read file '%s' header, exception '%s'", filepath.c_str(), e.what());
		return false;
	}

	// write data, set to output mode
	setInOut(false);

	_Mode = Update;

	//
	PDS_LOG_DEBUG(1)("CDBReferenceFile::prewrite(): opened file '%s' in Update mode", filepath.c_str());

	return true;
}

/*
 * Postwrite reference file
 * Mark file as valid, close file, flush anything still alive...
 */
bool	CDBReferenceFile::postwrite()
{
	if (_File == NULL)
		return true;

	string	filepath = _Path+_Name;

	if (fseek(_File, 0, SEEK_SET) != 0)
	{
		nlwarning("CDBReferenceFile::postwrite(): failed, cannot seek start of '%s' to read header", filepath.c_str());
		return false;
	}

	// serial header
	try
	{
		if (!serialHeader())
		{
			nlwarning("CDBReferenceFile::postwrite(): failed, cannot read file '%s' header", filepath.c_str());
			return false;
		}
	}
	catch (const Exception& e)
	{
		nlwarning("CDBReferenceFile::postwrite(): failed, cannot read file '%s' header, exception '%s'", filepath.c_str(), e.what());
		return false;
	}

	return true;
}


/*
 * Update Start/End Delta Ids
 */
bool	CDBReferenceFile::updateDeltaIds(uint32 startId, uint32 endId)
{
	if (_Header.StartDeltaId == 0 && _Header.EndDeltaId == 0)
	{
		_Header.StartDeltaId = startId;
		_Header.EndDeltaId = endId;
		return true;
	}

	if (_Header.EndDeltaId != startId && _Header.EndDeltaId != startId-1)
	{
		string	filepath = _Path+_Name;
		nlwarning("CDBReferenceFile::updateDeltaIds(): non consecutive delta ids, file '%s' end=%d, update start=%d", filepath.c_str(), _Header.EndDeltaId, startId);
		return false;
	}

	_Header.EndDeltaId = endId;
	return true;
}


/*
 * Get Start/End Delta Ids
 */
void	CDBReferenceFile::getUpdateDeltaIds(uint32& startId, uint32& endId)
{
	startId = _Header.StartDeltaId;
	endId = _Header.EndDeltaId;
}


/*
 * Update a row in the reference file
 * \param index is the absolute row index to update, not relative to file base index
 * \param data is the data buffer to store in file
 */
bool	CDBReferenceFile::update(uint32 index, const uint8* rowdata)
{
	string	filepath = _Path+_Name;

	if (!prewrite())
	{
		nlwarning("CDBReferenceFile::update(): failed, failed to prewrite '%s'", filepath.c_str());
		return false;
	}

	if (_Mode != Update)
	{
		nlwarning("CDBReferenceFile::update(): failed, file '%s' not opened in Update mode", filepath.c_str());
		return false;
	}

	// check row belongs to file (not over OverIndex)
	if (index >= _Header.OverIndex)
	{
		nlwarning("CDBReferenceFile::update(): failed, index '%d' is over file '%s' limit '%s'", index, filepath.c_str(), _Header.OverIndex);
		return false;
	}

	// check row is not beyong physical file data
	if (index >= _Header.EndIndex)
	{
		// not?
		// increase file size by filling blank, fill row indices where needed)

		// seek to end
		if (fseek(_File, getSeekPos(_Header.EndIndex), SEEK_SET) != 0)
		{
			nlwarning("CDBReferenceFile::update(): failed, can't seek to end in file '%s'", filepath.c_str());
			return false;
		}

		// allocate blank buffer
		uint8*	tempRowBuffer = new uint8[_Header.FullRowSize];
		memset(tempRowBuffer, 0, _Header.FullRowSize);

		// dump empty rows till we get to end index
		while (_Header.EndIndex <= index)
		{
			// setup row index
			*(uint32*)tempRowBuffer = index;
			if (!writeBuffer(tempRowBuffer, _Header.FullRowSize))
			{
				nlwarning("CDBReferenceFile::update(): failed, can't increase file '%s' size", filepath.c_str());
				delete tempRowBuffer;
				return false;
			}

			++_Header.EndIndex;
		}
	}

	// seek to row in file
	if (fseek(_File, getSeekPos(index)+getRowHeaderSize(), SEEK_SET) != 0)
	{
		nlwarning("CDBReferenceFile::update(): failed, can't seek to index '%d' data in file '%s'", index, filepath.c_str());
		return false;
	}

	// write data
	if (!writeBuffer(rowdata, _Header.RowSize))
	{
		nlwarning("CDBReferenceFile::update(): failed, can't write index '%d' data in file '%s'", index, filepath.c_str());
		return false;
	}

	return true;
}


/*
 * Preload reference file.
 * At least, read file header to known the base and index index in file
 */
bool	CDBReferenceFile::preload()
{
	// check file already open
	if (_File != NULL)
		return true;

	// force read mode
	_Mode = Read;

	string	filepath = _Path+_Name;

	// file doesn't exist, do nothing
	if (!CFile::fileExists(filepath))
		return true;

	// open file
	if (!open(filepath.c_str(), "rb"))
	{
		nlwarning("CDBReferenceFile::preload(): failed, cannot open file '%s'", filepath.c_str());
		return false;
	}

	//
	PDS_LOG_DEBUG(1)("CDBReferenceFile::preload(): opened file '%s' in Read mode", filepath.c_str());

	setInOut(true);

	// serial header
	try
	{
		if (!serialHeader())
		{
			nlwarning("CDBReferenceFile::preload(): failed, cannot read file '%s' header", filepath.c_str());
			return false;
		}
	}
	catch (const Exception& e)
	{
		nlwarning("CDBReferenceFile::preload(): failed, cannot read file '%s' header, exception '%s'", filepath.c_str(), e.what());
		return false;
	}

	return true;
}

/*
 * Read a row in the reference file
 * \param index is the absolute row index to read, not relative to file base index
 * \param data is the data buffer to store data read from file
 */
bool	CDBReferenceFile::read(uint32 index, uint8* rowdata)
{
	string	filepath = _Path+_Name;

	// preload will fail only if file exists and cannot be read
	// preload returns true when everything ok or file doesn't exist
	if (!preload())
	{
		nlwarning("CDBReferenceFile::read(): failed, failed to preload '%s'", filepath.c_str());
		return false;
	}

	// check mode...
	if (_Mode != Read)
	{
		nlwarning("CDBReferenceFile::read(): failed, file '%s' not opened in Read mode", filepath.c_str());
		return false;
	}

	// check row belongs to file (not over OverIndex)
	if (index >= _Header.OverIndex)
	{
		nlwarning("CDBReferenceFile::read(): failed, index '%d' is over file '%s' end '%d'", index, filepath.c_str(), _Header.OverIndex);
		return false;
	}

	// check file opened or row is not beyond file end
	if (_File == NULL || index >= _Header.EndIndex)
	{
		PDS_LOG_DEBUG(1)("CDBReferenceFile::read(): row '%d' is beyond file '%s' end '%d', row is empty", index, filepath.c_str(), _Header.EndIndex);
		memset(rowdata, 0, _Header.RowSize);
		return true;
	}

	// seek to row in file
	if (fseek(_File, getSeekPos(index)+getRowHeaderSize(), SEEK_SET) != 0)
	{
		nlwarning("CDBReferenceFile::read(): failed, can't seek to index '%d' data in file '%s'", index, filepath.c_str());
		return false;
	}

	// write data
	if (!readBuffer(rowdata, _Header.RowSize))
	{
		nlwarning("CDBReferenceFile::read(): failed, can't read index '%d' data in file '%s'", index, filepath.c_str());
		return false;
	}

	return true;
}





/*
 * Serial file header
 */
bool	CDBReferenceFile::serialHeader()
{
	serialCheck(NELID("DbRf');
	uint	version = serialVersion(0);

	if (isReading())
	{
		// on reading, read header in a temp buffer
		CRefHeader	hdr;
		serial(hdr);

		// check header complies
		if ((_Header.BaseIndex != 0 && hdr.BaseIndex != _Header.BaseIndex) || (isReading() && _Header.RowSize != 0 && hdr.RowSize != _Header.RowSize))
			return false;

		// compy temp to header
		_Header = hdr;
	}
	else
	{
		serial(_Header);
	}

	serialCheck(NELID("Data');

	_DataStart = ftell(_File);

	if (isReading())
	{
		// get file size to compute real EndIndex
		uint32	filesize = CFile::getFileSize(_File);

		uint32	numRows = (filesize-_DataStart) / _Header.FullRowSize;
		// check exact number of rows in file...
		if ((filesize-_DataStart) % _Header.FullRowSize != 0)
		{
			nlwarning("CDBReferenceFile::serialHeader(): failed, file doesn't contain an exact number of rows");
			return false;
		}

		// compute exact end index
		_Header.EndIndex = _Header.BaseIndex + numRows;
	}

	return true;
}
