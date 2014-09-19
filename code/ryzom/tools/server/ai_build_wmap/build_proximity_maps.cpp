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


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// Nel misc
#include "nel/misc/types_nl.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/sstring.h"
#include "nel/misc/file.h"

// Game share
#include "game_share/bmp4image.h"

// AI share
#include "ai_share/world_map.h"


//-------------------------------------------------------------------------------------------------
// using namespaces...
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace RYAI_MAP_CRUNCH;


//-------------------------------------------------------------------------------------------------
// External variables...
//-------------------------------------------------------------------------------------------------

extern string OutputPath;


//-------------------------------------------------------------------------------------------------
// Local type definitions
//-------------------------------------------------------------------------------------------------

typedef uint16 TBufferEntry;
typedef std::vector<TBufferEntry> TBuffer;
typedef std::vector<uint32> TOffsetsVector;


//-------------------------------------------------------------------------------------------------
// class CProximityZone
//-------------------------------------------------------------------------------------------------

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

	// read accessors for the ryzom world coordinate offsets
	sint32 getXOffset() const;
	sint32 getYOffset() const;

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

private:
	// private routine used by generateZoneProximityMap() to setup the zoneBuffer with the accessible points set
	void _prepareBufferForZoneProximityMap(const CProximityZone& zone,TBuffer& zoneBuffer,TOffsetsVector& accessiblePoints);

private:
	// the width and heilght of the scan zone (ie the dimentions of the buffer)
	uint32 _ScanWidth;
	uint32 _ScanHeight;

	// vector representing 2d array of points [_ScanHeight][_ScanWidth]
	TBuffer _Buffer;

	// buffer coordinate to world coordinate offsets...
	sint32 _XOffset;
	sint32 _YOffset;
};


//-------------------------------------------------------------------------------------------------
// Handy utility routines
//-------------------------------------------------------------------------------------------------

static void writeProximityBufferToTgaFile(const std::string& fileName,const TBuffer& buffer,uint32 scanWidth,uint32 scanHeight)
{
	uint	imageWidth = (scanWidth+15)&~15;
	uint	imageHeight = (scanHeight);

	CTGAImageGrey tgaImage;
	tgaImage.setup((uint16)imageWidth, (uint16)imageHeight, fileName, 0, 0);
	for (uint32 y=0;y<scanHeight;++y)
	{
		for (uint32 x=0; x<scanWidth; ++x)
		{
			uint32 value= buffer[y*scanWidth+x];
			tgaImage.set(x, (value>255*5)?255:value/5);
		}
		tgaImage.writeLine();
	}
}

static void processProximityBuffer(const TBuffer& inputBuffer, uint32 lineLength, TBuffer& resultBuffer)
{
	// a couple of constants to control the range over which our degressive filter is to be applied
	const uint32 smallValue= 2*5;
	const uint32 bigValue= 15*5;

	// determine numer of lines in the buffer...
	uint32 numLines= (uint32)inputBuffer.size()/ lineLength;

	// clear out the result buffer and reset all values to 5*255, remembering that this is the correct value for the image edges
	resultBuffer.clear();
	resultBuffer.resize(inputBuffer.size(),(TBufferEntry)5*255);

	for (uint32 y=1;y<numLines-1;++y)
	{
		uint32 lineOffset= y* lineLength;
		for (uint32 x=1;x<lineLength-1;++x)
		{
			uint32 offset= lineOffset+x;

			// apply a simple square filter
			uint32 value=
				(((uint32)inputBuffer[offset]+
				((uint32)inputBuffer[offset+1]+inputBuffer[offset-1]+inputBuffer[offset+lineLength]+inputBuffer[offset-lineLength])/2 +
				((uint32)inputBuffer[offset+lineLength+1]+inputBuffer[offset+lineLength-1]+inputBuffer[offset-lineLength+1]+inputBuffer[offset-lineLength-1])/4)/4);

			// apply a clip and cosine function
			if (value<smallValue) value=0;
			else if (value>bigValue) value=5*255;
			else value= (uint32)(((1.0-cos(3.14159265359*(float(value-smallValue)/(float)(bigValue-smallValue))))/2.0)*float(5*255));

			// store the value into the result buffer
			resultBuffer[offset]= (TBufferEntry)value;
		}
	}
}


//-------------------------------------------------------------------------------------------------
// methods CProximityZone
//-------------------------------------------------------------------------------------------------

