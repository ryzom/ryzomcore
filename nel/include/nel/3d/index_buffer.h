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

#ifndef NL_INDEX_BUFFER_H
#define NL_INDEX_BUFFER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include <vector>

namespace NLMISC
{
	class IStream;
}

namespace NL3D {

using NLMISC::CRefCount;
using NLMISC::CRefPtr;

// List typedef.
class	IIBDrvInfos;
class	CIndexBuffer;
class	IDriver;
class	CIndexBufferReadWrite;
class	CIndexBufferRead;
typedef	std::list<IIBDrvInfos*>			TIBDrvInfoPtrList;
typedef	TIBDrvInfoPtrList::iterator		ItIBDrvInfoPtrList;


#define NL_NAMED_INDEX_BUFFER
#ifdef NL_NAMED_INDEX_BUFFER
	#define NL_SET_IB_NAME(ib, n) if ((ib).getName().empty()) (ib).setName(n);
#else
	#define NL_SET_IB_NAME(ib, n)
#endif



#define NL_FORCE_INDEX_BUFFER_16

#ifdef NL_FORCE_INDEX_BUFFER_16
	typedef uint16 TIndexType; // default index type
	#define NL_DEFAULT_INDEX_BUFFER_FORMAT CIndexBuffer::Indices16

	#define NL_MESH_INDEX16
	#define NL_MESH_MRM_INDEX16
	#define NL_SKINNED_MESH_MRM_INDEX16
	//#define NL_LANDSCAPE_INDEX16
	#define NL_LOD_CHARACTER_INDEX16
	#define NL_COARSE_MESH_INDEX16
#else
	typedef uint32 TIndexType; // default index type
	#define NL_DEFAULT_INDEX_BUFFER_FORMAT CIndexBuffer::Indices32
#endif



// 16 bit indices for meshs ?
#ifdef NL_MESH_INDEX16
	typedef uint16 TMeshIndexType;
	#define NL_MESH_INDEX_FORMAT CIndexBuffer::Indices16
#else
	typedef uint32 TMeshIndexType;
	#define NL_MESH_INDEX_FORMAT CIndexBuffer::Indices32
#endif

// 16 bit indices for mesh_mrm ?
#ifdef NL_MESH_MRM_INDEX16
	typedef uint16 TMeshMRMIndexType;
	#define NL_MESH_MRM_INDEX_FORMAT CIndexBuffer::Indices16
#else
	typedef uint32 TMeshMRMIndexType;
	#define NL_MESH_MRM_INDEX_FORMAT CIndexBuffer::Indices32
#endif

// 16 bit indices for skinned mesh_mrm ?
#ifdef NL_SKINNED_MESH_MRM_INDEX16
	typedef uint16 TSkinnedMeshMRMIndexType;
	#define NL_SKINNED_MESH_MRM_INDEX_FORMAT CIndexBuffer::Indices16
#else
	typedef uint32 TSkinnedMeshMRMIndexType;
	#define NL_SKINNED_MESH_MRM_INDEX_FORMAT CIndexBuffer::Indices32
#endif


// 16 bit indices for landscape ?
#ifdef NL_LANDSCAPE_INDEX16
	typedef uint16 TLandscapeIndexType;
	#define NL_LANDSCAPE_INDEX_FORMAT CIndexBuffer::Indices16
#else
	typedef uint32 TLandscapeIndexType;
	#define NL_LANDSCAPE_INDEX_FORMAT CIndexBuffer::Indices32
#endif

// 16 bit for lod characters ?
#ifdef NL_LOD_CHARACTER_INDEX16
	typedef uint16 TLodCharacterIndexType;
	#define NL_LOD_CHARACTER_INDEX_FORMAT CIndexBuffer::Indices16
#else
	typedef uint32 TLodCharacterIndexType;
	#define NL_LOD_CHARACTER_INDEX_FORMAT CIndexBuffer::Indices32
#endif

// 16 bit for coarse meshs ?
#ifdef NL_COARSE_MESH_INDEX16
	typedef uint16 TCoarseMeshIndexType;
	#define NL_COARSE_MESH_INDEX_FORMAT CIndexBuffer::Indices16
#else
	typedef uint32 TCoarseMeshIndexType;
	#define NL_COARSE_MESH_INDEX_FORMAT CIndexBuffer::Indices32
#endif



/**
 * An index buffer to work with the driver
 *
 * Before the index buffer is resident (IDriver::activeIndexBuffer), it is in system memory.
 * Once the index buffer is resident, the driver creates its proprietary index buffer and release the internal index buffer.
 * At this moment the index buffer can be in VRAM, AGP or system memory.
 *
 * The index buffers resident in AGP and VRAM are writeonly, i-e, you can't read them after a lock(). If you change the capacity
 * of the format of a writeonly buffer, the content is lost.
 *
 * Volatile buffers must be completely filled at each pass. They are lost after each swapBuffers(). They are writeonly.
 * Volatile buffers must be resized before the lock call. Only one lock per render must be done with volatile buffers.
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */

// All these flags are similar to DX8
class CIndexBuffer : public CRefCount
{
public:
	friend class CIndexBufferReadWrite;
	friend class CIndexBufferRead;

