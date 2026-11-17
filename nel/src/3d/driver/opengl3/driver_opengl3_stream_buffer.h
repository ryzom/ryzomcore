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

#ifndef NL_DRIVER_OPENGL3_STREAM_BUFFER_H
#define NL_DRIVER_OPENGL3_STREAM_BUFFER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include <string>
#include <vector>

// Set to 1 to count what goes through the streams. The driver then logs the
// counts once per second.
#define NL3D_GL3_STREAM_STATS 0

// Define to make the driver work without streams where it could have them,
// with a GL buffer per owner (see CDriverGL3::_StreamMode), for testing.
// #define NL3D_GL3_STREAM_FORCE_PER_BUFFER

namespace NL3D {
namespace NLDRIVERGL3 {

class CDriverGLStates3;

/**
 * A range written into a CStreamBufferGL3.
 *
 * The handle is good until the stream gives its chunk a new generation: when
 * it writes the chunk again, replaces its storage, or wants it free of the
 * handles that are kept on it. The stream does that without telling the owner
 * of the handle. Before every use the owner asks CStreamBufferGL3::isValid,
 * and writes the data again when the handle has gone stale. A default handle
 * is never valid.
 */
struct CStreamHandleGL3
{
	uint32 Chunk; // slot of the chunk in the stream
	uint32 Generation; // generation of the chunk at the time of the write
	uint32 Offset; // first byte of the range in the GL buffer of the chunk
	uint32 Size; // size of the range in bytes

	CStreamHandleGL3()
	    : Chunk(0)
	    , Generation(0)
	    , Offset(0)
	    , Size(0)
	{
	}
};

/**
 * Buffer memory for data that is written once and drawn from soon after. The
 * driver keeps its uniform blocks in one, the indices of its draws in another,
 * and the vertices of the vertex buffers that are filled for every draw in a
 * third.
 *
 * A stream is a ring of chunks, a chunk is one GL buffer. A write takes the
 * next free bytes of the head chunk. Nothing is ever written over a range that
 * the GPU may still read: when the head chunk is full, the stream moves on to
 *  - a chunk of which the last frame that used it has finished on the GPU. The
 *    chunk is written again in place. Of several such chunks the youngest is
 *    taken, which leaves the chunks that the stream can do without unused;
 *  - else a new chunk, while the stream has fewer chunks than its limit;
 *  - else the oldest chunk that the current frame has not used, or failing
 *    that the oldest chunk that the current draw does not use. It gets new
 *    storage (the storage the GPU still reads from stays alive in GL until it
 *    is done with it). This is what keeps the stream at its limit in a program
 *    that renders without ever swapping buffers, where no frame ever finishes.
 *    A range in a chunk of the current frame that was written for a draw that
 *    is still to come is gone with it; the stream says so in the log, once;
 *  - else, when the current draw uses every chunk, a new chunk past the limit,
 *    with a warning.
 * A chunk that nothing has used for a few hundred frames is deleted, down to
 * a minimum number of chunks. A chunk with a range that is being written
 * through lock() is left alone by all of this.
 *
 * Each of these steps gives the chunk a new generation, which is how a handle
 * of an earlier write finds out that its data is gone.
 *
 * A handle that is kept and used in every frame (a camera block, a material)
 * would keep its chunk in use forever. So each time the head moves on, the
 * chunks that are only held by such uses (the GPU has finished the frames
 * that wrote to them) get a new generation while their storage stays as it
 * is: the owners of the handles write their data again into the head, nothing
 * uses the old chunks any more, and they are free a few frames later. Data
 * that does not change is thus written again every few frames when a chunk
 * fills within a frame or two, and seldom when little is written.
 *
 * A chunk has immutable storage (glBufferStorage) and stays mapped for its
 * whole life, with the persistent bit, which makes GL allow draws from a
 * mapped buffer, and the coherent bit. A write is a memory copy and costs no
 * GL call. A context that cannot do this has no streams: init() fails, and the
 * driver keeps a GL buffer per owner of data instead.
 *
 * One stream serves one binding target, which init() is given together with
 * the alignment of the ranges. Its buffers are never bound to another target.
 */
class CStreamBufferGL3
{
public:
	CStreamBufferGL3();
	~CStreamBufferGL3();