CProximityZone::CProximityZone(uint32 scanWidth,uint32 scanHeight,sint32 xOffset, sint32 yOffset)
{
	_ScanWidth	= scanWidth;
	_ScanHeight	= scanHeight;
	_XOffset	= xOffset;
	_YOffset	= yOffset;

	_MaxOffset	= scanWidth * scanHeight -1;

	_XMin = ~0u;
	_YMin = ~0u;
	_XMax = 0;
	_YMax = 0;
}

bool CProximityZone::add(uint32 offset)
{
	// make sure the requested point is in the zone
	if (offset>_MaxOffset)
		return false;

	// calculate the x and y coordinates of the point
	uint32 y= offset/ _ScanWidth;
	uint32 x= offset% _ScanWidth;

	// update the bounding coordinates for this zone
	if (x<_XMin) _XMin= x;
	if (x>_XMax) _XMax= x;
	if (y<_YMin) _YMin= y;
	if (y>_YMax) _YMax= y;
			 
	// add the point to the vector of points
	_Offsets.push_back(offset);
	return true;
}

const CProximityZone::TOffsets& CProximityZone::getOffsets() const
{
	return _Offsets;
}

uint32 CProximityZone::getZoneWidth() const
{
	return getZoneXMax()- getZoneXMin() +1;
}

uint32 CProximityZone::getZoneHeight() const
{
	return getZoneYMax()- getZoneYMin() +1;
}

sint32 CProximityZone::getZoneXMin() const
{
	return (_XMin+_XOffset)/160*160-_XOffset;
}

sint32 CProximityZone::getZoneYMin() const
{
	return (_YMin+_YOffset)/160*160-_YOffset;
}

uint32 CProximityZone::getZoneXMax() const
{
	return (((_XMax+_XOffset)/160+1)*160)-_XOffset-1;
}

uint32 CProximityZone::getZoneYMax() const
{
	return (((_YMax+_YOffset)/160+1)*160)-_YOffset-1;
}

sint32 CProximityZone::getXOffset() const
{
	return _XOffset+getZoneXMin();
}

sint32 CProximityZone::getYOffset() const
{
	return _YOffset+getZoneYMin();
}

uint32 CProximityZone::getBoundXMin() const
{
	return _XMin-getZoneXMin();
}

uint32 CProximityZone::getBoundYMin() const
{
	return _YMin-getZoneYMin();
}

uint32 CProximityZone::getBoundXMax() const
{
	return _XMax-getZoneXMin();
}

uint32 CProximityZone::getBoundYMax() const
{
	return _YMax-getZoneYMin();
}

uint32 CProximityZone::remapOffset(uint32 bufferOffset) const
{
	// decompose input coordinates into x and y parts
	uint32 bufferX= bufferOffset% _ScanWidth;
	uint32 bufferY= bufferOffset/ _ScanWidth;

	// remap the offset from a _Buffer-relative offset to a zone-relative offset
	return bufferX-getZoneXMin()+ (bufferY-getZoneYMin())*getZoneWidth();
}


//-------------------------------------------------------------------------------------------------
// methods CProximityMapBuffer
//-------------------------------------------------------------------------------------------------

void CProximityMapBuffer::load(const std::string& name)
{
	// load the AI collision map file
	CWorldMap worldMap;
	CIFile	f(name);
	f.serial(worldMap);

	// lookup the map bounds
	CMapPosition	min, max;
	worldMap.getBounds(min, max);

	// calculate a handful of constants relating to the bounds of the image...
	_ScanWidth = max.x()-min.x();
	_ScanHeight = max.y()-min.y();
	_XOffset= min.x();
	_YOffset= max.y();

	// redimension buffer to correct size
	_Buffer.resize(_ScanWidth*_ScanHeight);

	// setup a position variable to mark the start point of each line
	CMapPosition	scanpos(min.x(),min.y());

	// iterate over the scan area looking for points that are accessible
	for (uint32 y=0; y<_ScanHeight; ++y, scanpos = scanpos.getStepN())
	{
		CMapPosition pos(scanpos);

		// scan a line of the map
		for (uint32 x=0; x<_ScanWidth; ++x, pos = pos.getStepE())
		{
			bool isAccessible= false;
			// if the cell pointer is NULL it means that the 16x16 cell in question is inaccessible
			if (worldMap.getRootCellCst(pos) != NULL)
			{
				// run through the surfaces in the cell looking for a match for this position (may be as many as 3 surfaces per cell max)
				for (uint32 ns=0; ns<3; ++ns)
				{
					isAccessible |= worldMap.getSafeWorldPosition(pos, CSlot(ns)).isValid();
				}
			}
			// setup the next pixel in the output buffers...
			_Buffer[y*_ScanWidth+x]= (isAccessible? 0: (TBufferEntry)~0u);
		}
	}
}

