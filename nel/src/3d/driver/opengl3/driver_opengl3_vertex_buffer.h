// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_DRIVER_OPENGL3_VERTEX_BUFFER_H
#define NL_DRIVER_OPENGL3_VERTEX_BUFFER_H

#include "nel/misc/types_nl.h"

#include "driver_opengl3_stream_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

#define NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG 0

// The most GL buffers a CVertexBufferGL3 has. Storage that stays mapped is buffer storage,
// which OpenGL ES 3.0 does not have.
#if NL3D_GL3_PERSISTENT_VERTEX_BUFFERS && !defined(USE_OPENGLES3)
#define NL3D_GL3_VERTEX_BUFFER_RING_MAX NL3D_GL3_PERSISTENT_RING_MAX
#else
#define NL3D_GL3_VERTEX_BUFFER_RING_MAX NL3D_GL3_BUFFER_QUEUE_MAX
#endif

class CDriverGL3;
class IVertexBufferGL3;
class CVertexBufferInfo;
class CVertexBufferGL3;

class IVertexBufferGL3
{
public:
	enum TVBType
	{
		GL3,
		AMDPinned,
		Stream
	};

	IVertexBufferGL3(CDriverGL3 *drv, CVertexBuffer *vb, TVBType vbType);
	virtual	~IVertexBufferGL3();

	virtual	void *lock() = 0;
	virtual	void unlock() = 0;
	virtual void unlock(uint start, uint end) = 0;
	virtual void *getPointer() = 0;
	virtual	void enable() = 0;
	virtual	void disable() = 0;
	virtual GLuint getGLuint() = 0;
	virtual void setFrameInFlight(uint64 swapBufferCounter) = 0;
	virtual void flush() {} // Upload shadow data to GL buffer if dirty

	/// A lock that the caller only reads through, and its unlock. They are lock() and unlock(),
	/// unless a class has less to do for a read (CVertexBufferGL3 with a shadow copy).
	virtual void *lockRead() { return lock(); }
	virtual void unlockRead() { unlock(); }

	/// What glVertexAttribPointer gets for the first byte of the first vertex, with the buffer of getGLuint() bound: the offset of the vertices in that buffer
	virtual void *getArrayPointer() { return getPointer(); }

	// test if buffer content is invalid. If so, no rendering should occurs (rendering should silently fail)
	virtual bool isInvalid() { return m_Invalid; }

public:
	CVertexBuffer *VB;
	TVBType VBType;

protected:
	CDriverGL3 *m_Driver;
	bool m_Invalid;
};

/*
 * GL Core vertex buffer — upload strategy by TBufferUsage:
 *
 *  FullStream / SmallStream, in per-buffer mode only (in stream mode they are
 *  CVertexBufferStreamGL3, below), and FullRewrite in per-buffer mode:
 *    Round-robin ring of NL3D_GL3_BUFFER_QUEUE_MAX GL buffers.
 *    A lock moves on to the next buffer when the current one has been drawn
 *    since its last lock; locks without a draw between them get the same
 *    buffer. If the buffer is still in-flight, the lock orphans it via
 *    MAP_INVALIDATE_BUFFER_BIT, otherwise it maps unsynchronized.  No shadow.
 *    GL hint: GL_STATIC_DRAW.
 *
 *  UnsynchronizedWrite in per-buffer mode:
 *    Single GL buffer, mapped unsynchronized: the caller keeps its writes off
 *    what the GPU still reads.  No shadow.  GL hint: GL_STATIC_DRAW.
 *
 *  FullRewrite / UnsynchronizedWrite in stream mode, with
 *  NL3D_GL3_PERSISTENT_VERTEX_BUFFERS:
 *    Buffer storage that is mapped for writing when it is created, and stays
 *    mapped. A lock returns the memory of the mapping and costs no GL call,
 *    an unlock has nothing to do. GL allows drawing from such a buffer while
 *    it is mapped, and the draws see what was written before them. The memory
 *    is to be written, not read.  No shadow.
 *
 *    UnsynchronizedWrite: one buffer. The caller keeps its writes off what
 *    the GPU still reads.
 *
 *    FullRewrite: a ring that starts with one buffer. A lock without a draw
 *    since the lock before gets the same buffer, with what was written there.
 *    A lock after a draw leaves the buffer to the draws and goes on to the
 *    one that was drawn longest ago, the next of the ring, with what was
 *    written there some locks ago: the caller writes all its vertices. When
 *    the GPU has not finished the frame that drew that one, none of the ring
 *    is free: the ring gets one more buffer, up to
 *    NL3D_GL3_PERSISTENT_RING_MAX. At the limit that buffer is deleted and
 *    made again, which costs the GL calls a map costs. A vertex buffer that
 *    is locked and drawn once per frame ends with two or three buffers.
 *
 *  CpuReadWrite / PartialWrite:
 *    Single GL buffer + CPU shadow (m_ShadowData). Lock/unlock are pure-CPU
 *    operations on the shadow. At draw time, flush() uploads to the GL buffer:
 *
 *    - No dirty ranges (or first upload): full orphan glBufferData.
 *      First upload uses the creation hint (STATIC_DRAW for PartialWrite,
 *      DYNAMIC_DRAW for CpuReadWrite); subsequent full uploads use DYNAMIC_DRAW.
 *
 *    - With dirty ranges, small total (<50%, <=16 ranges): staging-copy pattern.
 *      All coalesced ranges are packed contiguously into a single orphaned
 *      GL staging buffer (GL_COPY_READ_BUFFER, STREAM_DRAW) — orphaning means
 *      GL allocates fresh storage so the CPU never waits. Then
 *      glCopyBufferSubData scatter-copies each range from the packed staging
 *      into the real buffer (GL_COPY_WRITE_BUFFER) entirely on the GPU side,
 *      scheduled in the normal command stream after any pending reads complete.
 *      The real buffer's existing data is preserved and neither side stalls.
 *
 *    - With dirty ranges, large total: falls back to full orphan as above.
 *
 *    Dirty ranges are coalesced (sorted, merged within 128 bytes, aligned
 *    to 64-byte boundaries) before upload.
 *
 *  Immutable:
 *    Single GL buffer. Lock uses glMapBuffer(WRITE_ONLY) — may stall if
 *    the GPU is reading, but Immutable buffers are only locked once at
 *    load time.  No shadow.  GL hint: GL_STATIC_DRAW.
 */
