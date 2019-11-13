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

#include "stddirect3d.h"

#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
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

// 500K min.
#define	NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE		(20*1024)



namespace NL3D
{

// ***************************************************************************

CVBDrvInfosD3D::CVBDrvInfosD3D(CDriverD3D *drv, ItVBDrvInfoPtrList it, CVertexBuffer *vb) : IVBDrvInfos(drv, it, vb)
{
	H_AUTO_D3D(CVBDrvInfosD3D_CVBDrvInfosD3D)
	VertexDecl = NULL;
	VertexDeclAliasDiffuseToSpecular = NULL;
	ColorOffset = 0;
	VertexBuffer = NULL;
	Usage = 0;
	VolatileVertexBuffer = NULL;
	VertexDeclNoDiffuse = NULL;
	#ifdef NL_DEBUG
		Locked = false;
	#endif
	Driver = drv;
}

// ***************************************************************************

uint vertexCount=0;

CVBDrvInfosD3D::~CVBDrvInfosD3D()
{
	H_AUTO_D3D(CVBDrvInfosD3D_CVBDrvInfosD3D)
	CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);
	// Restore non resident memory
	if (VertexBufferPtr)
	{
		VertexBufferPtr->setLocation(CVertexBuffer::NotResident);
		VertexBufferPtr = NULL;
	}

	// Don't release VertexDecl, it is release by the driver
	if (VertexBuffer && !Volatile)
	{
		if (Driver)
		{
			if (Driver->_VertexBufferCache.VertexBuffer == VertexBuffer)
			{
				Driver->_VertexBufferCache.VertexBuffer = NULL;
				Driver->touchRenderVariable(&Driver->_VertexBufferCache);
			}
		}
		vertexCount--;
		VertexBuffer->Release();
	}

	// Stats
	if (Hardware)
		driver->_VertexBufferHardSet.erase(this);

	#ifdef NL_DEBUG
		if (Locked)
		{
			nlinfo("VBuffer %s is still locked at destruction", VertexBufferPtr->getName().c_str())	;
			CDriverD3D *drv = NLMISC::safe_cast<CDriverD3D *>(_Driver);
			drv->_LockedBuffers.erase(this);
		}
	#endif
}



// ***************************************************************************

uint8	*CVBDrvInfosD3D::lock (uint begin, uint end, bool readOnly)
{
	H_AUTO_D3D(CVBDrvInfosD3D_lock)
	nlassert (begin != end);
	CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);
	//nlinfo("lock from %s", VertexBufferPtr->getName().c_str());
	#ifdef NL_DEBUG
		nlassert(!Locked);
		driver->_LockedBuffers.insert(this);
		Locked = true;
		static volatile bool dumpLockedBuffers = false;
		if (dumpLockedBuffers)
		{
			nlinfo("Num locked buffers = %d", (int) driver->_LockedBuffers.size());
			for(std::set<CVBDrvInfosD3D *>::iterator it = driver->_LockedBuffers.begin(); it != driver->_LockedBuffers.end(); ++it)
			{
				if (!(*it)->VertexBufferPtr)
				{
					nlinfo("Empty buffer");
				}
				else
				{
					nlinfo("Buffer %s at %p is Locked", (*it)->VertexBufferPtr->getName().c_str(), *it);
				}
			}
		}
	#endif
	if (Volatile)
	{
		// Lock the good buffer
		CVolatileVertexBuffer *&buffer = VolatileRAM ? (driver->_VolatileVertexBufferRAM[driver->_CurrentRenderPass&1]):
				(driver->_VolatileVertexBufferAGP[driver->_CurrentRenderPass&1]);
		uint8 *ptr = (uint8*)buffer->lock (end-begin, Stride, Offset);
		if (!ptr)
		{
			// buffer full, swap them
			CVolatileVertexBuffer *&bufferOther = VolatileRAM ? (driver->_VolatileVertexBufferRAM[(driver->_CurrentRenderPass + 1) &1]):
			(driver->_VolatileVertexBufferAGP[(driver->_CurrentRenderPass + 1) &1]);
			std::swap(buffer, bufferOther);
			buffer->reset();
			ptr = (uint8*)buffer->lock (end-begin, Stride, Offset);
			nlassert(ptr);
		}
		nlassert(!VolatileVertexBuffer);
		VolatileVertexBuffer = buffer;
		VertexBuffer = buffer->VertexBuffer;
		ptr -= begin;

		// Current lock time
		VolatileLockTime = driver->_CurrentRenderPass;

		// Touch the vertex buffer
		driver->touchRenderVariable (&driver->_VertexBufferCache);

		return ptr;
	}
	else
	{
		nlassert (VertexBuffer);
		// Lock Profile?
		TTicks	beforeLock = 0;
		if(driver->_VBHardProfiling /*&& Hardware*/)
		{
			beforeLock= CTime::getPerformanceTime();
		}

		void *pbData;
		if (VertexBuffer->Lock ( begin, end-begin, &pbData, readOnly?D3DLOCK_READONLY:0) != D3D_OK)
			return NULL;

		// Lock Profile?
		if(driver->_VBHardProfiling /*&& Hardware*/)
		{
			TTicks	afterLock;
			afterLock= CTime::getPerformanceTime();
			driver->appendVBHardLockProfile(afterLock-beforeLock, VertexBufferPtr);
		}
		return (uint8*)pbData;
	}
}

