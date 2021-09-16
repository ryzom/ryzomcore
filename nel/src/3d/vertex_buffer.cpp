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

#include "std3d.h"

#include "nel/3d/vertex_buffer.h"
#include "nel/misc/vector.h"
#include "nel/misc/fast_mem.h"
#include "nel/3d/driver.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// --------------------------------------------------

const uint CVertexBuffer::SizeType[NumType]=
{
	1*sizeof(double),
	1*sizeof(float),
	1*sizeof(short),
	2*sizeof(double),
	2*sizeof(float),
	2*sizeof(short),
	3*sizeof(double),
	3*sizeof(float),
	3*sizeof(short),
	4*sizeof(double),
	4*sizeof(float),
	4*sizeof(short),
	4*sizeof(char),
};


const uint CVertexBuffer::NumComponentsType[NumType] =
{
	1,
	1,
	1,
	2,
	2,
	2,
	3,
	3,
	3,
	4,
	4,
	4,
	4
};


// --------------------------------------------------

const CVertexBuffer::TType CVertexBuffer::DefaultValueType[NumValue]=
{
	Float3,		// Position
	Float3,		// Normal
	Float2,		// TexCoord0
	Float2,		// TexCoord1
	Float2,		// TexCoord2
	Float2,		// TexCoord3
	Float2,		// TexCoord4
	Float2,		// TexCoord5
	Float2,		// TexCoord6
	Float2,		// TexCoord7
	UChar4,		// Primary color
	UChar4,		// Secondary color
	Float4,		// 4 Weights
	UChar4,		// PaletteSkin
	Float1,		// Fog
	Float1,		// Empty
};


// --------------------------------------------------

void CVertexBuffer::construct()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	_Flags = 0;
	_Capacity = 0;
	_NbVerts = 0;
	_InternalFlags = 0;
	_VertexSize = 0;
	_VertexColorFormat = TRGBA;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	_Location = NotResident;
	_ResidentSize = 0;
	_KeepLocalMemory = false;

	// Default routing
	uint i;
	for (i=0; i<MaxStage; i++)
		_UVRouting[i] = i;
}

// --------------------------------------------------

CVertexBuffer::CVertexBuffer()
{
	construct();
}

CVertexBuffer::CVertexBuffer(const char *name)
{
	construct();
	_Name = name;
}


// --------------------------------------------------

CVertexBuffer::CVertexBuffer(const CVertexBuffer &vb) : CRefCount()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_Flags = 0;
	_Capacity = 0;
	_NbVerts = 0;
	_VertexSize = 0;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	_Location = NotResident;
	_ResidentSize = 0;
	_KeepLocalMemory = false;

	operator=(vb);

	// Default routing
	uint i;
	for (i=0; i<MaxStage; i++)
		_UVRouting[i] = i;
}

// --------------------------------------------------

CVertexBuffer::~CVertexBuffer()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	if (DrvInfos)
		DrvInfos->VertexBufferPtr = NULL;	// Tell the driver info to not restore memory when it will die

	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

// --------------------------------------------------

CVertexBuffer	&CVertexBuffer::operator=(const CVertexBuffer &vb)
{
	nlassertex (!isLocked(), ("The vertex buffer is locked."));
	nlassertex (!vb.isLocked(), ("Source buffer is locked."));

	// Single value
	_VertexSize = vb._VertexSize;
	_Flags = vb._Flags;
	_InternalFlags = vb._InternalFlags;
	_NbVerts = vb._NbVerts;
	_Capacity = vb._Capacity;
	_NonResidentVertices = vb._NonResidentVertices;
	_VertexColorFormat = vb._VertexColorFormat;
	_PreferredMemory = vb._PreferredMemory;
	_KeepLocalMemory = vb._KeepLocalMemory;
	uint i;
	_LockCounter = 0;
	_LockedBuffer = NULL;

	// Arraies
	for (uint value=0; value<NumValue; value++)
	{
		_Offset[value]= vb._Offset[value];
		_Type[value]= vb._Type[value];
	}

	// Copy the routing
	for (i=0; i<MaxStage; i++)
		_UVRouting[i] = vb._UVRouting[i];

	// Set touch flags
	_InternalFlags |= TouchedAll;
	_Location = NotResident;
	_ResidentSize = 0;

	return *this;
}

