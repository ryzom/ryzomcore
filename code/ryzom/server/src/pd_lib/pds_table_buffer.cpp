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

#include "pds_table_buffer.h"

#include "nel/misc/time_nl.h"

#include "pd_lib.h"
#include "pd_utils.h"

#include <time.h>

uint32	CTableBuffer::_MaxRefFileSize = 32768; //128*1024*1024;

// Common stamp
uint32	CTableBuffer::_CommonStamp = 0;

/*
 * Constructor
 */
CTableBuffer::CTableBuffer()
{
	clear();
}

/*
 * Destructor
 */
CTableBuffer::~CTableBuffer()
{
	clear();
}

/*
 * Clear all
 */
void	CTableBuffer::clear()
{
	_Init = false;
	_TableId = 0;
	_RowSize = 0;
	_InternalRowSize = 0;
	_RowsPerFile = 0;
	_RowMap.clear();
	_ReleaseSet.clear();
	_DirtyList.clear();
	purgeReferences();
	_RowMapper.clear();
	_Mapped = false;
	_ReferenceStamp = 0;
	_CurrentDeltaId = 0;
}

/*
 * Init
 */
void	CTableBuffer::init(uint32 tableId, uint32 rowSize, bool mapped)
{
	clear();

	_TableId = tableId;
	_RowSize = rowSize;
	_Mapped = mapped;
	_InternalRowSize = _RowSize + getHeaderSize();
	// compute maximum number of rows a ref file will contain
	if (!initRowsPerFile())
		return;

	_Init = true;
}

/*
 * Init
 */
void	CTableBuffer::init(uint32 tableId, const std::string& refRootPath, const std::string& refPath) //CRefIndex& ref)
{
	clear();

	_TableId = tableId;

	_RefRootPath = refRootPath;
	_RefPath = refPath;

	_Init = true;
}

/*
 * Setup Ref
 */
void	CTableBuffer::setupRef(CRefIndex& ref)
{
	nlassert(_Init);

	_RefRootPath = ref.getRootPath();
	_RefPath = ref.getPath();
	_ReferenceStamp = uint32(ref.Timestamp.toTime());

	// clear all ref files
	purgeReferences();

	// release all rows that can be released
	flushReleased();
}




/*
 * Return a pointer to straight row data
 * If row is not loaded, data are loaded from table file
 */
CTableBuffer::CAccessor	CTableBuffer::getRow(RY_PDS::TRowIndex row)
{
	nlassert(_Init);

	TRowData			rowData = NULL;
	TRowMap::iterator	it = _RowMap.find(row);

	PDS_FULL_DEBUG("getRow(): row '%d'", row);

	if (it == _RowMap.end())
	{
		rowData = new uint8[_InternalRowSize];

		it = _RowMap.insert(TRowMap::value_type(row, rowData)).first;
		loadRow(row, rowData);
	}

	acquireRow(it);

	return CAccessor(it, _Mapped);
}

/*
 * Return a pointer to straight new row data
 * Row is filled with blank, not loaded from file
 */
CTableBuffer::CAccessor	CTableBuffer::getEmptyRow(RY_PDS::TRowIndex row)
{
	nlassert(_Init);

	TRowData			rowData = NULL;
	TRowMap::iterator	it = _RowMap.find(row);

	if (it == _RowMap.end())
	{
		rowData = new uint8[_InternalRowSize];
		it = _RowMap.insert(TRowMap::value_type(row, rowData)).first;
		memset(rowData, 0, _InternalRowSize);
	}
	else
	{
		PDS_WARNING("getEmptyRow(): row '%d' already exists! data might be lost if buffer modified!", row);
	}

	acquireRow(it);

	return CAccessor(it, _Mapped);
}

/*
 * Acquire a Row
 * Row is marked as non releasable until it is released enough time
 */
bool	CTableBuffer::acquireRow(CAccessor accessor)
{
	nlassert(_Init);

	return acquireRow(accessor._MapIt);
}

/*
 * Release a Row
 * Row is marked as purgeable, and will be purged as soon as possible
 * unless it is reactivated before purge
 */
bool	CTableBuffer::releaseRow(CAccessor accessor)
{
	nlassert(_Init);

	return releaseRow(accessor._MapIt);
}

/*
 * Release a Row
 * Row is marked as purgeable, and will be purged as soon as possible
 * unless it is reactivated before purge
 */
