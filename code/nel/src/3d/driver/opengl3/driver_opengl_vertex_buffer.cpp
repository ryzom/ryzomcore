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

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

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

CVertexBufferGL3::CVertexBufferGL3(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TPreferredMemory preferred, CVertexBuffer *vb) 
	: IVertexBufferGL3(drv, vb, IVertexBufferGL3::GL3),
	m_VertexPtr(NULL),
	m_VertexObjectId(0)
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARB)

	// Create id and bind
	GLuint vertexBufferID;
	nglGenBuffers(1, &vertexBufferID);
	drv->_DriverGLStates.forceBindARBVertexBuffer(vertexBufferID);

	// Initialize
	nglBufferData(GL_ARRAY_BUFFER, size, NULL, drv->vertexBufferUsageGL3(preferred));
	m_VertexObjectId = vertexBufferID;
	m_MemType = preferred;

	// Unbind
	drv->_DriverGLStates.forceBindARBVertexBuffer(0);
}

// ***************************************************************************

CVertexBufferGL3::~CVertexBufferGL3()
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARBDtor)
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
		// recreate a vb
		GLuint vertexBufferID;
		
		glGetError();
		nglGenBuffers(1, &vertexBufferID);

		if (glGetError() != GL_NO_ERROR)
		{
			m_Driver->incrementResetCounter();
			return &m_DummyVB[0];
		}
		m_Driver->_DriverGLStates.forceBindARBVertexBuffer(vertexBufferID);
		nglBufferData(GL_ARRAY_BUFFER, size, NULL, m_Driver->vertexBufferUsageGL3(m_MemType));
		if (glGetError() != GL_NO_ERROR)
		{
			m_Driver->incrementResetCounter();
			nglDeleteBuffers(1, &vertexBufferID);
			return &m_DummyVB[0];;
		}
		m_VertexObjectId = vertexBufferID;
		NLMISC::contReset(m_DummyVB); // free vector memory for real
		nlassert(m_VertexObjectId);
		m_Invalid = false;
		m_Driver->_LostVBList.erase(m_IteratorInLostVBList);
		// continue to standard mapping code below ..
	}
	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId);

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
		// NOTE: GL_MAP_INVALIDATE_BUFFER_BIT removes the cost of waiting for synchronization (major performance impact), 
		// but adds the cost of allocating a new buffer (which hast a much lower performance impact)
		m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		break;
	case CVertexBuffer::RAMPreferred:
		// m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT | GL_MAP_COHERENT);
		// NOTE: Persistent / Coherent is only available in OpenGL 4.4 (2013/2014 hardware with recent drivers)
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		break;
	default:
		// m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT);
		m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		break;
	}

	if (!m_VertexPtr)
	{
		nglUnmapBuffer(GL_ARRAY_BUFFER);
		nlassert(nglIsBuffer((GLuint)m_VertexObjectId));
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
	if (!m_VertexObjectId) return;
	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	m_Driver->_DriverGLStates.bindARBVertexBuffer(m_VertexObjectId);
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
		m_Driver->_CurrentVertexBufferGL= this;
	}
}

// ***************************************************************************

void CVertexBufferGL3::disable()
{
	H_AUTO_OGL(CVertexBufferGLARB_disable)
	if (m_Driver->_CurrentVertexBufferGL != NULL)
	{
		m_Driver->_CurrentVertexBufferGL= NULL;
	}
}

// ***************************************************************************

void CVertexBufferGL3::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferGLARB_setupVBInfos)
	vb.VertexObjectId = m_VertexObjectId;
}

// ***************************************************************************

void CVertexBufferGL3::setFence()
{
	H_AUTO_OGL(CVertexBufferGLARB_setFence)

	// no-op
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
	m_VertexObjectId(0)
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
		nlerror("GL3: Failed to pin memory");
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

void CVertexBufferAMDPinned::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferAMDPinned_setupVBInfos)

	vb.VertexObjectId = m_VertexObjectId;
}

// ***************************************************************************

void CVertexBufferAMDPinned::setFence()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_setFence)

	// no-op
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

CVertexBufferAMDPinnedVolatile::CVertexBufferAMDPinnedVolatile(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TPreferredMemory preferred, CVertexBuffer *vb) 
	: IVertexBufferGL3(drv, vb, IVertexBufferGL3::AMDPinnedVolatile),
	m_Block(NULL),
	m_VertexPtr(NULL)
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_CVertexBufferAMDPinned)

	// no-op
}

// ***************************************************************************

CVertexBufferAMDPinnedVolatile::~CVertexBufferAMDPinnedVolatile()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_CVertexBufferAMDPinnedDtor)
	
	// no-op
}

// ***************************************************************************

void *CVertexBufferAMDPinnedVolatile::lock()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_lock);

	if (m_VertexPtr) // Already locked...
		return m_VertexPtr;

	// Profiling
	TTicks beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock = CTime::getPerformanceTime();
	}

	// Allocate
	const uint size = VB->getNumVertices() * VB->getVertexSize();
	m_Block = m_Driver->_AMDPinnedAllocator->allocate(size);

	// Lock
	nlassert(m_Block);
	m_VertexPtr = m_Block->Buffer;
	nlassert(m_VertexPtr);

	// Profiling
	if (m_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		m_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}

	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferAMDPinnedVolatile::unlock()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_unlock);

	m_VertexPtr = NULL;
	// m_Block = NULL;
}

