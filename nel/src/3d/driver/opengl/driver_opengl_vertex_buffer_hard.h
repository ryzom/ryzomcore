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
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
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
/** Interface to a Big block of AGP memory either throurgh NVVertexArrayRange or ATIVertexObject
 */
class IVertexArrayRange
{
public:
	IVertexArrayRange(CDriverGL *drv);
	virtual	~IVertexArrayRange();

	/// allocate a vertex array space. false if error. client must free before re-allocate.
	virtual	bool					allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)= 0;
	/// free this space.
	virtual	void					freeBlock()= 0;
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint size, CVertexBuffer *vb) =0;
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const =0;
	// Check & invalidate lost buffers. Default assume they can't be lost
	virtual void updateLostBuffers() {}
	// Get driver
	CDriverGL	*getDriver() const { return _Driver; }
	// tmp, for debug
	#ifdef NL_DEBUG
		virtual void		 dumpMappedBuffers() {}
	#endif
protected:
	CDriverGL	*_Driver;
};



// ***************************************************************************
/** Common interface for both NVidia and ATI extenstion
 *
 */
class IVertexBufferHardGL
{
public:

	IVertexBufferHardGL(CDriverGL *drv, CVertexBuffer *vb);
	virtual	~IVertexBufferHardGL();


	// ATI and NVidia have their own methods.
	virtual	void		*lock() = 0;
	virtual	void		unlock() = 0;
	virtual void		unlock(uint start, uint end) = 0;
	virtual void		*getPointer() = 0;

	virtual	void			enable() =0;
	virtual	void			disable() =0;

	virtual void		setupVBInfos(CVertexBufferInfo &vb) = 0;

	enum TVBType { NVidiaVB, ATIVB, ATIMapObjectVB, ARBVB, UnknownVB };
	// true if NVidia vertex buffer hard.
	TVBType	 VBType;
	// For Fence access. Ignored for ATI.
	bool				GPURenderingAfterFence;

	// test if buffer content is invalid. If so, no rendering should occurs (rendering should silently fail)
	bool							isInvalid() { return _Invalid; }

public:

	CVertexBuffer		*VB;

protected:
	CDriverGL			*_Driver;
	bool		 _Invalid;
};

#ifndef USE_OPENGLES

// ***************************************************************************
// ***************************************************************************
// NVidia implementation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/** Work only if ARRAY_RANGE_NV is enabled. else, only call to ctor/dtor/free() is valid.
 *	any call to allocateVB() will return NULL.
 */
class CVertexArrayRangeNVidia : public IVertexArrayRange
{
public:
	CVertexArrayRangeNVidia(CDriverGL *drv);


	/// \name Implementation
	// @{
	/// allocate a vertex array sapce. false if error. must free before re-allocate.
	virtual	bool					allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType);
	/// free this space.
	virtual	void					freeBlock();
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint size, CVertexBuffer *vb);
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const;
	// @}


	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/// active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	void			enable();
	/// disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	void			disable();

	/// free a VB allocated with allocateVB. No-op if NULL.
	void			freeVB(void	*ptr);


// *************************
private:
	void		*_VertexArrayPtr;
	uint32		_VertexArraySize;

	// Allocator.
	NLMISC::CHeapMemory		_HeapMemory;

	/// true if allocated.
	bool			allocated() const {return _VertexArrayPtr!=NULL;}
	/// Allocate a small subset of the memory. NULL if not enough mem.
	void			*allocateVB(uint32 size);

};



// ***************************************************************************
/// Work only if ARRAY_RANGE_NV is enabled.
class CVertexBufferHardGLNVidia : public IVertexBufferHardGL
{
public:

	CVertexBufferHardGLNVidia(CDriverGL *drv, CVertexBuffer *vb);
	virtual	~CVertexBufferHardGLNVidia();


	/// \name Implementation
	// @{
	virtual	void		*lock();
	virtual	void		unlock();
	virtual void		unlock(uint start, uint end);
	virtual void		*getPointer();
	virtual	void		enable();
	virtual	void		disable();
	virtual void		lockHintStatic(bool staticLock);
	virtual void		setupVBInfos(CVertexBufferInfo &vb);
	// @}


