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

#include <stdio.h>
#include <string>
#include <sstream>

#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/progress_callback.h"

#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

#include "nel/3d/zone.h"
#include "nel/3d/landscape.h"
#include "nel/3d/scene_group.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif // NL_OS_WINDOWS

// ***************************************************************************

/*
Documentation

This tool generates ig from primitive files

  - Load the landscape zone. Load all the *.zonew found in the InLandscapeDir and add them in a landscape
  - For each primitive files found in PrimDirs
	- Look for points with the "prim" primitive class
	- Get the good height on the landscape
	- Get the .plant georges file associed with the point
	- Add an entry in the good ig file
		- Set the shape filename
		- Set the plant name
		- Set the final position (x and y come from the primitive and z from height test with the landscape, the rotation comes from the primitive)
	- Set the ig date
		- The date is the most recent date between the .zonew .plant .primitive files associed with the ig.
  - Snap to ground only modified or created ig
  - Save the modified or created ig files
*/

// ***************************************************************************

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLGEORGES;
using namespace NLLIGO;

// ***************************************************************************
// Utility functions
// ***************************************************************************

#define SELECTION_EPSILON 0.1f

#define BAR_LENGTH 21

const char *progressbar[BAR_LENGTH]=
{
	"[                    ]",
	"[.                   ]",
	"[..                  ]",
	"[...                 ]",
	"[....                ]",
	"[.....               ]",
	"[......              ]",
	"[.......             ]",
	"[........            ]",
	"[.........           ]",
	"[..........          ]",
	"[...........         ]",
	"[............        ]",
	"[.............       ]",
	"[..............      ]",
	"[...............     ]",
	"[................    ]",
	"[.................   ]",
	"[..................  ]",
	"[................... ]",
	"[....................]"
};

// ***************************************************************************

// Progress bar
class CMyCallback : public IProgressCallback
{
public:
	void progress (float progress)
	{
		// Delta time, update max all the 300 ms
		static sint64 time = CTime::getLocalTime ();
		sint64 currentTime = CTime::getLocalTime ();
		if ((currentTime - time) > 300)
		{
			// Crop the progress bar value
			progress = getCropedValue (progress);

			// Progress bar
			char msg[512];
			uint	pgId= (uint)(progress*(float)BAR_LENGTH);
			pgId= min(pgId, (uint)(BAR_LENGTH-1));
			sprintf (msg, "\r%s: %s", DisplayString.c_str (), progressbar[pgId]);
			uint i;
			for (i=(uint)strlen(msg); i<79; i++)
				msg[i]=' ';
			msg[i]=0;
			printf ("%s", msg);
			printf ("\r");

			time = currentTime;
		}
	}
};

// ***************************************************************************

sint getXFromZoneName (const string &ZoneName)
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

// ***************************************************************************

bool getZoneCoordByName(const char * name, uint16& x, uint16& y)
{
	uint i;
	
	std::string zoneName(name);

	// y
	std::string::size_type ind1 = zoneName.find("_");
	if(ind1 == std::string::npos)
	{
		nlwarning("bad file name");
		return false;
	}
	std::string ystr = zoneName.substr(0,ind1);
	for(i=0; i<ystr.length(); i++)
	{
		if(!isdigit(ystr[i]))
		{
			nlwarning("y code size is not a 2 characters code");
			return false;
		}
	}

	NLMISC::fromString(ystr, y);
	y = -y;

	// x
	x = 0;
	uint ind2 = (uint)zoneName.length();
	if((ind2-ind1-1)!=2)
	{
		nlwarning("x code size is not a 2 characters code");
		return false;
	}
	std::string xstr = zoneName.substr(ind1+1,ind2-ind1-1);
	for(i=0; i<xstr.length(); i++)
	{
		if (isalpha(xstr[i]))
		{
			x *= 26;
			x += (tolower(xstr[i])-'a');
		}
		else
		{
			nlwarning("invalid");
			return false;
		}
	}
	return true;
}

// ***************************************************************************

sint getYFromZoneName (const string &ZoneName)
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

	sint y = 0;
	NLMISC::fromString(yStr, y);

	return -y;
}

// ***************************************************************************