// ***************************************************************************

void *CVertexBufferAMDPinnedVolatile::getPointer()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_getPointer)

	return m_VertexPtr;
}

// ***************************************************************************

void CVertexBufferAMDPinnedVolatile::unlock(uint /* startVert */,uint /* endVert */)
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_unlock)

	unlock();
}

// ***************************************************************************

void CVertexBufferAMDPinnedVolatile::enable()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_enable)
	if (m_Driver->_CurrentVertexBufferGL != this)
	{
		m_Driver->_CurrentVertexBufferGL = this;
	}
}

// ***************************************************************************

void CVertexBufferAMDPinnedVolatile::disable()
{
	H_AUTO_OGL(CVertexBufferAMDPinned_disable)
	if (m_Driver->_CurrentVertexBufferGL != NULL)
	{
		m_Driver->_CurrentVertexBufferGL = NULL;
	}
}

// ***************************************************************************

void CVertexBufferAMDPinnedVolatile::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferAMDPinned_setupVBInfos)

	nlassert(m_Block);
	vb.VertexObjectId = m_Block->VertexObjectId;
}

// ***************************************************************************

void CVertexBufferAMDPinnedVolatile::setFence()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedVolatile_setFence)

	// no-op
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

CVertexBufferAMDPinnedAllocator::CVertexBufferAMDPinnedAllocator(CDriverGL3 *driver)
{
	H_AUTO_OGL(CVertexBufferAMDPinnedAllocator_ctor)

	m_Driver = driver;
	swap();
}

// ***************************************************************************

CVertexBufferAMDPinnedAllocator::~CVertexBufferAMDPinnedAllocator()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedAllocator_dtor)

	// Release all pools
	for (uint bin = 0; bin < NLDRV_GL3_AMD_PINNED_VOLATILE_BINS; ++bin)
	{
		for (uint idx = 0; idx < m_Pool[bin].size(); ++idx)
		{
			CVertexBufferAMDPinnedBlock *block = m_Pool[bin][idx];

			if (m_Driver && block->VertexObjectId)
			{
				if (m_Driver->_DriverGLStates.getCurrBoundARBVertexBuffer() == block->VertexObjectId)
				{
					m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
				}
			}

			if (block->VertexObjectId)
			{
				nlassert(nglIsBuffer(block->VertexObjectId));
				nglDeleteBuffers(1, &block->VertexObjectId);
			}

			delete block->Allocated;
			delete block;
		}

		m_Pool[bin].clear();
	}

	m_Driver = NULL;
}

// ***************************************************************************

CVertexBufferAMDPinnedBlock *CVertexBufferAMDPinnedAllocator::allocate(uint size)
{
	H_AUTO_OGL(CVertexBufferAMDPinnedAllocator_allocate)

	uint bin = 0;
	uint sizeShift = size >> (NLDRV_GL3_AMD_PINNED_VOLATILE_SHIFT);
	while (sizeShift)
	{
		sizeShift >>= 1;
		++bin;
	}
	nlassert(bin < NLDRV_GL3_AMD_PINNED_VOLATILE_BINS); // Not implemented over 8MB

	uint idx = m_PoolIndex[bin];
	++m_PoolIndex[bin];

	if (idx < m_Pool[bin].size())
	{
		return m_Pool[bin][idx];
	}

	// Create new block
	CVertexBufferAMDPinnedBlock *block = new CVertexBufferAMDPinnedBlock();
	block->Bin = bin;

	// Allocate memory
	uint allocSize = 1 << (NLDRV_GL3_AMD_PINNED_VOLATILE_SHIFT + bin);
	static int totalAlloc = 0;
	totalAlloc += allocSize + 4096;
	nldebug("GL3: APV Alloc (%i -> %i, total: %i)", size, allocSize, totalAlloc);
	block->Allocated = new char[allocSize + 4096];
	nlassert(block->Allocated);
	uintptr_t addr = (uintptr_t)block->Allocated;
	addr = (addr + 4095) & (~0xfff);
	void *addrAligned = (void *)addr;
	block->Buffer = addrAligned;

	// Create id and bind
	nglGenBuffers(1, &block->VertexObjectId);
	nglBindBuffer(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, block->VertexObjectId);
	
	// Set buffer
	nglBufferData(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, allocSize, addrAligned, GL_DYNAMIC_DRAW);
	if (glGetError() == GL_INVALID_OPERATION)
	{
		nlerror("GL3: Failed to pin memory (volatile)");
		nglDeleteBuffers(1, &block->VertexObjectId);
		block->VertexObjectId = 0;
	}

	// Unbind
	nglBindBuffer(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, 0);

	// Add to pool
	m_Pool[bin].push_back(block);

	// TEST LOCK
	/*m_Driver->_DriverGLStates.bindARBVertexBuffer(newblock->VertexObjectId);
	// m_VertexPtr = nglMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	void *testptr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	nlassert(testptr);
	m_Driver->_DriverGLStates.bindARBVertexBuffer(0);*/

	return block;
}

// ***************************************************************************

void CVertexBufferAMDPinnedAllocator::swap()
{
	H_AUTO_OGL(CVertexBufferAMDPinnedAllocator_swap)

	for (uint bin = 0; bin < NLDRV_GL3_AMD_PINNED_VOLATILE_BINS; ++bin)
	{
		m_PoolIndex[bin] = 0;
	}
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

