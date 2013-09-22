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

#include "db_delta_file.h"

#include <nel/misc/debug.h>

using namespace NLMISC;
using namespace std;

/*
 * Constructor
 */
CDBDeltaFile::CDBDeltaFile()
{
}

/*
 * Constructor
 */
CDBDeltaFile::~CDBDeltaFile()
{
}


/*
 * Setup file name and path
 */
void	CDBDeltaFile::setup(const std::string& name, const std::string& path, uint32 rowSize, const CTimestamp& startTimestamp, const CTimestamp& endTimestamp)
{
	uint32		tableId;
	CTimestamp	timestamp;
	nlassert(isDeltaFileName(name, tableId, timestamp));

	_Name = name;
	_Path = NLMISC::CPath::standardizePath(path);

	_Header.RowSize = rowSize;
	_Header.FullRowSize = rowSize + getRowHeaderSize();
	//_Header.Timestamp = timestamp.toTime();
	_Header.StartTimestamp = uint32(startTimestamp.toTime());
	_Header.EndTimestamp = uint32(endTimestamp.toTime());
}

/*
 * Setup file name and path
 */
void	CDBDeltaFile::setup(const std::string& filepath, uint32 rowSize, const CTimestamp& startTimestamp, const CTimestamp& endTimestamp)
{
	setup(CFile::getFilename(filepath), NLMISC::CPath::standardizePath(CFile::getPath(filepath)), rowSize, startTimestamp, endTimestamp);
}



/*
 * Write next row modification
 */
bool	CDBDeltaFile::write(uint32 index, const uint8* rowdata)
{
	string	filepath = _Path+_Name;

	// check file already open
	if (_File == NULL)
	{
		// not?
		// check file exists
		if (CFile::fileExists(filepath))
		{
			nlwarning("CDBDeltaFile::write(): failed, file '%s' exists yet", filepath.c_str());
			return false;
		}

		// open file
		if (!open(filepath.c_str(), "wb"))
		{
			nlwarning("CDBDeltaFile::write(): failed, cannot open file '%s'", filepath.c_str());
			return false;
		}

		setInOut(false);

		// serial header
		try
		{
			if (!serialHeader())
			{
				nlwarning("CDBDeltaFile::write(): failed, cannot write file '%s' header", filepath.c_str());
				return false;
			}
		}
		catch (const Exception& e)
		{
			nlwarning("CDBDeltaFile::write(): failed, cannot write file '%s' header, exception '%s'", filepath.c_str(), e.what());
			return false;
		}
	}

	// check row doesn't belong to map yet
	TIndexMap::iterator	it = _IndexMap.find(index);
	if (it == _IndexMap.end())
	{
		// seek to end (should be after last row)
		uint32	rowSeek = _DataStart + _Header.FullRowSize*(uint32)_IndexMap.size();

		// a little check
		if (fseek(_File, 0, SEEK_END) != 0)
		{
			nlwarning("CDBDeltaFile::write(): failed to seek end of file '%s'", filepath.c_str());
			return false;
		}

		if (rowSeek != (uint32)ftell(_File))
		{
			nlwarning("CDBDeltaFile::write(): failed to seek end of file, computed seek '%d' different from end of file '%s' position '%d'", rowSeek, filepath.c_str(), ftell(_File));
			return false;
		}

		_IndexMap[index] = rowSeek;

		static uint8	rowHdrBuffer[32];
		*(uint32*)rowHdrBuffer = index;

		// write data
		if (!writeBuffer(rowHdrBuffer, getRowHeaderSize()))
		{
			nlwarning("CDBDeltaFile::write(): failed, can't write index '%d' header in file '%s'", index, filepath.c_str());
			return false;
		}
	}
	else
	{
		uint32	rowSeek = (*it).second;

		// a little check
		if (fseek(_File, rowSeek+getRowHeaderSize(), SEEK_SET) != 0)
		{
			nlwarning("CDBDeltaFile::write(): failed to seek to index '%d' data of file '%s'", index, filepath.c_str());
			return false;
		}
	}

	// write data
	if (!writeBuffer(rowdata, _Header.RowSize))
	{
		nlwarning("CDBDeltaFile::write(): failed, can't write index '%d' data in file '%s'", index, filepath.c_str());
		return false;
	}

	return true;
}

/*
 * Read next row modification
 */
