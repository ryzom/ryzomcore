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

#include "stdopengl3.h"

#include "driver_opengl3.h"
#include "driver_opengl3_vertex_buffer.h"

#include "nel/3d/vertex_buffer.h"

using	namespace std;
using	namespace NLMISC;

namespace NL3D {

namespace {
static bool dirtyRangeLess(const CVertexBuffer::CDirtyRange &a, const CVertexBuffer::CDirtyRange &b)
{
	return a.Begin < b.Begin;
}
} // anonymous namespace
namespace NLDRIVERGL3 {

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

IVertexBufferGL3::IVertexBufferGL3(CDriverGL3 *drv, CVertexBuffer *vb, TVBType vbType) 
	: VB(vb), VBType(vbType), m_Driver(drv), m_Invalid(false)
{
	H_AUTO_OGL(IVertexBufferGL_IVertexBufferGL)
}

// ***************************************************************************

IVertexBufferGL3::~IVertexBufferGL3()
{
	H_AUTO_OGL(IVertexBufferGL_IVertexBufferGLDtor)
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

static inline GLsizei vbgl3BufferForType(CVertexBuffer::TBufferUsage mem)
{
	switch (mem)
	{
	case CVertexBuffer::FullStream:
	case CVertexBuffer::SmallStream:
	case CVertexBuffer::FullRewrite:
		return NL3D_GL3_BUFFER_QUEUE_MAX;
	default:
		return 1;
	}
}

CVertexBufferGL3::CVertexBufferGL3(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TBufferUsage preferred, CVertexBuffer *vb)
	: IVertexBufferGL3(drv, vb, IVertexBufferGL3::GL3),
	m_VertexPtr(nullptr)
    ,
	m_ShadowDirty(false),
	m_InitialUploadDone(false),
	m_CurrentIndex(0),
	m_CurrentInFlight(false),
#if NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG
	m_ReuseCount(0),
	m_InvalidateCount(0),
#endif
	m_MemType(preferred),
	m_StagingBufferId(0)
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARB);

	m_ReadLockDirtyRanges = 0;
	m_ReadLockSum = 0;
	m_ReadLockWriteWarned = false;
	m_Size = size;
	m_Persistent = false;

	// Allocate shadow buffer for CpuReadWrite and PartialWrite (CPU reads/writes go here)
	if (preferred == CVertexBuffer::CpuReadWrite || preferred == CVertexBuffer::PartialWrite)
		m_ShadowData.resize(size, 0);

	for (GLsizei i = 0; i < NL3D_GL3_VERTEX_BUFFER_RING_MAX; ++i)
	{
		m_VertexObjectId[i] = 0;
		m_FrameInFlight[i] = NL3D_GL3_BUFFER_NOT_IN_FLIGHT;
		m_Mapping[i] = nullptr;
	}
	m_RingSize = vbgl3BufferForType(preferred);

#if NL3D_GL3_PERSISTENT_VERTEX_BUFFERS && !defined(USE_OPENGLES3)
	// Stream mode has buffer storage. A vertex buffer that is written where it is drawn
	// from gets storage that stays mapped: one buffer, to which the locks of the
	// FullRewrite usage add the others of the ring as they need them.
	if (drv->_StreamMode && (preferred == CVertexBuffer::FullRewrite || preferred == CVertexBuffer::UnsynchronizedWrite))
	{
		m_Persistent = true;
		m_RingSize = 1;
		if (!createPersistentBuffer(0))
			invalidate(); // nothing is drawn, and the vertex buffer is set up again after the frame
		return;
	}
#endif

	// Create ids
	nglGenBuffers(m_RingSize, m_VertexObjectId);

	// Initialize
	for (GLsizei i = 0; i < m_RingSize; ++i)
	{
		drv->_DriverGLStates.forceBindArrayBuffer(m_VertexObjectId[i]);
		nglBufferData(GL_ARRAY_BUFFER, size, nullptr, drv->vertexBufferUsageGL3(preferred));
	}
}

// ***************************************************************************

CVertexBufferGL3::~CVertexBufferGL3()
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARBDtor)
	deleteBuffers();
	if (m_StagingBufferId)
	{
		nglDeleteBuffers(1, &m_StagingBufferId);
	}
	if (m_Driver)
	{
		if (m_Invalid)
		{
			if (VB->getLocation() != CVertexBuffer::NotResident)
			{
				// when the vb is put in tthe NotResident state, it is removed from that list
				m_Driver->_LostVBList.erase(m_IteratorInLostVBList);
			}
		}
	}
}

// ***************************************************************************