bool	CTableBuffer::releaseRow(RY_PDS::TRowIndex row)
{
	nlassert(_Init);

	TRowMap::iterator	it = _RowMap.find(row);

	if (it == _RowMap.end())
	{
		return false;
	}

	return releaseRow(it);
}

/*
 * Release all rows
 */
bool	CTableBuffer::releaseAll()
{
	TRowMap::iterator	it, itr;

	for (it=_RowMap.begin(); it!=_RowMap.end(); )
	{
		// force all rows as unacquired
		itr = it++;
		releaseRow(itr, true);
	}

	return true;
}






/*
 * Load a row from the appropriate reference file
 */
bool	CTableBuffer::loadRow(RY_PDS::TRowIndex row, TRowData rowData)
{
	nlassert(_Init);

	// locate reference file
	uint32	file = row / _RowsPerFile;

	// check reference is ready
	if (!checkRef(file))
		return false;

	CDBReferenceFile&	refFile = *(_RefFileMap[file]);

	if (!refFile.read(row, rowData))
		return false;

	// row comes from reference
	// clear Dirty and AcquireCount, as row was loaded from reference
	CHeader*	hdr = (CHeader*)rowData;

	hdr->clearDirtStamp();
	hdr->clearAcquireCount();

	return true;
}



/*
 * Update a Row
 * Update in file the whole row
 */
bool	CTableBuffer::updateRow(CAccessor accessor)
{
	return updateRow(accessor.row(), accessor.fullRow(), true);
}

/*
 * Update a Row
 */
bool	CTableBuffer::updateRow(RY_PDS::TRowIndex row, const TRowData rowData, bool forceWriteToDisk)
{
	nlassert(_Init);

	TRowMap::iterator	it = _RowMap.find(row);

	// if row is not mapped in ram or if disk write is forced
	if (it == _RowMap.end() || forceWriteToDisk)
	{
		// locate reference file
		uint32	file = row / _RowsPerFile;
		// check reference is ready
		if (!checkRef(file))
			return false;

		// update row
		CDBReferenceFile&	refFile = *(_RefFileMap[file]);
		if (!refFile.update(row, rowData))
			return false;
	}

	// if row is mapped in ram
	if (it != _RowMap.end())
	{
		// get row data buffer
		TRowData	dest = (*it).second;

		// row is clean and warm from reference file
		((CHeader*)rowData)->clearDirtStamp();

		// copy data
		memcpy(dest, rowData, _InternalRowSize);
	}

	return true;
}

/*
 * Mark a Row as being dirty for later delta save
 */
bool	CTableBuffer::dirtyRow(CAccessor accessor)
{
	nlassert(_Init);

	CHeader*	header = (CHeader*)accessor.fullRow();

	// check row not dirty already
	if (!header->dirty())
	{
		// add row to list
		header->setDirty();
		_DirtyList.push_back(accessor);
	}

	return true;
}

/*
 * Build Delta
 */
bool	CTableBuffer::buildDelta(const CTimestamp& starttimestamp, const CTimestamp& endtimestamp)
{
	nlassert(_Init);

	// check there is something to write
	if (_DirtyList.empty())
		return true;

	// setup delta file
	CDBDeltaFile	delta;
	std::string		deltaFilename = CDBDeltaFile::getDeltaFileName(_TableId, endtimestamp);
	delta.setup(deltaFilename, _RefRootPath+"seconds", _InternalRowSize, starttimestamp, endtimestamp);

	// setup delta id
	delta.setDeltaIds(_CurrentDeltaId, _CurrentDeltaId);
	++_CurrentDeltaId;

	uint	i;
	// go through all dirty rows
	for (i=0; i<_DirtyList.size(); ++i)
	{
		RY_PDS::TRowIndex	row = _DirtyList[i].row();
		TRowData			data = _DirtyList[i].fullRow();

		// clean dirty flag
		((CHeader*)data)->clearFlags(CHeader::Dirty);

		if (!delta.write(row, data))
			return false;
	}

	// clear list
	_DirtyList.clear();

	//
	std::string		filename = _RefRootPath+"seconds/"+deltaFilename;
	PDS_DEBUG("buildDelta(): built table delta '%s', %d rows written", filename.c_str(), i);

	return true;
}

/*
 * Apply delta changes from a file
 */
