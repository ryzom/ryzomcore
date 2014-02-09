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

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/smart_ptr.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/misc/quat.h"

#include "nel/3d/tile_bank.h"
#include "nel/3d/zone_lighter.h"
#include "nel/3d/zone_symmetrisation.h"
#include "nel/3d/landscape.h"
#include "nel/3d/scene_group.h"

#include "nel/ligo/zone_region.h"
#include "nel/ligo/zone_bank.h"

#include "nel/pacs/collision_mesh_build.h"


#include "../../../leveldesign/export/tools.h"

#include <memory>

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace NL3D;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLGEORGES;
using namespace std;
using namespace NLPACS;







// ---------------------------------------------------------------------------
// Export options
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
SExportOptions::SExportOptions ()
{
	ZoneRegion = NULL;
	CellSize = 160.0f;
	Threshold = 1.0f;
	ZFactor = 1.0f;
	ZFactor2 = 1.0f;
	Light = false;
	ExportCollisions = false;
	ExportAdditionnalIGs = false;
}

// ---------------------------------------------------------------------------
void SExportOptions::serial (NLMISC::IStream& s)
{
	int version = s.serialVersion (11);

	s.serial (OutZoneDir);
	s.serial (RefZoneDir);

	if (version > 0)
		s.serial (LigoBankDir);

	if (version > 1)
		s.serial (TileBankFile);

	if (version > 2)
		s.serial (HeightMapFile);

	if (version > 3)
		s.serial (Light);

	if (version > 4)
	{
		s.serial (ZFactor);
		s.serial (HeightMapFile2);
		s.serial (ZFactor2);
	}

	if (version > 5)
	{
		s.serial (ZoneMin);
		s.serial (ZoneMax);
	}

	if (version > 6)
	{
		s.serial (RefIGDir);
		s.serial (OutIGDir);
	}

	if (version > 7)
	{
		s.serial(ExportCollisions);
		s.serial(RefCMBDir, OutCMBDir);
		s.serial(AdditionnalIGInDir, AdditionnalIGOutDir);
		s.serial(ContinentFile);
		s.serial(DFNDir);
	}

	if (version > 8)
	{
		s.serial(ExportAdditionnalIGs);
	}

	if (version > 9)
	{
		s.serial(ContinentsDir);
	}

	if (version > 10)
	{
		s.serial (ColorMapFile);
	}
}

// ---------------------------------------------------------------------------
// CExport
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
CExport::CExport ()
{
	_ZeZoneBank = NULL;
	_FormLoader = UFormLoader::createLoader ();
}

// ---------------------------------------------------------------------------
CExport::~CExport ()
{
	UFormLoader::releaseLoader (_FormLoader);
}

// ---------------------------------------------------------------------------
bool CExport::export_ (SExportOptions &options, IExportCB *expCB)
{
	string sCurDir = CTools::pwd();

	_Options = &options;
	_ExportCB = expCB;

	if (_Options->ZoneRegion == NULL)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispError ("No Zone to export");
		return false;
	}

	// LOADING
	// --- continent form
	CPath::addSearchPath(_Options->DFNDir, true, false);
	CPath::addSearchPath(_Options->RefCMBDir, true, false);
	CPath::addSearchPath(_Options->ContinentsDir, true, false);
	CPath::addSearchPath(_Options->AdditionnalIGInDir, true, false);

	// --- ligozone
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading ligozone bank");
	_ZeZoneBank = new CZoneBank;
	string error;
	_ZeZoneBank->initFromPath (_Options->LigoBankDir, error);

	// --- tile
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading tile bank");
	_ZeTileBank = new CTileBank;
	try
	{
		CIFile inFile (_Options->TileBankFile);
		_ZeTileBank->serial (inFile);
	}
	catch (const Exception &)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispError (string("Cant load tile bank : ") + _Options->TileBankFile);
		return false;
	}

	// --- height map
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading height map");
	_HeightMap = NULL;
	if (_Options->HeightMapFile != "")
	{
		_HeightMap = new CBitmap;
		try
		{
			CIFile inFile;
			if (inFile.open (_Options->HeightMapFile))
			{
				_HeightMap->load (inFile);
			}
			else
			{
				if (_ExportCB != NULL)
					_ExportCB->dispWarning (string("Cant load height map : ") + _Options->HeightMapFile);
				delete _HeightMap;
				_HeightMap = NULL;
			}
		}
		catch (const Exception &)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant load height map : ") + _Options->HeightMapFile);
			delete _HeightMap;
			_HeightMap = NULL;
		}
	}

	// --- height map 2
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading height map");
	_HeightMap2 = NULL;
	if (_Options->HeightMapFile2 != "")
	{
		_HeightMap2 = new CBitmap;
		try
		{
			CIFile inFile;
			if (inFile.open (_Options->HeightMapFile2))
			{
				_HeightMap2->load (inFile);
			}
			else
			{
				if (_ExportCB != NULL)
					_ExportCB->dispWarning (string("ERROR: Cant load height map : ") + _Options->HeightMapFile2);
				delete _HeightMap2;
				_HeightMap2 = NULL;
			}
		}
		catch (const Exception &)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("ERROR: Cant load height map : ") + _Options->HeightMapFile2);
			delete _HeightMap2;
			_HeightMap2 = NULL;
		}
	}

	// --- color map
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading color map");
	_ColorMap = NULL;
	if (_Options->ColorMapFile != "")
	{
		_ColorMap = new CBitmap;
		try
		{
			CIFile inFile;
			if (inFile.open (_Options->ColorMapFile))
			{
				_ColorMap->load (inFile);
			}
			else
			{
				if (_ExportCB != NULL)
					_ExportCB->dispWarning (string("ERROR: Cant load color map : ") + _Options->ColorMapFile);
				delete _ColorMap;
				_ColorMap = NULL;
			}
		}
		catch (const Exception &)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("ERROR: Cant load color map : ") + _Options->ColorMapFile);
			delete _ColorMap;
			_ColorMap = NULL;
		}
	}

	// ****************
	// *EXPORTING CODE*
	// ****************

	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Exporting");

	// Clip the min max to the x[0,255],y[0,-255] values
	sint32 nMinX = _Options->ZoneRegion->getMinX() < 0 ? 0 : _Options->ZoneRegion->getMinX();
	sint32 nMaxX = _Options->ZoneRegion->getMaxX() > 255 ? 255 : _Options->ZoneRegion->getMaxX();
	sint32 nMinY = _Options->ZoneRegion->getMinY() > 0 ? 0 : _Options->ZoneRegion->getMinY();
	sint32 nMaxY = _Options->ZoneRegion->getMaxY() < -255 ? -255 : _Options->ZoneRegion->getMaxY();


	vector<string> allFiles;

	// Delete zones that are not in the bounding square
	try
	{
		// Add zone files
		if (_Options->OutZoneDir != "")
			NLMISC::CPath::getPathContent(_Options->OutZoneDir, true, false, true, allFiles);

		// Add ig files
		vector<string> allOtherFiles;
		if (_Options->OutIGDir != "")
			NLMISC::CPath::getPathContent(_Options->OutIGDir, true, false, true, allOtherFiles);
		allFiles.insert(allFiles.end(), allOtherFiles.begin(), allOtherFiles.end());
	}
	catch (const Exception &e)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispError (string("GetPathcontent failed : ") + e.what());
		return false;
	}

	// zones



	for (uint32 nFile = 0; nFile < allFiles.size(); ++nFile)
	{
		string fileExt = NLMISC::CFile::getExtension(allFiles[nFile]);
		string fileName = NLMISC::CFile::getFilename(allFiles[nFile]);

		if (fileExt != "ig" && fileExt != "zone" && fileExt != "zonel" && fileExt != "zonenh" ) continue;
		sint32 x = getXFromZoneName (fileName);
		sint32 y = getYFromZoneName (fileName);
		if ((x>=0) && (y<=0))
		{
			bool bMustDelete = false;
			// Valid file
			if ((x<nMinX) || (x>nMaxX) || (y<nMinY) || (y>nMaxY))
			{
				bMustDelete = true;
			}
			else
			{
				const string &SrcZoneName = _Options->ZoneRegion->getName (x, y);

				if ((SrcZoneName == STRING_OUT_OF_BOUND) || (SrcZoneName == STRING_UNUSED))
				{
					bMustDelete = true;
				}
			}

			if (bMustDelete)
			{
				if (!CFile::deleteFile (allFiles[nFile]))
				{
					if (_ExportCB != NULL)
						_ExportCB->dispWarning (string("Can't delete ") + fileName);
				}
				else
				{
					if (_ExportCB != NULL)
						_ExportCB->dispInfo (string("Deleted ") + fileName);
				}
			}
		}
	}



	CTools::chdir (sCurDir);

	// Get the export limiters if any
	_ZoneMinX = nMinX;
	_ZoneMinY = nMinY;
	_ZoneMaxX = nMaxX;
	_ZoneMaxY = nMaxY;

	if ((_Options->ZoneMin != "") && (_Options->ZoneMax != ""))
	{
		_Options->ZoneMin = strupr (_Options->ZoneMin);
		_Options->ZoneMax = strupr (_Options->ZoneMax);
		sint32 nNewMinX = getXFromZoneName (_Options->ZoneMin);
		sint32 nNewMinY = getYFromZoneName (_Options->ZoneMin);
		sint32 nNewMaxX = getXFromZoneName (_Options->ZoneMax);
		sint32 nNewMaxY = getYFromZoneName (_Options->ZoneMax);

		if (nNewMinX > nNewMaxX)
			swap (nNewMinX, nNewMaxX);
		if (nNewMinY > nNewMaxY)
			swap (nNewMinY, nNewMaxY);

		if (nNewMinX > nMinX)
			nMinX = nNewMinX;
		if (nNewMinY > nMinY)
			nMinY = nNewMinY;

		if (nNewMaxX < nMaxX)
			nMaxX = nNewMaxX;
		if (nNewMaxY < nMaxY)
			nMaxY = nNewMaxY;
	}

	sint32 nTotalFile = (1 + nMaxY - nMinY) * (1 + nMaxX - nMinX);
	sint32 nCurrentFile = 0;

	vector<bool> ZoneTreated;
	ZoneTreated.resize(nTotalFile, false);

	for (sint32 j = nMinY; j <= nMaxY; ++j)
	{
		for (sint32 i = nMinX; i <= nMaxX; ++i)
		if (!ZoneTreated[i-nMinX+(j-nMinY)*(1+nMaxX-nMinX)])
		{
			++nCurrentFile;
			if (_ExportCB != NULL)
				_ExportCB->dispPassProgress(((float)nCurrentFile)/((float)nTotalFile));

			const string &SrcZoneName = _Options->ZoneRegion->getName(i,j);

			if ((SrcZoneName == STRING_OUT_OF_BOUND) ||
				(SrcZoneName == STRING_UNUSED))
				continue;

			treatPattern (i, j, ZoneTreated, nMinX, nMinY, 1+nMaxX-nMinX);

			if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
				break;
		}
		if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
			break;
	}

	exportCMBAndAdditionnalIGs();



	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Finished");
	delete _ZeZoneBank;
	delete _ZeTileBank;

	return true;
}

