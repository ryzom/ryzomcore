// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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






// ---------------------------------------------------------------------------

#include <vector>
#include <string>

#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/block_memory.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/app_context.h"

#include "nel/ligo/zone_region.h"

#include "nel/3d/scene_group.h"

// ---------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;

// ---------------------------------------------------------------------------
// Out a string to the stdout and log.log
void outString (const string &sText)
{
	createDebug ();
	InfoLog->displayRaw(sText.c_str());
	//printf ("%s", sText.c_str());
}

// ---------------------------------------------------------------------------
struct SExportOptions
{
	string	InputIGDir;
	string	OutputIGDir;
	float	CellSize;
	string	HeightMapFile1;
	float	ZFactor1;
	string	HeightMapFile2;
	float	ZFactor2;
	bool	ExtendCoords;
	string	LandFile;

	// -----------------------------------------------------------------------
	bool load (const string &sFilename)
	{
		FILE * f = fopen (sFilename.c_str(), "rt");
		if (f == NULL)
			return false;
		else 
			fclose (f);

		try
		{			
			CConfigFile cf;

			cf.load (sFilename);

			// Out
			CConfigFile::CVar &cvOutputIGDir = cf.getVar("OutputIGDir");
			OutputIGDir = cvOutputIGDir.asString();

			// In
			CConfigFile::CVar &cvInputIGDir = cf.getVar("InputIGDir");
			InputIGDir = cvInputIGDir.asString();

			CConfigFile::CVar &cvCellSize = cf.getVar("CellSize");
			CellSize = cvCellSize.asFloat();

			CConfigFile::CVar &cvHeightMapFile1 = cf.getVar("HeightMapFile1");
			HeightMapFile1 = cvHeightMapFile1.asString();

			CConfigFile::CVar &cvZFactor1 = cf.getVar("ZFactor1");
			ZFactor1 = cvZFactor1.asFloat();

			CConfigFile::CVar &cvHeightMapFile2 = cf.getVar("HeightMapFile2");
			HeightMapFile2 = cvHeightMapFile2.asString();

			CConfigFile::CVar &cvZFactor2 = cf.getVar("ZFactor2");
			ZFactor2 = cvZFactor2.asFloat();

			CConfigFile::CVar &cvExtendCoords = cf.getVar("ExtendCoords");
			ExtendCoords = cvExtendCoords.asFloat();

			CConfigFile::CVar &cvLandFile = cf.getVar("LandFile");
			LandFile = cvLandFile.asString();
		}
		catch (const EConfigFile &e)
		{
			string sTmp = string("ERROR : Error in config file : ") + e.what() + "\n";
			outString (sTmp);
			return false;
		}
		return true;
	}
};


struct CZoneLimits
{
	sint32 _ZoneMinX;
	sint32 _ZoneMaxX;
	sint32 _ZoneMinY;
	sint32 _ZoneMaxY;
};

// ---------------------------------------------------------------------------
CZoneRegion *loadLand (const string &filename)
{
	CZoneRegion *ZoneRegion = NULL;
	try
	{
		CIFile fileIn;
		if (fileIn.open (filename))
		{
			// Xml
			CIXml xml (true);
			nlverify (xml.init (fileIn));

			ZoneRegion = new CZoneRegion;
			ZoneRegion->serial (xml);
		}
		else
		{
			outString (toString("Can't open the land files: %s", filename.c_str()));
		}
	}
	catch (const Exception& e)
	{
		outString(toString("Error in land file: %s", e.what()));
	}
	return ZoneRegion;
}


// ***************************************************************************
CInstanceGroup* LoadInstanceGroup (const std::string &sFilename)
{
	CIFile file;
	CInstanceGroup *newIG = new CInstanceGroup;

	if( file.open( sFilename ) )
	{
		try
		{
			newIG->serial (file);
		}
		catch (const Exception &)
		{
			// Cannot save the file
			delete newIG;
			return NULL;
		}
	}
	else
	{
		delete newIG;
		return NULL;
	}
	return newIG;
}

// ***************************************************************************
void SaveInstanceGroup (const std::string  &sFilename, CInstanceGroup *pIG)
{
	COFile file;

	if( file.open( sFilename ) )
	{
		try
		{
			pIG->serial (file);
		}
		catch (const Exception &e)
		{
			outString(e.what());
		}
	}
	else
	{
		outString(toString("Couldn't create %s", sFilename.c_str()));
	}
}

/** Get the Z of the height map at the given position
 */