bool	CTableBuffer::applyDeltaChanges(const std::string& filename)
{
	CDBDeltaFile	delta;

	delta.setup(filename, _InternalRowSize, CTimestamp(), CTimestamp());

	if (!delta.preload())
	{
		PDS_WARNING("applyDeltaChanges(): failed to preload file '%s'", filename.c_str());
		return false;
	}

	// internal row size not set, get it from delta file
	if (_InternalRowSize == 0)
	{
		// get row size from delta
		_InternalRowSize = delta.getRowSize();
		// recompute good number of rows per file
		initRowsPerFile();
	}
	else if (_InternalRowSize != delta.getRowSize())
	{
		PDS_WARNING("applyDeltaChanges(): delta file '%s' has mismatching row size (%d bytes expected, %d bytes found)", filename.c_str(), _InternalRowSize, delta.getRowSize());
		return false;
	}

	uint32				startDeltaId, endDeltaId;
	delta.getDeltaIds(startDeltaId, endDeltaId);

	if (!updateDeltaIds(startDeltaId, endDeltaId))
	{
		PDS_WARNING("applyDeltaChanges(): failed to update delta ids from file '%s'", filename.c_str());
		return false;
	}

	uint32				index;
	std::vector<uint8>	buffer(_InternalRowSize);
	uint8*				data = &(buffer[0]);

	// read data from delta till the end
	while (true)
	{
		// read data from delta file
		if (!delta.read(index, data))
		{
			PDS_WARNING("applyDeltaChanges(): failed to load next row from delta file '%s'", filename.c_str());
			return false;
		}

		if (index == 0xffffffff)
			return true;

		// update disk data with read buffer
		if (!updateRow(index, data, true))
		{
			PDS_WARNING("applyDeltaChanges(): failed to update row '%d' data from file '%s'", index, filename.c_str());
			return false;
		}
	}

	return true;
}


/*
 * Flush Released Rows from memory
 */
void	CTableBuffer::flushReleased()
{
	TReleaseSet::iterator	it;

	// go through all released rows
	for (it=_ReleaseSet.begin(); it!=_ReleaseSet.end(); )
	{
		// get row
		TRowMap::iterator	rit = _RowMap.find(*it);
		if (rit == _RowMap.end())
		{
			PDS_WARNING("flushReleased(): row '%d' not present, already released?", *it);
			// anyway, remove from released set
			TReleaseSet::iterator	itr = (it++);
			_ReleaseSet.erase(itr);
			continue;
		}

		// check row has really been released
		CHeader*	header = (CHeader*)(*rit).second;
		if (!header->isAcquired())
		{
			PDS_WARNING("flushReleased(): try to release row '%d' not flagged as being released, bypassed", *it);
			// remove from release set
			TReleaseSet::iterator	itr = (it++);
			_ReleaseSet.erase(itr);
			continue;
		}

		// the row may have been released and dirtied at the same time
		// don't flush it, since it has not been deltaed...
		if (header->dirty() || header->getDirtStamp() >= _ReferenceStamp)
		{
			++it;
			continue;
		}

		// delete row
		delete (*rit).second;
		_RowMap.erase(rit);

		// remove from set
		TReleaseSet::iterator	itr = (it++);
		_ReleaseSet.erase(itr);
	}
}

/*
 * Reset dirty tags
 */
void	CTableBuffer::resetDirty()
{
	_DirtyList.clear();

	TRowMap::iterator	it;
	for (it=_RowMap.begin(); it!=_RowMap.end(); ++it)
	{
		CHeader*	header = (CHeader*)((*it).second);
		header->clearFlags(CHeader::Dirty);
	}
}

/*
 * Flush Reference files
 */
void	CTableBuffer::flushRefFiles()
{
	uint	i;
	for (i=0; i<_RefFileMap.size(); ++i)
		if (_RefFileMap[i] != NULL && _RefFileMap[i]->initialised())
			_RefFileMap[i]->flush();
}

/*
 * Purge all references
 */
bool	CTableBuffer::purgeReferences()
{
	uint	i;
	for (i=0; i<_RefFileMap.size(); ++i)
	{
		if (_RefFileMap[i] != NULL && _RefFileMap[i]->initialised())
		{
			_RefFileMap[i]->flush();
			delete _RefFileMap[i];
			_RefFileMap[i] = NULL;
		}
	}

	_RefFileMap.clear();

	return true;
}

/*
 * Open all reference files in reference directory
 */