void *CVertexBufferGL3::lock()
{
	H_AUTO_OGL(CVertexBufferGLARB_lock);

	if (m_VertexPtr) return m_VertexPtr; // already locked :|

	const uint size = VB->getNumVertices() * VB->getVertexSize();

	if (m_Invalid)
	{
		if (VB->getLocation() != CVertexBuffer::NotResident)
		{
			// Not yet resident — shadow is always valid if available
			if (!m_ShadowData.empty())
			{
				m_VertexPtr = &m_ShadowData[0];
				return m_VertexPtr;
			}
			nlassert(!m_DummyVB.empty());
			return &m_DummyVB[0];
		}

#if NL3D_GL3_PERSISTENT_VERTEX_BUFFERS && !defined(USE_OPENGLES3)
		if (m_Persistent)
		{
			// The first buffer of the ring again, with storage that stays mapped
			m_RingSize = 1;
			m_CurrentIndex = 0;
			m_CurrentInFlight = false;
			if (!createPersistentBuffer(0))
			{
				m_Driver->incrementResetCounter();
				return &m_DummyVB[0];
			}
			NLMISC::contReset(m_DummyVB);
			m_Invalid = false;
			m_Driver->_LostVBList.erase(m_IteratorInLostVBList);
			return lockPersistent();
		}
#endif

		// Create ids
		glGetError();
		GLsizei nbBuff = vbgl3BufferForType(m_MemType);
		nglGenBuffers(nbBuff, m_VertexObjectId);

		if (glGetError() != GL_NO_ERROR)
		{
			m_Driver->incrementResetCounter();
			if (!m_ShadowData.empty())
			{
				m_VertexPtr = &m_ShadowData[0];
				return m_VertexPtr;
			}
			return &m_DummyVB[0];
		}

		for (GLsizei i = 0; i < nbBuff; ++i)
		{
			m_Driver->_DriverGLStates.forceBindArrayBuffer(m_VertexObjectId[i]);
			nglBufferData(GL_ARRAY_BUFFER, size, nullptr, m_Driver->vertexBufferUsageGL3(m_MemType));
			if (glGetError() != GL_NO_ERROR)
			{
				m_Driver->incrementResetCounter();
				for (GLsizei j = 0; j < nbBuff; ++j)
					m_Driver->_DriverGLStates.arrayBufferDeleted(m_VertexObjectId[j]);
				nglDeleteBuffers(nbBuff, m_VertexObjectId);
				for (GLsizei j = 0; j < nbBuff; ++j)
					m_VertexObjectId[j] = 0;
				if (!m_ShadowData.empty())
				{
					m_VertexPtr = &m_ShadowData[0];
					return m_VertexPtr;
				}
				return &m_DummyVB[0];
			}
		}
		NLMISC::contReset(m_DummyVB); // free vector memory for real
		nlassert(m_VertexObjectId[m_CurrentIndex]);

		m_Invalid = false;
		m_Driver->_LostVBList.erase(m_IteratorInLostVBList);
		// Shadow needs re-upload to the new GL buffer
		if (!m_ShadowData.empty())
			m_ShadowDirty = true;
		// continue to standard mapping code below ..
	}

	// Shadow buffer fast path: no GL interaction needed
	if (!m_ShadowData.empty())
	{
		m_VertexPtr = &m_ShadowData[0];
		return m_VertexPtr;
	}

#if NL3D_GL3_PERSISTENT_VERTEX_BUFFERS && !defined(USE_OPENGLES3)
	// Storage that stays mapped: no GL interaction needed either
	if (m_Persistent)
		return lockPersistent();
#endif

	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}

#if NL3D_GL3_STREAM_STATS
	++m_Driver->_StreamStats.VertexLocksMapped[m_MemType];
#endif

	// Invalidate when updating volatile buffers, framerate from 24fps to 38fps in reference test on AMD platform
	switch (m_MemType)
	{
	case CVertexBuffer::FullStream:
	case CVertexBuffer::SmallStream:
	case CVertexBuffer::FullRewrite:
	{
		if (m_CurrentInFlight)
		{
			++m_CurrentIndex;
			m_CurrentIndex %= NL3D_GL3_BUFFER_QUEUE_MAX;
			m_CurrentInFlight = false;
		}
		m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId[m_CurrentIndex]);
		if (m_FrameInFlight[m_CurrentIndex] != NL3D_GL3_BUFFER_NOT_IN_FLIGHT
			&& m_FrameInFlight[m_CurrentIndex] >= m_Driver->getSwapBufferInFlight())
		{
#if NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG
			++m_InvalidateCount;
			nldebug("GL: Vertex buffer already in flight (reused: %u, invalidated: %u)", m_ReuseCount, m_InvalidateCount);
#endif
			// NOTE: GL_MAP_INVALIDATE_BUFFER_BIT removes the cost of waiting for synchronization (major performance impact),
			// but adds the cost of allocating a new buffer (which hast a much lower performance impact)
			m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		}
		else
		{
#if NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG
			++m_ReuseCount;
			nldebug("GL: Vertex buffer can be reused (reused: %u, invalidated: %u)", m_ReuseCount, m_InvalidateCount);
#endif
#ifdef __EMSCRIPTEN__
			// WebGL 2.0 only supports MAP_WRITE|INVALIDATE_BUFFER for glMapBufferRange
			m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
#else
			m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
#endif
		}
		break;
	}
	case CVertexBuffer::UnsynchronizedWrite:
		m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId[m_CurrentIndex]);
		m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		break;
	// case CVertexBuffer::FullRewrite:
	// 	m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId[m_CurrentIndex]);
	// 	m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	// 	break;
	default:
		m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId[m_CurrentIndex]);
