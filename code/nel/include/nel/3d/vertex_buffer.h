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

#ifndef NL_VERTEX_BUFFER_H
#define NL_VERTEX_BUFFER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/debug.h"
#include "nel/misc/uv.h"
#include <vector>
#include <list>

namespace NLMISC
{
	class CUV;
}


namespace NL3D
{

using NLMISC::CRefCount;
using NLMISC::CRefPtr;
using NLMISC::CRGBA;
using NLMISC::CBGRA;
using NLMISC::CVector;
using NLMISC::CVectorD;
using NLMISC::CUV;


class	IDriver;
class	CVertexBufferReadWrite;
class	CVertexBufferRead;


// --------------------------------------------------


// List typedef.
class	IVBDrvInfos;
class CVertexBuffer;
typedef	std::list<IVBDrvInfos*>			TVBDrvInfoPtrList;
typedef	TVBDrvInfoPtrList::iterator		ItVBDrvInfoPtrList;

// ***************************************************************************
/** Describe index for palette skinning.
 * Id must be in range [0, IDriver::MaxModelMatrix [
 */
struct	CPaletteSkin
{
	uint8	MatrixId[4];

	void	serial(NLMISC::IStream &f);
};

// ***************************************************************************

/**
 * A vertex buffer to work with the driver
 *
 * Before the vertex buffer is resident (IDriver::activeVertexBuffer), it is in system memory.
 * Once the vertex buffer is resident, the driver creates its proprietary vertex buffer and release the internal vertex buffer.
 * At this moment the vertex buffer can be in VRAM, AGP or system memory.
 *
 * The vertex buffers resident in AGP and VRAM are writeonly, i-e, you can't read them after a lock(). If you change the capacity
 * of the format of a writeonly buffer, the content is lost.
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
// All these flags are similar to DX8
class CVertexBuffer : public CRefCount
{
public:
	friend class CVertexBufferReadWrite;
	friend class CVertexBufferRead;

	/**
	  * Type of preferred memory
	  */
	enum TPreferredMemory
	{
		RAMPreferred = 0,	// A block of driver RAM memory is allocated for this buffer. The buffer is read/write.
		AGPPreferred,		// A block of driver AGP memory is allocated for this buffer. The buffer is writeonly.
		StaticPreferred,	// The buffer will not be modified. A block of driver AGP or VRAM memory is allocated for this buffer. The buffer is writeonly.
		RAMVolatile,		// A block of temporary driver RAM memory will be returned by lock(). The buffer must be entirely filled after each swapBuffers(). The buffer is writeonly.
		AGPVolatile,		// A block of temporary driver AGP memory will be returned by lock(). The buffer must be entirely filled after each swapBuffers(). The buffer is writeonly.
		PreferredCount
	};

	/**
	  * Type of buffer location
	  */
	enum TLocation
	{
		RAMResident = 0,
		AGPResident,
		VRAMResident,
		NotResident,
		LocationCount
	};

	/**
	  * Value ID, there is 16 value id
	  */
	enum TValue
	{
		Position		=0,
		Normal			=1,
		TexCoord0		=2,
		TexCoord1		=3,
		TexCoord2		=4,
		TexCoord3		=5,
		TexCoord4		=6,
		TexCoord5		=7,
		TexCoord6		=8,
		TexCoord7		=9,
		PrimaryColor	=10,
		SecondaryColor	=11,
		Weight			=12,
		PaletteSkin		=13,
		Fog				=14,
		Empty			=15,
		NumValue		=16
	};

	/**
	  * Misc infos
	  */
	enum
	{
		FirstTexCoordValue = TexCoord0,
		LastTexCoordValue  = TexCoord7
	};

	/**
	  * Value flags
	  */
	enum
	{
		PositionFlag		=	1<<Position,
		NormalFlag			=	1<<Normal,
		TexCoord0Flag		=	1<<TexCoord0,
		TexCoord1Flag		=	1<<TexCoord1,
		TexCoord2Flag		=	1<<TexCoord2,
		TexCoord3Flag		=	1<<TexCoord3,
		TexCoord4Flag		=	1<<TexCoord4,
		TexCoord5Flag		=	1<<TexCoord5,
		TexCoord6Flag		=	1<<TexCoord6,
		TexCoord7Flag		=	1<<TexCoord7,
		PrimaryColorFlag	=	1<<PrimaryColor,
		SecondaryColorFlag	=	1<<SecondaryColor,
		WeightFlag			=	1<<Weight,
		PaletteSkinFlag		=	(1<<PaletteSkin)|(1<<Weight),
		FogFlag				=	1<<Fog,
		EmptyFlag			=	1<<Empty
	};



	/**
	  * Value type, there is 13 kind of value type as in DirectX8 and gl_vertex_program used in exteneded mode
	  */
	enum TType
	{
		Double1=0,	// Deprecated
		Float1,
		Short1,		// Deprecated
		Double2,	// Deprecated
		Float2,
		Short2,		// Deprecated
		Double3,	// Deprecated
		Float3,
		Short3,		// Deprecated
		Double4,	// Deprecated
		Float4,
		Short4,		// Deprecated
		UChar4,
		NumType
	};

