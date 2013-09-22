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



// pch
#include "stdpch.h"

//
#include "light_ig_loader.h"


// misc
#include <nel/misc/vector_2f.h>
#include <nel/misc/object_vector.h>
#include <nel/misc/rgba.h>
#include <nel/misc/plane.h>
#include <nel/misc/aabbox.h>


using namespace std;
using namespace NLMISC;




//
#define	SERIAL_DUMMY(type)	{ type	dummy; _File.serial(dummy); }

struct	CCellCorner
{
	enum { NumLightPerCorner = 2 };

	uint8	SunContribution;
	uint8	Light[NumLightPerCorner];
	uint8	LocalAmbientId;

	void		serial(NLMISC::IStream &f)
	{
		uint	ver= f.serialVersion(1);
		nlassert(NumLightPerCorner==2);

		if(ver>=1)
			f.serial(LocalAmbientId);
		f.serial(SunContribution);
		f.serial(Light[0]);
		f.serial(Light[1]);
	}
};

class CSurfaceLightGrid
{
public:
	NLMISC::CVector2f					Origin;
	uint32								Width;
	uint32								Height;
	NLMISC::CObjectVector<CCellCorner>	Cells;

public:

	void		serial(NLMISC::IStream &f)
	{
		(void)f.serialVersion(0);
		f.serial(Origin);
		f.serial(Width);
		f.serial(Height);
		f.serial(Cells);
	}
};

class CRetrieverLightGrid
{
public:
	//NLMISC::CObjectVector<CSurfaceLightGrid>	Grids;

public:

	void		serial(NLMISC::IStream &_File)
	{
		(void)_File.serialVersion(0);
		SERIAL_DUMMY(NLMISC::CObjectVector<CSurfaceLightGrid>);
	}
};





/*
 * Constructor
 */
CLightIGLoader::CLightIGLoader()
{
}


