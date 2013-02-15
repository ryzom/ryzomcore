// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "nel/misc/vector.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"

#include "nel/3d/instance_lighter.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/shape.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/register_3d.h"
#include "nel/pacs/global_retriever.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/retriever_bank.h"
#include "../ig_lighter_lib/ig_lighter_lib.h"


using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

// ***************************************************************************
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



// My Ig lighter
class CMyIgLighter : public CInstanceLighter
{
public:
	static void	displayProgress(const char *message, float progress)
	{
		// Progress bar
		char msg[512];
		uint	pgId= (uint)(progress*(float)BAR_LENGTH);
		pgId= min(pgId, (uint)(BAR_LENGTH-1));
		sprintf (msg, "\r%s: %s", message, progressbar[pgId]);
		uint i;
		for (i=(uint)strlen(msg); i<79; i++)
			msg[i]=' ';
		msg[i]=0;
		printf ("%s\r", msg);
	}

protected:
	// Progress bar
	virtual void progress (const char *message, float progress)
	{
		displayProgress(message, progress);
	}
};


// ***************************************************************************
void	lightIg(const NL3D::CInstanceGroup &igIn, NL3D::CInstanceGroup &igOut, NL3D::CInstanceLighter::CLightDesc &lightDesc, 
		CIgLighterLib::CSurfaceLightingInfo &slInfo, const char *igName)
{
	// Create an instance of MyIgLighter.
	CMyIgLighter	lighter;
	// lightIg
	CIgLighterLib::lightIg(lighter, igIn, igOut, lightDesc, slInfo, igName);
}