void CProximityMapBuffer::calculateZones(TZones& zones)
{
	// clear out the result buffer before starting work
	zones.clear();

	// setup a container to hold the accessible points within this buffer
	typedef std::set<uint32> TAccessiblePoints;
	TAccessiblePoints accessiblePoints;

	// start by building the set of all accessible points
	for (uint32 i=0;i<_Buffer.size();++i)
	{
		if (_Buffer[i]==0)
			accessiblePoints.insert(i);
	}

	// while there are still points remaining in the set we must have another zone to process
	while (!accessiblePoints.empty())
	{
		// append a new zone to the zones vector and get a refference to it
		zones.push_back( CProximityZone(_ScanWidth,_ScanHeight,_XOffset,_YOffset) );
		CProximityZone& theZone= zones.back();

		// setup a todo list representing points that are part of the surface that we are dealing with
		// that haven't yet been treated to check for neighbours, etc
		std::vector<uint32> todo;

		// get hold of the first point in the accessilbe points set and push it onto the todo list
		todo.push_back(*accessiblePoints.begin());
		accessiblePoints.erase(todo.back());

		// while we have more points to deal with ...
		while (!todo.empty())
		{
			// pop the next point off the todo list
			uint32 thePoint= todo.back();
			todo.pop_back();

			// add the point to the zone
			theZone.add(thePoint);

			// a little macro for the code to perform for each movement test...
			#define TEST_MOVE(xoffs,yoffs)\
				{\
					TAccessiblePoints::iterator it= accessiblePoints.find(thePoint+xoffs+_ScanWidth*yoffs);\
					if (it!=accessiblePoints.end())\
					{\
						todo.push_back(*it);\
						accessiblePoints.erase(it);\
					}\
				}

			// N, S, W, E moves
			TEST_MOVE( 0, 1);
			TEST_MOVE( 0,-1);
			TEST_MOVE( 1, 0);
			TEST_MOVE(-1, 0);

			// NW, NE, WS, SE moves
			TEST_MOVE( 1, 1);
			TEST_MOVE(-1, 1);
			TEST_MOVE( 1,-1);
			TEST_MOVE(-1,-1);

			#undef TEST_MOVE
		}
	}

	nlinfo("Found %u zones",zones.size());
}

void CProximityMapBuffer::_prepareBufferForZoneProximityMap(const CProximityZone& zone,TBuffer& zoneBuffer,TOffsetsVector& accessiblePoints)
{
	// the length of runs that we consider too short to deal with...
	const uint32 shortRunLength=5;

	// redimention and initialise the zone buffer
	uint32 zoneWidth= zone.getZoneWidth();
	uint32 zoneHeight= zone.getZoneHeight();
	zoneBuffer.clear();
	zoneBuffer.resize(zoneWidth*zoneHeight,(TBufferEntry)~0u);

	// setup the buffer's accessible points and prime vects[0] with the set of accessible points in the zone buffer
	for (uint32 i=0;i<zone.getOffsets().size();++i)
	{
		// lookup the next offset in the zone's offsets vector and remap to a zone-relative offset
		uint32 zoneOffset= zone.remapOffset(zone.getOffsets()[i]);
		// flag the appropriate entry in the zoneBuffer as 'distance=0'
		zoneBuffer[zoneOffset]= 0;
		// add the zone offset to the accessible points vector
		accessiblePoints.push_back(zoneOffset);
	}
/*
	// run through the zone buffer looking for points that are surrounded that we can just throw out...

	// start by flagging all points that belong to a short run in the y direction
	for (uint32 i=0;i<zoneWidth;++i)
	{
		// setup start and end offsets marking first and last 'zero' value pixels in the columb
		uint32 startOffset=i, endOffset=i+(zoneHeight-1)*zoneWidth;
		for (; startOffset<endOffset && zoneBuffer[startOffset]!=0; startOffset+= zoneWidth) {}
		for (; endOffset>startOffset && zoneBuffer[endOffset]!=0;   endOffset-= zoneWidth) {}

		for (uint32 offset=startOffset, marker=startOffset;offset<=endOffset;offset+=zoneWidth)
		{
			// see if this is an accessible position 
			if (zoneBuffer[offset]==0)
			{
				// look to see whether this position follows a short run of inaccessible positions
				sint32 inaccessibleRunLength= (offset-marker)/zoneWidth-1;
				if (inaccessibleRunLength>0 && inaccessibleRunLength<=shortRunLength)
				{
					// flag all of the points in this run as belonging to a short run in y
					for (uint32 j=marker+zoneWidth;j<offset;j+=zoneWidth)
					{
//						zoneBuffer[j]&=~1;
						zoneBuffer[j]=0;
						accessiblePoints.push_back(j);
					}
				}
				// mark this position as 'last accessible position'
				marker= offset;
			}
		}
	}

	// continue by dealing with all points that belong to a short run in the x direction
	for (uint32 i=0;i<zoneHeight;++i)
	{
		// setup start and end offsets marking first and last 'zero' value pixels in the column
		uint32 startOffset= i*zoneWidth;
		uint32 endOffset= startOffset+zoneWidth-1;

		for (; startOffset<endOffset && zoneBuffer[startOffset]!=0; ++startOffset) {}
		for (; endOffset>startOffset && zoneBuffer[endOffset]!=0; --endOffset) {}

		for (uint32 offset=startOffset, marker=startOffset;offset<=endOffset;++offset)
		{
			// see if this is an accessible position 
			if (zoneBuffer[offset]==0)
			{
				// look to see whether this position follows a short run of inaccessible positions
				sint32 inaccessibleRunLength= offset-marker-1;
				if (inaccessibleRunLength>0 && inaccessibleRunLength<=shortRunLength)
				{
					// flag all of the points in this run as belonging to a short run in y
					for (uint32 j=marker+1;j<offset;++j)
					{
//						zoneBuffer[j]&=~2;
						zoneBuffer[j]=0;
						accessiblePoints.push_back(j);
					}
				}
				// mark this position as 'last accessible position'
				marker= offset;
			}
		}
	}
*/
}