// ---------------------------------------------------------------------------
void CExport::treatPattern (sint32 x, sint32 y,
							vector<bool> &ZoneTreated, sint32 nMinX, sint32 nMinY, sint32 nStride)
{

	CZoneRegion *pZR = _Options->ZoneRegion;
	const string &rSZone = pZR->getName (x, y);
	CZoneBankElement *pZBE = _ZeZoneBank->getElementByZoneName (rSZone);

	if (pZBE == NULL)
		return;

	sint32 sizeX = pZBE->getSizeX(), sizeY = pZBE->getSizeY();
	sint32 posX = pZR->getPosX (x, y), posY = pZR->getPosY (x, y);
	uint8 rot = pZR->getRot (x, y);
	uint8 flip = pZR->getFlip (x, y);
	sint32 i, j;
	sint32 deltaX, deltaY;

	if (flip == 0)
	{
		switch (rot)
		{
			case 0: deltaX = -posX; deltaY = -posY; break;
			case 1: deltaX = -(sizeY-1-posY); deltaY = -posX; break;
			case 2: deltaX = -(sizeX-1-posX); deltaY = -(sizeY-1-posY); break;
			case 3: deltaX = -posY; deltaY = -(sizeX-1-posX); break;
		}
	}
	else
	{
		switch (rot)
		{
			case 0: deltaX = -(sizeX-1-posX); deltaY = -posY; break;
			case 1: deltaX = -(sizeY-1-posY); deltaY = -(sizeX-1-posX); break;
			case 2: deltaX = -posX; deltaY = -(sizeY-1-posY); break;
			case 3: deltaX = -posY; deltaY = -posX; break;
		}
	}

	SPiece sMask;
	sMask.Tab.resize (sizeX*sizeY);
	for(i = 0; i < sizeX*sizeY; ++i)
		sMask.Tab[i] = pZBE->getMask()[i];
	sMask.w = sizeX;
	sMask.h = sizeY;
	sMask.rotFlip (rot, flip);

	// Check if we have to export the zones
	bool bHaveToExportZone = false;
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		// If date of the piece is newer than date of the final zones
		string finalZoneName = _Options->OutZoneDir + string("\\") + getZoneNameFromXY(x+deltaX+i, y+deltaY+j) + string(".zonel");
		if (!CTools::fileExist(finalZoneName))
		{
			finalZoneName = _Options->OutZoneDir + string("\\") + getZoneNameFromXY(x+deltaX+i, y+deltaY+j) + string(".zone");
			if (!CTools::fileExist(finalZoneName))
			{
				bHaveToExportZone = true; // A file do not exist -> export it

				if (_ExportCB != NULL)
					_ExportCB->dispInfo (string("finalZone do not exist"));


				continue;
			}
		}
		string refZoneName = _Options->RefZoneDir + string("\\") + rSZone + string(".zone");

		if (!CTools::fileExist(refZoneName))
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning(string("RefZone do not exist.") + refZoneName);
			return;
		}

		if (CTools::fileDateCmp(refZoneName, finalZoneName) > 0)
		{
			bHaveToExportZone = true;

			if (_ExportCB != NULL)
				_ExportCB->dispInfo (string("RefZone newer than finalZone."));

			break;
		}
		// Or if the date of the cell is newer than the date of the final zone
		uint32 cellDateLow = pZR->getDate(x+deltaX+i, y+deltaY+j, 0);
		uint32 cellDateHigh = pZR->getDate(x+deltaX+i, y+deltaY+j, 1);

		if (CTools::fileDateCmp(finalZoneName, cellDateLow, cellDateHigh) < 0)
		{
			bHaveToExportZone = true;

			if (_ExportCB != NULL)
				_ExportCB->dispInfo (string("Cell date newer"));

			break;
		}
	}

	// Check if we have to export the igs
	bool bHaveToExportIG = false;
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		// If date of the piece is newer than date of the final zones
		string finalIGName = _Options->OutIGDir + string("\\") + getZoneNameFromXY(x+deltaX+i, y+deltaY+j) + string(".ig");
		if (!CTools::fileExist(finalIGName))
		{
			finalIGName = _Options->OutIGDir + string("\\") + getZoneNameFromXY(x+deltaX+i, y+deltaY+j) + string(".ig");
			if (!CTools::fileExist(finalIGName))
			{
				bHaveToExportIG = true; // A file do not exist -> export it

				if (_ExportCB != NULL)
					_ExportCB->dispInfo (string("final instance group do not exist"));


				continue;
			}
		}
		string refIGName = _Options->RefIGDir + string("\\") + rSZone + string(".ig");

		if (!CTools::fileExist(refIGName))
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning(string("RefIG do not exist.") + refIGName);
			continue;
		}

		if (CTools::fileDateCmp(refIGName, finalIGName) > 0)
		{
			bHaveToExportIG = true;

			if (_ExportCB != NULL)
				_ExportCB->dispInfo (string("RefIG newer than finalIG."));

			break;
		}
		// Or if the date of the cell is newer than the date of the final ig
		uint32 cellDateLow = pZR->getDate(x+deltaX+i, y+deltaY+j, 0);
		uint32 cellDateHigh = pZR->getDate(x+deltaX+i, y+deltaY+j, 1);

		if (CTools::fileDateCmp(finalIGName, cellDateLow, cellDateHigh) < 0)
		{
			bHaveToExportIG = true;

			if (_ExportCB != NULL)
				_ExportCB->dispInfo (string("Cell date newer"));

			break;
		}
	}


	if (bHaveToExportZone == false && bHaveToExportIG == false)
		return;

	// Put the big zone at the right position
	CZone BigZone;


	// 1 - Load the zone and IG
	string BigZoneFileName;

	if (bHaveToExportZone)
	{
		try
		{
			BigZoneFileName = _Options->RefZoneDir + string("\\") + rSZone + string(".zone");
			CIFile inFile ;
			if (inFile.open(BigZoneFileName))
			{
				BigZone.serial (inFile);
			}
			else
			{
				_ExportCB->dispWarning(string("reference zone " + BigZoneFileName + " does not exists, skipping."));
				bHaveToExportZone = false;
			}
		}
		catch (const Exception &e)
		{
			if (_ExportCB != NULL)
			{
				_ExportCB->dispWarning (string("Cant load zone : ") + BigZoneFileName);
				_ExportCB->dispWarning (string("Reason : ") + e.what());
			}
			bHaveToExportZone = false;
		}
	}

	CInstanceGroup bigIG;
	string bigIGFileName;

	if (bHaveToExportIG)
	{
		try
		{
			bigIGFileName = _Options->RefIGDir + string("\\") + rSZone + string(".ig");
			CIFile inFile;
			if (inFile.open(bigIGFileName))
			{
				bigIG.serial (inFile);
			}
			else
			{
				// _ExportCB->dispWarning(string("reference ig " + bigIGFileName + " does not exists, skipping."));
				bHaveToExportIG = false;
			}
		}
		catch (const Exception &e)
		{
			if (_ExportCB != NULL)
			{
				_ExportCB->dispWarning (string("ERROR: Cant load ig : ") + bigIGFileName);
				_ExportCB->dispWarning (string("ERROR: Reason : ") + e.what());
			}
			bHaveToExportIG = false;
		}
	}

	// 1bis - Copy the zone as no heightmap
	CZone BigZoneNoHeightmap = BigZone;

	// 2 - Transform zone / ig
		// zone
	    if (bHaveToExportZone)
		{
			if (flip == 0)
			{
				switch (rot)
				{
					case 0:
						transformZone (BigZone, x+deltaX, y+deltaY, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX, y+deltaY, rot, flip, false);
					break;
					case 1:
						transformZone (BigZone, x+deltaX+sizeY, y+deltaY, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX+sizeY, y+deltaY, rot, flip, false);
					break;
					case 2:
						transformZone (BigZone, x+deltaX+sizeX, y+deltaY+sizeY, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX+sizeX, y+deltaY+sizeY, rot, flip, false);
					break;
					case 3:
						transformZone (BigZone, x+deltaX, y+deltaY+sizeX, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX, y+deltaY+sizeX, rot, flip, false);
					break;
				}
			}
			else
			{
				switch (rot)
				{
					case 0:
						transformZone (BigZone, x+deltaX+sizeX, y+deltaY, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX+sizeX, y+deltaY, rot, flip, false);
					break;
					case 1:
						transformZone (BigZone, x+deltaX+sizeY, y+deltaY+sizeX, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX+sizeY, y+deltaY+sizeX, rot, flip, false);
					break;
					case 2:
						transformZone (BigZone, x+deltaX, y+deltaY+sizeY, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX, y+deltaY+sizeY, rot, flip, false);
					break;
					case 3:
						transformZone (BigZone, x+deltaX, y+deltaY, rot, flip, true);
						transformZone (BigZoneNoHeightmap, x+deltaX, y+deltaY, rot, flip, false);
					break;
				}
			}
		}

		if (bHaveToExportIG)
		{
			if (flip == 0)
			{
				switch (rot)
				{
					case 0:
						transformIG (bigIG, x+deltaX, y+deltaY, rot, flip);
					break;
					case 1:
						transformIG (bigIG, x+deltaX+sizeY, y+deltaY, rot, flip);
					break;
					case 2:
						transformIG (bigIG, x+deltaX+sizeX, y+deltaY+sizeY, rot, flip);
					break;
					case 3:
						transformIG (bigIG, x+deltaX, y+deltaY+sizeX, rot, flip);
					break;
				}
			}
			else
			{
				switch (rot)
				{
					case 0:
						transformIG (bigIG, x+deltaX+sizeX, y+deltaY, rot, flip);
					break;
					case 1:
						transformIG (bigIG, x+deltaX+sizeY, y+deltaY+sizeX, rot, flip);
					break;
					case 2:
						transformIG (bigIG, x+deltaX, y+deltaY+sizeY, rot, flip);
					break;
					case 3:
						transformIG (bigIG, x+deltaX, y+deltaY, rot, flip);
					break;
				}
			}
		}

	// 3 - Add the global color map
	addColorMap (BigZone);

	// 4 - Cut the big zone into a set of unit zones

	// - Build patch information

	// Retrieve source patches
	vector<CPatchInfo>		SrcPI;
	vector<CBorderVertex>	BorderVertices;
	BigZone.retrieve (SrcPI, BorderVertices);
	vector<CPatchInfo>		SrcPINoHeightmap;
	vector<CBorderVertex>	BorderVerticesNoHeightmap;
	BigZoneNoHeightmap.retrieve (SrcPINoHeightmap, BorderVerticesNoHeightmap);

	// Resize bool array
	vector<bool> PatchTransfered; // Is the patch n is transfered in a zoneUnit ?
	PatchTransfered.resize (SrcPI.size(), false);

	// Patch bb
	vector<CAABBox> bb;
	bb.resize (SrcPI.size());
	for (i = 0; i < (sint)SrcPI.size(); ++i)
	{
		CPatchInfo &rPI = SrcPI[i];

		bb[i].setCenter (rPI.Patch.Vertices[0]);

		for (j = 0; j < 4; ++j)
			bb[i].extend (rPI.Patch.Vertices[j]);

		for (j = 0; j < 4; ++j)
			bb[i].extend (rPI.Patch.Interiors[j]);

		for (j = 0; j < 8; ++j)
			bb[i].extend (rPI.Patch.Tangents[j]);
	}

	bool first = true;
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		CZone UnitZone;
		CZone UnitZoneLighted;
		CZone UnitZoneNoHeightmap;

		CInstanceGroup unitIG;

		if (bHaveToExportZone)
		{
			// Put all the patches contained in the square ... in the unit zone
			cutZone (BigZone, BigZoneNoHeightmap, UnitZone, UnitZoneNoHeightmap, x+deltaX+i, y+deltaY+j, PatchTransfered, bb, SrcPI, SrcPINoHeightmap,
				sMask,  BorderVertices, BorderVerticesNoHeightmap, x+deltaX, y+deltaY);
			if (_Options->Light > 0)
				light (UnitZoneLighted, UnitZone);
			else
				UnitZoneLighted = UnitZone;
		}

		if (bHaveToExportIG)
		{
			// Put all the instances contained in the square ... in the unit zone
			cutIG (bigIG, unitIG, x+deltaX+i, y+deltaY+j, sMask, first, x+deltaX, y+deltaY);
		}

		if (bHaveToExportZone)
		{
			// Save the zone
			string DstZoneFileName;
			try
			{
				// Delete the .zone and .zonel file
				DstZoneFileName = getZoneNameFromXY(x+deltaX+i, y+deltaY+j);
				DstZoneFileName = _Options->OutZoneDir + string("/") + DstZoneFileName;
				string sTmp = DstZoneFileName + string(".zone");
				CFile::deleteFile (sTmp);
				DstZoneFileName = DstZoneFileName + string(".zonel");
				CFile::deleteFile (DstZoneFileName);

				// Delete the .zonenh file
				string DstZoneNoHeightmapFileName = _Options->OutZoneDir + string("/") + getZoneNameFromXY(x+deltaX+i, y+deltaY+j) + string(".zonenh");
				CFile::deleteFile (DstZoneNoHeightmapFileName);

				COFile outFile (DstZoneFileName);
				UnitZoneLighted.serial (outFile);
				if (_ExportCB != NULL)
					_ExportCB->dispInfo (string("Writing ") + getZoneNameFromXY(x+deltaX+i, y+deltaY+j) + ".zonel");

				COFile outFileNH (DstZoneNoHeightmapFileName);
				UnitZoneNoHeightmap.serial (outFileNH);
			}
			catch (const Exception &)
			{
				if (_ExportCB != NULL)
					_ExportCB->dispWarning (string("Cant write ") + DstZoneFileName);
			}
		}

		if (bHaveToExportIG)
		{
			// Save the ig
			string dstIGFileName;
			try
			{
				dstIGFileName = getZoneNameFromXY(x+deltaX+i, y+deltaY+j);
				dstIGFileName = _Options->OutIGDir + string("/") + dstIGFileName + string(".ig");
				CFile::deleteFile (dstIGFileName);
				COFile outFile (dstIGFileName);
				unitIG.serial(outFile);
				if (_ExportCB != NULL)
					_ExportCB->dispInfo (string("Writing ") + getZoneNameFromXY(x+deltaX+i, y+deltaY+j) + ".ig");
			}
			catch (const Exception &)
			{
				if (_ExportCB != NULL)
					_ExportCB->dispWarning (string("Cant write ") + dstIGFileName);
			}
		}

		// Set the zone as unused to not treat it the next time
		ZoneTreated[(x+deltaX+i)-nMinX + ((y+deltaY+j)-nMinY) * nStride] = true;

		if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
			return;

		first = false;
	}

}

