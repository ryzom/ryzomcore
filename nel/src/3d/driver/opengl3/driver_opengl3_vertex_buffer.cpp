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
	m_VertexPtr(NULL),
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

	// Allocate shadow buffer for CpuReadWrite and PartialWrite (CPU reads/writes go here)
	if (preferred == CVertexBuffer::CpuReadWrite || preferred == CVertexBuffer::PartialWrite)
		m_ShadowData.resize(size, 0);

	for (GLsizei i = 0; i < NL3D_GL3_BUFFER_QUEUE_MAX; ++i)
	{
		m_VertexObjectId[i] = 0;
		m_FrameInFlight[i] = NL3D_GL3_BUFFER_NOT_IN_FLIGHT;
	}

	// Create ids
	GLsizei nbBuff = vbgl3BufferForType(preferred);
	nglGenBuffers(nbBuff, m_VertexObjectId);

	// Initialize
	for (GLsizei i = 0; i < nbBuff; ++i)
	{
		drv->_DriverGLStates.forceBindArrayBuffer(m_VertexObjectId[i]);
		nglBufferData(GL_ARRAY_BUFFER, size, NULL, drv->vertexBufferUsageGL3(preferred));
		drv->_DriverGLStates.forceBindArrayBuffer(0);
	}
}

// ***************************************************************************

CVertexBufferGL3::~CVertexBufferGL3()
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARBDtor)
	if (m_Driver && m_VertexObjectId)
	{
		GLsizei nbBuff = vbgl3BufferForType(m_MemType);
		for (GLsizei i = 0; i < nbBuff; ++i)
		{
			if (m_Driver->_DriverGLStates.getCurrBoundArrayBuffer() == m_VertexObjectId[i])
			{
				m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
			}
		}
	}
	for (GLsizei i = 0; i < NL3D_GL3_BUFFER_QUEUE_MAX; ++i)
	{
		if (m_VertexObjectId[i])
		{
			GLuint id = m_VertexObjectId[i];
			nlassert(nglIsBuffer(id));
			nglDeleteBuffers(1, &id);
		}
	}
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
			nglBufferData(GL_ARRAY_BUFFER, size, NULL, m_Driver->vertexBufferUsageGL3(m_MemType));
			m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
			if (glGetError() != GL_NO_ERROR)
			{
				m_Driver->incrementResetCounter();
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

	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}

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
	m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
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

	m_VertexPtr = NULL;
	if (m_Invalid) return;

	// Shadow buffer: just mark dirty, no GL interaction
	if (!m_ShadowData.empty())
	{
		m_ShadowDirty = true;
		return;
	}

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
	m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
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
	if (m_Driver->_CurrentVertexBufferGL != NULL)
	{
		m_Driver->_CurrentVertexBufferGL = NULL;
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
		m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
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
			m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
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
			nglBufferData(GL_COPY_READ_BUFFER, totalDirty, NULL, GL_STREAM_DRAW);

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
	m_DummyVB.resize(VB->getNumVertices() * VB->getVertexSize(), 0);
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
	m_VertexPtr(NULL),
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
	if (m_Driver && m_VertexObjectId)
	{
		if (m_Driver->_DriverGLStates.getCurrBoundArrayBuffer() == m_VertexObjectId)
		{
			m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
		}
	}
	if (m_VertexObjectId)
	{
		GLuint id = (GLuint)m_VertexObjectId;
		nlassert(nglIsBuffer(id));
		nglDeleteBuffers(1, &id);
	}
	delete[] static_cast<char *>(m_VertexPtrAllocated);
	m_VertexPtrAllocated = NULL;
	m_VertexPtrAligned = NULL;
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
		m_VertexPtr = NULL;
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
	m_Driver->_DriverGLStates.forceBindArrayBuffer(0);
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

	m_VertexPtr = NULL;

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
	m_Driver->_DriverGLStates.forceBindArrayBuffer(0);

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
	if (m_Driver->_CurrentVertexBufferGL != NULL)
	{
		m_Driver->_CurrentVertexBufferGL = NULL;
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

} // NLDRIVERGL3
} // NL3D