class CVertexBufferGL3 : public IVertexBufferGL3
{
public:
	CVertexBufferGL3(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TBufferUsage preferred, CVertexBuffer *vb);
	virtual	~CVertexBufferGL3() NL_OVERRIDE;

	/// \name Implementation
	// @{
	virtual	void *lock() NL_OVERRIDE;
	virtual	void unlock() NL_OVERRIDE;
	virtual void unlock(uint startVert, uint endVert) NL_OVERRIDE;
	virtual void *getPointer() NL_OVERRIDE;
	virtual	void enable() NL_OVERRIDE;
	virtual	void disable() NL_OVERRIDE;
	virtual GLuint getGLuint() NL_OVERRIDE;
	virtual void setFrameInFlight(uint64 swapBufferCounter) NL_OVERRIDE;
	virtual void flush() NL_OVERRIDE;
	virtual void *lockRead() NL_OVERRIDE;
	virtual void unlockRead() NL_OVERRIDE;
	virtual void *getArrayPointer() NL_OVERRIDE;
	// @}

	/// Invalidate the buffer (when it is lost, or when a lock fails)
	void invalidate();

private:
	friend class CDriverGL3;

	/// Deletes the GL buffer of a place in the ring, if it has one
	void deleteBuffer(GLsizei slot);
	/// Deletes all the GL buffers
	void deleteBuffers();
#if NL3D_GL3_PERSISTENT_VERTEX_BUFFERS && !defined(USE_OPENGLES3)
	/// Gives a place in the ring a GL buffer with storage that stays mapped. False when GL has none: the place is then without a buffer
	bool createPersistentBuffer(GLsizei slot);
	/// The lock of a vertex buffer with such storage
	void *lockPersistent();
#endif

	CVertexBuffer::TBufferUsage m_MemType;
	void *m_VertexPtr; // pointer on current datas. Null if not locked
	uint m_Size; // The bytes of each GL buffer: those of the capacity of the vertex buffer when this object was made

	// The storage of the GL buffers stays mapped (see the comment of the class)
	bool m_Persistent;
	// The memory of each of them then. Null where there is no buffer
	void *m_Mapping[NL3D_GL3_VERTEX_BUFFER_RING_MAX];

	// Shadow buffer for CpuReadWrite/PartialWrite: CPU reads/writes go here, uploaded to GL at draw time
	std::vector<uint8> m_ShadowData;
	bool m_ShadowDirty;
	bool m_InitialUploadDone; // First flush uses creation hint, subsequent full flushes use GL_DYNAMIC_DRAW

	// A lock for reading of the shadow copy (lockRead): what tells at the unlock that it was written all the same
	size_t m_ReadLockDirtyRanges; // The number of dirty ranges of the vertex buffer at the lock
	uint32 m_ReadLockSum; // With NL3D_GL3_STATE_CHECKS: a sum over the shadow copy at the lock
	bool m_ReadLockWriteWarned; // The log has the line about a write under a lock for reading

	// Scratch buffer for coalescing dirty ranges during flush (avoids reallocation)
	std::vector<CVertexBuffer::CDirtyRange> m_MergedRanges;

	// Staging GL buffer for stall-free partial uploads via CopyBufferSubData
	GLuint m_StagingBufferId;