#ifdef USE_OPENGLES3
		m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
#else
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
#endif
		break;
	}

	if (!m_VertexPtr)
	{
		nlassert(nglIsBuffer(m_VertexObjectId[m_CurrentIndex]));
		invalidate();
		return &m_DummyVB[0];
	}

	#ifdef NL_DEBUG
		// Vertex array range tracking removed in GL3 driver
	#endif
	// The buffer stays bound: the unlock binds it, and nothing reads the binding in between
	// Lock Profile?
	if (m_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		m_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}
	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferGL3::unlock()
{
	H_AUTO_OGL(CVertexBufferGLARB_unlock);

	m_VertexPtr = nullptr;
	if (m_Invalid) return;

	// Shadow buffer: just mark dirty, no GL interaction
	if (!m_ShadowData.empty())
	{
		m_ShadowDirty = true;
		return;
	}

	// Storage that stays mapped: the draws that follow see what was written
	if (m_Persistent) return;

	if (!m_VertexObjectId[m_CurrentIndex]) return;
	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId[m_CurrentIndex]);
	GLboolean unmapOk = GL_FALSE;

	unmapOk = nglUnmapBuffer(GL_ARRAY_BUFFER);
	// Lock Profile?
	if (m_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		m_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}
	if (!unmapOk)
	{
		invalidate();
	}
}

// ***************************************************************************