bool	CTableBuffer::openAllRefFilesWrite()
{
	std::vector<std::string>	refs;

	getReferenceFilesList(refs);

	uint	i;
	for (i=0; i<refs.size(); ++i)
	{
		uint32	tableId;
		uint32	refFileId;

		// check file is a reference of the table
		if (!CDBReferenceFile::isRefFile(refs[i], tableId, refFileId) || tableId != _TableId)
		{
			PDS_WARNING("openAllRefFilesWrite(): file '%s' is not a reference file for table '%d'", refs[i].c_str(), _TableId);
			continue;
		}

		if (!checkRef(refFileId))
		{
			PDS_WARNING("openAllRefFilesWrite(): failed to check reference file '%s' for table '%d'", refs[i].c_str(), _TableId);
			return false;
		}

		if (!_RefFileMap[refFileId]->prewrite())
		{
			PDS_WARNING("openAllRefFilesWrite(): failed to check prewrite reference file '%s' for table '%d'", refs[i].c_str(), _TableId);
			return false;
		}
	}

	return true;
}

/*
 * Open all reference files in reference directory
 */
bool	CTableBuffer::openAllRefFilesRead()
{
	std::vector<std::string>	refs;

	getReferenceFilesList(refs);

	uint32	startId = 0xffffffff;
	uint32	endId = 0xffffffff;

	uint	i;
	for (i=0; i<refs.size(); ++i)
	{
		uint32	tableId;
		uint32	refFileId;

		// check file is a reference of the table
		if (!CDBReferenceFile::isRefFile(refs[i], tableId, refFileId) || tableId != _TableId)
		{
			PDS_WARNING("openAllRefFilesRead(): file '%s' is not a reference file for table '%d'", refs[i].c_str(), _TableId);
			continue;
		}

		if (!checkRef(refFileId))
		{
			PDS_WARNING("openAllRefFilesRead(): failed to check reference file '%s' for table '%d'", refs[i].c_str(), _TableId);
			return false;
		}

		if (!_RefFileMap[refFileId]->preload())
		{
			PDS_WARNING("openAllRefFilesRead(): failed to check preload reference file '%s' for table '%d'", refs[i].c_str(), _TableId);
			return false;
		}

		uint32	endIdCheck;
		_RefFileMap[refFileId]->getUpdateDeltaIds(startId, endIdCheck);

		if (endId == 0xffffffff)
		{
			endId = endIdCheck;
		}
		else if (endId != endIdCheck)
		{
			PDS_WARNING("openAllRefFilesRead(): expected endId '%d', found '%d' in file '%s'", endId, endIdCheck, refs[i].c_str());
			return false;
		}
	}

	_CurrentDeltaId = endId + 1;

	return true;
}


/*
 * Update delta ids for all references
 */
bool	CTableBuffer::updateDeltaIds(uint32 start, uint32 end)
{
	uint	i;
	for (i=0; i<_RefFileMap.size(); ++i)
	{
		if (_RefFileMap[i] == NULL || !_RefFileMap[i]->initialised())
		{
			PDS_WARNING("updateDeltaIds(): for table '%d', file %d not initialised", _TableId, i);
			return false;
		}

		if (!_RefFileMap[i]->updateDeltaIds(start, end))
		{
			PDS_WARNING("updateDeltaIds(): failed to update delta ids for table %d, file %d", _TableId, i);
			return false;
		}
	}

	return true;
}



/*
 * Get Reference Files list
 */
void	CTableBuffer::getReferenceFilesList(std::vector<std::string> &result)
{
	result.clear();
	std::vector<std::string>	files;
	NLMISC::CPath::getPathContent(_RefPath, false, false, true, files);

	uint	i;
	// check all files in reference directory
	for (i=0; i<files.size(); ++i)
	{
		uint32	tableId;
		uint32	refFileId;

		// check file is a reference of the table
		if (!CDBReferenceFile::isRefFile(files[i], tableId, refFileId) || tableId != _TableId)
			continue;

		if (result.size() <= refFileId)
			result.resize(refFileId+1);

		result[refFileId] = files[i];
	}
}


/*
 * Build RowMapper
 */