// ***************************************************************************

void	CVBDrvInfosD3D::unlock (uint /* begin */, uint /* end */)
{
	H_AUTO_D3D(CVBDrvInfosD3D_unlock )
	CDriverD3D *drv = NLMISC::safe_cast<CDriverD3D *>(_Driver);
	#ifdef NL_DEBUG
		nlassert(Locked);
		drv->_LockedBuffers.erase(this);
		Locked = false;
	#endif
	//nlinfo("unlock from %s", VertexBufferPtr->getName().c_str());
	if (Volatile)
	{
		nlassert(VolatileVertexBuffer);
		VolatileVertexBuffer->unlock ();
		VolatileVertexBuffer = NULL;
	}
	else
		VertexBuffer->Unlock ();
}

// ***************************************************************************

const D3DDECLTYPE RemapVertexBufferTypeNeL2D3D[CVertexBuffer::NumType]=
{
	D3DDECLTYPE_UNUSED, // Double1,
	D3DDECLTYPE_FLOAT1,	// Float1,
	D3DDECLTYPE_UNUSED, // Short1,
	D3DDECLTYPE_UNUSED, // Double2,
	D3DDECLTYPE_FLOAT2, // Float2,
	D3DDECLTYPE_SHORT2, // Short2,
	D3DDECLTYPE_UNUSED, // Double3,
	D3DDECLTYPE_FLOAT3, // Float3,
	D3DDECLTYPE_UNUSED, // Short3,
	D3DDECLTYPE_UNUSED, // Double4,
	D3DDECLTYPE_FLOAT4, // Float4,
	D3DDECLTYPE_SHORT4, // Short4,
	D3DDECLTYPE_D3DCOLOR, // UChar4,
};

// ***************************************************************************

const D3DDECLUSAGE RemapVertexBufferUsageNeL2D3D[CVertexBuffer::NumValue]=
{
	D3DDECLUSAGE_POSITION,	// Position
	D3DDECLUSAGE_NORMAL,	// Normal
	D3DDECLUSAGE_TEXCOORD,	// TexCoord0
	D3DDECLUSAGE_TEXCOORD,	// TexCoord1
	D3DDECLUSAGE_TEXCOORD,	// TexCoord2
	D3DDECLUSAGE_TEXCOORD,	// TexCoord3
	D3DDECLUSAGE_TEXCOORD,	// TexCoord4
	D3DDECLUSAGE_TEXCOORD,	// TexCoord5
	D3DDECLUSAGE_TEXCOORD,	// TexCoord6
	D3DDECLUSAGE_TEXCOORD,	// TexCoord7
	D3DDECLUSAGE_COLOR,		// PrimaryColor
	D3DDECLUSAGE_COLOR,		// SecondaryColor
	D3DDECLUSAGE_BLENDWEIGHT,	// Weight
	D3DDECLUSAGE_BLENDINDICES,	// PaletteSkin
	D3DDECLUSAGE_FOG,		// Fog
};