// --------------------------------------------------
void CVertexBuffer::copyVertices(CVertexBuffer &dest) const
{
	nlassert(_PreferredMemory != RAMVolatile);
	nlassert(_PreferredMemory != AGPVolatile);
	// copy setup
	dest = *this;
	CVertexBufferReadWrite srcDatas;
	const_cast<CVertexBuffer *>(this)->lock(srcDatas);
	nlassert(dest.getLocation() == NotResident);
	CVertexBufferReadWrite destDatas;
	dest.lock(destDatas); // will be in vram
	NLMISC::CFastMem::memcpy (destDatas.getVertexCoordPointer(), srcDatas.getVertexCoordPointer(), getVertexSize() * getNumVertices());
}

// --------------------------------------------------

bool CVertexBuffer::setVertexFormat(uint32 flags)
{
	nlassertex (!isLocked(), ("The vertex buffer is locked."));

	uint	i;

	// Clear extended values
	clearValueEx ();

	// Position ?
	if (flags & PositionFlag)
	{
		// Add a standard position value
		addValueEx (Position, Float3);
	}

	// Normal ?
	if (flags & NormalFlag)
	{
		// Add a standard normal value
		addValueEx (Normal, Float3);
	}

	// For each uv values
	for(i=0 ; i<MaxStage ; i++)
	{
		// UV ?
		if (flags & (TexCoord0Flag<<i))
		{
			// Add a standard uv value
			addValueEx ((TValue)(TexCoord0+i), Float2);
		}
	}

	// Fog ?
	if (flags & FogFlag)
	{
		// Add a standard primary color value
		addValueEx (Fog, Float1);
	}

	// Primary color ?
	if (flags & PrimaryColorFlag)
	{
		// Add a standard primary color value
		addValueEx (PrimaryColor, UChar4);
	}

	// Secondary color ?
	if (flags & SecondaryColorFlag)
	{
		// Add a standard primary color value
		addValueEx (SecondaryColor, UChar4);
	}

	// Weight ?
	if (flags & WeightFlag)
	{
		// Add a standard primary color value
		addValueEx (Weight, Float4);
	}

	// Palette skin ?
	if ((flags & PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag)
	{
		// Add a standard primary color value
		addValueEx (PaletteSkin, UChar4);
	}

	// Compute the vertex buffer
	initEx ();

	// Force non resident
	restoreNonResidentMemory();

	return (true);
}

// --------------------------------------------------

CVertexBuffer::TValue CVertexBuffer::getValueIdByNumberEx (uint valueNumber)
{
	// See NV_vertex_program spec, or driver_opengl_vertex.cpp:: GLVertexAttribIndex.
	static	TValue	lut[16]= {
		Position,
		Weight,
		Normal,
		PrimaryColor,
		SecondaryColor,
		Fog,
		PaletteSkin,
		Empty,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
	};

	return lut[valueNumber];
}

// --------------------------------------------------

void CVertexBuffer::clearValueEx ()
{
	nlassertex (!isLocked(), ("The vertex buffer is locked."));

	// Reset format flags
	_Flags=0;
}


// --------------------------------------------------

void CVertexBuffer::dumpFormat() const
{
	for(uint k = 0; k < NumValue; ++k)
	{
		if (_Flags & (1 << k))
		{
			std::string result = "Component :";
			switch(k)
			{
				case Position:      result += "Position"; break;
				case Normal:        result += "Normal"; break;
				case TexCoord0:		result += "TexCoord0"; break;
				case TexCoord1:		result += "TexCoord1"; break;
				case TexCoord2:		result += "TexCoord2"; break;
				case TexCoord3:		result += "TexCoord3"; break;
				case TexCoord4:		result += "TexCoord4"; break;
				case TexCoord5:		result += "TexCoord5"; break;
				case TexCoord6:		result += "TexCoord6"; break;
				case TexCoord7:		result += "TexCoord7"; break;
				case PrimaryColor:	result += "PrimaryColor"; break;
				case SecondaryColor:result += "SecondaryColor"; break;
				case Weight:		result += "Weight"; break;
				case PaletteSkin:	result += "PaletteSkin"; break;
				case Fog:			result += "Fog"; break;
				case Empty:			result += "Empty"; break;
				case NumValue:		result += "NumValue"; break;
				default:
					result += "???";
				break;
			}
			result += "; type :";
			switch(_Type[k])
			{
				case Double1:  result +="Double1"; break;
				case Float1:   result +="Float1";  break;
				case Short1:   result +="Short1";  break;
				case Double2:  result +="Double2"; break;
				case Float2:   result +="Float2";  break;
				case Short2:   result +="Short2";  break;
				case Double3:  result +="Double3"; break;
				case Float3:   result +="Float3";  break;
				case Short3:   result +="Short3";  break;
				case Double4:  result +="Double4"; break;
				case Float4:   result +="Float4";  break;
				case Short4:   result +="Short4";  break;
				case UChar4:   result +="UChar4";  break;
				default:
					result += "???";
				break;
			}
			nlinfo(result.c_str());
		}
	}
}


// --------------------------------------------------

void CVertexBuffer::addValueEx (TValue valueId, TType type)
{
	nlassertex (!isLocked(), ("The vertex buffer is locked."));

	// Reset format flags
	_Flags |= 1<<valueId;

	// Set the type
	_Type[valueId]=(uint8)type;

	uint numComp = NumComponentsType[type];
	// unfortunately, some vertex program implementations don't allow any type for any value
	switch (valueId)
	{
		case Position:			nlassert(numComp >= 2); break;
		case Normal:			nlassert(numComp == 3); break;
		case PrimaryColor:		nlassert(numComp == 4); break;
		case SecondaryColor:	nlassert(numComp == 4); break;
		case Weight:			nlassert(numComp == 4); break;
		case PaletteSkin:		nlassert(numComp == 4); break;
		case Fog:				nlassert(numComp == 4); break;
		default: break;
	}
}

// --------------------------------------------------

bool CVertexBuffer::hasValueEx(TValue valueId) const
{
	return (_Flags & (1 << valueId)) != 0;
}

// --------------------------------------------------

void CVertexBuffer::initEx ()
{
	nlassert (!isLocked());

	// Calc vertex size and set value's offset
	_VertexSize=0;
	for (uint value=0; value<NumValue; value++)
	{
		// Value used ?
		if (_Flags&(1<<value))
		{
			// Set offset
			_Offset[value]=_VertexSize;

			// New size
			_VertexSize+=SizeType[_Type[value]];
		}
	}

	// Reset number of vertices
	_NbVerts=0;

	// Compute new capacity
	if (_VertexSize)
		_Capacity = (uint32)_NonResidentVertices.size()/_VertexSize;
	else
		_Capacity = 0;

	// Force non resident
	restoreNonResidentMemory();
}

// --------------------------------------------------

void CVertexBuffer::reserve(uint32 n)
{
	nlassert (!isLocked());
	if (_Capacity != n)
	{
		_Capacity= n;
		_NbVerts=std::min (_NbVerts,_Capacity);

		// Force non resident
		restoreNonResidentMemory();
	}
}

// --------------------------------------------------

void CVertexBuffer::setNumVertices(uint32 n)
{
	if(_Capacity<n)
	{
		reserve(n);
	}
	if(_NbVerts != n)
	{
		_InternalFlags |= TouchedNumVertices;
		_NbVerts=n;
	}
}

// --------------------------------------------------

void	CVertexBuffer::deleteAllVertices()
{
	if (_Capacity)
	{
		nlassert (!isLocked());
		// free memory.
		contReset(_NonResidentVertices);
		_Capacity= 0;
		if(_NbVerts!=0)
		{
			_NbVerts=0;
			_InternalFlags |= TouchedNumVertices;
		}

		// Force non resident
		restoreNonResidentMemory();

		// Delete driver info
		nlassert (DrvInfos == NULL);
	}
}

// --------------------------------------------------

uint16		CVertexBuffer::remapV2Flags (uint32 oldFlags, uint& weightCount)
{
	// Old flags
	const uint32 OLD_IDRV_VF_XYZ = 0x00000001;
	const uint32 OLD_IDRV_VF_W0 = 0x00000002;
	const uint32 OLD_IDRV_VF_W1 = 0x00000004;
	const uint32 OLD_IDRV_VF_W2 = 0x00000008;
	const uint32 OLD_IDRV_VF_W3 = 0x00000010;
	const uint32 OLD_IDRV_VF_NORMAL = 0x00000020;
	const uint32 OLD_IDRV_VF_COLOR = 0x00000040;
	const uint32 OLD_IDRV_VF_SPECULAR = 0x00000080;
	const uint32 OLD_IDRV_VF_UV0 = 0x00000100;
	const uint32 OLD_IDRV_VF_UV1 = 0x00000200;
	const uint32 OLD_IDRV_VF_UV2 = 0x00000400;
	const uint32 OLD_IDRV_VF_UV3 = 0x00000800;
	const uint32 OLD_IDRV_VF_UV4 = 0x00001000;
	const uint32 OLD_IDRV_VF_UV5 = 0x00002000;
	const uint32 OLD_IDRV_VF_UV6 = 0x00004000;
	const uint32 OLD_IDRV_VF_UV7 = 0x00008000;
	const uint32 OLD_IDRV_VF_PALETTE_SKIN = 0x00010000 | OLD_IDRV_VF_W0 | OLD_IDRV_VF_W1 | OLD_IDRV_VF_W2 | OLD_IDRV_VF_W3;

	// Old Flags
	uint16 newFlags=0;

	// Number of weight values
	weightCount=0;

	// Remap the flags
	if (oldFlags&OLD_IDRV_VF_XYZ)
		newFlags|=PositionFlag;
	if (oldFlags&OLD_IDRV_VF_NORMAL)
		newFlags|=NormalFlag;
	if (oldFlags&OLD_IDRV_VF_COLOR)
		newFlags|=PrimaryColorFlag;
	if (oldFlags&OLD_IDRV_VF_SPECULAR)
		newFlags|=SecondaryColorFlag;
	if (oldFlags&OLD_IDRV_VF_UV0)
		newFlags|=TexCoord0Flag;
	if (oldFlags&OLD_IDRV_VF_UV1)
		newFlags|=TexCoord1Flag;
	if (oldFlags&OLD_IDRV_VF_UV2)
		newFlags|=TexCoord2Flag;
	if (oldFlags&OLD_IDRV_VF_UV3)
		newFlags|=TexCoord3Flag;
	if (oldFlags&OLD_IDRV_VF_UV4)
		newFlags|=TexCoord4Flag;
	if (oldFlags&OLD_IDRV_VF_UV5)
		newFlags|=TexCoord5Flag;
	if (oldFlags&OLD_IDRV_VF_UV6)
		newFlags|=TexCoord6Flag;
	if (oldFlags&OLD_IDRV_VF_UV7)
		newFlags|=TexCoord7Flag;
	if (oldFlags&OLD_IDRV_VF_W0)
	{
		weightCount=1;
		newFlags|=WeightFlag;
	}
	if (oldFlags&OLD_IDRV_VF_W1)
	{
		weightCount=2;
		newFlags|=WeightFlag;
	}
	if (oldFlags&OLD_IDRV_VF_W2)
	{
		weightCount=3;
		newFlags|=WeightFlag;
	}
	if (oldFlags&OLD_IDRV_VF_W3)
	{
		weightCount=4;
		newFlags|=WeightFlag;
	}
	if (oldFlags&(OLD_IDRV_VF_PALETTE_SKIN))
		newFlags|=PaletteSkinFlag;

	// Return the new flags
	return newFlags;
}

// --------------------------------------------------

void		CVertexBuffer::serialOldV1Minus(NLMISC::IStream &f, sint ver)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 1:
		- PaletteSkin version.
	Version 0:
		- base verison.
	*/

	// old Flags
	uint32 oldFlags;

	// Serial VBuffers format/size.
	//=============================
	f.serial(oldFlags);

	// Remap the flags
	uint weightCount;
	uint16 newFlags=remapV2Flags (oldFlags, weightCount);

	// Must be reading
	nlassert (f.isReading());

	// Set default value type
	uint i;
	for (i=0; i<NumValue; i++)
		_Type[i]=DefaultValueType[i];

	uint32 nbVert;	// Read only
	f.serial(nbVert);
	reserve(0);
	setVertexFormat(newFlags);
	setNumVertices(nbVert);
	// All other infos (but _NonResidentVertices) are computed by setVertexFormat() and setNumVertices().

	// Weight count ?
	switch (weightCount)
	{
	case 1:
		_Type[Weight]=Float1;
		break;
	case 2:
		_Type[Weight]=Float2;
		break;
	case 3:
		_Type[Weight]=Float3;
		break;
	case 4:
		_Type[Weight]=Float4;
		break;
	}

	// Serial VBuffers components.
	//============================
	for(sint id=0;id<(sint)_NbVerts;id++)
	{
		uint8 *pointer = &(*_NonResidentVertices.begin());
		uint stridedId = id * _VertexSize;
		// XYZ.
		if(_Flags & PositionFlag)
		{
			CVector		&vert= *(CVector*)(pointer + stridedId + _Offset[Position]);
			f.serial(vert);
		}
		// Normal
		if(_Flags & NormalFlag)
		{
			CVector		&norm= *(CVector*)(pointer + stridedId + _Offset[Normal]);
			f.serial(norm);
		}
		// Uvs.
		for(i=0;i<MaxStage;i++)
		{
			if(_Flags & (TexCoord0Flag<<i))
			{
				CUV		&uv= *(CUV*)(pointer + stridedId + _Offset[TexCoord0+i]);
				f.serial(uv);
			}
		}
		// Color.
		if(_Flags & PrimaryColorFlag)
		{
			CRGBA		&col= *(CRGBA*)(pointer + stridedId + _Offset[PrimaryColor]);
			f.serial(col);
		}
		// Specular.
		if(_Flags & SecondaryColorFlag)
		{
			CRGBA		&col= *(CRGBA*)(pointer + stridedId + _Offset[SecondaryColor]);
			f.serial(col);
		}
		// Weights
		for(i=0;i<weightCount;i++)
		{
			// Weight channel available ?
			float	&w= *(float*)(pointer + stridedId + _Offset[Weight] + i*sizeof(float));
			f.serial(w);
		}
		// CPaletteSkin (version 1+ only).
		if((ver>=1) && ((_Flags & PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag) )
		{
			CPaletteSkin	&ps= *(CPaletteSkin*)(pointer + stridedId + _Offset[PaletteSkin]);
			f.serial(ps);
		}

	}

	// Set touch flags
	_InternalFlags = 0;
	if(f.isReading())
	{
		// Force non resident
		restoreNonResidentMemory();
	}
}

// --------------------------------------------------

void		CVertexBuffer::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 2:
		- cut to use serialHeader() serialSubset().
	Version 1:
		- PaletteSkin version.
	Version 0:
		- base verison.
	*/
	nlassert (!isLocked());
	sint	ver= f.serialVersion(2);

	if (ver<2)
	{
		// old serial method
		serialOldV1Minus(f, ver);
	}
	else
	{
		// read write the header of the VBuffer.
		serialHeader(f);

		// read write the entire subset.
		serialSubset(f, 0, _NbVerts);
	}
}

