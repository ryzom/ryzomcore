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
IVertexBufferGL::IVertexBufferGL(CDriverGL3 *drv, CVertexBuffer *vb) : VB (vb)
{
	H_AUTO_OGL(IVertexBufferGL_IVertexBufferGL)
	m_Driver= drv;
	m_Invalid = false;
}
// ***************************************************************************
IVertexBufferGL::~IVertexBufferGL()
{
	H_AUTO_OGL(IVertexBufferGL_IVertexBufferGLDtor)
}

// ***************************************************************************
CVertexBufferGL::CVertexBufferGL(CDriverGL3 *drv, CVertexBuffer *vb) 
	: IVertexBufferGL(drv, vb),
	m_VertexPtr(NULL),
	VertexObjectId(0)
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARB)
}

// ***************************************************************************
CVertexBufferGL::~CVertexBufferGL()
{
	H_AUTO_OGL(CVertexBufferGLARB_CVertexBufferGLARBDtor)
	if (m_Driver && VertexObjectId)
	{
		if (m_Driver->_DriverGLStates.getCurrBoundARBVertexBuffer() == VertexObjectId)
		{
			m_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
		}
	}
	if (VertexObjectId)
	{
		GLuint id = (GLuint) VertexObjectId;
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
void *CVertexBufferGL::lock()
{
	H_AUTO_OGL(CVertexBufferGLARB_lock);

	if (m_VertexPtr) return m_VertexPtr; // already locked
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
		const uint size = VB->getNumVertices() * VB->getVertexSize();
		m_Driver->_DriverGLStates.forceBindARBVertexBuffer(vertexBufferID);
		nglBufferData(GL_ARRAY_BUFFER, size, NULL, m_Driver->vertexBufferUsageGL3(m_MemType));
		if (glGetError() != GL_NO_ERROR)
		{
			m_Driver->incrementResetCounter();
			nglDeleteBuffers(1, &vertexBufferID);
			return &m_DummyVB[0];;
		}
		VertexObjectId = vertexBufferID;
		NLMISC::contReset(m_DummyVB); // free vector memory for real
		nlassert(VertexObjectId);
		m_Invalid = false;
		m_Driver->_LostVBList.erase(m_IteratorInLostVBList);
		// continue to standard mapping code below ..
	}
	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	m_Driver->_DriverGLStates.bindARBVertexBuffer(VertexObjectId);


	m_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (!m_VertexPtr)
	{
		nglUnmapBuffer(GL_ARRAY_BUFFER);
		nlassert(nglIsBuffer((GLuint) VertexObjectId));
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
void CVertexBufferGL::unlock()
{
	H_AUTO_OGL(CVertexBufferGLARB_unlock);

	m_VertexPtr = NULL;
	if (m_Invalid) return;
	if (!VertexObjectId) return;
	TTicks	beforeLock = 0;
	if (m_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	m_Driver->_DriverGLStates.bindARBVertexBuffer(VertexObjectId);
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
void		*CVertexBufferGL::getPointer()
{
	H_AUTO_OGL(CVertexBufferGLARB_getPointer)
	return m_VertexPtr;
}

// ***************************************************************************
void CVertexBufferGL::unlock(uint /* startVert */,uint /* endVert */)
{
	H_AUTO_OGL(CVertexBufferGLARB_unlock)
	unlock(); // can't do a lock on a range of the vb..
}

// ***************************************************************************
void CVertexBufferGL::enable()
{
	H_AUTO_OGL(CVertexBufferGLARB_enable)
	if (m_Driver->_CurrentVertexBufferGL != this)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->enable(); */
		m_Driver->_CurrentVertexBufferGL= this;
	}
}

// ***************************************************************************
void CVertexBufferGL::disable()
{
	H_AUTO_OGL(CVertexBufferGLARB_disable)
	if (m_Driver->_CurrentVertexBufferGL != NULL)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->disable(); */
		m_Driver->_CurrentVertexBufferGL= NULL;
	}
}

// ***************************************************************************
void CVertexBufferGL::initGL(uint vertexObjectID, CVertexBuffer::TPreferredMemory memType)
{
	H_AUTO_OGL(CVertexBufferGLARB_initGL)
	VertexObjectId = vertexObjectID;
	m_MemType = memType;
}

// ***************************************************************************
void			CVertexBufferGL::lockHintStatic(bool /* staticLock */)
{
	H_AUTO_OGL(CVertexBufferGLARB_lockHintStatic)
	// no op.
}

// ***************************************************************************
void CVertexBufferGL::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferGLARB_setupVBInfos)
	vb.VertexObjectId = VertexObjectId;
}

// ***************************************************************************
void CVertexBufferGL::invalidate()
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

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