// ***************************************************************************

const uint RemapVertexBufferIndexNeL2D3D[CVertexBuffer::NumValue]=
{
	0,	// Position
	0,	// Normal
	0,	// TexCoord0
	1,	// TexCoord1
	2,	// TexCoord2
	3,	// TexCoord3
	4,	// TexCoord4
	5,	// TexCoord5
	6,	// TexCoord6
	7,	// TexCoord7
	0,	// PrimaryColor
	1,	// SecondaryColor
	0,	// Weight
	0,	// PaletteSkin
	0,	// Fog
};

// ***************************************************************************

DWORD RemapVertexBufferUsage[CVertexBuffer::LocationCount]=
{
	D3DUSAGE_DYNAMIC,						// RAMResident
	D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,	// AGPResident
	D3DUSAGE_WRITEONLY,						// VRAMResident
	0,										// Not used
};

// ***************************************************************************

D3DPOOL RemapVertexBufferPool[CVertexBuffer::LocationCount]=
{
	D3DPOOL_SYSTEMMEM,	// RAMResident
	D3DPOOL_DEFAULT,	// AGPResident
	D3DPOOL_DEFAULT,	// VRAMResident
	D3DPOOL_DEFAULT,	// Not used
};


// ***************************************************************************

bool CDriverD3D::activeVertexBuffer(CVertexBuffer& VB)
{
	H_AUTO_D3D(CDriverD3D_activeVertexBuffer)
	// Must not be locked
	nlassert (!VB.isLocked());

	// Must not be empty
	if (VB.capacity() == 0)
		return false;


	const bool touched = (VB.getTouchFlags() & (CVertexBuffer::TouchedReserve|CVertexBuffer::TouchedVertexFormat)) != 0;

	CVBDrvInfosD3D *info = static_cast<CVBDrvInfosD3D*>(static_cast<IVBDrvInfos*>(VB.DrvInfos));

	// Volatile buffers must be filled at each pass
	nlassertex (!info || !info->Volatile || VB.getKeepLocalMemory() || (info->VolatileLockTime == _CurrentRenderPass), ("Volatile buffers must be filled at each pass"));

	// Build the driver info
	if (touched)
	{
		// Delete previous vertex buffer info
		if (VB.DrvInfos)
		{
			delete VB.DrvInfos;
			nlassert (VB.DrvInfos == NULL);
		}

		// Force the vertex color format to BGRA
		VB.setVertexColorFormat (CVertexBuffer::TBGRA);

		// Rebuild it
		_VBDrvInfos.push_front (NULL);
		ItVBDrvInfoPtrList ite = _VBDrvInfos.begin();
		info = new CVBDrvInfosD3D(this, ite, &VB);
		*ite = info;

		// Use vertex color ?
		info->UseVertexColor = (VB.getVertexFormat()&CVertexBuffer::PrimaryColorFlag) != 0;
		info->Stride = (uint8)VB.getVertexSize();

		// Create the vertex declaration
		if (!createVertexDeclaration (VB.getVertexFormat(), VB.getValueTypePointer(), &(info->VertexDecl), info->ColorOffset, false, false))
			return false;
		info->VertexDeclAliasDiffuseToSpecular = NULL;
		info->VertexDeclNoDiffuse = NULL;
		if (VB.hasValueEx(CVertexBuffer::PrimaryColor) && !VB.hasValueEx(CVertexBuffer::SecondaryColor))
		{
			uint colorOffset2;
			if (!createVertexDeclaration (VB.getVertexFormat(), VB.getValueTypePointer(), &(info->VertexDeclAliasDiffuseToSpecular), colorOffset2, true, false))
			return false;
			nlassert(colorOffset2 == info->ColorOffset); // should be the same value
		}
		if (_NbNeLTextureStages == 3 && info->UseVertexColor)
		{
			// Fix for radeon 7xxx -> if vertex color is not used it should not be declared (example : lighted material + vertex color but, no vertexColorLighted)
			uint colorOffset2;
			if (!createVertexDeclaration (VB.getVertexFormat(), VB.getValueTypePointer(), &(info->VertexDeclNoDiffuse), colorOffset2, false, true))
			return false;
		}

		// Create the vertex buffer
		const uint size = VB.capacity()*VB.getVertexSize();
		uint preferredMemory = 0;
		if (_DisableHardwareVertexArrayAGP)
		{
			preferredMemory = CVertexBuffer::RAMResident;
			info->Volatile = false;
		}
		else
		{
			switch (VB.getPreferredMemory ())
			{
			case CVertexBuffer::RAMPreferred:
				preferredMemory = CVertexBuffer::RAMResident;
				info->Volatile = false;
				break;
			case CVertexBuffer::AGPPreferred:
				preferredMemory = CVertexBuffer::AGPResident;
				info->Volatile = false;
				break;
			case CVertexBuffer::StaticPreferred:
				if (getStaticMemoryToVRAM())
					preferredMemory = CVertexBuffer::VRAMResident;
				else
					preferredMemory = CVertexBuffer::AGPResident;
				info->Volatile = false;
				break;
			case CVertexBuffer::RAMVolatile:
				preferredMemory = CVertexBuffer::RAMResident;
				info->Volatile = true;
				break;
			case CVertexBuffer::AGPVolatile:
				preferredMemory = CVertexBuffer::AGPResident;
				info->Volatile = true;
				break;
			}
		}

		// Volatile vertex buffer
		if (info->Volatile)
		{
			nlassert (info->VertexBuffer == NULL);
			info->Hardware = false;
			info->VolatileRAM = preferredMemory == CVertexBuffer::RAMResident;
		}
		else
		{
			// Offset will be 0
			info->Offset = 0;

			bool success;
			do
			{
				success = _DeviceInterface->CreateVertexBuffer(size, RemapVertexBufferUsage[preferredMemory],
					0, RemapVertexBufferPool[preferredMemory], &(info->VertexBuffer), NULL) == D3D_OK;
				if (success)
					break;
			}
			while (preferredMemory--);
			if (!success)
				return false;

			++vertexCount;

			// Hardware ?
			info->Hardware = preferredMemory != CVertexBuffer::RAMResident;

			// Stats
			if (info->Hardware)
				_VertexBufferHardSet.insert(info);
		}

		// Release the local vertex buffer
		VB.DrvInfos = info;
		VB.setLocation ((CVertexBuffer::TLocation)preferredMemory);

		// Force the vertex buffer update
		touchRenderVariable (&_VertexDeclCache);
		touchRenderVariable (&_VertexBufferCache);
	}

	// Set the current vertex buffer
	nlassert (info);

	// Fill the buffer if in local memory
	VB.fillBuffer ();

	setVertexDecl (info->VertexDecl, info->VertexDeclAliasDiffuseToSpecular, info->VertexDeclNoDiffuse, info->Stride);
	//setVertexBuffer (info->VertexBuffer, info->Offset, info->Stride, info->UseVertexColor, VB.getNumVertices(), VB.getPreferredMemory(), info->Usage, info->ColorOffset);
	setVertexBuffer (info->VertexBuffer, info->Offset, info->Stride, info->UseVertexColor, VB.getNumVertices(), VB.getPreferredMemory(), info->Usage, info->ColorOffset);


	// Set UVRouting
	const uint8 *uvRouting = VB.getUVRouting();
	uint i;
	for (i=0; i<MaxTexture; i++)
		setTextureIndexUV (i, uvRouting[i]);


	// backup uv-routing, because some shader may change the routing
	// For example, if the same vb is used for lightmap, then for standard shader, then uv routing will be wrong
	std::copy(uvRouting, uvRouting + MaxTexture, _CurrentUVRouting);

	/* Hulud test : read in a "write only" vertex buffer. Seams to work well. */
/*
	// Read the vertex buffer
	CVBDrvInfosD3D *info = static_cast<CVBDrvInfosD3D*>(static_cast<IVBDrvInfos*>(VB.DrvInfos));
	static vector<uint8> temp;
	uint size = VB.capacity()*VB.getVertexSize();

	// No special flag for the lock, the driver should return a valid vertex buffer pointer with previous values.
	uint8 *out = info->lock (0, 0, false);
	nlassert (out);
	{
		temp.resize (size);
		memcpy (&(temp[0]), out, size);
	}
	info->unlock (0, 0);
	out = info->lock (0, 0, false);
	nlassert (out);
	{
		memcpy (out, &(temp[0]), size);
	}
	info->unlock (0, 0);
*/

	return true;
}


