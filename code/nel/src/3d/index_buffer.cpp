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

#include "nel/3d/index_buffer.h"
#include "nel/3d/driver.h"
#include "nel/misc/stream.h"
#include "nel/misc/fast_mem.h"

using namespace NLMISC;

namespace NL3D
{
// ***************************************************************************
// IIBDrvInfos
// ***************************************************************************

IIBDrvInfos::~IIBDrvInfos()
{
	_Driver->removeIBDrvInfoPtr(_DriverIterator);
}

// ***************************************************************************
// CIndexBuffer
// ***************************************************************************

CIndexBuffer::CIndexBuffer()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_Capacity = 0;
	_NbIndexes = 0;
	_InternalFlags = 0;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	_Location = NotResident;
	_ResidentSize = 0;
	_KeepLocalMemory = false;
	_Format = Indices32;
}

// ***************************************************************************

CIndexBuffer::CIndexBuffer(const CIndexBuffer &vb) : CRefCount()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	_Capacity = 0;
	_NbIndexes = 0;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	_Location = NotResident;
	_ResidentSize = 0;
	_KeepLocalMemory = false;
	operator=(vb);
}

// ***************************************************************************
CIndexBuffer::CIndexBuffer(const char *name)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	_Capacity = 0;
	_NbIndexes = 0;
	_InternalFlags = 0;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	_Location = NotResident;
	_ResidentSize = 0;
	_KeepLocalMemory = false;
	_Name = name;
}

// ***************************************************************************

CIndexBuffer::~CIndexBuffer()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// Single value
	if (DrvInfos)
		DrvInfos->IndexBufferPtr = NULL;	// Tell the driver info to not restaure memory when it will die

	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

// ***************************************************************************

CIndexBuffer	&CIndexBuffer::operator=(const CIndexBuffer &vb)
{
	nlassertex (!isLocked(), ("The index buffer is locked."));
	// Single value
	_InternalFlags = vb._InternalFlags;
	_NbIndexes = vb._NbIndexes;
	_Capacity = vb._Capacity;
	_NonResidentIndexes = vb._NonResidentIndexes;
	_PreferredMemory = vb._PreferredMemory;
	_KeepLocalMemory = vb._KeepLocalMemory;
	_Format = vb._Format;

	// Set touch flags
	_InternalFlags |= TouchedAll;
	_Location = NotResident;
	_ResidentSize = 0;

	return *this;
}

// ***************************************************************************

void CIndexBuffer::setPreferredMemory (TPreferredMemory preferredMemory, bool keepLocalMemory)
{
	if ((_PreferredMemory != preferredMemory) || (_KeepLocalMemory != keepLocalMemory))
	{
		_PreferredMemory = preferredMemory;
		_KeepLocalMemory = keepLocalMemory;

		// Force non resident
		restaureNonResidentMemory();
	}
}

// ***************************************************************************

void CIndexBuffer::reserve(uint32 n)
{
	nlassert (!isLocked());
	if (_Capacity != n)
	{
		_Capacity= n;
		_NbIndexes=std::min (_NbIndexes,_Capacity);

		// Force non resident
		restaureNonResidentMemory();
	}
}

// ***************************************************************************

void CIndexBuffer::setNumIndexes(uint32 n)
{
	if(_Capacity<n)
	{
		reserve(n);
	}
	if(_NbIndexes != n)
	{
		_InternalFlags |= TouchedNumIndexes;
		_NbIndexes=n;
	}
}

// ***************************************************************************

void CIndexBuffer::setFormat(TFormat format)
{
	if (format == _Format) return;
	uint numIndexes = getNumIndexes();
	deleteAllIndexes();
	_Format = format;
	setNumIndexes(numIndexes);
}

// ***************************************************************************

void CIndexBuffer::deleteAllIndexes()
{
	if (_Capacity)
	{
		nlassert (!isLocked());
		// free memory.
		contReset(_NonResidentIndexes);
		_Capacity= 0;
		if(_NbIndexes!=0)
		{
			_NbIndexes=0;
			_InternalFlags |= TouchedNumIndexes;
		}

		// Force non resident
		restaureNonResidentMemory();

		// Delete driver info
		nlassert (DrvInfos == NULL);
	}
}

// ***************************************************************************

