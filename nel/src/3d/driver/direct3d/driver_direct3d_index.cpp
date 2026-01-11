// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stddirect3d.h"

#include "nel/3d/index_buffer.h"
#include "nel/3d/light.h"
#include "nel/misc/rect.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

CIBDrvInfosD3D::CIBDrvInfosD3D(CDriverD3D *drv, ItIBDrvInfoPtrList it, CIndexBuffer *ib) : IIBDrvInfos(drv, it, ib)
{
	H_AUTO_D3D(CIBDrvInfosD3D_CIBDrvInfosD3D)
	Driver = drv;
	IndexBuffer = NULL;
	VolatileIndexBuffer = NULL;

}

// ***************************************************************************

uint indexCount=0;

CIBDrvInfosD3D::~CIBDrvInfosD3D()
{
	H_AUTO_D3D(CIBDrvInfosD3D_CIBDrvInfosD3DDtor);
	// Restore non resident memory
	if (IndexBufferPtr)
	{
		IndexBufferPtr->setLocation(CIndexBuffer::NotResident);
		IndexBufferPtr= NULL;
	}

	// release index buffer
	if (IndexBuffer && !Volatile)
	{
		if (Driver)
		{
			if (Driver->_IndexBufferCache.IndexBuffer == IndexBuffer)
			{
				Driver->_IndexBufferCache.IndexBuffer = NULL;
				Driver->touchRenderVariable(&Driver->_IndexBufferCache);
			}
		}
		indexCount--;
		IndexBuffer->Release();
	}
}

// ***************************************************************************

void *CIBDrvInfosD3D::lock (uint first, uint last, bool readOnly)
{
	H_AUTO_D3D(CIBDrvInfosD3D_lock);
	nlassert (first != last);
	CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);

	if (driver->getMaxVertexIndex() <= 0xffff && getFormat() != CIndexBuffer::Indices16)
	{
		nlassert(getFormat() == CIndexBuffer::Indices32);
		// 32-bit index not supported -> uses RAM mirror
		nlassert(!RamVersion.empty());
		return &RamVersion[0];
	}
	else
	{
		if (Volatile)
		{
			// Lock the good buffer
			CVolatileIndexBuffer **buffer = NULL;
			if (getFormat() == CIndexBuffer::Indices16)
			{
				buffer = VolatileRAM ? (&driver->_VolatileIndexBuffer16RAM[driver->_CurrentRenderPass&1]):(&driver->_VolatileIndexBuffer16AGP[driver->_CurrentRenderPass&1]);
			}
			else if (getFormat() == CIndexBuffer::Indices32)
			{
				buffer = VolatileRAM ? (&driver->_VolatileIndexBuffer32RAM[driver->_CurrentRenderPass&1]):(&driver->_VolatileIndexBuffer32AGP[driver->_CurrentRenderPass&1]);
			}
			else
			{
				nlassert(0);
			}
			void *ptr = (*buffer)->lock ((last-first)*getIndexNumBytes(), Offset);
			if (!ptr)
			{
				// buffer full, swap them
				CVolatileIndexBuffer **bufferOther;
				if (getFormat() == CIndexBuffer::Indices16)
				{
					bufferOther = VolatileRAM ? (&driver->_VolatileIndexBuffer16RAM[(driver->_CurrentRenderPass + 1) &1]):(&driver->_VolatileIndexBuffer16AGP[(driver->_CurrentRenderPass + 1 ) &1]);
				}
				else
				{
					bufferOther = VolatileRAM ? (&driver->_VolatileIndexBuffer32RAM[(driver->_CurrentRenderPass + 1) &1]):(&driver->_VolatileIndexBuffer32AGP[(driver->_CurrentRenderPass + 1 ) &1]);
				}
				std::swap(*buffer, *bufferOther);
				(*buffer)->reset();
				ptr = (*buffer)->lock ((last-first)*getIndexNumBytes(), Offset);
				nlassert(ptr);
			}
			nlassert(!VolatileIndexBuffer);
			VolatileIndexBuffer = *buffer;
			IndexBuffer = (*buffer)->IndexBuffer;
			ptr = (uint8 *) ptr - first * getIndexNumBytes();

			// Current lock time
			VolatileLockTime = driver->_CurrentRenderPass;

			// Touch the index buffer
			driver->touchRenderVariable (&driver->_IndexBufferCache);

			return ptr;
		}
		else
		{
			nlassert (IndexBuffer);
			// Lock Profile?
			TTicks	beforeLock = 0;
			if(driver->_IBProfiling /*&& Hardware*/)
			{
				beforeLock= CTime::getPerformanceTime();
			}
			void *pbData;
			HRESULT result = IndexBuffer->Lock ( first*getIndexNumBytes(), (last-first)*getIndexNumBytes(), &pbData, readOnly?D3DLOCK_READONLY:0);
			nlassert(result == D3D_OK);
			// Lock Profile?
			if(driver->_IBProfiling /*&& Hardware*/)
			{
				TTicks	afterLock;
				afterLock= CTime::getPerformanceTime();
				driver->appendIBLockProfile(afterLock-beforeLock, IndexBufferPtr);
			}
			if (result == D3D_OK) return pbData;
		}
	}
	return NULL;
}