// ***************************************************************************
bool CDriverD3D::createVertexDeclaration (uint16 vertexFormat, const uint8 *typeArray,
										IDirect3DVertexDeclaration9 **vertexDecl,
										uint &colorOffset,
										bool aliasDiffuseToSpecular,
										bool bypassDiffuse,
										uint *stride)
{
	H_AUTO_D3D(CDriverD3D_createVertexDeclaration)
	CVertexDeclaration declaration;

	if (aliasDiffuseToSpecular)
	{
		// there should be a single color stream : diffuse
		nlassert(vertexFormat & CVertexBuffer::PrimaryColorFlag);       // diffuse required
		nlassert(!(vertexFormat & CVertexBuffer::SecondaryColorFlag)); // specular should not be used
	}

	// Set the vertex format
	uint i;
	uint j = 0;
	uint offset = 0;
	colorOffset = 0;
	for (i=0; i<CVertexBuffer::NumValue; i++)
	{
		// Slot used ?
		if (vertexFormat & (1<<i))
		{
			if ((i != CVertexBuffer::Weight && i != CVertexBuffer::PaletteSkin) || _PixelShaderVersion != D3DPS_VERSION(1, 4)) // fix for radeon 8500/9000/9200 : hand when this is declared and not used
																															   // don't let gap for other cards else render bug on some ...
			{
				D3DVERTEXELEMENT9 &vertexElement = declaration.VertexElements[j];
				vertexElement.Stream = 0;
				vertexElement.Type = BYTE(RemapVertexBufferTypeNeL2D3D[(uint)typeArray[i]]);
				vertexElement.Offset = WORD(offset);
				vertexElement.Method = D3DDECLMETHOD_DEFAULT;
				vertexElement.Usage = BYTE(RemapVertexBufferUsageNeL2D3D[(uint)i]);
				if (aliasDiffuseToSpecular && i == CVertexBuffer::PrimaryColor)
				{
					vertexElement.UsageIndex = 1; // Map to specular stream -> this free PrimaryColor to build a constant
												  // Ueful to emulate per stage constant (which we can do on 2 stages only)
				}
				else
				{
					vertexElement.UsageIndex = BYTE(RemapVertexBufferIndexNeL2D3D[(uint)i]);
				}

				// nico : Fix for Radeon 7xxx series
				// Vertex declaration doesn't work when the vertex layout has vertex color defined after tex coord.
				// For example, the following layout (Position/TexCoord0/Diffuse) will silently be converted into (Position/Diffuse/TexCoord0)
				// It seems that the driver tries to map the vertex declaration to the matching FVF. FVF has a prefined order and requires Diffuse to appear
				// before texture coordinates in the vertex. Don't know if it is a limitation of D3D related to the 7xxx sries of if it is a driver bug.
				// The D3D debug dll doesn't issue a warning about it.
				// To solve this 2 vertex streams are declared :
				// - First streams contains Position/Normal/Texcoord
				// - When vertex color are used, second stream contains Diffuse/Specular vertex component(s)
				// In fact the 2 streams map to the same vertex buffer, but the 2nd stream has an added offset to point on the color component
				// I tried to add this offset directly into the vertex declaration, but D3D complains about it...
				// If the following field contains a non 0 value, then a second stream must be used for diffuse/specular with the given offset
				if (_NbNeLTextureStages == 3)
				{
					if (vertexElement.Usage == D3DDECLUSAGE_COLOR)
					{
						if (bypassDiffuse)
						{
							continue;
						}
						vertexElement.Stream = 1;
						if (colorOffset == 0)
						{
							vertexElement.Offset = 0;
							colorOffset = offset;
						}
						else
						{
							vertexElement.Offset = 4;
						}
					}
				}
				j++;
			}
			offset += CVertexBuffer::SizeType[typeArray[i]];
		}
	}

	// Set the stride ?
	if (stride)
		*stride = offset;

	// End
	D3DVERTEXELEMENT9 end = D3DDECL_END();
	declaration.VertexElements[j] = end;

	// Look for the same vertex declaration
	std::list<CVertexDeclaration>::iterator ite = _VertexDeclarationList.begin();
	while (ite != _VertexDeclarationList.end())
	{
		for (i=0; i<=j; i++)
		{
			const D3DVERTEXELEMENT9 &vertexElementNew = declaration.VertexElements[i];
			const D3DVERTEXELEMENT9 &vertexElementOld = ite->VertexElements[i];
			if ( (vertexElementNew.Stream != vertexElementOld.Stream) ||
				(vertexElementNew.Type != vertexElementOld.Type) ||
				(vertexElementNew.Offset != vertexElementOld.Offset) ||
				(vertexElementNew.Method != vertexElementOld.Method) ||
				(vertexElementNew.Usage != vertexElementOld.Usage) ||
				(vertexElementNew.UsageIndex != vertexElementOld.UsageIndex))
			{

				break;
			}
		}

		// All is good ?
		if (i == (j+1))
		{
			// It is the same vertex declaration
			*vertexDecl = ite->VertexDecl;
			return true;
		}

		ite++;
	}

	// Not found, create the vertex declaration
	if (_DeviceInterface->CreateVertexDeclaration (declaration.VertexElements, &(declaration.VertexDecl)) != D3D_OK)
	{
		return false;
	}

	// Add the vertex declaration
	_VertexDeclarationList.push_back (declaration);

	// Set the final declaration pointer
	*vertexDecl = declaration.VertexDecl;

	return true;

}