// ---------------------------------------------------------------------------
// Tile conversion
int TransitionFlipLR[48] =
{
	0,	// 0
	4,	// 1
	5,	// 2
	27,	// 3
	1,	// 4
	2,	// 5
	6,	// 6
	34,	// 7
	11,	// 8
	33,	// 9
	31,	// 10
	8,	// 11
	13,	// 12
	12,	// 13
	47,	// 14
	40,	// 15
	39,	// 16
	20,	// 17
	46,	// 18
	45,	// 19
	17,	// 20
	43,	// 21
	42,	// 22
	41,	// 23
	24,	// 24
	28,	// 25
	29,	// 26
	3,	// 27
	25,	// 28
	26,	// 29
	30,	// 30
	10,	// 31
	35,	// 32
	9,	// 33
	7,	// 34
	32,	// 35
	37,	// 36
	36,	// 37
	44,	// 38
	16,	// 39
	15,	// 40
	23,	// 41
	22,	// 42
	21,	// 43
	38,	// 44
	19,	// 45
	18,	// 46
	14	// 47
};

int TransitionFlipUD[48] =
{
	24,	// 0
	28,	// 1
	29,	// 2
	3,	// 3
	25,	// 4
	26,	// 5
	30,	// 6
	10,	// 7
	35,	// 8
	9,	// 9
	7,	// 10
	32,	// 11
	37,	// 12
	36,	// 13
	23,	// 14
	16,	// 15
	15,	// 16
	44,	// 17
	22,	// 18
	21,	// 19
	38,	// 20
	19,	// 21
	18,	// 22
	14,	// 23
	0,	// 24
	4,	// 25
	5,	// 26
	27,	// 27
	1,	// 28
	2,	// 29
	6,	// 30
	34,	// 31
	11,	// 32
	33,	// 33
	31,	// 34
	8,	// 35
	13,	// 36
	12,	// 37
	20,	// 38
	40,	// 39
	39,	// 40
	47,	// 41
	46,	// 42
	45,	// 43
	17,	// 44
	43,	// 45
	42,	// 46
	41	// 47
};

int TransitionRotCCW[48] =
{
	27,	// 0
	28,	// 1
	29,	// 2
	0,	// 3
	1,	// 4
	2,	// 5
	33,	// 6
	34,	// 7
	35,	// 8
	6,	// 9
	7,	// 10
	8,	// 11
	39,	// 12
	40,	// 13
	17,	// 14
	12,	// 15
	13,	// 16
	41,	// 17
	45,	// 18
	46,	// 19
	47,	// 20
	18,	// 21
	19,	// 22
	20,	// 23
	3,	// 24
	4,	// 25
	5,	// 26
	24,	// 27
	25,	// 28
	26,	// 29
	9,	// 30
	10,	// 31
	11,	// 32
	30,	// 33
	31,	// 34
	32,	// 35
	15,	// 36
	16,	// 37
	14,	// 38
	36,	// 39
	37,	// 40
	38,	// 41
	21,	// 42
	22,	// 43
	23,	// 44
	42,	// 45
	43,	// 46
	44	// 47
};

// ---------------------------------------------------------------------------
// Come from rpo2nel.cpp
// ---------------------------------------------------------------------------
bool transformTile (const CTileBank &bank, uint &tile, uint &tileRotation, bool symmetry, uint rotate)
{
	// Tile exist ?
	if ( (rotate!=0) || symmetry )
	{
		if (tile < (uint)bank.getTileCount())
		{
			// Get xref
			int tileSet;
			int number;
			CTileBank::TTileType type;

			// Get tile xref
			bank.getTileXRef ((int)tile, tileSet, number, type);

			// Transition ?
			if (type == CTileBank::transition)
			{
				// Number should be ok
				nlassert (number>=0);
				nlassert (number<CTileSet::count);

				// Tlie set number
				const CTileSet *pTileSet = bank.getTileSet (tileSet);

				// Get border desc
				CTileSet::TFlagBorder oriented[4] =
				{
					pTileSet->getOrientedBorder (CTileSet::left, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::left)),
					pTileSet->getOrientedBorder (CTileSet::bottom, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::bottom)),
					pTileSet->getOrientedBorder (CTileSet::right, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::right)),
					pTileSet->getOrientedBorder (CTileSet::top, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::top))
				};

				// Symmetry ?
				if (symmetry)
				{
					CTileSet::TFlagBorder tmp = oriented[(0)&3];
					oriented[(0)&3] = CTileSet::getInvertBorder (oriented[(2)&3]);
					oriented[(2)&3] = CTileSet::getInvertBorder (tmp);
					oriented[(1)&3] = CTileSet::getInvertBorder (oriented[(1)&3]);
					oriented[(3)&3] = CTileSet::getInvertBorder (oriented[(3)&3]);
				}

				// Rotation
				CTileSet::TFlagBorder edges[4];
				edges[0] = pTileSet->getOrientedBorder (CTileSet::left, oriented[(0 + rotate )&3]);
				edges[1] = pTileSet->getOrientedBorder (CTileSet::bottom, oriented[(1 + rotate )&3]);
				edges[2] = pTileSet->getOrientedBorder (CTileSet::right, oriented[(2 + rotate )&3]);
				edges[3] = pTileSet->getOrientedBorder (CTileSet::top, oriented[(3 + rotate )&3]);

				// Get the good tile number
				CTileSet::TTransition transition = pTileSet->getTransitionTile (edges[3], edges[1], edges[0], edges[2]);
				nlassert ((CTileSet::TTransition)transition != CTileSet::notfound);
				tile = (uint)(pTileSet->getTransition (transition)->getTile ());
			}

			// Transform rotation
			if (symmetry)
				tileRotation = (4-tileRotation)&3;
			tileRotation += rotate;
			tileRotation &= 3;
		}
		else
			return false;
	}

	// Ok
	return true;
}

// ---------------------------------------------------------------------------
// Come from rpo2nel.cpp
// ---------------------------------------------------------------------------
void transform256Case (const CTileBank &bank, uint &case256, uint tileRotation, bool symmetry, uint rotate)
{
	// Tile exist ?
	if ( (rotate!=0) || symmetry )
	{
		// Remove its rotation
		case256 += tileRotation;
		case256 &= 3;

		// Symmetry ?
		if (symmetry)
		{
			// Take the symmetry
			uint symArray[4] = {3, 2, 1, 0};
			case256 = symArray[case256];
		}

		// Rotation ?
		case256 -= rotate + tileRotation;
		case256 &= 3;
	}
}

// ---------------------------------------------------------------------------

void CExport::addColorMap (CZone &zeZone)
{
	// Colormap available ?
	if (_ColorMap)
	{
		// Conversion nPosX,nPosY to Zone Coordinate ZoneX, ZoneY
		vector<CPatchInfo>		PatchInfos;
		vector<CBorderVertex>	BorderVertices;

		// Retrieve the zone
		zeZone.retrieve (PatchInfos, BorderVertices);

		// Apply the Colormap to all tile color.
		// --------
		uint i;
		for (i = 0; i < PatchInfos.size(); ++i)
		{
			CPatchInfo &rPI = PatchInfos[i];

			// Get size
			uint sizeU = rPI.OrderS+1;
			uint sizeV = rPI.OrderT+1;

			// For each color tiles
			uint u, v;
			for (v = 0; v < sizeV; v++)
			for (u = 0; u < sizeU; u++)
			{
				// Compute the coordinate for this color tile
				CVector pos = rPI.Patch.eval ((float)u/(float)(sizeU-1), (float)v/(float)(sizeV-1));

				// Get the color
				CRGBAF colorf = getColor (pos.x, pos.y);
				clamp (colorf.R, 0.f, 255.f);
				clamp (colorf.G, 0.f, 255.f);
				clamp (colorf.B, 0.f, 255.f);
				clamp (colorf.A, 0.f, 255.f);
				CRGBA color = CRGBA ((uint8)colorf.R, (uint8)colorf.G, (uint8)colorf.B, (uint8)colorf.A);

				// Destination color
				uint16 &dest = rPI.TileColors[u+v*(sizeU)].Color565;

				// Original color
				CRGBA src;
				src.set565 (dest);

				// Blend the original and new color
				CRGBA tmp;
				tmp.blendFromui (src, color, ((uint)color.A) * 256 / 255);
				dest = tmp.get565 ();
			}
		}

		zeZone.build (zeZone.getZoneId (), PatchInfos, BorderVertices);
	}
}

