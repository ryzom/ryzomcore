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

#include "export.h"
#include "formFlora.h"
#include "formPlant.h"
#ifdef NL_OS_WINDOWS
#define NOMINMAX
#include <windows.h>
#endif // NL_OS_WINDOWS

#include "nel/ligo/zone_region.h"
#include "nel/ligo/primitive.h"

#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

#include "nel/3d/zone.h"
#include "nel/3d/landscape.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/visual_collision_manager.h"
#include "nel/3d/visual_collision_entity.h"

#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "tools.h"
#include "../master/ContinentCfg.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLLIGO;
using namespace NLGEORGES;

#define MAX_SYS_DIR 6
const char *gExportSysDir[MAX_SYS_DIR] =
{
	".",
	"..",
	"ZoneBitmaps",
	"ZoneLigos",
	"dfn",
	"tmp"
};


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------
// Segment line intersection P1P2 and P3P4
bool CExport::segmentIntersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double denominator = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
	if( denominator == 0.0 )
		return false; // The segment are colinear
	double k = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	k = ( (x2-x1)*(y1-y3) - (y2-y1)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	return true;
}

// ---------------------------------------------------------------------------
void CExport::delIGZone (sint32 x, sint32 y)
{
	string sZoneName = CExport::getZoneNameFromXY (x, y);
	sZoneName += ".ig";
	if (CFile::deleteFile(sZoneName))
	{
		CTools::chdir (_ExeDir);
		string sTmp = string("  zone ") + sZoneName + " deleted";
		if (_ExportCB != NULL)
			_ExportCB->dispInfo(sTmp);
	}
}

// ---------------------------------------------------------------------------
void CExport::delAllIGZoneUnderPoint (float fCellSize, CPrimPoint *pPoint, const string &sIGOutputDir)
{
	if (pPoint == NULL) return;
	sint32 nX, nY;
	CTools::chdir (sIGOutputDir);
	nX = (sint32) floor (pPoint->Point.x / fCellSize);
	nY = (sint32) floor (pPoint->Point.y / fCellSize);
	delIGZone (nX, nY);
	CTools::chdir (sIGOutputDir);
}

// ---------------------------------------------------------------------------
void CExport::delAllIGZoneUnderPath (float fCellSize, CPrimPath *pPath, const string &sIGOutputDir)
{
	if (pPath == NULL) return;
	if (pPath->VPoints.size() == 0) return;
	uint32 i, j;
	CVector vMin, vMax;

	CTools::chdir (sIGOutputDir);
	vMin = vMax = pPath->VPoints[0];
	for (i = 0; i < pPath->VPoints.size(); ++i)
	{
		if (vMin.x > pPath->VPoints[i].x) vMin.x = pPath->VPoints[i].x;
		if (vMin.y > pPath->VPoints[i].y) vMin.y = pPath->VPoints[i].y;
		if (vMin.z > pPath->VPoints[i].z) vMin.z = pPath->VPoints[i].z;
		if (vMax.x < pPath->VPoints[i].x) vMax.x = pPath->VPoints[i].x;
		if (vMax.y < pPath->VPoints[i].y) vMax.y = pPath->VPoints[i].y;
		if (vMax.z < pPath->VPoints[i].z) vMax.z = pPath->VPoints[i].z;
	}

	sint32 x, y;
	sint32 nMinX, nMinY, nMaxX, nMaxY;
	nMinX = (sint32) floor (vMin.x / fCellSize);
	nMinY = (sint32) floor (vMin.y / fCellSize);
	nMaxX = (sint32) floor (vMax.x / fCellSize);
	nMaxY = (sint32) floor (vMax.y / fCellSize);

	for (x = nMinX; x <= nMaxX; ++x)
	for (y = nMinY; y <= nMaxY; ++y)
	{
		// Does the zone (x,y) is under the patah ?

		CVector vSquare[4];
		vSquare[0].x = x * fCellSize;
		vSquare[0].y = y * fCellSize;
		vSquare[0].z = 0.0f;

		vSquare[1].x = (x+1) * fCellSize;
		vSquare[1].y = y * fCellSize;
		vSquare[1].z = 0.0f;

		vSquare[2].x = (x+1) * fCellSize;
		vSquare[2].y = (y+1) * fCellSize;
		vSquare[2].z = 0.0f;

		vSquare[3].x = x * fCellSize;
		vSquare[3].y = (y+1) * fCellSize;
		vSquare[3].z = 0.0f;

		// Is a vertex of the path inside the zone ?
		for (i = 0; i < pPath->VPoints.size(); ++i)
		{
			if ((pPath->VPoints[i].x >= (x*fCellSize)) &&
				(pPath->VPoints[i].x <= ((x+1)*fCellSize)) &&
				(pPath->VPoints[i].y <= (y*fCellSize)) &&
				(pPath->VPoints[i].y <= ((y+1)*fCellSize)))
			{
				delIGZone (x, y);
				CTools::chdir (sIGOutputDir);
			}
		}

		// Is an segment of the path cut an edge of the patat ?
		for (i = 0; i < pPath->VPoints.size()-1; ++i)
		for (j = 0; j < 4; ++j)
		{
			double x1 = vSquare[j].x;
			double y1 = vSquare[j].y;
			double x2 = vSquare[(j+1)%4].x;
			double y2 = vSquare[(j+1)%4].y;

			double x3 = pPath->VPoints[i].x;
			double y3 = pPath->VPoints[i].y;
			double x4 = pPath->VPoints[i+1].x;
			double y4 = pPath->VPoints[i+1].y;

			if (segmentIntersection(x1, y1, x2, y2, x3, y3, x4, y4))
			{
				delIGZone (x, y);
				CTools::chdir (sIGOutputDir);
			}
		}
	}
}

