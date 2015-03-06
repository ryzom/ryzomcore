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

#ifndef CL_SCRRENSHOT_ISLANDS_H
#define CL_SCRRENSHOT_ISLANDS_H

// Misc
#include "nel/misc/singleton.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/rgba.h"
#include "nel/misc/bitmap.h"

#include <stdio.h>

namespace NL3D
{
	class UScene;
	class ULandscape;
}

namespace R2
{
	class CProximityZone;

typedef uint16 TBufferEntry;
typedef std::vector<TBufferEntry> TBuffer;
typedef std::vector<uint32> TOffsetsVector;

//-----------------------------------------------------------------------------
// class CScreenshotIslands
//-----------------------------------------------------------------------------
struct CIslandData
{
	NLMISC::CVector2f EntryPoint;
	NLMISC::CVector2f Max;
	NLMISC::CVector2f Min;

	CIslandData() 
	{
		EntryPoint = NLMISC::CVector2f(0, 0);
		Max = NLMISC::CVector2f(0, 0);
		Min = NLMISC::CVector2f(0, 0);
	}

	CIslandData(float x, float y) 
	{
		EntryPoint = NLMISC::CVector2f(x, y);
		Max = NLMISC::CVector2f(0, 0);
		Min = NLMISC::CVector2f(0, 0);
	}
};

struct CContinentData
{
	std::string			SmallBank;
	std::string			FarBank;
	std::string			IGFile;
	std::string			CoarseMeshMap;
	NLMISC::CVector2f	ZoneMin;
	NLMISC::CVector2f	ZoneMax;
	std::list< std::string >	Islands;
	NLMISC::CRGBA		Ambiant;
	NLMISC::CRGBA		Diffuse;

	CContinentData() {}
};

typedef std::map< const std::string, CProximityZone> TIslandsData;
typedef std::map< NLMISC::CVector2f, bool >			TIslandsMap;
typedef std::map< const std::string, CContinentData >	TContinentsData;
typedef std::map< std::string, std::list< NLMISC::CVector2f > > TIslandsBordersMap;

class CScreenshotIslands : public NLMISC::CSingleton<CScreenshotIslands>
{

public:

	CScreenshotIslands();

	void buildScreenshots();

private:

	void init();

	void loadIslands();

	void buildIslandsTextures();

	void getBuffer(NL3D::UScene * scene, NL3D::ULandscape * landscape, NLMISC::CBitmap &btm);

	bool getPosFromZoneName(const std::string &name, NLMISC::CVector2f &dest);
	
	void writeProximityBufferToTgaFile(const std::string& fileName,const TBuffer& buffer,
		uint32 scanWidth,uint32 scanHeight);

	void processProximityBuffer(TBuffer& inputBuffer, uint32 lineLength, TBuffer& resultBuffer);

	void buildBackTextureHLS(const std::string & islandName, const NLMISC::CBitmap & islandBitmap);

	void searchIslandsBorders();

	void attenuateIslandBorders(const std::string & islandName, NLMISC::CBitmap & islandBitmap, const CProximityZone & islandData);

	TIslandsData				_IslandsData;
	TIslandsMap					_IslandsMap;
	TContinentsData				_ContinentsData;
	std::list< std::string >	_SeasonSuffixes;
	int							_MeterPixelSize;
	std::string					_OutDirectory;
	std::list< std::string >	_TempFileNames;
	TIslandsBordersMap			_BorderIslands;
	bool						_Vegetation;
	bool						_InverseZTest;

	NLMISC::CRGBA				_BackColor;
	NLMISC::CBitmap				_BackBitmap;
	std::string					_CompleteIslandsFile;
};






class CProximityZone
{
public:
	typedef std::vector<uint32> TOffsets;

	// ctor
	// scanWidth and scanHeight define the dimentions of the buffer that this zone is being extracted from
	// xOffset and yOffset are for re-mapping coordinates from buffer-relative to absolute ryzom world coordinates
	CProximityZone(uint32 scanWidth=0,uint32 scanHeight=0,sint32 xOffset=0, sint32 yOffset=0);

	// add an 'accessible position' to a zone (offset is y*scanWidth+x)
	bool add(uint32 offset);

	// zone dimention accessors (note that zone dimentions line up with 160x160 world zones)
	// note that this means that the zone bounds may extend outside the scan area
	uint32 getZoneWidth() const;
	uint32 getZoneHeight() const;
	sint32 getZoneXMin() const;
	sint32 getZoneYMin() const;
	uint32 getZoneXMax() const;
	uint32 getZoneYMax() const;

	// read accessors for the bounding limits that define the area occupied by the accessible points
	uint32 getBoundXMin() const;
	uint32 getBoundYMin() const;
	uint32 getBoundXMax() const;
	uint32 getBoundYMax() const;

	// read accessor for the _Offsets vector
	// this is a vector of offsets into the scan area. It needs to be remapped to zone offsets
	// via the remapOffset() routine in order to be used to index into a zone buffer
	const TOffsets& getOffsets() const;

	// remap a scan buffer offset to a zone offset by decomposing into x and y parts and
	// subtracting getZoneXMin() and getZoneYMin()
	uint32 remapOffset(uint32 bufferOffset) const;

private:
	// parameters setup at construction time, giving info on the context that we're in
	uint32 _ScanWidth;
	uint32 _ScanHeight;
	sint32 _XOffset;
	sint32 _YOffset;
	uint32 _MaxOffset;

	// the vector of points that are part of this zone
	TOffsets _Offsets;

	// the min and max coords of the points that are part of this zone
	uint32 _XMax;
	uint32 _XMin;
	uint32 _YMax;
	uint32 _YMin;

	// border add to bouding zone (pixels number)
	int _BorderPixels;
};

//-------------------------------------------------------------------------------------------------
// class CProximityMapBuffer
//-------------------------------------------------------------------------------------------------

class CProximityMapBuffer
{
public:
	typedef	std::vector<CProximityZone> TZones;

	// load a cwmap2 file and setup this object from its contents
	// the 'name' parameter is the full file name of the file to load with path and extension
	void load(const std::string& name);

	// scan the buffer to generate the set of non-connecting zones that it contains
	void calculateZones(TZones& zones);

	// generate the proximity map for a given zone
	void generateZoneProximityMap(const CProximityZone& zone,TBuffer& zoneBuffer);

	// read accessors...
	const TBuffer& getBuffer() const;
	uint32 getScanHeight() const;
	uint32 getScanWidth() const;

	// buffer coordinate to world coordinate offsets...
	sint32 _XOffset;
	sint32 _YOffset;

private:
	// private routine used by generateZoneProximityMap() to setup the zoneBuffer with the accessible points set
	void _prepareBufferForZoneProximityMap(const CProximityZone& zone,TBuffer& zoneBuffer,TOffsetsVector& accessiblePoints);

private:
	// the width and heilght of the scan zone (ie the dimentions of the buffer)
	uint32 _ScanWidth;
	uint32 _ScanHeight;

	// vector representing 2d array of points [_ScanHeight][_ScanWidth]
	TBuffer _Buffer;
};


}

#endif