// ---------------------------------------------------------------------------
void CExport::transformZone (CZone &zeZone, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip, bool computeHeightmap)
{
	// Conversion nPosX,nPosY to Zone Coordinate ZoneX, ZoneY
	uint32 i, j;
	vector<CPatchInfo>		PatchInfos;
	vector<CBorderVertex>	BorderVertices;

	sint32 nZoneX = nPosX;
	sint32 nZoneY = -1 - nPosY;
	uint16 nZoneId = nZoneX+(nZoneY*256);

	zeZone.retrieve (PatchInfos, BorderVertices);

	nlassert (BorderVertices.size() == 0);

	CMatrix Transfo;
	Transfo.setRot (CQuat(CVector::K, (float)(nRot * Pi / 2.0f)));
	Transfo.setPos (CVector(nPosX*_Options->CellSize, (nPosY)*_Options->CellSize, 0.0f));

	if (nFlip != 0)
		nFlip = 1;

	if (nFlip == 1)
		Transfo.scale(CVector(-1.0f, 1.0f, 1.0f));

	// Transform the Patchs, and apply HeightMap.
	//=============================


	// 0. Transfrom zone indexes
	//=======================
	{
		CMatrix invTransfo = Transfo;
		invTransfo.invert ();

		NL3D::CZoneSymmetrisation zoneSymmetry;
		if (! CPatchInfo::transform (PatchInfos, zoneSymmetry, *_ZeTileBank, nFlip != 0, nRot, _Options->CellSize, _Options->Threshold, invTransfo) )
		{
			std::string name = getZoneNameFromXY (nPosX, nPosY);

			// Can't transform the zone
			nlwarning ("ERROR: can't transform zone %s with sym:%d rot:%d", name.c_str (), nFlip, nRot);
		}
	}

	// 1. Transfom ligo zone in world
	//=======================
	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];

		rPI.Patch.applyMatrix (Transfo);
	}

	// Bkup the original patchs.
	vector<CPatchInfo>		oldPatchInfos= PatchInfos;

	// 2. Apply the Heighmap to all vertices/tangents/interiors.
	// --------
	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];

		if (computeHeightmap)
		{
			// Elevate the vertices.
			CVector		verticesBeforeHeightMap[4];
			for (j = 0; j < 4; ++j)
			{
				verticesBeforeHeightMap[j]= rPI.Patch.Vertices[j];
				rPI.Patch.Vertices[j].z += getHeight(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);
			}

			// Interior and tangent are rotated to follow the heightmap normal, avoiding the "Stair Effect".
			// Compute the matrix to apply to interiors and tangents.
			CMatrix		tgMatrix[4];
			for (j = 0; j < 4; ++j)
			{
				// compute the normal of the heightmap.
				CVector		hmapNormal= getHeightNormal(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);

				// Compute the rotation which transforms the original normal: (0,0,1), to this normal.
				CAngleAxis	angleAxis;
				angleAxis.Axis= CVector::K ^ hmapNormal;
				angleAxis.Angle= (float)asin(angleAxis.Axis.norm());
				angleAxis.Axis.normalize();

				// build the matrix which transform the old tgt/interior to his newValue:
				// newVertexPos+ rotate*(oldTgPos-oldVertexPos)
				tgMatrix[j].identity();
				tgMatrix[j].translate(rPI.Patch.Vertices[j]);
				tgMatrix[j].setRot( CQuat(angleAxis) );
				tgMatrix[j].translate(-verticesBeforeHeightMap[j]);
			}

			// For all interior.
			for (j = 0; j < 4; ++j)
				rPI.Patch.Interiors[j]= tgMatrix[j] * rPI.Patch.Interiors[j];

			// when j == 7 or 0 use vertex 0 for delta Z to ensure continuity of normals
			// when j == 1 or 2 use vertex 1
			// when j == 3 or 4 use vertex 2
			// when j == 5 or 6 use vertex 3
			for (j = 0; j < 8; ++j)
			{
				// get the correct vertex
				uint	vertexId= ((j+1)/2)%4;
				// apply the tgMatrix to the tangent
				rPI.Patch.Tangents[j]= tgMatrix[vertexId] * rPI.Patch.Tangents[j];
			}
		}

		for (j = 0; j < 4; ++j)
			rPI.BindEdges[j].ZoneId = nZoneId;
	}

	// 3. For all binds, reset the position of near vertices/tangents/interiors. Must do it at last
	// --------
	bool	bindVertexModified= true;
	// Since this is a recursive problem (binded patchs may bind other patchs), do it unitl all vertices no more move :)
	while(bindVertexModified)
	{
		bindVertexModified= false;
		for (i = 0; i < PatchInfos.size(); ++i)
		{
			CPatchInfo &rPI = PatchInfos[i];

			// For all edges
			for (j = 0; j < 4; ++j)
			{
				uint	numBinds= rPI.BindEdges[j].NPatchs;
				// If this edge is binded on 2 or 4 patches.
				if( numBinds==2 || numBinds==4 )
				{
					// compute the 4 or 8 tangents along the edge (in CCW)
					CVector		subTangents[8];
					computeSubdividedTangents(numBinds, rPI.Patch, j, subTangents);


					// For all vertex to bind: 1 or 3.
					for(uint vb=0; vb<numBinds-1; vb++)
					{
						// compute the s/t coordinate
						float	bindS, bindT;
						// 0.5, or 0.25, 0.5, 0.75
						float		ec= (float)(vb+1)/(float)numBinds;
						switch(j)
						{
						case 0:	bindS= 0;	 bindT= ec; break;
						case 1:	bindS= ec;	 bindT= 1; break;
						case 2:	bindS= 1;	 bindT= 1-ec; break;
						case 3:	bindS= 1-ec; bindT= 0; break;
						}


						// compute the vertex position from big patch.
						CVector		bindedPos;
						bindedPos= rPI.Patch.eval(bindS, bindT);

						// Compute a TgSpace matrix around this position.
						CMatrix		oldTgSpace;
						CMatrix		newTgSpace;

						// Build the original tgtSpace (patch before deformation)
						oldTgSpace.setRot(oldPatchInfos[i].Patch.evalTangentS(bindS, bindT),
							oldPatchInfos[i].Patch.evalTangentT(bindS, bindT),
							oldPatchInfos[i].Patch.evalNormal(bindS, bindT));
						oldTgSpace.normalize(CMatrix::ZYX);
						oldTgSpace.setPos( oldPatchInfos[i].Patch.eval(bindS, bindT) );

						// Build the new tgtSpace
						newTgSpace.setRot(rPI.Patch.evalTangentS(bindS, bindT),
							rPI.Patch.evalTangentT(bindS, bindT),
							rPI.Patch.evalNormal(bindS, bindT));
						newTgSpace.normalize(CMatrix::ZYX);
						newTgSpace.setPos( bindedPos );


						// apply to the 2 smaller binded patchs which share this vertex.
						uint	edgeToModify;
						sint	ngbId;
						CVector	bindedTangent;

						// The first patch (CCW) must change the vertex which starts on the edge (CCW)
						edgeToModify= rPI.BindEdges[j].Edge[vb];
						// get the tangent to set
						bindedTangent= subTangents[vb*2+1];
						// get the patch id.
						ngbId= rPI.BindEdges[j].Next[vb];
						bindVertexModified|= applyVertexBind(PatchInfos[ ngbId ], oldPatchInfos[ ngbId ], edgeToModify,
							true, oldTgSpace, newTgSpace, bindedPos, bindedTangent);

						// The second patch (CCW) must change the vertex which ends on the edge (CCW)
						edgeToModify= rPI.BindEdges[j].Edge[vb+1];
						// get the tangent to set
						bindedTangent= subTangents[vb*2+2];
						// get the patch id.
						ngbId= rPI.BindEdges[j].Next[vb+1];
						bindVertexModified|= applyVertexBind(PatchInfos[ ngbId ], oldPatchInfos[ ngbId ], edgeToModify,
							false, oldTgSpace, newTgSpace, bindedPos, bindedTangent);
					}
				}
			}
		}

	}

	zeZone.build (nZoneId, PatchInfos, BorderVertices);
}