	/**
	  * Some constants
	  */
	enum
	{
		// Max count of texture coordinates
		MaxStage = 8,

		// Max count of skinning weight
		MaxWeight = 4
	};

	/**
	  * Vertex color format
	  */
	enum TVertexColorType
	{
		TRGBA = 0,
		TBGRA = 1
	};

	/**
	  * Static array with the size in byte of each value type
	  */
	static const uint SizeType[NumType];

	/**
	  * Static array with the standard type of each value
	  */
	static const TType DefaultValueType[NumValue];

	/**
	  * Static array with the number of component of each value type
	  */
	static const uint  NumComponentsType[NumType];

	/**
	  * Internal flags
	  */
	enum
	{
		/// Vertex format touched
		TouchedVertexFormat		= 1,

		/// Num vertices touched
		TouchedNumVertices		= 2,

		/// Reserve touched
		TouchedReserve			= 4,

		/// All touhched
		TouchedAll				= 0xFFFF
	};

public:

	// \name Private. For Driver only.
	// @{
	CRefPtr<IVBDrvInfos>	DrvInfos;
	uint					getTouchFlags() const { return _InternalFlags&TouchedAll; }

	/** Used by the driver implementation. The driver must first allocate its internal buffer and fill DrvInfos. Then it has to call setLocation(true).
	  *
	  * If newLocation!=NotResident, setLocation() will copy the non resident buffer in the choosed resident memory and
	  * untouch the buffer. If the buffer preferres RAM or AGP memory, it will release the non resident memory.
	  *
	  * If newLocation==NotResident, setLocation() will realloc the non resident buffer, copy the vertex data if the buffer was resident in RAM. Then
	  * it will touch the buffer.*/
	void					setLocation (TLocation newLocation);	// The driver implementation must set the location after activeVertexBuffer.

	/** Called by the driver implementation during the buffer activation */
	void					fillBuffer ();
	// @}

public:

	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/**
	  * Default constructor. Make an empty vertex buffer. No value, no vertex. Vertex color format is set to TRGBA.
	  */
	CVertexBuffer(void);

	/**
	  * Named vertex buffer. Make an empty vertex buffer. No value, no vertex. Vertex color format is set to TRGBA.
	  */
	CVertexBuffer(const char *name);

	/**
	  * Copy constructor.
	  *  Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  */
	CVertexBuffer(const CVertexBuffer &vb);

	/**
	  * Destructor.
	  */
	~CVertexBuffer(void);

	/**
	  * Copy operator.
	  * Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  * All the destination vertex buffer is invalidated. Data are lost.
	  * The source and destination vertex buffers must be unlocked.
	  * Vertices datas are not copied
	  */
	CVertexBuffer			&operator=(const CVertexBuffer &vb);

	/** Copy a vertex buffer, including vertices data. Destination vb is not resident.
	  * May be slow if there's agp / vram readback.
	  * Initial content of the destination vertex buffer is lost.
	  * Use this to retrieve content of a write-only buffer.
	  * Using lock with CVertexBufferRead won't work (used as a foolkeeper to prevent
	  * reading of a resident buffer that is not in ram)
	  * NB : will assert when used with volatile vb
	  */
	void		copyVertices(CVertexBuffer &dest) const;
	/**
	  * Set the buffer preferred memory. Default preferred memory is RAM.
	  *
	  * Preferre RAM if the buffer is changed several times in the same render pass.
	  * Preferre AGP if the buffer is changed only one time in the same render pass.
	  * Preferre Static if the buffer is changed only one time for initialisation.
	  *
	  * If static memory is chosen, the driver will choose VRAM or AGP depending of the user configuration.
	  *
	  * If static or RAM memory is preferred, the buffer won't be lost after a driver reset.
	  *
	  * If the buffer preferres AGP memory, the buffer is lost after a driver reset. When the buffer is lost, it returns
	  * in a non resident state. The state must be tested at each pass with isResident(). If the buffer is in a
	  * non resident state, the user must refill it.
	  *
	  * If VRAM memory allocation failed, the driver will try with AGP and then with RAM.
	  * If AGP memory allocation failed, the driver will try with RAM.
	  * RAM allocation should never failed.
	  *
 	  *	Performance note:
	  *	 - for RAM CVertexBuffer, you can read / write as you like.
	  *	 - for AGP CVertexBuffer, you should write sequentially to take full advantage of the write combiners. You can't read.
	  *	 - for Static CVertexBuffer, you should write only one time, to init. You can read. Each modification will be done
	  * in a RAM buffer. Then the unlocked area will be copied into the VRAM or AGP memory.
	  *
 	  * Volatile buffers must be completely filled at each pass. They are lost after each swapBuffers(). They are writeonly.
	  * Volatile buffers must be resized before the lock call. Only one lock per render must be done with volatile buffers if
	  * keepLocalMemory is false.
	  *
	  * If keepLocalMemory is true, lock() will return a local memory pointer. The local memory will copied in resident memory
	  * during the activation of the buffer. The not all the buffer capacity is copied but only the used size.
	  *
	  * If the buffer preferres AGP memory, the data are lost.
	  * The buffer is no more resident.
	  * The buffer is invalidated.
	  * The buffer must be unlocked before the call.
	  */
	void setPreferredMemory (TPreferredMemory preferredMemory, bool keepLocalMemory);