void		*CVertexBufferGL3::getPointer()
{
	H_AUTO_OGL(CVertexBufferGLARB_getPointer)
	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferGL3::unlock(uint /* startVert */,uint /* endVert */)
{
	H_AUTO_OGL(CVertexBufferGLARB_unlock)
	unlock(); // can't do a lock on a range of the vb..
}

// ***************************************************************************

void *CVertexBufferGL3::getArrayPointer()
{
	H_AUTO_OGL(CVertexBufferGL3_getArrayPointer);

	// The vertices start at the first byte of the GL buffer, also while a lock is held:
	// a buffer whose storage stays mapped can be drawn from then
	return nullptr;
}

// ***************************************************************************

void CVertexBufferGL3::deleteBuffer(GLsizei slot)
{
	H_AUTO_OGL(CVertexBufferGL3_deleteBuffer);

	GLuint id = m_VertexObjectId[slot];
	if (!id)
		return;

	nlassert(nglIsBuffer(id));
	if (m_Driver)
	{
		if (m_Mapping[slot])
		{
			m_Driver->_DriverGLStates.bindArrayBuffer(id);
			nglUnmapBuffer(GL_ARRAY_BUFFER);
		}
		// GL takes the buffer out of its bindings, the state cache follows
		m_Driver->_DriverGLStates.arrayBufferDeleted(id);
	}
	// GL keeps the storage for the draws that still read it
	nglDeleteBuffers(1, &id);
	m_VertexObjectId[slot] = 0;
	m_Mapping[slot] = nullptr;
}

// ***************************************************************************

void CVertexBufferGL3::deleteBuffers()
{
	H_AUTO_OGL(CVertexBufferGL3_deleteBuffers);

	for (GLsizei i = 0; i < NL3D_GL3_VERTEX_BUFFER_RING_MAX; ++i)
		deleteBuffer(i);
}

// ***************************************************************************

#if NL3D_GL3_PERSISTENT_VERTEX_BUFFERS && !defined(USE_OPENGLES3)

bool CVertexBufferGL3::createPersistentBuffer(GLsizei slot)
{
	H_AUTO_OGL(CVertexBufferGL3_createPersistentBuffer);

	nlassert(!m_VertexObjectId[slot]);

	nglGenBuffers(1, &m_VertexObjectId[slot]);
	m_Driver->_DriverGLStates.forceBindArrayBuffer(m_VertexObjectId[slot]);
	// Persistent: GL allows drawing from the buffer while it is mapped. Coherent: what
	// is written through the mapping is seen by the draws that follow, without a flush.
	const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	nglBufferStorage(GL_ARRAY_BUFFER, m_Size, nullptr, flags);
	m_Mapping[slot] = nglMapBufferRange(GL_ARRAY_BUFFER, 0, m_Size, flags);
	m_FrameInFlight[slot] = NL3D_GL3_BUFFER_NOT_IN_FLIGHT;
	if (!m_Mapping[slot])
	{
		if (!m_Driver->_VertexStorageFailureWarned)
		{
			m_Driver->_VertexStorageFailureWarned = true;
			nlwarning("3D: cannot create and map a buffer of %u bytes with glBufferStorage for the vertex buffer '%s'. "
			          "It is not drawn until it gets one",
			    m_Size, VB->getName().c_str());
		}
		deleteBuffer(slot);
		return false;
	}
	m_Driver->_VertexStorageFailureWarned = false;
	return true;
}

// ***************************************************************************

void *CVertexBufferGL3::lockPersistent()
{
	H_AUTO_OGL(CVertexBufferGL3_lockPersistent);

	if (m_MemType == CVertexBuffer::FullRewrite && m_CurrentInFlight)
	{
		// The vertices have been drawn since the last lock: the buffer is left to those
		// draws. The caller writes all its vertices again, into the buffer of the ring
		// that was drawn longest ago: the one after the current (in a ring of one, the
		// current one itself).
		GLsizei next = (m_CurrentIndex + 1) % m_RingSize;
		if (m_FrameInFlight[next] != NL3D_GL3_BUFFER_NOT_IN_FLIGHT
		    && m_FrameInFlight[next] >= m_Driver->getSwapBufferInFlight())
		{
			// The GPU has not finished the frame that drew it last, and then none of the ring is free
			if (m_RingSize < NL3D_GL3_VERTEX_BUFFER_RING_MAX)
			{
				// One more buffer, behind the current one
				next = m_CurrentIndex + 1;
				for (GLsizei i = m_RingSize; i > next; --i)
				{
					m_VertexObjectId[i] = m_VertexObjectId[i - 1];
					m_FrameInFlight[i] = m_FrameInFlight[i - 1];
					m_Mapping[i] = m_Mapping[i - 1];
				}
				++m_RingSize;
				m_VertexObjectId[next] = 0;
				m_Mapping[next] = nullptr;
#if NL3D_GL3_STREAM_STATS
				++m_Driver->_StreamStats.VertexRingAdds;
#endif
			}
			else
			{
				// The ring is at its limit: that buffer is made again, with new storage
				deleteBuffer(next);
#if NL3D_GL3_STREAM_STATS
				++m_Driver->_StreamStats.VertexRingDiscards;
#endif
			}
			if (!createPersistentBuffer(next))
			{
				// As a map that fails: nothing is drawn, and the vertex buffer is set up again after the frame
				m_CurrentIndex = next;
				invalidate();
				return &m_DummyVB[0];
			}
		}
		m_CurrentIndex = next;
		m_FrameInFlight[next] = NL3D_GL3_BUFFER_NOT_IN_FLIGHT;
		m_CurrentInFlight = false;
	}

#if NL3D_GL3_STREAM_STATS
	++m_Driver->_StreamStats.VertexLocksPersistent;
#endif
	m_VertexPtr = m_Mapping[m_CurrentIndex];
	return m_VertexPtr;
}

#endif

// ***************************************************************************

#if NL3D_GL3_STATE_CHECKS
namespace /* anonymous */ {

// A sum over memory that changes when a byte of it does, for all but contrived changes
uint32 sumOfBytes(const std::vector<uint8> &bytes)
{
	uint32 sum = 0;
	const size_t words = bytes.size() / sizeof(uint32);
	for (size_t i = 0; i < words; ++i)
	{
		uint32 word;
		memcpy(&word, &bytes[i * sizeof(uint32)], sizeof(uint32));
		sum = ((sum << 5) | (sum >> 27)) ^ word;
	}
	for (size_t i = words * sizeof(uint32); i < bytes.size(); ++i)
		sum = ((sum << 5) | (sum >> 27)) ^ bytes[i];
	return sum;
}

} /* anonymous namespace */
#endif

void *CVertexBufferGL3::lockRead()
{
	H_AUTO_OGL(CVertexBufferGL3_lockRead);

	void *ptr = lock();

	// A lock of the shadow copy for reading leaves the copy as it is, so the unlock has
	// nothing to give to GL. The driver is told of the outermost lock of a vertex buffer
	// only: a lock for writing that is taken inside this one is not seen. What tells of
	// it is noted here, for the unlock.
	if (!m_ShadowData.empty())
	{
		m_ReadLockDirtyRanges = VB->getDirtyRanges().size();
#if NL3D_GL3_STATE_CHECKS
		m_ReadLockSum = sumOfBytes(m_ShadowData);
#endif
	}

	return ptr;
}

// ***************************************************************************

void CVertexBufferGL3::unlockRead()
{
	H_AUTO_OGL(CVertexBufferGL3_unlockRead);

	// Without a shadow copy the lock was a map of the GL buffer, to be undone as any other
	if (m_ShadowData.empty())
	{
		unlock();
		return;
	}

	// The shadow copy was written under the lock for reading after all, when a range of
	// the vertex buffer was reported as changed meanwhile
	bool written = VB->getDirtyRanges().size() != m_ReadLockDirtyRanges;
#if NL3D_GL3_STATE_CHECKS
	if (!written && sumOfBytes(m_ShadowData) != m_ReadLockSum)
	{
		written = true;
		if (!m_ReadLockWriteWarned)
		{
			m_ReadLockWriteWarned = true;
			nlwarning("3D: the vertex buffer '%s' was written under a lock for reading. The driver is told of the outermost lock only: "
			          "without NL3D_GL3_STATE_CHECKS such a write is not drawn before the next lock for writing",
			    VB->getName().c_str());
		}
	}
#endif
	if (written)
	{
		// as a lock for writing
		unlock();
		return;
	}

	// Nothing to upload: what m_ShadowDirty and the dirty ranges say of earlier writes, and
	// of an upload that is still to come (the first one, the one after a lost buffer), stays
	m_VertexPtr = nullptr;
}

// ***************************************************************************

void CVertexBufferGL3::enable()
{
	H_AUTO_OGL(CVertexBufferGLARB_enable)
	if (m_Driver->_CurrentVertexBufferGL != this)
	{
		m_Driver->_CurrentVertexBufferGL = this;
	}
}

// ***************************************************************************

void CVertexBufferGL3::disable()
{
	H_AUTO_OGL(CVertexBufferGLARB_disable)
	// (a vertex buffer that dies while another one is the active one leaves that one active)
	if (m_Driver->_CurrentVertexBufferGL == this)
	{
		m_Driver->_CurrentVertexBufferGL = nullptr;
	}
}

// ***************************************************************************

GLuint CVertexBufferGL3::getGLuint()
{
	H_AUTO_OGL(CVertexBufferGLARB_getGLuint);

	return m_VertexObjectId[m_CurrentIndex];
}

// ***************************************************************************

void CVertexBufferGL3::setFrameInFlight(uint64 swapBufferCounter)
{
	H_AUTO_OGL(CVertexBufferGL3_setFrameInFlight);

	// Set buffer frame in flight
	m_FrameInFlight[m_CurrentIndex] = swapBufferCounter;
	m_CurrentInFlight = true;
}

// ***************************************************************************

void CVertexBufferGL3::flush()
{
	H_AUTO_OGL(CVertexBufferGL3_flush);

	if (!m_ShadowDirty) return;
	if (m_Invalid) return;

	const uint size = VB->getNumVertices() * VB->getVertexSize();

	// First full upload preserves creation hint (e.g. GL_STATIC_DRAW for PartialWrite),
	// subsequent full orphan uploads use GL_DYNAMIC_DRAW since we're doing repeated full rewrites.
	const bool firstUpload = !m_InitialUploadDone;
	const GLenum fullUploadHint = firstUpload
		? m_Driver->vertexBufferUsageGL3(m_MemType)
		: GL_DYNAMIC_DRAW;
	m_InitialUploadDone = true;

	const std::vector<CVertexBuffer::CDirtyRange> &ranges = VB->getDirtyRanges();
	if (ranges.empty())
	{
		// No explicit dirty ranges: full orphan+upload
		m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId[m_CurrentIndex]);
		nglBufferData(GL_ARRAY_BUFFER, size, &m_ShadowData[0], fullUploadHint);
#if NL3D_GL3_STREAM_STATS
		++m_Driver->_StreamStats.ShadowUploads;
		m_Driver->_StreamStats.ShadowUploadBytes += size;
#endif
	}
	else
	{
		// Copy into reusable scratch vector (avoids per-flush allocation)
		m_MergedRanges.resize(ranges.size());
		memcpy(&m_MergedRanges[0], &ranges[0], ranges.size() * sizeof(CVertexBuffer::CDirtyRange));

		// Sort by Begin offset
		std::sort(m_MergedRanges.begin(), m_MergedRanges.end(), dirtyRangeLess);

		// Merge overlapping/adjacent and ranges within 128 bytes of each other
		uint writeIdx = 0;
		for (uint i = 1; i < m_MergedRanges.size(); ++i)
		{
			if (m_MergedRanges[i].Begin <= m_MergedRanges[writeIdx].End + 128)
			{
				if (m_MergedRanges[i].End > m_MergedRanges[writeIdx].End)
					m_MergedRanges[writeIdx].End = m_MergedRanges[i].End;
			}
			else
			{
				++writeIdx;
				m_MergedRanges[writeIdx] = m_MergedRanges[i];
			}
		}
		m_MergedRanges.resize(writeIdx + 1);

		// Align: round Begin down to 64 bytes, round End up to 64 bytes, clamp to buffer
		for (uint i = 0; i < m_MergedRanges.size(); ++i)
		{
			m_MergedRanges[i].Begin = m_MergedRanges[i].Begin & ~(uint32)63;
			m_MergedRanges[i].End = std::min((m_MergedRanges[i].End + 63) & ~(uint32)63, (uint32)size);
			if (m_MergedRanges[i].Begin >= size) { m_MergedRanges.resize(i); break; }
		}

		// Sum total dirty bytes
		uint totalDirty = 0;
		for (uint i = 0; i < m_MergedRanges.size(); ++i)
			totalDirty += m_MergedRanges[i].End - m_MergedRanges[i].Begin;

		if (firstUpload && (totalDirty >= size / 2 || m_MergedRanges.size() > 16))
		{
			// Too much dirty: full orphan+upload from CPU, data persists for many frames
			m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId[m_CurrentIndex]);
			nglBufferData(GL_ARRAY_BUFFER, size, &m_ShadowData[0], fullUploadHint);
#if NL3D_GL3_STREAM_STATS
			++m_Driver->_StreamStats.ShadowUploads;
			m_Driver->_StreamStats.ShadowUploadBytes += size;
#endif
		}
		else
		{
			// Staging buffer pattern: pack all dirty ranges into one orphaned
			// staging buffer, then scatter-copy GL-side into the real buffer.
			// Single allocation + single orphan; neither CPU nor GPU stalls.
			if (!m_StagingBufferId)
				nglGenBuffers(1, &m_StagingBufferId);

			nglBindBuffer(GL_COPY_READ_BUFFER, m_StagingBufferId);
			nglBindBuffer(GL_COPY_WRITE_BUFFER, m_VertexObjectId[m_CurrentIndex]);

			// Orphan staging once and pack all ranges contiguously
			nglBufferData(GL_COPY_READ_BUFFER, totalDirty, nullptr, GL_STREAM_DRAW);

			uint32 packOffset = 0;
			for (uint i = 0; i < m_MergedRanges.size(); ++i)
			{
				uint32 rangeBegin = m_MergedRanges[i].Begin;
				uint32 rangeSize = m_MergedRanges[i].End - rangeBegin;
				nglBufferSubData(GL_COPY_READ_BUFFER, packOffset, rangeSize,
					&m_ShadowData[rangeBegin]);
				packOffset += rangeSize;
			}

			// Scatter-copy from packed staging into destination
			packOffset = 0;
			for (uint i = 0; i < m_MergedRanges.size(); ++i)
			{
				uint32 rangeBegin = m_MergedRanges[i].Begin;
				uint32 rangeSize = m_MergedRanges[i].End - rangeBegin;
				nglCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
					packOffset, rangeBegin, rangeSize);
				packOffset += rangeSize;
			}

			nglBindBuffer(GL_COPY_READ_BUFFER, 0);
			nglBindBuffer(GL_COPY_WRITE_BUFFER, 0);
#if NL3D_GL3_STREAM_STATS
			++m_Driver->_StreamStats.ShadowUploads;
			m_Driver->_StreamStats.ShadowUploadBytes += totalDirty;
#endif
		}
	}

	m_ShadowDirty = false;
	VB->clearDirtyRanges();
}