// ---------------------------------------------------------------------------
/*void CExport::transformZone (CZone &zeZone, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip)
{
	// Conversion nPosX,nPosY to Zone Coordinate ZoneX, ZoneY
	uint32 i, j, k, pass;
	vector<CPatchInfo>		PatchInfos;
	vector<CBorderVertex>	BorderVertices;

	sint32 nZoneX = nPosX;
	sint32 nZoneY = -1 - nPosY;
	uint16 nZoneId = nZoneX+(nZoneY*256);

	zeZone.retrieve (PatchInfos, BorderVertices);

	nlassert (BorderVertices.size() == 0);

	CMatrix Transfo;
	Transfo.setRot (CQuat(CVector::K, (float)(nRot * Pi / 2.0f)));
	Transfo.setPos (CVector(nPosX*_Options->CellSize, (nPosY)*_Options->CellSize, 0.0f));

	if (nFlip != 0)
		nFlip = 1;

	if (nFlip == 1)
		Transfo.scale(CVector(-1.0f, 1.0f, 1.0f));

	// Transform the Patchs, and apply HeightMap.
	//=============================

	// Bkup the original patchs.
	vector<CPatchInfo>		oldPatchInfos= PatchInfos;


	// 1. Apply the Heighmap to all vertices/tangents/interiors.
	// --------
	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];

		rPI.Patch.applyMatrix (Transfo);

		// Elevate the vertices.
		CVector		verticesBeforeHeightMap[4];
		for (j = 0; j < 4; ++j)
		{
			verticesBeforeHeightMap[j]= rPI.Patch.Vertices[j];
			rPI.Patch.Vertices[j].z += getHeight(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);
		}

		// Interior and tangent are rotated to follow the heightmap normal, avoiding the "Stair Effect".
		// Compute the matrix to apply to interiors and tangents.
		CMatrix		tgMatrix[4];
		for (j = 0; j < 4; ++j)
		{
			// compute the normal of the heightmap.
			CVector		hmapNormal= getHeightNormal(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);

			// Compute the rotation which transforms the original normal: (0,0,1), to this normal.
			CAngleAxis	angleAxis;
			angleAxis.Axis= CVector::K ^ hmapNormal;
			angleAxis.Angle= (float)asin(angleAxis.Axis.norm());
			angleAxis.Axis.normalize();

			// build the matrix which transform the old tgt/interior to his newValue:
			// newVertexPos+ rotate*(oldTgPos-oldVertexPos)
			tgMatrix[j].identity();
			tgMatrix[j].translate(rPI.Patch.Vertices[j]);
			tgMatrix[j].setRot( CQuat(angleAxis) );
			tgMatrix[j].translate(-verticesBeforeHeightMap[j]);
		}

		// For all interior.
		for (j = 0; j < 4; ++j)
			rPI.Patch.Interiors[j]= tgMatrix[j] * rPI.Patch.Interiors[j];

		// when j == 7 or 0 use vertex 0 for delta Z to ensure continuity of normals
		// when j == 1 or 2 use vertex 1
		// when j == 3 or 4 use vertex 2
		// when j == 5 or 6 use vertex 3
		for (j = 0; j < 8; ++j)
		{
			// get the correct vertex
			uint	vertexId= ((j+1)/2)%4;
			// apply the tgMatrix to the tangent
			rPI.Patch.Tangents[j]= tgMatrix[vertexId] * rPI.Patch.Tangents[j];
		}

		for (j = 0; j < 4; ++j)
			rPI.BindEdges[j].ZoneId = nZoneId;
	}


	// 2. For all binds, reset the position of near vertices/tangents/interiors. Must do it at last
	// --------
	bool	bindVertexModified= true;
	// Since this is a recursive problem (binded patchs may bind other patchs), do it unitl all vertices no more move :)
	while(bindVertexModified)
	{
		bindVertexModified= false;
		for (i = 0; i < PatchInfos.size(); ++i)
		{
			CPatchInfo &rPI = PatchInfos[i];

			// For all edges
			for (j = 0; j < 4; ++j)
			{
				uint	numBinds= rPI.BindEdges[j].NPatchs;
				// If this edge is binded on 2 or 4 patches.
				if( numBinds==2 || numBinds==4 )
				{
					// compute the 4 or 8 tangents along the edge (in CCW)
					CVector		subTangents[8];
					computeSubdividedTangents(numBinds, rPI.Patch, j, subTangents);


					// For all vertex to bind: 1 or 3.
					for(uint vb=0; vb<numBinds-1; vb++)
					{
						// compute the s/t coordinate
						float	bindS, bindT;
						// 0.5, or 0.25, 0.5, 0.75
						float		ec= (float)(vb+1)/(float)numBinds;
						switch(j)
						{
						case 0:	bindS= 0;	 bindT= ec; break;
						case 1:	bindS= ec;	 bindT= 1; break;
						case 2:	bindS= 1;	 bindT= 1-ec; break;
						case 3:	bindS= 1-ec; bindT= 0; break;
						}


						// compute the vertex position from big patch.
						CVector		bindedPos;
						bindedPos= rPI.Patch.eval(bindS, bindT);

						// Compute a TgSpace matrix around this position.
						CMatrix		oldTgSpace;
						CMatrix		newTgSpace;

						// Build the original tgtSpace (patch before deformation)
						oldTgSpace.setRot(oldPatchInfos[i].Patch.evalTangentS(bindS, bindT),
							oldPatchInfos[i].Patch.evalTangentT(bindS, bindT),
							oldPatchInfos[i].Patch.evalNormal(bindS, bindT));
						oldTgSpace.normalize(CMatrix::ZYX);
						oldTgSpace.setPos( oldPatchInfos[i].Patch.eval(bindS, bindT) );

						// Build the new tgtSpace
						newTgSpace.setRot(rPI.Patch.evalTangentS(bindS, bindT),
							rPI.Patch.evalTangentT(bindS, bindT),
							rPI.Patch.evalNormal(bindS, bindT));
						newTgSpace.normalize(CMatrix::ZYX);
						newTgSpace.setPos( bindedPos );


						// apply to the 2 smaller binded patchs which share this vertex.
						uint	edgeToModify;
						sint	ngbId;
						CVector	bindedTangent;

						// The first patch (CCW) must change the vertex which starts on the edge (CCW)
						edgeToModify= rPI.BindEdges[j].Edge[vb];
						// get the tangent to set
						bindedTangent= subTangents[vb*2+1];
						// get the patch id.
						ngbId= rPI.BindEdges[j].Next[vb];
						bindVertexModified|= applyVertexBind(PatchInfos[ ngbId ], oldPatchInfos[ ngbId ], edgeToModify,
							true, oldTgSpace, newTgSpace, bindedPos, bindedTangent);

						// The second patch (CCW) must change the vertex which ends on the edge (CCW)
						edgeToModify= rPI.BindEdges[j].Edge[vb+1];
						// get the tangent to set
						bindedTangent= subTangents[vb*2+2];
						// get the patch id.
						ngbId= rPI.BindEdges[j].Next[vb+1];
						bindVertexModified|= applyVertexBind(PatchInfos[ ngbId ], oldPatchInfos[ ngbId ], edgeToModify,
							false, oldTgSpace, newTgSpace, bindedPos, bindedTangent);
					}
				}
			}
		}

	}


	// Extra code for Fliping the zone.
	//=============================

	if (nFlip == 1)
	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];

		// Flip the bezier patch (reorder)
		swap(rPI.Patch.Vertices[0], rPI.Patch.Vertices[3]); // A <-> D
		swap(rPI.Patch.Vertices[1], rPI.Patch.Vertices[2]); // B <-> C

		swap(rPI.Patch.Tangents[0], rPI.Patch.Tangents[5]); // ab <-> dc
		swap(rPI.Patch.Tangents[1], rPI.Patch.Tangents[4]); // ba <-> cd
		swap(rPI.Patch.Tangents[2], rPI.Patch.Tangents[3]); // bc <-> cb
		swap(rPI.Patch.Tangents[7], rPI.Patch.Tangents[6]); // ad <-> da

		swap(rPI.Patch.Interiors[0], rPI.Patch.Interiors[3]); // ia <-> id
		swap(rPI.Patch.Interiors[1], rPI.Patch.Interiors[2]); // ib <-> ic

		// Flip the base vertice
		swap(rPI.BaseVertices[0], rPI.BaseVertices[3]);
		swap(rPI.BaseVertices[1], rPI.BaseVertices[2]);

		// Flip bind edge only AB and CD
		swap(rPI.BindEdges[0], rPI.BindEdges[2]);

		// Flip bind edge content only if multiple bind
		for (j = 0; j < 4; j++)
		{
			if (rPI.BindEdges[j].NPatchs == 2)
			{
				swap (rPI.BindEdges[j].Next[0], rPI.BindEdges[j].Next[1]);
				swap (rPI.BindEdges[j].Edge[0], rPI.BindEdges[j].Edge[1]);
			}
			if (rPI.BindEdges[j].NPatchs == 4)
			{
				swap (rPI.BindEdges[j].Next[0], rPI.BindEdges[j].Next[3]);
				swap (rPI.BindEdges[j].Next[1], rPI.BindEdges[j].Next[2]);

				swap (rPI.BindEdges[j].Edge[0], rPI.BindEdges[j].Edge[3]);
				swap (rPI.BindEdges[j].Edge[1], rPI.BindEdges[j].Edge[2]);
			}
		}

		for (j = 0; j < 4; ++j)
		{
			uint32 nNbPatch = rPI.BindEdges[j].NPatchs == 5 ? 1 : rPI.BindEdges[j].NPatchs;
			for (k = 0; k < nNbPatch; ++k)
			{
				if (rPI.BindEdges[j].Edge[k] == 0)
					rPI.BindEdges[j].Edge[k] = 2;
				else if (rPI.BindEdges[j].Edge[k] == 2)
					rPI.BindEdges[j].Edge[k] = 0;
			}
		}

		// Tile switching
		for (j = 0; j < (uint32)(rPI.OrderS/2); ++j)
		{
			for (k = 0; k < rPI.OrderT; ++k)
			{
				swap(rPI.Tiles[j+k*rPI.OrderS], rPI.Tiles[(rPI.OrderS-1-j)+k*rPI.OrderS]);
			}
		}

		for (j = 0; j < (uint32)((rPI.OrderS+1)/2); ++j)
		{
			for (k = 0; k < (uint32)(rPI.OrderT+1); ++k)
			{
				swap(rPI.TileColors [j+k*(rPI.OrderS+1)], rPI.TileColors[(rPI.OrderS-j)+k*(rPI.OrderS+1)]);
			}
		}
	}

	// Extra code for Rotating the zone.
	//=============================

	// Rotate all tile elements in CW (because zones are turned in CCW)
	// If zone flipped rotate tile elements by 180 degrees
	set<string> allnames; // Debug
	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];

		for (j = 0; j < rPI.Tiles.size(); ++j)
		{

			int tileID, tileSet;
			unsigned int tileIDun;
			CTileBank::TTileType type;
			CTileSet *pTS;
			unsigned int nbOfRot;

			// Is the tile is painted ?
			if (rPI.Tiles[j].Tile[0] == 65535)
				continue;

			// Rotate Tiles
			// Invert rotation effect on transition
			for (pass = 0; pass < 3; ++pass)
			{
				if (rPI.Tiles[j].Tile[pass] == NL_TILE_ELM_LAYER_EMPTY)
					break;
				uint8 ori = rPI.Tiles[j].getTileOrient (pass);

				// Invert rotation effect on transition
				_ZeTileBank->getTileXRef(rPI.Tiles[j].Tile[pass], tileSet, tileID, type);
				pTS = _ZeTileBank->getTileSet (tileSet);
				tileIDun = rPI.Tiles[j].Tile[pass];
				nbOfRot = rPI.Tiles[j].getTileOrient (pass);
				if (!pTS->getOriented())
				{
					transformTile (*_ZeTileBank, tileIDun, nbOfRot, (nFlip == 1), (4-nRot)%4);
				}
				else
				{
					transformTile (*_ZeTileBank, tileIDun, nbOfRot, (nFlip == 1), 0);
				}
				rPI.Tiles[j].Tile[pass] = tileIDun;
				// Rotate tile
				rPI.Tiles[j].setTileOrient (pass, nbOfRot);
			}

			bool is256x256;
			uint8 uvOff;
			rPI.Tiles[j].getTile256Info (is256x256, uvOff);
			if (is256x256)
			{
				unsigned int unCase = uvOff;
				transform256Case (*_ZeTileBank, unCase, 0, (nFlip == 1), (4-nRot)%4);
				rPI.Tiles[j].setTile256Info (true, unCase);
			}
		}
	}

	zeZone.build (nZoneId, PatchInfos, BorderVertices);
}*/

/*
CLandscape gLand;
bool gLandInited = false;
*/


// ---------------------------------------------------------------------------
void CExport::buildTransfo(sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip, CMatrix &posTransfo, CQuat &rotTransfo)
{
	// we don't use a single matrix because we don't want to apply a mirror on meshs
	rotTransfo = CQuat(CVector::K, (float)(nRot * Pi / 2.0f));

	posTransfo.setRot(rotTransfo);
	posTransfo.setPos(CVector(nPosX * _Options->CellSize, (nPosY) * _Options->CellSize, 0.0f));
	if (nFlip == 1)
		posTransfo.scale(CVector(-1.0f, 1.0f, 1.0f));
}

// ---------------------------------------------------------------------------
void CExport::transformIG (CInstanceGroup &ig, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip)
{
	uint k;
	CQuat	rotTransfo;
	CMatrix transformation;

	buildTransfo(nPosX, nPosY, nRot, nFlip, transformation, rotTransfo);

	///////////////
	// instances //
	///////////////

	for (k = 0; k < ig.getNumInstance(); ++k)
	{
		/* CInstanceGroup::CInstance &igi = ig.getInstance(k);
		igi.Pos = transformation * igi.Pos;
		igi.Rot = rotTransfo * igi.Rot; */

		// add height map influence
		//igi.Pos.z += getHeight(igi.Pos.x, igi.Pos.y);

		// Instance
		CInstanceGroup::CInstance &igi = ig.getInstance(k);

		// Get the previous instance matrix
		CMatrix previousMt;
		previousMt.identity ();
		previousMt.setRot(igi.Rot);
		previousMt.setPos(igi.Pos);
		previousMt.scale(igi.Scale);

		// Compose the matrix
		previousMt = transformation * previousMt;

		// Extract the rotation and pos
		igi.Rot = previousMt.getRot ();
		igi.Pos = previousMt.getPos ();

		// Extract the scale
		CMatrix scaleMatrix;
		scaleMatrix.identity ();
		scaleMatrix.setRot (igi.Rot);
		scaleMatrix.invert ();
		scaleMatrix = scaleMatrix * previousMt;
		igi.Scale.set (scaleMatrix.getI ().x, scaleMatrix.getJ ().y, scaleMatrix.getK ().z);
	}

	//////////////////
	// point lights //
	//////////////////

	for (k = 0; k < ig.getNumPointLights(); ++k)
	{
		CPointLightNamed		&plm = ig.getPointLightNamed(k);
		plm.setPosition(transformation * plm.getPosition());

		// add height map influence
	//	plm.setPosition(plm.getPosition() + CVector::K * getHeight(plm.getPosition().x, plm.getPosition().y));
	}

}


// ---------------------------------------------------------------------------
void CExport::transformCMB (NLPACS::CCollisionMeshBuild &cmb,sint32 nPosX,sint32 nPosY,uint8 nRot,uint8 nFlip)
{
	CQuat	rotTransfo;
	CMatrix posTransfo;
	buildTransfo(nPosX, nPosY, nRot, nFlip, posTransfo, rotTransfo);
	for(std::vector<NLMISC::CVector>::iterator it = cmb.Vertices.begin(); it != cmb.Vertices.end(); ++it)
	{
		*it = posTransfo * *it;
	}
}

