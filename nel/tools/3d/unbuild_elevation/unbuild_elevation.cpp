// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2021  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
bool s_ExtendCoords;

std::string s_SourceDir; /* R:\reference\2008_july\data\r2_desert */
std::string s_ReferenceDir; /* R:\pipeline\export\continents\r2_desert\zone_weld */

std::string s_OutputPy; 

std::string s_SourceExt = "zonel";
std::string s_ReferenceExt = "zonenhw";

CZoneRegion s_Land; /* "R:\graphics\landscape\ligo\desert\r2_desert.land" */

int s_Warnings;

// unbuild_elevation --land "R:\graphics\landscape\ligo\desert\r2_desert.land" "Y:\temp\r2_desert_elevation.Py" "R:\reference\2008_july\data\r2_desert" "R:\pipeline\export\continents\r2_desert\zone_weld"
// --land "R:\graphics\landscape\ligo\jungle\zorai.land" --referenceext zonenhw "X:\wsl\big_zorai.py" "R:\reference\2008_july\data\zorai_zones" "R:\pipeline\export\continents\zorai\zone_weld"

// --land "R:\graphics\landscape\ligo\desert\r2_desert.land" "X:\wsl\big_r2_desert.py" "R:\reference\2008_july\data\r2_desert" "R:\pipeline\export\continents\r2_desert\zone_weld" --extendcoords
// --land "R:\graphics\landscape\ligo\jungle\r2_jungle.land" "X:\wsl\big_r2_jungle.py" "R:\reference\2008_july\data\r2_jungle" "R:\pipeline\export\continents\r2_jungle\zone_weld" --extendcoords
// --land "R:\graphics\landscape\ligo\jungle\r2_forest.land" "X:\wsl\big_r2_forest.py" "R:\reference\2008_july\data\r2_forest" "R:\pipeline\export\continents\r2_forest\zone_weld" --extendcoords
// --land "R:\graphics\landscape\ligo\lacustre\r2_lakes.land" "X:\wsl\big_r2_lakes.py" "R:\reference\2008_july\data\r2_lakes" "R:\pipeline\export\continents\r2_lakes\zone_weld" --extendcoords
// --land "R:\graphics\landscape\ligo\primes_racines\r2_roots.land" "X:\wsl\big_r2_roots.py" "R:\reference\2008_july\data\r2_roots" "R:\pipeline\export\continents\r2_roots\zone_weld" --extendcoords

// --land "R:\graphics\landscape\ligo\desert\r2_desert.land" --referenceext zonew "X:\wsl\check_r2_desert.py" "R:\reference\2008_july\data\r2_desert" "R:\pipeline\export\continents\r2_desert\zone_weld" --extendcoords
// --land "R:\graphics\landscape\ligo\jungle\r2_jungle.land" --referenceext zonew "X:\wsl\check_r2_jungle.py" "R:\reference\2008_july\data\r2_jungle" "R:\pipeline\export\continents\r2_jungle\zone_weld" --extendcoords
// --land "R:\graphics\landscape\ligo\primes_racines\r2_roots.land" --referenceext zonew "X:\wsl\check_r2_roots.py" "R:\reference\2008_july\data\r2_roots" "R:\pipeline\export\continents\r2_roots\zone_weld" --extendcoords

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
	xStr = NLMISC::toUpperAscii(xStr);
	x = ((xStr[0] - 'A') * 26 + (xStr[1] - 'A'));
	return true;
Fail:
	x = -1;
	y = -1;
	return false;
}

void getBitmapSize(int &w, int &h)
{
	sint32 sizeX = s_ZoneMaxX - s_ZoneMinX + 1;
	sint32 sizeY = s_ZoneMaxY - s_ZoneMinY + 1;
	w = sizeX * 20;
	h = sizeY * 20;

	if (!s_ExtendCoords)
	{
		++w;
		++h;
	}
}