// ***************************************************************************

void CVertexBufferGL3::invalidate()
{
	H_AUTO_OGL(CVertexBufferGLARB_invalidate)
	nlassert(!m_Invalid);
	// Buffer is lost (maybe there was a alt-tab or fullscrren / windowed change)
	// Buffer is deleted at end of frame only
	m_Invalid = true;
	m_Driver->incrementResetCounter();
	// As large as the GL buffers: the engine writes up to the capacity of the vertex buffer through a lock
	m_DummyVB.resize(m_Size, 0);
	// insert in lost vb list
	m_Driver->_LostVBList.push_front(this);
	m_IteratorInLostVBList = m_Driver->_LostVBList.begin();
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

CVertexBufferAMDPinned::CVertexBufferAMDPinned(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TBufferUsage preferred, CVertexBuffer *vb) 
	: IVertexBufferGL3(drv, vb, IVertexBufferGL3::AMDPinned),
	m_MemType(preferred),
	m_VertexPtr(nullptr)
    ,
	m_VertexObjectId(0),
	m_FrameInFlight(NL3D_GL3_BUFFER_NOT_IN_FLIGHT)
{
	H_AUTO_OGL(CVertexBufferAMDPinned_CVertexBufferAMDPinned)

	// Create id and bind
	GLuint vertexBufferID;
	nglGenBuffers(1, &vertexBufferID);
	nglBindBuffer(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, vertexBufferID);

	// Initialize
	// Align allocated to page size, which is assumed to be 4K
	// nldebug("GL3: Allocate AMD Pinned Memory (%i bytes, %i vertices)", size, numVertices);
	m_VertexPtrAllocated = new char[size + 4096];
	uintptr_t addr = (uintptr_t)m_VertexPtrAllocated;
	addr = (addr + 4095) & (~0xfff);
	m_VertexPtrAligned = (void *)addr;
	nglBufferData(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, size, m_VertexPtrAligned, drv->vertexBufferUsageGL3(preferred));
	if (glGetError() == GL_INVALID_OPERATION)
	{
		nlerror("Failed to pin memory");
		nglDeleteBuffers(1, &vertexBufferID);
		vertexBufferID = 0;
	}
	m_VertexObjectId = vertexBufferID;

	// Unbind
	nglBindBuffer(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, 0);
}

// ***************************************************************************

CVertexBufferAMDPinned::~CVertexBufferAMDPinned()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_CVertexBufferAMDPinnedDtor)
	if (m_VertexObjectId)
	{
		GLuint id = (GLuint)m_VertexObjectId;
		nlassert(nglIsBuffer(id));
		// GL takes the buffer out of its bindings, the state cache follows
		if (m_Driver)
			m_Driver->_DriverGLStates.arrayBufferDeleted(id);
		nglDeleteBuffers(1, &id);
	}
	delete[] static_cast<char *>(m_VertexPtrAllocated);
	m_VertexPtrAllocated = nullptr;
	m_VertexPtrAligned = nullptr;
	nlassert(m_VertexPtr == NULL);
}

