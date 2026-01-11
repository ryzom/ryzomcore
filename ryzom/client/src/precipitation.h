// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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





#ifndef CL_PRECIPITATION_H
#define CL_PRECIPITATION_H

#include "precipitation_clip_grid.h"
#include "nel/misc/vector_2f.h"
#include "nel/3d/u_particle_system_instance.h"
#include <vector>
#include <map>

namespace NL3D
{
	class UScene;
	class UDriver;
}
namespace NLPACS
{
	class UGlobalRetriever;
}
namespace NLMISC
{
	class CMatrix;
}

///////////////////////////////////////////////////////////
// class to describe precipitations                      //
// Several instances of the same shared FX are displayed //
///////////////////////////////////////////////////////////
struct CPrecipitationDesc
{
	std::string FxName;           // name of the FX used for precipitations
	uint		GridSize;         // Size of the grid used to display FXs.
	bool        UseBBoxSize;      // The size of each block of the grid is taken from the x & y coordinates of the bbox.
	                              // When set to false, 'Size' is used instead
	float		Size;             // Size of the blocks, it is used only if UseBBoxSize is set to false.
	bool		ReleasableModel;  // Models are allocated only if particles are needed. This avoid useless models traversal during the render when thare are no precipitations.
	CPrecipitationDesc()
	{
		GridSize        = 7;
		UseBBoxSize     = true;
		Size            = 0;
		ReleasableModel = true;
	}
};


///////////////////////////////////
// class to manage Precipitations //
///////////////////////////////////

class CPrecipitation
{
public:
	// ctor
	CPrecipitation();
	// Init the precipitations, and load associated fx
	void init(const CPrecipitationDesc &desc);
	// Release datas. Should call this if the scene is still present.
	void release();
	// Update precipitations depending on the camera position & orientation
	void update(const NLMISC::CMatrix &camMat, NLPACS::UGlobalRetriever *retriever);
	/** Set strenght for rain. Should go from 0 to 1 (clamped).
	  * 0 stops the rains
	  */
	void setStrenght(float strenght);
	/// After strenght has been set to 0, it may need some time before there are no more particle in the fx. This allow to now if the fx i still running
	bool isRunning() const;

	// For clip grid : Draw the clip grid associated with taht precipitation
	void drawClipGrid(NL3D::UDriver &drv) const;

	// get description of precipitation
	const CPrecipitationDesc &getDesc() const { return _Desc; }

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
private:
	typedef std::map<NLMISC::CVector2f, CPrecipitationClipGrid> TClipGridMap;
private:
	CPrecipitationClipGrid						  *_ClipGrid;
	std::vector<NL3D::UParticleSystemInstance>		_Blocks;
	float										   _Strenght;
	float										   _TimeOut;
	float										   _XSize;
	float										   _YSize;
	sint										   _OldX;
	sint										   _OldY;
	bool                                           _Touched; // when set to true, force the next grid update
	CPrecipitationDesc							   _Desc;
	// the precipitation clip girds, sorted by size
	static TClipGridMap				               _PrecipitationClipGripMap;
private:
	// helper to view the vector as a 2D tab
	NL3D::UParticleSystemInstance getBlock(uint x, uint y)
	{
		nlassert(x < _Desc.GridSize && y < _Desc.GridSize);
		return _Blocks[x + y * _Desc.GridSize];
	}
	// allocate the FXs
	void allocateFXs();
	// deallocate the FXs
	void deallocateFXs();
	//
	bool areFXsAllocated() const { return !_Blocks.empty(); }
	// Force the setup of strenght with no check for previous value
	void forceSetStrenght(float strenght);
};




#endif

