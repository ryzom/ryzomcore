/**
 * \file storage_chunks.h
 * \brief CStorageChunks
 * \date 2012-08-18 09:20GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 * CStorageChunks
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_STORAGE_CHUNKS_H
#define PIPELINE_STORAGE_CHUNKS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/stream.h>

// Project includes

namespace PIPELINE {
namespace MAX {

/**
 * \brief CStorageChunks
 * \date 2012-08-18 09:20GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 * CStorageChunks
 */
class CStorageChunks
{
private:
	struct CChunk
	{
		// Size of the chunk header, 6 for 32 bit, 14 for 64 bit
		uint8 HeaderSize;
		// Where the header starts
		sint32 OffsetBegin;

		// Identifier
		uint16 Id;
		// Size including header size
		uint32 Size;

		inline sint32 getSizeWithHeader() const { return (sint32)(Size & 0x7FFFFFFF); }
		inline sint32 getSize() const { return getSizeWithHeader() - (sint32)HeaderSize; }
		inline bool isContainer() const { return (Size & 0x80000000) == 0x80000000; }
		inline sint32 endOfChunk() const { return OffsetBegin + getSizeWithHeader(); }
		inline sint32 getDataBegin() const { return OffsetBegin + (sint32)HeaderSize; }
	};

public:
	CStorageChunks(NLMISC::IStream &stream, sint64 size = 0);
	virtual ~CStorageChunks();

	// Returns true if there's another chunk, false if no more chunks in this container or if the current chunk is not a container
	bool enterChunk();
	// Reads and skips chunks until the one with given id is found, or writes a chunk with this id.
	// In write mode, as64Bit picks the header width for THIS chunk specifically (6 vs 14 bytes) —
	// callers should pass the per-chunk width recorded at read time (IStorageObject::
	// wasRead64BitChunk()), not the stream-wide is64Bit() sticky flag, so a stream with a genuine
	// mix of 32-bit and 64-bit chunk headers round-trips each chunk at its original width instead
	// of widening everything to whatever the outermost chunk happened to use.
	bool enterChunk(uint16 id, bool container, bool as64Bit = false);
	// Returns the number of skipped bytes in read more, returns chunk size including header in write mode
	sint32 leaveChunk();

	inline bool is64Bit() const { return m_Is64Bit; }
	inline void set64Bit(bool enabled = true) { m_Is64Bit = enabled; }
	// True if the chunk just entered (via enterChunk()) had a 64-bit header. Only meaningful in
	// read mode, right after a successful enterChunk().
	inline bool isCurrentChunk64Bit() const { return currentChunk()->HeaderSize == 14; }

	inline uint16 getChunkId() const { return currentChunk()->Id; }
	inline sint32 getChunkSize() const { return currentChunk()->getSize(); }
	inline bool isChunkContainer() const { return currentChunk()->isContainer(); }
	inline bool endOfChunk() const { return /*m_Chunks.size() == 1 ? eof() :*/ m_Stream.getPos() >= currentChunk()->endOfChunk(); }

	inline NLMISC::IStream &stream() { return m_Stream; }

private:
	inline const CChunk *currentChunk() const { return &m_Chunks[m_Chunks.size() - 1]; }
	inline CChunk *currentChunk() { return &m_Chunks[m_Chunks.size() - 1]; }

private:
	NLMISC::IStream &m_Stream;
	std::vector<CChunk> m_Chunks;
	bool m_Is64Bit;

}; /* class CStorageChunks */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_CHUNKS_H */

/* end of file */