bool	CTableBuffer::buildRowMapper()
{
	std::vector<std::string>	files;
	getReferenceFilesList(files);

	std::vector<uint8>	buffer(_InternalRowSize);
	uint8*	rowData = &(buffer[0]);

	_RowMapper.clear();

	uint	i;
	// check all files in reference directory
	for (i=0; i<files.size(); ++i)
	{
		if (files[i].empty())
			continue;

		// get row indices in reference file
		RY_PDS::TRowIndex	base, end, row;

		CDBReferenceFile	refFile;

		// init and preload file
		refFile.setup(NLMISC::CFile::getFilename(files[i]), _RefPath, 0, 0, _InternalRowSize);
		if (!refFile.preload())
		{
			PDS_WARNING("buildRowMapper(): failed to preload() '%s'", files[i].c_str());
			return false;
		}

		// get row indices in reference file
		base = refFile.getBaseIndex();
		end = refFile.getEndIndex();

		refFile.close();

		for (row=base; row<end; ++row)
		{
			CAccessor		accessor = getRow(row);

			if (!processRow(accessor))
			{
				PDS_WARNING("buildRowMapper(): failed to process row '%d' in file '%s'", row, files[i].c_str());
				releaseRow(accessor);
				return false;
			}

			releaseRow(accessor);
		}
	}

	return true;
}

/*
 * Process row for RowMapper
 */
bool	CTableBuffer::processRow(CAccessor& accessor)
{
	CMappedHeader*	header = (CMappedHeader*)accessor.fullRow();

	// allocate row if needed
	if (!header->allocated())
		return true;

	if (!_RowMapper.allocate(accessor.row()))
	{
		PDS_WARNING("processRow(): failed to allocate row '%d'", accessor.row());
		return false;
	}

	// map row if need
	// unallocated rows shouldn't be mapped...
	if (!_Mapped)
		return true;

	if (!header->mapped())
	{
		if (!RY_PDS::ResolveUnmappedRows)
		{
			PDS_WARNING("processRow(): failed, row '%d' not mapped", accessor.row());
			return false;
		}
		return true;
	}

	if (_RowMapper.isMapped(header->getKey()))
	{
		// check key not yet mapped
		RY_PDS::CObjectIndex	prevMap = _RowMapper.get(header->getKey());

		// already mapped
		if (!RY_PDS::ResolveDoubleMappedRows)
		{
			PDS_WARNING("processRow(): key '%016"NL_I64"X' already mapped to '%s', failed", header->getKey(), prevMap.toString().c_str());
			return false;
		}

		PDS_WARNING("processRow(): key '%016"NL_I64"X' already mapped to '%s'", header->getKey(), prevMap.toString().c_str());

		if (RY_PDS::ResolveDoubleMappedKeepOlder)
		{
			// clear header map
			header->setKey(0);
			dirtyRow(accessor);
			return true;
		}

		unmapRow(prevMap, header->getKey());
	}

	if (!_RowMapper.map(header->getKey(), RY_PDS::CObjectIndex((RY_PDS::TTableIndex)_TableId, accessor.row())))
	{
		PDS_WARNING("processRow(): failed to map row '%d'", accessor.row());
		return false;
	}

	return true;
}



/*
 * Process Rows, apply external row processing after rows are loaded
 */
bool	CTableBuffer::processRows(IRowProcessor* processor)
{
	RY_PDS::TRowIndex	row;

	for (row=0; row<maxRowIndex(); ++row)
	{
		// process only allocated rows
		if (!_RowMapper.allocated(row))
			continue;

		CAccessor		accessor = getRow(row);

		if (!processor->processRow((RY_PDS::TTableIndex)_TableId, accessor))
		{
			PDS_WARNING("processRows(): failed to process row '%d'", row);
			releaseRow(accessor);
			return false;
		}

		releaseRow(accessor);
	}

	return true;
}



/*
 * Allocate a row in a table
 * \param row is the row to allocate
 * Return true if succeeded
 */
bool	CTableBuffer::allocate(RY_PDS::TRowIndex row, CAccessor& accessor)
{
	// check row is free
	if (_RowMapper.allocated(row))
	{
		PDS_WARNING("allocate(): row '%d' already allocated", row);
		return false;
	}

	accessor = getRow(row);
	CMappedHeader	*header = (CMappedHeader*)accessor.fullRow();
	header->setFlags(CHeader::Allocated);
	_RowMapper.allocate(row);

	// just in case, clear map key
	if (_Mapped && header->getKey() != 0)
	{
		header->setKey(0);
	}

	return dirtyRow(accessor);
}

/*
 * Deallocate a row in a table
 * \param row is the row to deallocate
 * Return true if succeeded
 */