// ***************************************************************************

void *CVertexBufferAMDPinned::lock()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_lock);

	if (m_VertexPtr) // Already locked...
		return m_VertexPtr;

	if (!m_VertexObjectId) // Failed to pin
		return m_VertexPtrAligned;

	// Profiling
	TTicks beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}

	// Lock
	m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId);
	switch (m_MemType)
	{
	case CVertexBuffer::FullStream:
	case CVertexBuffer::SmallStream:
		nlerror("Volatile currently not supported by pinned memory, this would require a re-allocating RAM, and thus require a fast allocation mechanism");
		m_VertexPtr = nullptr;
		break;
	case CVertexBuffer::CpuReadWrite:
#ifdef USE_OPENGLES3
		m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, VB->getNumVertices() * VB->getVertexSize(), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
#else
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
#endif
		break;
	default:
#ifdef USE_OPENGLES3
		m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, VB->getNumVertices() * VB->getVertexSize(), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
#else
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
#endif
		break;
	}
	nlassert(m_VertexPtr);

	// May actually return a different virtual address on some systems
	// nlassert(m_VertexPtr == m_VertexPtrAligned);

	// Profiling
	if (m_Driver->_VBHardProfiling)
	{
		TTicks afterLock;
		afterLock = CTime::getPerformanceTime();
		m_Driver->appendVBHardLockProfile(afterLock - beforeLock, VB);
	}

	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferAMDPinned::unlock()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_unlock);

	m_VertexPtr = nullptr;

	if (!m_VertexObjectId)
		return;

	// Profiling
	TTicks beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock = CTime::getPerformanceTime();
	}

	// Unlock
	m_Driver->_DriverGLStates.bindArrayBuffer(m_VertexObjectId);
	nglUnmapBuffer(GL_ARRAY_BUFFER);

	// Profiling
	if (m_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		m_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}
}