	/**
	  * Buffer usage hint — describes how the buffer will be written/updated.
	  * See setBufferUsage() for detailed semantics of each value.
	  */
	enum TBufferUsage
	{
		/// Written once during init, never modified afterward. Driver may place in VRAM.
		/// Suitable for static meshes. (Replaces old StaticPreferred.)
		Immutable = 0,

		/// Small trivial geometry (UI quads, debug draws). Contents undefined after
		/// swapBuffers — must fill entirely each frame unless keepLocalMemory is set.
		/// Driver may use pinned memory or client memory. (Replaces old RAMVolatile.)
		SmallStream,

		/// Full rewrite every frame, ring-buffered so lock never stalls. Contents
		/// undefined after swapBuffers — must fill entirely each frame unless
		/// keepLocalMemory is set. Standard pattern for particle systems.
		/// (Replaces old AGPVolatile.)
		FullStream,

		/// Full rewrite occasionally (not every frame). Driver orphans on lock so it
		/// never stalls. Water geometry, skinned mesh vertex streams.
		/// (Replaces old AGPPreferred for non-landscape use.)
		FullRewrite,

		/// Partial writes each frame into a static GPU buffer. Dirty pages uploaded
		/// via a staging stream. Landscape, vegetation.
		/// (Replaces old AGPPreferred for landscape/vegetation use.)
		PartialWrite,

		/// CPU-side read/write. GPU only reads. Buffer stays in system RAM.
		/// Skinned mesh source data, software transform targets.
		/// (Replaces old RAMPreferred.)
		CpuReadWrite,

		/// Caller maps unsynchronized and guarantees no write conflicts with
		/// in-flight GPU reads. Used with allocator-level deferred freeing.
		/// Fallback: PartialWrite on legacy drivers.
		UnsynchronizedWrite,

		UsageCount
	};


	/**
	  * Dirty byte range for partial buffer uploads.
	  */
	struct CDirtyRange
	{
		uint32 Begin; ///< First dirty byte offset (inclusive)
		uint32 End;   ///< Last dirty byte offset (exclusive)
	};

	/**
	  * Type of index buffer location
	  */
	enum TLocation
	{
		RAMResident = 0,
		AGPResident,
		VRAMResident,
		NotResident,
		LocationCount,
	};

	enum TFormat
	{
		Indices16 = 0,
		Indices32,
		IndicesUnknownFormat // reserved, for use by driver
	};

	/**
	  * Internal flags
	  */
	enum
	{
		/// Vertex format touched
		TouchedIndexFormat		= 1,

		/// Num vertices touched
		TouchedNumIndexes		= 2,

		/// Reserve touched
		TouchedReserve			= 4,

		/// All touhched
		TouchedAll				= 0xFFFF
	};

public:

	// \name Private. For Driver only.
	// @{
	CRefPtr<IIBDrvInfos>	DrvInfos;
	uint					getTouchFlags() const { return _InternalFlags&TouchedAll; }