	/**
	  * Get the vertex buffer preferred memory.
	  */
	TPreferredMemory getPreferredMemory () const { return _PreferredMemory; }

	/**
	  * Get the keep local memory flag.
	  */
	bool getKeepLocalMemory () const { return _KeepLocalMemory; }

	/**
	  * Get the vertex buffer current location.
	  */
	TLocation getLocation () const { return _Location; }

	/**
	  * Returns if the vertex buffer is driver resident or not.
	  * The vertex buffer is resident after a call to IDriver::activeVertexBuffer().
	  */
	bool isResident () const { return (_Location != NotResident) && DrvInfos; }

	/**
	  * \name Standard values vertex buffer mgt.
	  * \name Thoses methods manage the vertex buffer (position, normal, colors and uv) with standard value.
	  */
	// @{

		/**
		  * Setup the vertex format using standard values. Do it before any other standard methods.
		  *
		  * Use one or several flag between : PositionFlag, WeightFlag, NormalFlag, PrimaryColorFlag,
		  * SecondaryColorFlag, FogFlag, TexCoord0Flag, TexCoord1Flag, TexCoord2Flag,
		  * TexCoord3Flag, TexCoord4Flag, TexCoord5Flag, TexCoord6Flag, TexCoord7Flag, PaletteSkinFlag
		  *
		  * If the buffer preferres AGP memory, the data are lost.
		  * The vertex buffer is no more resident.
		  * The vertex buffer is invalidated.
		  * The vertex buffer must be unlocked before the call.
		  *
		  * If WeightFlag is specified, 4 float are used to setup the skinning value on 4 bones.
		  */
		bool					setVertexFormat (uint32 Flags);

		/**
		  * Return the vertex format used by the vertex buffer.
		  *
		  * Return one or several flags between : PositionFlag, WeightFlag, NormalFlag, PrimaryColorFlag,
		  * SecondaryColorFlag, FogFlag, TexCoord0Flag, TexCoord1Flag, TexCoord2Flag,
		  * TexCoord3Flag, TexCoord4Flag, TexCoord5Flag, TexCoord6Flag, TexCoord7Flag, PaletteSkinFlag
		  *
		  * If WeightFlag is specified, 4 float are used to setup the skinning value on 4 bones.
		  */
		uint16					getVertexFormat (void) const  { return(_Flags); };


		/// Returns the number of texture coordinate stages used by this vertex buffer
		uint					getNumTexCoordUsed() const;

		// It is an error (assert) to query a vertex offset of a vertex component not setuped in setVertexFormat().
		// NB: The Vertex offset is always 0.
		sint					getNormalOff() const {nlassert(_Flags & NormalFlag); return _Offset[Normal];}
		sint					getTexCoordOff(uint8 stage=0) const  {nlassert(_Flags & (TexCoord0Flag<<stage)); return _Offset[TexCoord0+stage]; }

		/** See getColorPointer() */
  		sint					getColorOff() const {nlassert(_Flags & PrimaryColorFlag); return _Offset[PrimaryColor];}
		/** See getColorPointer() */
		sint					getSpecularOff() const {nlassert(_Flags & SecondaryColorFlag); return _Offset[SecondaryColor];}

		/// NB: it is ensured that   WeightOff(i)==WeightOff(0)+i*sizeof(float).
		sint					getWeightOff(sint wgt) const {nlassert(_Flags & WeightFlag); return _Offset[Weight]+(wgt*sizeof(float));}
		sint					getPaletteSkinOff() const {nlassert(_Flags & PaletteSkin); return _Offset[PaletteSkin];}
	// @}

	/**
	  * \name Extended values vertex buffer mgt.
	  * \name Thoses methods manage the vertex buffer with extended value.
	  * \name This is usable only with OpenGL vertex_program or DX8 vertex shaders.
	  */
	// @{

		/**
		  * get the corresponding TValue according to the number of vertex attribute wanted (v[0], v[1] ...)
		  *	Warning!: (TValue)valueNumber != getValueIdByNumberEx(valueNumber).
		  *
		  * \param valueNumber is the value index (0..15) you want to know the valueId.
		  */
		static TValue		getValueIdByNumberEx (uint valueNumber);

		/**
		  * Clear all value in the vertex buffer. After this call, call addValue for each value you want in your vertex
		  * buffer then call initEx() to init the vertex buffer.
		  *
		  * If the buffer preferres AGP memory, the data are lost.
		  * The vertex buffer is no more resident.
		  * The vertex buffer is invalidated.
		  * The vertex buffer must be unlocked before the call.
		  */
		void				clearValueEx ();