// ***************************************************************************
int main(int argc, char* argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter ("adding the path");

	// Register 3d
	registerSerial3d ();

	// Good number of args ?
	if (argc<4)
	{
		// Help message
		printf ("ig_lighter [directoryIn] [pathOut] [parameter_file] \n");
	}
	else
	{
		try
		{
			string	directoryIn= argv[1];
			string	pathOut= argv[2];
			string	paramFile= argv[3];
			CInstanceLighter::CLightDesc	lighterDesc;
			string	grFile, rbankFile;

			// Verify directoryIn.
			directoryIn= CPath::standardizePath(directoryIn);
			if( !CFile::isDirectory(directoryIn) )
			{
				printf("DirectoryIn %s is not a directory", directoryIn.c_str());
				return -1;
			}
			// Verify pathOut.
			pathOut= CPath::standardizePath(pathOut);
			if( !CFile::isDirectory(pathOut) )
			{
				printf("PathOut %s is not a directory", pathOut.c_str());
				return -1;
			}

			// Load and setup configFile.
			//=================
			CConfigFile parameter;
			// Load and parse the param file
			parameter.load (paramFile);

			// Get the search pathes
			CConfigFile::CVar &search_pathes = parameter.getVar ("search_pathes");
			uint path;
			for (path = 0; path < (uint)search_pathes.size(); path++)
			{
				// Add to search path
				CPath::addSearchPath (search_pathes.asString(path));
			}

			// Light direction
			CConfigFile::CVar &sun_direction = parameter.getVar ("sun_direction");
			lighterDesc.LightDirection.x=sun_direction.asFloat(0);
			lighterDesc.LightDirection.y=sun_direction.asFloat(1);
			lighterDesc.LightDirection.z=sun_direction.asFloat(2);
			lighterDesc.LightDirection.normalize ();

			// Grid size
			CConfigFile::CVar &quad_grid_size = parameter.getVar ("quad_grid_size");
			lighterDesc.GridSize=quad_grid_size.asInt();

			// Grid size
			CConfigFile::CVar &quad_grid_cell_size = parameter.getVar ("quad_grid_cell_size");
			lighterDesc.GridCellSize=quad_grid_cell_size.asFloat();

			// Shadows enabled ?
			CConfigFile::CVar &shadow = parameter.getVar ("shadow");
			lighterDesc.Shadow=shadow.asInt ()!=0;

			// OverSampling
				CConfigFile::CVar &ig_oversampling = parameter.getVar ("ig_oversampling");
				lighterDesc.OverSampling= ig_oversampling.asInt ();
			// validate value: 0, 2, 4, 8, 16
			lighterDesc.OverSampling= raiseToNextPowerOf2(lighterDesc.OverSampling);
			clamp(lighterDesc.OverSampling, 0U, 16U);
			if(lighterDesc.OverSampling<2)
				lighterDesc.OverSampling= 0;

			// gr
			CConfigFile::CVar &grbank = parameter.getVar ("grbank");
			grFile= grbank.asString ();

			// rbank
			CConfigFile::CVar &rbank = parameter.getVar ("rbank");
			rbankFile= rbank.asString ();

			// CellSurfaceLightSize;
			CConfigFile::CVar &cell_surface_light_size = parameter.getVar ("cell_surface_light_size");
			float cellSurfaceLightSize= cell_surface_light_size.asFloat ();
			if(cellSurfaceLightSize<=0)
				throw Exception("cell_surface_light_size must be > 0");

			// CellRaytraceDeltaZ
			CConfigFile::CVar &cell_raytrace_delta_z = parameter.getVar ("cell_raytrace_delta_z");
			float cellRaytraceDeltaZ= cell_raytrace_delta_z.asFloat ();


			// colIdentifierPrefix
			CConfigFile::CVar &col_identifier_prefix = parameter.getVar ("col_identifier_prefix");
			string colIdentifierPrefix= col_identifier_prefix.asString ();

			// colIdentifierSuffix
			CConfigFile::CVar &col_identifier_suffix = parameter.getVar ("col_identifier_suffix");
			string colIdentifierSuffix= col_identifier_suffix.asString ();

			// colIdentifierSuffix
			CConfigFile::CVar &build_debug_surface_shape = parameter.getVar ("build_debug_surface_shape");
			bool	buildDebugSurfaceShape= build_debug_surface_shape.asInt()!=0;
			

			// try to open gr and rbank
			CRetrieverBank		*retrieverBank= NULL;
			CGlobalRetriever	*globalRetriever= NULL;
			uint32		grFileDate= 0;
			uint32		rbankFileDate= 0;
			if( grFile!="" && rbankFile!="" )
			{
				CIFile	fin;
				// serial the retrieverBank. Exception if not found.
				fin.open(CPath::lookup(rbankFile));
				retrieverBank= new CRetrieverBank;
				retrieverBank->setNamePrefix(CFile::getFilenameWithoutExtension(rbankFile).c_str ());

				// Add the search path for LR files
				CPath::addSearchPath (CFile::getPath(rbankFile));

				fin.serial(*retrieverBank);
				fin.close();

				// serial the globalRetriever. Exception if not found.
				fin.open(CPath::lookup(grFile));
				globalRetriever= new CGlobalRetriever;

				// set the RetrieverBank before loading
				globalRetriever->setRetrieverBank(retrieverBank);
				fin.serial(*globalRetriever);
				fin.close();

				// Get File Dates
				rbankFileDate= CFile::getFileModificationDate(CPath::lookup(rbankFile));
				grFileDate= CFile::getFileModificationDate(CPath::lookup(grFile));

				// And init them.
				globalRetriever->initAll();
			}


			// Scan and load all files .ig in directories
			//=================
			vector<string>				listFile;
			vector<CInstanceGroup*>		listIg;
			vector<string>				listIgFileName;
			vector<string>				listIgPathName;
			CPath::getPathContent(directoryIn, false, false, true, listFile);
			for(uint iFile=0; iFile<listFile.size(); iFile++)
			{
				string	&igFile= listFile[iFile];
				// verify it is a .ig.
				if( CFile::getExtension(igFile) == "ig" )
				{
					// Read the InstanceGroup.
					CInstanceGroup	*ig= new CInstanceGroup;
					CIFile	fin;
					fin.open(CPath::lookup(igFile));
					fin.serial(*ig);

					// add to list.
					listIg.push_back(ig);
					listIgPathName.push_back(igFile);
					listIgFileName.push_back(CFile::getFilename(igFile));
				}
			}


			// For all ig, light them, and save.
			//=================
			for(uint iIg= 0; iIg<listIg.size(); iIg++)
			{
				string	fileNameIn= listIgFileName[iIg];
				string	fileNameOut= pathOut + fileNameIn;

				// If File Out exist 
				if(CFile::fileExists(fileNameOut))
				{
					// If newer than file In (and also newer than retrieverInfos), skip
					uint32		fileOutDate= CFile::getFileModificationDate(fileNameOut);
					if(	fileOutDate > CFile::getFileModificationDate(listIgPathName[iIg]) &&
						fileOutDate > rbankFileDate && 
						fileOutDate > grFileDate 
						)
					{
						printf("Skiping %s\n", fileNameIn.c_str());
						continue;
					}
				}

				// progress
				printf("Processing %s\n", fileNameIn.c_str());

				CInstanceGroup	igOut;

				// Export a debugSun Name.
				string	debugSunName;
				debugSunName= pathOut + "/" + CFile::getFilenameWithoutExtension(fileNameIn) + "_debug_sun_.shape";
				string	debugPLName;
				debugPLName= pathOut + "/" + CFile::getFilenameWithoutExtension(fileNameIn) + "_debug_pl_.shape";

				// light the ig.
				CIgLighterLib::CSurfaceLightingInfo	slInfo;
				slInfo.CellSurfaceLightSize= cellSurfaceLightSize;
				slInfo.CellRaytraceDeltaZ= cellRaytraceDeltaZ;
				slInfo.RetrieverBank= retrieverBank;
				slInfo.GlobalRetriever= globalRetriever;
				slInfo.IgFileName= CFile::getFilenameWithoutExtension(fileNameIn);
				slInfo.ColIdentifierPrefix= colIdentifierPrefix;
				slInfo.ColIdentifierSuffix= colIdentifierSuffix;
				slInfo.BuildDebugSurfaceShape= buildDebugSurfaceShape;
				slInfo.DebugSunName= debugSunName;
				slInfo.DebugPLName= debugPLName;
				lightIg(*listIg[iIg], igOut, lighterDesc, slInfo, fileNameIn.c_str ());

				// Save this ig.
				COFile	fout;
				fout.open(fileNameOut);
				fout.serial(igOut);
				fout.close();

				// skip a line
				printf("\n");
			}

		}
		catch (const Exception& except)
		{
			// Error message
			nlwarning ("ERROR %s\n", except.what());
		}
	}


	// Landscape is not deleted, nor the instanceGroups, for faster quit.
	// Must disalbe BlockMemory checks (for pointLights).
	NL3D_BlockMemoryAssertOnPurge= false;


	// exit.
	return 0;
}