	/** Used by the driver implementation. The driver must first allocate its internal buffer and fill DrvInfos. Then it has to call setLocation(true).
	  *
	  * If newLocation!=NotResident, setLocation() will copy the non resident buffer in the choosed resident memory and
	  * untouch the buffer. If the buffer preferres RAM or AGP memory, it will release the non resident memory.
	  *
	  * If newLocation==NotResident, setLocation() will realloc the non resident buffer, copy the vertex data if the buffer was resident in RAM. Then
	  * it will touch the buffer.*/
	void					setLocation (TLocation newLocation);	// The driver implementation must set the location after activeIndexBuffer.

	/** Called by the driver implementation during the buffer activation */
	void					fillBuffer ();

	/// Enable/disable dirty range accumulation. Called by the driver during buffer setup.
	void					setDirtyTracking(bool enable) { _DirtyTracking = enable; if (!enable) _DirtyRanges.clear(); }
	bool					getDirtyTracking() const { return _DirtyTracking; }

	/// Mark an index range as dirty (first inclusive, last exclusive index indices).
	/// Called by engine/user code while the buffer is locked.
	/// Ignored when dirty tracking is not enabled.
	void					invalidateRange(uint first, uint last);

	/// Read accumulated dirty byte ranges. Called by the driver during flush.
	const std::vector<CDirtyRange> &getDirtyRanges() const { return _DirtyRanges; }

	/// Clear accumulated dirty ranges. Called by the driver after uploading.
	void					clearDirtyRanges() { _DirtyRanges.clear(); }
	// @}

public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/**
	  * Default constructor. Make an empty index buffer. No value, no index. Index color format is set to TRGBA.
	  */
	CIndexBuffer(void);

	/** Make an empty index buffer. No value, no index. Index color format is set to TRGBA.
	  * Set its name to the given value
	  */
	CIndexBuffer(const char *name);

	/**
	  * Copy constructor.
	  *  Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  */
	CIndexBuffer(const CIndexBuffer &vb);

	/**
	  * Destructor.
	  */
	~CIndexBuffer(void);

	/**
	  * Copy operator.
	  * Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  * All the destination index buffer is invalidated. Data are lost.
	  * The source and destination buffers must be unlocked.
	  */
	CIndexBuffer			&operator=(const CIndexBuffer &vb);

	/**
	  * Set the buffer usage hint and keepLocalMemory flag.
	  *
	  * Usage determines how the driver manages the underlying GPU buffer:
	  *
	  * - Immutable: Written once during init, never modified. The driver may place
	  *   the buffer in VRAM. With keepLocalMemory, the shadow copy can be
	  *   re-uploaded on device-lost recovery, but the driver won't optimise for
	  *   frequent updates. Suitable for static meshes.
	  *
	  * - SmallStream: Small trivial geometry (UI quads, debug draws). Contents are
	  *   undefined after swapBuffers — the caller must fill the buffer entirely
	  *   each frame unless keepLocalMemory is set. The driver may use pinned
	  *   memory, client memory, or a small shared staging buffer.
	  *
	  * - FullStream: Full rewrite every frame, ring-buffered so lock never stalls.
	  *   Contents are undefined after swapBuffers — the caller must fill the
	  *   buffer entirely each frame unless keepLocalMemory is set. With
	  *   keepLocalMemory, if the buffer is not locked this frame the driver
	  *   re-streams the previous fill. Standard pattern for particle systems.
	  *
	  * - FullRewrite: Full rewrite occasionally (not every frame). The driver
	  *   orphans on lock so it never stalls. Water geometry, skinned mesh vertex
	  *   streams.
	  *
	  * - PartialWrite: Partial writes each frame into a static GPU buffer; dirty
	  *   pages are uploaded via a staging stream. Landscape, vegetation.
	  *
	  * - CpuReadWrite: CPU-side read/write buffer. The GPU only reads. Used for
	  *   skinned mesh source data, software transform targets. The buffer stays
	  *   in system RAM.
	  *
	  * If keepLocalMemory is true, lock() returns a local (shadow) memory pointer.
	  * The local memory is copied into resident memory during buffer activation.
	  * Only the used size (not full capacity) is copied. This allows the driver
	  * to re-stream unchanged data without the caller having to re-lock.
	  *
	  * Calling this method makes the buffer non-resident, invalidates it, and
	  * requires the buffer to be unlocked.
	  */
	void setBufferUsage (TBufferUsage usage, bool keepLocalMemory);