void getBitmapCoord(float &xc, float &yc, float x, float y)
{
	float deltaZ = 0.0f, deltaZ2 = 0.0f;
	CRGBAF color;
	sint32 sizeX = s_ZoneMaxX - s_ZoneMinX + 1;
	sint32 sizeY = s_ZoneMaxY - s_ZoneMinY + 1;

	xc = (x - s_CellSize * s_ZoneMinX) / (s_CellSize * sizeX);
	yc = 1.0f - ((y - s_CellSize * s_ZoneMinY) / (s_CellSize * sizeY));

	if (s_ExtendCoords)
	{
		int w, h;
		getBitmapSize(w, h);
		xc -= .5f / (float)w;
		yc -= .5f / (float)h;
		xc = xc * (float)(w + 1) / (float)w;
		yc = yc * (float)(h + 1) / (float)h;
	}
}

bool processZone(std::vector<NLMISC::CVector> &output, const std::string &sourceFile, const std::string &referenceFile)
{
	std::string zone = CFile::getFilenameWithoutExtension(referenceFile);

	std::vector<CVector> sourceVertices;
	std::vector<CVector> referenceVertices;
	{
		CZoneInfo zoneInfo;
		{
			CZone zone;
			{
				CIFile f(sourceFile);
				zone.serial(f);
				f.close();
			}
			zone.retrieve(zoneInfo);
		}
		for (ptrdiff_t i = 0; i < (ptrdiff_t)zoneInfo.Patchs.size(); ++i)
		{
			sourceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[0]);
			sourceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[1]);
			sourceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[2]);
			sourceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[3]);
		}
	}
	{
		CZoneInfo zoneInfo;
		{
			CZone zone;
			{
				CIFile f(referenceFile);
				zone.serial(f);
				f.close();
			}
			zone.retrieve(zoneInfo);
		}
		for (ptrdiff_t i = 0; i < (ptrdiff_t)zoneInfo.Patchs.size(); ++i)
		{
			referenceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[0]);
			referenceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[1]);
			referenceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[2]);
			referenceVertices.push_back(zoneInfo.Patchs[i].Patch.Vertices[3]);
		}
	}
	printf("Source vertices: %i, reference vertices: %i\n", (int)sourceVertices.size(), (int)referenceVertices.size());
	if (sourceVertices.size() != referenceVertices.size())
	{
		nlwarning("Mismatching vertex count in zone %s, source vertices: %i, reference vertices: %i", zone.c_str(), (int)sourceVertices.size(), (int)referenceVertices.size());
		++s_Warnings;
	}

	std::vector<bool> processedSourceIndices;
	std::vector<bool> processedReferenceIndices;
	processedSourceIndices.resize(sourceVertices.size());
	processedReferenceIndices.resize(referenceVertices.size());

	int referenceProcessedCount = 0;
	int sourceProcessedCount = 0;

	for (ptrdiff_t i = 0; i < (ptrdiff_t)referenceVertices.size(); ++i)
	{
		if (processedReferenceIndices[i])
			continue;

		float referenceHeight = referenceVertices[i].z;
		int referenceHeightDiv = 1;
		++referenceProcessedCount;

		for (ptrdiff_t j = i + 1; j < (ptrdiff_t)referenceVertices.size(); ++j)
		{
			if (processedReferenceIndices[j])
				continue;

			if (abs(referenceVertices[i].x - referenceVertices[j].x) < 0.1f
				&& abs(referenceVertices[i].y - referenceVertices[j].y) < 0.1f)
			{
				processedReferenceIndices[j] = true;
				referenceHeight += referenceVertices[j].z;
				++referenceHeightDiv;
				++referenceProcessedCount;
			}
		}

		float sourceHeight = 0.f;
		int sourceHeightDiv = 0;

		for (ptrdiff_t j = 0; j < (ptrdiff_t)sourceVertices.size(); ++j)
		{
			if (processedSourceIndices[j])
				continue;

			if (abs(referenceVertices[i].x - sourceVertices[j].x) < 0.1f
				&& abs(referenceVertices[i].y - sourceVertices[j].y) < 0.1f)
			{
				processedSourceIndices[j] = true;
				sourceHeight += sourceVertices[j].z;
				++sourceHeightDiv;
				++sourceProcessedCount;
			}
		}

		if (!sourceHeightDiv)
		{
			nlwarning("No matching vertices in source for zone %s, x: %f, y: %f", zone.c_str(), referenceVertices[i].x, referenceVertices[i].y);
			++s_Warnings;
			continue;
		}

		if (referenceHeightDiv != sourceHeightDiv)
		{
			nlwarning("Mismatching vertex count for zone %s, x: %f, y: %f (reference: %i, source: %i)", zone.c_str(), referenceVertices[i].x, referenceVertices[i].y, referenceHeightDiv, sourceHeightDiv);
			++s_Warnings;
			continue;
		}

		referenceHeight /= (float)referenceHeightDiv;
		sourceHeight /= (float)sourceHeightDiv;

		output.push_back(NLMISC::CVector(referenceVertices[i].x, referenceVertices[i].y, sourceHeight - referenceHeight));
	}

	return true;
}