// --------------------------------------------------

void		CVertexBuffer::serialHeader(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 3:
		- Preferred memory.
	Version 2:
		- Vertex color format management.
	Version 1:
		- Extended vertex format management.
	Version 0:
		- base verison of the header serialisation.
	*/
	sint	ver= f.serialVersion(3);	// Hulud

	// Serial VBuffers format/size.
	//=============================

	// Flags
	uint16 flags=_Flags;

	if (ver<1)
	{
		// Must be reading
		nlassert (f.isReading());

		// Serial old flags
		uint32 oldFlags;
		f.serial(oldFlags);

		// Remap flags
		uint weightCount;
		flags=remapV2Flags (oldFlags, weightCount);

		// Set default value type
		for (uint i=0; i<NumValue; i++)
			_Type[i]=DefaultValueType[i];

		// weight count ?
		switch (weightCount)
		{
		case 1:
			_Type[Weight]=Float1;
			break;
		case 2:
			_Type[Weight]=Float2;
			break;
		case 3:
			_Type[Weight]=Float3;
			break;
		case 4:
			_Type[Weight]=Float4;
			break;
		}
	}
	else
	{
		// Serial new vertex flags
		f.serial(flags);

		// Serial type of values
		for (uint i=0; i<NumValue; i++)
		{
			if (!(flags & (1 << i)))
			{
				_Type[i] = DefaultValueType[i];
			}
			f.serial (_Type[i]);
		}
	}

	// Serial nb vertices
	uint32 nbVerts=_NbVerts;
	f.serial(nbVerts);

	if(f.isReading())
	{
		reserve(0);

		// Init vertex format setup
		clearValueEx ();

		// Init vertex format
		for (uint i=0; i<NumValue; i++)
		{
			// Setup this value ?
			if (flags&(1<<i))
			{
				// Add a value
				addValueEx ((TValue)i, (TType)_Type[i]);
			}
		}

		// Build final vertex format
		initEx ();

		// Set num of vertices
		setNumVertices(nbVerts);
	}
	// All other infos (but _NonResidentVertices) are computed by initEx() and setNumVertices().

	if (ver>=2)
		f.serial (_VertexColorFormat);

	if (ver>=3)
	{
		f.serialEnum(_PreferredMemory);
		f.serial(_Name);
	}
	else
	{
		// Init preferred memory
		if(f.isReading())
		{
			_PreferredMemory = RAMPreferred;
			_Name.clear();
		}
	}
}