// ***************************************************************************

bool CDriverD3D::supportVertexBufferHard() const
{
	H_AUTO_D3D(CDriverD3D_supportVertexBufferHard)
	return !_DisableHardwareVertexArrayAGP;
}

// ***************************************************************************

bool CDriverD3D::supportVolatileVertexBuffer() const
{
	H_AUTO_D3D(CDriverD3D_supportVolatileVertexBuffer)
	return true;
}


// ***************************************************************************

void CDriverD3D::disableHardwareVertexArrayAGP()
{
	H_AUTO_D3D(CDriverD3D_disableHardwareVertexArrayAGP)
	_DisableHardwareVertexArrayAGP = true;
}

// ***************************************************************************

uint CDriverD3D::getMaxVerticesByVertexBufferHard() const
{
	H_AUTO_D3D(CDriverD3D_getMaxVerticesByVertexBufferHard)
	return _MaxVerticesByVertexBufferHard;
}

// ***************************************************************************

uint32 CDriverD3D::getAvailableVertexAGPMemory ()
{
	H_AUTO_D3D(CDriverD3D_getAvailableVertexAGPMemory )
	return _AGPMemoryAllocated;
}

// ***************************************************************************

uint32 CDriverD3D::getAvailableVertexVRAMMemory ()
{
	H_AUTO_D3D(CDriverD3D_getAvailableVertexVRAMMemory )
	return _VRAMMemoryAllocated;
}

