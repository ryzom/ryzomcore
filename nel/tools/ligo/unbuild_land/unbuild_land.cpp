// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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
//
// This utility is intended to rescue a lost .land file from existing
// runtime zones. It does not recover the heightmap.
//
// Author: Jan BOON (Kaetemi) <jan.boon@kaetemi.be>

// #include "../../3d/zone_lib/zone_utility.h"

#include <iostream>
#include <nel/misc/types_nl.h>
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
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

bool s_Height = false;

std::string s_ZoneBricksDirIn; // UTF-8 path
std::string s_ZoneRuntimeDirIn; // UTF-8 path
std::string s_LandFileOut; // UTF-8 path

CZoneRegion s_Land;

bool saveLand()
{
	try
	{
		COFile fileOut;
		if (fileOut.open(s_LandFileOut, false, true, false))
		{
			COXml xml;
			nlverify(xml.init(&fileOut));
			s_Land.serial(xml);
		}
		else
		{
			nlwarning("Can't open the land file for writing: %s", s_LandFileOut.c_str());
			return false;
		}
	}
	catch (const Exception &e)
	{
		nlwarning("Error in writing land file: %s", e.what());
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
	xStr = NLMISC::toUpperAscii(xStr);
	x = ((xStr[0] - 'A') * 26 + (xStr[1] - 'A'));
	return true;
Fail:
	x = -1;
	y = -1;
	return false;
}

void centerVertices(std::vector<CVector> &vertices)
{
	CVector avg = CVector(0.0f, 0.0f, 0.0f);
	for (ptrdiff_t i = 0; i < vertices.size(); ++i)
		avg += vertices[i];
	avg /= (float)vertices.size();
	nldebug("Average: %f, %f", avg.x, avg.y);
	for (ptrdiff_t i = 0; i < vertices.size(); ++i)
		vertices[i] -= avg;
}

void offsetVertices(std::vector<CVector> &vertices, int x, int y)
{
	CVector off = CVector((float)x * s_CellSize, (float)y * s_CellSize, 0.0f);
	for (ptrdiff_t i = 0; i < vertices.size(); ++i)
		vertices[i] -= off;
}

float ratePoints(const std::vector<CVector> &zone, const std::vector<CVector> &ref, float xx, float xy, float yx, float yy)
{
	// Rudimentary distance rating of vertices (not very reliable, but good enough!)

	float md = 0.f;
	// std::vector<CVector> refcpy = ref;
	std::vector<bool> usedref;
	usedref.resize(ref.size(), false);
	for (ptrdiff_t i = 0; i < zone.size(); ++i)
	{
		if (ref.size())
		{
			int lowj = 0;
			float lowv = (CVector(ref[0].x * xx + ref[0].y * xy, ref[0].x * yx + ref[0].y * yy, ref[0].z) - zone[i]).sqrnorm();
			for (ptrdiff_t j = 1; j < ref.size(); ++j)
			{
				float v = (CVector(ref[j].x * xx + ref[j].y * xy, ref[j].x * yx + ref[j].y * yy, ref[j].z) - zone[i]).sqrnorm();
				if (v < lowv)
				{
					lowj = j;
					lowv = v;
				}
			}
			md += sqrtf(lowv);
			usedref[lowj] = true;
			// keep it! - refcpy.erase(refcpy.begin() + lowj);
		}
		else
		{
			md += zone[i].norm();
		}
	}
#if 0
	md = 0.f;
	std::vector<bool> usedzone;
	usedzone.resize(zone.size(), false);
	for (ptrdiff_t j = 0; j < ref.size(); ++j)
	{
		if (usedref[j])
		{
			if (zone.size())
			{
				int lowi = 0;
				float lowv = (CVector2f(ref[j].x * xx + ref[j].y * xy, ref[j].x * yx + ref[j].y * yy) - zone[0]).sqrnorm();
				for (ptrdiff_t i = 1; i < zone.size(); ++i)
				{
					float v = (CVector2f(ref[j].x * xx + ref[j].y * xy, ref[j].x * yx + ref[j].y * yy) - zone[i]).sqrnorm();
					if (v < lowv)
					{
						lowi = i;
						lowv = v;
					}
				}
				md += lowv;
				usedzone[lowi] = true;
			}
			else
			{
				md += ref[j].norm();
			}
		}
	}
	md = 0.f;
	int nc = 0;
	for (ptrdiff_t i = 0; i < zone.size(); ++i)
	{
		if (usedzone[i])
		{
			if (ref.size())
			{
				int lowj = 0;
				float lowv = (CVector2f(ref[0].x * xx + ref[0].y * xy, ref[0].x * yx + ref[0].y * yy) - zone[i]).sqrnorm();
				for (ptrdiff_t j = 1; j < ref.size(); ++j)
				{
					float v = (CVector2f(ref[j].x * xx + ref[j].y * xy, ref[j].x * yx + ref[j].y * yy) - zone[i]).sqrnorm();
					if (v < lowv)
					{
						lowj = j;
						lowv = v;
					}
				}
				md += sqrtf(lowv);
				usedref[lowj] = true;
				// keep it! - refcpy.erase(refcpy.begin() + lowj);
			}
			else
			{
				md += zone[i].norm();
			}
		}
		else
		{
			++nc;
			// md += 1.0f;
			// md += 0.01;
		}
	}
	for (ptrdiff_t j = 0; j < ref.size(); ++j)
	{
		if (!usedref[j])
		{
			// md += 1.0f;
			// md += 0.01;
		}
	}
	if (nc * 8 > zone.size())
		return md + 10000;
#endif
	return md;
}

void findBestBrick(std::string &brick, int &rotate, int &flip, float &es, std::vector<CVector> &zoneVertices, const std::map<std::string, std::vector<CVector>> &brickVertices)
{
	float bestPoints = (float)(uint32)~0;
	for (std::map<std::string, std::vector<CVector>>::const_iterator it = brickVertices.begin(), end = brickVertices.end(); it != end; ++it)
	{
		float rating;
		rating = ratePoints(zoneVertices, it->second, 1.0, 0.0, 0.0, 1.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 0;
			flip = 0;
			bestPoints = rating;
		}
		rating = ratePoints(zoneVertices, it->second, 0.0, -1.0, 1.0, 0.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 1;
			flip = 0;
			bestPoints = rating;
		}
		rating = ratePoints(zoneVertices, it->second, -1.0, 0.0, 0.0, -1.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 2;
			flip = 0;
			bestPoints = rating;
		}
		rating = ratePoints(zoneVertices, it->second, 0.0, 1.0, -1.0, 0.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 3;
			flip = 0;
		}
		rating = ratePoints(zoneVertices, it->second, -1.0, 0.0, 0.0, 1.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 0;
			flip = 1;
			bestPoints = rating;
		}
		rating = ratePoints(zoneVertices, it->second, 0.0, -1.0, -1.0, 0.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 1;
			flip = 1;
			bestPoints = rating;
		}
		rating = ratePoints(zoneVertices, it->second, 1.0, 0.0, 0.0, -1.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 2;
			flip = 1;
			bestPoints = rating;
		}
		rating = ratePoints(zoneVertices, it->second, 0.0, 1.0, 1.0, 0.0);
		if (rating < bestPoints)
		{
			brick = it->first;
			rotate = 3;
			flip = 1;
			bestPoints = rating;
		}
	}
	es = bestPoints;
}

bool unbuildLand()
{
	s_Land.resize(s_ZoneMinX, s_ZoneMaxX, s_ZoneMinY, s_ZoneMaxY);

	// float th = s_CellSize * 0.5f;
	float th = (s_CellSize * 0.5f) - (s_CellSize * 0.2f * 0.5f);

	// Read in all the bricks
	std::vector<std::string> brickFiles;
	CPath::getPathContent(s_ZoneBricksDirIn, false, false, true, brickFiles);
	std::map<std::string, std::vector<CVector>> brickVertices;
	for (std::vector<std::string>::const_iterator it = brickFiles.begin(), end = brickFiles.end(); it != end; ++it)
	{
		if (CFile::getExtension(*it) != "zone")
			continue;

		if (NLMISC::startsWith(CFile::getFilename(*it), "converted-")) // Not a real ligo
			continue;

		// if (NLMISC::startsWith(CFile::getFilename(*it), "foret-moor")) // Not useful for r2_forest
		// 	continue;
		// if (NLMISC::startsWith(CFile::getFilename(*it), "goo-")) // Not useful for r2_forest
		// 	continue;
		// if (NLMISC::endsWith(CFile::getFilenameWithoutExtension(*it), "_zc")) // Not useful for r2_forest
		// 	continue;

		nldebug("Load %s", CFile::getFilenameWithoutExtension(*it).c_str());

		CIFile zoneFile(*it);
		CZone zone;
		zone.serial(zoneFile);
		zoneFile.close();

		// Retrieve patches and vertices
		uint16 zoneId = zone.getZoneId();
		std::vector<CPatchInfo> zonePatches;
		std::vector<CBorderVertex> zoneBorderVertices;
		zone.retrieve(zonePatches, zoneBorderVertices);

		brickVertices[CFile::getFilenameWithoutExtension(*it)] = std::vector<CVector>();
		std::vector<CVector> &vec = brickVertices[CFile::getFilenameWithoutExtension(*it)];
		CVector off = CVector(s_CellSize * 0.5f, s_CellSize * 0.5f, 0.0f);
		for (ptrdiff_t i = 0; i < zonePatches.size(); ++i)
		{
			for (ptrdiff_t j = 0; j < 4; ++j)
			{
				float rx = zonePatches[i].Patch.Vertices[j].x - off.x;
				float ry = zonePatches[i].Patch.Vertices[j].y - off.y;
				if (rx <= -th || rx >= th
					|| ry <= -th || ry >= th)
					goto SkipA;
			}
			for (ptrdiff_t j = 0; j < 4; ++j)
			{
				float rx = zonePatches[i].Patch.Vertices[j].x - off.x;
				float ry = zonePatches[i].Patch.Vertices[j].y - off.y;
				vec.push_back(CVector(rx, ry, s_Height ? zonePatches[i].Patch.Vertices[j].z : 0.0f));
			}
		SkipA:;
		}
		
		// centerVertices(vec);
	}

	std::vector<std::string> runtimeFiles;
	CPath::getPathContent(s_ZoneRuntimeDirIn, false, false, true, runtimeFiles);
	for (std::vector<std::string>::const_iterator it = runtimeFiles.begin(), end = runtimeFiles.end(); it != end; ++it)
	{
		if (CFile::getExtension(*it) != "zonel")
			continue;

		int x, y;
		std::string zoneName = CFile::getFilenameWithoutExtension(*it);
		if (!getXYFromZoneName(x, y, zoneName))
		{
			nlerrornoex("Bad zone name: %s", zoneName.c_str());
			continue;
		}

		CIFile zoneFile(*it);
		CZone zone;
		zone.serial(zoneFile);
		zoneFile.close();

		// Retrieve patches and vertices
		uint16 zoneId = zone.getZoneId();
		std::vector<CPatchInfo> zonePatches;
		std::vector<CBorderVertex> zoneBorderVertices;
		zone.retrieve(zonePatches, zoneBorderVertices);

		std::vector<CVector> vec;
		CVector off = CVector((float)x * s_CellSize, (float)y * s_CellSize, 0.0f) + CVector(s_CellSize * 0.5f, s_CellSize * 0.5f, 0.0f);
		for (ptrdiff_t i = 0; i < zonePatches.size(); ++i)
		{
			for (ptrdiff_t j = 0; j < 4; ++j)
			{
				float rx = zonePatches[i].Patch.Vertices[j].x - off.x;
				float ry = zonePatches[i].Patch.Vertices[j].y - off.y;
				if (rx <= -th || rx >= th
					|| ry <= -th || ry >= th)
					goto SkipB;
			}
			for (ptrdiff_t j = 0; j < 4; ++j)
			{
				float rx = zonePatches[i].Patch.Vertices[j].x - off.x;
				float ry = zonePatches[i].Patch.Vertices[j].y - off.y;
				vec.push_back(CVector(rx, ry, s_Height ? zonePatches[i].Patch.Vertices[j].z : 0.0f));
			}
		SkipB:;
		}

		// offsetVertices(vec, x, y);
		// centerVertices(vec);

		std::string brick;
		int rotate;
		int flip;
		float es;
		findBestBrick(brick, rotate, flip, es, vec, brickVertices);

		nlinfo("Zone: %s, brick: %s, rotate: %i; flip: %i, error score: %f", zoneName.c_str(), brick.c_str(), rotate, flip, es);

		s_Land.setName(x, y, brick);
		s_Land.setRot(x, y, rotate);
		s_Land.setFlip(x, y, flip);
	}

	return saveLand();
}

} /* anonymous namespace */

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext myApplicationContext;

	NLMISC::CCmdArgs args;

	args.addAdditionalArg("land", "Output ligo land file");
	args.addAdditionalArg("zonebricks", "Input zone bricks directory");
	args.addAdditionalArg("zoneruntime", "Input runtime zone directory");
	args.addAdditionalArg("zonemin", "Zone boundary");
	args.addAdditionalArg("zonemax", "Zone boundary");
	args.addArg("", "cellsize", "meters", "Zone cell size");
	args.addArg("", "height", "", "Take Z coordinate into account");

	if (!args.parse(argc, argv))
	{
		return EXIT_FAILURE;
	}

	s_LandFileOut = args.getAdditionalArg("land")[0];
	s_ZoneBricksDirIn = args.getAdditionalArg("zonebricks")[0];
	s_ZoneRuntimeDirIn = args.getAdditionalArg("zoneruntime")[0];

	sint32 zoneMinX, zoneMinY;
	sint32 zoneMaxX, zoneMaxY;
	if (!getXYFromZoneName(zoneMinX, zoneMinY, args.getAdditionalArg("zonemin")[0])
	    || !getXYFromZoneName(zoneMaxX, zoneMaxY, args.getAdditionalArg("zonemax")[0]))
	{
		goto Fail;
	}
	s_ZoneMinX = min(zoneMinX, zoneMaxX);
	s_ZoneMaxX = max(zoneMinX, zoneMaxX);
	s_ZoneMinY = min(zoneMinY, zoneMaxY);
	s_ZoneMaxY = max(zoneMinY, zoneMaxY);

	s_Height = args.haveLongArg("height");

	if (args.haveLongArg("cellsize"))
	{
		if (!NLMISC::fromString(args.getLongArg("cellsize")[0], s_CellSize))
			goto Fail;
	}

	if (!unbuildLand())
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
Fail:
	args.displayHelp();
	return EXIT_FAILURE;
}
