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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"

// game share
#include "game_share/brick_types.h"
// Client
#include "prim_file.h"
#include "client_cfg.h"

///////////
// USING //
///////////
using namespace NLLIGO;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;
using namespace NLNET;
using namespace std;

#define POINT_HEIGHT 1.f
#define POINT_SIZE 1.f

extern UGlobalRetriever	*GR;
extern UScene	*Scene;
extern UTextContext *TextContext;

CRGBA PrimColors[] =
{
	CRGBA (255, 0, 0),
	CRGBA (0, 255, 0),
	CRGBA (0, 0, 255),
	CRGBA (255, 255, 0),
	CRGBA (0, 255, 255),
	CRGBA (255, 0, 255),
	CRGBA (127, 0, 0),
	CRGBA (0, 127, 0),
	CRGBA (0, 0, 127),
	CRGBA (127, 127, 0),
	CRGBA (0, 127, 127),
	CRGBA (127, 0, 127),
};

/////////////
// GLOBALS //
/////////////

// Show prim file
CPrimFileMgr PrimFiles;

//---------------------------------------------------

CPrimFileMgr::CPrimFileMgr ()
{
	_ShowPrim = false;
	_Loaded = false;
	_PrimFile = 0;
}

//---------------------------------------------------

void CPrimFileMgr::adjustPosition (NLMISC::CVector &pos)
{
	UGlobalPosition	globalPos = GR->retrievePosition (pos);
	pos.z = GR->getMeanHeight (globalPos) + POINT_HEIGHT;
}

//---------------------------------------------------

void CPrimFileMgr::load (sint primFileIndex)
{
	_Loaded = false;
	// Prim file count
	sint primFileCount = (sint)ClientCfg.PrimFiles.size ();
	if (primFileCount)
	{
		// Get a valid index
		while (primFileIndex >= primFileCount)
		{
			primFileIndex -= primFileCount;
		}
		while (primFileIndex < 0)
		{
			primFileIndex += primFileCount;
		}

		// Current prim index
		_PrimFile = primFileIndex;

		// Get the path name
		string pathName = CPath::lookup (ClientCfg.PrimFiles[_PrimFile], false, true);
		if (pathName.empty ())
		{
			pathName = ClientCfg.PrimFiles[_PrimFile];
		}

		// Reset the container
		_PrimRegion = NLLIGO::CPrimRegion ();

		// Open the file
		CIFile file;
		if (file.open (pathName))
		{
			try
			{
				// Serial XML
				CIXml xml;
				xml.init (file);

				// Serial the region
				_PrimRegion.serial (xml);
				_Loaded = true;

				// Put to the ground

				// Adjust points
				uint point;
				for (point = 0; point < _PrimRegion.VPoints.size (); point++)
				{
					// Adjuste the position
					adjustPosition (_PrimRegion.VPoints[point].Point);
				}

				// Adjust segments
				uint seg;
				for (seg = 0; seg < _PrimRegion.VPaths.size (); seg++)
				{
					// For each
					uint pointCount = (uint)_PrimRegion.VPaths[seg].VPoints.size ();

					for (point = 0; point < pointCount; point++)
					{
						adjustPosition (_PrimRegion.VPaths[seg].VPoints[point]);
					}
				}

				// Adjust polygons
				for (seg = 0; seg < _PrimRegion.VZones.size (); seg++)
				{
					// For each
					uint pointCount = (uint)_PrimRegion.VZones[seg].VPoints.size ();

					for (point = 0; point < pointCount; point ++)
					{
						adjustPosition (_PrimRegion.VZones[seg].VPoints[point]);
					}
				}

			}
			catch (const Exception &e)
			{
				// Error
				nlwarning ("Error while reading the prim file (%s) : %s", pathName.c_str(), e.what ());

				// Reset the container
				_PrimRegion = NLLIGO::CPrimRegion ();
			}
		}
		else
		{
			// Can't open
			nlwarning ("Can't open the prim file %s", pathName.c_str ());
		}
	}
}

//---------------------------------------------------

void CPrimFileMgr::changeColor (uint &currentColor)
{
	const uint colorCount = sizeof (PrimColors) / sizeof (CRGBA);
	if (currentColor >= colorCount)
		currentColor = 0;
	_Material.setColor (PrimColors[currentColor]);
	TextContext->setColor (PrimColors[currentColor]);
	currentColor++;
}

//---------------------------------------------------

void CPrimFileMgr::draw3dText (const NLMISC::CVector &pos, NL3D::UCamera cam, const char *text)
{
	// Create the matrix and set the orientation according to the camera.
	CMatrix matrix;
	matrix.identity();
	matrix.setRot(cam.getRotQuat());
	matrix.setPos (pos);
	matrix.scale (60);

	// Draw the name.
	TextContext->render3D (matrix, text);
}