bool unbuildElevation()
{
	std::vector<std::string> referenceZones;
	CPath::getPathContent(s_ReferenceDir, true, false, true, referenceZones);
	int totalZones = 0;

	std::vector<NLMISC::CVector> output;

	for (std::vector<std::string>::iterator it(referenceZones.begin()), end(referenceZones.end()); it != end; ++it)
	{
		if (CFile::getExtension(*it) != s_ReferenceExt)
			continue;

		std::string zone = CFile::getFilenameWithoutExtension(*it);
		std::string sourceZone = s_SourceDir + "/" + CFile::getFilenameWithoutExtension(*it) + "." + s_SourceExt;

		if (!CFile::fileExists(sourceZone))
			continue;

		if (zone == "137_JK") // Bad zone
			continue;

		printf("%s\n", nlUtf8ToMbcs(zone));
		++totalZones;

		if (!processZone(output, sourceZone, *it))
			return false;

		printf("\n");
	}

	printf("Total zones: %i\n", totalZones);

#if 1

	std::vector<bool> processedOutput;
	processedOutput.resize(output.size());
	std::vector<NLMISC::CVector> reducedOutput;
	for (ptrdiff_t i = 0; i < (ptrdiff_t)output.size(); ++i)
	{
		if (processedOutput[i])
			continue;

		CVector v = output[i];
		int div = 1;

		for (ptrdiff_t j = i + 1; j < (ptrdiff_t)output.size(); ++j)
		{
			if (processedOutput[j])
				continue;

			if (abs(output[i].x - output[j].x) < 0.1f
				&& abs(output[i].y - output[j].y) < 0.1f)
			{
				processedOutput[j] = true;
				v.z += output[j].z;
				++div;
			}
		}

		v.z /= (float)div;
		reducedOutput.push_back(v);
	}

	printf("Reduced vertex count from %i to %i\n", (int)output.size(), (int)reducedOutput.size());

#else

	std::vector<NLMISC::CVector> reducedOutput = output;

#endif

	{
		int w, h;
		getBitmapSize(w, h);

		FILE *fo = fopen(nlUtf8ToMbcs(s_OutputPy), "w");
		if (!fo)
			return false;

		fprintf(fo, "import naturalneighbor # https://github.com/innolitics/natural-neighbor-interpolation - pip3 install naturalneighbor\n");
		fprintf(fo, "import numpy as np\n");
		fprintf(fo, "import png # pip3 install pypng\n");
		fprintf(fo, "grid_ranges = [[0, 1, %i%s], [0, 1, %i%s], [-1, 1, 1j]]\n", w, "j", h, "j");

		fprintf(fo, "points = np.array([\n");

		for (ptrdiff_t i = 0; i < (ptrdiff_t)reducedOutput.size(); ++i)
		{
			float x, y;
			getBitmapCoord(x, y, reducedOutput[i].x, reducedOutput[i].y);
			fprintf(fo, "\t[ %f, %f, 0 ], # %f, %f\n", x, y, reducedOutput[i].x, reducedOutput[i].y);
		}

		fprintf(fo, "])\n");
		fprintf(fo, "\n");
		fprintf(fo, "values = np.array([\n");

		for (ptrdiff_t i = 0; i < (ptrdiff_t)reducedOutput.size(); ++i)
		{
			fprintf(fo, "\t%f,\n", reducedOutput[i].z);
		}

		fprintf(fo, "])\n");

		fprintf(fo, "nn_interpolated_values = naturalneighbor.griddata(points, values, grid_ranges)\n");
		fprintf(fo, "img = []\n");
		fprintf(fo, "for y in range(0, %i):\n", h);
		fprintf(fo, "\tline = []\n");
		fprintf(fo, "\tfor x in range(0, %i):\n", w);
		fprintf(fo, "\t\tline.append(np.round(nn_interpolated_values[x][y][0]).astype(int) + 127)\n");
		fprintf(fo, "\timg.append(line)\n");
		fprintf(fo, "with open('%s.png', 'wb') as f:\n", CFile::getFilenameWithoutExtension(s_OutputPy).c_str());
		fprintf(fo, "\tw = png.Writer(%i, %i, greyscale=True)\n", w, h);
		fprintf(fo, "\tw.write(f, img)\n");
		fflush(fo);
		fclose(fo);
	}

	printf("Warnings: %i\n", s_Warnings);
	return true;
}