	/**
	  * Get the index buffer usage hint.
	  */
	TBufferUsage getBufferUsage () const { return _BufferUsage; }


	/**
	  * Get the keep local memory flag.
	  */
	bool getKeepLocalMemory () const { return _KeepLocalMemory; }

	/**
	  * Get the index buffer current location.
	  */
	TLocation getLocation () const { return _Location; }

	/**
	  * Returns if the index buffer is driver resident or not.
	  * The index buffer is resident after a call to IDriver::activeIndexBuffer().
	  */
	bool isResident () const { return (_Location != NotResident) && DrvInfos; }

	/**
	  * Set the number of active indexes. It enlarge capacity, if needed.
	  * If the new size is bigger than capacity, reserve() will be called. see reserve().
	  * If the new size is smaller than capacity, the data are keeped, the index buffer stay resident if it is resident.
	  */
	void					setNumIndexes(uint32 n);

	// Set format for indices. This discards previous indices
	void					setFormat(TFormat format);

	// Get format
	TFormat					getFormat() const { return _Format; }

	// get number of bytes per index
	uint					getIndexNumBytes() const { return _Format == Indices16 ? 2 : 4; }

	/**
	  * Get the number of active indexes.
	  */
	uint32					getNumIndexes(void) const  { return(_NbIndexes); }

	/**
	  * Reset all the indexes from memory (contReset()), so that capacity() == getNumIndexes() == 0.
	  *
	  * If the index buffer is resident in AGP or VRAM, the data are lost.
	  * The index buffer is no more resident.
	  * The index buffer is invalidated.
	  * The index buffer must be unlocked before the call.
	  */
	void					deleteAllIndexes();

	/**
	  * Reserve space for nIndexes indexes. You are allowed to write your indexes on this space.
	  *
	  * If the index buffer is resident in AGP or VRAM, the data are lost.
	  * The index buffer is no more resident.
	  * The index buffer is invalidated.
	  * The index buffer must be unlocked before the call.
	  */
	void					reserve(uint32 nIndexes);

	/**
	  * Return the number of indexes reserved.
	  */
	uint32					capacity() { return _Capacity; }

	/**
	  * If the index buffer is resident in AGP or VRAM, the data are lost.
	  * The index buffer is no more resident.
	  * The index buffer is invalidated.
	  * The index buffer must be unlocked before the call.
	  */
	void					serial(NLMISC::IStream &f);

	/**
	  * Access indexes. Multi lock is possible only if no regions are used. Each lock need an accessor to be unlocked.
	  *
	  * Lock the index buffer and return and fill an accessor object. Once the object is destroyed, the buffer in unlocked.
	  *
	  * \param accessor is the accessor object to fill
	  * \param first is the first index to be accessed. Put 0 to select all the indexes. What ever is this index,
	  * the indexices in the index buffer remain the same.
	  * \param last is the last index to be accessed + 1. Put 0 to select all the indexes.
	  */
	inline void	lock (CIndexBufferReadWrite &accessor, uint first=0, uint last=0);

	/**
	  * Read only indexes access. Multi lock is possible only if no regions are used. Each lock need an accessor to be unlocked.
	  *
	  * Lock the index buffer and return and fill an accessor object. Once the object is destroyed, the buffer in unlocked.
	  *
	  * Readonly lock will fail if the buffer is resident in AGP or VRAM.
	  *
	  * \param accessor is the accessor object to fill
	  * \param first is the first index to be accessed. Put 0 to select all the indexes. What ever is this index,
	  * the indexices in the index buffer remain the same.
	  * \param last is the last index to be accessed + 1. Put 0 to select all the indexes.
	  */
	inline void	lock (CIndexBufferRead &accessor, uint first=0, uint last=0) const;

	// Return true if the vetx buffer is locked
	bool		isLocked () const {return _LockCounter!=0;}

