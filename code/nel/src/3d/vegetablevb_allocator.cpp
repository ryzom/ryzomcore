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


#include "nel/3d/vegetablevb_allocator.h"
#include "nel/3d/vegetable_def.h"


using namespace std;
using namespace NLMISC;

namespace NL3D
{


/*
	Once a reallocation of a VB occurs, how many vertices we add to the re-allocation, to avoid
	as possible reallocations.
*/
#define	NL3D_VEGETABLE_VERTEX_ALLOCATE_SECURITY		1024
/*
	The start size of the array.
*/
#define	NL3D_VEGETABLE_VERTEX_ALLOCATE_START		4048


#define	NL3D_VEGETABLE_VERTEX_FREE_MEMORY_RESERVE	1024


// ***************************************************************************
CVegetableVBAllocator::CVegetableVBAllocator()
{
	_Type= VBTypeUnlit;
	_MaxVertexInBufferHard= 0;

	// Init free list
	_VertexFreeMemory.reserve(NL3D_VEGETABLE_VERTEX_FREE_MEMORY_RESERVE);
	_NumVerticesAllocated= 0;

	// Init vbhard
	_VBHardOk= false;
	_AGPBufferPtr= NULL;
	_RAMBufferPtr= NULL;
}


// ***************************************************************************
void			CVegetableVBAllocator::init(TVBType type, uint maxVertexInBufferHard)
{
	_Type= type;
	_MaxVertexInBufferHard= maxVertexInBufferHard;

	// According to _Type, build VB format, and create VertexProgram
	setupVBFormat();
}


// ***************************************************************************
CVegetableVBAllocator::~CVegetableVBAllocator()
{
	clear();
}

// ***************************************************************************
void			CVegetableVBAllocator::updateDriver(IDriver *driver)
{
	// test change of driver.
	nlassert(driver && !_VBHard.isLocked());
	// If change of driver
	if( _Driver==NULL || driver!=_Driver || (!_VBHard.isResident() && (_VBHard.capacity()!=0)))
	{
		// delete old VBHard.
		deleteVertexBufferHard();
		_Driver= driver;
		_VBHardOk= (_MaxVertexInBufferHard>0) && (_Driver->supportVertexBufferHard());
		/* Because so much lock/unlock are performed during a frame (refine/clip etc...).
			we must disable VBHard for ATI Gl extension.
			NB: CLandscape don't do this and fast copy the entire VB each frame.
			This is not possible for vegetables because the VB describe all Vegetable around the camera, not only
			what is in frustrum. Hence a fast copy each frame would copy far too much unseen vertices (4x).
		*/
		if(_Driver->slowUnlockVertexBufferHard())
			_VBHardOk= false;

		// Driver must support VP.
		nlassert(_Driver->supportVertexProgram());

		// must reallocate the VertexBuffer.
		if( _NumVerticesAllocated>0 )
			allocateVertexBufferAndFillVBHard(_NumVerticesAllocated);
	}
	else
	{
		// if VBHard possible, and if vbHardDeleted but space needed, reallocate.
		if( _VBHardOk && _VBHard.getNumVertices()==0 && _NumVerticesAllocated>0 )
			allocateVertexBufferAndFillVBHard(_NumVerticesAllocated);
	}

}

// ***************************************************************************
void			CVegetableVBAllocator::clear()
{
	// clear list.
	_VertexFreeMemory.clear();
	_NumVerticesAllocated= 0;

	// must unlock for vbhard and vbsoft
	unlockBuffer();

	// delete the VB.
	deleteVertexBufferHard();
	// really delete the VB soft too
	_VBSoft.deleteAllVertices();

	// clear other states.
	_Driver= NULL;
	_VBHardOk= false;
}


// ***************************************************************************
void			CVegetableVBAllocator::lockBuffer()
{
	// force unlock
	unlockBuffer();

	// need to lock only if the VBHard is created
	if(_VBHardOk)
	{
		// lock the VBHard for writing
		_VBHard.lock(_VBAHard);
		_AGPBufferPtr=(uint8*)_VBAHard.getVertexCoordPointer();

		// lock the Input VertexBuffer for reading
		_VBSoft.lock(_VBASoft);
		_RAMBufferPtr=(const uint8*)_VBASoft.getVertexCoordPointer();
	}
}

// ***************************************************************************
void			CVegetableVBAllocator::unlockBuffer()
{
	// unlock the VBHard
	_VBAHard.unlock();
	_AGPBufferPtr= NULL;

	// unlock the VBSoft
	_VBASoft.unlock();
	_RAMBufferPtr= NULL;
}


// ***************************************************************************
uint			CVegetableVBAllocator::getNumUserVerticesAllocated() const
{
	// get the number of vertices which are allocated by allocateVertex().
	return _NumVerticesAllocated - (uint)_VertexFreeMemory.size();
}

// ***************************************************************************
bool			CVegetableVBAllocator::exceedMaxVertexInBufferHard(uint numAddVerts) const
{
	return (getNumUserVerticesAllocated() + numAddVerts) > _MaxVertexInBufferHard;
}


// ***************************************************************************
uint			CVegetableVBAllocator::allocateVertex()
{
	// if no more free, allocate.
	if( _VertexFreeMemory.size()==0 )
	{
		// enlarge capacity.
		uint	newResize;
		if(_NumVerticesAllocated==0)
			newResize= NL3D_VEGETABLE_VERTEX_ALLOCATE_START;
		else
			newResize= NL3D_VEGETABLE_VERTEX_ALLOCATE_SECURITY;
		// try to not overlap _MaxVertexInBufferHard limit, to avoid VBufferHard to be disabled.
		if(_NumVerticesAllocated<_MaxVertexInBufferHard && _NumVerticesAllocated+newResize > _MaxVertexInBufferHard)
		{
			newResize= _MaxVertexInBufferHard - _NumVerticesAllocated;
		}
		_NumVerticesAllocated+= newResize;
		// re-allocate VB.
		allocateVertexBufferAndFillVBHard(_NumVerticesAllocated);
		// resize infos on vertices.
		_VertexInfos.resize(_NumVerticesAllocated);

		// Fill list of free elements.
		for(uint i=0;i<newResize;i++)
		{
			// create a new entry which points to this vertex.
			// the list is made so allocation is in growing order.
			_VertexFreeMemory.push_back( _NumVerticesAllocated - (i+1) );

			// Mark as free the new vertices. (Debug).
			_VertexInfos[_NumVerticesAllocated - (i+1)].Free= true;
		}
	}

	// get a vertex (pop_back).
	uint	id= _VertexFreeMemory.back();
	// delete this vertex free entry.
	_VertexFreeMemory.pop_back();

	// check and Mark as not free the vertex. (Debug).
	nlassert(id<_NumVerticesAllocated);
	nlassert(_VertexInfos[id].Free);
	_VertexInfos[id].Free= false;


	return id;
}

// ***************************************************************************
void			CVegetableVBAllocator::deleteVertex(uint vid)
{
	// check and Mark as free the vertex. (Debug).
	nlassert(vid<_NumVerticesAllocated);
	nlassert(!_VertexInfos[vid].Free);
	_VertexInfos[vid].Free= true;

	// Add this vertex to the free list.
	// create a new entry which points to this vertex.
	_VertexFreeMemory.push_back( vid );
}

// ***************************************************************************
void			CVegetableVBAllocator::flushVertex(uint i)
{
	if(_VBHardOk)
	{
		nlassert(_VBHard.getNumVertices() && _VBHard.isLocked() && _VBSoft.isLocked());

		// copy the VB soft to the VBHard.
		uint	size= _VBSoft.getVertexSize();
		const void	*src= _RAMBufferPtr + i*size;
		void		*dst= _AGPBufferPtr + i*size;
		CHECK_VBA_RANGE(_VBAHard, (uint8 *) dst, size);
		CHECK_VBA_RANGE(_VBASoft, (uint8 *) src, size);
		memcpy(dst, src, size);
	}
}

// ***************************************************************************
void			CVegetableVBAllocator::activate()
{
	nlassert(_Driver);
	nlassert(!_VBHard.isLocked());
	nlassert(!_VBSoft.isLocked());

	// Activate VB.
	if(_VBHard.getNumVertices())
		_Driver->activeVertexBuffer(_VBHard);
	else
		_Driver->activeVertexBuffer(_VBSoft);
}


// ***************************************************************************
// ***************************************************************************
// Vertex Buffer hard.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void				CVegetableVBAllocator::deleteVertexBufferHard()
{
	// must unlock VBhard before.
	unlockBuffer();

	// test (refptr) if the object still exist in memory.
	if(_VBHard.getNumVertices()!=0)
	{
		// A vbufferhard should still exist only if driver still exist.
		nlassert(_Driver!=NULL);

		// delete it from driver.
		_VBHard.deleteAllVertices ();
	}

}

// ***************************************************************************
void				CVegetableVBAllocator::allocateVertexBufferAndFillVBHard(uint32 numVertices)
{
	// no allocation must be done if the Driver is not setuped, or if the driver has been deleted by refPtr.
	nlassert(_Driver);

	// must unlock VBhard and VBSoft before.
	bool	wasLocked= bufferLocked();
	unlockBuffer();

	// resize the Soft VB.
	_VBSoft.setNumVertices(numVertices);

	// try to allocate a vbufferhard if possible.
	if( _VBHardOk )
	{
		/* Prefer allocate the VBHard with the Max Vertex count only ONCE,
			to avoid problems with AGP allocation
			The problem is with 50000 AGP vertices, it costs 3 Mo. If we do iterative allocation
			(500 Ko, 600 Ko, 700 Ko,.....)
			we may have problem with free holes (Vegetable could no more enter in the 16 or 8 Mo AGP limit!)
		*/
		if(_VBHard.getNumVertices() != _MaxVertexInBufferHard)
		{
			// delete possible old _VBHard.
			if(_VBHard.getNumVertices()!=0)
			{
				// VertexBufferHard lifetime < Driver lifetime.
				nlassert(_Driver!=NULL);
				_VBHard.deleteAllVertices();
			}

			// try to create new one, in AGP Ram
			// If too many vertices wanted, abort VBHard.
			if(numVertices <= _MaxVertexInBufferHard)
			{
				_VBHard = _VBSoft;
				_VBHard.setPreferredMemory(CVertexBuffer::AGPPreferred, false);
				_VBHard.setNumVertices (_MaxVertexInBufferHard);

				// Force this VB to be hard
				nlverify (_Driver->activeVertexBuffer (_VBHard));
				nlassert (_VBHard.isResident());

				// if fails, abort VBHard.
				if (_VBHard.getLocation() == CVertexBuffer::RAMResident)
					_VBHard.deleteAllVertices();

				// Set Name For lock Profiling.
				if(_VBHard.getNumVertices()!=0)
					_VBHard.setName("VegetableVB");
			}
			else
				_VBHard.deleteAllVertices();

			// If KO, never try again.
			if(_VBHard.getNumVertices()==0)
				_VBHardOk= false;
		}
	}

	// if still OK, must refill the VBHard. Slow, but rare
	if(_VBHardOk)
	{
		// else, fill this AGP VBuffer Hard.
		// lock before the AGP buffer
		lockBuffer();

		// copy all the vertices to AGP.
		memcpy(_AGPBufferPtr, _RAMBufferPtr, _VBSoft.getVertexSize() * numVertices);

		// If was not locked before, unlock this VB
		if(!wasLocked)
			unlockBuffer();
	}

	//nlinfo("VEGET: Alloc %d verts. %s", numVertices, _VBHardOk?"VBHard":"VBSoft");
}


// ***************************************************************************
void				CVegetableVBAllocator::setupVBFormat()
{
	// Build the Vertex Format.
	_VBSoft.clearValueEx();

	// if lighted, need world space normal and AmbientColor for each vertex.
	if( _Type == VBTypeLighted )
	{
		_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_POS,	CVertexBuffer::Float3);		// v[0]
		_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_NORMAL, CVertexBuffer::Float3);		// v[2]
		_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_BENDINFO,	CVertexBuffer::Float3);		// v[9]
	}
	// If unlit
	else
	{
		// slightly different VertexProgram, v[0].w== BendWeight, and v[9].x== v[0].norm()
		_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_POS,		CVertexBuffer::Float4);		// v[0]
		// Unlit VP has BlendDistance in v[9].w
		_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_BENDINFO,	CVertexBuffer::Float4);		// v[9]
	}
	_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_COLOR0,		CVertexBuffer::UChar4);		// v[3]
	_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_COLOR1,		CVertexBuffer::UChar4);		// v[4]
	_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_TEX0,		CVertexBuffer::Float2);		// v[8]
	_VBSoft.addValueEx(NL3D_VEGETABLE_VPPOS_CENTER,		CVertexBuffer::Float3);		// v[10]
	_VBSoft.initEx();

}



} // NL3D