		/**
		  * Add a value in the vertex buffer. After this call, call initEx() to init the vertex buffer.
		  *
		  * If the buffer preferres AGP memory, the data are lost.
		  * The vertex buffer is no more resident.
		  * The vertex buffer is invalidated.
		  * The vertex buffer must be unlocked before the call.
		  *
		  * \param valueId is the value id to setup.
		  * \param type is the type used for this value.
		  */
		void				addValueEx (TValue valueId, TType type);

		/// Test if the given value is present in the vertex buffer
		bool				hasValueEx(TValue valueId) const;

		/**
		  * Init the vertex buffer in extended mode.
		  *
		  * If the buffer preferres AGP memory, the data are lost.
		  * The vertex buffer is no more resident.
		  * The vertex buffer is invalidated.
		  * The vertex buffer must be unlocked before the call.
		  */
		void				initEx ();

		/**
		  * Get value offset.
		  */
		sint				getValueOffEx (TValue valueId) const { nlassert(_Flags & (1<<valueId)); return _Offset[valueId]; };

	// @}

	/**
	  * Set the number of active vertices. It enlarge capacity, if needed.
	  * If the new size is bigger than capacity, reserve() will be called. see reserve().
	  * If the new size is smaller than capacity, the data are keeped, the vertex buffer stay resident if it is resident.
	  */
	void					setNumVertices(uint32 n);

	/**
	  * Get the number of active vertices.
	  */
	uint32					getNumVertices(void) const  { return(_NbVerts); }

	/**
	  * Reset all the vertices from memory (contReset()), so that capacity() == getNumVertices() == 0.
	  *
	  * If the buffer preferres AGP memory, the data are lost.
	  * The vertex buffer is no more resident.
	  * The vertex buffer is invalidated.
	  * The vertex buffer must be unlocked before the call.
	  */
	void					deleteAllVertices();

	/**
	  * Reserve space for nVerts vertices. You are allowed to write your vertices on this space.
	  *
	  * If the buffer preferres AGP memory, the data are lost.
	  * The vertex buffer is no more resident.
	  * The vertex buffer is invalidated.
	  * The vertex buffer must be unlocked before the call.
	  */
	void					reserve(uint32 nVerts);

	/**
	  * Return the number of vertices reserved.
	  */
	uint32					capacity() { return _Capacity; }

	/**
	  * Return the size of a vertex
	  */
	uint16					getVertexSize (void) const { return(_VertexSize); }

	/**
	  * Return the type of a value
	  */
	TType					getValueType (uint value) const { nlassert (value<NumValue); return((TType)_Type[value]); }

	/**
	  * Return the type array
	  */
	const uint8				*getValueTypePointer () const { return _Type; }

	/**
	  * Return number of weight value in vertices
	  */
	uint8					getNumWeight () const;

	/**
	  * If the buffer preferres AGP memory, the data are lost.
	  * The vertex buffer is no more resident.
	  * The vertex buffer is invalidated.
	  * The vertex buffer must be unlocked before the call.
	  */
	void		serial(NLMISC::IStream &f);

	/**
	  * Access vertices. Multi lock is possible only if no regions are used. Each lock need an accessor to be unlocked.
	  *
	  * Lock the vertex buffer and return and fill an accessor object. Once the object is destroyed, the buffer in unlocked.
	  *
	  * \param accessor is the accessor object to fill
	  * \param first is the first vertex to be accessed. Put 0 to select all the vertices. What ever is this index,
	  * the indexices in the vertex buffer remain the same.
	  * \param last is the last vertex to be accessed + 1. Put 0 to select all the vertices.
	  */
	inline void	lock (CVertexBufferReadWrite &accessor, uint first=0, uint last=0);

	/**
	  * Read only vertices access. Multi lock is possible only if no regions are used. Each lock need an accessor to be unlocked.
	  *
	  * Lock the vertex buffer and return and fill an accessor object. Once the object is destroyed, the buffer in unlocked.
	  *
	  * Readonly lock will fail if the buffer is resident in AGP or VRAM.
	  *
	  * \param accessor is the accessor object to fill
	  * \param first is the first vertex to be accessed. Put 0 to select all the vertices. What ever is this index,
	  * the indexices in the vertex buffer remain the same.
	  * \param last is the last vertex to be accessed + 1. Put 0 to select all the vertices.
	  */
	inline void	lock (CVertexBufferRead &accessor, uint first=0, uint last=0) const;

	// Return true if the vetx buffer is locked
	bool		isLocked () const {return _LockCounter!=0;}

	/// \name Lod VB serialisation.
	// @{
	/// just read/write the VertexFormat of the VB, number of vertices .... If read, VB is resized to numVertices.
	void		serialHeader(NLMISC::IStream &f);
	/** just read/write a subset of the vertex buffer. NB: because a version is serialised by subset, you must read
	 * same subset of vertices you have saved. (can't write 0-10, 10-20, and after load 0-5, 5-20).
	 */
	void		serialSubset(NLMISC::IStream &f, uint vertexStart, uint vertexEnd);
	// @}

	/// \name UV Routing.
	const uint8	*getUVRouting () const { return _UVRouting; }
	void		setUVRouting (uint8 uvChannel, uint newUVRouting) { _UVRouting[uvChannel] = uint8(newUVRouting); }

