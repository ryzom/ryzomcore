// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdopengl3.h"
#include "driver_opengl3.h"
#include "driver_opengl3_stream_buffer.h"

#include <cstring>

#include <nel/misc/debug.h>

namespace NL3D {
namespace NLDRIVERGL3 {

namespace /* anonymous */ {

// A chunk that no frame has used for this many frames is deleted
const uint64 s_UnusedFrames = 300;

#if NL3D_GL3_STATE_CHECKS
// What a chunk is filled with before it is written again in place, so that a draw
// from a stale handle shows instead of looking right by accident
const int s_ReusePattern = 0xDD;
#endif

inline uint32 alignOffset(uint32 offset, uint32 alignment)
{
	uint32 rest = offset % alignment;
	return rest ? offset + (alignment - rest) : offset;
}

} /* anonymous namespace */

// ***************************************************************************

CStreamBufferGL3::CStreamBufferGL3()
    : m_States(nullptr)
    , m_Target(0)
    , m_Initialized(false)
    , m_OverLimitWarned(false)
    , m_FrameLimitWarned(false)
    , m_StorageFailureWarned(false)
    , m_ChunkSize(0)
    , m_Alignment(1)
    , m_MinChunks(1)
    , m_MaxChunks(1)
    , m_Head(0)
    , m_HeadOffset(0)
    , m_Frame(0)
    , m_FrameInFlight(0)
    , m_Draw(0)
{
}

CStreamBufferGL3::~CStreamBufferGL3()
{
	// The GL buffers are deleted by release(), which the driver calls while it has
	// its context. Here the context may be gone, so nothing is done with them.
}

// ***************************************************************************

bool CStreamBufferGL3::init(CDriverGLStates3 *states, const char *name, GLenum target, uint32 chunkSize, uint32 alignment,
    uint minChunks, uint maxChunks, uint64 frame, uint64 frameInFlight)
{
	H_AUTO_OGL(CStreamBufferGL3_init)

	nlassert(states);
	nlassert(chunkSize);
	nlassert(alignment);
	nlassert(minChunks >= 1 && maxChunks >= minChunks);

	// Chunks that still have a buffer here were not released: they belong to a display
	// that failed to set up, and their context is not the current one. They are
	// forgotten without a GL call. The state cache forgets them too: the new context
	// hands out the same buffer names, and a bind of one of them is not to be skipped.
	for (size_t i = 0; i < m_Chunks.size(); ++i)
	{
		if (m_Chunks[i].Buffer)
			bufferDeleted(m_Chunks[i].Buffer);
		m_Chunks[i].Buffer = 0;
		m_Chunks[i].Mapped = nullptr;
	}
	releaseChunks();
	m_Initialized = false;
	m_OverLimitWarned = false;
	m_FrameLimitWarned = false;

	m_States = states;
	m_Name = name;
	m_Target = target;
	m_ChunkSize = chunkSize;
	m_Alignment = alignment;
	m_MinChunks = minChunks;
	m_MaxChunks = maxChunks;
	m_Frame = frame;
	m_FrameInFlight = frameInFlight;

	// The first chunk tells whether the context can do this: the warning of a failure names what failed
	m_StorageFailureWarned = false;
	if (!insertChunk(m_ChunkSize))
	{
		releaseChunks();
		return false;
	}

	m_Initialized = true;
	nlinfo("3D: the %s stream has chunks of %u bytes, %u to %u of them, alignment %u",
	    m_Name.c_str(), m_ChunkSize, m_MinChunks, m_MaxChunks, m_Alignment);
	return true;
}

void CStreamBufferGL3::release()
{
	H_AUTO_OGL(CStreamBufferGL3_release)

	if (!m_Initialized)
		return;
	releaseChunks();
	m_Initialized = false;
}

void CStreamBufferGL3::releaseChunks()
{
	// The slots stay, with their generations: a handle that was written before
	// does not become valid again when its slot gets a new chunk
	m_FreeSlots.clear();
	for (size_t i = 0; i < m_Chunks.size(); ++i)
	{
		destroyStorage(m_Chunks[i]);
		m_FreeSlots.push_back((uint32)i);
	}
	m_Ring.clear();
	m_Head = 0;
	m_HeadOffset = 0;
}

// ***************************************************************************

void CStreamBufferGL3::bindBuffer(GLuint buffer)
{
	switch (m_Target)
	{
	case GL_UNIFORM_BUFFER:
		m_States->bindUniformBuffer(buffer);
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		m_States->bindElementArrayBuffer(buffer);
		break;
	case GL_ARRAY_BUFFER:
		m_States->bindArrayBuffer(buffer);
		break;
	default:
		// Any other target is bound directly, which is right for the targets that the state cache does not track
		nglBindBuffer(m_Target, buffer);
		break;
	}
}

void CStreamBufferGL3::bufferDeleted(GLuint buffer)
{
	// GL sets the bindings of a deleted buffer back to zero, the state cache follows
	switch (m_Target)
	{
	case GL_UNIFORM_BUFFER:
		m_States->uniformBufferDeleted(buffer);
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		m_States->elementArrayBufferDeleted(buffer);
		break;
	case GL_ARRAY_BUFFER:
		m_States->arrayBufferDeleted(buffer);
		break;
	default:
		break;
	}
}

bool CStreamBufferGL3::warnFailure(bool &warned)
{
	// A chunk that cannot get storage is tried again at every write that needs
	// it, which can be every block of every draw: the log gets one line for it,
	// and the next one when the same call has worked in between
	if (warned)
		return false;
	warned = true;
	return true;
}

void CStreamBufferGL3::nextGeneration(CChunk &chunk)
{
	// 0 stays the generation of a handle that was never written
	++chunk.Generation;
	if (!chunk.Generation)
		++chunk.Generation;
}

void CStreamBufferGL3::markWritten(CChunk &chunk)
{
	chunk.LastFrame = m_Frame;
	chunk.LastWriteFrame = m_Frame;
	chunk.LastDraw = m_Draw;
	chunk.Evacuated = false;
}

void CStreamBufferGL3::destroyStorage(CChunk &chunk)
{
	if (chunk.Buffer)
	{
		if (chunk.Mapped)
		{
			bindBuffer(chunk.Buffer);
			nglUnmapBuffer(m_Target);
			chunk.Mapped = nullptr;
		}
		bufferDeleted(chunk.Buffer);
		nglDeleteBuffers(1, &chunk.Buffer);
		chunk.Buffer = 0;
	}
	chunk.Size = 0;
	chunk.Locks = 0;
	nextGeneration(chunk);
}

bool CStreamBufferGL3::createStorage(CChunk &chunk, uint32 size)
{
	nextGeneration(chunk);
	markWritten(chunk);

#ifdef USE_OPENGLES3
	// OpenGL ES 3.0 has no immutable storage with a persistent mapping, and the driver creates no stream there
	destroyStorage(chunk);
	if (warnFailure(m_StorageFailureWarned))
		nlwarning("3D: the %s stream cannot create a buffer of %u bytes: no glBufferStorage in OpenGL ES 3.0", m_Name.c_str(), size);
	return false;
#else
	// Immutable storage cannot be replaced: the chunk gets a new buffer. GL keeps
	// the deleted one alive for the draws that still read from it.
	destroyStorage(chunk);
	nglGenBuffers(1, &chunk.Buffer);
	bindBuffer(chunk.Buffer);
	// Persistent: GL allows drawing from the buffer while it is mapped. Coherent: what
	// is written through the mapping is seen by the draws that follow, without a flush.
	const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	nglBufferStorage(m_Target, size, nullptr, flags);
	chunk.Mapped = static_cast<uint8 *>(nglMapBufferRange(m_Target, 0, size, flags));
	if (!chunk.Mapped)
	{
		if (warnFailure(m_StorageFailureWarned))
			nlwarning("3D: the %s stream cannot create and map a buffer of %u bytes with glBufferStorage", m_Name.c_str(), size);
		destroyStorage(chunk);
		return false;
	}
	chunk.Size = size;
	m_StorageFailureWarned = false;
	return true;
#endif
}

// ***************************************************************************

bool CStreamBufferGL3::insertChunk(uint32 size)
{
	uint32 slot;
	if (!m_FreeSlots.empty())
	{
		slot = m_FreeSlots.back();
		m_FreeSlots.pop_back();
	}
	else
	{
		slot = (uint32)m_Chunks.size();
		m_Chunks.push_back(CChunk());
	}

	if (!createStorage(m_Chunks[slot], size))
	{
		m_FreeSlots.push_back(slot);
		return false;
	}

	uint pos = m_Ring.empty() ? 0 : m_Head + 1;
	m_Ring.insert(m_Ring.begin() + pos, slot);
	m_Head = pos;
	m_HeadOffset = 0;
	return true;
}

void CStreamBufferGL3::makeHead(uint pos)
{
	if (pos != m_Head)
	{
		// The chunk moves to the place behind the head, where the ring goes on
		uint32 slot = m_Ring[pos];
		m_Ring.erase(m_Ring.begin() + pos);
		if (pos < m_Head)
			--m_Head;
		++m_Head;
		m_Ring.insert(m_Ring.begin() + m_Head, slot);
	}
	m_HeadOffset = 0;
}

bool CStreamBufferGL3::nextChunk(uint32 size)
{
	H_AUTO_OGL(CStreamBufferGL3_nextChunk)

	const uint32 chunkSize = std::max(m_ChunkSize, size);
	const uint ringSize = (uint)m_Ring.size();

	// The ring is walked from its oldest chunk, the one behind the head, to the head
	// itself, for two things.
	// A chunk that the GPU is done with is found. It is written again in place:
	// nothing reads it any more, so this needs no new storage and no wait. Of several
	// such chunks the youngest is taken: the stream then keeps using as few chunks as
	// it needs, and the ones that a heavy frame added stay unused until they are deleted.
	// And the chunks that only kept handles hold are freed of them: the GPU is done
	// with every write to such a chunk, and still it is in use, because handles that
	// are kept across frames have their data there and each draw from them marks the
	// chunk. Those handles are made stale, so that their owners write the data again
	// into the head. The storage is left alone, the draws that read it stay right.
	// Once the frames that used it have finished, the chunk can be written again.
	// Without this a chunk with one block that is used in every frame would never
	// come free, and in the end no chunk would.
	uint retiredPos = ringSize;
	for (uint i = 1; i <= ringSize; ++i)
	{
		uint pos = (m_Head + i) % ringSize;
		CChunk &chunk = m_Chunks[m_Ring[pos]];
		if (isRetired(chunk))
		{
			retiredPos = pos;
		}
		else if (!chunk.Locks && chunk.LastWriteFrame < m_FrameInFlight && !chunk.Evacuated)
		{
			// Once is enough: nothing is written to the chunk before it is the head again
			nextGeneration(chunk);
			chunk.Evacuated = true;
#if NL3D_GL3_STREAM_STATS
			++m_Stats.Evacuations;
#endif
		}
	}

	if (retiredPos != ringSize)
	{
		CChunk &chunk = m_Chunks[m_Ring[retiredPos]];
		if (chunk.Size >= size)
		{
			nextGeneration(chunk);
			markWritten(chunk);
#if NL3D_GL3_STATE_CHECKS
			if (chunk.Mapped)
				memset(chunk.Mapped, s_ReusePattern, chunk.Size);
#endif
#if NL3D_GL3_STREAM_STATS
			++m_Stats.Reuses;
#endif
		}
		else if (!createStorage(chunk, chunkSize))
		{
			return false;
		}
		makeHead(retiredPos);
		return true;
	}

	// A new chunk
	if (ringSize < m_MaxChunks)
		return insertChunk(chunkSize);

	// The stream is at its limit: a chunk gets new storage. First choice is the oldest
	// chunk that the current frame has not used, no handle of this frame goes stale
	// with it. Else the oldest chunk that the current draw does not use: the draws
	// of this frame that read it are issued, GL keeps the old storage for them, and
	// the blocks of the current draw are in other chunks. Never a chunk with a range
	// that is being written through lock(): its memory is in the hands of the caller.
	for (uint pass = 0; pass < 2; ++pass)
	{
		for (uint i = 1; i <= ringSize; ++i)
		{
			uint pos = (m_Head + i) % ringSize;
			CChunk &chunk = m_Chunks[m_Ring[pos]];
			if (!chunk.Locks && (pass == 0 ? (chunk.LastFrame < m_Frame) : (chunk.LastDraw < m_Draw)))
			{
				if (pass == 1 && !m_FrameLimitWarned)
				{
					// A range of the chunk that was written for a draw that is still to come
					// goes with it: its owner finds the handle stale
					nlwarning("3D: the %s stream fills its limit of %u chunks of %u bytes within one frame, and gives a chunk that this frame has used new storage",
					    m_Name.c_str(), m_MaxChunks, m_ChunkSize);
					m_FrameLimitWarned = true;
				}
				if (!createStorage(chunk, chunk.Size >= size ? chunk.Size : chunkSize))
					return false;
#if NL3D_GL3_STREAM_STATS
				++m_Stats.Discards;
#endif
				makeHead(pos);
				return true;
			}
		}
	}

	// The current draw uses every chunk, or writes to it: none of them can be given up
	if (!m_OverLimitWarned)
	{
		nlwarning("3D: the %s stream grows past its limit of %u chunks of %u bytes, one draw uses all of them or they are being written",
		    m_Name.c_str(), m_MaxChunks, m_ChunkSize);
		m_OverLimitWarned = true;
	}
	return insertChunk(chunkSize);
}

// ***************************************************************************

CStreamHandleGL3 CStreamBufferGL3::reserve(uint32 size)
{
	H_AUTO_OGL(CStreamBufferGL3_reserve)

	nlassert(m_Initialized);
	nlassert(size);

	uint32 offset = alignOffset(m_HeadOffset, m_Alignment);
	CChunk *chunk = &m_Chunks[m_Ring[m_Head]];
#if NL3D_GL3_STREAM_STATS
	// The range and the gap that the alignment leaves before it
	uint64 reserved = (uint64)(offset - m_HeadOffset) + size;
#endif
	if (offset > chunk->Size || size > chunk->Size - offset)
	{
#if NL3D_GL3_STREAM_STATS
		// The range and what is left of the chunk that the head leaves
		reserved = (uint64)(chunk->Size > m_HeadOffset ? chunk->Size - m_HeadOffset : 0) + size;
#endif
		if (!nextChunk(size))
			return CStreamHandleGL3();
		chunk = &m_Chunks[m_Ring[m_Head]];
		offset = 0;
	}

	m_HeadOffset = offset + size;
	markWritten(*chunk);

#if NL3D_GL3_STREAM_STATS
	++m_Stats.Writes;
	m_Stats.Reserved += reserved;
#endif

	CStreamHandleGL3 handle;
	handle.Chunk = m_Ring[m_Head];
	handle.Generation = chunk->Generation;
	handle.Offset = offset;
	handle.Size = size;
	return handle;
}

CStreamHandleGL3 CStreamBufferGL3::write(const void *data, uint32 size, uint32 allocSize)
{
	H_AUTO_OGL(CStreamBufferGL3_write)

	nlassert(size <= allocSize);

	CStreamHandleGL3 handle = reserve(allocSize);
	if (size && isValid(handle))
	{
		// The range is free, no draw reads it: the copy needs no GL call
		memcpy(m_Chunks[handle.Chunk].Mapped + handle.Offset, data, size);
#if NL3D_GL3_STREAM_STATS
		m_Stats.Bytes += size;
#endif
	}
	return handle;
}

void CStreamBufferGL3::writePart(const CStreamHandleGL3 &handle, uint32 offset, const void *data, uint32 size)
{
	H_AUTO_OGL(CStreamBufferGL3_writePart)

	nlassert(isValid(handle));
	nlassert(offset <= handle.Size && size <= handle.Size - offset);

	CChunk &chunk = m_Chunks[handle.Chunk];
	memcpy(chunk.Mapped + handle.Offset + offset, data, size);
	markWritten(chunk);
#if NL3D_GL3_STREAM_STATS
	m_Stats.Bytes += size;
#endif
}

uint8 *CStreamBufferGL3::lock(const CStreamHandleGL3 &handle)
{
	nlassert(isValid(handle));

	// The caller writes at its own pace: until unlock() the chunk keeps its storage,
	// its place and its generation
	CChunk &chunk = m_Chunks[handle.Chunk];
	++chunk.Locks;
	return chunk.Mapped + handle.Offset;
}

void CStreamBufferGL3::unlock(const CStreamHandleGL3 &handle)
{
	// The slot outlives its chunk, and release() drops the locks with the storage
	nlassert(handle.Chunk < m_Chunks.size());
	CChunk &chunk = m_Chunks[handle.Chunk];
	if (!chunk.Locks)
		return;
	--chunk.Locks;

	// What was written through the pointer was written up to now
	markWritten(chunk);
}

// ***************************************************************************

void CStreamBufferGL3::frameSwapped(uint64 frame, uint64 frameInFlight)
{
	H_AUTO_OGL(CStreamBufferGL3_frameSwapped)

	m_Frame = frame;
	m_FrameInFlight = frameInFlight;

	if (!m_Initialized)
		return;

	// A chunk that has been unused for long is deleted, one per frame at most.
	// The head stays.
	const uint ringSize = (uint)m_Ring.size();
	if (ringSize > m_MinChunks)
	{
		for (uint i = 1; i < ringSize; ++i)
		{
			uint pos = (m_Head + i) % ringSize;
			uint32 slot = m_Ring[pos];
			CChunk &chunk = m_Chunks[slot];
			if (isRetired(chunk) && chunk.LastFrame + s_UnusedFrames < m_Frame)
			{
				destroyStorage(chunk);
				m_Ring.erase(m_Ring.begin() + pos);
				if (pos < m_Head)
					--m_Head;
				m_FreeSlots.push_back(slot);
				break;
			}
		}
		if (m_Ring.size() <= m_MaxChunks)
			m_OverLimitWarned = false;
	}
}

} // NLDRIVERGL3
} // NL3D

/* end of file */