// --------------------------------------------------
uint	CVertexBuffer::	getNumTexCoordUsed() const
{
	for (sint k = (MaxStage - 1); k >= 0; --k)
	{
		if (_Flags & (TexCoord0Flag << k))  return (uint) (k + 1);
	}
	return 0;
}

// --------------------------------------------------

uint8		CVertexBuffer::getNumWeight () const
{
	// Num weight
	switch (_Type[Weight])
	{
	case Float1:
		return 1;
	case Float2:
		return 2;
	case Float3:
		return 3;
	case Float4:
		return 4;
	}

	// No weight
	return 0;
}

// --------------------------------------------------

void		CVertexBuffer::serialSubset(NLMISC::IStream &f, uint vertexStart, uint vertexEnd)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 2:
		- UVRouting
	Version 1:
		- weight is 4 float in standard format.
	Version 0:
		- base verison of a vbuffer subset serialisation.
	*/
	sint ver = f.serialVersion(2);


	// Serial VBuffers components.
	//============================
	nlassert(vertexStart<_NbVerts || _NbVerts==0);
	nlassert(vertexEnd<=_NbVerts);
	for(uint id=vertexStart; id<vertexEnd; id++)
	{
		// For each value
		for (uint value=0; value<NumValue; value++)
		{
			// Value used ?
			if (_Flags&(1<<value))
			{
				// Get the pointer on it
				void *ptr=(void*)((&(*_NonResidentVertices.begin()))+id*_VertexSize+getValueOffEx ((TValue)value));
				f.serialBuffer ((uint8*)ptr, SizeType[_Type[value]]);
			}
		}
	}

	// Serial the UV Routing table
	//============================
	if (ver>=2)
	{
		f.serialBuffer (_UVRouting, sizeof(uint8)*MaxStage);
	}
	else
	{
		// Reset the table
		uint i;
		for (i=0; i<MaxStage; i++)
			_UVRouting[i] = i;
	}

	// Set touch flags
	if(f.isReading())
	{
		// Force non resident
		restoreNonResidentMemory();
	}
}