	// if buffer has been invalidated, returns a dummy memory block and silently fails rendering
	std::vector<uint8> m_DummyVB;
	// for use by CVertexArrayRange
	std::list<CVertexBufferGL3*>::iterator m_IteratorInLostVBList;

	// The ring of GL buffers: the places below m_RingSize are in use
	GLuint m_VertexObjectId[NL3D_GL3_VERTEX_BUFFER_RING_MAX];
	uint64 m_FrameInFlight[NL3D_GL3_VERTEX_BUFFER_RING_MAX]; // The last frame that drew from each
	GLsizei m_RingSize;
	GLsizei m_CurrentIndex; // The one that the locks write and the draws read
	bool m_CurrentInFlight; // It has been drawn since its last lock

#if NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG
	uint32 m_ReuseCount;
	uint32 m_InvalidateCount;
#endif
};

class CVertexBufferAMDPinned : public IVertexBufferGL3
{
public:
	CVertexBufferAMDPinned(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TBufferUsage preferred, CVertexBuffer *vb);
	virtual	~CVertexBufferAMDPinned() NL_OVERRIDE;

	/// \name Implementation
	// @{
	virtual	void *lock() NL_OVERRIDE;
	virtual	void unlock() NL_OVERRIDE;
	virtual void unlock(uint startVert, uint endVert) NL_OVERRIDE;
	virtual void *getPointer() NL_OVERRIDE;
	virtual	void enable() NL_OVERRIDE;
	virtual	void disable() NL_OVERRIDE;
	virtual GLuint getGLuint() NL_OVERRIDE;
	virtual void setFrameInFlight(uint64 swapBufferCounter) NL_OVERRIDE;
	// @}

private:
	CVertexBuffer::TBufferUsage m_MemType;
	void *m_VertexPtrAllocated;
	void *m_VertexPtrAligned;
	void *m_VertexPtr;
	uint m_VertexObjectId;

	uint64 m_FrameInFlight;
};

/*
 * FullStream / SmallStream vertex buffer in stream mode: the vertices are in a
 * range of the vertex stream of the driver (CStreamBufferGL3), the object has
 * no GL buffer of its own.
 *
 *  Lock: when the vertices have been drawn since the range was reserved, when
 *    the stream has given the range to other data, or when the vertex buffer
 *    has more vertices than the range has room for, the lock reserves a new
 *    range of getNumVertices() * getVertexSize() bytes, with undefined
 *    contents, and returns its memory. Else, for locks without a draw between
 *    them, it returns the same memory with what was written before.
 *    The memory is that of the GL buffer, mapped for writing only: it is to be
 *    written, not read. A lock costs no GL call.
 *
 *  Draw: activeVertexBuffer sets the attribute pointers to the offset of the
 *    range in the buffer of its chunk, and every draw marks the chunk as used
 *    by the frame. A lock between activeVertexBuffer and a draw that moves the
 *    vertex buffer to a new range is not seen by that draw.
 *
 *  Contents: the stream gives a chunk to other data once the frames that wrote
 *    it have finished on the GPU. A vertex buffer of these usages is to be
 *    filled in every frame it is drawn in; one with local memory is filled
 *    from it at every activeVertexBuffer. A vertex buffer that is drawn with
 *    vertices that are gone is not drawn, and says so in the log once.
 */
class CVertexBufferStreamGL3 : public IVertexBufferGL3
{
public:
	CVertexBufferStreamGL3(CDriverGL3 *drv, CVertexBuffer *vb);
	virtual ~CVertexBufferStreamGL3() NL_OVERRIDE;

	/// \name Implementation
	// @{
	virtual void *lock() NL_OVERRIDE;
	virtual void unlock() NL_OVERRIDE;
	virtual void unlock(uint startVert, uint endVert) NL_OVERRIDE;
	virtual void *getPointer() NL_OVERRIDE;
	virtual void enable() NL_OVERRIDE;
	virtual void disable() NL_OVERRIDE;
	virtual GLuint getGLuint() NL_OVERRIDE;
	virtual void setFrameInFlight(uint64 swapBufferCounter) NL_OVERRIDE;
	virtual void *getArrayPointer() NL_OVERRIDE;
	virtual bool isInvalid() NL_OVERRIDE;
	// @}

private:
	CStreamHandleGL3 m_Handle; // the range that has the vertices
	uint8 *m_VertexPtr; // what lock() returned; null when not locked
	bool m_RangeLocked; // the lock is one of the range, and not of m_DummyVB
	bool m_Drawn; // the range has been drawn since it was reserved
	bool m_LostWarned; // the log has the line about vertices that are gone

	// What a lock returns when the stream has no range for it. The vertices written there are not drawn.
	std::vector<uint8> m_DummyVB;
};

} // NLDRIVERGL3
} // NL3D

#endif // NL_DRIVER_OPENGL3_VERTEX_BUFFER_H

/* End of driver_opengl_vertex_buffer.h */