//---------------------------------------------------

void CPrimFileMgr::display (NL3D::UDriver &driver)
{
	if (_ShowPrim)
	{
		// Material exist ?
		if (!_Material.empty())
		{
			_Material = driver.createMaterial ();
			_Material.initUnlit ();
			_Material.setZFunc (UMaterial::always);
			_Material.setZWrite (false);
		}

		// Remove fog
		bool fogState = driver.fogEnabled ();
		driver.enableFog (false);

		// Load current
		if (!_Loaded)
			load (_PrimFile);

		// Current color
		uint currentColor = 0;

		// Draw points
		uint point;
		for (point = 0; point < _PrimRegion.VPoints.size (); point++)
		{
			// Set the color for the next primitive
			changeColor (currentColor);

			// Ref on the vector
			CVector &vect = _PrimRegion.VPoints[point].Point;

			// Line
			CLine line;
			line.V0 = vect;
			line.V1 = vect;
			line.V0.x -= POINT_SIZE/2;
			line.V1.x += POINT_SIZE/2;
			driver.drawLine (line, _Material);
			line.V0 = vect;
			line.V1 = vect;
			line.V0.y -= POINT_SIZE/2;
			line.V1.y += POINT_SIZE/2;
			driver.drawLine (line, _Material);
			line.V0 = vect;
			line.V1 = vect;
			line.V0.z -= POINT_SIZE/2;
			line.V1.z += POINT_SIZE/2;
			driver.drawLine (line, _Material);

			// Draw a text
			string *name;
			if (_PrimRegion.VPoints[point].getPropertyByName("name", name))
				draw3dText (vect + CVector (0, 0, 10 * POINT_HEIGHT), Scene->getCam(), name->c_str ());
		}

		// Draw segments
		uint seg;
		for (seg = 0; seg < _PrimRegion.VPaths.size (); seg++)
		{
			// Set the color for the next primitive
			changeColor (currentColor);

			// Mean pos
			CVector pos (0,0,0);

			// For each
			uint pointCount = (uint)_PrimRegion.VPaths[seg].VPoints.size ();
			vector<CPrimVector> &points = _PrimRegion.VPaths[seg].VPoints;

			// Some points ?
			if (pointCount != 0)
			{
				pos = points[0];
				for (point = 0; point < pointCount-1; point++)
				{
					// Draw the line
					CLine line;
					line.V0 = points[point];
					line.V1 = points[point+1];
					driver.drawLine (line, _Material);
					pos += points[point+1];
				}

				pos /= (float) pointCount;

				// Draw a text
				string *name;
				if (_PrimRegion.VPaths[seg].getPropertyByName("name", name))
					draw3dText (pos + CVector (0, 0, 10 * POINT_HEIGHT), Scene->getCam(), name->c_str ());
			}
		}

		// Draw polygons
		for (seg = 0; seg < _PrimRegion.VZones.size (); seg++)
		{
			// Set the color for the next primitive
			changeColor (currentColor);

			// For each
			uint pointCount = (uint)_PrimRegion.VZones[seg].VPoints.size ();
			vector<CPrimVector> &points = _PrimRegion.VZones[seg].VPoints;

			// Some points ?
			if (pointCount != 0)
			{
				// Mean pos
				CVector pos (0,0,0);

				// Previous point
				NLMISC::CVector *previous = &points[pointCount-1];

				for (point = 0; point < pointCount; point ++)
				{
					// Next point
					NLMISC::CVector *next = &points[point];

					// Draw the line
					CLine line;
					line.V0 = *previous;
					line.V1 = *next;
					driver.drawLine (line, _Material);

					pos += points[point];

					previous = next;
				}

				pos /= (float) pointCount;

				// Draw a text
				string *name;
				if (_PrimRegion.VZones[seg].getPropertyByName("name", name))
					draw3dText (pos + CVector (0, 0, POINT_HEIGHT), Scene->getCam(), name->c_str ());
			}
		}

		// Reset fog
		driver.enableFog (fogState);
	}
}

//---------------------------------------------------

void CPrimFileMgr::release (NL3D::UDriver &driver)
{
	driver.deleteMaterial (_Material);
}

//---------------------------------------------------

string CPrimFileMgr::getCurrentPrimitive () const
{
	if (_ShowPrim)
	{
		if ( (_PrimFile < (sint)ClientCfg.PrimFiles.size ()) && (_PrimFile >= 0) )
		{
			return ClientCfg.PrimFiles[_PrimFile];
		}
	}
	return "";
}

