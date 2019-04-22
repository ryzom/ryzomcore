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

#include "nel/3d/point_light_named_array.h"
#include "nel/3d/scene.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


// ***************************************************************************
CPointLightNamedArray::CPointLightNamedArray()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
}


// ***************************************************************************
struct	CPointLightNamedSort
{
	const CPointLightNamed	*PointLight;
	uint					SrcId;

	bool	operator<(const CPointLightNamedSort &b) const
	{
		if (PointLight->AnimatedLight < b.PointLight->AnimatedLight)
			return true;
		if (PointLight->AnimatedLight > b.PointLight->AnimatedLight)
			return false;
		return (PointLight->LightGroup < b.PointLight->LightGroup);
	}
};


// ***************************************************************************
void			CPointLightNamedArray::clear()
{
	_PointLights.clear();
	_PointLightGroupMap.clear();
}


// ***************************************************************************
void			CPointLightNamedArray::build(const std::vector<CPointLightNamed> &pointLights, std::vector<uint> &indexRemap)
{
	uint	i;

	// sort by name.
	//----------
	// Fill Sort array
	vector<CPointLightNamedSort>	pointLightSorts;
	pointLightSorts.resize(pointLights.size());
	for(i=0; i<pointLightSorts.size(); i++)
	{
		pointLightSorts[i].PointLight= &pointLights[i];
		pointLightSorts[i].SrcId= i;
	}
	// sort
	sort(pointLightSorts.begin(), pointLightSorts.end());
	// Copy data, and Fill indexRemap array
	_PointLights.resize(pointLights.size());
	indexRemap.resize(pointLights.size());
	for(i=0; i<pointLightSorts.size(); i++)
	{
		// Copy yhe PointLight to its new destination
		_PointLights[i]= *pointLightSorts[i].PointLight;
		// set the new index at the old position.
		indexRemap[pointLightSorts[i].SrcId]= i;
	}

	// Regroup.
	// ---------
	_PointLightGroupMap.clear();
	if (!_PointLights.empty())
	{
		bool	first= true;
		string	precName;
		uint precGroup= 0;
		// for all sorted pointLights
		uint i;
		for(i=0;i<_PointLights.size();i++)
		{
			const	std::string &curName = _PointLights[i].AnimatedLight;
			const	uint curGroup = _PointLights[i].LightGroup;
			if ( first || (precName!=curName) || (precGroup != curGroup) )
			{
				// End last group
				if(first)
					first= false;
				else
					_PointLightGroupMap.back ().EndId= i;

				// Start new group
				_PointLightGroupMap.push_back (CPointLightGroup ());
				_PointLightGroupMap.back ().StartId= i;
				_PointLightGroupMap.back ().AnimationLight = curName;
				_PointLightGroupMap.back ().LightGroup = curGroup;
				precName = curName;
				precGroup = curGroup;
			}
		}
		// End last group.
		_PointLightGroupMap.back ().EndId= i;
	}
}

// ***************************************************************************
void			CPointLightNamedArray::setPointLightFactor(const CScene &scene)
{
	// Search in the map.
	const uint count = (uint)_PointLightGroupMap.size ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Ref
		CPointLightGroup &lightGroup = _PointLightGroupMap[i];

		// Get the factor
		CRGBA factorAnimated=	 scene.getAnimatedLightFactor (lightGroup.AnimationLightIndex, lightGroup.LightGroup);
		CRGBA factorNotAnimated= scene.getLightmapGroupColor (lightGroup.LightGroup);;

		// Found the group. what entries in the array?
		uint	startId= lightGroup.StartId;
		const uint	endId= lightGroup.EndId;
		nlassert(endId<=_PointLights.size());

		// for all entries, setLightFactor
		for(uint i=startId;i<endId;i++)
		{
			_PointLights[i].setLightFactor (factorAnimated, factorNotAnimated);
		}
	}
}


// ***************************************************************************
void			CPointLightNamedArray::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	sint ver = f.serialVersion(1);

	f.serialCont(_PointLights);

	if (ver == 0)
	{
		std::map<string, CPointLightGroupV0> oldMap;
		f.serialCont(oldMap);
	}
	else
	{
		f.serialCont(_PointLightGroupMap);
	}
}


// ***************************************************************************
void			CPointLightNamedArray::initAnimatedLightIndex (const CScene &scene)
{
	// Search in the map.
	const uint count = (uint)_PointLightGroupMap.size ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Ref
		CPointLightGroup &lightGroup = _PointLightGroupMap[i];
		lightGroup.AnimationLightIndex = scene.getAnimatedLightNameToIndex (lightGroup.AnimationLight);
	}
}

} // NL3D