// ***************************************************************************

bool CDriverD3D::initVertexBufferHard(uint agpMem, uint vramMem)
{
	H_AUTO_D3D(CDriverD3D_initVertexBufferHard)
	if(!supportVertexBufferHard())
		return false;

	// First, reset any VBHard created.
	bool	ok= true;

	// Try to allocate AGPMemory.
	_AGPMemoryAllocated = agpMem;
	if(_AGPMemoryAllocated>0)
	{
		_AGPMemoryAllocated&= ~15;	// ensure 16-bytes aligned mem count (maybe useful :) ).
		_AGPMemoryAllocated= max(_AGPMemoryAllocated, (uint32)NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE);
		while(_AGPMemoryAllocated >= NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			IDirect3DVertexBuffer9 *vb;
			if (_DeviceInterface->CreateVertexBuffer (_AGPMemoryAllocated, D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, 0,
				D3DPOOL_DEFAULT, &vb, NULL) == D3D_OK)
			{
				D3DVERTEXBUFFER_DESC desc;
				nlverify (vb->GetDesc (&desc) == D3D_OK);
				if (((desc.Usage&(D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC)) == (D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC)) &&
					(desc.Pool == D3DPOOL_DEFAULT))
				{
					nlinfo("%.d vertices supported", _MaxVerticesByVertexBufferHard);
					nlinfo("Success to allocate %.1f Mo of AGP VAR Ram", _AGPMemoryAllocated / 1000000.f);
					vb->Release();
					break;
				}
				else
					vb->Release();
			}
			else
			{
				_AGPMemoryAllocated/=2;
				_AGPMemoryAllocated &=~15;
			}
		}

		if(_AGPMemoryAllocated< NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			nlwarning("%.d vertices supported", _MaxVerticesByVertexBufferHard);
			nlwarning("Failed to allocate %.1f Mo of AGP VAR Ram", NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE / 1000000.f);
			ok= false;
		}
	}


	// Try to allocate VRAMMemory.
	_VRAMMemoryAllocated = vramMem;
	if(_VRAMMemoryAllocated>0)
	{
		_VRAMMemoryAllocated&= ~15;	// ensure 16-bytes aligned mem count (maybe useful :) ).
		_VRAMMemoryAllocated= max(_VRAMMemoryAllocated, (uint32)NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE);
		while(_VRAMMemoryAllocated>= NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			IDirect3DVertexBuffer9 *vb;
			if (_DeviceInterface->CreateVertexBuffer (_VRAMMemoryAllocated, D3DUSAGE_WRITEONLY, 0,
				D3DPOOL_DEFAULT, &vb, NULL) == D3D_OK)
			{
				vb->Release();
				break;
			}
			else
			{
				_VRAMMemoryAllocated/=2;
				_VRAMMemoryAllocated &=~15;
			}
		}

		if(_VRAMMemoryAllocated< NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			ok= false;
		}
	}

	return ok;
}