// ---------------------------------------------------------------------------
void CExport::cutZone (NL3D::CZone &bigZone, NL3D::CZone &bigZoneNoHeightmap, NL3D::CZone &unitZone, NL3D::CZone &unitZoneNoHeightmap,
					   sint32 nPosX, sint32 nPosY, vector<bool> &PatchTransfered, const vector<CAABBox> &bb, vector<CPatchInfo> &SrcPI,
					   vector<CPatchInfo> &SrcPINoHeightmap, SPiece &sMask, vector<CBorderVertex> &BorderVertices,
					   vector<CBorderVertex> &BorderVerticesNoHeightmap, sint32 baseX, sint32 baseY)
{
	string DstZoneFileName = getZoneNameFromXY (nPosX, nPosY);

	uint32 i, j, k, l, m;
	vector<CPatchInfo>		DstPI;
	vector<CPatchInfo>		DstPINoHeightmap;

	sint32 nZoneX = nPosX;
	sint32 nZoneY = -1 - nPosY;
	uint16 nZoneId = nZoneX+(nZoneY*256);

	float rMinX = nPosX		* _Options->CellSize;
	float rMaxX = (1+nPosX) * _Options->CellSize;
	float rMinY = nPosY		* _Options->CellSize;
	float rMaxY = (1+nPosY) * _Options->CellSize;

	map<int,int> OldToNewPatchId; // Used to convert old patch id to new patch id

	for (i = 0; i < SrcPI.size(); ++i)
	if (!PatchTransfered[i]) // If patch not already transfered in a zone unit
	{
		CPatchInfo &rPI = SrcPI[i];
		CPatchInfo &rPINoHeightmap = SrcPINoHeightmap[i];

		// Is the Patch contained in the current mask ? Center of bbox tested
		if ((bb[i].getCenter().x >= rMinX)&&(bb[i].getCenter().x <= rMaxX)&&
			(bb[i].getCenter().y >= rMinY)&&(bb[i].getCenter().y <= rMaxY))
		{
			for (j = 0; j < 4; ++j)
			{
				rPI.BindEdges[j].ZoneId = nZoneId;
				rPINoHeightmap.BindEdges[j].ZoneId = nZoneId;
			}

			PatchTransfered[i] = true;
			DstPI.push_back (rPI);
			DstPINoHeightmap.push_back (rPINoHeightmap);
			OldToNewPatchId.insert (pair<int,int>(i, (int)DstPI.size()-1));
		}
	}

	// Look for patches out of the mask attached to a patch in this zone
	bool foundOne;
	do
	{
		foundOne = false;
		for (i = 0; i < SrcPI.size(); ++i)
		{
			// Not yet transfered ?
			if (!PatchTransfered[i]) // If patch not already transfered in a zone unit
			{
				// Get the center of the zone
				sint x = (sint)(floor(bb[i].getCenter().x / _Options->CellSize)) - baseX;
				sint y = (sint)(floor(bb[i].getCenter().y / _Options->CellSize)) - baseY;

				// Not in the mask ?
				bool inTheMask = (x>=0) && (x<(sint)sMask.w) && (y>=0) && (y<(sint)sMask.h);
				if (!inTheMask || !sMask.Tab[x+y*sMask.w])
				{
					// Attached to a patch in this zone ?
					CPatchInfo &rPI = SrcPI[i];
					CPatchInfo &rPINoHeightmap = SrcPINoHeightmap[i];
					for (j = 0; j < 4; ++j)
					{
						// Binded ?
						if (rPI.BindEdges[j].NPatchs)
						{
							// Number of others
							uint otherCount = rPI.BindEdges[j].NPatchs;
							if (rPI.BindEdges[j].NPatchs == 5)
								otherCount = 1;

							// For each other patch
							for (k=0; k<otherCount; k++)
							{
								// This patch is in the current zone ?
								if (OldToNewPatchId.find (rPI.BindEdges[j].Next[k]) != OldToNewPatchId.end())
								{
									// Ok add this patch
									for (l = 0; l < 4; ++l)
									{
										rPI.BindEdges[l].ZoneId = nZoneId;
										rPINoHeightmap.BindEdges[l].ZoneId = nZoneId;
									}

									PatchTransfered[i] = true;
									DstPI.push_back (rPI);
									DstPINoHeightmap.push_back (rPINoHeightmap);
									OldToNewPatchId.insert (pair<int,int>(i, (int)DstPI.size()-1));
									foundOne = true;
									break;
								}
							}
							if (k<otherCount)
								break;
						}
					}
				}
			}
		}
	}
	while (foundOne);

	// Add all patch that are binded to one of those of the DstPI list
	uint32 nPreviousDstPISize = (uint32)DstPI.size();
	for (;;)
	{
		for (i = 0; i < DstPI.size(); ++i)
		{
			for (j = 0; j < 4; ++j)
			{
				if (DstPI[i].BindEdges[j].NPatchs == 5)
				{
					uint next = DstPI[i].BindEdges[j].Next[0];
					if (!PatchTransfered[next])
					{
						CPatchInfo &rPITmp = SrcPI[next];
						CPatchInfo &rPITmpNoHeightmap = SrcPINoHeightmap[next];
						for (k = 0; k < 4; ++k)
						{
							rPITmp.BindEdges[k].ZoneId = nZoneId;
							rPITmpNoHeightmap.BindEdges[k].ZoneId = nZoneId;
						}
						DstPI.push_back (rPITmp);
						DstPINoHeightmap.push_back (rPITmpNoHeightmap);
						OldToNewPatchId.insert (pair<int,int>(next, (int)DstPI.size()-1));
						PatchTransfered[next] = true;
					}
				}

				if ((DstPI[i].BindEdges[j].NPatchs == 2) || (DstPI[i].BindEdges[j].NPatchs == 4))
				{
					for (k = 0; k < DstPI[i].BindEdges[j].NPatchs; ++k)
					{
						uint next = DstPI[i].BindEdges[j].Next[k];
						if (!PatchTransfered[next])
						{
							CPatchInfo &rPITmp = SrcPI[next];
							CPatchInfo &rPITmpNoHeightmap = SrcPINoHeightmap[next];
							for (m = 0; m < 4; ++m)
							{
								rPITmp.BindEdges[m].ZoneId = nZoneId;
								rPITmpNoHeightmap.BindEdges[m].ZoneId = nZoneId;
							}
							DstPI.push_back (rPITmp);
							DstPINoHeightmap.push_back (rPITmpNoHeightmap);
							OldToNewPatchId.insert (pair<int,int>(next, (int)DstPI.size()-1));
							PatchTransfered[next] = true;
						}
					}
				}
			}
		}

		// Do it until no more patch added
		if (nPreviousDstPISize == DstPI.size())
			break;
		nPreviousDstPISize = (uint32)DstPI.size();
	}

	for (i = 0; i < DstPI.size(); ++i)
	{
		CPatchInfo &rPI = DstPI[i];
		CPatchInfo &rPINoHeightmap = DstPINoHeightmap[i];
		for (j = 0; j < 4; ++j)
		{
			if ((rPI.BindEdges[j].NPatchs == 1) || (rPI.BindEdges[j].NPatchs == 5))
			{
				map<int,int>::iterator it = OldToNewPatchId.find (rPI.BindEdges[j].Next[0]);
				if (it == OldToNewPatchId.end())
				{
					if (rPI.BindEdges[j].NPatchs == 5)
					{
						if (_ExportCB != NULL)
							_ExportCB->dispError (string("Continuity problem in zone ") + DstZoneFileName);
					}
					else
					{
						rPI.BindEdges[j].NPatchs = 0;
						rPINoHeightmap.BindEdges[j].NPatchs = 0;
					}
				}
				else
				{
					rPI.BindEdges[j].Next[0] = it->second;
					rPINoHeightmap.BindEdges[j].Next[0] = it->second;
				}
			}

			if ((rPI.BindEdges[j].NPatchs == 2) || (rPI.BindEdges[j].NPatchs == 4))
			{
				for (k = 0; k < rPI.BindEdges[j].NPatchs; ++k)
				{
					map<int,int>::iterator it = OldToNewPatchId.find (rPI.BindEdges[j].Next[k]);
					if (it == OldToNewPatchId.end())
					{
						if (_ExportCB != NULL)
							_ExportCB->dispError (string("Continuity problem in zone ") + DstZoneFileName);
					}
					else
					{
						rPI.BindEdges[j].Next[k] = it->second;
						rPINoHeightmap.BindEdges[j].Next[k] = it->second;
					}
				}
			}
		}
	}

	unitZone.build (nZoneId, DstPI, BorderVertices);
	unitZoneNoHeightmap.build (nZoneId, DstPINoHeightmap, BorderVerticesNoHeightmap);
/*	{ // Debug
		if (!gLandInited)
		{
			gLand.init();
			gLandInited = true;
		}

		gLand.addZone(unitZone);
		gLand.checkBinds();
	}*/
}


// ---------------------------------------------------------------------------
void CExport::cutIG(CInstanceGroup &bigIG, CInstanceGroup &unitIG, sint32 nPosX, sint32 nPosY, SPiece &sMask, bool first, sint32 baseX, sint32 baseY)
{
	uint k;
	string DstZoneFileName = getZoneNameFromXY (nPosX, nPosY);

	float rMinX = nPosX		* _Options->CellSize;
	float rMaxX = (1+nPosX) * _Options->CellSize;
	float rMinY = nPosY		* _Options->CellSize;
	float rMaxY = (1+nPosY) * _Options->CellSize;

	CInstanceGroup::TInstanceArray			instances;
	std::vector<CCluster>				portals;
	std::vector<CPortal>				clusters;
	std::vector<CPointLightNamed>		pointLightList;

	bool realTimeSuncontribution = bigIG.getRealTimeSunContribution();

	///////////////
	// instances //
	///////////////
	for (k = 0; k < bigIG.getNumInstance(); ++k)
	{
		CInstanceGroup::CInstance &igi = bigIG.getInstance(k);
		const CVector &pos = igi.Pos;
		if (pos.x >= rMinX && pos.x < rMaxX && pos.y >= rMinY && pos.y < rMaxY)
		{
			instances.push_back(igi);
		}

		// First zone ?
		if (first)
		{
			// Out of the mask ?
			sint x = (sint)(floor(pos.x / _Options->CellSize)) - baseX;
			sint y = (sint)(floor(pos.y / _Options->CellSize)) - baseY;

			// Not in the mask ?
			bool inTheMask = (x>=0) && (x<(sint)sMask.w) && (y>=0) && (y<(sint)sMask.h);
			if (!inTheMask || !sMask.Tab[x+y*sMask.w])
			{
				instances.push_back(igi);
			}
		}
	}

	////////////
	// lights //
	////////////

	const std::vector<CPointLightNamed> &lights = bigIG.getPointLightList();
	for (k = 0; k < lights.size(); ++k)
	{
		const CVector &pos = lights[k].getPosition();
		if (pos.x >= rMinX && pos.x < rMaxX && pos.y >= rMinY && pos.y < rMaxY)
		{
			pointLightList.push_back(lights[k]);
		}

		// First zone ?
		if (first)
		{
			// Out of the mask ?
			sint x = (sint)(floor(pos.x / _Options->CellSize)) - baseX;
			sint y = (sint)(floor(pos.y / _Options->CellSize)) - baseY;

			// Not in the mask ?
			bool inTheMask = (x>=0) && (x<(sint)sMask.w) && (y>=0) && (y<(sint)sMask.h);
			if (!inTheMask || !sMask.Tab[x+y*sMask.w])
			{
				pointLightList.push_back(lights[k]);
			}
		}
	}

	// build the resulting ig
	unitIG.build(bigIG.getGlobalPos(), instances, portals, clusters, pointLightList);
	unitIG.enableRealTimeSunContribution(realTimeSuncontribution);

}


