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

#ifndef NL_VERTEX_STREAM_MANAGER_H
#define NL_VERTEX_STREAM_MANAGER_H

#include "nel/misc/types_nl.h"


#include "nel/misc/types_nl.h"
#include "nel/3d/driver.h"


namespace NL3D
{


// ***************************************************************************
/**
 * A class used to fill a virtual Vertexbuffer, while rendering it, avoiding Stalls during Locks. Actually multiple versions
 *	of VertexBuffer are kept and the swapVBHard() method cycles around them.
 *	NB: it is used for instance to render all skins of a CSkeletonModel, in just one CVertexStreamManager.
 *	This allow optimisation because less VBuffer activation is needed.
 * NB: only a single lock/unlock may happen between 2 calls of swapVBHard
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CVertexStreamManager
{
public:

	/// Constructor
	CVertexStreamManager();
	~CVertexStreamManager();

	/// \name Init/Setup
	// @{

	/** init the manager with a driver, allocate the VBHards, and setup the vertexFormat.
	 *	Must call each time the drive changes.
	 *	NB: if VBufferHard creation fail (not supported etc...), then a VBSoft is created instead (and just one since not useful)
	 *	\param numVBHard the number of VBHard to create. The max you set, the lower lock you'll have.
	 *	\param vbName base, for Lock profiling
	 *  \param allowVolatileVertexBuffer allow to use a volatile vertex buffer instead of several buffers on devices that support it
	 *         NB : in this case, you should not keep the vertex buffer locked outside of render, you should follow an atomic lock/unlock/render sequence
	 */
	void			init(IDriver *driver, uint vertexFormat, uint maxVertices, uint numVBHard, const std::string &vbName, bool allowVolatileVertexBuffer = false);
	/// release the VBHard. init() can be called after this.
	void			release();

	/// get the numVBhard used (Nb: if !vbHardMode(), still returns the argument passed in init(), ie not 0 or 1)
	uint			getNumVB() const {return _NumVB;}
	/// return the driver used.
	IDriver			*getDriver() const {return _Driver;}
	/// get the vertexFormat
	uint			getVertexFormat() const {return _VertexFormat;}
	/// get the vertexSize
	uint			getVertexSize() const {return _VertexSize;}
	/// get max vertices the Buffer allows.
	uint			getMaxVertices() const {return _MaxVertices;}

	// @}

	/// \name Rendering. Those method assert if init() not called with correct parameters.
	// @{

	/** lock the currently activated VBHard, for future filling
	  * NB: only a single lock/unlock may happen between 2 calls of swapVBHard
	  */
	uint8			*lock();
	/** unlock the currently activated VBHard. Tell how many vertices have changed.
	  * NB: only a single lock/unlock may happen between 2 calls of swapVBHard
	  */
	void			unlock(uint numVertices);

	/// activate the currentVBhard as the current VB in the driver, for future rendering
	void			activate();

	/// return true if the current activated VBHard is in BGRA mode. nlassert(_Lockdone)
	bool			isBRGA() const;

	/** Swap to the next VBHard. This allow some parralelism, since CPU fill one VBHard while the other is rendered
	 *	NB: no-op if the vertex stream manager falls down to the VBSoft solution.
	 */
	void			swapVBHard();

	// @}

// ********************
private:
	uint			_NumVB;

	NLMISC::CRefPtr<IDriver>			_Driver;
	std::vector<CVertexBuffer>			_VB;
	CVertexBuffer                       _VBVolatile; // volatile vertex buffer, used if supported by driver
	CVertexBufferReadWrite				_VBA;
	bool			_InitOk;
	bool			_SupportVolatileVB;
	bool			_LockDone;
	uint			_VertexFormat;
	uint			_VertexSize;
	uint			_MaxVertices;

	uint			_CurentVB;

};


} // NL3D


#endif // NL_VERTEX_STREAM_MANAGER_H

/* End of vertex_stream_manager.h */