void CIndexBuffer::setLocation (TLocation newLocation)
{
	// Upload ?
	if (newLocation != NotResident)
	{
		// The driver must have setuped the driver info
		nlassert (DrvInfos);

		// Current size of the buffer
		const uint size = ((_PreferredMemory==RAMVolatile)||(_PreferredMemory==AGPVolatile))?_NbIndexes:_Capacity;

		// The buffer must not be resident
		if (_Location != NotResident)
			setLocation (NotResident);

		// Copy the buffer content
		void *dest = DrvInfos->lock (0, size, false);
		nlassert (_NonResidentIndexes.size() / getIndexNumBytes() == _Capacity);	// Internal buffer must have the good size
		if (_Capacity != 0)
			memcpy (dest, &(_NonResidentIndexes[0]), size*getIndexNumBytes());
		DrvInfos->unlock(0, 0);

		// Reset the non resident container if not a static preferred memory and not put in RAM
		if ((_PreferredMemory != StaticPreferred) && (_Location != RAMResident) && !_KeepLocalMemory)
			contReset(_NonResidentIndexes);

		// Clear touched flags
		resetTouchFlags ();

		_Location =	newLocation;
		_ResidentSize = _Capacity;
	}
	else
	{
		// Resize the non resident buffer
		_NonResidentIndexes.resize (_Capacity * getIndexNumBytes());

		// If resident in RAM, backup the data in non resident memory
		if ((_Location == RAMResident) && (_PreferredMemory != RAMVolatile) && (_PreferredMemory != AGPVolatile) && !_KeepLocalMemory)
		{
			// The driver must have setuped the driver info
			nlassert (DrvInfos);

			// Copy the old buffer data
			const void *src = DrvInfos->lock (0, _ResidentSize, true);
			uint size = std::min ((uint)(_Capacity*getIndexNumBytes()), (uint)(_ResidentSize*getIndexNumBytes()));
			if (size)
				memcpy (&(_NonResidentIndexes[0]), src, size);
			DrvInfos->unlock(0, 0);
		}

		_Location = NotResident;
		_ResidentSize = 0;

		// Touch the buffer
		_InternalFlags |= TouchedAll;
	}
}

// ***************************************************************************

void CIndexBuffer::restaureNonResidentMemory()
{
	setLocation (NotResident);

	if (DrvInfos)
		DrvInfos->IndexBufferPtr = NULL;	// Tell the driver info to not restaure memory when it will die

	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}


// ***************************************************************************
void CIndexBuffer::buildSerialVector(std::vector<uint32> &dest) const
{
	dest.resize(getNumIndexes());
	if (_Format == Indices16)
	{
		const uint16 *src = (const uint16 *) &_NonResidentIndexes[0];
		// convert to 32 bits
		for(uint k = 0; k < getNumIndexes(); ++k)
		{
			dest[k] = *src ++;
		}
	}
	else
	{
		// direct copy
		memcpy(&dest[0], &_NonResidentIndexes[0], sizeof(uint32) * getNumIndexes());
	}
}

// ***************************************************************************
void CIndexBuffer::restoreFromSerialVector(const std::vector<uint32> &src)
{
	// for now, just convert to wanted format
	if (_Format == Indices16)
	{
		_NonResidentIndexes.resize(sizeof(uint16) * src.size());
		uint16 *dest = (uint16 *) &_NonResidentIndexes[0];
		for(uint k = 0; k < src.size(); ++k)
		{
			nlassert(src[k] <= 0xffff);
			*dest++ = (uint16) src[k];
		}
	}
	else
	{
		nlassert(_Format == Indices32);
		_NonResidentIndexes.resize(sizeof(uint32) * src.size());
		memcpy(&_NonResidentIndexes[0], &src[0], sizeof(uint32) * src.size());
	}
}

// ***************************************************************************
void CIndexBuffer::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/** Version 2 : no more write only flags
	  * Version 1 : index buffer
	  * Version 0 : primitive block
	  */

	sint ver = f.serialVersion(2);

	// Primitive block?
	if (ver < 1)
	{
		uint32 nb, capacity;
		std::vector<uint32>	indexes;

		// Skip lines
		f.serial(nb, capacity);
		f.serialCont(indexes);

		// Read tri
		// NB : for backward compatibility, indices are always saved in 32 bit format
		std::vector<uint32> nonResidentIndexes;
		if (!f.isReading())
		{
			buildSerialVector(nonResidentIndexes);
		}
		f.serial(nb, capacity);
		_NbIndexes = nb*3;
		_Capacity = capacity*3;
		f.serialCont(nonResidentIndexes);

		if (f.isReading())
		{
			restoreFromSerialVector(nonResidentIndexes);
		}

		// Skip quads
		f.serial(nb, capacity);
		f.serialCont(indexes);
	}

	// Index buffer?
	if (ver >= 1)
	{
		// NB : for backward compatibility, indices are always saved in 32 bit format
		std::vector<uint32> nonResidentIndexes;
		if (!f.isReading())
		{
			buildSerialVector(nonResidentIndexes);
		}
		f.serial(_NbIndexes, _Capacity);
		f.serialCont(nonResidentIndexes);
		f.serialEnum(_PreferredMemory);
		if (f.isReading())
		{
			restoreFromSerialVector(nonResidentIndexes);
		}
		// Read the old format
		if (ver == 1)
		{
			uint i;
			bool temp;
			for (i=0; i<PreferredCount; i++)
				f.serial(temp);
		}
	}

	// Loaded ?
	if (f.isReading())
	{
		// Force non resident
		restaureNonResidentMemory();
	}
}

// ***************************************************************************

void CIndexBuffer::fillBuffer ()
{
	if (DrvInfos && _KeepLocalMemory)
	{
		// Copy the local memory in local memory
		nlassert ((_NbIndexes * getIndexNumBytes()) <=_NonResidentIndexes.size());
		void *dest = DrvInfos->lock (0, _NbIndexes, false);
		NLMISC::CFastMem::memcpy (dest, &(_NonResidentIndexes[0]), _NbIndexes*getIndexNumBytes());
		DrvInfos->unlock(0, _NbIndexes);
	}
}

// ***************************************************************************

} // namespace NL3D
