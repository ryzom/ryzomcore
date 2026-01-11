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

#ifndef NL_WATER_POOL_MANAGER_H
#define NL_WATER_POOL_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"

#include "nel/3d/water_shape.h"

#include <map>
#include <vector>
#include <string>

namespace NL3D {

class CWaterHeightMap;
class IDriver;

/**
 * This class helps managing various waters pools
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CWaterPoolManager
{
public:
	/// this struct is used to specify a water pool parameter's
	struct CWaterHeightMapBuild
	{
		uint32		ID;
		uint32		Size;
		std::string Name;
		float		Damping;
		float		FilterWeight;
		float		UnitSize;
		bool		WavesEnabled;
		float		WaveIntensity;
		uint32		WaveRadius;
		float		WavePeriod;
		bool		BorderWaves;
		CWaterHeightMapBuild() : ID(0), Size(256), Damping(0.99f), FilterWeight(3), UnitSize(0.30f), WavesEnabled(false), WaveIntensity(1.5), WaveRadius(3), WavePeriod(0.05f), BorderWaves(true) {}
	};
	/// create a water pool with the given id and the given parameters. If the pool existed before, its parameter are reset
	CWaterHeightMap				*createWaterPool(const CWaterHeightMapBuild &params = CWaterHeightMapBuild());

	/// Get a water pool by its ID. If the ID doesn't exist, a new pool is created with default parameters
	CWaterHeightMap				&getPoolByID(uint32 ID);

	/// test whether a pool of the given ID exists
	bool						hasPool(uint32 ID) const;

	/// remove the pool of the given ID
	void						removePool(uint32 ID);

	/// Get the number of pools
	uint						getNumPools() const;

	/// get the id of the i-th pool (O(n) lookup)
	uint						getPoolID(uint i) const;




	/// delete all heightmaps
	void reset();

	// dtor
	~CWaterPoolManager() { reset(); }

	/** Set a blend factor for all pool (more precisely, all models based on a water shape) that have a blend texture for their envmap (to have cycle between night and day for example)
	  * NB : once this is called, textures are not released from memory because subsequent blends are expected to happend
	  *      when transition has finished, one should call releaseBlendTexture to eventually release textures from system memory
	  * \param factor The blend factor which range from 0 to 1
	  */
	void setBlendFactor(IDriver *drv, float factor);
	/** release blend textures from memory
	  * \see setblendFactor
	  */
	void releaseBlendTextures();


	/// serial the pools data's
	void serial(NLMISC::IStream &f);

private:
	friend class CWaterShape;
	friend CWaterPoolManager				  &GetWaterPoolManager();
	CWaterPoolManager() {}	// private ctor needed to use the singleton pattern
	typedef std::map<uint32, CWaterHeightMap *> TPoolMap;
	TPoolMap _PoolMap;

	/// register a water height map. The water height map will be notified when a setBlend is applied
	void	registerWaterShape(CWaterShape *shape);
	void	unRegisterWaterShape(CWaterShape *shape);
	bool    isWaterShapeObserver(const CWaterShape *shape) const;
	typedef std::vector<CWaterShape *> TWaterShapeVect;
	TWaterShapeVect _WaterShapes;
};


// get the only water pool manager (caution : with several dll, there may be duplication however,
// if NL3D is linked as a static lib with several of them, so you may need to pass the address of the manager being used to other dlls)
CWaterPoolManager &GetWaterPoolManager();


} // NL3D


#endif // NL_WATER_POOL_MANAGER_H

/* End of water_pool_manager.h */