// ---------------------------------------------------------------------------
float CExport::getHeight (float x, float y)
{
	float deltaZ = 0.0f, deltaZ2 = 0.0f;
	CRGBAF color;
	sint32 SizeX = _ZoneMaxX - _ZoneMinX + 1;
	sint32 SizeY = _ZoneMaxY - _ZoneMinY + 1;

	clamp (x, _Options->CellSize*_ZoneMinX, _Options->CellSize*(_ZoneMaxX+1));
	clamp (y, _Options->CellSize*_ZoneMinY, _Options->CellSize*(_ZoneMaxY+1));

	if (_HeightMap != NULL)
	{
		color = _HeightMap->getColor (	(x-_Options->CellSize*_ZoneMinX)/(_Options->CellSize*SizeX),
										1.0f - ((y-_Options->CellSize*_ZoneMinY)/(_Options->CellSize*SizeY)));
		color *= 255;
		deltaZ = color.A;
		deltaZ = deltaZ - 127.0f; // Median intensity is 127
		deltaZ *= _Options->ZFactor;
	}

	if (_HeightMap2 != NULL)
	{
		color = _HeightMap2->getColor (	(x-_Options->CellSize*_ZoneMinX)/(_Options->CellSize*SizeX),
										1.0f - ((y-_Options->CellSize*_ZoneMinY)/(_Options->CellSize*SizeY)));
		color *= 255;
		deltaZ2 = color.A;
		deltaZ2 = deltaZ2 - 127.0f; // Median intensity is 127
		deltaZ2 *= _Options->ZFactor2;
	}

	return (deltaZ + deltaZ2);
}

// ---------------------------------------------------------------------------
CRGBAF CExport::getColor (float x, float y)
{
	CRGBAF color = CRGBA::Black;
	sint32 SizeX = _ZoneMaxX - _ZoneMinX + 1;
	sint32 SizeY = _ZoneMaxY - _ZoneMinY + 1;

	clamp (x, _Options->CellSize*_ZoneMinX, _Options->CellSize*(_ZoneMaxX+1));
	clamp (y, _Options->CellSize*_ZoneMinY, _Options->CellSize*(_ZoneMaxY+1));

	if (_ColorMap != NULL)
	{
		color = _ColorMap->getColor (	(x-_Options->CellSize*_ZoneMinX)/(_Options->CellSize*SizeX),
										1.0f - ((y-_Options->CellSize*_ZoneMinY)/(_Options->CellSize*SizeY)));
		color *= 255;
	}

	return color;
}

// ---------------------------------------------------------------------------
void CExport::light (NL3D::CZone &zoneOut, NL3D::CZone &zoneIn)
{
	// Same as zone_lighter stand-alone exe
	// ------------------------------------
	/*	CZoneLighter zl;
	CLandscape land;
	CZoneLighter::CLightDesc ld;
	vector<CZoneLighter::CTriangle> obstacle;
	vector<uint> listzone;

	ld.SkyContribution = false;
	ld.Oversampling = CZoneLighter::CLightDesc::NoOverSampling;
	ld.Shadow = false;
	ld.Softshadow = false;
	ld.NumCPU = 1;
	ld.GridSize = 2;

	try
	{
		zl.init ();
		land.init ();
		land.TileBank = *_ZeTileBank;
		land.initTileBanks();
		land.addZone (zoneIn);

		listzone.push_back(zoneIn.getZoneId());

		zl.light (land, zoneOut, zoneIn.getZoneId(), ld, obstacle, listzone);
	}
	catch (const Exception &e)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispError (e.what());
	}*/

	// Quickest version without noise
	// ------------------------------

	CLandscape land;

	land.init ();
	land.TileBank = *_ZeTileBank;
	land.initTileBanks();
	land.addZone (zoneIn);


	vector<CPatchInfo> vPI;
	vector<CBorderVertex> vBV;
	uint32 i, j, k, m;
	float s, t, val;
	CVector n, l = CVector (1.0f, 1.0f, -1.0f);
	vector<CVector> vertices;
	CVector v[4];

	l.normalize();

	CZone *dyn = land.getZone(zoneIn.getZoneId());
	uint32 numPatch = dyn->getNumPatchs();

	zoneIn.retrieve (vPI, vBV);

	if (_Options->Light == 2) // Noise ?
	for (i = 0; i < numPatch; ++i)
	{
		const CPatch *pCP = const_cast<const CZone *>(dyn)->getPatch (i);

		CPatchInfo &rPI = vPI[i];
		vertices.resize((rPI.OrderT*4+1)*(rPI.OrderS*4+1));

		for (k = 0; k < (uint32)(rPI.OrderT*4+1); ++k)
		for (j = 0; j < (uint32)(rPI.OrderS*4+1); ++j)
		{
			s = (((float)j) / (rPI.OrderS*4));
			t = (((float)k) / (rPI.OrderT*4));
			vertices[j+k*(rPI.OrderS*4+1)] = pCP->computeVertex(s, t);
		}

		for (k = 0; k < (uint32)(rPI.OrderT*4); ++k)
		for (j = 0; j < (uint32)(rPI.OrderS*4); ++j)
		{
			v[0] = vertices[(j+0)+(k+0)*(rPI.OrderS*4+1)];
			v[1] = vertices[(j+1)+(k+0)*(rPI.OrderS*4+1)];
			v[2] = vertices[(j+1)+(k+1)*(rPI.OrderS*4+1)];
			v[3] = vertices[(j+0)+(k+1)*(rPI.OrderS*4+1)];

			val = 0.0f;
			for (m = 0; m < 4; ++m)
			{
				n = (v[(m+0)%4]-v[(m+2)%4])^(v[(m+0)%4]-v[(m+1)%4]);
				n.normalize();
				val += 255.0f*(1.0f-n*l)/2.0f;
			}
			val = val / 4.0f;
			clamp (val, 0.0f, 255.0f);
			rPI.Lumels[j+k*rPI.OrderS*4] = (uint8)(val);
		}
	}
	else // No noise
	for (i = 0; i < numPatch; ++i)
	{
		const CPatch *pCP = const_cast<const CZone *>(dyn)->getPatch (i);
		CBezierPatch *pBP = pCP->unpackIntoCache();

		CPatchInfo &rPI = vPI[i];

		for (k = 0; k < (uint32)(rPI.OrderT*4); ++k)
		for (j = 0; j < (uint32)(rPI.OrderS*4); ++j)
		{
			s = ((0.5f+(float)j) / (rPI.OrderS*4));
			t = ((0.5f+(float)k) / (rPI.OrderT*4));
			n = pBP->evalNormal (s, t);
			val = 255.0f*(1.0f-n*l)/2.0f;
			clamp (val, 0.0f, 255.0f);
			rPI.Lumels[j+k*rPI.OrderS*4] = (uint8)(val);
		}
	}

	zoneOut.build (zoneIn.getZoneId(), vPI, vBV);
}

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

// ---------------------------------------------------------------------------
void CExport::transformCMB (const std::string &name, const NLMISC::CMatrix &transfo, bool verbose) const
{
	if (name.empty())
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning("Can't load cmb : the name is empty");
		return;
	}
	std::string cmbNoExtension = CFile::getFilenameWithoutExtension(name);
	std::string cmbName = CPath::lookup(cmbNoExtension + ".cmb" , false, false, false);
	if (cmbName.empty())
	{
		if ((_ExportCB != NULL) && verbose)
			_ExportCB->dispWarning("Can't find " + cmbNoExtension + ".cmb");
		return;
	}
	std::string outFileName = _Options->OutCMBDir +"/" + cmbNoExtension + ".cmb";
	bool needUpdate = true;
	if (CFile::fileExists(outFileName))
	{
		uint32 outModification = CFile::getFileModificationDate(outFileName);
		needUpdate = 
			CFile::getFileModificationDate(cmbName) > outModification
			|| (CFile::fileExists(_Options->HeightMapFile) && (CFile::getFileModificationDate(_Options->HeightMapFile) > outModification))
			|| (CFile::fileExists(_Options->HeightMapFile2) && (CFile::getFileModificationDate(_Options->HeightMapFile2) > outModification))
			|| (CFile::fileExists(_Options->ContinentFile) && (CFile::getFileModificationDate(_Options->ContinentFile) > outModification))
			|| (CFile::fileExists(_Options->ZoneRegionFile) && (CFile::getFileModificationDate(_Options->ZoneRegionFile) > outModification));
	}
	if (needUpdate)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispInfo("UPDATE " + cmbName);
		printf("UPDATE %s\n", cmbName.c_str());

		CIFile inStream;
		if (inStream.open(cmbName))
		{
			try
			{
				CCollisionMeshBuild cmb;
				cmb.serial(inStream);
				// translate and save
				cmb.transform (transfo);
				COFile outStream;
				if (!outStream.open(outFileName))
				{
					if (_ExportCB != NULL)
						_ExportCB->dispWarning("Couldn't open " + outFileName + "for writing, not exporting");
				}
				else
				{
					try
					{
						cmb.serial(outStream);
						outStream.close();
					}
					catch (const EStream &e)
					{
						outStream.close();
						if (_ExportCB != NULL)
						{
							_ExportCB->dispWarning("Error while writing " + outFileName);
							_ExportCB->dispWarning(e.what());
						}
					}
				}
				inStream.close();
			}
			catch (const EStream &e)
			{
				inStream.close();
				if (_ExportCB != NULL)
				{
						_ExportCB->dispWarning("Error while reading " + cmbName);
						_ExportCB->dispWarning(e.what());
				}
			}
		}
		else
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning("Unable to open " + cmbName);
		}
	}
	else
	{
		if (_ExportCB != NULL)
			_ExportCB->dispInfo("SKIP " + cmbName);
		printf("SKIP %s\n", cmbName.c_str());
	}
}

// ---------------------------------------------------------------------------
void CExport::transformAdditionnalIG (const std::string &name, const NLMISC::CMatrix &transfo, const NLMISC::CQuat &rotTransfo) const
{
	if (name.empty())
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning("Can't load cmb : the name is empty");
		return;
	}
	std::string igNoExtension = CFile::getFilenameWithoutExtension(name);
	std::string igName = CPath::lookup(igNoExtension + ".ig" , false, false, false);
	if (igName.empty())
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning("Can't find " + igNoExtension + ".cmb");
		return;
	}
	std::string outFileName = _Options->AdditionnalIGOutDir +"/" + igNoExtension + ".ig";
	bool needUpdate = true;
	if (CFile::fileExists(outFileName))
	{
		uint32 outModification = CFile::getFileModificationDate(outFileName);
		needUpdate = 
			CFile::getFileModificationDate(igName) > outModification
			|| (CFile::fileExists(_Options->HeightMapFile) && (CFile::getFileModificationDate(_Options->HeightMapFile) > outModification))
			|| (CFile::fileExists(_Options->HeightMapFile2) && (CFile::getFileModificationDate(_Options->HeightMapFile2) > outModification))
			|| (CFile::fileExists(_Options->ContinentFile) && (CFile::getFileModificationDate(_Options->ContinentFile) > outModification))
			|| (CFile::fileExists(_Options->ZoneRegionFile) && (CFile::getFileModificationDate(_Options->ZoneRegionFile) > outModification));
	}
	if (needUpdate)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispInfo("UPDATE " + igName);
		printf("UPDATE %s\n", igName.c_str());

		CIFile inStream;
		if (inStream.open(igName))
		{
			try
			{
				CInstanceGroup ig, igOut;
				ig.serial(inStream);

				CVector globalPos;
				CInstanceGroup::TInstanceArray IA;
				std::vector<CCluster> Clusters;
				std::vector<CPortal>  Portals;
				std::vector<CPointLightNamed> PLN;

				ig.retrieve(globalPos, IA, Clusters, Portals, PLN);
				bool realTimeSuncontribution = ig.getRealTimeSunContribution();

				uint k;
				// elevate instance
				for(k = 0; k < IA.size(); ++k)
				{
					IA[k].Pos = transfo * IA[k].Pos;
					IA[k].Rot = rotTransfo * IA[k].Rot;
				}
				// lights
				for(k = 0; k < PLN.size(); ++k)
				{
					PLN[k].setPosition(transfo * PLN[k].getPosition());
				}
				// portals
				std::vector<CVector> portal;
				for(k = 0; k < Portals.size(); ++k)
				{
					Portals[k].getPoly(portal);
					for(uint l = 0; l < portal.size(); ++l)
					{
						portal[l] = transfo * portal[l];
					}
					Portals[k].setPoly(portal);
				}

				// clusters
				for(k = 0; k < Clusters.size(); ++k)
				{
					Clusters[k].applyMatrix (transfo);
				}



				igOut.build(globalPos, IA, Clusters, Portals, PLN);
				igOut.enableRealTimeSunContribution(realTimeSuncontribution);

				COFile outStream;
				if (!outStream.open(outFileName))
				{
					if (_ExportCB != NULL)
						_ExportCB->dispWarning("Couldn't open " + outFileName + "for writing, not exporting");
				}
				else
				{
					try
					{
						igOut.serial(outStream);
						outStream.close();
					}
					catch (const EStream &e)
					{
						outStream.close();
						if (_ExportCB != NULL)
						{
							_ExportCB->dispWarning("Error while writing " + outFileName);
							_ExportCB->dispWarning(e.what());
						}
					}
				}
				inStream.close();
			}
			catch (const EStream &e)
			{
				inStream.close();
				if (_ExportCB != NULL)
				{
						_ExportCB->dispWarning("Error while reading " + igName);
						_ExportCB->dispWarning(e.what());
				}
			}
		}
		else
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning("Unable to open " +  igName);
		}
	}
	else
	{
		if (_ExportCB != NULL)
			_ExportCB->dispInfo("SKIP " + igName);
		printf("SKIP %s\n", igName.c_str());
	}
}