void outString (const string &sText)
{
	createDebug ();
	InfoLog->displayRaw(sText.c_str());
}

// ***************************************************************************

string getPrimitiveName (const IPrimitive &primitive)
{
	string name;
	primitive.getPropertyByName ("name", name);
	return name;
}

// ***************************************************************************

uint16 getZoneId (sint x, sint y)
{
	return (uint16)((((-y)-1)<<8) + x);
}

// ***************************************************************************

void getLettersFromNum(uint16 num, std::string& code)
{
	if(num>26*26) 
	{
		nlwarning("zone index too high");
		return;
	}
	code.resize(0);
	uint16 remainder = num%26;
	code += 'A' + num/26;
	code += 'A' + remainder;
}

// ***************************************************************************

void getZoneNameByCoord(sint16 x, sint16 y, std::string& zoneName)
{
	if ((y>0) || (y<-255) || (x<0) || (x>255))
		return;
	zoneName = toString(-y) + "_";
	zoneName += ('A' + (x/26));
	zoneName += ('A' + (x%26));
}

// ***************************************************************************

bool triangleIntersect2DGround (const CTriangle &tri, const CVector &pos0)
{
	const CVector		&p0= tri.V0;
	const CVector		&p1= tri.V1;
	const CVector		&p2= tri.V2;

	// Test if the face enclose the pos in X/Y plane.
	// NB: compute and using a BBox to do a rapid test is not a very good idea, since it will 
	// add an overhead which is NOT negligeable compared to the following test.
	float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
	// Line p0-p1.
	a= -(p1.y-p0.y);
	b= (p1.x-p0.x);
	c= -(p0.x*a + p0.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;
	// Line p1-p2.
	a= -(p2.y-p1.y);
	b= (p2.x-p1.x);
	c= -(p1.x*a + p1.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;
	// Line p2-p0.
	a= -(p0.y-p2.y);
	b= (p0.x-p2.x);
	c= -(p2.x*a + p2.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;

	return true;
}

// ***************************************************************************
// CExportOptions
// ***************************************************************************

struct CExportOptions
{
	std::string InLandscapeDir;		// Directory where to get .zonew files
	std::string OutIGDir;			// Directory where to put IG
	std::string LandBankFile;		// The .smallbank file associated with the landscape
	std::string LandFarBankFile;	// The .farbank file
	float		CellSize;			// Typically 160.0
	std::string	LandTileNoiseDir;	// Directory where to get displacement map

	std::vector<std::string> PrimDirs;			// Directory to parse for .flora and .prim associated
									// This is here we get continent.cfg file
	std::string FormDir;				// Directory to get georges dfn

	CExportOptions ();
	bool loadcf (NLMISC::CConfigFile &cf);
};

// ***************************************************************************

CExportOptions::CExportOptions ()
{
	CellSize = 160.0f;
}

// ***************************************************************************

bool CExportOptions::loadcf (CConfigFile &cf)
{
	// Out
	CConfigFile::CVar &cvOutIGDir = cf.getVar("OutIGDir");
	OutIGDir = cvOutIGDir.asString();

	// In
	CConfigFile::CVar &cvInLandscapeDir = cf.getVar("ZoneWDir");
	InLandscapeDir = cvInLandscapeDir.asString();

	CConfigFile::CVar &cvLandBankFile = cf.getVar("SmallBank");
	LandBankFile = cvLandBankFile.asString();
	CConfigFile::CVar &cvLandFarBankFile = cf.getVar("FarBank");
	LandFarBankFile = cvLandFarBankFile.asString();
	CConfigFile::CVar &cvLandTileNoiseDir = cf.getVar("DisplaceDir");
	LandTileNoiseDir = cvLandTileNoiseDir.asString();

	CConfigFile::CVar &cvCellSize = cf.getVar("CellSize");
	CellSize = cvCellSize.asFloat();

	CConfigFile::CVar &cvPrimDir = cf.getVar("PrimDirs");
	uint i;
	PrimDirs.resize (cvPrimDir.size());
	for (i=0; i<(uint)cvPrimDir.size(); i++)
		PrimDirs[i] = cvPrimDir.asString(i);

	CConfigFile::CVar &cvFormDir = cf.getVar("FormDir");
	FormDir = cvFormDir.asString();

	return true;
}

// ***************************************************************************

// Array of ig
class CIgContainer
{
public:

	// An ig
	class CIG
	{
	public:
		// Additionnal parameters
		class CAdditionnalParam
		{
		public:
			CAdditionnalParam (uint snapLayer, const string &primitiveName, const string &primitiveFile, bool snap) : SnapLayer (snapLayer), PrimitiveName (primitiveName), PrimitiveFile (primitiveFile) 
			{
				Snap = snap;
			};

			// Snap over the landscape
			bool	Snap;

			// Snap layer
			uint	SnapLayer;

			// Primitive name
			string	PrimitiveName;

			// Primitive file
			string	PrimitiveFile;
		};

		// Default ctor
		CIG ()
		{
			Date = 0;
		}

		// Update date if new date is more recent
		void updateDate (uint32 newDate)
		{
			if (newDate > Date)
				Date = newDate;
		}

		// The ig
		CInstanceGroup::TInstanceArray	Instances;

		// Additionnal information
		vector<CAdditionnalParam>		AdditionnalInfo;

		// The ig date
		uint32				Date;
	};

	// Init the container
	void init (sint minx, sint maxx, sint miny, sint maxy, const char *zoneDir)
	{
		// Save the values
		Minx = minx;
		Miny = miny;
		Maxx = maxx+1;
		Maxy = maxy+1;
		Width = maxx - minx + 1;

		// Resize the array
		IGS.clear ();
		IGS.resize (Width*(maxy-miny+1));

		// Directory
		string dir = CPath::standardizePath (zoneDir, true);

		// For each zone
		for (sint y=miny; y<=maxy; y++)
		for (sint x=minx; x<=maxx; x++)
		{
			// The zone name
			string zoneFilename;
			getZoneNameByCoord ((sint16)x, (sint16)y, zoneFilename);
			zoneFilename = dir + zoneFilename + ".zonew";

			// Get the date
			if (CFile::fileExists (zoneFilename))
				get (x, y).Date = CFile::getFileModificationDate (zoneFilename);
		}
	}

	// Get the ig
	CIG		&get (sint x, sint y)
	{
		return IGS[(x-Minx)+(y-Miny)*Width];
	}

	// Size and position
	uint					Width;
	sint					Minx;
	sint					Miny;
	sint					Maxx;
	sint					Maxy;

	// The ig vector
	vector<CIG>				IGS;
};

// ***************************************************************************

// Array of ig
class CFormContainer
{
private:
	// The map value
	struct CValue
	{
		CValue (CSmartPtr<UForm> ptr, uint32 date) : Ptr (ptr), Date (date) {};

		// The form pointer
		CSmartPtr<UForm>	Ptr;

		// Its date
		uint32				Date;
	};
public:

	// Default ctor
	CFormContainer ()
	{
		_FormLoader = UFormLoader::createLoader ();
	}

	// Dtor
	~CFormContainer ()
	{
		UFormLoader::releaseLoader (_FormLoader);
	}

	// The form container
	const UForm		*loadForm (const char *formName, uint32 &formDate)
	{
		// The form
		UForm *form = NULL;
		formDate = 0;

		// In the map ?
		string formShortName = NLMISC::toLower(CFile::getFilename (formName));
		map<string, CValue >::iterator ite = _FormMap.find (formShortName);
		if (ite == _FormMap.end ())
		{
			// Look for this plant file
			string path = CPath::lookup (formName, false, false, false);
			if (!path.empty ())
			{
				// Load it !
				form = _FormLoader->loadForm (path.c_str ());
				if (form)
				{
					// Get dependencies
					set<string> dependencies;
					form->getDependencies (dependencies);

					// Get dependencies dates
					formDate = 0;
					set<string>::const_iterator ite = dependencies.begin ();
					while (ite != dependencies.end ())
					{
						// Get the path name
						string path = CPath::lookup (*ite, false, false, false);
						if (!path.empty ())
						{
							// Get the file date
							uint32 date = CFile::getFileModificationDate (path);

							// Update date
							if (date > formDate)
								formDate = date;
						}

						// Next dependency
						ite++;
					}

					// Add it
					_FormMap.insert (map<string, CValue >::value_type (formShortName, CValue (form, formDate)));
				}
				else
				{
					// Error in the log
					nlwarning ("Error : Can't load the form (%s)", path.c_str ());
				}
			}
		}
		else
		{
			form = ite->second.Ptr;
			formDate = ite->second.Date;
		}

		// Return the form or NULL
		return form;
	}

private:

	// The form loader
	UFormLoader						*_FormLoader;

	// The form map
	map<string, CValue >	_FormMap;
};

// ***************************************************************************

void addPointPrimitive (CLandscape &landscape, const char *primFilename, uint32 primFileDate, const IPrimitive &primitive, CIgContainer &igs, 
						const CExportOptions &options, CFormContainer &formContainer, IProgressCallback &callback)
{
	// Is this primitive a point ?
	const CPrimPoint *point = dynamic_cast<const CPrimPoint *>(&primitive);
	if (point)
	{
		// Get the class name
		string className;
		if (point->getPropertyByName ("class", className))
		{
			// Is it a plant ?
			if (className == "prim")
			{
				// Get its plant name
				string plantFilename;
				if (point->getPropertyByName ("form", plantFilename))
				{
					// Add an extension
					if (NLMISC::toLower(CFile::getExtension (plantFilename)) != "plant")
						plantFilename += ".plant";

					// Load this form
					uint32 formDate;
					const UForm *form = formContainer.loadForm (plantFilename.c_str (), formDate);
					if (form)
					{
						// Get the parameters
						string shape;
						if (form->getRootNode ().getValueByName (shape, "3D.Shape"))
						{
							// Get the position
							CVector position = point->Point;

							// Get the scale
							string scaleText;
							float scale = 1;
							if (point->getPropertyByName ("scale", scaleText))
								scale = (float) atof (scaleText.c_str ());

							// Get zone coordinates
							sint x = (sint)floor (position.x / options.CellSize);
							sint y = (sint)floor (position.y / options.CellSize);
							if ( (x >= igs.Minx) && (x < igs.Maxx) && (y >= igs.Miny) && (y < igs.Maxy) )
							{
								// Get its layer
								string text;
								uint layer = 0;
								if (point->getPropertyByName ("depth", text))
								{
									layer = atoi (text.c_str ());
								}
									
								// Not snap yet
								position.z = 0;

								// Snap flag
								bool snap = true;
								if (point->getPropertyByName ("snap", text))
									snap = text != "false";

								// Get height
								if (!snap && point->getPropertyByName ("height", text))
									position.z = (float)atof(text.c_str());

								// *** Add the instance
								
								// Create it
								CInstanceGroup::CInstance instance;
								instance.Pos = position;
								instance.Rot = CQuat(CVector::K, point->Angle);
								instance.Scale = CVector (scale, scale, scale);
								instance.nParent = -1;
								instance.Name = shape;
								instance.InstanceName = NLMISC::toLower(CFile::getFilename (plantFilename));

								// Get the instance group ref
								CIgContainer::CIG	&instances = igs.get (x, y);
								instances.Instances.push_back (instance);
								instances.AdditionnalInfo.push_back (CIgContainer::CIG::CAdditionnalParam (layer, 
									getPrimitiveName (primitive), primFilename, snap));
								
								// Update the date with the primitive filename
								instances.updateDate (primFileDate);

								// Update the date with the plant filename
								instances.updateDate (formDate);

								// Update the date with the zone filename
								string zoneFilename;
								getZoneNameByCoord (x, y, zoneFilename);
								zoneFilename = CPath::standardizePath (options.InLandscapeDir, true) + zoneFilename + ".zonew";
								// todo hulud needed ? instances.updateDate (zoneFilename);
							}
						}
						else
						{
							// Error in the log
							nlwarning ("Error : Can't get a shape name in the form (%s) for the primitive (%s) in the file (%s)",
								plantFilename.c_str (), getPrimitiveName (primitive).c_str (), primFilename);
						}
					}
					else
					{
						// Error in the log
						nlwarning ("Error : can't load the file (%s) used by the primitive (%s) in the file (%s).", plantFilename.c_str (), 
							getPrimitiveName (primitive).c_str (), primFilename);
					}
				}
				else
				{
					// Error in the log
					nlwarning ("Error : in file (%s), the primitive (%s) has no plant file.", primFilename, getPrimitiveName (primitive).c_str ());
				}
			}
		}
	}

	// Look in children
	uint numChildren = primitive.getNumChildren ();
	for (uint i=0; i<numChildren; i++)
	{
		// Progress bar
		callback.progress ((float)i/(float)numChildren);
		callback.pushCropedValues ((float)i/(float)numChildren, (float)(i+1)/(float)numChildren);
			
		// Get the child
		const IPrimitive *child;
		nlverify (primitive.getChild (child, i));
		addPointPrimitive (landscape, primFilename, primFileDate, *child, igs, options, formContainer, callback);

		// Progress bar
		callback.popCropedValues ();
	}
}

// ***************************************************************************

int main (int argc, char**argv)
{
	new NLMISC::CApplicationContext;

	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter ("addSearchPath");

	// Register ligo
	NLLIGO::Register ();

	if (argc != 2)
	{
		printf ("Use : prim_export configfile.cfg\n");
		printf ("\nExample of config.cfg\n\n");

		printf ("\n// Export Options\n");
		printf ("OutIGDir = \"c:/temp/outIG\";\n");
		printf ("ZoneWDir = \"c:/temp/inZoneW\";\n");
		printf ("SmallBank = \"//amiga/3d/database/landscape/_texture_tiles/jungle/jungle.bank\";\n");
		printf ("FarBank = \"//amiga/3d/database/landscape/_texture_tiles/jungle/jungle.farbank\";\n");
		printf ("DisplaceDir = \"//amiga/3d/database/landscape/_texture_tiles/displace\";\n");
		printf ("CellSize = 160.0;\n");
		printf ("PrimDirs = {\"//server/leveldesign/world/fyros\"};\n");

		return -1;
	}

	try
	{
		// Load the config file
		CExportOptions options;

		string sTmp = string("loading cfg file : ") + string(argv[1]) + "\n";
		// outString(sTmp);
		{
			CConfigFile cf;
			cf.load (argv[1]);
			if (!options.loadcf(cf))
			{
				sTmp = "Error : options not loaded from config file\n";
				outString (sTmp);
				return -1;
			}
		}

		// *** Add pathes in the search path for georges forms

		CPath::addSearchPath (options.FormDir, true, true);

		// Ligo config
		CLigoConfig config;
		string path = CPath::lookup ("world_editor_classes.xml", false, false, false);
		if (path.empty())
			nlwarning ("Error : File world_editor_classes.xml not found");
		else
			config.readPrimitiveClass (path.c_str(), false);

		// *** Load the landscape

		// Init the landscape
		CLandscape landscape;
		landscape.init ();

		// Get file list
		vector<string> files;
		CPath::getPathContent (options.InLandscapeDir, false, false, true, files);

		// Landscape bounding box
		sint minx = 0x7fffffff;
		sint miny = 0x7fffffff;
		sint maxx = 0x80000000;
		sint maxy = 0x80000000;

		// The callback
		CMyCallback callback;

		// For each zone files
		if (files.empty())
		{
			nlwarning ("Error : no zonew files found. Abort.");
		}
		else
		{
			uint i;
			for (i=0; i<files.size (); i++)
			{
				// Progress
				callback.DisplayString = "Loading zones";
				callback.progress ((float)i/(float)files.size ());

				// Zonew ?
				if (NLMISC::toLower(CFile::getExtension (files[i])) == "zonew")
				{
					// Load it
					try
					{
						// The zone
						CZone zone;

						// Load
						CIFile inFile;
						if (inFile.open (files[i]))
						{
							// Read the zone
							zone.serial (inFile);

							// Add the zone in the landscape
							landscape.addZone (zone);
							
							// Extand the bounding box
							string name = CFile::getFilenameWithoutExtension (files[i]);
							sint x = getXFromZoneName (name);
							sint y = getYFromZoneName (name);
							if (x<minx)
								minx = x;
							if (y<miny)
								miny = y;
							if (x>maxx)
								maxx = x;
							if (y>maxy)
								maxy = y;
						}
						else
						{
							// Error in the log
							nlwarning ("Error : can't open the file (%s) for reading", files[i].c_str ());
						}
					}
					catch(const Exception &e)
					{
						// Error in the log
						nlwarning ("Error loading zone file (%s) : %s", files[i].c_str (), e.what ());
					}
				}
			}

			// *** Create the igs
			CIgContainer igs;
			igs.init (minx, maxx, miny, maxy, options.InLandscapeDir.c_str ());

			// *** Create a form container
			CFormContainer formContainer;

			// *** For each primitive files

			// Get the primitive files liste
			files.clear ();
			for (i=0; i<options.PrimDirs.size(); i++)
				CPath::getPathContent (options.PrimDirs[i], true, false, true, files);

			// For each
			uint fileCount = (uint)files.size ();
			for (i=0; i<fileCount; i++)
			{
				// Primitive file ?
				if (NLMISC::toLower(CFile::getExtension (files[i])) == "primitive")
				{
					// Progress bar
					nlinfo (files[i].c_str());
					callback.DisplayString = "Add primitives from "+CFile::getFilename(files[i]);
					callback.progress ((float)i/(float)fileCount);
					callback.pushCropedValues ((float)i/(float)fileCount, (float)(i+1)/(float)fileCount);

					// Load it
					try
					{
						// Load it 
						CIFile inFile;
						if (inFile.open (files[i]))
						{
							// Open an xml stream
							CIXml inXml;
							inXml.init (inFile);

							// Read it
							CPrimitives primitives;
							if (primitives.read (inXml.getRootNode (), files[i].c_str (), config))
							{
								// Look for primitives
								addPointPrimitive (landscape, files[i].c_str (), CFile::getFileModificationDate (files[i]), *primitives.RootNode, igs, options, formContainer, callback);
							}
							else
							{
								// Error in the log
								nlwarning ("Error : can't read the primitive file %s", files[i].c_str ());
							}
						}
						else
						{
							// Error in the log
							nlwarning ("Error : can't open the file (%s) for reading", files[i].c_str ());
						}
					}
					catch(const Exception &e)
					{
						// Error in the log
						nlwarning ("Error loading primitive file (%s) : %s", files[i].c_str (), e.what ());
					}

					// Progress bar
					callback.popCropedValues ();
				}
			}

			// *** Save igs
			
			for (sint y=miny; y<=maxy; y++)
			for (sint x=minx; x<=maxx; x++)
			{
				// Get the instance ref
				CIgContainer::CIG &instance = igs.get (x, y);

				// Good date ?
				if (instance.Date)
				{
					// Get the final filename
					string igFilename;
					getZoneNameByCoord (x, y, igFilename);
					igFilename = CPath::standardizePath (options.OutIGDir, true) + igFilename + ".ig";

					// Something in the ig ?
					if (!instance.Instances.empty ())
					{
						// Check date
						if (CFile::fileExists (igFilename) && (instance.Date < CFile::getFileModificationDate (igFilename)))
						{
							outString ("SKIP " + CFile::getFilename (igFilename) + "                                                       \n");
						}
						else
						{
							// *** Snap to ground

							// Get the zone bbox
							const CZone *zone = landscape.getZone (getZoneId (x, y));
							if (zone)
							{
								// Checks
								nlassert (instance.Instances.size () == instance.AdditionnalInfo.size ());

								// For each instances
								uint i;
								for (i=0; i<instance.Instances.size (); i++)
								{
									// Have to snap it ?
									if (instance.AdditionnalInfo[i].Snap)
									{
										// Progress bar
										callback.DisplayString = "Snap to ground " + CFile::getFilename (igFilename);
										callback.progress ((float)i/(float)instance.Instances.size ());

										// Get the zone bbox
										CAABBoxExt zoneBBox = zone->getZoneBB ();

										// The bbox used to select triangles
										CAABBox bbox;
										CVector &position = instance.Instances[i].Pos;
										bbox.setCenter (CVector (position.x + SELECTION_EPSILON, position.y + SELECTION_EPSILON, zoneBBox.getMax ().z + SELECTION_EPSILON));
										bbox.extend (CVector (position.x - SELECTION_EPSILON, position.y - SELECTION_EPSILON, zoneBBox.getMin ().z - SELECTION_EPSILON));

										// Select some triangles
										vector<CTrianglePatch> triangles;
										landscape.buildTrianglesInBBox (bbox, triangles, 0);

										// Ray trace triangles
										set<float> selectedHeight;
										uint j;
										for (j=0; j<triangles.size (); j++)
										{
											// Ref on triangle
											const CTriangle &triangle = triangles[j];

											// Test this triangle
											if (triangleIntersect2DGround (triangle, position))
											{
												// build the plane
												CPlane plane;
												plane.make (triangle.V0, triangle.V1, triangle.V2);

												// Get the final height
												CVector intersect = plane.intersect (bbox.getMin (), bbox.getMax ());
												selectedHeight.insert (intersect.z);
											}
										}

										// Get its layer
										uint layer = instance.AdditionnalInfo[i].SnapLayer;
										
										// Found some triangles ?
										const uint setSize = (uint)selectedHeight.size ();
										if (setSize)
										{
											// Look for the triangle in the good layer
											uint currentLayer = 0;

											// Layer exist ?
											if (layer >= setSize)
											{
												// Error in the log
												nlwarning ("Error : Layer %d used by the primitive (%s) in the file (%s) doesn't exist. Select layer %d instead.", 
													layer, instance.AdditionnalInfo[i].PrimitiveName.c_str (), 
													instance.AdditionnalInfo[i].PrimitiveFile.c_str (), setSize-1);

												// New layer
												layer = setSize-1;
											}

											// Invert the layer number
											layer = setSize - layer - 1;

											set<float>::iterator ite = selectedHeight.begin ();
											while (ite != selectedHeight.end ())
											{
												// Good layer ?
												if (currentLayer == layer)
													break;
												
												// Next layer
												currentLayer++;
												ite++;
											}

											// Should be found
											nlassert (ite != selectedHeight.end ());

											// Get the final height
											position.z = *ite;
										}
										else
										{
											// Error in the log
											nlwarning ("Error : No landscape under the primitive (%s) in the file (%s).", 
												instance.AdditionnalInfo[i].PrimitiveName.c_str (), instance.AdditionnalInfo[i].PrimitiveFile.c_str ());
										}
									}
								}
							}
							else
							{
								// Error in the log
								nlwarning ("Error : No landscape for the zone (%s)", CFile::getFilename (igFilename).c_str ());
							}

							// Build an instance group
							CInstanceGroup ig;
							CVector vGlobalPos = CVector::Null;
							vector<CCluster> Portals;
							vector<CPortal> Clusters;
							ig.build (vGlobalPos, instance.Instances, Portals, Clusters);

							// *** Save the ig file
							
							try 
							{
								// The file
								COFile outFile;
								if (outFile.open (igFilename))
								{
									ig.serial (outFile);

									// Done
									outString ("OK " + CFile::getFilename (igFilename) + "                                                   \n");
								}
								else
								{
									// Error in the log
									nlwarning ("Error : can't open the file (%s) for writing.", igFilename.c_str ());
								}
							}
							catch (const Exception &e)
							{
								// Error in the log
								nlwarning ("Error writing the file (%s) : %s", igFilename.c_str (), e.what ());
							}
						}
					}
					else
					{
						// File exist ?
						if (CFile::fileExists (igFilename))
						{
							// Done
							outString ("REMOVE " + CFile::getFilename (igFilename) + "                                              \n");

							// Remove it
							if (remove (igFilename.c_str ()) != 0)
							{
								// Error in the log
								nlwarning ("Error : Can't remove the file (%s)", igFilename.c_str ());
							}
						}
					}
				}
			}
		}
	}
	catch (const Exception &e)
	{
		string sTmp = string("ERROR : ") + e.what();
		outString (sTmp);
	}

	return 1;
}








/*









								// *** Snap to the ground

								// Get zone coordinates
								sint x = (sint)floor (position.x / options.CellSize);
								sint y = (sint)floor (position.y / options.CellSize);

*/