// ---------------------------------------------------------------------------
void CExport::delAllIGZoneUnderPatat (float fCellSize, CPrimZone *pPatat, const string &sIGOutputDir)
{
	if (pPatat == NULL) return;
	if (pPatat->VPoints.size() == 0) return;
	uint32 i, j;
	CVector vMin, vMax;

	CTools::chdir (sIGOutputDir);
	vMin = vMax = pPatat->VPoints[0];
	for (i = 0; i < pPatat->VPoints.size(); ++i)
	{
		if (vMin.x > pPatat->VPoints[i].x) vMin.x = pPatat->VPoints[i].x;
		if (vMin.y > pPatat->VPoints[i].y) vMin.y = pPatat->VPoints[i].y;
		if (vMin.z > pPatat->VPoints[i].z) vMin.z = pPatat->VPoints[i].z;
		if (vMax.x < pPatat->VPoints[i].x) vMax.x = pPatat->VPoints[i].x;
		if (vMax.y < pPatat->VPoints[i].y) vMax.y = pPatat->VPoints[i].y;
		if (vMax.z < pPatat->VPoints[i].z) vMax.z = pPatat->VPoints[i].z;
	}

	sint32 x, y;
	sint32 nMinX, nMinY, nMaxX, nMaxY;
	nMinX = (sint32) floor (vMin.x / fCellSize);
	nMinY = (sint32) floor (vMin.y / fCellSize);
	nMaxX = (sint32) floor (vMax.x / fCellSize);
	nMaxY = (sint32) floor (vMax.y / fCellSize);

	for (x = nMinX; x <= nMaxX; ++x)
	for (y = nMinY; y <= nMaxY; ++y)
	{
		// Does the zone (x,y) is under the patat ?

		// Is a vertex of the zone in the patat ?
		CVector vSquare[4];
		vSquare[0].x = x * fCellSize;
		vSquare[0].y = y * fCellSize;
		vSquare[0].z = 0.0f;

		vSquare[1].x = (x+1) * fCellSize;
		vSquare[1].y = y * fCellSize;
		vSquare[1].z = 0.0f;

		vSquare[2].x = (x+1) * fCellSize;
		vSquare[2].y = (y+1) * fCellSize;
		vSquare[2].z = 0.0f;

		vSquare[3].x = x * fCellSize;
		vSquare[3].y = (y+1) * fCellSize;
		vSquare[3].z = 0.0f;

		for (i = 0; i < 4; ++i)
		{
			if (pPatat->contains(vSquare[i]))
			{
				delIGZone (x, y);
				CTools::chdir (sIGOutputDir);
			}
		}

		// Is a vertex of the patat inside the zone ?
		for (i = 0; i < pPatat->VPoints.size(); ++i)
		{
			if ((pPatat->VPoints[i].x >= (x*fCellSize)) &&
				(pPatat->VPoints[i].x <= ((x+1)*fCellSize)) &&
				(pPatat->VPoints[i].y <= (y*fCellSize)) &&
				(pPatat->VPoints[i].y <= ((y+1)*fCellSize)))
			{
				delIGZone (x, y);
				CTools::chdir (sIGOutputDir);
			}
		}

		// Is an edge of the zone cut an edge of the patat ?
		for (i = 0; i < pPatat->VPoints.size(); ++i)
		for (j = 0; j < 4; ++j)
		{
			double x1 = vSquare[j].x;
			double y1 = vSquare[j].y;
			double x2 = vSquare[(j+1)%4].x;
			double y2 = vSquare[(j+1)%4].y;

			double x3 = pPatat->VPoints[i].x;
			double y3 = pPatat->VPoints[i].y;
			double x4 = pPatat->VPoints[(i+1)%pPatat->VPoints.size()].x;
			double y4 = pPatat->VPoints[(i+1)%pPatat->VPoints.size()].y;

			if (segmentIntersection(x1, y1, x2, y2, x3, y3, x4, y4))
			{
				delIGZone (x, y);
				CTools::chdir (sIGOutputDir);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// A patat needs an update if there is at least one zone under itself that is not present
bool CExport::isPatatNeedUpdate (float fCellSize, CPrimZone *pPatat, const string &sIGOutputDir)
{
	uint32 i, j;
	CVector vMin, vMax;

	CTools::chdir (sIGOutputDir);
	if (pPatat->VPoints.size() == 0)
		return false;
	vMin = vMax = pPatat->VPoints[0];
	for (i = 0; i < pPatat->VPoints.size(); ++i)
	{
		if (vMin.x > pPatat->VPoints[i].x) vMin.x = pPatat->VPoints[i].x;
		if (vMin.y > pPatat->VPoints[i].y) vMin.y = pPatat->VPoints[i].y;
		if (vMin.z > pPatat->VPoints[i].z) vMin.z = pPatat->VPoints[i].z;
		if (vMax.x < pPatat->VPoints[i].x) vMax.x = pPatat->VPoints[i].x;
		if (vMax.y < pPatat->VPoints[i].y) vMax.y = pPatat->VPoints[i].y;
		if (vMax.z < pPatat->VPoints[i].z) vMax.z = pPatat->VPoints[i].z;
	}

	sint32 x, y;
	sint32 nMinX, nMinY, nMaxX, nMaxY;
	nMinX = (sint32) floor (vMin.x / fCellSize);
	nMinY = (sint32) floor (vMin.y / fCellSize);
	nMaxX = (sint32) floor (vMax.x / fCellSize);
	nMaxY = (sint32) floor (vMax.y / fCellSize);

	for (x = nMinX; x <= nMaxX; ++x)
	for (y = nMinY; y <= nMaxY; ++y)
	{
		// Does the zone (x,y) is under the patat ?
		bool bZoneUnderPatat = false;
		// Is a vertex of the zone in the patat ?
		CVector vSquare[4];
		vSquare[0].x = x * fCellSize;
		vSquare[0].y = y * fCellSize;
		vSquare[0].z = 0.0f;

		vSquare[1].x = (x+1) * fCellSize;
		vSquare[1].y = y * fCellSize;
		vSquare[1].z = 0.0f;

		vSquare[2].x = (x+1) * fCellSize;
		vSquare[2].y = (y+1) * fCellSize;
		vSquare[2].z = 0.0f;

		vSquare[3].x = x * fCellSize;
		vSquare[3].y = (y+1) * fCellSize;
		vSquare[3].z = 0.0f;

		for (i = 0; i < 4; ++i)
		{
			if (pPatat->contains(vSquare[i]))
			{
				string sTmp = CExport::getZoneNameFromXY(x,y) + ".ig";
				if (!CTools::fileExist(sTmp)) // If the file does not exist
					return true; // need update
			}
		}

		// Is a vertex of the patat inside the zone ?
		for (i = 0; i < pPatat->VPoints.size(); ++i)
		{
			if ((pPatat->VPoints[i].x >= (x*fCellSize)) &&
				(pPatat->VPoints[i].x <= ((x+1)*fCellSize)) &&
				(pPatat->VPoints[i].y >= (y*fCellSize)) &&
				(pPatat->VPoints[i].y <= ((y+1)*fCellSize)))
			{
				string sTmp = CExport::getZoneNameFromXY(x,y) + ".ig";
				if (!CTools::fileExist(sTmp)) // If the file does not exist
					return true; // need update
			}
		}

		// Is an edge of the zone cut an edge of the patat ?
		for (i = 0; i < pPatat->VPoints.size(); ++i)
		for (j = 0; j < 4; ++j)
		{
			double x1 = vSquare[j].x;
			double y1 = vSquare[j].y;
			double x2 = vSquare[(j+1)%4].x;
			double y2 = vSquare[(j+1)%4].y;

			double x3 = pPatat->VPoints[i].x;
			double y3 = pPatat->VPoints[i].y;
			double x4 = pPatat->VPoints[(i+1)%pPatat->VPoints.size()].x;
			double y4 = pPatat->VPoints[(i+1)%pPatat->VPoints.size()].y;

			if (segmentIntersection(x1, y1, x2, y2, x3, y3, x4, y4))
			{
				string sTmp = CExport::getZoneNameFromXY (x, y) + ".ig";
				if (!CTools::fileExist(sTmp)) // If the file does not exist
					return true; // need update
			}
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// A path needs an update if there is at least one zone under itself that is not present
bool CExport::isPathNeedUpdate (float fCellSize, CPrimPath *pPath, const string &sIGOutputDir)
{
	uint32 i, j;
	CVector vMin, vMax;

	CTools::chdir (sIGOutputDir);
	if (pPath->VPoints.size() == 0)
		return false;
	vMin = vMax = pPath->VPoints[0];
	for (i = 0; i < pPath->VPoints.size(); ++i)
	{
		if (vMin.x > pPath->VPoints[i].x) vMin.x = pPath->VPoints[i].x;
		if (vMin.y > pPath->VPoints[i].y) vMin.y = pPath->VPoints[i].y;
		if (vMin.z > pPath->VPoints[i].z) vMin.z = pPath->VPoints[i].z;
		if (vMax.x < pPath->VPoints[i].x) vMax.x = pPath->VPoints[i].x;
		if (vMax.y < pPath->VPoints[i].y) vMax.y = pPath->VPoints[i].y;
		if (vMax.z < pPath->VPoints[i].z) vMax.z = pPath->VPoints[i].z;
	}

	sint32 x, y;
	sint32 nMinX, nMinY, nMaxX, nMaxY;
	nMinX = (sint32) floor (vMin.x / fCellSize);
	nMinY = (sint32) floor (vMin.y / fCellSize);
	nMaxX = (sint32) floor (vMax.x / fCellSize);
	nMaxY = (sint32) floor (vMax.y / fCellSize);

	for (x = nMinX; x <= nMaxX; ++x)
	for (y = nMinY; y <= nMaxY; ++y)
	{
		// Does the zone (x,y) is under the patat ?
		bool bZoneUnderPatat = false;
		// Is a vertex of the zone in the patat ?
		CVector vSquare[4];
		vSquare[0].x = x * fCellSize;
		vSquare[0].y = y * fCellSize;
		vSquare[0].z = 0.0f;

		vSquare[1].x = (x+1) * fCellSize;
		vSquare[1].y = y * fCellSize;
		vSquare[1].z = 0.0f;

		vSquare[2].x = (x+1) * fCellSize;
		vSquare[2].y = (y+1) * fCellSize;
		vSquare[2].z = 0.0f;

		vSquare[3].x = x * fCellSize;
		vSquare[3].y = (y+1) * fCellSize;
		vSquare[3].z = 0.0f;

		// Is a vertex of the path inside the zone ?
		for (i = 0; i < pPath->VPoints.size(); ++i)
		{
			if ((pPath->VPoints[i].x >= (x*fCellSize)) &&
				(pPath->VPoints[i].x <= ((x+1)*fCellSize)) &&
				(pPath->VPoints[i].y >= (y*fCellSize)) &&
				(pPath->VPoints[i].y <= ((y+1)*fCellSize)))
			{
				string sTmp = CExport::getZoneNameFromXY(x,y) + ".ig";
				if (!CTools::fileExist(sTmp)) // If the file does not exist
					return true; // need update
			}
		}

		// Is an edge of the zone cut an edge of the patat ?
		for (i = 0; i < (pPath->VPoints.size()-1); ++i)
		for (j = 0; j < 4; ++j)
		{
			double x1 = vSquare[j].x;
			double y1 = vSquare[j].y;
			double x2 = vSquare[(j+1)%4].x;
			double y2 = vSquare[(j+1)%4].y;

			double x3 = pPath->VPoints[i].x;
			double y3 = pPath->VPoints[i].y;
			double x4 = pPath->VPoints[i+1].x;
			double y4 = pPath->VPoints[i+1].y;

			if (segmentIntersection(x1, y1, x2, y2, x3, y3, x4, y4))
			{
				string sTmp = CExport::getZoneNameFromXY (x, y) + ".ig";
				if (!CTools::fileExist(sTmp)) // If the file does not exist
					return true; // need update
			}
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// A path needs an update if there is at least one zone under itself that is not present
bool CExport::isPointNeedUpdate (float fCellSize, CPrimPoint *pPoint, const string &sIGOutputDir)
{
	CTools::chdir (sIGOutputDir);

	sint32 x, y;
	x = (sint32) floor (pPoint->Point.x / fCellSize);
	y = (sint32) floor (pPoint->Point.y / fCellSize);

	// Does the zone (x,y) is under the patat ?
	bool bZoneUnderPatat = false;
	// Is a vertex of the zone in the patat ?
	CVector vSquare[4];
	vSquare[0].x = x * fCellSize;
	vSquare[0].y = y * fCellSize;
	vSquare[0].z = 0.0f;

	vSquare[1].x = (x+1) * fCellSize;
	vSquare[1].y = y * fCellSize;
	vSquare[1].z = 0.0f;

	vSquare[2].x = (x+1) * fCellSize;
	vSquare[2].y = (y+1) * fCellSize;
	vSquare[2].z = 0.0f;

	vSquare[3].x = x * fCellSize;
	vSquare[3].y = (y+1) * fCellSize;
	vSquare[3].z = 0.0f;

	// Is the vertex inside the zone ?
	if ((pPoint->Point.x >= (x*fCellSize)) &&
		(pPoint->Point.x <= ((x+1)*fCellSize)) &&
		(pPoint->Point.y >= (y*fCellSize)) &&
		(pPoint->Point.y <= ((y+1)*fCellSize)))
	{
		string sTmp = CExport::getZoneNameFromXY(x,y) + ".ig";
		if (!CTools::fileExist(sTmp)) // If the file does not exist
			return true; // need update
	}

	return false;
}

// ---------------------------------------------------------------------------
// SExportOptions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
SExportOptions::SExportOptions ()
{
	CellSize = 160.0f;
}

// ---------------------------------------------------------------------------
bool SExportOptions::loadcf (CConfigFile &cf)
{
	// Out
	CConfigFile::CVar &cvOutIGDir = cf.getVar("EXP_OutIGDir");
	OutIGDir = cvOutIGDir.asString();

	// In
	CConfigFile::CVar &cvInLandscapeDir = cf.getVar("EXP_ZoneWDir");
	InLandscapeDir = cvInLandscapeDir.asString();

	CConfigFile::CVar &cvLandBankFile = cf.getVar("EXP_SmallBank");
	LandBankFile = cvLandBankFile.asString();
	CConfigFile::CVar &cvLandFarBankFile = cf.getVar("EXP_FarBank");
	LandFarBankFile = cvLandFarBankFile.asString();
	CConfigFile::CVar &cvLandTileNoiseDir = cf.getVar("EXP_DisplaceDir");
	LandTileNoiseDir = cvLandTileNoiseDir.asString();

	CConfigFile::CVar &cvCellSize = cf.getVar("EXP_CellSize");
	CellSize = cvCellSize.asFloat();

	CConfigFile::CVar &cvPrimFloraDir = cf.getVar("EXP_PrimFloraDir");
	PrimFloraDir = cvPrimFloraDir.asString();

	return true;
}

// ---------------------------------------------------------------------------
bool SExportOptions::save (FILE *f)
{
	fprintf (f,"\n// Export Options\n");
	fprintf (f, "EXP_OutIGDir = \"%s\";\n",		OutIGDir);
	fprintf (f, "EXP_ZoneWDir = \"%s\";\n",		InLandscapeDir);
	fprintf (f, "EXP_SmallBank = \"%s\";\n",	LandBankFile);
	fprintf (f, "EXP_FarBank = \"%s\";\n",		LandFarBankFile);
	fprintf (f, "EXP_DisplaceDir = \"%s\";\n",	LandTileNoiseDir);
	fprintf (f, "EXP_CellSize = %f;\n",			CellSize);
	fprintf (f, "EXP_PrimFloraDir = \"%s\";\n", PrimFloraDir);
	return true;
}

// ---------------------------------------------------------------------------
// SExportPrimitive
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
bool SExportPrimitive::operator == (const SExportPrimitive &rRightArg)
{
	if (strcmp(this->FullPrimName.c_str(), rRightArg.FullPrimName.c_str()) == 0)
	if (strcmp(this->PrimitiveName.c_str(), rRightArg.PrimitiveName.c_str()) == 0)
		return true;
	return false;
}

// ---------------------------------------------------------------------------
// CExport
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
CExport::CExport()
{
	_Landscape = NULL;
	_VCM = NULL;
	_VCE = NULL;
	_ExportCB = NULL;
	_Options = NULL;
}

// ---------------------------------------------------------------------------
CExport::~CExport()
{
	if (_Landscape != NULL)
		delete _Landscape;
	if (_VCM != NULL)
	{
		_VCM->deleteEntity (_VCE);
		delete _VCM;
	}
}



// ---------------------------------------------------------------------------
bool CExport::newExport (SExportOptions &opt, IExportCB *expCB)
{
	_ExeDir = CTools::pwd();
	_Options = &opt;
	_ExportCB = expCB;

	string sTmp;
	uint32 i, j, k, m;



	// First of all read the CFG
	string sContinentDir = _Options->PrimFloraDir;

	sContinentDir = CTools::normalizePath (sContinentDir);
	CTools::chdir (sContinentDir.c_str()); // Relative to absolute path
	sContinentDir = CTools::pwd () + "\\";
	CTools::chdir (_ExeDir);

	// Read continent.cfg
	SContinentCfg ContinentCFG;
	{
		string sTmp = sContinentDir + "continent.cfg";
		if (!ContinentCFG.load(sTmp.c_str()))
			throw Exception("Cannot load continent.cfg");
	}

	// Ok set parameters from options first and with CFG if no options set

	if (_Options->OutIGDir == "")
		_OutIGDir = CTools::normalizePath (ContinentCFG.OutIGDir);
	else
		_OutIGDir = CTools::normalizePath (_Options->OutIGDir);

	if (_Options->LandFile == "")
		_LandFile			= ContinentCFG.LandFile;
	else
		_LandFile			= _Options->LandFile;

	if (_Options->DfnDir == "")
		_DfnDir				= ContinentCFG.DfnDir;
	else
		_DfnDir				= _Options->DfnDir;

	if (_Options->GameElemDir == "")
		_GameElemDir		= ContinentCFG.GameElemDir;
	else
		_GameElemDir		= _Options->GameElemDir;

	if (_Options->InLandscapeDir == "")
		_InLandscapeDir		= ContinentCFG.LandZoneWDir;		// Directory where to get .zonew files
	else
		_InLandscapeDir		= _Options->InLandscapeDir;

	if (_Options->LandFarBankFile == "")
		_LandBankFile		= ContinentCFG.LandBankFile;		// The .smallbank file associated with the landscape
	else
		_LandBankFile		= _Options->LandBankFile;

	if (_Options->LandFarBankFile == "")
		_LandFarBankFile	= ContinentCFG.LandFarBankFile;		// The .farbank file
	else
		_LandFarBankFile	= _Options->LandFarBankFile;

	if (_Options->LandTileNoiseDir == "")
		_LandTileNoiseDir	= ContinentCFG.LandTileNoiseDir;	// Directory where to get displacement map
	else
		_LandTileNoiseDir	= _Options->LandTileNoiseDir;


	// Now create output directory
	CTools::mkdir (_OutIGDir);

	CTools::chdir (_OutIGDir.c_str());
	_OutIGDir = CTools::pwd () + "\\";
	CTools::chdir (_ExeDir);


	CPath::addSearchPath (_DfnDir, true, true);
	CPath::addSearchPath (_GameElemDir, true, true);
	CPath::addSearchPath (_LandTileNoiseDir, true, true);


	// Get all regions
	vector<string> vRegions;
	WIN32_FIND_DATA findData;
	HANDLE hFind;

	CTools::chdir (sContinentDir);
	hFind = FindFirstFile ("*.*", &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (GetFileAttributes(findData.cFileName)&FILE_ATTRIBUTE_DIRECTORY)
		{
			// Look if the name is a system directory
			bool bFound = false;
			for (i = 0; i < MAX_SYS_DIR; ++i)
				if (stricmp (findData.cFileName, gExportSysDir[i]) == 0)
				{
					bFound = true;
					break;
				}
			if (!bFound) // No, ok lets recurse it
			{
				vRegions.push_back (findData.cFileName);
			}
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);


	// Process all regions
	uint32 nRegion;
	for (nRegion = 0; nRegion < vRegions.size(); ++nRegion)
	{
		CTools::chdir (sContinentDir + vRegions[nRegion]);

		// Process if a flora has been modified
		vector<SFloraToUpdate> vFloraToUpdate;
		uint32 nFloraFile;

		// Fill the structure for update
		{
			// Get all new flora of the region
			vector<string> vFloraFiles;
			CTools::dirSub ("*.flora", vFloraFiles, true);
			for (nFloraFile = 0; nFloraFile < vFloraFiles.size(); ++nFloraFile)
			{
				// Compare the date with the old file stored in the IG output directory
				CTools::chdir (sContinentDir + vRegions[nRegion]);
				string sBaseName = vFloraFiles[nFloraFile].substr(sContinentDir.size());
				sBaseName = sBaseName.substr(0,sBaseName.rfind('\\'));
				for (i = 0; i < sBaseName.size(); ++i)
					if (sBaseName[i] == '\\')
						sBaseName[i] = '-';
				sBaseName += '-';
				string sOldFloraName = _OutIGDir + sBaseName + vFloraFiles[nFloraFile].substr(vFloraFiles[nFloraFile].rfind('\\')+1);

				if (CTools::fileExist(sOldFloraName))
				{
					if (CTools::fileDateCmp(vFloraFiles[nFloraFile], sOldFloraName) > 0)
					{
						// Delete zones from the 2 files
						SFloraToUpdate ftuTmp;
						// Delete zones from the newest file
						ftuTmp.FloraFile = vFloraFiles[nFloraFile];
						CTools::chdir (vFloraFiles[nFloraFile].substr(0,vFloraFiles[nFloraFile].rfind('\\')));
						CTools::dir ("*.prim", ftuTmp.PrimFile, true);
						vFloraToUpdate.push_back (ftuTmp);
						// Delete zones from the oldest file
						CTools::chdir (_OutIGDir);
						ftuTmp.FloraFile = sOldFloraName;
						CTools::dir (sBaseName + "*.prim", ftuTmp.PrimFile, true);
						vFloraToUpdate.push_back (ftuTmp);
						CTools::chdir (_ExeDir);
						sTmp = vFloraFiles[nFloraFile] + " has been modified.";
						if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
					}
					else
					{
						CTools::chdir (_ExeDir);
						sTmp = vFloraFiles[nFloraFile] + string(" up to date.");
						if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
					}
				}
				else
				{
					// Delete zone from newest file
					SFloraToUpdate ftuTmp;
					ftuTmp.FloraFile = vFloraFiles[nFloraFile];
					CTools::chdir (vFloraFiles[nFloraFile].substr(0,vFloraFiles[nFloraFile].rfind('\\')));
					CTools::dir ("*.prim", ftuTmp.PrimFile, true);
					vFloraToUpdate.push_back (ftuTmp);
					CTools::chdir (_ExeDir);
					sTmp = vFloraFiles[nFloraFile] + " not generated.";
					if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
				}
			}

			// Get all old flora of the current region
			CTools::chdir (_OutIGDir);
			CTools::dir (vRegions[nRegion] + "-*.flora", vFloraFiles, true);
			for (nFloraFile = 0; nFloraFile < vFloraFiles.size(); ++nFloraFile)
			{
				CTools::chdir (_OutIGDir);
				// if the old flora file still exist but the new one not -> delete zone for the oldest file
				string sNewFloraName = vFloraFiles[nFloraFile].substr(vFloraFiles[nFloraFile].rfind('\\')+1);
				string sBaseName = sNewFloraName.substr (0, vFloraFiles[nFloraFile].rfind('-'));
				for (i = 0; i < sNewFloraName.size(); ++i)
					if (sNewFloraName[i] == '-')
						sNewFloraName[i] = '\\';

				sNewFloraName = sContinentDir + sNewFloraName;
				if (!CTools::fileExist(sNewFloraName))
				{
					SFloraToUpdate ftuTmp;
					// Delete zones from the oldest file
					ftuTmp.FloraFile = vFloraFiles[nFloraFile];
					CTools::dir (sBaseName + "*.prim", ftuTmp.PrimFile, true);
					vFloraToUpdate.push_back (ftuTmp);
					CTools::chdir (_ExeDir);
					sTmp = vFloraFiles[nFloraFile] + " not needed anymore.";
					if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
				}
			}
		}
		// End of fill the structure for update

		// Interpret the FloraToUpdate
		for (nFloraFile = 0; nFloraFile < vFloraToUpdate.size(); ++nFloraFile)
		{
			// Get all patats referenced by the flora and suppress zones under

			// Load the .flora file
			SFormFlora FormFlora;
			{
				// Get a loader
				UFormLoader *loader = UFormLoader::createLoader ();

				// Load the form
				CSmartPtr<UForm> form = loader->loadForm (vFloraToUpdate[nFloraFile].FloraFile.c_str ());
				if (form)
					FormFlora.build (form->getRootNode ());

				// Release the loader
				UFormLoader::releaseLoader (loader);
			}

			// Load all the .prim files that can be referenced by the flora
			vector<CPrimRegion> vPrimRegions;
			{
				vector<string> &rPrimFiles = vFloraToUpdate[nFloraFile].PrimFile;
				for (i = 0; i < rPrimFiles.size(); ++i)
				{
					CPrimRegion tmpPrimRegion;
					CIFile fileIn;
					fileIn.open (rPrimFiles[i]);
					CIXml input;
					input.init (fileIn);
					tmpPrimRegion.serial (input);
					vPrimRegions.push_back (tmpPrimRegion);
				}
			}

			// Delete zones under the old prims referenced by the old zone

			for (i = 0; i < FormFlora.IncludePatats.size(); ++i)
			{
				// Get the patat
				CPrimZone  *pPatat = NULL;
				CPrimPoint *pPoint = NULL;
				CPrimPath  *pPath  = NULL;
				for (j = 0; j < vPrimRegions.size(); ++j)
				{
					for (k = 0; k < vPrimRegions[j].VZones.size(); ++k)
					if (vPrimRegions[j].VZones[k].getName() == FormFlora.IncludePatats[i])
						pPatat = &vPrimRegions[j].VZones[k];
					for (k = 0; k < vPrimRegions[j].VPoints.size(); ++k)
					if (vPrimRegions[j].VPoints[k].getName() == FormFlora.IncludePatats[i])
						pPoint = &vPrimRegions[j].VPoints[k];
					for (k = 0; k < vPrimRegions[j].VPaths.size(); ++k)
					if (vPrimRegions[j].VPaths[k].getName() == FormFlora.IncludePatats[i])
						pPath = &vPrimRegions[j].VPaths[k];
				}

				if ((pPatat == NULL) && (pPoint == NULL) && (pPath == NULL))
				{
					CTools::chdir (_ExeDir);
					sTmp = "WARNING : Cannot find " + FormFlora.IncludePatats[i];
					if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
					continue;
				}

				if ((pPatat != NULL) && (pPatat->VPoints.size() <= 2))
				{
					CTools::chdir (_ExeDir);
					sTmp = "Patat " + pPatat->getName() + " has less than 3 points";
					if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
					continue;
				}

				if ((pPath != NULL) && (pPath->VPoints.size() < 2))
				{
					CTools::chdir (_ExeDir);
					sTmp = "Patat " + pPatat->getName() + " has less than 2 points";
					if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
					continue;
				}

				if (pPatat != NULL)
					delAllIGZoneUnderPatat (160.0f, pPatat, _OutIGDir);
				if (pPoint != NULL)
					delAllIGZoneUnderPoint (160.0f, pPoint, _OutIGDir);
				if (pPath != NULL)
					delAllIGZoneUnderPath (160.0f, pPath, _OutIGDir);
			}
		}
		// End of Process if a flora has been modified

		// Process if a prim has been modified
		// Fill the structure for update
		CTools::chdir (sContinentDir + vRegions[nRegion]);
		vector<SPrimToUpdate> vPrimToUpdate;
		{
			uint32 nPrimFile;
			vector<string> vPrimFiles;
			// Get all new prim of the region
			CTools::dirSub ("*.prim", vPrimFiles, true);
			for (nPrimFile = 0; nPrimFile < vPrimFiles.size(); ++nPrimFile)
			{
				CTools::chdir (sContinentDir + vRegions[nRegion]);
				// Compare the date with the old file stored in the IG output directory
				string sBaseName = vPrimFiles[nPrimFile].substr(sContinentDir.size());
				sBaseName = sBaseName.substr(0,sBaseName.rfind('\\'));
				for (i = 0; i < sBaseName.size(); ++i)
					if (sBaseName[i] == '\\')
						sBaseName[i] = '-';
				sBaseName += '-';
				string sOldPrimName = _OutIGDir + sBaseName + vPrimFiles[nPrimFile].substr(vPrimFiles[nPrimFile].rfind('\\')+1);

				if (CTools::fileExist(sOldPrimName))
				{
					if (CTools::fileDateCmp(vPrimFiles[nPrimFile], sOldPrimName) > 0)
					{
						// Delete zones from the 2 files
						SPrimToUpdate ptuTmp;
						// Delete zones from the newest file
						ptuTmp.PrimFile = vPrimFiles[nPrimFile];
						CTools::chdir (vPrimFiles[nPrimFile].substr(0,vPrimFiles[nPrimFile].rfind('\\')));
						CTools::dir ("*.flora", ptuTmp.FloraFile, true);
						vPrimToUpdate.push_back (ptuTmp);
						// Delete zones from the oldest file
						CTools::chdir (_OutIGDir);
						ptuTmp.PrimFile = sOldPrimName;
						CTools::dir (sBaseName + "*.flora", ptuTmp.FloraFile, true);
						vPrimToUpdate.push_back (ptuTmp);
						CTools::chdir (_ExeDir);
						sTmp = vPrimFiles[nPrimFile] + " has been modified.";
						if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
					}
					else
					{
						CTools::chdir (_ExeDir);
						sTmp = vPrimFiles[nPrimFile] + " up to date.";
						if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
					}
				}
				else
				{
					// Delete zone from newest file
					SPrimToUpdate ptuTmp;
					ptuTmp.PrimFile = vPrimFiles[nPrimFile];
					CTools::chdir (vPrimFiles[nPrimFile].substr(0,vPrimFiles[nPrimFile].rfind('\\')));
					CTools::dir ("*.flora", ptuTmp.FloraFile, true);
					vPrimToUpdate.push_back (ptuTmp);
					CTools::chdir (_ExeDir);
					sTmp = vPrimFiles[nPrimFile] + " not generated.";
					if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
				}
			}

			// Get all old prim of the current region
			CTools::chdir (_OutIGDir);
			CTools::dir (vRegions[nRegion] + "-*.prim", vPrimFiles, false);
			for (nPrimFile = 0; nPrimFile < vPrimFiles.size(); ++nPrimFile)
			{
				CTools::chdir (_OutIGDir);
				// if the old prim file still exist but the new one not -> delete zone for the oldest file
				string sNewPrimName = vPrimFiles[nPrimFile].substr(vPrimFiles[nPrimFile].rfind('\\')+1);
				string sBaseName = sNewPrimName.substr (0, vPrimFiles[nPrimFile].rfind('-'));
				for (i = 0; i < sNewPrimName.size(); ++i)
					if (sNewPrimName[i] == '-')
						sNewPrimName[i] = '\\';

				sNewPrimName = sContinentDir + sNewPrimName;
				if (!CTools::fileExist(sNewPrimName))
				{
					// Delete zones from the oldest file
					CPrimRegion PrimRegion;
					{
						CIFile fileIn;
						fileIn.open (vPrimFiles[nPrimFile]);
						CIXml input;
						input.init (fileIn);
						PrimRegion.serial (input);
					}

					for (j = 0; j < PrimRegion.VZones.size(); ++j)
						delAllIGZoneUnderPatat (_Options->CellSize, &PrimRegion.VZones[j], _OutIGDir);
					for (j = 0; j < PrimRegion.VPaths.size(); ++j)
						delAllIGZoneUnderPath (_Options->CellSize, &PrimRegion.VPaths[j], _OutIGDir);
					for (j = 0; j < PrimRegion.VPoints.size(); ++j)
						delAllIGZoneUnderPoint (_Options->CellSize, &PrimRegion.VPoints[j], _OutIGDir);

					CTools::chdir (_ExeDir);
					sTmp = vPrimFiles[nPrimFile] + " not needed anymore.";
					if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
				}
			}
		}
		// End of fill the structure for update

		// Interpretation of structure for update
		for (i = 0; i < vPrimToUpdate.size(); ++i)
		{
			// Load all the .flora file
			vector<SFormFlora> vFormFloras;
			for (j = 0; j < vPrimToUpdate[i].FloraFile.size(); ++j)
			{
				// Create the loader
				UFormLoader *loader = UFormLoader::createLoader ();

				// Load the form
				CSmartPtr<UForm> form = loader->loadForm (vPrimToUpdate[i].FloraFile[j].c_str ());
				if (form)
				{
					SFormFlora FormFloraTmp;
					FormFloraTmp.build (form->getRootNode ());
					vFormFloras.push_back (FormFloraTmp);
				}

				// Release the loader
				UFormLoader::releaseLoader (loader);
			}

			// Load all the .prim files that can be referenced by the flora
			CPrimRegion PrimRegion;
			{
				CIFile fileIn;
				fileIn.open (vPrimToUpdate[i].PrimFile);
				CIXml input;
				input.init (fileIn);
				PrimRegion.serial (input);
			}

			// Delete zones under the prims that has been referenced by a flora

			for (j = 0; j < PrimRegion.VZones.size(); ++j)
			{
				// Get the patat
				CPrimZone *pPatat = NULL;

				// Check if the current patat is referenced by a flora
				for (k = 0; k < vFormFloras.size(); ++k)
				for (m = 0; m < vFormFloras[k].IncludePatats.size(); ++m)
					if (PrimRegion.VZones[j].getName() == vFormFloras[k].IncludePatats[m])
						pPatat = &PrimRegion.VZones[j];

				if ((pPatat == NULL) || (pPatat->VPoints.size() <= 2))
				{
					continue;
				}
				delAllIGZoneUnderPatat (_Options->CellSize, pPatat, _OutIGDir);
			}
			for (j = 0; j < PrimRegion.VPoints.size(); ++j)
			{
				// Get the point
				CPrimPoint *pPoint = NULL;

				// Check if the current point is referenced by a flora
				for (k = 0; k < vFormFloras.size(); ++k)
				for (m = 0; m < vFormFloras[k].IncludePatats.size(); ++m)
					if (PrimRegion.VPoints[j].getName() == vFormFloras[k].IncludePatats[m])
						pPoint = &PrimRegion.VPoints[j];
				delAllIGZoneUnderPoint (_Options->CellSize, pPoint, _OutIGDir);
			}
			for (j = 0; j < PrimRegion.VPaths.size(); ++j)
			{
				// Get the path
				CPrimPath *pPath = NULL;

				// Check if the current path is referenced by a flora
				for (k = 0; k < vFormFloras.size(); ++k)
				for (m = 0; m < vFormFloras[k].IncludePatats.size(); ++m)
					if (PrimRegion.VPaths[j].getName() == vFormFloras[k].IncludePatats[m])
						pPath = &PrimRegion.VPaths[j];
				delAllIGZoneUnderPath (_Options->CellSize, pPath, _OutIGDir);
			}
		}
		// End of Process if a prim has been modified

	}
	// End of process all regions

	// Check all patat to export (a patat that has no zone under itself (deleted or not present))
	vector<SExportPrimitive> vExportPrimitives;
	vector<string> vAllPrimFiles; // All prim files of a continent
	CTools::chdir (sContinentDir);
	CTools::dirSub ("*.prim", vAllPrimFiles, true);
	for (i = 0; i < vAllPrimFiles.size(); ++i)
	{
		vAllPrimFiles[i] = strlwr(vAllPrimFiles[i]);
		// Load the primfile
		CPrimRegion PrimRegion;
		{
			CIFile fileIn;
			fileIn.open (vAllPrimFiles[i]);
			CIXml input;
			input.init (fileIn);
			PrimRegion.serial (input);
		}

		for (j = 0; j < PrimRegion.VZones.size(); ++j)
		{
			// Check all zones to know if this patat must be updated
			if (isPatatNeedUpdate(_Options->CellSize, &PrimRegion.VZones[j], _OutIGDir))
			{
				SExportPrimitive epTmp;
				epTmp.FullPrimName = vAllPrimFiles[i];
				epTmp.PrimitiveName = PrimRegion.VZones[j].getName();
				vExportPrimitives.push_back (epTmp);
			}
		}

		for (j = 0; j < PrimRegion.VPaths.size(); ++j)
		{
			// Check all pathes to know if some must be updated (if no zone under)
			if (isPathNeedUpdate(_Options->CellSize, &PrimRegion.VPaths[j], _OutIGDir))
			{
				SExportPrimitive epTmp;
				epTmp.FullPrimName = vAllPrimFiles[i];
				epTmp.PrimitiveName = PrimRegion.VPaths[j].getName();
				vExportPrimitives.push_back (epTmp);
			}
		}

		for (j = 0; j < PrimRegion.VPoints.size(); ++j)
		{
			// Check all points to know if some must be updated (if no zone under)
			if (isPointNeedUpdate(_Options->CellSize, &PrimRegion.VPoints[j], _OutIGDir))
			{
				SExportPrimitive epTmp;
				epTmp.FullPrimName = vAllPrimFiles[i];
				epTmp.PrimitiveName = PrimRegion.VPoints[j].getName();
				vExportPrimitives.push_back (epTmp);
			}
		}
	}

	// Export

	CTools::chdir (_ExeDir);
	_Options->PrimFloraDir = sContinentDir;
	sTmp = "Exporting";
	if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
	doExport (*_Options, _ExportCB, &vExportPrimitives);

	// Copy new files for incremental purpose
	CTools::chdir (_ExeDir);
	sTmp = "Incrementing";
	if (_ExportCB != NULL) _ExportCB->dispInfo (sTmp);
	for (nRegion = 0; nRegion < vRegions.size(); ++nRegion)
	{
		CTools::chdir (sContinentDir + vRegions[nRegion]);
		vector<string> vFiles;
		CTools::dirSub ("*.prim", vFiles, true);
		for (i = 0; i < vFiles.size(); ++i)
		{
			string sDst = vFiles[i].substr (sContinentDir.size());
			for (j = 0; j < sDst.size(); ++j)
				if (sDst[j] == '\\')
					sDst[j] = '-';
			sDst = _OutIGDir + sDst;
			CTools::copy (sDst, vFiles[i]);
		}
		CTools::chdir (sContinentDir + vRegions[nRegion]);
		CTools::dirSub ("*.flora", vFiles, true);
		for (i = 0; i < vFiles.size(); ++i)
		{
			string sDst = vFiles[i].substr (sContinentDir.size());
			for (j = 0; j < sDst.size(); ++j)
				if (sDst[j] == '\\')
					sDst[j] = '-';
			sDst = _OutIGDir + sDst;
			CTools::copy (sDst, vFiles[i]);
		}

		// -------------------------------------------------------------------
		// If the new file do not exists anymore but the old file exists -> delete old file
		CTools::chdir (_OutIGDir);
		CTools::dir (vRegions[nRegion] + "-*.prim", vFiles, false);
		for (i = 0; i < vFiles.size(); ++i)
		{
			// Get the name of the recent file
			string sNewName = vFiles[i];
			for (j = 0; j < sNewName.size(); ++j)
				if (sNewName[j] == '-')
					sNewName[j] = '\\';

			sNewName = sContinentDir + sNewName;
			if (!CTools::fileExist(sNewName))
			{
				// Delete the oldest file
				CFile::deleteFile(vFiles[i]);
			}
		}

		// If the new file do not exists anymore but the old file exists -> delete old file
		CTools::dir (vRegions[nRegion] + "-*.flora", vFiles, false);
		for (i = 0; i < vFiles.size(); ++i)
		{
			// Get the name of the recent file
			string sNewName = vFiles[i];
			for (j = 0; j < sNewName.size(); ++j)
				if (sNewName[j] == '-')
					sNewName[j] = '\\';

			sNewName = sContinentDir + sNewName;
			if (!CTools::fileExist(sNewName))
			{
				// Delete the oldest file
				CFile::deleteFile(vFiles[i]);
			}
		}
	}

	CTools::chdir (_ExeDir);
	return true;
}

// ---------------------------------------------------------------------------
bool CExport::doExport (SExportOptions &opt, IExportCB *expCB, vector<SExportPrimitive> *selection)
{
	char sTmp[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sTmp);

	_Options = &opt;
	_ExportCB = expCB;

	// Does we have something to export
	if ((selection != NULL) && (selection->size() == 0))
	{
		if (_ExportCB)
			_ExportCB->dispInfo ("Nothing to export");
		return true;
	}

	// If we want to generate flora then we have to load the landscape
	uint32 i;

	if (_ExportCB)
		_ExportCB->dispPass ("Load Landscape");
	if (_Landscape == NULL)
	{
		_Landscape = new CLandscape;
		_Landscape->init ();
		_VCM = new CVisualCollisionManager;
		_VCE = _VCM->createEntity ();
		_VCM->setLandscape (_Landscape);
		_VCE->setSnapToRenderedTesselation (false);
		try
		{
			CIFile bankFile (_LandBankFile);
			_Landscape->TileBank.serial (bankFile);
			CIFile farbankFile (_LandFarBankFile);
			_Landscape->TileFarBank.serial (farbankFile);
			_Landscape->TileBank.makeAllPathRelative ();
			_Landscape->TileBank.setAbsPath ("");
			_Landscape->TileBank.makeAllExtensionDDS ();
			_Landscape->initTileBanks ();

			loadLandscape (_LandFile);
		}
		catch (const Exception &/*e*/)
		{
			if (_ExportCB)
				_ExportCB->dispError ("Cannot load banks files");
		}
	}

	_FloraInsts.clear ();
	if (_ExportCB)
		_ExportCB->dispPass ("Generate Flora");
	vector<string>	allFloraFiles;
	SetCurrentDirectory (_Options->PrimFloraDir.c_str());
	getAllFiles (".Flora", allFloraFiles);
	SetCurrentDirectory (sTmp);
	for (i = 0; i < allFloraFiles.size(); ++i)
	{
		generateIGFromFlora (allFloraFiles[i], selection);
	}

	writeFloraIG (_LandFile, (selection != NULL)); // If selection != NULL then test for writing

	SetCurrentDirectory (sTmp);
	if (_ExportCB)
		_ExportCB->dispPass ("Finished");

	return true;
}

// ---------------------------------------------------------------------------
void CExport::getAllFiles (const string &ext, vector<string> &files)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile ("*.*", &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string sNewDir = sCurDir;
				sNewDir += string("\\") + findData.cFileName;
				SetCurrentDirectory (sNewDir.c_str());
				getAllFiles (ext, files);
				SetCurrentDirectory (sCurDir);
			}
			else
			{
				if (strlen(findData.cFileName) > strlen(ext.c_str()))
					if (stricmp(&findData.cFileName[strlen(findData.cFileName)-strlen(ext.c_str())], ext.c_str()) == 0)
					{
						string fullName = sCurDir;
						fullName += string("\\") + findData.cFileName;
						fullName = strlwr (fullName);
						files.push_back (fullName);
					}
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
bool CExport::searchFile (const std::string &plantName, std::string &dir)
{
	char sCurDir[MAX_PATH];
	bool bFound = false;
	GetCurrentDirectory (MAX_PATH, sCurDir);

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile ("*.*", &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		string filename = findData.cFileName;
		filename = strlwr(filename);
		if (!((filename == ".") || (filename == "..")))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string sNewDir = sCurDir;
				sNewDir += string("\\") + filename;
				SetCurrentDirectory (sNewDir.c_str());
				if (searchFile (plantName, dir))
				{
					bFound = true;
					break;
				}
				SetCurrentDirectory (sCurDir);
			}
			else
			{
				if (strlwr(plantName) == filename)
				{
					dir = sCurDir;
					bFound = true;
					break;
				}
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (sCurDir);
	return bFound;
}

// ---------------------------------------------------------------------------
bool CExport::generateIGFromFlora (const std::string &SrcFile, std::vector<SExportPrimitive> *selection)
{
	uint32 i, j, k, l, m;

	if (_ExportCB)
		_ExportCB->dispPass ("Generating From " + SrcFile);

	// Load all .prim
	vector<CPrimRegion> allPrimRegion;
	vector<string> allPrimFiles;
	{
		char sCurDir[MAX_PATH];
		GetCurrentDirectory (MAX_PATH, sCurDir);
		SetCurrentDirectory (_Options->PrimFloraDir.c_str());
		getAllFiles (".prim", allPrimFiles);
		SetCurrentDirectory (sCurDir);
		for (i = 0; i < allPrimFiles.size(); ++i)
		{
			try
			{
				CPrimRegion tmpPrimRegion;
				CIFile fileIn;
				fileIn.open (allPrimFiles[i]);
				CIXml input;
				input.init (fileIn);
				tmpPrimRegion.serial (input);
				allPrimRegion.push_back (tmpPrimRegion);
			}
			catch (const Exception &/*e*/)
			{
				if (_ExportCB != NULL)
					_ExportCB->dispWarning (string("Cant load ") + allPrimFiles[i]);
				CPrimRegion tmpPrimRegion;
				allPrimRegion.push_back (tmpPrimRegion);
			}
		}
	}

	// Load the .Flora file (georges file) and load all associated .plant
	SFormFlora formFlora;
	map<string, SFormPlant> Plants;
	{
		// Create a loader
		UFormLoader *loader = UFormLoader::createLoader ();
//		CPath::addSearchPath (_Options->DfnDir, true, true);
//		CPath::addSearchPath (_Options->GameElemDir, true, true);
//		CPath::addSearchPath (_Options->LandTileNoiseDir, true, true);

		// Load the form
		CSmartPtr<UForm> form = loader->loadForm (SrcFile.c_str ());
		if (form == NULL)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispError (string("Cant load ") + SrcFile );
		}
		else
		{
			// Read .Flora
			formFlora.build (form->getRootNode ());

			// Load the .plant associated
			for (i = 0; i < formFlora.PlantInstances.size(); ++i)
			{
				const string &plantName = formFlora.PlantInstances[i].Name;
				map<string, SFormPlant>::iterator it = Plants.find (plantName);
				if (it != Plants.end()) // Already here ?!
					continue; // Zap it

				char sCurDir[MAX_PATH];
				GetCurrentDirectory (MAX_PATH, sCurDir);

				try
				{
					SetCurrentDirectory (_GameElemDir.c_str());
					string dir;

					if (searchFile (plantName, dir))
					{
						string tmpName = dir + string("\\") + plantName;

						CSmartPtr<UForm> form2 = loader->loadForm (tmpName.c_str());
						if (form2)
						{
							SFormPlant plantTmp;
							plantTmp.build (form2->getRootNode ());
							Plants.insert (map<string, SFormPlant>::value_type(plantName, plantTmp));
						}
						else
						{
							if (_ExportCB != NULL)
								_ExportCB->dispWarning (string("Cant load ") + plantName);
						}
					}
					else
					{
						if (_ExportCB != NULL)
							_ExportCB->dispWarning (string("Cant load ") + plantName);
					}
					SetCurrentDirectory (sCurDir);
				}
				catch (const Exception &e)
				{
					SetCurrentDirectory (sCurDir);
					if (_ExportCB != NULL)
						_ExportCB->dispWarning (string("Cant load ") + plantName + "(" + e.what() + ")" );
				}
			}
		}
	}

	// Sort PlantInstances by biggest radius first
	if (formFlora.PlantInstances.size() > 1)
	for (i = 0; i < (formFlora.PlantInstances.size()-1); ++i)
	for (j = i+1; j < formFlora.PlantInstances.size(); ++j)
	{
		SPlantInstance &rPlantI = formFlora.PlantInstances[i];
		SPlantInstance &rPlantJ = formFlora.PlantInstances[j];
		map<string, SFormPlant>::iterator it = Plants.find (rPlantI.Name);
		if (it == Plants.end())
			continue;
		SFormPlant &rFormPlantI = it->second;

		it = Plants.find (rPlantJ.Name);
		if (it == Plants.end())
			continue;
		SFormPlant &rFormPlantJ = it->second;
		if (rFormPlantI.BoundingRadius < rFormPlantJ.BoundingRadius)
		{
			SPlantInstance pi = formFlora.PlantInstances[i];
			formFlora.PlantInstances[i] = formFlora.PlantInstances[j];
			formFlora.PlantInstances[j] = pi;
		}
	}

	// Generating
	float jitter = formFlora.JitterPos;
	clamp (jitter, 0.0f, 1.0f);
	srand (formFlora.RandomSeed);
	for (i = 0; i < formFlora.IncludePatats.size(); ++i)
	{
		uint32 nCurPlant = 0;
		CVector vMin, vMax;

		if (_ExportCB)
			_ExportCB->dispPass ("IncludePatats("+toString(i+1)+"/"+toString(formFlora.IncludePatats.size())+")");

		// Get the patat
		CPrimZone  *pPatat = NULL;
		CPrimPoint *pPoint = NULL;
		CPrimPath  *pPath  = NULL;

		// Look if this is a patat
		for (j = 0; j < allPrimRegion.size(); ++j)
		{
			for (k = 0; k < allPrimRegion[j].VZones.size(); ++k)
			{
				if (allPrimRegion[j].VZones[k].getName() == formFlora.IncludePatats[i])
				{
					if (selection != NULL)
					{
						SExportPrimitive epTmp;
						epTmp.FullPrimName = allPrimFiles[j];
						epTmp.PrimitiveName = allPrimRegion[j].VZones[k].getName();
						for (m = 0; m < selection->size(); ++m)
						if (selection->operator[](m) == epTmp)
						{
							pPatat = &allPrimRegion[j].VZones[k];
							break;
						}
					}
					else
					{
						pPatat = &allPrimRegion[j].VZones[k];
					}
				}
			}
		}

		//Look if this is a point
		for (j = 0; j < allPrimRegion.size(); ++j)
		{
			for (k = 0; k < allPrimRegion[j].VPoints.size(); ++k)
			{
				if (allPrimRegion[j].VPoints[k].getName() == formFlora.IncludePatats[i])
				{
					if (selection != NULL)
					{
						SExportPrimitive epTmp;
						epTmp.FullPrimName = allPrimFiles[j];
						epTmp.PrimitiveName = allPrimRegion[j].VPoints[k].getName();
						for (m = 0; m < selection->size(); ++m)
						if (selection->operator[](m) == epTmp)
						{
							pPoint = &allPrimRegion[j].VPoints[k];
							break;
						}
					}
					else
					{
						pPoint = &allPrimRegion[j].VPoints[k];
					}
				}
			}
		}

		//Look if this is a path
		for (j = 0; j < allPrimRegion.size(); ++j)
		{
			for (k = 0; k < allPrimRegion[j].VPaths.size(); ++k)
			{
				if (allPrimRegion[j].VPaths[k].getName() == formFlora.IncludePatats[i])
				{
					if (selection != NULL)
					{
						SExportPrimitive epTmp;
						epTmp.FullPrimName = allPrimFiles[j];
						epTmp.PrimitiveName = allPrimRegion[j].VPaths[k].getName();
						for (m = 0; m < selection->size(); ++m)
						if (selection->operator[](m) == epTmp)
						{
							pPath = &allPrimRegion[j].VPaths[k];
							break;
						}
					}
					else
					{
						pPath = &allPrimRegion[j].VPaths[k];
					}
				}
			}
		}


		if ((pPatat == NULL) && (pPoint == NULL) && (pPath == NULL))
		{
			if (selection == NULL)
				if (_ExportCB)
					_ExportCB->dispWarning ("Cannot find " + formFlora.IncludePatats[i]);
			continue;
		}

		if ((pPatat != NULL) && (pPatat->VPoints.size() <= 2))
		{
			if (_ExportCB)
				_ExportCB->dispWarning ("Patat " + pPatat->getName() + " has less than 3 points");
			continue;
		}

		if ((pPath != NULL) && (pPath->VPoints.size() <= 1))
		{
			if (_ExportCB)
				_ExportCB->dispWarning ("Path " + pPath->getName() + " has less than 2 points");
			continue;
		}

		// Generate for a patat
		if (pPatat != NULL)
		{
			vMin = vMax = pPatat->VPoints[0];
			for (j = 0; j < pPatat->VPoints.size(); ++j)
			{
				if (vMin.x > pPatat->VPoints[j].x) vMin.x = pPatat->VPoints[j].x;
				if (vMin.y > pPatat->VPoints[j].y) vMin.y = pPatat->VPoints[j].y;
				if (vMin.z > pPatat->VPoints[j].z) vMin.z = pPatat->VPoints[j].z;
				if (vMax.x < pPatat->VPoints[j].x) vMax.x = pPatat->VPoints[j].x;
				if (vMax.y < pPatat->VPoints[j].y) vMax.y = pPatat->VPoints[j].y;
				if (vMax.z < pPatat->VPoints[j].z) vMax.z = pPatat->VPoints[j].z;
			}

			for (j = 0; j < formFlora.PlantInstances.size(); ++j)
			{
				SPlantInstance &rPlant = formFlora.PlantInstances[j];
				map<string, SFormPlant>::iterator it = Plants.find (rPlant.Name);
				if (it == Plants.end())
				{
					if (_ExportCB)
						_ExportCB->dispWarning ("Cannot find " + rPlant.Name);
					continue;
				}
				SFormPlant &rFormPlant = it->second;

				float squareLength = (float)sqrt (Pi*rFormPlant.BoundingRadius*rFormPlant.BoundingRadius / rPlant.Density);
				uint32 nNbPlantX = 1+(int)floor ((vMax.x-vMin.x) / squareLength);
				uint32 nNbPlantY = 1+(int)floor ((vMax.y-vMin.y) / squareLength);
				for (l = 0; l < nNbPlantY; ++l)
				for (k = 0; k < nNbPlantX; ++k)
				{
					if (_ExportCB)
						_ExportCB->dispPassProgress (((float)(k+l*nNbPlantX))/((float)(nNbPlantX*nNbPlantY)));

					bool bExists = false;
					CVector pos;
					float scaleTmp;
					for (m = 0; m < 32; ++m)
					{
						pos.x = vMin.x + squareLength * k + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
						pos.y = vMin.y + squareLength * l + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
						pos.z = 0.0f;
						scaleTmp = (formFlora.ScaleMax-formFlora.ScaleMin)*frand(1.0)+formFlora.ScaleMin;
						if (pPatat->contains(pos))
						{
							if (isWellPlaced(pos, rPlant, rFormPlant, scaleTmp))
							{
								// Testt finally with the exclude patats ...
								bExists	= true;
								for (uint32 expat = 0; expat < formFlora.ExcludePatats.size(); ++expat)
								{
									CPrimZone *pExPatat = NULL;
									for (uint32 epj = 0; epj < allPrimRegion.size(); ++epj)
									for (uint32 epk = 0; epk < allPrimRegion[epj].VZones.size(); ++epk)
										if (allPrimRegion[epj].VZones[epk].getName() == formFlora.ExcludePatats[expat])
										{
											pExPatat = &allPrimRegion[epj].VZones[epk];
											break;
										}
									if (pExPatat != NULL)
									{
										if (pExPatat->contains(pos))
										{
											bExists = false;
											break;
										}
									}
								}
								if (bExists)
									break;
							}
						}
					}

					if (!bExists)
						continue;

					SFloraInst vi;
					vi.ShapeName = rFormPlant.Shape;
					vi.PlantName = rPlant.Name;
					vi.Scale = scaleTmp;
					vi.Radius = rFormPlant.BoundingRadius * vi.Scale;
					vi.Rot = (float)Pi * frand (1.0);

					if (formFlora.PutOnWater)
					{
						if (pos.z < formFlora.WaterHeight)
							pos.z = formFlora.WaterHeight;
					}
					vi.Pos = pos;
					_FloraInsts.push_back (vi);
				} // End of for all position of a plant x,y check if we cant put it
			} // End of for all plant instances
		} // End of Generate for a patat

		// Generate for a point
		if (pPoint != NULL)
		{
			// Choose  a plant
			float total = 0.0f;
			for (j = 0; j < formFlora.PlantInstances.size(); ++j)
			{
				total += formFlora.PlantInstances[j].Density;
			}
			float posf = total * frand(1.0);
			total = 0.0f;
			for (j = 0; j < formFlora.PlantInstances.size(); ++j)
			{
				total += formFlora.PlantInstances[j].Density;
				if (posf < total) break;
			}
			if (j == formFlora.PlantInstances.size())
				j = (uint32)formFlora.PlantInstances.size()-1;

			SPlantInstance &rPlant = formFlora.PlantInstances[j];
			map<string, SFormPlant>::iterator it = Plants.find (rPlant.Name);
			if (it == Plants.end())
			{
				if (_ExportCB)
					_ExportCB->dispWarning ("Cannot find " + rPlant.Name);
				continue;
			}
			SFormPlant &rFormPlant = it->second;

			SFloraInst vi;
			vi.ShapeName = rFormPlant.Shape;
			vi.PlantName = rPlant.Name;
			vi.Scale = (formFlora.ScaleMax-formFlora.ScaleMin)*frand(1.0)+formFlora.ScaleMin;
			vi.Radius = rFormPlant.BoundingRadius * vi.Scale;
			vi.Rot = (float)Pi * frand (1.0);

			CVector pos;
			pos.x = pPoint->Point.x;
			pos.y = pPoint->Point.y;
			pos.z = getZFromXY (pos.x, pos.y);
			if (formFlora.PutOnWater)
			{
				if (pos.z < formFlora.WaterHeight)
					pos.z = formFlora.WaterHeight;
			}
			vi.Pos = pos;
			if (pos.z > -90000.0f)
				_FloraInsts.push_back (vi);
		} // End of Generate for a point

		// Generate for a path
		if (pPath != NULL)
		{
			float rLength = 0.0f; // Total length of the broken line
			for (j = 0; j < pPath->VPoints.size()-1; ++j)
			{
				rLength += (pPath->VPoints[j]-pPath->VPoints[j+1]).norm();
			}

			for (j = 0; j < formFlora.PlantInstances.size(); ++j)
			{
				SPlantInstance &rPlant = formFlora.PlantInstances[j];
				map<string, SFormPlant>::iterator it = Plants.find (rPlant.Name);
				if (it == Plants.end())
				{
					if (_ExportCB)
						_ExportCB->dispWarning ("Cannot find " + rPlant.Name);
					continue;
				}
				SFormPlant &rFormPlant = it->second;

				float squareLength = (float)(2*rFormPlant.BoundingRadius / rPlant.Density);
				uint32 nNbPlant = 1+(int)floor (rLength / squareLength);

				for (k = 0; k < nNbPlant; ++k)
				{
					if (_ExportCB)
						_ExportCB->dispPassProgress (((float)(k))/((float)(nNbPlant)));

					bool bExists = false;
					CVector pos;
					float scaleTmp;
					for (m = 0; m < 32; ++m)
					{
						// Calculate the curviline abscisse
						float curvAbs = squareLength * k + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
						float TempLength = 0.0f;
						// Convert to a real point along the curve (broken line)
						for (l = 0; l < pPath->VPoints.size()-1; ++l)
						{
							float newSize = (pPath->VPoints[l]-pPath->VPoints[l+1]).norm();
							if (curvAbs < (TempLength+newSize))
							{
								curvAbs -= TempLength;
								break;
							}
							TempLength += newSize;
						}
						if (l == (pPath->VPoints.size()-1))
						{
							l = (uint32)pPath->VPoints.size()-2;
							curvAbs = (pPath->VPoints[l]-pPath->VPoints[l+1]).norm();
						}
						// Calculate the coord
						curvAbs = curvAbs / (pPath->VPoints[l]-pPath->VPoints[l+1]).norm();
						pos = pPath->VPoints[l] + (pPath->VPoints[l+1]-pPath->VPoints[l])*curvAbs;
						pos.z = 0.0f;
						scaleTmp = (formFlora.ScaleMax-formFlora.ScaleMin)*frand(1.0)+formFlora.ScaleMin;
						if (isWellPlaced(pos, rPlant, rFormPlant, scaleTmp))
						{
							// Test finally with the exclude patats ...
							bExists	= true;
							for (uint32 expat = 0; expat < formFlora.ExcludePatats.size(); ++expat)
							{
								CPrimZone *pExPatat = NULL;
								for (uint32 epj = 0; epj < allPrimRegion.size(); ++epj)
								for (uint32 epk = 0; epk < allPrimRegion[epj].VZones.size(); ++epk)
									if (allPrimRegion[epj].VZones[epk].getName() == formFlora.ExcludePatats[expat])
									{
										pExPatat = &allPrimRegion[epj].VZones[epk];
										break;
									}
								if (pExPatat != NULL)
								{
									if (pExPatat->contains(pos))
									{
										bExists = false;
										break;
									}
								}
							}
							if (bExists)
								break;
						}
					}

					if (!bExists)
						continue;

					SFloraInst vi;
					vi.ShapeName = rFormPlant.Shape;
					vi.PlantName = rPlant.Name;
					vi.Scale = scaleTmp;
					vi.Radius = rFormPlant.BoundingRadius * vi.Scale;
					vi.Rot = (float)Pi * frand (1.0);

					if (formFlora.PutOnWater)
					{
						if (pos.z < formFlora.WaterHeight)
							pos.z = formFlora.WaterHeight;
					}
					vi.Pos = pos;
					_FloraInsts.push_back (vi);
				} // End of for all position of a plant x,y check if we cant put it
			} // End of for all plant instances

		} // End of Generate for a path

	} // End of for all IncludePatats
	return true;
}

// ---------------------------------------------------------------------------
float CExport::getZFromXY (float x, float y)
{
	CVector pos = CVector(x, y, 0);
	CVector normal;
	float z, zmin, zmax;

	// Approximate the z with patch bounding boxes
	sint32 zoneX = (sint32)floor (x/_Options->CellSize);
	sint32 zoneY = (sint32)floor (-y/_Options->CellSize);
	sint32 zoneId = zoneY * 256 +  zoneX;
	CZone *pZone = _Landscape->getZone (zoneId);
	if (pZone == NULL)
		return -100000.0f;

	CAABBoxExt bb = pZone->getZoneBB();
	zmin = bb.getMin().z;
	zmax = bb.getMax().z;
	pos.z = zmin;
	z = zmin;
	while (z < zmax)
	{
		if (_VCE->snapToGround(pos, normal))
			break;
		z += CVisualCollisionEntity::BBoxRadiusZ / 2.0f; // Super sampling due to max frequency on radiosity
		pos.z = z;
	}

	if (z >= zmax)
		return -100000.0f;

	return pos.z;
}

// ---------------------------------------------------------------------------
bool CExport::isWellPlaced (CVector &pos, SPlantInstance &rPI, SFormPlant &rFP, float scale)
{
	uint32 i;

	// Look if this Flora intersect with one of the current ones
	for (i = 0; i < _FloraInsts.size(); ++i)
	{
		CVector temp = _FloraInsts[i].Pos - pos;
		temp.z = 0.0f;
		if (temp.norm() < (_FloraInsts[i].Radius + scale*rFP.BoundingRadius))
			return false;
	}

	// Get the real Z
	pos.z = getZFromXY (pos.x, pos.y);
	if (pos.z < -90000.0f)
		return false;

	// Get some Z around to see if we can put the Flora on the ground
	uint32 nNbSamples = 8; // Const to be put somewhere
	vector<CVector> base;
	base.resize (nNbSamples);
	for (i = 0; i < nNbSamples; ++i)
	{
		base[i] = pos;
		base[i].x += scale * rFP.CollisionRadius * cosf((2.0f*(float)Pi*i)/(float)nNbSamples);
		base[i].y += scale * rFP.CollisionRadius * sinf((2.0f*(float)Pi*i)/(float)nNbSamples);
		base[i].z = getZFromXY (base[i].x, base[i].y);

		if (fabs(base[i].z-pos.z) > 0.8f)
			return false;
	}

	return true;
}

// ---------------------------------------------------------------------------
void CExport::writeFloraIG (const string &LandFile, bool bTestForWriting)
{
	sint32 i, j, k;

	if (_FloraInsts.size() == 0)
		return;

	CZoneRegion zoneRegion;
	CIFile inFile;
	if (inFile.open (LandFile))
	{
		CIXml xml(true);
		xml.init (inFile);
		zoneRegion.serial (xml);

		inFile.close();
	}
	else
	{
		nlwarning ("Can't open the file %s", LandFile.c_str());
	}

	// Load all zone

	for (j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
	for (i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		if ((zoneRegion.getName(i,j) == STRING_OUT_OF_BOUND) ||
			(zoneRegion.getName(i,j) == STRING_UNUSED))
			continue;

		vector<int> vegZone;
		// Take all Flora instances in the zone (i,j)
		for (k = 0; k < (sint32)_FloraInsts.size(); ++k)
		{
			if (((i*_Options->CellSize) < _FloraInsts[k].Pos.x) && (_FloraInsts[k].Pos.x < ((i+1)*_Options->CellSize)) &&
				((j*_Options->CellSize) < _FloraInsts[k].Pos.y) && (_FloraInsts[k].Pos.y < ((j+1)*_Options->CellSize)))
			{
				vegZone.push_back (k);
			}
		}


		// Make the .IG
		string ZoneName;
		ZoneName += NLMISC::toString(-j) + "_";
		ZoneName += 'a' + (i/26);
		ZoneName += 'a' + (i%26);

		CVector vGlobalPos = CVector (0.0f, 0.0f, 0.0f);
		CInstanceGroup::TInstanceArray Instances;
		vector<CCluster> Portals;
		vector<CPortal> Clusters;
		Instances.resize (vegZone.size());

		for (k = 0; k < (sint32)vegZone.size(); ++k)
		{
//vGlobalPos += _FloraInsts[vegZone[k]].Pos;
			Instances[k].Pos = _FloraInsts[vegZone[k]].Pos;
			Instances[k].Rot = CQuat(CVector::K, _FloraInsts[vegZone[k]].Rot);
			Instances[k].Scale = CVector(_FloraInsts[vegZone[k]].Scale, _FloraInsts[vegZone[k]].Scale, _FloraInsts[vegZone[k]].Scale);
			Instances[k].nParent = -1;
			Instances[k].Name = _FloraInsts[vegZone[k]].ShapeName;
			Instances[k].InstanceName = _FloraInsts[vegZone[k]].PlantName;
			/*Instances[k].InstanceName = "Flora_"; // see if it works
			Instances[k].InstanceName += ZoneName + "_";
			Instances[k].InstanceName += '0' + ((k/1000)%10);
			Instances[k].InstanceName += '0' + ((k/100) %10);
			Instances[k].InstanceName += '0' + ((k/10)  %10);
			Instances[k].InstanceName += '0' + ( k      %10);*/
		}

// \todo trap -> look why it dont seems to work with a global positionning
//vGlobalPos /= (float)vegZone.size();
//for (k = 0; k < (sint32)vegZone.size(); ++k)
//	Instances[k].Pos -= vGlobalPos;

		CInstanceGroup IG;
		IG.build (vGlobalPos, Instances, Portals, Clusters);

		ZoneName = _OutIGDir + "\\" + ZoneName;
		ZoneName += ".ig";

		CIFile inFile; // If file already exists and we have selection...
		if (bTestForWriting)
			if (inFile.open(ZoneName))
			{
				inFile.close();
				continue;
			}

		try
		{
			COFile outFile (ZoneName);
			IG.serial (outFile);
			if (_ExportCB != NULL)
				_ExportCB->dispInfo (ZoneName + " generated");
		}
		catch (const Exception &e)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning ("Cant write " + ZoneName + " (" + e.what() + ")");
		}
	}
}

// ---------------------------------------------------------------------------
void CExport::loadLandscape (const string &LandFile)
{
	CZoneRegion zoneRegion;

	CIFile inFile;
	try
	{
		if (inFile.open (LandFile))
		{
			CIXml xml(true);
			xml.init (inFile);
			zoneRegion.serial (xml);

			inFile.close();
		}
		else
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Can't open file ") + LandFile);
		}
	}
	catch (const Exception &e)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning (string("Cant load ") + LandFile + " : " + e.what());
	}
	// Load all zone

	sint32 nTotalFile = (1 + zoneRegion.getMaxY() - zoneRegion.getMinY()) * (1 + zoneRegion.getMaxX() - zoneRegion.getMinX());
	sint32 nCurrentFile = 0;
	for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		++nCurrentFile;
		if (_ExportCB != NULL)
			_ExportCB->dispPassProgress(((float)nCurrentFile)/((float)nTotalFile));

		if ((zoneRegion.getName(i,j) == STRING_OUT_OF_BOUND) ||
			(zoneRegion.getName(i,j) == STRING_UNUSED))
			continue;

		// Generate zone name

		string ZoneName = getZoneNameFromXY (i, j);

		ZoneName = _InLandscapeDir + string("\\") + ZoneName;

		//if (_ExportCB != NULL)
		//	_ExportCB->dispInfo (string("Loading ") + ZoneName);

		try
		{
			CZone zone;
			if (!inFile.open (ZoneName + string(".zonew")))
				inFile.open (ZoneName + string(".zonel"));
			zone.serial (inFile);
			inFile.close ();
			_Landscape->addZone (zone);
		}
		catch(const Exception &/*e*/)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant load ") + ZoneName + string(".zone(l,w)"));
		}
		if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
			return;
	}

}

// Public Helpers
// **************

// ---------------------------------------------------------------------------
string CExport::getZoneNameFromXY (sint32 x, sint32 y)
{
	string tmp;

	if ((y>0) || (y<-255) || (x<0) || (x>255))
		return "NOT VALID";
	tmp = toString(-y) + "_";
	tmp += ('A' + (x/26));
	tmp += ('A' + (x%26));
	return tmp;
}

// ---------------------------------------------------------------------------
sint32 CExport::getXFromZoneName (const string &ZoneName)
{
	string xStr, yStr;
	uint32 i = 0;
	while (ZoneName[i] != '_')
	{
		yStr += ZoneName[i]; ++i;
		if (i == ZoneName.size())
			return -1;
	}
	++i;
	while (i < ZoneName.size())
	{
		xStr += ZoneName[i]; ++i;
	}
	return ((xStr[0] - 'A')*26 + (xStr[1] - 'A'));
}

// ---------------------------------------------------------------------------
sint32 CExport::getYFromZoneName (const string &ZoneName)
{
	string xStr, yStr;
	uint32 i = 0;
	while (ZoneName[i] != '_')
	{
		yStr += ZoneName[i]; ++i;
		if (i == ZoneName.size())
			return 1;
	}
	++i;
	while (i < ZoneName.size())
	{
		xStr += ZoneName[i]; ++i;
	}
	return -atoi(yStr.c_str());
}
