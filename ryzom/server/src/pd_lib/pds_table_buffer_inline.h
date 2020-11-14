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

#ifndef NL_PDS_TABLE_BUFFER_H
#error pds_table_buffer_inline.h MUST be included by pds_table_buffer.h
#endif

#ifndef NL_PDS_TABLE_BUFFER_INLINE_H
#define NL_PDS_TABLE_BUFFER_INLINE_H


/*
 * Copy header info from another accessor
 * Warning, this is only a local copy of header, data won't been updated in parent CTableBuffer
 */
inline void	CTableBuffer::CAccessor::copyHeader(const CAccessor& access)
{
	const CMappedHeader*	srcHdr = (const CMappedHeader*)(access.fullRow());
	CMappedHeader*			dstHdr = (CMappedHeader*)fullRow();

	// copy flags
	dstHdr->clear();
	dstHdr->setFlags(srcHdr->getFlags());
	// if row is mapped, copy map key
	if (_Mapped)
	{
		dstHdr->setKey(0);
		// only copy if src is also mapped
		if (access.mapped())
			dstHdr->setKey(srcHdr->getKey());
	}
}



/*
 * Init Number of Rows per File
 */
inline bool	CTableBuffer::initRowsPerFile()
{
	if (_InternalRowSize == 0)
	{
		PDS_WARNING("initRowsPerFile(): failed, internal row size is 0");
		return false;
	}

	uint32	actualRowSize = _InternalRowSize+CDBReferenceFile::getRowHeaderSize();
	_RowsPerFile = (_MaxRefFileSize+actualRowSize-1) / actualRowSize;
	return true;
}

/*
 * Check reference file is ready
 */
inline bool	CTableBuffer::checkRef(uint32 refFile)
{
	nlassert(_Init);

	// get reference path
	std::string		path = NLMISC::CPath::standardizePath(_RefPath);

	// reserve room
	if (_RefFileMap.size() <= refFile)
		_RefFileMap.resize(refFile+1, NULL);

	if (_RefFileMap[refFile] == NULL)
		_RefFileMap[refFile] = new CDBReferenceFile();

	// if file not initialised, do it
	if (!_RefFileMap[refFile]->initialised())
	{
		uint32		base = _RowsPerFile * refFile;
		uint32		end = base + _RowsPerFile;
		std::string	file = CDBReferenceFile::getRefFileName(_TableId, refFile);
		_RefFileMap[refFile]->setup(file, path, base, end, _InternalRowSize);
	}

	return true;
}

/*
 * Acquire a row, internal version
 */
inline bool	CTableBuffer::acquireRow(TRowMap::iterator it)
{
	CHeader*	header =(CHeader*)((*it).second);

	if (header->acquire())
	{
		// remove from release set, if belonged to it
		_ReleaseSet.erase((*it).first);
	}

	PDS_FULL_DEBUG("acquireRow(%d)", (*it).first);

	return true;
}

/*
 * Release a row, internal version
 */
inline bool	CTableBuffer::releaseRow(TRowMap::iterator it, bool forceNotAcquired)
{
	CHeader*	header =(CHeader*)((*it).second);

	if (header->unacquire() || forceNotAcquired)
	{
		header->clearAcquireCount();

		// if row is not different from reference, purge it now
		if (header->getDirtStamp() < _ReferenceStamp)
		{
			PDS_FULL_DEBUG("releaseRow(%d): unloaded", (*it).first);

			delete (*it).second;
			_ReleaseSet.erase((*it).first);
			_RowMap.erase(it);
			return true;
		}

		// insert into releasable rows
		_ReleaseSet.insert((*it).first);
	}

	PDS_FULL_DEBUG("releaseRow(%d)", (*it).first);

	return true;
}

/*
 * Set Row As Dirty
 */
inline void	CTableBuffer::CHeader::setDirty()
{
	uint32	dirtStamp = CTableBuffer::getCommonStamp();
	setFlags(Dirty);
	if (dirtStamp > _DirtStamp)
		_DirtStamp = dirtStamp;
	else if (dirtStamp < _DirtStamp)
		nlwarning("CTableBuffer::CHeader::setDirty(): try to dirty row with an older timestamp (new=%d, previous=%d)", dirtStamp, _DirtStamp);
}


#endif // NL_PDS_TABLE_BUFFER_INLINE_H

/* End of pds_table_buffer_inline.h */