static float  getHeightMapZ(float x, float y, const CZoneLimits &zl, const SExportOptions &options, CBitmap *heightMap1, CBitmap *heightMap2)
{
	float deltaZ = 0.0f, deltaZ2 = 0.0f;
	CRGBAF color;
	sint32 SizeX = zl._ZoneMaxX - zl._ZoneMinX + 1;
	sint32 SizeY = zl._ZoneMaxY - zl._ZoneMinY + 1;

	if (heightMap1 != NULL)
	{
		float xc = (x - options.CellSize * zl._ZoneMinX) / (options.CellSize * SizeX);
		float yc = 1.0f - ((y - options.CellSize * zl._ZoneMinY) / (options.CellSize * SizeY));
		if (options.ExtendCoords)
		{
			uint32 w = heightMap1->getWidth(), h = heightMap1->getHeight();
			xc -= .5f / (float)w;
			yc -= .5f / (float)h;
			xc = xc * (float)(w + 1) / (float)w;
			yc = yc * (float)(h + 1) / (float)h;
		}
		color = heightMap1->getColor(xc, yc);
		color *= 255.f;
		deltaZ = color.A;
		deltaZ = deltaZ - 127.0f; // Median intensity is 127
		deltaZ *= options.ZFactor1;
	}

	if (heightMap2 != NULL)
	{
		float xc = (x - options.CellSize * zl._ZoneMinX) / (options.CellSize * SizeX);
		float yc = 1.0f - ((y - options.CellSize * zl._ZoneMinY) / (options.CellSize * SizeY));
		if (options.ExtendCoords)
		{
			uint32 w = heightMap2->getWidth(), h = heightMap2->getHeight();
			xc -= .5f / (float)w;
			yc -= .5f / (float)h;
			xc = xc * (float)(w + 1) / (float)w;
			yc = yc * (float)(h + 1) / (float)h;
		}
		color = heightMap2->getColor(xc, yc);
		color *= 255.f;
		deltaZ2 = color.A;
		deltaZ2 = deltaZ2 - 127.0f; // Median intensity is 127
		deltaZ2 *= options.ZFactor2;
	}

	return deltaZ + deltaZ2;	
}