// ***************************************************************************

void CDriverD3D::mapTextureStageToUV(uint stage, uint uv)
{
	H_AUTO_D3D(CDriverD3D_mapTextureStageToUV)
	setTextureIndexUV (stage, uv);
}

// ***************************************************************************
// CVolatileVertexBuffer
// ***************************************************************************

CVolatileVertexBuffer::CVolatileVertexBuffer()
{
	H_AUTO_D3D(CVolatileVertexBuffer_CVolatileVertexBuffer)
	VertexBuffer = NULL;
	Locked = false;
}

// ***************************************************************************

CVolatileVertexBuffer::~CVolatileVertexBuffer()
{
	H_AUTO_D3D(CVolatileVertexBuffer_CVolatileVertexBufferDtor)
	release ();
}

// ***************************************************************************

void CVolatileVertexBuffer::release ()
{
	H_AUTO_D3D(CVolatileVertexBuffer_release )
	if (VertexBuffer)
		VertexBuffer->Release();
	VertexBuffer = NULL;
}

// ***************************************************************************

void CVolatileVertexBuffer::init (CVertexBuffer::TLocation	location, uint size, uint maxSize, CDriverD3D *driver)
{
	H_AUTO_D3D(CVolatileVertexBuffer_init )
	release();
	if (maxSize < size) maxSize = size;
	MaxSize = maxSize;
	// Init the buffer
	Location = location;
	Size = size;
	Driver = driver;

	// Allocate the vertex buffer
	if (Driver->_DeviceInterface->CreateVertexBuffer(size, RemapVertexBufferUsage[location],
		0, RemapVertexBufferPool[location], &VertexBuffer, NULL) != D3D_OK)
	{
		// Location in RAM must not failed
		nlassert (location != CVertexBuffer::RAMResident);

		// Allocate in RAM
		nlverify (Driver->_DeviceInterface->CreateVertexBuffer(size, RemapVertexBufferUsage[CVertexBuffer::RAMResident],
				0, RemapVertexBufferPool[CVertexBuffer::RAMResident], &VertexBuffer, NULL) != D3D_OK);

		Location = CVertexBuffer::RAMResident;
	}
}

