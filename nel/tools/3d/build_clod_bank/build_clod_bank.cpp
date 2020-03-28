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


#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include "nel/3d/lod_character_shape_bank.h"
#include "nel/3d/lod_character_shape.h"
#include "nel/3d/lod_character_builder.h"
#include "nel/3d/animation.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/register_3d.h"


using namespace std;
using namespace NLMISC;
using namespace NL3D;

int	main(int argc, char *argv[])
{
	uint	i;

	// Avoid warnings.
	NLMISC::createDebug();
	DebugLog->addNegativeFilter("Exception will be launched");
	WarningLog->addNegativeFilter("Exception will be launched");
	InfoLog->addNegativeFilter("FEHTIMER>");
	InfoLog->addNegativeFilter("adding the path");

	// Init serial
	registerSerial3d();

	if(argc<4)
	{
		puts("Usage:    build_clod_bank  path_file.cfg  config_file.cfg  destfile.clodbank  [bakeFrameRate=20] ");
		return 0;
	}


	try
	{
		// The bank to fill
		CLodCharacterShapeBank	lodShapeBank;


		// Read the frameRate to process bake of anims
		float	bakeFrameRate= 20;
		if(argc>=5)
		{
			NLMISC::fromString(argv[4], bakeFrameRate);
			if(bakeFrameRate<=1)
			{
				nlwarning("bad bakeFrameRate value, use a default of 20");
				bakeFrameRate= 20;
			}
		}


		// parse the path file.
		//==================

		// try to load the config file.
		CConfigFile pathConfig;
		pathConfig.load (argv[1]);

		// Get the search pathes
		CConfigFile::CVar &search_pathes = pathConfig.getVar ("search_pathes");
		for (i = 0; i < (uint)search_pathes.size(); i++)
		{
			// Add to search path
			CPath::addSearchPath (search_pathes.asString(i));
		}

		// parse the config file.
		//==================

		// try to load the config file.
		CConfigFile config;
		config.load (argv[2]);

		// For all .clod to process
		//==================
		CConfigFile::CVar &clod_list = config.getVar ("clod_list");
		uint	lodId;
		for (lodId = 0; lodId < (uint)clod_list.size(); lodId++)
		{
			string	lodName= clod_list.asString(lodId);

			printf("Process LOD: %s\n", lodName.c_str());

			// Search the variable with this name.
			try
			{
				CIFile		iFile;

				// get the anim list.
				CConfigFile::CVar &clod_anim_list = config.getVar (lodName);

				// Correct format?
				if(clod_anim_list.size()<3)
				{
					nlwarning("%s skipped. Must have at least the skeleton file name, the lod file name, and one animation", lodName.c_str());
					// go to next.
					continue;
				}

				// Init lod shape process
				//===========================

				// The first variable is the name of the skeleton.
				string	skeletonName= clod_anim_list.asString(0);
				CSmartPtr<CSkeletonShape>	skeletonShape;

				// Load it.
				iFile.open(CPath::lookup(skeletonName));
				CShapeStream		strShape;
				strShape.serial(iFile);
				iFile.close();

				// Get the pointer, check it's a skeleton
				if(dynamic_cast<CSkeletonShape*>(strShape.getShapePointer()) == NULL)
					throw Exception("%s is not a Skeleton", skeletonName.c_str());
				skeletonShape= (CSkeletonShape*)strShape.getShapePointer();

				// The second var is the filename of the lod.
				string	lodFileName= clod_anim_list.asString(1);

				// Load the shape.
				CLodCharacterShapeBuild		lodShapeBuild;
				iFile.open( CPath::lookup(lodFileName) );
				iFile.serial(lodShapeBuild);
				iFile.close();

				// Prepare to build the lod.
				CLodCharacterBuilder		lodBuilder;
				lodBuilder.setShape(lodName, skeletonShape, &lodShapeBuild);


				// Traverse all anim in the list.
				//===========================
				uint	animId;
				for (animId = 2; animId < (uint)clod_anim_list.size(); animId++)
				{
					string	animFileName= clod_anim_list.asString(animId);

					// display.
					printf("Process Anim: %d/%d\r", animId-1, clod_anim_list.size()-2);

					// Try to load the animation
					CAnimation			*anim= new CAnimation;
					// NB: continue, to list all ANIM if anim not found
					try
					{
						iFile.open( CPath::lookup(animFileName) );
						iFile.serial(*anim);
						iFile.close();
						// Add to the builder. NB: animation will be delete in this method.
						// NB: the key name here is the entire file, with the .anim, for easier georges editing.
						lodBuilder.addAnim(animFileName.c_str(), anim, bakeFrameRate);
					}
					catch(const EPathNotFound &)
					{
						printf("ERROR anim not found %s\n", animFileName.c_str());
						delete	anim;
					}
				}
				printf("\n");

				// Add to the bank.
				//===========================
				uint32	shapeId= lodShapeBank.addShape();
				*lodShapeBank.getShapeFullAcces(shapeId)= lodBuilder.getLodShape();
			}
			catch(const EUnknownVar &evar)
			{
				nlwarning(evar.what());
				// Any other exception will make the program quit.
			}

		}

		// Save bank.
		//===========================

		// compile
		lodShapeBank.compile();

		// Save
		COFile	oFile(argv[3]);
		oFile.serial(lodShapeBank);
		oFile.close();
	}
	catch (const Exception& except)
	{
		// Error message
		printf ("ERROR %s.\n Aborting.\n", except.what());
	}


	return 0;
}