	// debug name
	void		setName (const std::string &name) { _Name = name; };
	const std::string &getName () const { return _Name; };

private:

	// Check locked buffers
	bool checkLockedBuffer () const { return _LockedBuffer || (!isResident() && _NonResidentIndexes.empty()); }

	/**
	  * Unlock the index buffer. Called by CIndexBufferReadWrite.
	  * \param first the first index that as been modified.
	  * \param last the last index that as been modified + 1.
	  */
	inline void	unlock (uint first, uint last);

	/**
	  * Unlock the index buffer. Called by CIndexBufferRead.
	  * \param first the first index that as been modified.
	  * \param last the last index that as been modified + 1.
	  */
	inline void	unlock () const;

	// Reset the touch flags
	void		resetTouchFlags() {_InternalFlags &= (uint16)(~TouchedAll);}

	// Force non resident memory
	void		restoreNonResidentMemory();

	// Convert current index to a serializable vector
	void buildSerialVector(std::vector<uint32> &dest) const;

	// Convert current index from a serialized vector
	void restoreFromSerialVector(const std::vector<uint32> &src);


private:
	// Internal flags
	uint16					_InternalFlags;		// Offset 18 : aligned

	// Format of indices
	TFormat					_Format;

	// Index count in the buffer
	uint32					_NbIndexes;			// Offset 20 : aligned

	// Capacity of the buffer
	uint32					_Capacity;

	// Index array
	std::vector<uint8>		_NonResidentIndexes;

	// The locked index buffer
	mutable void			*_LockedBuffer;

	// The index buffer is locked n times
	mutable uint			_LockCounter;

	// Location of the buffer
	TLocation				_Location;

	// Buffer usage hint
	TBufferUsage			_BufferUsage;

	// Resident buffer size
	uint32					_ResidentSize;

	// Keep in local memory
	bool					_KeepLocalMemory;

	// Dirty range tracking for partial uploads
	std::vector<CDirtyRange>	_DirtyRanges;
	bool					_DirtyTracking;

	// debug name
	std::string				_Name;
};

/**
  * Index buffer driver information.
  */
// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
class IIBDrvInfos : public CRefCount
{
protected:
	IDriver				*_Driver;
private:
	ItIBDrvInfoPtrList	_DriverIterator;

public:
	CRefPtr<CIndexBuffer>	IndexBufferPtr;

	IIBDrvInfos(IDriver	*drv, ItIBDrvInfoPtrList it, CIndexBuffer *ib) {_Driver= drv; _DriverIterator= it; IndexBufferPtr = ib;}

	/** Lock method.
	  * \param first is the first index to be accessed. Put 0 to select all the indexes. What ever is this index,
	  * the indexices in the index buffer remain the same.
	  * \param last is the last index to be accessed + 1. Put 0 to select all the indexes.
	  */
	virtual void	    *lock (uint first, uint last, bool readOnly) =0;

	/** Unlock method.
	  * \param first is the index of the first indexes to update. 0 to update all the indexes.
	  * \param last is the index of the last indexes to update + 1. 0 to update all the indexes.
	  */
	virtual void	unlock (uint first, uint last) =0;

	/* The virtual dtor is important.
	 * The driver implementation must call setLocation (NotResident) if IndexBufferPtr!=NULL.*/
	virtual ~IIBDrvInfos();


	CIndexBuffer::TFormat getFormat() const { nlassert(IndexBufferPtr); return IndexBufferPtr->getFormat(); }
	uint				  getIndexNumBytes() const { nlassert(IndexBufferPtr); return IndexBufferPtr->getIndexNumBytes(); }
};

/**
 * The index buffer accessor read / write
 */
class CIndexBufferReadWrite
{
public:
	friend class CIndexBuffer;

	CIndexBufferReadWrite()
	{
		_Parent = NULL;
	}
	~CIndexBufferReadWrite()
	{
		unlock();
	}

	/**
	  * Unlock the index buffer.
	  * After this call, the accessor should not be used before a new lock.
	  */
	void unlock()
	{
		if (_Parent)
		{
			_Parent->unlock(_First, _Last);
			_Parent = NULL;
		}
	}

