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

#ifndef NL_DRIVER_OPENGL_VERTEX_BUFFER_H
#define NL_DRIVER_OPENGL_VERTEX_BUFFER_H

#include "nel/misc/types_nl.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

class CDriverGL3;
class IVertexBufferGL3;
class CVertexBufferInfo;
class CVertexBufferGL3;

class IVertexBufferGL3
{
public:
	enum TVBType { GL3 };

	IVertexBufferGL3(CDriverGL3 *drv, CVertexBuffer *vb, TVBType vbType);
	virtual	~IVertexBufferGL3();

	virtual	void *lock() = 0;
	virtual	void unlock() = 0;
	virtual void unlock(uint start, uint end) = 0;
	virtual void *getPointer() = 0;
	virtual	void enable() = 0;
	virtual	void disable() = 0;
	virtual void setupVBInfos(CVertexBufferInfo &vb) = 0;

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
	CVertexBufferGL3(CDriverGL3 *drv, CVertexBuffer *vb);
	virtual	~CVertexBufferGL3();

	/// \name Implementation
	// @{
	virtual	void *lock();
	virtual	void unlock();
	virtual void unlock(uint startVert, uint endVert);
	virtual void *getPointer();
	virtual	void enable();
	virtual	void disable();
	virtual void setupVBInfos(CVertexBufferInfo &vb);
	// @}

	/// Setup ptrs allocated by createVBHard()
	void initGL(uint vertexObjectID, CVertexBuffer::TPreferredMemory memType);

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

	uint m_VertexObjectId;
};

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

#endif // NL_DRIVER_OPENGL_VERTEX_BUFFER_H

/* End of driver_opengl_vertex_buffer.h */