// --------------------------------------------------

bool CVertexBuffer::setVertexColorFormat (TVertexColorType format)
{
	// If resident, quit
	if (isResident())
		return false;

	nlassert (!isLocked());

	// Format is not good ?
	if ((TVertexColorType)_VertexColorFormat != format)
	{
		// Diffuse or specualr component ?
		if (_Flags & (PrimaryColorFlag|SecondaryColorFlag))
		{
			uint i;
			uint32 *ptr0 = (_Flags&PrimaryColorFlag)?(uint32*)&(_NonResidentVertices[_Offset[PrimaryColor]]):NULL;
			uint32 *ptr1 = (_Flags&SecondaryColorFlag)?(uint32*)&(_NonResidentVertices[_Offset[SecondaryColor]]):NULL;
			for (i=0; i<_NbVerts; i++)
			{
				if (ptr0)
				{
					const register uint32 value = *ptr0;
#ifdef NL_LITTLE_ENDIAN
					*ptr0 = (value&0xff00ff00)|((value&0xff)<<16)|((value&0xff0000)>>16);
#else // NL_LITTLE_ENDIAN
					*ptr0 = (value&0x00ff00ff)|((value&0xff00)<<16)|((value&0xff000000)>>16);
#endif // NL_LITTLE_ENDIAN
					ptr0 = (uint32*)(((uint8*)ptr0)+_VertexSize);
				}
				if (ptr1)
				{
					const register uint32 value = *ptr1;
#ifdef NL_LITTLE_ENDIAN
					*ptr1 = (value&0xff00ff00)|((value&0xff)<<16)|((value&0xff0000)>>16);
#else // NL_LITTLE_ENDIAN
					*ptr1 = (value&0x00ff00ff)|((value&0xff00)<<16)|((value&0xff000000)>>16);
#endif // NL_LITTLE_ENDIAN
					ptr1 = (uint32*)(((uint8*)ptr1)+_VertexSize);
				}
			}
		}
		_VertexColorFormat = (uint8)format;

		// Force non resident
		restoreNonResidentMemory();
	}
	return true;
}

