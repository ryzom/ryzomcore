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

#ifndef NL_DRIVER_OPENGL_VERTEX_BUFFER_HARD_H
#define NL_DRIVER_OPENGL_VERTEX_BUFFER_HARD_H

#include "nel/misc/types_nl.h"


namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

class	CDriverGL;
class	IVertexBufferHardGL;
class   CVertexBufferInfo;
class	CVertexBufferHardGLMapObjectATI;
class	CVertexBufferHardARB;

// ***************************************************************************
// ***************************************************************************
// VBHard interface for both NVidia / ATI extension.
// ***************************************************************************
// ***************************************************************************




// ***************************************************************************
/** Interface to a Big block of video memory
 */
class IVertexArrayRange
{
public:
	IVertexArrayRange(CDriverGL3 *drv);
	virtual	~IVertexArrayRange();

	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint size, CVertexBuffer *vb) =0;
	// Check & invalidate lost buffers. Default assume they can't be lost
	virtual void updateLostBuffers() {}
	// Get driver
	CDriverGL3* getDriver() const { return _Driver; }
	// tmp, for debug
	#ifdef NL_DEBUG
		virtual void		 dumpMappedBuffers() {}
	#endif
protected:
	CDriverGL3	*_Driver;
};



// ***************************************************************************
/** Common interface for both NVidia and ATI extenstion
 *
 */
class IVertexBufferHardGL
{
public:

	IVertexBufferHardGL(CDriverGL3 *drv, CVertexBuffer *vb);
	virtual	~IVertexBufferHardGL();


	// ATI and NVidia have their own methods.
	virtual	void		*lock() = 0;
	virtual	void		unlock() = 0;
	virtual void		unlock(uint start, uint end) = 0;
	virtual void		*getPointer() = 0;

	virtual	void			enable() =0;
	virtual	void			disable() =0;

	virtual void		setupVBInfos(CVertexBufferInfo &vb) = 0;

	// test if buffer content is invalid. If so, no rendering should occurs (rendering should silently fail)
	bool							isInvalid() { return _Invalid; }

public:

	CVertexBuffer		*VB;

protected:
	CDriverGL3			*_Driver;
	bool		 _Invalid;
};


// ***************************************************************************
// ***************************************************************************
// ARB_vertex_buffer_object implementation
// ***************************************************************************
// ***************************************************************************
class CVertexArrayRangeARB : public IVertexArrayRange
{
public:
	CVertexArrayRangeARB(CDriverGL3 *drv);

	virtual	IVertexBufferHardGL		*createVBHardGL(uint size, CVertexBuffer *vb);

	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/** active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	 *	NB: no-op for ARB, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			enable();
	/** disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	 *	NB: no-op for ARB, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			disable();
	// check & invalidate lost buffers
	void updateLostBuffers();
	//
	#ifdef NL_DEBUG
		virtual void		 dumpMappedBuffers();
	#endif
// *************************
private:
	CVertexBuffer::TPreferredMemory _VBType;
	// for use by CVertexBufferHardARB
public:
	std::list<CVertexBufferHardARB *> _LostVBList;
	#ifdef NL_DEBUG
		std::list<CVertexBufferHardARB *> _MappedVBList;
	#endif
};


/** vb hard using the ARB_vertex_buffer_object extension. Buffer are kept separate rather than managed in a heap
  */
class CVertexBufferHardARB : public IVertexBufferHardGL
{
public:

	CVertexBufferHardARB(CDriverGL3 *drv, CVertexBuffer *vb);
	virtual	~CVertexBufferHardARB();


	/// \name Implementation
	// @{
	virtual	void		*lock();
	virtual	void		unlock();
	virtual void		unlock(uint startVert, uint endVert);
	virtual void		*getPointer();
	virtual	void		enable();
	virtual	void		disable();
	virtual void		lockHintStatic(bool staticLock);
	virtual void		setupVBInfos(CVertexBufferInfo &vb);
	// @}

   /**	setup ptrs allocated by createVBHard()
	 */
	void					initGL(uint vertexObjectID, CVertexArrayRangeARB *var, CVertexBuffer::TPreferredMemory memType);


public:

	/// get Handle of the ARB buffer.
	uint					getARBVertexObjectId() const { return _VertexObjectId;}

	// Invalidate the buffer (when it is lost, or when a lock fails)
	void							invalidate();

	// tmp
	void checkMappedVBList();

// *************************
private:
	CVertexArrayRangeARB			*_VertexArrayRange;
	CVertexBuffer::TPreferredMemory _MemType;
	void							*_VertexPtr; // pointer on current datas. Null if not locked

	// if buffer has been invalidated, returns a dummy memory block and silently fails rendering
	std::vector<uint8>				_DummyVB;
	// for use by CVertexArrayRangeARB
	std::list<CVertexBufferHardARB *>::iterator _IteratorInLostVBList;
public:
	uint							_VertexObjectId;
	// tmp for debug
	#ifdef NL_DEBUG
		bool _Unmapping;
		std::list<CVertexBufferHardARB *>::iterator _IteratorInMappedVBList;
	#endif
};

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D


#endif // NL_DRIVER_OPENGL_VERTEX_BUFFER_HARD_H

/* End of driver_opengl_vertex_buffer_hard.h */