// ---------------------------------------------------------------------------
void CExport::exportCMBAndAdditionnalIGs()
{
	if (!_Options->ExportCollisions && !_Options->ExportCollisions) return;
	CSmartPtr<UForm> continent = loadContinent(_Options->ContinentFile);
	if (!continent) return;

	NLGEORGES::UFormElm  *villages;
	if (!continent->getRootNode ().getNodeByName (&villages, "Villages") || !villages)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant find villages in a continent form");
		return;
	}

	uint size;
	nlverify (villages->getArraySize (size));

	for(uint k = 0; k < size; ++k)
	{
		NLGEORGES::UFormElm  *village;
		if (!villages->getArrayNode (&village, k) || !village)
		{
			if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant get a village ");
			continue;
		}

		// get position of village
		float altitude;
		if (!village->getValueByName (altitude, "Altitude"))
		{
			if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant get a village position");
			continue;
		}

		// get rotation of village
		sint32 rotation;
		if (!village->getValueByName (rotation, "Rotation"))
		{
			if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant get a village rotation");
			continue;
		}

		// get width and height of village
		uint32 width;
		if (!village->getValueByName (width, "Width"))
		{
			if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant get a village width");
			continue;
		}

		// get width and height of village
		uint32 height;
		if (!village->getValueByName (height, "Height"))
		{
			if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant get a village height");
			continue;
		}

		// get position of village
		std::string zoneName;
		if (!village->getValueByName (zoneName, "Zone"))
		{
			if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant get a village name");
			continue;
		}

		zoneName = strupr(zoneName);
		//
		sint32 x = CExport::getXFromZoneName(zoneName), y = CExport::getYFromZoneName(zoneName);
		CVector pos(160.f * x, 160.f * y, altitude + getHeight(160.f * x, 160.f * y));

		// *** Build the transformation
		CMatrix transfo;
		CQuat rotTransfo;
		rotTransfo.identity ();
		transfo.identity ();

		// Rotation
		float angle = (float)rotation * (float)Pi / 2.f;
		transfo.rotateZ (angle);
		rotTransfo = CQuat (CVector::K, angle);

		// Add translation
		switch (rotation&3)
		{
		case 0:
			transfo.setPos (pos);
			break;
		case 1:
			transfo.setPos (pos+CVector (160.f * height, 0, 0));
			break;
		case 2:
			transfo.setPos (pos+CVector (160.f * width, 160.f * height, 0));
			break;
		case 3:
			transfo.setPos (pos+CVector (0, 160.f * width, 0));
			break;
		}

		// process ig / cmb of the village
		NLGEORGES::UFormElm  *igNamesItem;
		if (!village->getNodeByName (&igNamesItem, "IgList") || !igNamesItem)
		{
			if (_ExportCB != NULL)
			_ExportCB->dispWarning("Cant get village ig list");
			continue;
		}

		uint sizeIg;
		nlverify (igNamesItem->getArraySize (sizeIg));

		std::string igName;

		// For each children
		for(uint k = 0; k < sizeIg; ++k)
		{
			// Get the aray element
			const NLGEORGES::UFormElm *currIg;
			if (igNamesItem->getArrayNode (&currIg, k) && currIg)
			{
				const NLGEORGES::UFormElm *igNameItem;
				if (!currIg->getNodeByName (&igNameItem, "IgName") || !igNameItem)
				{
					nlwarning("Unable to get village name from a form");
					continue;
				}

				if (igNameItem->getValue (igName))
				{
					if (_Options->ExportCollisions)
					{
						transformCMB (igName, transfo, true);

						// Try to export alternative cmb like ig#0.cmb, ig#1.cmb, ig#2.cmb ... ig#9.cmb
						std::string cmbNoExtension = CFile::getFilenameWithoutExtension(igName) + "#";
						uint alt;
						for (alt=0; alt<10; alt++)
							transformCMB (cmbNoExtension+toString (alt), transfo, false);
					}
					if (_Options->ExportAdditionnalIGs) transformAdditionnalIG (igName, transfo, rotTransfo);
				}
				else
				{
					nlwarning("Unable to get village name from a form");
				}
			}
		}

	}
	return;
}


// ---------------------------------------------------------------------------
NLGEORGES::UForm* CExport::loadContinent(const std::string &name) const
{
	if (!NLMISC::CFile::fileExists(name))
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning("Can't find " + name);
		return NULL;
	}

	UForm *form;
	if (!(form = _FormLoader->loadForm (name.c_str ())))
	{
		if (_ExportCB != NULL)
		{
			_ExportCB->dispWarning("Unable to load continent form : " +  name);
		}
		return NULL;
	}

	return form;
}



// ***************************************************************************
void CExport::computeSubdividedTangents(uint numBinds, const NL3D::CBezierPatch &patch, uint edge, NLMISC::CVector subTangents[8])
{
	// Subdivide the Bezier patch to get the correct tangents to apply to neighbors
	CBezierPatch	subPatchs1_2[2];
	CBezierPatch	subPatchs1_4[4];

	// subdivide on s if edge is horizontal
	bool		subDivideOnS= (edge&1)==1;

	// Subdivide one time.
	if(subDivideOnS)	patch.subdivideS(subPatchs1_2[0], subPatchs1_2[1]);
	else				patch.subdivideT(subPatchs1_2[0], subPatchs1_2[1]);

	// Subdivide again for bind 1/4.
	if(numBinds==4)
	{
		if(subDivideOnS)
		{
			subPatchs1_2[0].subdivideS(subPatchs1_4[0], subPatchs1_4[1]);
			subPatchs1_2[1].subdivideS(subPatchs1_4[2], subPatchs1_4[3]);
		}
		else
		{
			subPatchs1_2[0].subdivideT(subPatchs1_4[0], subPatchs1_4[1]);
			subPatchs1_2[1].subdivideT(subPatchs1_4[2], subPatchs1_4[3]);
		}
	}

	// Now, fill the tangents according to edge.
	bool	invertPaSrc= edge>=2;
	// Bind 1/2 case.
	if(numBinds==2)
	{
		// 4 tangents to fill.
		for(uint i=0;i<4;i++)
		{
			// get patch id from 0 to 1.
			uint	paSrcId= i/2;
			// invert if edge is 2 or 3
			if(invertPaSrc) paSrcId= 1-paSrcId;
			// get tg id in this patch.
			uint	tgSrcId= (i&1) + edge*2;
			// fill result.
			subTangents[i]= subPatchs1_2[paSrcId].Tangents[tgSrcId];
		}
	}
	// Bind 1/4 case.
	else
	{
		// 8 tangents to fill.
		for(uint i=0;i<8;i++)
		{
			// get patch id from 0 to 3.
			uint	paSrcId= i/2;
			// invert if edge is 2 or 3
			if(invertPaSrc) paSrcId= 3-paSrcId;
			// get tg id in this patch.
			uint	tgSrcId= (i&1) + edge*2;
			// fill result.
			subTangents[i]= subPatchs1_4[paSrcId].Tangents[tgSrcId];
		}
	}
}


// ***************************************************************************
bool CExport::applyVertexBind(NL3D::CPatchInfo &pa, NL3D::CPatchInfo &oldPa, uint edgeToModify, bool startEdge,
	const NLMISC::CMatrix &oldTgSpace, const NLMISC::CMatrix &newTgSpace,
	const NLMISC::CVector &bindedPos, const NLMISC::CVector &bindedTangent )
{
	// Get the vertex to modify according to edge/startEdge
	uint	vertexToModify= edgeToModify + (startEdge?0:1);
	vertexToModify&=3;

	// If already moved, no-op
	if(pa.Patch.Vertices[vertexToModify]==bindedPos)
		return false;
	else
	{
		// Change the vertex
		pa.Patch.Vertices[vertexToModify]= bindedPos;

		// change the tangent, according to startEdge
		pa.Patch.Tangents[edgeToModify*2 + (startEdge?0:1) ]= bindedTangent;

		// Must change the tangent which is on the other side of the vertex:
		uint	tgToModify= 8 + edgeToModify*2 + (startEdge?-1:+2);
		tgToModify&=7;
		/* To keep the same continuity aspect around the vertex, we compute the original tangent in a
			special space: the Binded Patch Tangent Space. Once we have the original tangent in the original patch TgSpace,
			we reapply it in the transformed patch TgSpace, to get the transformed tangent
		*/
		pa.Patch.Tangents[tgToModify]= newTgSpace * ( oldTgSpace.inverted() * oldPa.Patch.Tangents[tgToModify] );


		// Do the same to the associated interior.
		pa.Patch.Interiors[vertexToModify]= newTgSpace * ( oldTgSpace.inverted() * oldPa.Patch.Interiors[vertexToModify] );


		// modified
		return true;
	}
}


// ***************************************************************************
NLMISC::CVector	CExport::getHeightNormal (float x, float y)
{
	sint32 SizeX = _ZoneMaxX - _ZoneMinX + 1;
	sint32 SizeY = _ZoneMaxY - _ZoneMinY + 1;
	sint32 bmpW, bmpH;


	// get width/height of the bitmap
	if (_HeightMap != NULL)
	{
		bmpW= _HeightMap->getWidth();
		bmpH= _HeightMap->getHeight();
	}
	else if (_HeightMap2 != NULL)
	{
		bmpW= _HeightMap2->getWidth();
		bmpH= _HeightMap2->getHeight();
	}
	else
	{
		// no heightmap: unmodified normal
		return CVector::K;
	}


	// compute a good delta to compute tangents of the heightmap: 1/10 of a pixel, avoiding precision problem.
	float	dx= ((_Options->CellSize*SizeX)/bmpW)/10;	// eg: 160m/20pixels/10= 0.8
	float	dy= ((_Options->CellSize*SizeY)/bmpH)/10;

	// compute tangent around the position.
	float	hc= getHeight(x,y);
	float	hx= getHeight(x+dx,y);
	float	hy= getHeight(x,y+dy);
	CVector	ds(dx,0,hx-hc);
	CVector	dt(0,dy,hy-hc);

	// compute the heightmap normal with the tangents
	return (ds^dt).normed();
}