	/**
	  * Set the vertex color format. If the vertex buffer is not in this format, the colors will be converted
	  * See getColorPointer()
	  *
	  * The vertex buffer must not be resident.
	  *
	  * The vertex buffer is invalidated.
	  * The vertex buffer must be unlocked.
	  * \return false if the vertex buffer is resident.
	  */
	bool		setVertexColorFormat (TVertexColorType format);

	/**
	  * Get the vertex color format. See getColorPointer()
	  */
	TVertexColorType	getVertexColorFormat () const
	{
		return (TVertexColorType)_VertexColorFormat;
	}

	// for debug : dump format of vertex buffer
	void		dumpFormat() const;
	void		setName (const std::string &name) { _Name = name; };
	const std::string &getName () const { return _Name; };

private:

	void construct();

	// Check locked buffers
	bool checkLockedBuffer () const { return _LockedBuffer || (!isResident() && _NonResidentVertices.empty()); }

	/**
	  * Unlock the vertex buffer. Called by CVertexBufferReadWrite.
	  * \param first the first vertex that as been modified.
	  * \param last the last vertex that as been modified + 1.
	  */
	inline void	unlock (uint first, uint last);

	/**
	  * Unlock the vertex buffer. Called by CVertexBufferRead.
	  * \param first the first vertex that as been modified.
	  * \param last the last vertex that as been modified + 1.
	  */
	inline void	unlock () const;

	/// Old version serialisation. V0 and V1.
	void		serialOldV1Minus(NLMISC::IStream &f, sint ver);

	/// Translate old flags
	uint16		remapV2Flags (uint32 oldFlags, uint& weightCount);

	// Reset the touch flags
	void		resetTouchFlags() {_InternalFlags &= (uint16)(~TouchedAll);}

	// Force non resident memory
	void		restoreNonResidentMemory();

private:

	// Type of data stored in each value
	uint8					_Type[NumValue];	// Offset 0 : aligned
	uint8					_VertexColorFormat;	// Offset 13 : aligned

	// Size of the vertex (sum of the size of each value
	uint16					_VertexSize;		// Offset 14 : aligned

	// Flags: bit #n is 1 if the value #n is used
	uint16					_Flags;				// Offset 16 : aligned

	// Internal flags
	uint16					_InternalFlags;		// Offset 18 : aligned

	// Vertex count in the buffer
	uint32					_NbVerts;			// Offset 20 : aligned

	// Capacity of the buffer
	uint32					_Capacity;

	// Vertex array
	std::vector<uint8>		_NonResidentVertices;

	// The locked vertex buffer
	mutable uint8*			_LockedBuffer;

	// Offset of each value
	uint16					_Offset[NumValue];

	// The UV routing table
	uint8					_UVRouting[MaxStage];

	// The vertex buffer is locked n times
	mutable uint			_LockCounter;

	// Prefered memory
	TPreferredMemory		_PreferredMemory;

	// Location of the buffer
	TLocation				_Location;

	// Resident buffer size
	uint32					_ResidentSize;

	// Debug string
	std::string				_Name;

	// Keep in local memory
	bool					_KeepLocalMemory;
};

// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
class IVBDrvInfos : public CRefCount
{
protected:
	IDriver				*_Driver;
private:
	ItVBDrvInfoPtrList	_DriverIterator;

public:
	CRefPtr<CVertexBuffer>	VertexBufferPtr;

	IVBDrvInfos(IDriver	*drv, ItVBDrvInfoPtrList it, CVertexBuffer *vb) {_Driver= drv; _DriverIterator= it; VertexBufferPtr=vb;}

	/** Lock method. Lock the region between begin and end (in bytes).
	  * What ever begin is, the returned pointer always points on the index 0.
	  */
	virtual uint8	*lock (uint begin, uint end, bool readOnly) =0;

	/** Unlock method. Unlock the region between begin and end (in bytes).
	  */
	virtual void	unlock (uint begin, uint end) =0;

	/* The virtual dtor is important.
	 * The driver implementation must call setLocation (NotResident) if VertexBufferPtr!=NULL.*/
	virtual ~IVBDrvInfos();
};

/**
 * The vertex buffer accessor read / write
 */
class CVertexBufferReadWrite
{
public:
	friend class CVertexBuffer;

	CVertexBufferReadWrite()
	{
		_Parent = NULL;
	}
	~CVertexBufferReadWrite()
	{
		unlock();
	}