// --------------------------------------------------

void CVertexBuffer::setPreferredMemory (TPreferredMemory preferredMemory, bool keepLocalMemory)
{
	if ((_PreferredMemory != preferredMemory) || (_KeepLocalMemory != keepLocalMemory))
	{
		_PreferredMemory = preferredMemory;
		_KeepLocalMemory = keepLocalMemory;

		// Force non resident
		restoreNonResidentMemory();
	}
}

// --------------------------------------------------
void CVertexBuffer::setLocation (TLocation newLocation)
{
	// Upload ?
	if (newLocation != NotResident)
	{
		// The driver must have setuped the driver info
		nlassert (DrvInfos);

		// Current size of the buffer
		const uint size = ((_PreferredMemory==RAMVolatile)||(_PreferredMemory==AGPVolatile))?_NbVerts*_VertexSize:_Capacity*_VertexSize;

		// The buffer must not be resident
		if (_Location != NotResident)
			setLocation (NotResident);

		// Copy the buffer containt
		uint8 *dest = DrvInfos->lock (0, size, false);
		nlassert (size<=_NonResidentVertices.size());	// Internal buffer must have the good size
		memcpy (dest, &(_NonResidentVertices[0]), size);
		DrvInfos->unlock(0, 0);

		// Reset the non resident container if not a static preferred memory and not put in RAM
		if ((_PreferredMemory != StaticPreferred) && (_Location != RAMResident) && !_KeepLocalMemory)
			contReset(_NonResidentVertices);

		// Clear touched flags
		resetTouchFlags ();

		_Location =	newLocation;
		_ResidentSize = size;
	}
	else
	{
		// Current size of the buffer
		const uint size = _Capacity*_VertexSize;

		// Resize the non resident buffer
		_NonResidentVertices.resize (size);

		// If resident in RAM, backup the data in non resident memory
		if ((_Location == RAMResident) && (_PreferredMemory != RAMVolatile) && (_PreferredMemory != AGPVolatile) && !_KeepLocalMemory)
		{
			// The driver must have setuped the driver info
			nlassert (DrvInfos);

			// Copy the old buffer data
			const uint8 *src = DrvInfos->lock (0, _ResidentSize, true);
			if (!_NonResidentVertices.empty())
				memcpy (&(_NonResidentVertices[0]), src, std::min (size, (uint)_ResidentSize));
			DrvInfos->unlock(0, 0);
		}

		_Location = NotResident;
		_ResidentSize = 0;

		// Touch the buffer
		_InternalFlags |= TouchedAll;
	}
}

