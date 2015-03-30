// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdopengl.h"

#include "driver_opengl.h"
#include "driver_opengl_vertex_buffer.h"

#include "nel/3d/vertex_buffer.h"

using	namespace std;
using	namespace NLMISC;

namespace NL3D {
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

static inline GLsizei vbgl3BufferForType(CVertexBuffer::TPreferredMemory mem)
{
	switch (mem)
	{
	case CVertexBuffer::AGPVolatile:
	case CVertexBuffer::RAMVolatile:
		return NL3D_GL3_BUFFER_QUEUE_MAX;
	default: 
		return 1;
	}
}

CVertexBufferGL3::CVertexBufferGL3(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TPreferredMemory preferred, CVertexBuffer *vb)
	: IVertexBufferGL3(drv, vb, IVertexBufferGL3::GL3),
	m_VertexPtr(NULL),
	m_CurrentIndex(0),
	m_CurrentInFlight(false),
#if NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG
	m_ReuseCount(0),
	m_InvalidateCount(0),
#endif
	m_MemType(preferred)
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARB);

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
		drv->_DriverGLStates.forceBindARBVertexBuffer(m_VertexObjectId[i]);
		nglBufferData(GL_ARRAY_BUFFER, size, NULL, drv->vertexBufferUsageGL3(preferred));
		drv->_DriverGLStates.forceBindARBVertexBuffer(0);
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
			if (m_Driver->_DriverGLStates.getCurrBoundARBVertexBuffer() == m_VertexObjectId[i])
			{
				m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
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
			return &m_DummyVB[0];
		}

		for (GLsizei i = 0; i < nbBuff; ++i)
		{
			m_Driver->_DriverGLStates.forceBindARBVertexBuffer(m_VertexObjectId[i]);
			nglBufferData(GL_ARRAY_BUFFER, size, NULL, m_Driver->vertexBufferUsageGL3(m_MemType));
			m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
			if (glGetError() != GL_NO_ERROR)
			{
				m_Driver->incrementResetCounter();
				nglDeleteBuffers(1, &m_VertexObjectId[i]);
				return &m_DummyVB[0];
			}
		}
		NLMISC::contReset(m_DummyVB); // free vector memory for real
		nlassert(m_VertexObjectId[m_CurrentIndex]);

		m_Invalid = false;
		m_Driver->_LostVBList.erase(m_IteratorInLostVBList);
		// continue to standard mapping code below ..
	}
	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}

	// m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	// PERFORMANCE: AMD: This brings framerate from 24fps to 38fps, glitches with volatile buffers such as animated models and gui, likely glitches with others
	// m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

	// PERFORMANCE: AMD: This brings framerate from 24fps to 38fps, glitches with landscape rendering
	// m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	// Invalidate when updating volatile buffers, framerate from 24fps to 38fps in reference test on AMD platform
	// TODO: Find where we can optimize with GL_MAP_UNSYNCHRONIZED_BIT
	switch (m_MemType)
	{
	case CVertexBuffer::AGPVolatile:
	case CVertexBuffer::RAMVolatile:
	{
		if (m_CurrentInFlight)
		{
			++m_CurrentIndex;
			m_CurrentIndex %= NL3D_GL3_BUFFER_QUEUE_MAX;
			m_CurrentInFlight = false;
		}
		m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId[m_CurrentIndex]);
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
			m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		}
		break;
	}
	case CVertexBuffer::RAMPreferred:
		m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId[m_CurrentIndex]);
		// m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT | GL_MAP_COHERENT);
		// NOTE: Persistent / Coherent is only available in OpenGL 4.4 (2013/2014 hardware with recent drivers)
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		break;
	default:
		m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId[m_CurrentIndex]);
		// m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT);
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		break;
	}

	if (!m_VertexPtr)
	{
		nglUnmapBuffer(GL_ARRAY_BUFFER);
		nlassert(nglIsBuffer(m_VertexObjectId[m_CurrentIndex]));
		invalidate();
		return &m_DummyVB[0];
	}

	#ifdef NL_DEBUG
		_VertexArrayRange->_MappedVBList.push_front(this);
		_IteratorInMappedVBList = _VertexArrayRange->_MappedVBList.begin();
	#endif
	m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
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
	if (!m_VertexObjectId[m_CurrentIndex]) return;
	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId[m_CurrentIndex]);
	// double start = CTime::ticksToSecond(CTime::getPerformanceTime());
	#ifdef NL_DEBUG
		_Unmapping = true;
	#endif
	GLboolean unmapOk = GL_FALSE;

	unmapOk = nglUnmapBuffer(GL_ARRAY_BUFFER);

	#ifdef NL_DEBUG
		_Unmapping = false;
	#endif
	// Lock Profile?
	if (m_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		m_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}
	#ifdef NL_DEBUG
		_VertexArrayRange->_MappedVBList.erase(_IteratorInMappedVBList);
	#endif
	m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	if (!unmapOk)
	{
		invalidate();
	}
	/* double end = CTime::ticksToSecond(CTime::getPerformanceTime());
	nlinfo("3D: Unlock = %f ms", (float) ((end - start) * 1000)); */
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

CVertexBufferAMDPinned::CVertexBufferAMDPinned(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TPreferredMemory preferred, CVertexBuffer *vb) 
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
		if (m_Driver->_DriverGLStates.getCurrBoundARBVertexBuffer() == m_VertexObjectId)
		{
			m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
		}
	}
	if (m_VertexObjectId)
	{
		GLuint id = (GLuint)m_VertexObjectId;
		nlassert(nglIsBuffer(id));
		nglDeleteBuffers(1, &id);
	}
	delete m_VertexPtrAllocated;
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
	m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId);
	switch (m_MemType)
	{
	case CVertexBuffer::AGPVolatile:
	case CVertexBuffer::RAMVolatile:
		nlerror("Volatile currently not supported by pinned memory, this would require a re-allocating RAM, and thus require a fast allocation mechanism");
		m_VertexPtr = NULL;
		break;
	case CVertexBuffer::RAMPreferred:
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		break;
	default:
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		break;
	}
	m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
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
	m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId);
	nglUnmapBuffer(GL_ARRAY_BUFFER);
	m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);

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

