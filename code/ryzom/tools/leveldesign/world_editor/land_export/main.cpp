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

#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"

#include "../land_export_lib/export.h"
#include "nel/ligo/zone_region.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ---------------------------------------------------------------------------
// Out a string to the stdout and log.log
void outString (const string &sText)
{
	createDebug ();
	InfoLog->displayRaw(sText.c_str());
	//printf ("%s", sText.c_str());
}

// ---------------------------------------------------------------------------
class CExportCB : public IExportCB
{
	int _PassProgress;
	int _NiouLine;
public:

	bool isCanceled ()
	{
		return false; // Never ending story
	}

	// Display callbacks

	void dispPass (const std::string &Text)
	{
		/* string sTmp = string("PASS : ") + Text + "\n";
		outString (sTmp); */
		_PassProgress = 0;
		_NiouLine = 0;
	}

	void dispPassProgress (float percentage)
	{
		if (_PassProgress != (int)(percentage*100.0f))
		{
			_PassProgress = (int)(percentage*100.0f);
			string sTmp = toString(_PassProgress) + ".";
			if ((_NiouLine++) == 20)
			{
				_NiouLine = 0;
				sTmp += "\n";
			}
			outString (sTmp);
			fflush (stdout);
		}
	}

	void dispInfo (const std::string &Text)
	{
		/* string sTmp = string("INFO : ") + Text + "\n";
		outString (sTmp); */
	}

	void dispWarning (const std::string &Text)
	{
		string sTmp = string("WARNING : ") + Text + "\n";
		outString (sTmp);
	}

	void dispError (const std::string &Text)
	{
		string sTmp = string("ERROR : ") + Text + "\n";
		outString (sTmp);
		exit (-1);
	}

};

// ---------------------------------------------------------------------------
struct SOptions : public SExportOptions
{
	string ZoneRegionFile;
	string FileName;

	CConfigFile cf;

	string getStr (const string &var)
	{
		string ret;
		try
		{
			CConfigFile::CVar &cvString = cf.getVar (var);
			ret = cvString.asString ();
		}
		catch (const EConfigFile &/*e*/)
		{
			outString (string("WARNING : variable not found : ") + var + " in " + FileName + "\n");
		}
		return ret;
	}

	float getFloat (const string &var)
	{
		float ret = 0.0;
		try
		{
			CConfigFile::CVar &cvString = cf.getVar (var);
			ret = cvString.asFloat ();
		}
		catch (const EConfigFile &/*e*/)
		{
			outString (string("WARNING : variable not found : ") + var + " in " + FileName + "\n");
		}
		return ret;
	}

	sint32 getInt (const string &var)
	{
		sint32 ret = 0;
		try
		{
			CConfigFile::CVar &cvString = cf.getVar (var);
			ret = cvString.asInt ();
		}
		catch (const EConfigFile &/*e*/)
		{
			outString (string("WARNING : variable not found : ") + var + " in " + FileName + "\n");
		}
		return ret;
	}

	bool loadFromCfg (const string &filename)
	{
		FileName = filename;
		FILE * f = fopen (filename.c_str(), "rt");
		if (f == NULL)
			return false;
		else 
			fclose (f);
		
		
		try
		{			
			cf.load (filename);
			this->OutZoneDir = getStr ("OutZoneDir");
			this->OutIGDir = getStr ("OutIGDir");
			this->RefZoneDir = getStr ("RefZoneDir");
			this->RefIGDir = getStr ("RefIGDir");
			this->LigoBankDir = getStr ("LigoBankDir");
			this->TileBankFile = getStr ("TileBankFile");
			this->ColorMapFile = getStr ("ColorMapFile");
			this->HeightMapFile = getStr ("HeightMapFile1");
			this->ZFactor = getFloat ("ZFactor1");
			this->HeightMapFile2 = getStr ("HeightMapFile2");
			this->ZFactor2 = getFloat ("ZFactor2");
			this->Light = (uint8)getInt ("ZoneLight");
			this->CellSize = getFloat ("CellSize");
			this->Threshold = getFloat ("Threshold");
			this->ZoneRegionFile = getStr ("ZoneRegionFile");						
			this->ContinentFile = getStr("ContinentFile");			
			this->RefCMBDir = getStr("RefCMBDir");
			this->OutCMBDir = getStr("OutCMBDir");			
			this->AdditionnalIGInDir = getStr("AdditionnalIGInDir");
			this->AdditionnalIGOutDir = getStr("AdditionnalIGOutDir");
			this->ExportCollisions = getInt("ExportCollisions") != 0;						
			this->ExportAdditionnalIGs = getInt("ExportAdditionnalIGs") != 0;			
			this->DFNDir = getStr("DFNDir");
			this->ContinentsDir = getStr("ContinentsDir");
		}
		catch (const EConfigFile &e)
		{
			string sTmp = string("ERROR : Error in config file : ") + e.what() + "\n";
			outString (sTmp);
			return false;
		}
		return true;
	}