// --------------------------------------------------
void CVertexBuffer::restoreNonResidentMemory()
{
	setLocation (NotResident);

	if (DrvInfos)
		DrvInfos->VertexBufferPtr = NULL;	// Tell the driver info to not restore memory when it will die

	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

// --------------------------------------------------

void CVertexBuffer::fillBuffer ()
{
	if (DrvInfos && _KeepLocalMemory)
	{
		// Copy the local memory in local memory
		const uint size = _NbVerts*_VertexSize;
		nlassert (size<=_NonResidentVertices.size());
		uint8 *dest = DrvInfos->lock (0, size, false);
		NLMISC::CFastMem::memcpy (dest, &(_NonResidentVertices[0]), size);
		DrvInfos->unlock(0, size);
	}
}

// --------------------------------------------------

// CPaletteSkin serial (no version chek).
void	CPaletteSkin::serial(NLMISC::IStream &f)
{
	f.serial(MatrixId[0], MatrixId[1], MatrixId[2], MatrixId[3]);
}

// --------------------------------------------------

IVBDrvInfos::~IVBDrvInfos()
{
	_Driver->removeVBDrvInfoPtr(_DriverIterator);
}

// --------------------------------------------------
// CVertexBufferReadWrite
// --------------------------------------------------

NLMISC::CVector* CVertexBufferReadWrite::getVertexCoordPointer(uint idx)
{
	nlassert (_Parent->checkLockedBuffer());
	uint8*	ptr;

	ptr=_Parent->_LockedBuffer;
	ptr+=(idx*_Parent->_VertexSize);
	return((NLMISC::CVector*)ptr);
}

// --------------------------------------------------

NLMISC::CVector* CVertexBufferReadWrite::getNormalCoordPointer(uint idx)
{
	nlassert (_Parent->checkLockedBuffer());
	uint8*	ptr;

	if ( !(_Parent->_Flags & CVertexBuffer::NormalFlag) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::Normal];
	ptr+=idx*_Parent->_VertexSize;
	return((NLMISC::CVector*)ptr);
}

// --------------------------------------------------

void* CVertexBufferReadWrite::getColorPointer(uint idx)
{
	nlassert (_Parent->checkLockedBuffer());
	uint8*	ptr;

	if ( !(_Parent->_Flags & CVertexBuffer::PrimaryColorFlag) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::PrimaryColor];
	ptr+=idx*_Parent->_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

void* CVertexBufferReadWrite::getSpecularPointer(uint idx)
{
	nlassert (_Parent->checkLockedBuffer());
	uint8*	ptr;

	if ( !(_Parent->_Flags & CVertexBuffer::SecondaryColorFlag) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::SecondaryColor];
	ptr+=idx*_Parent->_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

NLMISC::CUV* CVertexBufferReadWrite::getTexCoordPointer(uint idx, uint8 stage)
{
	nlassert (_Parent->checkLockedBuffer());
	uint8*	ptr;

	if ( !(_Parent->_Flags & (CVertexBuffer::TexCoord0Flag<<stage)) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::TexCoord0+stage];
	ptr+=idx*_Parent->_VertexSize;
	return((NLMISC::CUV*)ptr);
}

// --------------------------------------------------

float* CVertexBufferReadWrite::getWeightPointer(uint idx, uint8 wgt)
{
	nlassert (_Parent->checkLockedBuffer());
	uint8*	ptr;

	nlassert(wgt<CVertexBuffer::MaxWeight);
	if( !(_Parent->_Flags & CVertexBuffer::WeightFlag))
		return NULL;

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::Weight]+wgt*sizeof(float);

	return (float*)ptr;
}

// --------------------------------------------------

CPaletteSkin* CVertexBufferReadWrite::getPaletteSkinPointer(uint idx)
{
	nlassert (_Parent->checkLockedBuffer());
	uint8*	ptr;

	if ( (_Parent->_Flags & CVertexBuffer::PaletteSkinFlag) != CVertexBuffer::PaletteSkinFlag )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::PaletteSkin];
	ptr+=idx*_Parent->_VertexSize;
	return((CPaletteSkin*)ptr);
}

// --------------------------------------------------

void CVertexBufferReadWrite::touchVertices (uint first, uint last)
{
	nlassert (_Parent->checkLockedBuffer());
	_First = first;
	_Last = last;
}

// --------------------------------------------------
// CVertexBufferRead
// --------------------------------------------------

const NLMISC::CVector* CVertexBufferRead::getVertexCoordPointer(uint idx) const
{
	nlassert (_Parent->checkLockedBuffer());
	const uint8*	ptr;

	ptr=_Parent->_LockedBuffer;
	ptr+=(idx*_Parent->_VertexSize);
	return((const NLMISC::CVector*)ptr);
}

// --------------------------------------------------

const NLMISC::CVector* CVertexBufferRead::getNormalCoordPointer(uint idx) const
{
	nlassert (_Parent->checkLockedBuffer());
	const uint8*	ptr;

	if ( !(_Parent->_Flags & CVertexBuffer::NormalFlag) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::Normal];
	ptr+=idx*_Parent->_VertexSize;
	return((const NLMISC::CVector*)ptr);
}

// --------------------------------------------------

const void* CVertexBufferRead::getColorPointer(uint idx) const
{
	nlassert (_Parent->checkLockedBuffer());
	const uint8*	ptr;

	if ( !(_Parent->_Flags & CVertexBuffer::PrimaryColorFlag) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::PrimaryColor];
	ptr+=idx*_Parent->_VertexSize;
	return((const void*)ptr);
}

// --------------------------------------------------

const void* CVertexBufferRead::getSpecularPointer(uint idx) const
{
	nlassert (_Parent->checkLockedBuffer());
	const uint8*	ptr;

	if ( !(_Parent->_Flags & CVertexBuffer::SecondaryColorFlag) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::SecondaryColor];
	ptr+=idx*_Parent->_VertexSize;
	return((const void*)ptr);
}

// --------------------------------------------------

const NLMISC::CUV* CVertexBufferRead::getTexCoordPointer(uint idx, uint8 stage) const
{
	nlassert (_Parent->checkLockedBuffer());
	const uint8*	ptr;

	if ( !(_Parent->_Flags & (CVertexBuffer::TexCoord0Flag<<stage)) )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::TexCoord0+stage];
	ptr+=idx*_Parent->_VertexSize;
	return((const NLMISC::CUV*)ptr);
}