	/**
	  * Unlock the vertex buffer.
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

	/* Set a value into the vertex buffer. */
	inline void				setVertexCoord(uint idx, float x, float y, float z);
	inline void				setVertexCoord(uint idx, const CVector &v);
	inline void				setNormalCoord(uint idx, const CVector &v);
	inline void				setTexCoord(uint idx, uint8 stage, float u, float v);
	inline void				setTexCoord(uint idx, uint8 stage, const CUV &uv);
	inline void				setColor(uint idx, CRGBA rgba);
	inline void				setSpecular(uint idx, CRGBA rgba);
	inline void				setWeight(uint idx, uint8 wgt, float w);
	inline void				setPaletteSkin(uint idx, CPaletteSkin ps);
	inline void				setValueFloat1Ex (CVertexBuffer::TValue valueId, uint idx, float value);
	inline void				setValueFloat2Ex (CVertexBuffer::TValue valueId, uint idx, float x, float y);
	inline void				setValueFloat3Ex (CVertexBuffer::TValue valueId, uint idx, float x, float y, float z);
	inline void				setValueFloat3Ex (CVertexBuffer::TValue valueId, uint idx, const CVector& vector);
	inline void				setValueFloat4Ex (CVertexBuffer::TValue valueId, uint idx, float x, float y, float z, float w);
	inline void				setValueUChar4Ex (CVertexBuffer::TValue valueId, uint idx, CRGBA rgba);

	/** Get a pointer on a value.
	 *
	 *  For Color pointers :
	 *  This method returns a (CRGBA*) or a (CBGRA*) regarding the vertex color format returned by CVertexBufferReadWrite::getVertexColorFormat().
	 *	A call to IDriver::activeVertexBuffer() will change this format to the format returned by IDriver::getVertexColorFormat().
	 *	So, before each write of vertex color in the vertex buffer, the vertex color format must be checked with CVertexBuffer::getVertexColorFormat().
	 */
	NLMISC::CVector*		getVertexCoordPointer(uint idx=0);
	NLMISC::CVector*		getNormalCoordPointer(uint idx=0);
	NLMISC::CUV*			getTexCoordPointer(uint idx=0, uint8 stage=0);
	void*					getColorPointer(uint idx=0);
	void*					getSpecularPointer(uint idx=0);
	float*					getWeightPointer(uint idx=0, uint8 wgt=0);
	CPaletteSkin*			getPaletteSkinPointer(uint idx=0);
	void*					getValueEx (CVertexBuffer::TValue valueId, uint idx=0) { nlassert (_Parent->checkLockedBuffer()); nlassert (_Parent->_Flags & (1<<valueId));	return (void*)(_Parent->_LockedBuffer+idx*_Parent->_VertexSize+_Parent->getValueOffEx (valueId)); }

	/** Touch the updated vertices. If the method is not call, the accessor update all the vertices.
	  * \param first is the index of the first vertices to update.
	  * \param last is the index of the last vertices to update + 1.
	  */
	void					touchVertices (uint first, uint last);

	const CVertexBuffer *getParent() const { return _Parent; }

private:

	// No copy operators available
	void		operator=(const CVertexBufferReadWrite& /* other */) {}
	CVertexBufferReadWrite(const CVertexBufferReadWrite& /* other */) {}

	CVertexBuffer		*_Parent;
	uint				_First, _Last;
};

/**
 * The vertex buffer read accessor
 */
class CVertexBufferRead
{
public:
	friend class CVertexBuffer;

	CVertexBufferRead()
	{
		_Parent = NULL;
	}
	~CVertexBufferRead()
	{
		unlock();
	}

	/**
	  * Unlock the vertex buffer.
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

	/** Get a pointer on a value.
	 *
	 *  For Color pointers :
	 *  This method returns a (CRGBA*) or a (CBGRA*) regarding the vertex color format returned by CVertexBufferRead::getVertexColorFormat().
	 *	A call to IDriver::activeVertexBuffer() will change this format to the format returned by IDriver::getVertexColorFormat().
	 *	So, before each write of vertex color in the vertex buffer, the vertex color format must be checked with CVertexBuffer::getVertexColorFormat().
	 */
	const NLMISC::CVector*	getVertexCoordPointer(uint idx=0) const;
	const NLMISC::CVector*	getNormalCoordPointer(uint idx=0) const;
	const NLMISC::CUV*		getTexCoordPointer(uint idx=0, uint8 stage=0) const;
	const void*				getColorPointer(uint idx=0) const;
	const void*				getSpecularPointer(uint idx=0) const;
	const float*			getWeightPointer(uint idx=0, uint8 wgt=0) const;
	const CPaletteSkin*		getPaletteSkinPointer(uint idx=0) const;
	const void*				getValueEx (CVertexBuffer::TValue valueId, uint idx=0) const { nlassert (_Parent->_Flags & (1<<valueId));	return (void*)(_Parent->_LockedBuffer+idx*_Parent->_VertexSize+_Parent->getValueOffEx (valueId)); }

	const CVertexBuffer *getParent() const { return _Parent; }

private:

	// No copy operators available
	void		operator=(const CVertexBufferRead& /* other */) {}
	CVertexBufferRead(const CVertexBufferRead& /* other */) {}

	const CVertexBuffer		*_Parent;
};

//////////////////////////////////////
// implementation of inline methods //
//////////////////////////////////////
// --------------------------------------------------