void CProximityMapBuffer::generateZoneProximityMap(const CProximityZone& zone,TBuffer& zoneBuffer)
{
	// a set of vectors to hold sets of points that need to be treated
	TOffsetsVector vects[16];
	// a counter that should always contain sum of all vects[i].size()
	uint32 entriesToTreat= 0;

	// setup the buffer's accessible points and prime vects[0] with the set of accessible points in the zone buffer
	_prepareBufferForZoneProximityMap(zone,zoneBuffer,vects[0]);
	entriesToTreat= (uint32)vects[0].size();

	// lookup the buffer dimentions
	uint32 zoneWidth= zone.getZoneWidth();
	uint32 zoneHeight= zone.getZoneHeight();

	// for dist=0 to ? treat points with distance 'dist' from centre, iterating until all vects are empty
	for (TBufferEntry dist=0; entriesToTreat!=0; ++dist)
	{
		// setup refference to the vector that we are supposed to be iterating over for this dist
		TOffsetsVector &vect= vects[dist&15];

		// iterate over contents of points for this distance, treating NSWE neighbours
		for(TOffsetsVector::iterator it=vect.begin(); it!=vect.end(); ++it)
		{
			uint32 val=(*it);

			// deal with the case where this point has already been refferenced via a better route
			if (zoneBuffer[val]<dist)
				continue;

			// write the new distance into this buffer entry
			zoneBuffer[val]=dist;

			// decompose into x and y in order to manage identification of neighbour cells correctly
			uint32 x= val% zoneWidth;
			uint32 y= val/ zoneWidth;

			#define TEST_MOVE(xoffs,yoffs,newDist)\
				{\
					if (((uint32)(x+(xoffs))<zoneWidth) && ((uint32)(y+(yoffs))<zoneHeight))\
					{\
						uint32 newVal= val+(xoffs)+((yoffs)*zoneWidth);\
						if (zoneBuffer[newVal]>(newDist))\
						{\
							zoneBuffer[newVal]=(newDist);\
							vects[(newDist)&15].push_back(newVal);\
							++entriesToTreat;\
						}\
					}\
				}

			// N, S, W, E moves
			TEST_MOVE( 0, 1,dist+5);
			TEST_MOVE( 0,-1,dist+5);
			TEST_MOVE( 1, 0,dist+5);
			TEST_MOVE(-1, 0,dist+5);

			// NW, NE, WS, SE moves
			TEST_MOVE( 1, 1,dist+7);
			TEST_MOVE(-1, 1,dist+7);
			TEST_MOVE( 1,-1,dist+7);
			TEST_MOVE(-1,-1,dist+7);

			// NNW, NNE, SSW, SSE moves
			TEST_MOVE( 1, 2,dist+11);
			TEST_MOVE(-1, 2,dist+11);
			TEST_MOVE( 1,-2,dist+11);
			TEST_MOVE(-1,-2,dist+11);

			// WNW, WSW, ENE, ESE moves
			TEST_MOVE( 2, 1,dist+11);
			TEST_MOVE(-2, 1,dist+11);
			TEST_MOVE( 2,-1,dist+11);
			TEST_MOVE(-2,-1,dist+11);

			#undef TEST_MOVE
		}

		// clear out the vector
		entriesToTreat-= (uint32)vect.size();
		vect.clear();
	}
}