// --------------------------------------------------

const float* CVertexBufferRead::getWeightPointer(uint idx, uint8 wgt) const
{
	nlassert (_Parent->checkLockedBuffer());
	const uint8*	ptr;

	nlassert(wgt<CVertexBuffer::MaxWeight);
	if( !(_Parent->_Flags & CVertexBuffer::WeightFlag))
		return NULL;

	ptr=(uint8*)(&_Parent->_LockedBuffer[idx*_Parent->_VertexSize]);
	ptr+=_Parent->_Offset[CVertexBuffer::Weight]+wgt*sizeof(float);

	return (float*)ptr;
}

// --------------------------------------------------

const CPaletteSkin* CVertexBufferRead::getPaletteSkinPointer(uint idx) const
{
	nlassert (_Parent->checkLockedBuffer());
	const uint8*	ptr;

	if ( (_Parent->_Flags & CVertexBuffer::PaletteSkinFlag) != CVertexBuffer::PaletteSkinFlag )
	{
		return(NULL);
	}
	ptr=_Parent->_LockedBuffer;
	ptr+=_Parent->_Offset[CVertexBuffer::PaletteSkin];
	ptr+=idx*_Parent->_VertexSize;
	return((const CPaletteSkin*)ptr);
}

// --------------------------------------------------

}





















