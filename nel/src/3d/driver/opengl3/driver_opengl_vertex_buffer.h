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

#ifndef NL_DRIVER_OPENGL_VERTEX_BUFFER_H
#define NL_DRIVER_OPENGL_VERTEX_BUFFER_H

#include "nel/misc/types_nl.h"

namespace NL3D {
namespace NLDRIVERGL3 {

#define NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG 0

class CDriverGL3;
class IVertexBufferGL3;
class CVertexBufferInfo;
class CVertexBufferGL3;

class IVertexBufferGL3
{
public:
	enum TVBType { GL3, AMDPinned };

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

	// test if buffer content is invalid. If so, no rendering should occurs (rendering should silently fail)
	inline bool isInvalid() { return m_Invalid; }

public:
	CVertexBuffer *VB;
	TVBType VBType;

protected:
	CDriverGL3 *m_Driver;
	bool m_Invalid;
};

/* GL Core vertex buffer. */
class CVertexBufferGL3 : public IVertexBufferGL3
{
public:
	CVertexBufferGL3(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TPreferredMemory preferred, CVertexBuffer *vb);
	virtual	~CVertexBufferGL3();

	/// \name Implementation
	// @{
	virtual	void *lock();
	virtual	void unlock();
	virtual void unlock(uint startVert, uint endVert);
	virtual void *getPointer();
	virtual	void enable();
	virtual	void disable();
	virtual GLuint getGLuint();
	virtual void setFrameInFlight(uint64 swapBufferCounter);
	// @}

	/// Invalidate the buffer (when it is lost, or when a lock fails)
	void invalidate();

private:
	friend class CDriverGL3;

	CVertexBuffer::TPreferredMemory m_MemType;
	void *m_VertexPtr; // pointer on current datas. Null if not locked

	// if buffer has been invalidated, returns a dummy memory block and silently fails rendering
	std::vector<uint8> m_DummyVB;
	// for use by CVertexArrayRange
	std::list<CVertexBufferGL3*>::iterator m_IteratorInLostVBList;

	GLuint m_VertexObjectId[NL3D_GL3_BUFFER_QUEUE_MAX];
	uint64 m_FrameInFlight[NL3D_GL3_BUFFER_QUEUE_MAX];
	GLsizei m_CurrentIndex;
	bool m_CurrentInFlight;

#if NL3D_GL3_VERTEX_BUFFER_INFLIGHT_DEBUG
	uint32 m_ReuseCount;
	uint32 m_InvalidateCount;
#endif
};

class CVertexBufferAMDPinned : public IVertexBufferGL3
{
public:
	CVertexBufferAMDPinned(CDriverGL3 *drv, uint size, uint numVertices, CVertexBuffer::TPreferredMemory preferred, CVertexBuffer *vb);
	virtual	~CVertexBufferAMDPinned();

	/// \name Implementation
	// @{
	virtual	void *lock();
	virtual	void unlock();
	virtual void unlock(uint startVert, uint endVert);
	virtual void *getPointer();
	virtual	void enable();
	virtual	void disable();
	virtual GLuint getGLuint();
	virtual void setFrameInFlight(uint64 swapBufferCounter);
	// @}

private:
	CVertexBuffer::TPreferredMemory m_MemType;
	void *m_VertexPtrAllocated;
	void *m_VertexPtrAligned;
	void *m_VertexPtr;
	uint m_VertexObjectId;

	uint64 m_FrameInFlight;
};

/*
// Follows design of D3D::CVolatileVertexBuffer
// A large buffer from which sequentially is allocated
// Must have one instance per in-flight frame (count NL3D_GL3_BUFFER_QUEUE_MAX)
// OR use different strategy; fix the sizes of the volatile storage buffers; and synchronize to next buffer whenever end of buffer is reached
class CVolatileBufferStorageGL3
{
public:
	CVolatileBufferGL3();
	~CVolatileBufferGL3();

	// Initialize
	//  location: Either AGPVolatile or RAMVolatile
	void init(CVertexBuffer::TLocation location, GLsizei size, GLsizei maxSize, CDriverGL3 *driver);
	void release();

	// Runtime buffer access, no-blocking lock.
	void *lock(uint size, uint stride, uint &offset);
	void unlock();

	void reset();

private:
	CDriverGL3 *m_Driver;
	GLuint *m_BufferObjectId;
	GLsizei m_Size;
	GLsizei m_MaxSize;
	GLsizei m_CurrentOffset;
	bool Locked;
	CVertexBuffer::TLocation m_Location;

};

// Buffer which makes use of driver provided CVolatileBufferStorageGL3 for storage backing
class CVolatileBufferGL3 : public IVertexBufferGL3
{
	virtual void getBufferObject(GLuint &buffer, GLintptr &offset) const; // Use glBindBufferRange instead of glBindBuffer
	// Or don't expose this, and expose a bind function targeting the right target...
};
*/

} // NLDRIVERGL3
} // NL3D

#endif // NL_DRIVER_OPENGL_VERTEX_BUFFER_H

/* End of driver_opengl_vertex_buffer.h */