// Load IG
void	CLightIGLoader::loadIG(const string &filename)
{
	_File.open(CPath::lookup(filename));


	// Serial a header
	_File.serialCheck (NELID("TPRG');

	// Serial a version number
	sint version = _File.serialVersion (5);


	// _RealTimeSunContribution
	if (version >= 5)
	{
		SERIAL_DUMMY(bool);		// _RealTimeSunContribution
	}


	// Serial the IGSurfaceLight
	if (version >= 4)
	{
		sint	ver = _File.serialVersion(1);

		SERIAL_DUMMY(float);	// _CellSize
		SERIAL_DUMMY(float);	// _OOCellSize

		if(ver<1)
		{
			std::map<std::string, CRetrieverLightGrid>		_RetrieverGridMap;
			_File.serialCont(_RetrieverGridMap);
		}
		else
		{
			std::map<uint32, CRetrieverLightGrid>			_RetrieverGridMap;
			_File.serialCont(_RetrieverGridMap);
		}
	}


	// Serial the PointLights info
	if (version >= 3)
	{
		sint ver = _File.serialVersion(1);

		// _PointLights
		uint32	num;
		_File.serial(num);
		for (; num>0; --num)
		{
			sint	ver = _File.serialVersion(1);

			// Serialize parent.
			sint	verp= _File.serialVersion(2);

			if (verp>=2)
			{
				SERIAL_DUMMY(bool);
			}
				
			if (verp>=1)
			{
				SERIAL_DUMMY(sint32);	// enum _Type
				SERIAL_DUMMY(CVector);	// _SpotDirection
				SERIAL_DUMMY(float);	// _SpotAngleBegin
				SERIAL_DUMMY(float);	// _SpotAngleEnd
			}

			SERIAL_DUMMY(CVector);		// _Position
			SERIAL_DUMMY(CRGBA);		// _Ambient
			SERIAL_DUMMY(CRGBA);		// _Diffuse
			SERIAL_DUMMY(CRGBA);		// _Specular
			SERIAL_DUMMY(float);		// _AttenuationBegin
			SERIAL_DUMMY(float);		// _AttenuationEnd

			// Serialize my data
			SERIAL_DUMMY(string);		// AnimatedLight
			SERIAL_DUMMY(CRGBA);		// _DefaultAmbient
			SERIAL_DUMMY(CRGBA);		// _DefaultDiffuse
			SERIAL_DUMMY(CRGBA);		// _DefaultSpecular

			if (ver>=1)
				SERIAL_DUMMY(uint32);	// LightGroup
		}

		if (ver == 0)
		{
			uint32	num;
			_File.serial(num);
			for (; num>0; --num)
			{
				SERIAL_DUMMY(uint32);	// StartId
				SERIAL_DUMMY(uint32);	// EndId
			}
		}
		else
		{
			// _PointLightGroupMap
			uint32	num;
			_File.serial(num);
			for (; num>0; --num)
			{
				_File.serialVersion (0);
				SERIAL_DUMMY(string);		// AnimationLight
				SERIAL_DUMMY(uint32);		// LightGroup
				SERIAL_DUMMY(uint32);		// StartId
				SERIAL_DUMMY(uint32);		// EndId
			}
		}
	}


	if (version >= 2)
		SERIAL_DUMMY(CVector);			// _GlobalPos







	if (version >= 1)
	{
		uint32	numClusters;
		uint32	num;

		// f.serialCont (_ClusterInfos);
		_File.serial(numClusters);
		for (num=numClusters; num>0; --num)
		{
			sint version = _File.serialVersion (3);

			if (version >= 1)
				SERIAL_DUMMY(string);	// Name

			uint32	n;

			// f.serialCont (_LocalVolume);
			_File.serial(n);
			for (; n>0; --n)
				SERIAL_DUMMY(CPlane);	// _LocalVolume

			SERIAL_DUMMY(CAABBox);		// _LocalBBox
			SERIAL_DUMMY(bool)	;		// FatherVisible
			SERIAL_DUMMY(bool);			// VisibleFromFather

			if (version >= 2)
			{
				SERIAL_DUMMY(string);	// soundGroup
				SERIAL_DUMMY(string);	// envFxName
			}

			if (version >= 3)
			{
				SERIAL_DUMMY(bool);		// AudibleFromFather
				SERIAL_DUMMY(bool);		// FatherAudible
			}
		}

		// f.serialCont (_Portals);
		_File.serial(num);
		for (; num>0; --num)
		{
			int version = _File.serialVersion (1);

			uint32	n;

			//f.serialCont (_LocalPoly);
			_File.serial(n);
			for (; n>0; --n)
				SERIAL_DUMMY(CVector);

			SERIAL_DUMMY(string);		// _Name

			if (version >= 1)
			{
				SERIAL_DUMMY(string);		// occName;
				SERIAL_DUMMY(string);		// occName;
			}
		}

		// Links
		uint32 i, j;
		for (i = 0; i < numClusters; ++i)
		{
			uint32 nNbPortals;
			_File.serial (nNbPortals);

			for (j = 0; j < nNbPortals; ++j)
				SERIAL_DUMMY(sint32);	// nPortalNb
		}
	}

	// Serial the array
	// f.serialCont (_InstancesInfos);
	uint32	num;
	_File.serial(num);
	for (; num>0; --num)
	{
		/*
		Version 7:
			- Visible
		Version 6:
			- DontCastShadowForExterior
		Version 5:
			- DontCastShadowForInterior
		Version 4:
			- LocalAmbientId.
		Version 3:
			- StaticLight.
		Version 2:
			- gameDev data.
		Version 1:
			- Clusters
		*/
		// Serial a version number
		sint version = _File.serialVersion (7);

		// Visible
		if (version >= 7)
			SERIAL_DUMMY(bool);
		
		// DontCastShadowForExterior
		if (version >= 6)
			SERIAL_DUMMY(bool);

		// DontCastShadowForInterior
		if (version >= 5)
			SERIAL_DUMMY(bool);


		// Serial the LocalAmbientId.
		if (version >= 4)
			SERIAL_DUMMY(uint8);	// LocalAmbientId)

		// Serial the StaticLight
		if (version >= 3)
		{

			SERIAL_DUMMY(bool);		// AvoidStaticLightPreCompute
			SERIAL_DUMMY(bool);		// DontCastShadow
			SERIAL_DUMMY(bool);		// StaticLightEnabled
			SERIAL_DUMMY(uint8);	// SunContribution

			SERIAL_DUMMY(uint8);	// Light[0]
			SERIAL_DUMMY(uint8);	// Light[1]
		}

		// Serial the gamedev data
		_InstanceNames.resize(_InstanceNames.size()+1);
		if (version >= 2)
		{
			_File.serial(_InstanceNames.back());
			SERIAL_DUMMY(bool);		// DontAddToScene
		}

		// Serial the clusters
		if (version >= 1)
		{
			// f.serialCont (Clusters);
			uint32	num;
			_File.serial(num);
			for (; num>0; --num)
				SERIAL_DUMMY(uint32);
		}

		// Serial the name
		_ShapeNames.resize(_ShapeNames.size()+1);
		_File.serial(_ShapeNames.back());
		//serialDummy<string>();		// Name

		// Serial the position vector
		_ShapePos.resize(_ShapePos.size()+1);
		_File.serial(_ShapePos.back());
		//serialDummy<CVector>();		// Pos

		// Serial the rotation vector
		_ShapeRots.resize(_ShapeRots.size()+1);
		_File.serial(_ShapeRots.back());
		//serialDummy<CQuat>();		// Rot

		// Serial the scale vector
		_ShapeScales.resize(_ShapeScales.size()+1);
		_File.serial(_ShapeScales.back());
		//serialDummy<CVector>();		// Scale

		// Serial the parent location in the vector (-1 if no parent)
		SERIAL_DUMMY(sint32);		// nParent
	}
}