	// setup ptrs allocated by createVBHard(), and init Fence.
	void					initGL(CVertexArrayRangeNVidia *var, void *vertexPtr);


	// true if a setFence() has been done, without a finishFence(). NB: not the same thing as nglTestFenceNV()!!
	bool			isFenceSet() const {return _FenceSet;}
	// set or re-set the fence, whatever isFenceSet().
	void			setFence();
	// if(isFenceSet()), finish the fence, else no-op
	void			finishFence();
	// if fence is not set, no-op, else test nglTestFenceNV(), and update local _FenceSet flag.
	void			testFence();

	bool			getLockHintStatic() const {return _LockHintStatic;}

// *************************
private:
	CVertexArrayRangeNVidia		*_VertexArrayRange;
	void						*_VertexPtr;

	// The fence inserted in command stream
	GLuint				_Fence;
	// True if a setFence() has been done, without a finishFence().
	bool				_FenceSet;

	bool				_LockHintStatic;

};



// ***************************************************************************
// ***************************************************************************
// ATI implementation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// This is a fake value to simulate allocation on CHeapMemory for ATI mgt.
// NB: can set 0, since won't work for multiple reasons....
#define	NL3D_DRV_ATI_FAKE_MEM_START	0x10000


// ***************************************************************************
/** Work only if ATIVertexArrayObject is enabled. else, only call to ctor/dtor/free() is valid.
 *	any call to allocateVB() will return NULL.
 */
class CVertexArrayRangeATI : public IVertexArrayRange
{
public:
	CVertexArrayRangeATI(CDriverGL *drv);


	/// \name Implementation
	// @{
	/// allocate a vertex array sapce. false if error. must free before re-allocate.
	virtual	bool					allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType);
	/// free this space.
	virtual	void					freeBlock();
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint size, CVertexBuffer *vb);
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const;
	// @}


	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/** active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	 *	NB: no-op for ATI, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			enable();
	/** disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	 *	NB: no-op for ATI, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			disable();

	/// free a VB allocated with allocateVB. No-op if NULL.
	void			freeVB(void	*ptr);


	/// get Handle of the ATI buffer.
	uint			getATIVertexObjectId() const {return _VertexObjectId;}

// *************************
private:
	uint		_VertexObjectId;
	bool		_Allocated;
	uint32		_VertexArraySize;

	// Allocator. NB: We don't have any Mem Ptr For ATI extension, so use a Fake Ptr: NL3D_DRV_ATI_FAKE_MEM_START
	NLMISC::CHeapMemory		_HeapMemory;

	/// Allocate a small subset of the memory. NULL if not enough mem.
	void			*allocateVB(uint32 size);

};




// ***************************************************************************
/// Work only if ARRAY_RANGE_NV is enabled.
class CVertexBufferHardGLATI : public IVertexBufferHardGL
{
public:

	CVertexBufferHardGLATI(CDriverGL *drv, CVertexBuffer *vb);
	virtual	~CVertexBufferHardGLATI();


	/// \name Implementation
	// @{
	virtual	void		*lock();
	virtual	void		unlock();
	virtual void		unlock(uint start, uint end);
	virtual void		*getPointer();
	virtual	void		enable();
	virtual	void		disable();
	virtual void		lockHintStatic(bool staticLock);
	virtual void		setupVBInfos(CVertexBufferInfo &vb);
	// @}


	/// try to create a RAM mirror that 'll contain a copy of the VB.
	bool					createRAMMirror(uint memSize);

	/**	setup ptrs allocated by createVBHard()
	 */
	void					initGL(CVertexArrayRangeATI *var, void *vertexPtr);


public:

	/// get Handle of the ATI buffer.
	uint					getATIVertexObjectId() const {return _VertexArrayRange->getATIVertexObjectId();}


// *************************
private:
	CVertexArrayRangeATI		*_VertexArrayRange;
	void						*_VertexPtr;
	void						*_RAMMirrorVertexPtr;
	uint						_RAMMirrorVertexSize;

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** TEMP nico : test if better with ATI_map_object_buffer
  * We don't manage a heap there, we just allocate separate objects (can't get a pointer on a portion of the buffer only ..)
  * todo : test if such maneer is still efficient (because of vb switching)
  * NB : this is available only it GL_ATI_map_object_buffer is available
  */