	// Set a line index at index idx.
	inline void				setLine(uint idx, uint32 i0, uint32 i1)
	{
		nlassert (_Parent->checkLockedBuffer());
		#ifdef NL_DEBUG
			nlassert(idx < _Parent->getNumIndexes());
			nlassert(idx + 1 < _Parent->getNumIndexes());
		#endif
		if (_Parent->getFormat() == CIndexBuffer::Indices16)
		{
			#ifdef NL_DEBUG
				nlassert(i0 <= 65535);
				nlassert(i1 <= 65535);
			#endif
			((uint16 *) _Parent->_LockedBuffer)[idx] = (uint16) i0;
			((uint16 *) _Parent->_LockedBuffer)[idx+1] = (uint16) i1;
		}
		else if (_Parent->getFormat() == CIndexBuffer::Indices32)
		{
			((uint32 *) _Parent->_LockedBuffer)[idx] = i0;
			((uint32 *) _Parent->_LockedBuffer)[idx+1] = i1;
		}
		else
		{
			nlassert(0);
		}
	}

	// Set a line index at index idx.
	inline void				setTri(uint idx, uint32 i0, uint32 i1, uint32 i2)
	{
		nlassert (_Parent->checkLockedBuffer());
		#ifdef NL_DEBUG
			nlassert(idx < _Parent->getNumIndexes());
			nlassert(idx + 1 < _Parent->getNumIndexes());
			nlassert(idx + 2 < _Parent->getNumIndexes());
		#endif
		if (_Parent->getFormat() == CIndexBuffer::Indices16)
		{
			#ifdef NL_DEBUG
				nlassert(i0 <= 65535);
				nlassert(i1 <= 65535);
				nlassert(i2 <= 65535);
			#endif
			((uint16 *) _Parent->_LockedBuffer)[idx] = (uint16) i0;
			((uint16 *) _Parent->_LockedBuffer)[idx+1] = (uint16) i1;
			((uint16 *) _Parent->_LockedBuffer)[idx+2] = (uint16) i2;
		}
		else if (_Parent->getFormat() == CIndexBuffer::Indices32)
		{
			((uint32 *) _Parent->_LockedBuffer)[idx] = i0;
			((uint32 *) _Parent->_LockedBuffer)[idx+1] = i1;
			((uint32 *) _Parent->_LockedBuffer)[idx+2] = i2;
		}
		else
		{
			nlassert(0);
		}
	}

	// Get a pointer on an index.
	void *getPtr (uint idx=0)
	{
		nlassert (_Parent->checkLockedBuffer());
		if (_Parent->getFormat() == CIndexBuffer::Indices16)
		{
			return &(((uint16 *) _Parent->_LockedBuffer)[idx]);
		}
		else if (_Parent->getFormat() == CIndexBuffer::Indices32)
		{
			return &(((uint32 *) _Parent->_LockedBuffer)[idx]);
		}
		else
		{
			nlassert(0);
		}
		return NULL;
	}

	/** Touch the updated indexes. If the method is not call, the accessor update all the indexes.
	  * \param first is the index of the first indexes to update.
	  * \param last is the index of the last indexes to update + 1.
	  */
	void					touchIndexes (uint first, uint last);

	const CIndexBuffer *getParent() const { return _Parent; }

	CIndexBuffer::TFormat getFormat() const { nlassert(_Parent); return _Parent->getFormat(); }
	uint getIndexNumBytes() const { nlassert(_Parent); return _Parent->getIndexNumBytes(); }

private:

	// No copy operators available
	void		operator=(const CIndexBufferReadWrite& /* other */) {}
	CIndexBufferReadWrite(const CIndexBufferReadWrite& /* other */) {}

	CIndexBuffer		*_Parent;
	uint				_First, _Last;
};

/**
 * The index buffer read accessor
 */
class CIndexBufferRead
{
public:
	friend class CIndexBuffer;

	CIndexBufferRead()
	{
		_Parent = NULL;
	}
	~CIndexBufferRead()
	{
		unlock();
	}