inline void CVertexBufferReadWrite::setVertexCoord(uint idx, float x, float y, float z)
{
	float*	ptr;

	nlassert (_Parent->checkLockedBuffer());
	nlassert (_Parent->_Flags & CVertexBuffer::PositionFlag);
	nlassert (_Parent->_Type[CVertexBuffer::Position]==CVertexBuffer::Float3);

	ptr=(float*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	*ptr=x;
	ptr++;
	*ptr=y;
	ptr++;
	*ptr=z;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setVertexCoord(uint idx, const CVector &v)
{
	uint8*	ptr;

	nlassert (_Parent->checkLockedBuffer());
	nlassert (_Parent->_Flags & CVertexBuffer::PositionFlag);
	nlassert (_Parent->_Type[CVertexBuffer::Position]==CVertexBuffer::Float3);

	ptr=&_Parent->_LockedBuffer[idx*_Parent->_VertexSize];
	memcpy(ptr, &(v.x), 3*sizeof(float));
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setNormalCoord(uint idx, const CVector &v)
{
	uint8*	ptr;

	nlassert (_Parent->checkLockedBuffer());
	nlassert (_Parent->_Flags & CVertexBuffer::NormalFlag);
	nlassert (_Parent->_Type[CVertexBuffer::Normal]==CVertexBuffer::Float3);

	ptr=&_Parent->_LockedBuffer[idx*_Parent->_VertexSize];
	ptr+=_Parent->_Offset[CVertexBuffer::Normal];
	memcpy(ptr, &(v.x), 3*sizeof(float));
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setColor(uint idx, CRGBA rgba)
{
	uint8*	ptr;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(_Parent->_Flags & CVertexBuffer::PrimaryColorFlag);
	nlassert (_Parent->_Type[CVertexBuffer::PrimaryColor]==CVertexBuffer::UChar4);

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::PrimaryColor];
	if (_Parent->getVertexColorFormat () == CVertexBuffer::TRGBA)
		*(CRGBA*)ptr = rgba;
	else
		*(CBGRA*)ptr = rgba;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setSpecular(uint idx, CRGBA rgba)
{
	uint8*	ptr;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(_Parent->_Flags & CVertexBuffer::SecondaryColorFlag);
	nlassert (_Parent->_Type[CVertexBuffer::SecondaryColor]==CVertexBuffer::UChar4);

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::SecondaryColor];
	if (_Parent->getVertexColorFormat () == CVertexBuffer::TRGBA)
		*(CRGBA*)ptr = rgba;
	else
		*(CBGRA*)ptr = rgba;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setTexCoord(uint idx, uint8 stage, float u, float v)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(stage<CVertexBuffer::MaxStage);
	nlassert(_Parent->_Flags & (CVertexBuffer::TexCoord0Flag<<stage));
	nlassert (_Parent->_Type[CVertexBuffer::TexCoord0+stage]==CVertexBuffer::Float2);

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::TexCoord0+stage];
	ptrf=(float*)ptr;
	*ptrf=u;
	ptrf++;
	*ptrf=v;
}

// --------------------------------------------------

inline void	CVertexBufferReadWrite::setTexCoord(uint idx, uint8 stage, const CUV &uv)
{
	uint8*	ptr;
	CUV*	ptruv;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(stage<CVertexBuffer::MaxStage);
	nlassert(_Parent->_Flags & (CVertexBuffer::TexCoord0Flag<<stage));
	nlassert (_Parent->_Type[CVertexBuffer::TexCoord0+stage]==CVertexBuffer::Float2);

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::TexCoord0+stage];
	ptruv=(CUV*)ptr;
	*ptruv=uv;
}


// --------------------------------------------------

inline void CVertexBufferReadWrite::setWeight(uint idx, uint8 wgt, float w)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(wgt<CVertexBuffer::MaxWeight);
	nlassert(_Parent->_Flags & (CVertexBuffer::WeightFlag));

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::Weight]+sizeof(float)*wgt;
	ptrf=(float*)ptr;
	*ptrf=w;
}

// --------------------------------------------------