// ***************************************************************************

void	CIBDrvInfosD3D::unlock (uint /* first */, uint /* last */)
{
	H_AUTO_D3D(CIBDrvInfosD3D_unlock)
	CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);
	if (driver->getMaxVertexIndex() > 0xffff || getFormat() == CIndexBuffer::Indices16)
	{
		if (Volatile)
		{
			nlassert(VolatileIndexBuffer);
			VolatileIndexBuffer->unlock ();
			VolatileIndexBuffer = NULL;
		}
		else
		{
			if (IndexBuffer) IndexBuffer->Unlock ();
		}
	}
}

// ***************************************************************************

DWORD RemapIndexBufferUsage[CIndexBuffer::LocationCount]=
{
	D3DUSAGE_DYNAMIC,						// RAMResident
	D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,	// AGPResident
	D3DUSAGE_WRITEONLY,						// VRAMResident
	0,										// Not used
};

// ***************************************************************************

D3DPOOL RemapIndexBufferPool[CIndexBuffer::LocationCount]=
{
	D3DPOOL_SYSTEMMEM,	// RAMResident
	D3DPOOL_DEFAULT,	// AGPResident
	D3DPOOL_DEFAULT,	// VRAMResident
	D3DPOOL_DEFAULT,	// Not used
};

// ***************************************************************************