	/**
	 * Create the first chunk. Needs the GL context, and glBufferStorage loaded
	 * (CGlExtensions::ARBBufferStorage).
	 * \param states the state cache of the driver, which the stream binds its buffers through
	 * \param name what the stream holds, for the log
	 * \param target the binding target of the buffers
	 * \param chunkSize size of a chunk in bytes; a larger write gets a chunk of its own size
	 * \param alignment every range starts at a multiple of this
	 * \param minChunks unused chunks are deleted down to this number
	 * \param maxChunks more chunks than this are only created when one draw needs them
	 * \param frame the current frame, IDriver::getSwapBufferCounter()
	 * \param frameInFlight the oldest frame the GPU has not finished, IDriver::getSwapBufferInFlight()
	 * \return false when the chunk cannot be created and mapped; a warning in the log says why
	 */
	bool init(CDriverGLStates3 *states, const char *name, GLenum target, uint32 chunkSize, uint32 alignment,
	    uint minChunks, uint maxChunks, uint64 frame, uint64 frameInFlight);

	/// Unmap and delete the chunks. To call while the GL context is still current.
	void release();

	bool isInitialized() const { return m_Initialized; }
	uint getChunkCount() const { return (uint)m_Ring.size(); }

	/**
	 * Reserve a range of allocSize bytes and copy size bytes of data to its start.
	 * allocSize is larger than size for a block of which only the first part is
	 * known; the rest of the range then holds undefined bytes. size may be 0.
	 * \return the handle of the range. It is not valid when the write failed,
	 * which is when a chunk cannot get storage. The stream logs that as a warning
	 * once, and again when a chunk got storage in between.
	 */
	CStreamHandleGL3 write(const void *data, uint32 size, uint32 allocSize);

	/**
	 * Reserve a range of size bytes for a caller that writes it through lock().
	 * The range holds undefined bytes.
	 * \return the handle of the range, not valid when the reservation failed (see write)
	 */
	CStreamHandleGL3 reserve(uint32 size);

	/**
	 * The memory of the range of a valid handle, to write to: it is memory of
	 * the mapped GL buffer, and reading from it is slow. Until unlock() the
	 * stream keeps the chunk as it is, whatever else it needs room for, so
	 * that the pointer stays good and the handle valid. The range may be
	 * locked again later to write more, as long as the handle is valid and no
	 * draw has read the range: a draw may still be reading it otherwise.
	 */
	uint8 *lock(const CStreamHandleGL3 &handle);
	/// The end of a lock(). The handle may have gone stale only through release().
	void unlock(const CStreamHandleGL3 &handle);

	/// True while the handle may be used: its range still holds what was written
	/// to it, and the stream has not asked for the data to be written elsewhere.
	inline bool isValid(const CStreamHandleGL3 &handle) const
	{
		return handle.Chunk < m_Chunks.size() && m_Chunks[handle.Chunk].Generation == handle.Generation;
	}

	/**
	 * To call for every draw that reads the range of a valid handle: marks the
	 * chunk as used by the current frame and the current draw, which keeps the
	 * stream from writing it again before the GPU has finished that frame.
	 * \return the GL buffer to bind, with the offset and the size of the handle
	 */
	inline GLuint use(const CStreamHandleGL3 &handle)
	{
#ifdef NL_DEBUG
		nlassert(isValid(handle));
#endif
		CChunk &chunk = m_Chunks[handle.Chunk];
		chunk.LastFrame = m_Frame;
		chunk.LastDraw = m_Draw;
		return chunk.Buffer;
	}

	/**
	 * To call before the ranges of a draw are written and used. From then on the
	 * stream may give up the ranges that only the draws before it used, when it
	 * is at its limit; the handles of the new draw are safe until the next call.
	 * A stream that never gets this call gives up nothing of the current frame.
	 */
	inline void nextDraw() { ++m_Draw; }

	/**
	 * To call after every buffer swap, with the new values of the two frame
	 * counters of the driver. Also deletes a chunk that has been unused for long.
	 */
	void frameSwapped(uint64 frame, uint64 frameInFlight);

#if NL3D_GL3_STREAM_STATS
	struct CStats
	{
		uint32 Writes; // ranges reserved, by write() and by reserve()
		uint64 Bytes; // bytes copied by write(), and the sizes of the ranges that were locked
		uint64 Reserved; // bytes the head went forward by: the ranges, the gaps before them for the alignment, and what was left of a chunk when the head moved on
		uint32 Reuses; // chunks written again in place
		uint32 Evacuations; // chunks held by kept handles that had them made stale
		uint32 Discards; // chunks that got new storage because the GPU was not done with them