const TBuffer& CProximityMapBuffer::getBuffer() const
{
	return _Buffer;
}

uint32 CProximityMapBuffer::getScanHeight() const
{
	return _ScanHeight;
}

uint32 CProximityMapBuffer::getScanWidth() const
{
	return _ScanWidth;
}


//-------------------------------------------------------------------------------------------------
// NLMISC_COMMAND pacsBuildProximityMap
//-------------------------------------------------------------------------------------------------

NLMISC_COMMAND(pacsBuildProximityMap,"build a set of proximity maps from a cwmap2 file","<file name root>")
{
	// deal with command arguments
	nlinfo("Building proximity map...");
	if(args.size()<1)
		return false;
	const std::string& name= args[0];

	// load the collision map file and generate our buffer from it
	CProximityMapBuffer buffer;
	string	ext = CFile::getExtension(name);
	if (ext == "")
		ext = "cwmap2";
	string fileName= OutputPath+CFile::getFilenameWithoutExtension(name)+"."+ext;
	nlinfo("Building proximity map: Loading cwmap2 file: %s",fileName.c_str());
	buffer.load(fileName);

	// generate a tga file from the buffer contents
	nlinfo("Building proximity map: Writing accessibilty file: %s",(OutputPath+name+"_accessible.tga").c_str());
	writeProximityBufferToTgaFile(OutputPath+name+"_accessible.tga",buffer.getBuffer(),buffer.getScanWidth(),buffer.getScanHeight());

	// divide space up into non-connecting zones
	nlinfo("Building proximity map: Dividing accessible space into zones");
	CProximityMapBuffer::TZones zones;
	buffer.calculateZones(zones);

	// setup a string to contain the proximity zone list (in csv format)
	NLMISC::CSString proximityZoneList="idx,x_offset,y_offset,width,height,xmin,ymin,xmax,ymax\n";

	// generate proximity info for inaccessible points in each zone
	for (uint32 i=0;i<zones.size();++i)
	{
		// generate a proximity map for this zone
		nlinfo("Building proximity map: generating zone proximity map: %d/ %d",i+1,zones.size()+1);
		TBuffer zoneBuffer;
		buffer.generateZoneProximityMap(zones[i],zoneBuffer);

		// write the proximity map to an output file
		std::string fileName= NLMISC::toString("%s_prox_%03d.tga",(OutputPath+name).c_str(),i);
		nlinfo("Building proximity map: writing output file: %s",fileName.c_str());
		writeProximityBufferToTgaFile(fileName,zoneBuffer,zones[i].getZoneWidth(),zones[i].getZoneHeight());

		// process the proximity map to generate a cleaned version
		TBuffer cleanBuffer;
		processProximityBuffer(zoneBuffer,zones[i].getZoneWidth(),cleanBuffer);

		// write the processed proximity map to an output file
		std::string fileName2= NLMISC::toString("%s_prox_%03db.tga",(OutputPath+name).c_str(),i);
		nlinfo("Building proximity map: writing output file: %s",fileName2.c_str());
		writeProximityBufferToTgaFile(fileName2,cleanBuffer,zones[i].getZoneWidth(),zones[i].getZoneHeight());

		// add an entry to our proximity zone list string
		proximityZoneList+=NLMISC::toString("%u,%d,%d,%u,%u,%u,%u,%u,%u\n", i,
			zones[i].getXOffset(), zones[i].getYOffset(), zones[i].getZoneWidth(), zones[i].getZoneHeight(),
			zones[i].getBoundXMin(), zones[i].getBoundYMin(), zones[i].getBoundXMax(), zones[i].getBoundYMax());
	}

	// write out the proximity zone index file
	std::string proximityZoneIndexFileName= OutputPath+CFile::getFilenameWithoutExtension(name)+"_zones.csv";
	nlinfo("Building proximity map: Writing zone index file: %s",proximityZoneIndexFileName.c_str());
	proximityZoneList.writeToFile(proximityZoneIndexFileName);

	nlinfo("Building proximity map: Done");
	return true;
}