// ***************************************************************************

void *CVertexBufferAMDPinned::getPointer()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_getPointer)

	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferAMDPinned::unlock(uint /* startVert */,uint /* endVert */)
{
	H_AUTO_OGL(CVertexBufferAMDPinned_unlock)

	unlock();
}

// ***************************************************************************

void CVertexBufferAMDPinned::enable()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_enable)
	if (m_Driver->_CurrentVertexBufferGL != this)
	{
		m_Driver->_CurrentVertexBufferGL = this;
	}
}

// ***************************************************************************

void CVertexBufferAMDPinned::disable()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_disable)
	if (m_Driver->_CurrentVertexBufferGL == this)
	{
		m_Driver->_CurrentVertexBufferGL = nullptr;
	}
}

// ***************************************************************************

GLuint CVertexBufferAMDPinned::getGLuint()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_getGLuint);

	return m_VertexObjectId;
}

// ***************************************************************************

void CVertexBufferAMDPinned::setFrameInFlight(uint64 swapBufferCounter)
{
	H_AUTO_OGL(CVertexBufferAMDPinned_setFrameInFlight);

	m_FrameInFlight = swapBufferCounter;
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

CVertexBufferStreamGL3::CVertexBufferStreamGL3(CDriverGL3 *drv, CVertexBuffer *vb)
    : IVertexBufferGL3(drv, vb, IVertexBufferGL3::Stream)
    , m_VertexPtr(nullptr)
    , m_RangeLocked(false)
    , m_Drawn(false)
    , m_LostWarned(false)
{
	H_AUTO_OGL(CVertexBufferStreamGL3_CVertexBufferStreamGL3);
}

// ***************************************************************************

CVertexBufferStreamGL3::~CVertexBufferStreamGL3()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_CVertexBufferStreamGL3Dtor);

	// The range is not given back: the stream writes over it when its chunk comes round again
	if (m_RangeLocked)
		m_Driver->_VertexStream.unlock(m_Handle);
}

