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
#include "driver_opengl_vertex_buffer_hard.h"

#include "nel/3d/vertex_buffer.h"

using	namespace std;
using	namespace NLMISC;

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************
IVertexArrayRange::IVertexArrayRange(CDriverGL3 *drv)
{
	H_AUTO_OGL(IVertexArrayRange_IVertexArrayRange)
	_Driver= drv;
}
// ***************************************************************************
IVertexArrayRange::~IVertexArrayRange()
{
	H_AUTO_OGL(IVertexArrayRange_IVertexArrayRangeDtor)
}

// ***************************************************************************
IVertexBufferHardGL::IVertexBufferHardGL(CDriverGL3 *drv, CVertexBuffer *vb) : VB (vb)
{
	H_AUTO_OGL(IVertexBufferHardGL_IVertexBufferHardGL)
	_Driver= drv;
	_Invalid = false;
}
// ***************************************************************************
IVertexBufferHardGL::~IVertexBufferHardGL()
{
	H_AUTO_OGL(IVertexBufferHardGL_IVertexBufferHardGLDtor)
}


// ***************************************************************************
// CVertexArrayRangeARB
// ***************************************************************************


// ***************************************************************************
CVertexArrayRange::CVertexArrayRange(CDriverGL3 *drv) : IVertexArrayRange(drv),
															 _VBType(CVertexBuffer::AGPPreferred)
{
	H_AUTO_OGL(CVertexArrayRangeARB_CVertexArrayRangeARB)
}

// ***************************************************************************
IVertexBufferHardGL *CVertexArrayRange::createVBHardGL(uint size, CVertexBuffer *vb)
{
	H_AUTO_OGL(CVertexArrayRange_createVBHardGL)

	// create a ARB VBHard
	GLuint vertexBufferID;
	glGetError();

	nglGenBuffers(1, &vertexBufferID);

	if (glGetError() != GL_NO_ERROR) return NULL;
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(vertexBufferID);
	switch(_VBType)
	{
		case CVertexBuffer::AGPPreferred:
			nglBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
			break;
		case CVertexBuffer::StaticPreferred:
			if (_Driver->getStaticMemoryToVRAM())
				nglBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
			else
				nglBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
			break;
		default:
			nlassert(0);
			break;
	}
	if (glGetError() != GL_NO_ERROR)
	{
		nglDeleteBuffers(1, &vertexBufferID);
		return NULL;
	}
	CVertexBufferHard *newVbHard= new CVertexBufferHard(_Driver, vb);
	newVbHard->initGL(vertexBufferID, this, _VBType);
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	return newVbHard;
}

// ***************************************************************************
void CVertexArrayRange::enable()
{
	H_AUTO_OGL(CVertexArrayRangeARB_enable)
	if (_Driver->_CurrentVertexArrayRange != this)
	{
		_Driver->_CurrentVertexArrayRange= this;
	}
}