inline void	CVertexBufferReadWrite::setPaletteSkin(uint idx, CPaletteSkin ps)
{
	uint8*	ptr;
	CPaletteSkin	*pPalSkin;

	nlassert (_Parent->checkLockedBuffer());
	nlassert ( (_Parent->_Flags & CVertexBuffer::PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag);
	nlassert (_Parent->_Type[CVertexBuffer::PaletteSkin]==CVertexBuffer::UChar4);

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::PaletteSkin];
	pPalSkin= (CPaletteSkin*)ptr;
	*pPalSkin= ps;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setValueFloat1Ex (CVertexBuffer::TValue valueId, uint idx, float value)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(valueId<CVertexBuffer::NumValue);
	nlassert(_Parent->_Flags & (1<<(uint)valueId));
	nlassert((_Parent->_Type[valueId]==CVertexBuffer::Float4)||(_Parent->_Type[valueId]==CVertexBuffer::Float3)||(_Parent->_Type[valueId]==CVertexBuffer::Float2)||(_Parent->_Type[valueId]==CVertexBuffer::Float1));

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[valueId];
	ptrf=(float*)ptr;
	*ptrf=value;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setValueFloat2Ex (CVertexBuffer::TValue valueId, uint idx, float x, float y)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(valueId<CVertexBuffer::NumValue);
	nlassert(_Parent->_Flags & (1<<(uint)valueId));
	nlassert((_Parent->_Type[valueId]==CVertexBuffer::Float4)||(_Parent->_Type[valueId]==CVertexBuffer::Float3)||(_Parent->_Type[valueId]==CVertexBuffer::Float2));

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[valueId];
	ptrf=(float*)ptr;
	ptrf[0]=x;
	ptrf[1]=y;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setValueFloat3Ex (CVertexBuffer::TValue valueId, uint idx, float x, float y, float z)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(valueId<CVertexBuffer::NumValue);
	nlassert(_Parent->_Flags & (1<<(uint)valueId));
	nlassert((_Parent->_Type[valueId]==CVertexBuffer::Float4)||(_Parent->_Type[valueId]==CVertexBuffer::Float3));

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[valueId];
	ptrf=(float*)ptr;
	ptrf[0]=x;
	ptrf[1]=y;
	ptrf[2]=z;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setValueFloat3Ex (CVertexBuffer::TValue valueId, uint idx, const NLMISC::CVector& theVector)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(valueId<CVertexBuffer::NumValue);
	nlassert(_Parent->_Flags & (1<<(uint)valueId));
	nlassert((_Parent->_Type[valueId]==CVertexBuffer::Float4)||(_Parent->_Type[valueId]==CVertexBuffer::Float3));

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[valueId];
	ptrf=(float*)ptr;
	memcpy (ptrf, &theVector, sizeof(float)*3);
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setValueFloat4Ex (CVertexBuffer::TValue valueId, uint idx, float x, float y, float z, float w)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(valueId<CVertexBuffer::NumValue);
	nlassert(_Parent->_Flags & (1<<(uint)valueId));
	nlassert(_Parent->_Type[valueId]==CVertexBuffer::Float4);

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[valueId];
	ptrf=(float*)ptr;
	ptrf[0]=x;
	ptrf[1]=y;
	ptrf[2]=z;
	ptrf[3]=w;
}

// --------------------------------------------------

inline void CVertexBufferReadWrite::setValueUChar4Ex (CVertexBuffer::TValue valueId, uint idx, CRGBA rgba)
{
	uint8*	ptr;
	CRGBA*	ptrr;

	nlassert (_Parent->checkLockedBuffer());
	nlassert(valueId<CVertexBuffer::NumValue);
	nlassert(_Parent->_Flags & (1<<(uint)valueId));
	nlassert(_Parent->_Type[valueId]==CVertexBuffer::UChar4);

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[valueId];
	ptrr=(CRGBA*)ptr;
	*ptrr=rgba;
}

// --------------------------------------------------

inline void CVertexBuffer::lock (CVertexBufferReadWrite &accessor, uint first, uint last)
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
				last = _NbVerts;
			_LockedBuffer = DrvInfos->lock (first*_VertexSize, last*_VertexSize, false);
		}
		else
		{
			if (_NonResidentVertices.empty())
				_LockedBuffer = NULL;
			else
				_LockedBuffer = &(_NonResidentVertices[0]);
		}
	}
	else
	{
		nlassert ((first==0)&&(last==0));
	}

	_LockCounter++;
}

// --------------------------------------------------

inline void CVertexBuffer::lock (CVertexBufferRead &accessor, uint first, uint last) const
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
				last = _NbVerts;
			// Can read it ?
			nlassertex (_Location==RAMResident, ("Try to read a write only vertex buffer"));
			_LockedBuffer = DrvInfos->lock (first*_VertexSize, last*_VertexSize, true);
		}
		else
		{
			if (_NonResidentVertices.empty())
				_LockedBuffer = NULL;
			else
				_LockedBuffer = const_cast<uint8*>(&(_NonResidentVertices[0]));
		}
	}
	else
	{
		nlassert ((first==0)&&(last==0));
	}

	_LockCounter++;
}

// --------------------------------------------------

inline void CVertexBuffer::unlock (uint /* first */, uint /* end */)
{
	nlassertex (_LockCounter!=0, ("Vertex buffer not locked"));
	nlassert (_LockedBuffer || (!isResident() && _NonResidentVertices.empty()));

	if (_LockCounter)
		_LockCounter--;

	if (_LockCounter == 0)
	{
		if (isResident() && !_KeepLocalMemory)
			DrvInfos->unlock (0, 0);

		_LockedBuffer = NULL;
	}
}

// --------------------------------------------------

inline void CVertexBuffer::unlock () const
{
	nlassertex (_LockCounter!=0, ("Vertex buffer not locked"));
	nlassert (_LockedBuffer || (!isResident() && _NonResidentVertices.empty()));

	if (_LockCounter)
		_LockCounter--;

	if (_LockCounter == 0)
	{
		if (isResident() && !_KeepLocalMemory)
			DrvInfos->unlock (0, 0);

		_LockedBuffer = NULL;
	}
}

// --------------------------------------------------

} // NL3D


#endif // NL_VERTEX_BUFFER_H

/* End of vertex_buffer.h */





















