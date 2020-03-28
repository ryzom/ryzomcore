// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "../zone_lib/zone_utility.h"

#include <iostream>
#include <nel/misc/types_nl.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/common.h>
#include <nel/misc/cmd_args.h>
#include <nel/misc/bitmap.h>
//#include <nel/3d/quad_tree.h>
#include <nel/3d/zone.h>
//#include <nel/3d/landscape.h>
//#include <nel/3d/zone_smoother.h>
//#include <nel/3d/zone_tgt_smoother.h>
//#include <nel/3d/zone_corner_smoother.h>
#include <nel/ligo/zone_region.h>
#include <vector>
#include <set>

using namespace NL3D;
using namespace NLMISC;
using namespace NLLIGO;
using namespace std;

namespace /* anonymous */
{

sint32 s_ZoneMinX, s_ZoneMinY, s_ZoneMaxX, s_ZoneMaxY;
float s_CellSize = 160.0f;

float s_ZFactor = 1.0f;
NLMISC::CBitmap *s_HeightMap;

float s_ZFactor2 = 1.0f;
NLMISC::CBitmap *s_HeightMap2;

std::string s_InputZone; // UTF-8
std::string s_OutputZone; // UTF-8

CZoneRegion s_Land;

bool loadLand(const string &filename)
{
	try
	{
		CIFile fileIn;
		if (fileIn.open (filename))
		{
			CIXml xml(true);
			nlverify(xml.init(fileIn));
			s_Land.serial(xml);
		}
		else
		{
			nlwarning("Can't open the land file: %s", filename.c_str());
			return false;
		}
	}
	catch (const Exception& e)
	{
		nlwarning("Error in land file: %s", e.what());
		return true;
	}
	return true;
}

bool getXYFromZoneName(sint32 &x, sint32 &y, const string &zoneName)
{
	string xStr, yStr;
	uint32 i = 0;
	while (zoneName[i] != '_')
	{
		yStr += zoneName[i];
		++i;
		if (i == zoneName.size())
			goto Fail;
	}
	if (!NLMISC::fromString(yStr, y))
		goto Fail;
	y = -y;
	++i;
	while (i < zoneName.size())
	{
		xStr += zoneName[i];
		++i;
	}
	if (xStr.size() != 2)
		goto Fail;
	xStr = NLMISC::toUpper(xStr);
	x = ((xStr[0] - 'A') * 26 + (xStr[1] - 'A'));
	return true;
Fail:
	x = -1;
	y = -1;
	return false;
}

float getHeight(float x, float y)
{
	float deltaZ = 0.0f, deltaZ2 = 0.0f;
	CRGBAF color;
	sint32 sizeX = s_ZoneMaxX - s_ZoneMinX + 1;
	sint32 sizeY = s_ZoneMaxY - s_ZoneMinY + 1;

	clamp(x, s_CellSize * s_ZoneMinX, s_CellSize * (s_ZoneMaxX + 1));
	clamp(y, s_CellSize * s_ZoneMinY, s_CellSize * (s_ZoneMaxY + 1));

	if (s_HeightMap != NULL)
	{
		float xc = (x - s_CellSize * s_ZoneMinX) / (s_CellSize * sizeX);
		float yc = 1.0f - ((y - s_CellSize * s_ZoneMinY) / (s_CellSize * sizeY));
		color = s_HeightMap->getColor(xc, yc);
		color *= 255;
		deltaZ = color.A;
		deltaZ = deltaZ - 127.0f; // Median intensity is 127
		deltaZ *= s_ZFactor;
	}

	if (s_HeightMap2 != NULL)
	{
		float xc = (x - s_CellSize * s_ZoneMinX) / (s_CellSize * sizeX);
		float yc = 1.0f - ((y - s_CellSize * s_ZoneMinY) / (s_CellSize * sizeY));
		color = s_HeightMap2->getColor(xc, yc);
		color *= 255;
		deltaZ2 = color.A;
		deltaZ2 = deltaZ2 - 127.0f; // Median intensity is 127
		deltaZ2 *= s_ZFactor2;
	}

	return (deltaZ + deltaZ2);
}

NLMISC::CVector getHeightNormal(float x, float y)
{
	sint32 SizeX = s_ZoneMaxX - s_ZoneMinX + 1;
	sint32 SizeY = s_ZoneMaxY - s_ZoneMinY + 1;
	sint32 bmpW, bmpH;

	// get width/height of the bitmap
	if (s_HeightMap != NULL)
	{
		bmpW = s_HeightMap->getWidth();
		bmpH = s_HeightMap->getHeight();
	}
	else if (s_HeightMap2 != NULL)
	{
		bmpW = s_HeightMap2->getWidth();
		bmpH = s_HeightMap2->getHeight();
	}
	else
	{
		// no heightmap: unmodified normal
		return CVector::K;
	}

	// compute a good delta to compute tangents of the heightmap: 1/10 of a pixel, avoiding precision problem.
	float dx = ((s_CellSize * SizeX) / bmpW) / 10; // eg: 160m/20pixels/10= 0.8
	float dy = ((s_CellSize * SizeY) / bmpH) / 10;

	// compute tangent around the position.
	float hc = getHeight(x, y);
	float hx = getHeight(x + dx, y);
	float hy = getHeight(x, y + dy);
	CVector ds(dx, 0, hx - hc);
	CVector dt(0, dy, hy - hc);

	// compute the heightmap normal with the tangents
	return (ds ^ dt).normed();
}

void applyZoneHeightmap()
{
	// Load zone
	CIFile zoneFile(s_InputZone);
	CZone zone;
	zone.serial(zoneFile);
	zoneFile.close();

	// Retrieve patches and vertices
	uint16 zoneId = zone.getZoneId();
	std::vector<CPatchInfo> zonePatches;
	std::vector<CBorderVertex> zoneBorderVertices;
	zone.retrieve(zonePatches, zoneBorderVertices);

	// Apply the Heighmap to all vertices/tangents/interiors (see Land Export tool.)
	for (size_t i = 0; i < zonePatches.size(); ++i)
	{
		CPatchInfo &rPI = zonePatches[i];

		// Elevate the vertices.
		CVector verticesBeforeHeightMap[4];
		for (size_t j = 0; j < 4; ++j)
		{
			verticesBeforeHeightMap[j] = rPI.Patch.Vertices[j];
			float height = getHeight(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);
			rPI.Patch.Vertices[j].z += height;
		}

		// Interior and tangent are rotated to follow the heightmap normal, avoiding the "Stair Effect".
		// Compute the matrix to apply to interiors and tangents.
		CMatrix tgMatrix[4];
		for (size_t j = 0; j < 4; ++j)
		{
			// compute the normal of the heightmap.
			CVector hmapNormal = getHeightNormal(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);

			// Compute the rotation which transforms the original normal: (0,0,1), to this normal.
			CAngleAxis angleAxis;
			angleAxis.Axis = CVector::K ^ hmapNormal;
			angleAxis.Angle = (float)asin(angleAxis.Axis.norm());
			angleAxis.Axis.normalize();

			// build the matrix which transform the old tgt/interior to his newValue:
			// newVertexPos + rotate * (oldTgPos - oldVertexPos)
			tgMatrix[j].identity();
			tgMatrix[j].translate(rPI.Patch.Vertices[j]);
			tgMatrix[j].setRot(CQuat(angleAxis));
			tgMatrix[j].translate(-verticesBeforeHeightMap[j]);
		}

		// For all interior.
		for (size_t j = 0; j < 4; ++j)
			rPI.Patch.Interiors[j] = tgMatrix[j] * rPI.Patch.Interiors[j];

		// when j == 7 or 0 use vertex 0 for delta Z to ensure continuity of normals
		// when j == 1 or 2 use vertex 1
		// when j == 3 or 4 use vertex 2
		// when j == 5 or 6 use vertex 3
		for (size_t j = 0; j < 8; ++j)
		{
			// get the correct vertex
			uint vertexId = ((j + 1) / 2) % 4;
			// apply the tgMatrix to the tangent
			rPI.Patch.Tangents[j] = tgMatrix[vertexId] * rPI.Patch.Tangents[j];
		}
	}

	// Save zone
	zone.build(zoneId, zonePatches, zoneBorderVertices);
	COFile centerSave(s_OutputZone);
	nldebug("Writing file %s", s_OutputZone.c_str());
	zone.serial(centerSave);
}

} /* anonymous namespace */

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext myApplicationContext;

	NLMISC::CCmdArgs args;

	args.addAdditionalArg("zonenhw", "Input zone"); // .zonenhw
	args.addAdditionalArg("zonew", "Output zone"); // .zonew
	args.addArg("", "land", "land", "Ligo land file (either specify this or the boundaries)");
	args.addArg("", "zonemin", "zone", "Zone boundary");
	args.addArg("", "zonemax", "zone", "Zone boundary");
	args.addArg("", "cellsize", "meters", "Zone cell size");
	args.addArg("", "zfactor", "factor", "Factor for heightmap");
	args.addArg("", "heightmap", "bitmap", "Heightmap");
	args.addArg("", "zfactor2", "factor", "Factor for second heightmap");
	args.addArg("", "heightmap2", "bitmap", "Second heightmap");
	// TODO: args.addArg("", "batch", "", "Process all zones in input (specify input as C:/folder/.zonenhw)");

	if (!args.parse(argc, argv))
	{
		return EXIT_FAILURE;
	}

	s_InputZone = args.getAdditionalArg("zonenhw")[0];
	s_OutputZone = args.getAdditionalArg("zonew")[0];

	if (args.haveLongArg("zonemin") && args.haveLongArg("zonemax"))
	{
		sint32 zoneMinX, zoneMinY;
		sint32 zoneMaxX, zoneMaxY;
		if (!getXYFromZoneName(zoneMinX, zoneMinY, args.getLongArg("zonemin")[0])
			|| !getXYFromZoneName(zoneMaxX, zoneMaxY, args.getLongArg("zonemax")[0]))
		{
			goto Fail;
		}
		s_ZoneMinX = min(zoneMinX, zoneMaxX);
		s_ZoneMaxX = max(zoneMinX, zoneMaxX);
		s_ZoneMinY = min(zoneMinY, zoneMaxY);
		s_ZoneMaxY = max(zoneMinY, zoneMaxY);
	}
	else if (args.haveLongArg("land"))
	{
		if (!loadLand(args.getLongArg("land")[0]))
			goto Fail;
		s_ZoneMinX = s_Land.getMinX();
		s_ZoneMaxX = s_Land.getMaxX();
		s_ZoneMinY = s_Land.getMinY();
		s_ZoneMaxY = s_Land.getMaxY();
	}
	else
	{
		nlwarning("Must have either both 'zonemin' and 'zonemax', or 'land' specified");
		goto Fail;
	}

	if (args.haveLongArg("heightmap"))
	{
		nldebug("Loading height map");
		s_HeightMap = new CBitmap();
		try
		{
			CIFile inFile;
			if (inFile.open(args.getLongArg("heightmap")[0]))
			{
				s_HeightMap->load(inFile);
			}
			else
			{
				nldebug("Cant load height map: %s", args.getLongArg("heightmap")[0].c_str());
				delete s_HeightMap;
				s_HeightMap = NULL;
			}
		}
		catch (const Exception &)
		{
			nldebug("Cant load height map: %s", args.getLongArg("heightmap")[0].c_str());
			delete s_HeightMap;
			s_HeightMap = NULL;
		}
	}

	if (args.haveLongArg("heightmap2"))
	{
		nldebug("Loading height map");
		s_HeightMap2 = new CBitmap();
		try
		{
			CIFile inFile;
			if (inFile.open(args.getLongArg("heightmap2")[0]))
			{
				s_HeightMap2->load(inFile);
			}
			else
			{
				nldebug("Cant load height map: %s", args.getLongArg("heightmap2")[0].c_str());
				delete s_HeightMap2;
				s_HeightMap2 = NULL;
			}
		}
		catch (const Exception &)
		{
			nldebug("Cant load height map: %s", args.getLongArg("heightmap2")[0].c_str());
			delete s_HeightMap2;
			s_HeightMap2 = NULL;
		}
	}

	if (args.haveLongArg("zfactor"))
	{
		if (!NLMISC::fromString(args.getLongArg("zfactor")[0], s_ZFactor))
			goto Fail;
	}

	if (args.haveLongArg("zfactor2"))
	{
		if (!NLMISC::fromString(args.getLongArg("zfactor2")[0], s_ZFactor2))
			goto Fail;
	}

	if (args.haveLongArg("cellsize"))
	{
		if (!NLMISC::fromString(args.getLongArg("cellsize")[0], s_CellSize))
			goto Fail;
	}

	applyZoneHeightmap();

	return EXIT_SUCCESS;
Fail:
	args.displayHelp();
	delete s_HeightMap;
	delete s_HeightMap2;
	return EXIT_FAILURE;
}