// ***************************************************************************
void CVertexArrayRange::disable()
{
	H_AUTO_OGL(CVertexArrayRangeARB_disbale)
	if (_Driver->_CurrentVertexBufferHard != NULL)
	{
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void CVertexArrayRange::updateLostBuffers()
{
	H_AUTO_OGL(CVertexArrayRangeARB_updateLostBuffers)
	// Put all vb that have been lost in the NotResident state so that they will be recomputed
	// We do this only if the app is active, because if vb were lost, it is likely that there are no resources available.
	nlassert(_Driver);
	if (_Driver->isWndActive())
	{
		for (std::list<CVertexBufferHard *>::iterator it = _LostVBList.begin(); it != _LostVBList.end(); ++it)
		{
			nlassert((*it)->_VertexObjectId);
			GLuint id = (GLuint) (*it)->_VertexObjectId;
			nlassert(nglIsBuffer(id));
			nglDeleteBuffers(1, &id);
			(*it)->_VertexObjectId = 0;
			(*it)->VB->setLocation(CVertexBuffer::NotResident);
		}
		_LostVBList.clear();
	}
}


// ***************************************************************************
// CVertexBufferHardARB
// ***************************************************************************


// ***************************************************************************
CVertexBufferHard::CVertexBufferHard(CDriverGL3 *drv, CVertexBuffer *vb) :  IVertexBufferHardGL(drv, vb),
                                                                                 _VertexPtr(NULL),
																				_VertexObjectId(0)

{
	H_AUTO_OGL(CVertexBufferHardARB_CVertexBufferHardARB)

	_VertexArrayRange = NULL;
	#ifdef NL_DEBUG
		_Unmapping = false;
	#endif

}

// ***************************************************************************
CVertexBufferHard::~CVertexBufferHard()
{
	H_AUTO_OGL(CVertexBufferHardARB_CVertexBufferHardARBDtor)
	if (_Driver && _VertexObjectId)
	{
		if (_Driver->_DriverGLStates.getCurrBoundARBVertexBuffer() == _VertexObjectId)
		{
			_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
		}
	}
	if (_VertexObjectId)
	{
		GLuint id = (GLuint) _VertexObjectId;
		nlassert(nglIsBuffer(id));
		nglDeleteBuffers(1, &id);
	}
	if (_VertexArrayRange)
	{
		if (_Invalid)
		{
			if (VB->getLocation() != CVertexBuffer::NotResident)
			{
				// when the vb is put in tthe NotResident state, it is removed from that list
				_VertexArrayRange->_LostVBList.erase(_IteratorInLostVBList);
			}
		}
	}
	#ifdef NL_DEBUG
		if (_VertexPtr)
		{
			_VertexArrayRange->_MappedVBList.erase(_IteratorInMappedVBList);
		}
	#endif

}

// ***************************************************************************
void *CVertexBufferHard::lock()
{
	H_AUTO_OGL(CVertexBufferHardARB_lock);

	if (_VertexPtr) return _VertexPtr; // already locked
	if (_Invalid)
	{
		if (VB->getLocation() != CVertexBuffer::NotResident)
		{
			nlassert(!_DummyVB.empty());
			return &_DummyVB[0];
		}
		// recreate a vb
		GLuint vertexBufferID;

		nglGenBuffers(1, &vertexBufferID);

		if (glGetError() != GL_NO_ERROR)
		{
			_Driver->incrementResetCounter();
			return &_DummyVB[0];
		}
		const uint size = VB->getNumVertices() * VB->getVertexSize();
		_Driver->_DriverGLStates.forceBindARBVertexBuffer(vertexBufferID);
		switch(_MemType)
		{
			case CVertexBuffer::AGPPreferred:
				nglBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
			break;
			case CVertexBuffer::StaticPreferred:
				if (_Driver->getStaticMemoryToVRAM())
					nglBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
				else
					nglBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
			break;
			default:
				nlassert(0);
			break;
		}
		if (glGetError() != GL_NO_ERROR)
		{
			_Driver->incrementResetCounter();
			nglDeleteBuffers(1, &vertexBufferID);
			return &_DummyVB[0];;
		}
		_VertexObjectId = vertexBufferID;
		NLMISC::contReset(_DummyVB); // free vector memory for real
		nlassert(_VertexObjectId);
		_Invalid = false;
		_VertexArrayRange->_LostVBList.erase(_IteratorInLostVBList);
		// continue to standard mapping code below ..
	}
	TTicks	beforeLock = 0;
	if (_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	_Driver->_DriverGLStates.bindARBVertexBuffer(_VertexObjectId);


	_VertexPtr = nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (!_VertexPtr)
	{
		nglUnmapBuffer(GL_ARRAY_BUFFER);
		nlassert(nglIsBuffer((GLuint) _VertexObjectId));
		invalidate();
		return &_DummyVB[0];
	}

	#ifdef NL_DEBUG
		_VertexArrayRange->_MappedVBList.push_front(this);
		_IteratorInMappedVBList = _VertexArrayRange->_MappedVBList.begin();
	#endif
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	// Lock Profile?
	if (_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHard::unlock()
{
	H_AUTO_OGL(CVertexBufferHardARB_unlock);

	_VertexPtr = NULL;
	if (_Invalid) return;
	if (!_VertexObjectId) return;
	TTicks	beforeLock = 0;
	if (_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	_Driver->_DriverGLStates.bindARBVertexBuffer(_VertexObjectId);
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
	if (_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}
	#ifdef NL_DEBUG
		_VertexArrayRange->_MappedVBList.erase(_IteratorInMappedVBList);
	#endif
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	if (!unmapOk)
	{
		invalidate();
	}
	/* double end = CTime::ticksToSecond(CTime::getPerformanceTime());
	nlinfo("3D: Unlock = %f ms", (float) ((end - start) * 1000)); */
}

// ***************************************************************************
void		*CVertexBufferHard::getPointer()
{
	H_AUTO_OGL(CVertexBufferHardARB_getPointer)
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHard::unlock(uint /* startVert */,uint /* endVert */)
{
	H_AUTO_OGL(CVertexBufferHardARB_unlock)
	unlock(); // can't do a lock on a range of the vb..
}

// ***************************************************************************
void CVertexBufferHard::enable()
{
	H_AUTO_OGL(CVertexBufferHardARB_enable)
	if (_Driver->_CurrentVertexBufferHard != this)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->enable(); */
		_Driver->_CurrentVertexBufferHard= this;
	}
}

// ***************************************************************************
void CVertexBufferHard::disable()
{
	H_AUTO_OGL(CVertexBufferHardARB_disable)
	if (_Driver->_CurrentVertexBufferHard != NULL)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->disable(); */
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void CVertexBufferHard::initGL(uint vertexObjectID, CVertexArrayRange *var, CVertexBuffer::TPreferredMemory memType)
{
	H_AUTO_OGL(CVertexBufferHardARB_initGL)
	_VertexObjectId = vertexObjectID;
	_MemType = memType;
	_VertexArrayRange = var;
}

// ***************************************************************************
void			CVertexBufferHard::lockHintStatic(bool /* staticLock */)
{
	H_AUTO_OGL(CVertexBufferHardARB_lockHintStatic)
	// no op.
}

// ***************************************************************************
void CVertexBufferHard::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferHardARB_setupVBInfos)
	vb.VertexObjectId = _VertexObjectId;
}

// ***************************************************************************
void CVertexBufferHard::invalidate()
{
	H_AUTO_OGL(CVertexBufferHardARB_invalidate)
	nlassert(!_Invalid);
	// Buffer is lost (maybe there was a alt-tab or fullscrren / windowed change)
	// Buffer is deleted at end of frame only
	_Invalid = true;
	_Driver->incrementResetCounter();
	_DummyVB.resize(VB->getNumVertices() * VB->getVertexSize(), 0);
	// insert in lost vb list
	_VertexArrayRange->_LostVBList.push_front(this);
	_IteratorInLostVBList = _VertexArrayRange->_LostVBList.begin();
}

// ***************************************************************************
#ifdef NL_DEBUG
	void CVertexArrayRange::dumpMappedBuffers()
	{
		nlwarning("*****************************************************");
		nlwarning("Mapped buffers :");
		for (std::list<CVertexBufferHard *>::iterator it = _MappedVBList.begin(); it != _MappedVBList.end(); ++it)
		{
			CVertexBufferHard &vbarb = **it;
			nlwarning("Buffer id = %u, size = %u, address = %p", vbarb._VertexObjectId, vbarb.VB->getVertexSize() * vbarb.VB->getNumVertices(), vbarb.getPointer());
		}
	}
#endif

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