bool unbuildElevation(NLMISC::CCmdArgs &args)
{
	s_OutputPy = args.getAdditionalArg("output")[0];
	s_SourceDir = args.getAdditionalArg("source")[0];
	s_ReferenceDir = args.getAdditionalArg("reference")[0];

	if (args.haveLongArg("zonemin") && args.haveLongArg("zonemax"))
	{
		sint32 zoneMinX, zoneMinY;
		sint32 zoneMaxX, zoneMaxY;
		if (!getXYFromZoneName(zoneMinX, zoneMinY, args.getLongArg("zonemin")[0])
			|| !getXYFromZoneName(zoneMaxX, zoneMaxY, args.getLongArg("zonemax")[0]))
		{
			return false;
		}
		s_ZoneMinX = min(zoneMinX, zoneMaxX);
		s_ZoneMaxX = max(zoneMinX, zoneMaxX);
		s_ZoneMinY = min(zoneMinY, zoneMaxY);
		s_ZoneMaxY = max(zoneMinY, zoneMaxY);
	}
	else if (args.haveLongArg("land"))
	{
		if (!loadLand(args.getLongArg("land")[0]))
			return false;
		s_ZoneMinX = s_Land.getMinX();
		s_ZoneMaxX = s_Land.getMaxX();
		s_ZoneMinY = s_Land.getMinY();
		s_ZoneMaxY = s_Land.getMaxY();
	}
	else
	{
		nlwarning("Must have either both 'zonemin' and 'zonemax', or 'land' specified");
		return false;
	}

	if (args.haveLongArg("cellsize"))
	{
		if (!NLMISC::fromString(args.getLongArg("cellsize")[0], s_CellSize))
			return false;
	}

	s_ExtendCoords = args.haveLongArg("extendcoords");

	if (args.haveLongArg("sourceext"))
	{
		s_SourceExt = args.getLongArg("sourceext")[0];
	}

	if (args.haveLongArg("referenceext"))
	{
		s_ReferenceExt = args.getLongArg("referenceext")[0];
	}

	return unbuildElevation();
}

} /* anonymous namespace */

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext myApplicationContext;

	NLMISC::CCmdArgs args;

	args.addAdditionalArg("output", "Output Python file path");
	args.addAdditionalArg("source", "Input folder with zones at the right height");
	args.addAdditionalArg("reference", "Input folder with zones at the wrong height");

	args.addArg("", "sourceext", "extension", "Source zone extension (default: zonel)");
	args.addArg("", "referenceext", "extension", "Reference zone extension (default: zonew)");

	args.addArg("", "land", "land", "Ligo land file (either specify this or the boundaries)");
	args.addArg("", "zonemin", "zone", "Zone boundary");
	args.addArg("", "zonemax", "zone", "Zone boundary");
	args.addArg("", "cellsize", "meters", "Zone cell size (default: 160)");
	args.addArg("", "extendcoords", "flag", "Extend coordinates to edge of bitmap pixels");

	if (!args.parse(argc, argv))
	{
		return EXIT_FAILURE;
	}

	if (!unbuildElevation(args))
	{
		args.displayHelp();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/* end of file */