bool	CTableBuffer::deallocate(RY_PDS::TRowIndex row)
{
	// check row is allocated
	if (!_RowMapper.allocated(row))
	{
		PDS_WARNING("deallocate(): row '%d' not yet allocated", row);
		return false;
	}

	CAccessor		accessor = getRow(row);
	CMappedHeader	*header = (CMappedHeader*)accessor.fullRow();

	// unmap row if was previously mapped -- just in case unmap not called
	if (_Mapped && header->getKey() != 0)
	{
		_RowMapper.unmap(header->getKey());
		header->setKey(0);
	}

	header->clearAcquireCount();
	header->clearFlags(CHeader::Allocated);
	_RowMapper.deallocate(row);

	return dirtyRow(accessor);
}

/*
 * Map a row
 * \param row is the row to allocate
 * \param key is the 64 bits row key
 * Return true if succeeded
 */
bool	CTableBuffer::mapRow(const RY_PDS::CObjectIndex &index, uint64 key)
{
	if (!_Mapped)
	{
		PDS_WARNING("mapRow(): table not mapped");
		return false;
	}

	// check row is allocated
	if (!_RowMapper.allocated(index.row()))
	{
		PDS_WARNING("mapRow(): row '%d' not yet allocated", index.row());
		return false;
	}

	CAccessor		accessor = getRow(index.row());
	CMappedHeader	*header = (CMappedHeader*)accessor.fullRow();

	if (!_RowMapper.map(key, index))
	{
		PDS_WARNING("mapRow(): failed to map row '%d' to key '%016"NL_I64"X'", index.row(), key);
		return false;
	}

	// unmap row if was previously mapped
	if (header->getKey() != 0)
	{
		_RowMapper.unmap(header->getKey());
	}

	header->setKey(key);
	dirtyRow(accessor);

	return true;
}

/*
 * Unmap a row in a table
 * \param tableIndex is the table to find row
 * \param key is the 64 bits row key
 * Return true if succeeded
 */
bool	CTableBuffer::unmapRow(const RY_PDS::CObjectIndex &index, uint64 key)
{
	if (!_Mapped)
	{
		PDS_WARNING("unmapRow(): table not mapped");
		return false;
	}

	// check row is allocated
	if (!_RowMapper.allocated(index.row()))
	{
		PDS_WARNING("unmapRow(): row '%d' not yet allocated", index.row());
		return false;
	}

	if (!_RowMapper.unmap(key))
	{
		PDS_WARNING("unmapRow(): failed to unmap row '%d' from key '%016"NL_I64"X", index.row(), key);
		return false;
	}

	CAccessor		accessor = getRow(index.row());
	CMappedHeader	*header = (CMappedHeader*)accessor.fullRow();

	if (header->getKey() != key)
	{
		PDS_WARNING("unmapRow(): row '%d' is mapped to '%016"NL_I64"X', unmap row anyway, system may not recover object", index.row(), key);
	}

	header->setKey(0);
	dirtyRow(accessor);

	return true;
}

/*
 * Get a mapped row
 * \param key is the 64 bits row key
 * Return a valid TRowIndex if success
 */
RY_PDS::CObjectIndex	CTableBuffer::getMappedRow(uint64 key) const
{
	if (!_Mapped)
	{
		PDS_WARNING("mapRow(): table not mapped");
		return RY_PDS::CObjectIndex::null();
	}

	return _RowMapper.get(key);
}



/*
 * Update common Timestamp
 */
void	CTableBuffer::updateCommonStamp()
{
	_CommonStamp = NLMISC::CTime::getSecondsSince1970();
}









/*
 * Setup debug delta file
 */
bool	CTableBuffer::setupDebugDeltaFile(const std::string& filename, CDBDeltaFile& delta) const
{
	delta.setup(filename, _InternalRowSize, CTimestamp(), CTimestamp());

	if (!delta.preload())
	{
		PDS_WARNING("setupDebugDeltaFile(): failed to preload file '%s'", filename.c_str());
		return false;
	}

	return true;
}

/*
 * Get Delta file Row
 */
uint8*	CTableBuffer::getDeltaRow(uint32& row, CDBDeltaFile& delta) const
{
	static std::vector<uint8>	buffer;

	buffer.resize(_InternalRowSize);
	uint8*	data = &(buffer[0]);

	// read data from delta file
	if (!delta.read(row, data))
	{
		PDS_WARNING("getDeltaRow(): failed to load next row from delta file");
		return NULL;
	}

	if (row == 0xffffffff)
		return NULL;

	return data + getHeaderSize();
}