	/**
	  * Unlock the index buffer.
	  * After this call, the accessor should not be used before a new lock.
	  */
	void unlock()
	{
		if (_Parent)
		{
			_Parent->unlock();
			_Parent = NULL;
		}
	}

	// Get a pointer on an index.
	const void *getPtr (uint idx=0) const
	{
		nlassert (_Parent->checkLockedBuffer());
		if (_Parent->getFormat() == CIndexBuffer::Indices16)
		{
			return &(((const uint16 *) _Parent->_LockedBuffer)[idx]);
		}
		else if (_Parent->getFormat() == CIndexBuffer::Indices32)
		{
			return &(((const uint32 *) _Parent->_LockedBuffer)[idx]);
		}
		else
		{
			nlassert(0);
		}
		return NULL;
	}
	const CIndexBuffer *getParent() const { return _Parent; }
	uint getIndexNumBytes() const { nlassert(_Parent); return _Parent->getIndexNumBytes(); }
	CIndexBuffer::TFormat getFormat() const { nlassert(_Parent); return _Parent->getFormat(); }
private:

	// No copy operators available
	void		operator=(const CIndexBufferRead& /* other */) {}
	CIndexBufferRead(const CIndexBufferRead& /* other */) {}

	const CIndexBuffer		*_Parent;
};

// ***************************************************************************

inline void CIndexBuffer::lock (CIndexBufferReadWrite &accessor, uint first, uint last)
{
	accessor.unlock();
	accessor._Parent = this;
	accessor._First = 0;
	accessor._Last = 0;

	// Already locked ?
	if (_LockCounter == 0)
	{
		nlassert (_LockedBuffer == NULL);

		// No
		if (isResident() && !_KeepLocalMemory)
		{
			if (last == 0)
				last = _NbIndexes;
			_LockedBuffer = DrvInfos->lock (first, last, false);
		}
		else
		{
			if (_NonResidentIndexes.empty())
				_LockedBuffer = NULL;
			else
				_LockedBuffer = &(_NonResidentIndexes[0]);
		}
	}
	else
	{
		nlassert ((first==0)&&(last==0));
	}

	_LockCounter++;
}

// ***************************************************************************

inline void CIndexBuffer::lock (CIndexBufferRead &accessor, uint first, uint last) const
{
	accessor.unlock();
	accessor._Parent = this;

	// Already locked ?
	if (_LockCounter == 0)
	{
		nlassert (_LockedBuffer == NULL);

		// No
		if (isResident() && !_KeepLocalMemory)
		{
			if (last == 0)
				last = _NbIndexes;
			// Can read it ?
			nlassertex (_Location==RAMResident, ("Try to read a write only index buffer"));
			_LockedBuffer = DrvInfos->lock (first, last, true);
		}
		else
		{
			if (_NonResidentIndexes.empty())
				_LockedBuffer = NULL;
			else
				_LockedBuffer = const_cast<void*>((const void *) &(_NonResidentIndexes[0]));
		}
	}
	else
	{
		nlassert ((first==0)&&(last==0));
	}

	_LockCounter++;
}

// ***************************************************************************

inline void CIndexBuffer::unlock (uint /* first */, uint /* end */)
{
	nlassertex (_LockCounter!=0, ("Index buffer not locked"));
	nlassert (_LockedBuffer || (!isResident() && _NonResidentIndexes.empty()));

	if (_LockCounter)
		_LockCounter--;

	if (_LockCounter == 0)
	{
		if (isResident() && !_KeepLocalMemory)
			DrvInfos->unlock (0, 0);

		_LockedBuffer = NULL;
	}
}

// ***************************************************************************

inline void CIndexBuffer::unlock () const
{
	nlassertex (_LockCounter!=0, ("Index buffer not locked"));
	nlassert (_LockedBuffer || (!isResident() && _NonResidentIndexes.empty()));

	if (_LockCounter)
		_LockCounter--;

	if (_LockCounter == 0)
	{
		if (isResident() && !_KeepLocalMemory)
			DrvInfos->unlock (0, 0);

		_LockedBuffer = NULL;
	}
}

// ***************************************************************************
} // NL3D


#endif // NL_INDEX_BUFFER_H

/* End of index_buffer.h */