	bool loadLand (const string &filename)
	{
		try
		{
			CIFile fileIn;
			if (fileIn.open (filename))
			{
				CIXml xml (true);
				xml.init (fileIn);
				ZoneRegion = new CZoneRegion;
				ZoneRegion->serial (xml);
			}
			else
			{
				string sTmp = string("Can't open file : ") + filename;
				outString (sTmp);
				return false;
			}
		}
		catch (const Exception& e)
		{
			string sTmp = string("Error in land file : ") + e.what();
			outString (sTmp);
			return false;
		}
		return true;
	}
};

// ---------------------------------------------------------------------------
int main (int argc, char**argv)
{
	new NLMISC::CApplicationContext;
	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter ("addSearchPath");

	// Verify the number of arguments
	if (argc != 2)
	{
		printf ("Use : land_export configfile.cfg\n");
		printf ("\nExample of config.cfg\n\n");
		printf ("// Where to put all .zone generated\n");
		printf ("OutZoneDir = \"\";\n");
		printf ("// Where the reference zones are\n");
		printf ("RefZoneDir = \"\";\n");
		printf ("// Where the reference igs are\n");
		printf ("RefIGDir = \"\";\n");
		printf ("// Where to put all .ig generated\n");
		printf ("OutIGDir = \"\";\n");
		printf ("// Where all .ligozone are (those used by the .land)\n");
		printf ("LigoBankDir = \"\";\n");
		printf ("// The .bank file (used to know if a tile is oriented and the like)\n");
		printf ("TileBankFile = "";\n");
		printf ("// The grayscale tga file (127 is 0, 0 is -127*ZFactor and 255 is 128*ZFactor)\n");
		printf ("HeightMapFile1 = \"\";\n");
		printf ("// The heightmap factor\n");
		printf ("ZFactor1 = 1.0;\n");
		printf ("// The second grayscale tga file\n");
		printf ("HeightMapFile2 = \"\";\n");
		printf ("// The 2nd heightmap factor\n");
		printf ("ZFactor2 = 1.0;\n");
		printf ("// Roughly light the zone (0-none, 1-patch, 2-noise)\n");
		printf ("ZoneLight = 0;\n");
		printf ("// A cellsize (zonesize)\n");
		printf ("CellSize = 0.0;\n");
		printf ("// The .land to compute.\n");
		printf ("ZoneRegionFile = \"\";\n");
		printf ("// 0 if collisions should not be exported.\n");
		printf ("ExportCollisions = 0;\n");
		printf ("// 0 if additionnal igs should not be exported.\n");
		printf ("ExportAdditionnalIGs = 0;\n");
		printf ("// where the collision mesh builds are. They must match a village ig in the .continent that match the .land name.\n");
		printf ("RefCMBDir = \"\";\n");
		printf ("// where the collision mesh builds are put after being translated.\n");
		printf ("OutCMBDir = \"\";\n");
		printf ("// where the additionnal instance groups are. They must match a village ig in the .continent that match the .land name.\n");
		printf ("AdditionnalIGInDir = \"\";\n");
		printf ("// where the additionnal instance groups are put after being translated.\n");
		printf ("AdditionnalIGOutDir = \"\";\n");
		printf ("// where the dfn are located.\n");
		printf ("DFNDir = \"\";\n");		
		printf("// the continent containing cmb and igs to export");
		printf ("ContinentFile = \"\";\n");
		printf("// search path for continents");
		printf ("ContinentsDir = \"\";\n");


		return -1;
	}

	SOptions options;
	CExportCB exportCB;
	CExport exporter;

	string sTmp = toString("Loading cfg file : ") + argv[1] + "\n";
	outString (sTmp);
	if (!options.loadFromCfg (argv[1]))
		return -1;

	sTmp = string("Loading land file : ") + options.ZoneRegionFile + "\n";
	outString (sTmp);
	if (!options.loadLand (options.ZoneRegionFile))
		return -1;

	exporter.export_ (options, &exportCB);

	return 1;
}