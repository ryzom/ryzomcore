/**
 * \file storage_stream.h
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
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
#include <vector>

// NeL includes
#include <nel/misc/stream.h>

// Project includes

namespace PIPELINE {
namespace MAX {

/**
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
 * A random-access, in-memory NLMISC::IStream over a single OLE stream's bytes.
 *
 * Historically this was a thin adapter onto a libgsf GsfInput/GsfOutput. The OLE container access
 * moved into the self-contained CStorageOle layer (storage_ole.h) so the whole library is
 * cross-platform and deterministic without a third-party dependency; this stream is now purely a
 * byte-buffer view with the seek semantics the chunk layer needs.
 *
 * Two modes:
 * - Read: constructed over an external, immutable byte buffer (the caller owns it and must keep it
 *   alive for the stream's lifetime). serial() consumes forward; seek() moves the cursor freely.
 * - Write: constructed empty; serial() appends into a growable internal buffer. CStorageChunks
 *   back-patches chunk sizes by seeking to an earlier offset, writing, then seeking back to the
 *   end — this buffer supports that (unlike NLMISC::CMemStream, whose length()==write-position
 *   makes the seek-back-then-restore fail, which is why the tools used temp files before). Retrieve
 *   the accumulated bytes with buffer() / swapBuffer() once serialization is done.
 */
class CStorageStream : public NLMISC::IStream
{
public:
	/// Read mode over an external buffer (not copied, not owned — must outlive this stream).
	CStorageStream(const uint8 *data, size_t size);
	/// Read mode over an external buffer (not copied, not owned — must outlive this stream).
	explicit CStorageStream(const std::vector<uint8> &data);
	/// Write mode into a fresh internal growable buffer.
	CStorageStream();
	virtual ~CStorageStream();

	virtual bool seek(sint32 offset, TSeekOrigin origin) const;
	virtual sint32 getPos() const;
	// virtual std::string getStreamName() const;
	virtual void serialBuffer(uint8 *buf, uint len);
	virtual void serialBit(bool &bit);

	sint32 size();
	bool eof();

	/// Write mode only: the bytes written so far.
	const std::vector<uint8> &buffer() const { return m_WriteBuffer; }
	/// Write mode only: move the accumulated bytes out into \a out (leaves this stream empty).
	void swapBuffer(std::vector<uint8> &out);

private:
	// Read mode: external buffer view.
	const uint8 *m_ReadData;
	size_t m_ReadSize;

	// Write mode: owned growable buffer.
	std::vector<uint8> m_WriteBuffer;

	// Cursor (mutable because IStream::seek()/getPos() are const).
	mutable size_t m_Pos;

}; /* class CStorageStream */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_STREAM_H */

/* end of file */