// ***************************************************************************

//volatile int callCount = 0;
//volatile int callStop = 17700;



void *CVolatileVertexBuffer::lock (uint size, uint stride, uint &offset)
{
	nlassertex(!Locked, ("Volatile buffer usage should follow an atomic lock/unlock/render sequence"));
	H_AUTO_D3D(CVolatileVertexBuffer_lock)
	/* If not enough room to allocate this buffer, resise the buffer to Size+Size/2 but do not reset CurrentIndex
	 * to be sure the buffer will be large enough next pass. */

	//if (callCount == callStop)
	//	nlstop;
	//callCount++;

	// Align the index
	uint mod = CurrentIndex / stride;
	if (CurrentIndex != (mod*stride))
		CurrentIndex = (mod+1)*stride;


	// Enough room for this vertex ?
	if (CurrentIndex+size+stride > Size)
	{
		if (CurrentIndex+size > MaxSize && CurrentIndex != 0)
		{
			reset();
			if (size > MaxSize)
			{
				init (Location, std::max (std::min(Size+Size/2, MaxSize),  size), MaxSize, Driver);
			}
		}
		else
		{
			// Max size not reached, so reallocate
			init (Location, std::max (std::min(Size+Size/2, MaxSize),  CurrentIndex+size), MaxSize, Driver);
			reset(); // reallocate will cause a cpu stall anyway ...
		}
	}

	// Lock Profile?
	TTicks	beforeLock = 0;
	if(Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	// Lock the buffer, noblocking lock here if not the first allocation since a reset

	VOID *pbData;
	if (CurrentIndex==0)
	{
		nlverify (VertexBuffer->Lock (0, Size, &pbData, D3DLOCK_DISCARD) == D3D_OK);
	}
	else
	{
		nlverify (VertexBuffer->Lock (CurrentIndex, size, &pbData, D3DLOCK_NOOVERWRITE) == D3D_OK);
	}
	if(Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		Driver->_VolatileVBLockTime += afterLock - beforeLock;
	}

	// Old buffer position
	offset = CurrentIndex/stride;

	// New buffer position
	CurrentIndex += size;
	Locked = true;
	return pbData;
}

// ***************************************************************************

void CVolatileVertexBuffer::unlock ()
{
	H_AUTO_D3D(CVolatileVertexBuffer_unlock )
	nlassertex(Locked, ("Volatile buffer usage should follow an atomic lock/unlock/render sequence"));
	nlverify (VertexBuffer->Unlock () == D3D_OK);
	Locked = false;
}

// ***************************************************************************
void CVolatileVertexBuffer::reset ()
{
	H_AUTO_D3D(CVolatileVertexBuffer_reset )
	CurrentIndex = 0;
//	callCount = 0;
}

// ***************************************************************************

} // NL3D





















