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

#ifndef NL_VEGETABLEVB_ALLOCATOR_H
#define NL_VEGETABLEVB_ALLOCATOR_H

#include "nel/misc/types_nl.h"
#include "nel/3d/driver.h"


namespace NL3D
{


// ***************************************************************************
// Vegetable VertexProgram: Position of vertices in VertexBuffer.
#define	NL3D_VEGETABLE_VPPOS_POS		(CVertexBuffer::Position)
#define	NL3D_VEGETABLE_VPPOS_NORMAL		(CVertexBuffer::Normal)
#define	NL3D_VEGETABLE_VPPOS_COLOR0		(CVertexBuffer::PrimaryColor)
#define	NL3D_VEGETABLE_VPPOS_COLOR1		(CVertexBuffer::SecondaryColor)
#define	NL3D_VEGETABLE_VPPOS_TEX0		(CVertexBuffer::TexCoord0)
#define	NL3D_VEGETABLE_VPPOS_BENDINFO	(CVertexBuffer::TexCoord1)
#define	NL3D_VEGETABLE_VPPOS_CENTER		(CVertexBuffer::TexCoord2)


// ***************************************************************************
/**
 * A VB allocator (landscape like).
 *	Big difference is that here, we do not really matter about reallocation because both software
 *	and hardware VB are present. Also, VertexProgram MUST be supported by driver here.
 *	NB: unlike Landscape VBAllocator, the VertexProgram is not managed by this class.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableVBAllocator
{
public:

	enum	TVBType	{VBTypeLighted=0, VBTypeUnlit, VBTypeCount};

	/// Constructor
	CVegetableVBAllocator();
	~CVegetableVBAllocator();
	/** init the VB allocator, with the good type. must do it first.
	 *	maxVertexInBufferHard is the number of AGP vertex allocated. if 0 => buffer soft only
	 */
	void			init(TVBType vbType, uint maxVertexInBufferHard);


	/** setup driver, and test for possible VBHard reallocation. if reallocation, refill the VBHard
	 *	to do anytime you're not sure of change of the driver/vbHard state.
	 *
	 *	\param driver must not be NULL.
	 */
	void			updateDriver(IDriver *driver);


	// delete all VB, and free driver ressources (if RefPtr driver not deleted). clear list too.
	void			clear();


	/// \name Allocation.
	// @{

	/**	if I add numAddVerts vertices, will it overide _MaxVertexInBufferHard ??
	 */
	bool			exceedMaxVertexInBufferHard(uint numAddVerts) const;
	/** return number of vertices allocated with allocateVertex() (NB: do not return the actual number of
	 *	vertices allocated in VBuffer, but the number of vertices asked to be allocated).
	 */
	uint			getNumUserVerticesAllocated() const;

	/// Allocate free vertices in VB. (RAM and AGP if possible). work with locked or unlocked buffer.
	uint			allocateVertex();
	/// Delete free vertices in VB. (AGP or RAM).
	void			deleteVertex(uint vid);

	// @}


	/// \name Buffer access.
	// @{
	// return soft VB, for info only.
	CVertexBuffer			&getSoftwareVertexBuffer() {return _VBSoft;}
	// return soft VB, for info only.
	const CVertexBuffer		&getSoftwareVertexBuffer() const {return _VBSoft;}
	/// If VBHard ok, copy the vertex in AGP. Warning: buffer must be locked!
	void			flushVertex(uint i);

	/// if any, lock the AGP buffer.
	void			lockBuffer();
	/// if any, unlock the AGP buffer.
	void			unlockBuffer();
	bool			bufferLocked() const {return _VBHard.isLocked();}

	/// true if the VBHard is in BGRA mode
	bool			isBGRA() const {return _VBHard.getVertexColorFormat()==CVertexBuffer::TBGRA;}

	/** activate the VB or the VBHard in Driver setuped. nlassert if driver is NULL or if buffer is locked.
	 */
	void			activate();
	// @}


// ******************
private:

	// For Debug.
	struct	CVertexInfo
	{
		bool	Free;
	};

private:
	TVBType						_Type;

	// List of vertices free.
	std::vector<uint>			_VertexFreeMemory;
	std::vector<CVertexInfo>	_VertexInfos;
	uint						_NumVerticesAllocated;


	/// \name VB mgt .
	// @{

	// Our software VB. always here, and always correct.
	CVertexBuffer						_VBSoft;
	CVertexBuffer						_VBHard;
	CVertexBufferRead					_VBASoft;
	CVertexBufferReadWrite				_VBAHard;

	// a refPtr on the driver, to delete VBuffer Hard at clear().
	NLMISC::CRefPtr<IDriver>			_Driver;
	// tell if VBHard is possible.
	bool								_VBHardOk;
	const uint8							*_RAMBufferPtr;
	uint8								*_AGPBufferPtr;
	/// Maximum vertices in BufferHard allowed for this VBAllocator
	uint								_MaxVertexInBufferHard;


	/// delete only the Vertexbuffer hard.
	void				deleteVertexBufferHard();
	/* create a VertexBufferSoft, and try to create a vertexBufferHard
		After this call, the vertexBufferHard may be NULL.
		if VBHard allocation, copy from soft.
	*/
	void				allocateVertexBufferAndFillVBHard(uint32 numVertices);

	// init VB according to type. called in cons() only.
	void				setupVBFormat();

	// @}


};


} // NL3D


#endif // NL_VEGETABLEVB_ALLOCATOR_H

/* End of vegetablevb_allocator.h */
