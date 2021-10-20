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

#ifndef NL_POINT_LIGHT_NAMED_ARRAY_H
#define NL_POINT_LIGHT_NAMED_ARRAY_H

#include "nel/misc/types_nl.h"
#include "nel/3d/point_light_named.h"


namespace NL3D {

class CScene;


// ***************************************************************************
/**
 * A class used bi CInstanceGroup and CZone. Just a list of pointLights, grouped by name.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightNamedArray
{
public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/// Constructor
	CPointLightNamedArray();

	/// clear All: pointLights and map.
	void			clear();

	/** sort pointLights and make Groups by name for setFactor().
	 *	indexRemap gets news Ids for pointLights.
	 *	eg: old PointLight nb 0 is now at indexRemap[0].
	 */
	void			build(const std::vector<CPointLightNamed> &pointLights, std::vector<uint> &indexRemap);


	/// get the pointLights sorted (const version).
	const std::vector<CPointLightNamed>		&getPointLights() const {return _PointLights;}

	/// get the pointLights sorted (mutable version). Be sure what you do!
	std::vector<CPointLightNamed>		&getPointLights() {return _PointLights;}

	/// update the Light factor for all pointLights (animated and unanimated one)
	void			setPointLightFactor(const CScene &scene);

	// serial
	void			serial(NLMISC::IStream &f);

	/* Init lighting information
	 * Scan lights used by the array, and for each, bind the array to an
	 * animated light index from the scene. This index will be used at runtime to
	 * get quickly a light factor.
	 */
	void			initAnimatedLightIndex (const CScene &scene);

private:

	/// List of pointLight
	std::vector<CPointLightNamed>	_PointLights;

	/// LightGroupName mgt.
	struct	CPointLightGroup
	{
		std::string		AnimationLight;
		sint32			AnimationLightIndex;
		uint32			LightGroup;
		uint32			StartId;	// start in the array.
		uint32			EndId;		// EndId-StartId==number of pointlights with this name.

		CPointLightGroup ()
		{
			AnimationLightIndex = -1;
		}

		void	serial(NLMISC::IStream &f)
		{
			f.serialVersion (0);
			f.serial(AnimationLight);
			f.serial(LightGroup);
			f.serial(StartId);
			f.serial(EndId);

			// Reset the animation light index in the scene
			AnimationLightIndex = -1;
		}
	};

	/// Deprecated serials
	struct	CPointLightGroupV0
	{
		uint32			StartId;	// start in the array.
		uint32			EndId;		// EndId-StartId==number of pointlights with this name.

		void	serial(NLMISC::IStream &f)
		{
			f.serial(StartId, EndId);
		}
	};

	typedef std::vector<CPointLightGroup>				TPLGVec;
	/// Info for LightGroupName and setPointLightFactor
	TPLGVec							_PointLightGroupMap;

};


} // NL3D


#endif // NL_POINT_LIGHT_NAMED_ARRAY_H

/* End of point_light_named_array.h */