// ***************************************************************************

void *CVertexBufferStreamGL3::lock()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_lock);

	if (m_VertexPtr) return m_VertexPtr; // already locked

	CStreamBufferGL3 &stream = m_Driver->_VertexStream;

	uint32 size = VB->getNumVertices() * VB->getVertexSize();
	if (!size)
		size = 1; // a range has a size

	const bool valid = stream.isValid(m_Handle);
	if (valid && !m_Drawn && size <= m_Handle.Size)
	{
		// Nothing has drawn the vertices since they were written, so nothing reads the
		// range: the caller goes on writing where it was
		m_VertexPtr = stream.lock(m_Handle);
		m_RangeLocked = true;
		return m_VertexPtr;
	}

	// A new range, with undefined contents. The one before is left to the draws that
	// read it; and where the stream has given it to other data, there is nothing to
	// go on with. A range that is only too small, because the vertex buffer has more
	// vertices now, is left as well: its vertices are not carried over, which would
	// take a read of memory that is mapped for writing. With local memory the engine
	// writes every vertex at each of its locks here; without, it does not keep the
	// vertices itself when the growth passes the capacity.
	//
	// The attribute pointers of the active vertex buffer are on the range it had when
	// activeVertexBuffer was called: a draw reads the new range after the next call.
	m_Handle = stream.reserve(size);
	m_Drawn = false;
	if (!stream.isValid(m_Handle))
	{
		// The stream has logged why. The caller gets memory to write to all the same,
		// and the vertex buffer has no vertices to draw until a lock works.
		m_DummyVB.resize(size);
		m_VertexPtr = &m_DummyVB[0];
		m_RangeLocked = false;
		return m_VertexPtr;
	}

	m_VertexPtr = stream.lock(m_Handle);
	m_RangeLocked = true;
	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferStreamGL3::unlock()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_unlock);

	if (m_RangeLocked)
	{
		m_Driver->_VertexStream.unlock(m_Handle);
		m_RangeLocked = false;
	}
	m_VertexPtr = nullptr;
}

// ***************************************************************************

void CVertexBufferStreamGL3::unlock(uint /* startVert */, uint /* endVert */)
{
	H_AUTO_OGL(CVertexBufferStreamGL3_unlock);

	unlock();
}

// ***************************************************************************

void *CVertexBufferStreamGL3::getPointer()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_getPointer);

	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferStreamGL3::enable()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_enable);

	if (m_Driver->_CurrentVertexBufferGL != this)
	{
		m_Driver->_CurrentVertexBufferGL = this;
	}
}

// ***************************************************************************

void CVertexBufferStreamGL3::disable()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_disable);

	if (m_Driver->_CurrentVertexBufferGL == this)
	{
		m_Driver->_CurrentVertexBufferGL = nullptr;
	}
}

// ***************************************************************************

GLuint CVertexBufferStreamGL3::getGLuint()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_getGLuint);

	// Asked for when the vertex buffer is made the active one: from here on the draws
	// may read the range, and the stream is told that the frame and the draw use it
	CStreamBufferGL3 &stream = m_Driver->_VertexStream;
	return stream.isValid(m_Handle) ? stream.use(m_Handle) : 0;
}

// ***************************************************************************

void *CVertexBufferStreamGL3::getArrayPointer()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_getArrayPointer);

	return (void *)(size_t)m_Handle.Offset;
}

// ***************************************************************************

void CVertexBufferStreamGL3::setFrameInFlight(uint64 /* swapBufferCounter */)
{
	H_AUTO_OGL(CVertexBufferStreamGL3_setFrameInFlight);

	// A draw has read the range: the next lock is to get another one, and the
	// chunk is not to be written again before the GPU has finished this frame
	m_Drawn = true;
	CStreamBufferGL3 &stream = m_Driver->_VertexStream;
	if (stream.isValid(m_Handle))
		stream.use(m_Handle);
}

// ***************************************************************************

bool CVertexBufferStreamGL3::isInvalid()
{
	H_AUTO_OGL(CVertexBufferStreamGL3_isInvalid);

	if (m_Driver->_VertexStream.isValid(m_Handle))
		return false;

	// The stream has given the range to other data, or no lock has worked. Nothing
	// is drawn, where garbage would be.
	if (!m_LostWarned)
	{
		m_LostWarned = true;
		nlwarning("3D: the vertex buffer '%s' is to be drawn with vertices that the driver no longer has, and is not drawn. "
		          "A vertex buffer of the stream usages without local memory is to be filled in every frame it is drawn in",
		    VB->getName().c_str());
	}
	return true;
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

} // NLDRIVERGL3
} // NL3D