bool	CDBDeltaFile::read(uint32& index, uint8* rowdata)
{
	string	filepath = _Path+_Name;

	if (!preload())
	{
		nlwarning("CDBDeltaFile::read(): failed to preload() file '%s'", filepath.c_str());
		return false;
	}

	static uint8	rowHdrBuffer[32];

	// read data
	uint			readLen;
	if (!readBuffer(rowHdrBuffer, getRowHeaderSize(), readLen))
	{
		// failed to read, check if end of file
		if (feof(_File) && readLen == 0)
		{
			index = 0xffffffff;
			return true;
		}

		nlwarning("CDBDeltaFile::read(): failed, can't read index header in file '%s'", filepath.c_str());
		return false;
	}

	index = *(uint32*)rowHdrBuffer;

	// write data
	if (!readBuffer(rowdata, _Header.RowSize))
	{
		nlwarning("CDBDeltaFile::read(): failed, can't read index '%d' data in file '%s'", index, filepath.c_str());
		return false;
	}

	return true;
}



/*
 * Preload file
 */
bool	CDBDeltaFile::preload()
{
	// check file already open
	if (_File != NULL)
		return true;

	string	filepath = _Path+_Name;

	// not?
	// check file exists
	if (!CFile::fileExists(filepath))
	{
		nlwarning("CDBDeltaFile::read(): failed, file '%s' doesn't exist", filepath.c_str());
		return false;
	}

	// open file
	if (!open(filepath.c_str(), "rb"))
	{
		nlwarning("CDBDeltaFile::read(): failed, cannot open file '%s'", filepath.c_str());
		return false;
	}

	setInOut(true);

	// serial header
	try
	{
		if (!serialHeader())
		{
			nlwarning("CDBDeltaFile::read(): failed, cannot write file '%s' header", filepath.c_str());
			return false;
		}
	}
	catch (const Exception& e)
	{
		nlwarning("CDBDeltaFile::read(): failed, cannot write file '%s' header, exception '%s'", filepath.c_str(), e.what());
		return false;
	}

	return true;
}



/*
 * Serial file header
 */
bool	CDBDeltaFile::serialHeader()
{
	serialCheck(NELID("DbDt');
	uint	version = serialVersion(0);

	if (isReading())
	{
		// on reading, read header in a temp buffer
		CDeltaHeader	hdr;
		serial(hdr);

		// check header complies
		if (_Header.RowSize != 0 && hdr.RowSize != _Header.RowSize)
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

	return true;
}



/*
 * Concats a delta file to this one
 */
bool	CDBDeltaFile::concat(CDBDeltaFile& delta, const CTimestamp& starttime, const CTimestamp& endtime)
{
	string	filepath = _Path+_Name;

	if (!delta.preload())
	{
		nlwarning("CDBDeltaFile::concat(): failed to concat in file '%s', cannot preload file '%s'", filepath.c_str(), delta._Name.c_str());
		return false;
	}

	// check file has valid row size
	if (_Header.RowSize == 0)
	{
		setup(_Name, _Path, delta.getRowSize(), starttime, endtime);
	}
	else if (_Header.RowSize != delta.getRowSize())
	{
		nlwarning("CDBDeltaFile::concat(): failed, file '%s' has different row size from '%s'", delta._Name.c_str(), filepath.c_str());
		return false;
	}

	// check & set header ids
	if (_Header.StartDeltaId == 0 && _Header.EndDeltaId == 0)
	{
		// header ids not yet set
		_Header.StartDeltaId = delta._Header.StartDeltaId;
		_Header.EndDeltaId = delta._Header.EndDeltaId;
	}
	else if (_Header.EndDeltaId == delta._Header.StartDeltaId ||
			 _Header.EndDeltaId == delta._Header.StartDeltaId-1)
	{
		// update delta id
		_Header.EndDeltaId = delta._Header.EndDeltaId;
	}
	else
	{
		// failure!
		nlwarning("CDBDeltaFile::concat(): non consecutive delta ids in delta files: file '%s' end=%d, file '%s' start=%d", filepath.c_str(), _Header.EndDeltaId, delta._Path.c_str(), delta._Header.StartDeltaId);
		return false;
	}

	vector<uint8>	databuffer(_Header.RowSize);
	uint8*			data = &(databuffer[0]);

	uint32			row;

	// read a row, and write it if read row is valid
	// loop till there are rows to read
	while (true)
	{
		if (!delta.read(row, data))
		{
			nlwarning("CDBDeltaFile::concat(): failed to concat to '%s', cannot read next row update in file '%s'", filepath.c_str(), delta._Name.c_str());
			return false;
		}

		if (row == 0xffffffff)
			break;

		if (!write(row, data))
		{
			nlwarning("CDBDeltaFile::concat(): failed to write row '%d' update in file '%s'", row, filepath.c_str());
			return false;
		}
	}

	return true;
}