		CStats()
		    : Writes(0)
		    , Bytes(0)
		    , Reserved(0)
		    , Reuses(0)
		    , Evacuations(0)
		    , Discards(0)
		{
		}
	};

	/// The counts since the previous call
	CStats fetchStats()
	{
		CStats stats = m_Stats;
		m_Stats = CStats();
		return stats;
	}
#endif

private:
	struct CChunk
	{
		GLuint Buffer; // 0 when the chunk has no storage
		uint8 *Mapped; // the whole buffer, mapped for writing as long as the chunk has storage
		uint32 Size;
		uint32 Generation; // changes whenever the handles of the chunk go stale; never 0 once the chunk has storage
		uint64 LastFrame; // last frame that wrote to the chunk or drew from it
		uint64 LastWriteFrame; // last frame that wrote to the chunk
		uint64 LastDraw; // last draw that wrote to the chunk or drew from it, see nextDraw()
		bool Evacuated; // the handles kept on the chunk were made stale, and nothing was written to it since
		uint32 Locks; // ranges of the chunk that are being written through lock()

		CChunk()
		    : Buffer(0)
		    , Mapped(nullptr)
		    , Size(0)
		    , Generation(0)
		    , LastFrame(0)
		    , LastWriteFrame(0)
		    , LastDraw(0)
		    , Evacuated(false)
		    , Locks(0)
		{
		}
	};

	/// Make another chunk the head, for a write of size bytes. False when it has no storage for it.
	bool nextChunk(uint32 size);
	/// Add a chunk behind the head and make it the head
	bool insertChunk(uint32 size);
	/// Make the chunk at a position of the ring the head, empty
	void makeHead(uint pos);
	/// Give a chunk new storage of size bytes, and a new generation
	bool createStorage(CChunk &chunk, uint32 size);
	/// Unmap and delete the GL buffer of a chunk
	void destroyStorage(CChunk &chunk);
	void releaseChunks();
	void nextGeneration(CChunk &chunk);
	/// The chunk can be written again in place: the GPU has finished every frame that used it, and nothing is writing to it
	inline bool isRetired(const CChunk &chunk) const { return !chunk.Locks && (!chunk.Buffer || chunk.LastFrame < m_FrameInFlight); }
	/// Bind a buffer to the target of the stream
	void bindBuffer(GLuint buffer);
	/// Tell the state cache that a buffer of the stream is gone
	void bufferDeleted(GLuint buffer);
	/// True when a failure is to be logged: the first one, and the first one after the flag was cleared
	bool warnFailure(bool &warned);
	/// Mark a chunk as written to by the current frame and the current draw
	void markWritten(CChunk &chunk);

private:
	CDriverGLStates3 *m_States;
	std::string m_Name;
	GLenum m_Target;
	bool m_Initialized;
	bool m_OverLimitWarned;
	// The log has the line about a chunk of the current frame that got new storage at the limit
	bool m_FrameLimitWarned;
	// A failure to create the storage of a chunk has been logged, and no chunk got storage since
	bool m_StorageFailureWarned;

	uint32 m_ChunkSize;
	uint32 m_Alignment;
	uint m_MinChunks;
	uint m_MaxChunks;

	// Handles refer to the slots of this vector. A slot is never removed and its
	// generation only goes up, also over release() and init(), so that a handle
	// never matches a later chunk in the same slot.
	std::vector<CChunk> m_Chunks;
	// Slots of m_Chunks that are not in the ring
	std::vector<uint32> m_FreeSlots;
	// Slots of m_Chunks in the order the chunks get written
	std::vector<uint32> m_Ring;
	// Position in m_Ring of the chunk that is being filled
	uint m_Head;
	// First free byte of the head chunk
	uint32 m_HeadOffset;

	// The frame counters of the driver, as of the last buffer swap
	uint64 m_Frame; // the frame that is being drawn
	uint64 m_FrameInFlight; // the oldest frame that the GPU has not finished
	// Counts the calls of nextDraw()
	uint64 m_Draw;

#if NL3D_GL3_STREAM_STATS
	CStats m_Stats;
#endif
};

} // NLDRIVERGL3
} // NL3D

#endif // NL_DRIVER_OPENGL3_STREAM_BUFFER_H

/* end of file */