bool CDriverD3D::activeIndexBuffer(CIndexBuffer& IB)
{
	H_AUTO_D3D(CDriverD3D_activeIndexBuffer)

	// Must not be locked
	nlassert (!IB.isLocked());

	// Must not be empty
	if (IB.capacity() == 0)
		return false;

	const bool touched = (IB.getTouchFlags() & (CIndexBuffer::TouchedReserve|CIndexBuffer::TouchedIndexFormat)) != 0;
	CIBDrvInfosD3D *info = static_cast<CIBDrvInfosD3D*>(static_cast<IIBDrvInfos*>(IB.DrvInfos));

	// Volatile buffers must be filled at each pass (exception if emulated)
	if (_MaxVertexIndex > 0xffff)
	{
		nlassertex (!info || !info->Volatile || IB.getKeepLocalMemory() || (info->VolatileLockTime == _CurrentRenderPass), ("Volatile buffers must be filled at each pass"));
	}

	// Build the driver info
	if (touched)
	{
		// Delete previous index buffer info
		if (IB.DrvInfos)
		{
			delete IB.DrvInfos;
			nlassert (IB.DrvInfos == NULL);
		}

		// Rebuild it
		_IBDrvInfos.push_front (NULL);
		ItIBDrvInfoPtrList ite = _IBDrvInfos.begin();
		info = new CIBDrvInfosD3D(this, ite, &IB);
		*ite = info;
		// Create the index buffer
		const uint size = (uint)IB.capacity();
		uint preferredMemory = 0;
		if (_DisableHardwareIndexArrayAGP)
		{
			preferredMemory = CIndexBuffer::RAMResident;
			info->Volatile = false;
		}
		else
		{
			switch (IB.getPreferredMemory ())
			{
			case CIndexBuffer::RAMPreferred:
				preferredMemory = CIndexBuffer::RAMResident;
				info->Volatile = false;
				break;
			case CIndexBuffer::AGPPreferred:
				preferredMemory = CIndexBuffer::AGPResident;
				info->Volatile = false;
				break;
			case CIndexBuffer::StaticPreferred:
				if (getStaticMemoryToVRAM())
					preferredMemory = CIndexBuffer::VRAMResident;
				else
					preferredMemory = CIndexBuffer::AGPResident;
				info->Volatile = false;
				break;
			case CIndexBuffer::RAMVolatile:
				preferredMemory = CIndexBuffer::RAMResident;
				info->Volatile = true;
				break;
			case CIndexBuffer::AGPVolatile:
				preferredMemory = CIndexBuffer::AGPResident;
				info->Volatile = true;
				break;
			}
		}

		// if 32 bit index not supported, the index buffer will be reformated so return a RAM mirror
		// Real index buffer will be allocated if indices are not modified (e.g a single lock is used to update the content)
		if (_MaxVertexIndex <= 0xffff && IB.getFormat() == CIndexBuffer::Indices32)
		{
			info->RamVersion.resize(size);
			info->IndexBuffer = NULL;
		}
		else
		{
			// Volatile index buffer
			if (info->Volatile)
			{
				nlassert (info->IndexBuffer == NULL);
				info->VolatileRAM = preferredMemory == CIndexBuffer::RAMResident;
			}
			else
			{
				// Offset will be 0
				info->Offset = 0;
				bool success = false;
				do
				{
					success = _DeviceInterface->CreateIndexBuffer(size*IB.getIndexNumBytes(),
						RemapIndexBufferUsage[preferredMemory],
						IB.getFormat() == CIndexBuffer::Indices32 ? D3DFMT_INDEX32 : D3DFMT_INDEX16,
						RemapIndexBufferPool[preferredMemory],
						&(info->IndexBuffer), NULL) == D3D_OK;

					if (success)
						break;
				}
				while (preferredMemory--);
				if (!success)
					return false;
				++indexCount;
			}
			// Force the vertex buffer update
			touchRenderVariable (&_IndexBufferCache);
		}
		// Release the local index buffer
		IB.DrvInfos = info;
		IB.setLocation((CIndexBuffer::TLocation)preferredMemory);
	}

	// Set the current index buffer
	nlassert (info);
	_LastIndexBufferInfo = info;

	// Fill the buffer if in local memory
	IB.fillBuffer ();

	_CurrIndexBufferFormat = IB.getFormat();

	// Set the index buffer
	if (_MaxVertexIndex > 0xffff || IB.getFormat() == CIndexBuffer::Indices16)
	{
		setIndexBuffer (info->IndexBuffer, info->Offset);
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::supportIndexBufferHard() const
{
	H_AUTO_D3D(CDriverD3D_supportIndexBufferHard);
	return !_DisableHardwareIndexArrayAGP;
}

// ***************************************************************************

void CDriverD3D::disableHardwareIndexArrayAGP()
{
	H_AUTO_D3D(CDriverD3D_disableHardwareIndexArrayAGP)
	_DisableHardwareIndexArrayAGP = true;
}

// ***************************************************************************
// CVolatileIndexBuffer
// ***************************************************************************

CVolatileIndexBuffer::CVolatileIndexBuffer()
{
	H_AUTO_D3D(CVolatileIndexBuffer_CVolatileIndexBuffer);
	IndexBuffer = NULL;
	Locked = false;
}

// ***************************************************************************

CVolatileIndexBuffer::~CVolatileIndexBuffer()
{
	H_AUTO_D3D(CVolatileIndexBuffer_CVolatileIndexBufferDtor);
	release ();
}

// ***************************************************************************

void CVolatileIndexBuffer::release ()
{
	H_AUTO_D3D(CVolatileIndexBuffer_release);
	if (IndexBuffer)
		IndexBuffer->Release();
	IndexBuffer = NULL;
}

// ***************************************************************************
void CVolatileIndexBuffer::init (CIndexBuffer::TLocation location, uint sizeInBytes, uint maxSize, CDriverD3D *driver, CIndexBuffer::TFormat format)
{
	H_AUTO_D3D(CVolatileIndexBuffer_init);
	release();
	if (maxSize < sizeInBytes) maxSize = sizeInBytes;

	// Init the buffer
	Location = location;
	Size = sizeInBytes;
	MaxSize = maxSize;
	Driver = driver;

	nlassert(format == CIndexBuffer::Indices16 || format == CIndexBuffer::Indices32);
	if (format == CIndexBuffer::Indices32)
	{
		// device must support 32 bits indices
		nlassert(driver->_MaxVertexIndex > 0xffff);
	}
	D3DFORMAT d3dFormat = format == CIndexBuffer::Indices16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32;

	// Allocate the vertex buffer
	if (Driver->_DeviceInterface->CreateIndexBuffer(sizeInBytes, RemapIndexBufferUsage[location],
		d3dFormat, RemapIndexBufferPool[location], &IndexBuffer, NULL) != D3D_OK)
	{
		// Location in RAM must not failed
		nlassert (location != CIndexBuffer::RAMResident);

		// Allocate in RAM
		nlverify (Driver->_DeviceInterface->CreateIndexBuffer(sizeInBytes, RemapIndexBufferUsage[CIndexBuffer::RAMResident],
				 d3dFormat, RemapIndexBufferPool[CIndexBuffer::RAMResident], &IndexBuffer, NULL) != D3D_OK);
		Location = CIndexBuffer::RAMResident;
	}
	Format = format;
}




// ***************************************************************************
void *CVolatileIndexBuffer::lock (uint size, uint &offset)
{
	nlassertex(!Locked, ("Volatile buffer usage should follow an atomic lock/unlock/render sequence"));
	H_AUTO_D3D(CVolatileIndexBuffer_lock);
	/* If not enough room to allocate this buffer, resise the buffer to Size+Size/2 but do not reset CurrentIndex
	 * to be sure the buffer will be large enough next pass. */

	// Enough room for this index ?
	if (CurrentIndex+size > Size)
	{
		if (CurrentIndex+size > MaxSize && CurrentIndex != 0)
		{
			return NULL;  // max size exceeded -> can reallocate only if we are at start of block
		}
		// No, reallocate
		init (Location, std::max (std::min(Size+Size/2, MaxSize),  CurrentIndex+size), MaxSize, Driver, Format);
	}
	// Lock Profile?
	TTicks	beforeLock = 0;
	if(Driver->_IBProfiling /*&& Hardware*/)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	// Lock the buffer, noblocking lock here if not the first allocation since a reset
	VOID *pbData;
	if (CurrentIndex==0)
	{
		nlverify (IndexBuffer->Lock (0, size, &pbData, 0) == D3D_OK);
	}
	else
	{
		nlverify (IndexBuffer->Lock (CurrentIndex, size, &pbData, D3DLOCK_NOOVERWRITE) == D3D_OK);
	}
	if(Driver->_IBProfiling /*&& Hardware*/)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		Driver->_VolatileIBLockTime += afterLock - beforeLock;
	}

	// Old buffer position
	offset = CurrentIndex / (Format == CIndexBuffer::Indices32 ? sizeof(uint32) : sizeof(uint16));

	// New buffer position
	CurrentIndex += size;
	Locked = true;
	return pbData;
}

// ***************************************************************************

void CVolatileIndexBuffer::unlock ()
{
	H_AUTO_D3D(CVolatileIndexBuffer_unlock);
	nlassertex(Locked, ("Volatile buffer usage should follow an atomic lock/unlock/render sequence"));
	nlverify (IndexBuffer->Unlock () == D3D_OK);
	Locked = false;
}

// ***************************************************************************
void CVolatileIndexBuffer::reset ()
{
	H_AUTO_D3D(CVolatileIndexBuffer_reset);
	CurrentIndex = 0;
}

// ***************************************************************************
bool CDriverD3D::buildQuadIndexBuffer()
{
	// this code will becomes useless when 16 bits buffer are really supported
	nlassert(!_QuadIB);
	uint numQuads = std::min(MAX_NUM_QUADS, (uint) (_MaxPrimitiveCount * 2)); // 2 primitives for each quads
	HRESULT r = _DeviceInterface->CreateIndexBuffer(sizeof(uint16) * 6 * numQuads, D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &_QuadIB, NULL);
	if (r != D3D_OK) return false;
	void *datas;
	r = _QuadIB->Lock(0, sizeof(uint16) * 6 * numQuads, &datas, 0);
	if (r != D3D_OK) return false;
	fillQuadIndexes((uint16 *) datas, 0, 6 * numQuads);
	_QuadIB->Unlock();
	return true;
}


// ***************************************************************************

} // NL3D