// ---------------------------------------------------------------------------
int main(int nNbArg, char**ppArgs)
{
	if (!NLMISC::INelContext::isContextInitialised())
		new CApplicationContext();

	NL3D_BlockMemoryAssertOnPurge = false;
	std::string sCurDir = CPath::getCurrentPath();

	if (nNbArg != 2)
	{
		printf ("Use : ig_elevation configfile.cfg\n");
		printf ("\nExample of config.cfg\n\n");
		printf ("InputIGDir = \"ig_land_max\";\n");
		printf ("OutputIGDir = \"ig_land_max_elev\";\n");
		printf ("CellSize = 160.0;\n");
		printf ("HeightMapFile1 = \"R:/graphics/landscape/ligo/jungle/big.tga\";\n");
		printf ("ZFactor1 = 1.0;\n");
		printf ("HeightMapFile2 = \"R:/graphics/landscape/ligo/jungle/noise.tga\";\n");
		printf ("ZFactor2 = 0.5;\n");
		printf ("ExtendCoords = 0;\n");
		printf ("LandFile = \"w:/matis.land\";\n");

		return EXIT_FAILURE;
	}

	SExportOptions options;
	if (!options.load(ppArgs[1]))
	{
		return EXIT_FAILURE;
	}

	// Get all ig files in the input directory and elevate to the z of the double heightmap

	// Load the land
	CZoneRegion *ZoneRegion = loadLand(options.LandFile);

	CZoneLimits zl;
	if (ZoneRegion)
	{
		zl._ZoneMinX = ZoneRegion->getMinX() < 0	? 0		: ZoneRegion->getMinX();
		zl._ZoneMaxX = ZoneRegion->getMaxX() > 255	? 255	: ZoneRegion->getMaxX();
		zl._ZoneMinY = ZoneRegion->getMinY() > 0	? 0		: ZoneRegion->getMinY();
		zl._ZoneMaxY = ZoneRegion->getMaxY() < -255 ? -255	: ZoneRegion->getMaxY();
	}
	else
	{
		nlwarning("A ligo .land file cannot be found");
		zl._ZoneMinX = 0;
		zl._ZoneMaxX = 255;
		zl._ZoneMinY = 0;
		zl._ZoneMaxY = 255;
	}

	// Load the 2 height maps
	CBitmap *HeightMap1 = NULL;
	if (!options.HeightMapFile1.empty())
	{
		HeightMap1 = new CBitmap;
		try 
		{
			CIFile inFile;
			if (inFile.open(options.HeightMapFile1))
			{
				HeightMap1->load (inFile);
			}
			else
			{
				outString(toString("Couldn't not open %s: heightmap 1 map ignored", options.HeightMapFile1.c_str()));
				delete HeightMap1;
				HeightMap1 = NULL;
			}
		}
		catch (const Exception &e)
		{
			outString(toString("Cant load height map : %s : %s", options.HeightMapFile1.c_str(), e.what()));
			delete HeightMap1;
			HeightMap1 = NULL;
		}
	}
	CBitmap *HeightMap2 = NULL;
	if (!options.HeightMapFile2.empty())
	{
		HeightMap2 = new CBitmap;
		try 
		{
			CIFile inFile;
			if (inFile.open(options.HeightMapFile2))
			{
				HeightMap2->load (inFile);
			}
			else
			{
				outString(toString("Couldn't not open %s: heightmap 2 map ignored", options.HeightMapFile2.c_str()));
				delete HeightMap2;
				HeightMap2 = NULL;
			}
		}
		catch (const Exception &e)
		{
			outString (string("Cant load height map : ") + options.HeightMapFile2 + " : " + e.what() + "\n");
			delete HeightMap2;
			HeightMap1 = NULL;
		}
	}

	// get all files
	vector<string> vAllFilesUnfiltered;
	CPath::getPathContent(options.InputIGDir, false, false, true, vAllFilesUnfiltered);

	// keep only .ig files
	vector<string> vAllFiles;
	for(uint i = 0, len = (uint)vAllFilesUnfiltered.size(); i < len; ++i)
	{
		if (toLowerAscii(CFile::getExtension(vAllFilesUnfiltered[i])) == "ig")
		{
			vAllFiles.push_back(vAllFilesUnfiltered[i]);
		}
	}

	for (uint32 i = 0; i < vAllFiles.size(); ++i)
	{
		CInstanceGroup *pIG = LoadInstanceGroup (vAllFiles[i]);

		if (pIG != NULL)
		{
			bool realTimeSunContribution = pIG->getRealTimeSunContribution();
			// For all instances !!!
			CVector vGlobalPos;
			CInstanceGroup::TInstanceArray IA;
			vector<CCluster> Clusters;
			vector<CPortal> Portals;
			vector<CPointLightNamed> PLN;
			pIG->retrieve (vGlobalPos, IA, Clusters, Portals, PLN);

			if (IA.empty() && PLN.empty() && Portals.empty() && Clusters.empty()) continue;


			uint k;

			// elevate instance
			for(k = 0; k < IA.size(); ++k)
			{
				CVector instancePos = vGlobalPos + IA[k].Pos;
				IA[k].Pos.z += getHeightMapZ(instancePos.x, instancePos.y, zl, options, HeightMap1, HeightMap2);
			}

			// lights
			for(k = 0; k < PLN.size(); ++k)
			{
				CVector lightPos = vGlobalPos + PLN[k].getPosition();
				PLN[k].setPosition( PLN[k].getPosition() + getHeightMapZ(lightPos.x, lightPos.y, zl, options, HeightMap1, HeightMap2) * CVector::K);
			}

			// portals
			std::vector<CVector> portal;
			for(k = 0; k < Portals.size(); ++k)
			{
				Portals[k].getPoly(portal);
				if (portal.empty())
				{
					nlwarning("Empty portal found");
					continue;
				}
				// compute mean position of the poly
				CVector meanPos(0, 0, 0);
				uint l;
				for(l = 0; l < portal.size(); ++l)
					meanPos += portal[l];
				meanPos /= (float) portal.size();
				meanPos += vGlobalPos;
				float z = getHeightMapZ(meanPos.x, meanPos.y, zl, options, HeightMap1, HeightMap2);
				for(l = 0; l < portal.size(); ++l)
				{
					portal[l].z += z;
				}
				Portals[k].setPoly(portal);
			}

			// clusters
			std::vector<CPlane> volume;
			CMatrix transMatrix;
			for(k = 0; k < Clusters.size(); ++k)
			{
				CVector clusterPos = vGlobalPos + Clusters[k].getBBox().getCenter();
				float z = getHeightMapZ(clusterPos.x, clusterPos.y, zl, options, HeightMap1, HeightMap2);
				transMatrix.setPos(z * CVector::K);
				Clusters[k].applyMatrix(transMatrix);
			}

			CInstanceGroup *pIGout = new CInstanceGroup;
			pIGout->build (vGlobalPos, IA, Clusters, Portals, PLN);
			pIGout->enableRealTimeSunContribution(realTimeSunContribution);

			std::string outFilePath = CPath::standardizePath(options.OutputIGDir, true) + CFile::getFilename(vAllFiles[i]);
			nldebug("Writing %s...", outFilePath.c_str());
			SaveInstanceGroup(outFilePath, pIGout);

			delete pIG;
		}
	}

	return EXIT_SUCCESS;
}
