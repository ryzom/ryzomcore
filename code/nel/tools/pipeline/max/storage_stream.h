/**
 * \file storage_stream.h
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
 * CStorageStream
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

#ifndef PIPELINE_STORAGE_STREAM_H
#define PIPELINE_STORAGE_STREAM_H
#include <nel/misc/types_nl.h>

// STL includes
#include <map>

// 3rd Party includes
#include <gsf/gsf-infile.h>

// NeL includes
#include <nel/misc/stream.h>

// Project includes

namespace PIPELINE {
namespace MAX {

/**
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
 * CStorageStream
 */
class CStorageStream : public NLMISC::IStream
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

		inline sint32 getSizeWithHeader() { return (sint32)(Size & 0x7FFFFFFF); }
		inline sint32 getSize() { return getSizeWithHeader() - (sint32)HeaderSize; }
		inline bool isContainer() { return (Size & 0x80000000) == 0x80000000; }
		inline sint32 endOfChunk() { return OffsetBegin + getSizeWithHeader(); }
		inline sint32 getDataBegin() { return OffsetBegin + (sint32)HeaderSize; }
	};

public:
	CStorageStream(GsfInput *input);
	virtual ~CStorageStream();

	virtual bool seek(sint32 offset, TSeekOrigin origin) const;
	virtual sint32 getPos() const;
	// virtual std::string getStreamName() const; // char const   *      gsf_input_name                      (GsfInput *input);
	virtual void serialBuffer(uint8 *buf, uint len);
	virtual void serialBit(bool &bit);

	bool eof();

	// Returns true if there's another chunk, false if no more chunks in this container or if the current chunk is not a container
	bool enterChunk();
	// Reads and skips chunks until the one with given id is found, or writes a chunk with this id
	bool enterChunk(uint16 id);
	// Returns the number of skipped bytes in read more, returns chunk size including header in write mode
	sint32 leaveChunk();

	inline bool is64Bit() const { return m_Is64Bit; }
	inline void set64Bit(bool enabled = true) { m_Is64Bit = enabled; }

	inline uint16 getChunkId() { return currentChunk()->Id; }
	inline sint32 getChunkSize() { return currentChunk()->getSize(); }
	inline bool isChunkContainer() { return currentChunk()->isContainer(); }
	inline bool endOfChunk() { return m_Chunks.size() == 1 ? eof() : CStorageStream::getPos() >= currentChunk()->endOfChunk(); }

private:
	inline CChunk *currentChunk() { return &m_Chunks[m_Chunks.size() - 1]; }

private:
	GsfInput *m_Input;
	void *m_Output; // todo
	std::vector<CChunk> m_Chunks;
	bool m_Is64Bit;

/* there exist compressed max files, so maybe we will need this at some point
GsfInput *          gsf_input_uncompress                (GsfInput *src);
*/

}; /* class CStorageStream */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_STREAM_H */

/* end of file */