class CVertexArrayRangeMapObjectATI : public IVertexArrayRange
{
public:
	CVertexArrayRangeMapObjectATI(CDriverGL *drv);


	/// \name Implementation
	// @{
	/** Allocate a vertex array space. false if error. must free before re-allocate.
	  * Will always succeed, because vb are not managed in a heap, but are rather kept as separate objects
	  */
	virtual	bool					allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType);
	/// free this space.
	virtual	void					freeBlock();
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint size, CVertexBuffer *vb);
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const { return _SizeAllocated; }
	// @}


	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/** active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	 *	NB: no-op for ATI, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			enable();
	/** disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	 *	NB: no-op for ATI, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			disable();

	CVertexBuffer::TPreferredMemory getVBType() const { return _VBType; }

	// check & invalidate lost buffers
	void updateLostBuffers();

	// tmp for debug
	#ifdef NL_DEBUG
		void dumpMappedBuffers();
	#endif

// *************************
private:
	CVertexBuffer::TPreferredMemory _VBType;
	uint32				 _SizeAllocated;
public:
	// for use by CVertexBufferHardGLMapObjectATI
	std::list<CVertexBufferHardGLMapObjectATI *> _LostVBList;
	#ifdef NL_DEBUG
		std::list<CVertexBufferHardGLMapObjectATI *> _MappedVBList;
	#endif
};


/** vb hard using the ATI_map_object_buffer extension. Buffer are kept separate rather than managed in a heap
  */
class CVertexBufferHardGLMapObjectATI : public IVertexBufferHardGL
{
public:

	CVertexBufferHardGLMapObjectATI(CDriverGL *drv, CVertexBuffer *vb);
	virtual	~CVertexBufferHardGLMapObjectATI();


	/// \name Implementation
	// @{
	virtual	void		*lock();
	virtual	void		unlock();
	virtual void		unlock(uint start, uint end);
	virtual void		*getPointer();
	virtual	void		enable();
	virtual	void		disable();
	virtual void		lockHintStatic(bool staticLock);
	virtual void		setupVBInfos(CVertexBufferInfo &vb);
	// @}

   /**	setup ptrs allocated by createVBHard()
	 */
	void					initGL(CVertexArrayRangeMapObjectATI *var, uint vertexObjectID);


public:

	/// get Handle of the ATI buffer.
	uint					getATIVertexObjectId() const { return _VertexObjectId;}


// *************************
private:
	void						   *_VertexPtr; // pointer on current datas. Null if not locked
	CVertexArrayRangeMapObjectATI  *_VertexArrayRange;
	// if buffer has been invalidated, returns a dummy memory block and silently fails rendering
	std::vector<uint8>				_DummyVB;
	// Invalidate the buffer (when it is lost, or when a lock fails)
	void							invalidate();
public:
	// for use by CVertexArrayRangeMapObjectATI
	std::list<CVertexBufferHardGLMapObjectATI *>::iterator _IteratorInLostVBList;
	uint						   _VertexObjectId;
	// tmp for debug
	#ifdef NL_DEBUG
		bool							_Unmapping;
		std::list<CVertexBufferHardGLMapObjectATI *>::iterator _IteratorInMappedVBList;
	#endif
};

#endif

// ***************************************************************************
// ***************************************************************************
// ARB_vertex_buffer_object implementation
// ***************************************************************************
// ***************************************************************************
class CVertexArrayRangeARB : public IVertexArrayRange
{
public:
	CVertexArrayRangeARB(CDriverGL *drv);


	/// \name Implementation
	// @{
	/** Allocate a vertex array space. false if error. must free before re-allocate.
	  * Will always succeed, because vb are not managed in a heap, but are rather kept as separate objects
	  */
	virtual	bool					allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType);
	/// free this space.
	virtual	void					freeBlock();
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint size, CVertexBuffer *vb);
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const { return _SizeAllocated; }
	// @}


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
	uint32							_SizeAllocated;
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

	CVertexBufferHardARB(CDriverGL *drv, CVertexBuffer *vb);
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
#ifdef USE_OPENGLES
	uint8							*_Buffer;
	uint32							_BufferSize;
	uint32							_LastBufferSize;
#endif
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
