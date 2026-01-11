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

#ifndef LD_EXPORT_H
#define LD_EXPORT_H

// ---------------------------------------------------------------------------

#include "nel/misc/config_file.h"
#include "nel/misc/vector.h"
#include <string>
#include <vector>

// ---------------------------------------------------------------------------

namespace NLLIGO
{
	class CPrimZone;
	class CPrimPoint;
	class CPrimPath;
}

namespace NL3D
{
	class CLandscape;
	class CVisualCollisionManager;
	class CVisualCollisionEntity;
}

struct SPlantInstance;
struct SFormPlant;

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Export options
// ---------------------------------------------------------------------------
struct SExportOptions
{
	std::string InLandscapeDir;		// Directory where to get .zonew files
	std::string OutIGDir;			// Directory where to put IG
	std::string LandBankFile;		// The .smallbank file associated with the landscape
	std::string LandFarBankFile;	// The .farbank file
	float		CellSize;			// Typically 160.0
	std::string	LandTileNoiseDir;	// Directory where to get displacement map

	std::string PrimFloraDir;		// Directory to parse for .flora and .prim associated
									// This is here we get continent.cfg file

	// Not read for the moment
	std::string LandFile;			// The .land file to get the mask and date of zones
	std::string DfnDir;				// Directory to get georges dfn
	std::string GameElemDir;		// Directory to get georges file (pipoti.plant)

	// =======================================================================

	SExportOptions ();
	bool loadcf (NLMISC::CConfigFile &cf);
	bool save (FILE *f);
};

// ---------------------------------------------------------------------------
// Export callback
// ---------------------------------------------------------------------------
class IExportCB
{
public:
	virtual bool isCanceled () = 0; // Tell the exporter if it must end as quick as possible
	// Display callbacks
	virtual void dispPass (const std::string &Text) = 0; // Pass (generate land, flora, etc...)
	virtual void dispPassProgress (float percentage) = 0; // [ 0.0 , 1.0 ]
	virtual void dispInfo (const std::string &Text) = 0; // Verbose
	virtual void dispWarning (const std::string &Text) = 0; // Error but not critical
	virtual void dispError (const std::string &Text) = 0; // Should block (misfunction)
};

// ---------------------------------------------------------------------------
// Export Patat
// ---------------------------------------------------------------------------
struct SExportPrimitive
{
	std::string		FullPrimName;
	std::string		PrimitiveName;

	bool operator == (const SExportPrimitive &rRightArg);
};

// ---------------------------------------------------------------------------
// Flora export
// ---------------------------------------------------------------------------
struct SFloraInst
{
	NLMISC::CVector Pos;
	float			Rot;
	float			Scale;

	float			Radius;
	std::string		ShapeName; // the .shape stored in the name field of the instance (that represents the shape name)
	std::string		PlantName; // the .plant associated which is stored in the instance name
};

// ---------------------------------------------------------------------------
// Export class
// ---------------------------------------------------------------------------
class CExport
{

public:

	CExport ();
	~CExport ();

	// EXPORT one region :
	// Parse the SourceDir find the .land and .prim
	// newExport is the incremental export
	bool newExport (SExportOptions &options, IExportCB *expCB = NULL);
	bool doExport (SExportOptions &options, IExportCB *expCB = NULL, std::vector<SExportPrimitive> *selection = NULL);

	// HELPERS
	// Get All files with the extension ext in the current directory and subdirectory
	static void getAllFiles (const std::string &ext, std::vector<std::string> &files);
	// Search a file through all subdirectories of the current one (and in the current too)
	static bool searchFile (const std::string &plantName, std::string &dir);

private:

	SExportOptions		*_Options;
	IExportCB			*_ExportCB;

	// Temp data to generate ig
	NL3D::CLandscape				*_Landscape;
	NL3D::CVisualCollisionManager	*_VCM;
	NL3D::CVisualCollisionEntity	*_VCE;

	std::vector<SFloraInst>	_FloraInsts;

	std::string _ExeDir;
	std::string _LandFile;
	std::string _GameElemDir;
	std::string _DfnDir;

	std::string _InLandscapeDir;		// Directory where to get .zonew files
	std::string _OutIGDir;			// Directory where to put IG
	std::string _LandBankFile;		// The .smallbank file associated with the landscape
	std::string _LandFarBankFile;	// The .farbank file
	std::string	_LandTileNoiseDir;	// Directory where to get displacement map

private:

	struct SFloraToUpdate
	{
		std::string					FloraFile;
		std::vector<std::string>	PrimFile;
	};

	struct SPrimToUpdate
	{
		std::string					PrimFile;
		std::vector<std::string>	FloraFile;
	};

	// All the functions to generate the igs from flora
	// ************************************************

	// Entry point
	bool generateIGFromFlora (const std::string &SrcFile, std::vector<SExportPrimitive> *selection = NULL);

	// Get the altitude from the position in 2D
	float getZFromXY (float x, float y);

	// Does the plant is well placed
	bool isWellPlaced (NLMISC::CVector &pos, SPlantInstance &rPI, SFormPlant &rFP, float scale);

	// Write zone by zone the instance group of the flora generated in the specific land
	void writeFloraIG (const std::string &LandFile, bool bTestForWriting = false);

	// Helpers
	// *******

	// Load all zones of a .land
	void loadLandscape (const std::string &name);

	bool segmentIntersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
	void delIGZone (sint32 x, sint32 y);
	void delAllIGZoneUnderPatat (float fCellSize, NLLIGO::CPrimZone  *pPatat,	const std::string &sIGOutputDir);
	void delAllIGZoneUnderPoint (float fCellSize, NLLIGO::CPrimPoint *pPoint,	const std::string &sIGOutputDir);
	void delAllIGZoneUnderPath  (float fCellSize, NLLIGO::CPrimPath  *pPath,	const std::string &sIGOutputDir);
	bool isPatatNeedUpdate	(float fCellSize, NLLIGO::CPrimZone *pPatat,	const std::string &sIGOutputDir);
	bool isPathNeedUpdate	(float fCellSize, NLLIGO::CPrimPath *pPath,		const std::string &sIGOutputDir);
	bool isPointNeedUpdate	(float fCellSize, NLLIGO::CPrimPoint *pPoint,	const std::string &sIGOutputDir);


public:

	static std::string getZoneNameFromXY (sint32 x, sint32 y);
	static sint32 getXFromZoneName (const std::string &ZoneName);
	static sint32 getYFromZoneName (const std::string &ZoneName);

};

#endif // LD_EXPORT_H
