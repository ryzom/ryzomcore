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


#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include <math.h>
#include "lod_texture_builder.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/register_3d.h"
#include "nel/misc/config_file.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;


// ***************************************************************************
bool	computeOneShape(const char *lodFile, const char *shapeIn, const char *shapeOut)
{
	try
	{
		// Load the clod.
		CLodCharacterShapeBuild		theLod;
		CIFile	fIn;
		if(!fIn.open(lodFile))
			throw Exception("Can't load %s", lodFile);
		fIn.serial(theLod);
		fIn.close();

		// Load the shape.
		CSmartPtr<IShape>			theShape;
		if(!fIn.open(shapeIn))
			throw Exception("Can't load %s", shapeIn);
		CShapeStream	ss;
		fIn.serial(ss);
		if(!ss.getShapePointer())
			throw Exception("Can't load %s", shapeIn);
		theShape= ss.getShapePointer();
		fIn.close();

		// init the LodBuilder
		CLodTextureBuilder	lodBuilder;
		lodBuilder.setLod(theLod);

		// compute the texture.
		CLodCharacterTexture	lodTexture;
		CMesh		*mesh= dynamic_cast<CMesh*>((IShape*)theShape);
		CMeshMRM	*meshMRM= dynamic_cast<CMeshMRM*>((IShape*)theShape);
		CMeshMRMSkinned	*meshMRMSkinned= dynamic_cast<CMeshMRMSkinned*>((IShape*)theShape);
		CMeshBase	*base = NULL;
		if(mesh)
		{
			base = mesh;
			lodBuilder.computeTexture(*mesh, lodTexture);
		}
		else if(meshMRM)
		{
			base = meshMRM;
			lodBuilder.computeTexture(*meshMRM, lodTexture);
		}
		else if(meshMRMSkinned)
		{
			base = meshMRMSkinned;
			lodBuilder.computeTexture(*meshMRMSkinned, lodTexture);
		}
		else
			throw Exception("The shape %s is not a Mesh, a MeshMRM or MeshMMRMSkinned", shapeIn);

		// store in mesh
		nlassert (base);
		base->setupLodCharacterTexture(lodTexture);

		// serial
		COFile	fOut;
		if(!fOut.open(shapeOut))
			throw Exception("Can't open %s for writing", shapeOut);
		ss.setShapePointer(theShape);
		fOut.serial(ss);

		// TestYoyo
		/*CBitmap		dbg;
		dbg.resize(lodTexture.getWidth(), lodTexture.getHeight());
		memcpy(&dbg.getPixels(0)[0], &lodTexture.Texture[0], dbg.getSize()*4);
		COFile	dbgF("testDBG.tga");
		dbg.writeTGA(dbgF, 32);*/
	}
	catch(const Exception &e)
	{
		nlwarning("ERROR: %s", e.what());
		return false;
	}

	return true;
}


// ***************************************************************************
int main(int argc, char *argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	NLMISC::InfoLog->addNegativeFilter ("adding the path");

	NL3D::registerSerial3d();

	// What usage?
	bool	usageSingle= argc==4;
	bool	usageDir= argc==6 && argv[1]==string("-d");

	if (! (usageSingle || usageDir) )
	{
		string	execName= CFile::getFilename(argv[0]);
		printf("%s compute a Lod textureInfo to put in a shape\n", execName.c_str());
		printf("   usage 1: %s clod_in shape_in shape_out \n", execName.c_str());
		printf("   usage 2: %s -d clod_filters.cfg  clod_dir_in  shape_dir_in  shape_dir_out \n", execName.c_str());
		printf("      This usage type try to build lod_tex according to 'clod_tex_shape_filters' variable in the cfg.\n");
		printf("      'clod_tex_shape_filters' is a list of tuple: lod_file / shape file expression\n");
		printf("      eg: clod_tex_shape_filters= {\n");
		printf("                                  \"HOM_LOD\", \"FY_HOM*\", \n");
		printf("                                  \"HOM_LOD\", \"??_HOM*\", \n");
		printf("                                  }; \n");
		printf("      NB: unmatched shapes are just copied into dest directory\n");
		printf("      NB: if error in config_file, shapes are just copied into dest directory\n");
		printf("      NB: file date is checked, allowing caching\n");
		exit(-1);
	}


	if(usageSingle)
	{
		computeOneShape(argv[1], argv[2], argv[3]);
	}
	else
	{
		string	clod_cfg= argv[2];
		string	clod_dir_in= argv[3];
		string	shape_dir_in= argv[4];
		string	shape_dir_out= argv[5];

		// dir check
		if(!CFile::isDirectory(clod_dir_in))
			nlwarning("ERROR: %s is not a directory", clod_dir_in.c_str());
		if(!CFile::isDirectory(shape_dir_in))
			nlwarning("ERROR: %s is not a directory", shape_dir_in.c_str());
		if(!CFile::isDirectory(shape_dir_out))
			nlwarning("ERROR: %s is not a directory", shape_dir_out.c_str());

		// Open the CFG, and read the vars.
		vector<string>	LodNames;
		vector<string>	LodFilters;
		try
		{
			CConfigFile		fcfg;
			fcfg.load(clod_cfg);
			CConfigFile::CVar	&var= fcfg.getVar("clod_tex_shape_filters");
			if(var.size()<2)
				throw Exception("Must have 2+ strings in clod_tex_shape_filters");
			LodNames.resize(var.size()/2);
			LodFilters.resize(var.size()/2);
			for(uint i=0;i<LodNames.size();i++)
			{
				LodNames[i]= var.asString(i*2+0);
				LodFilters[i]= var.asString(i*2+1);
			}
		}
		catch(const Exception &e)
		{
			// It is not an error to have a bad config file: files will be copied
			nlwarning(e.what());
		}

		// List all files.
		vector<string>		fileList;
		CPath::getPathContent(shape_dir_in, false, false, true, fileList);

		// For all files.
		for(uint i=0;i<fileList.size();i++)
		{
			string	pathNameIn= fileList[i];
			string	fileNameIn= CFile::getFilename(pathNameIn);
			uint32	fileInDate= CFile::getFileModificationDate(pathNameIn);
			string	pathNameOut= shape_dir_out + "/" + fileNameIn;

			// Get the output file Date
			uint32		fileOutDate= 0;
			// If File Out exist 
			if(CFile::fileExists(pathNameOut))
			{
				// If newer than file In (and also newer than retrieverInfos), skip
				fileOutDate= CFile::getFileModificationDate(pathNameOut);
			}

			// search in all lods if the file Name match a filter
			uint	j;
			bool	skipped= false;
			for(j=0;j<LodFilters.size();j++)
			{
				// Make the test case-unsensitive
				string	lwrFileName= toLower(fileNameIn);
				string	lwrFilter= toLower(LodFilters[j]);
				if( testWildCard(lwrFileName.c_str(), lwrFilter.c_str()) )
				{
					string	clodFile= clod_dir_in+"/"+LodNames[j]+".clod";

					// test clod date against fileOut, only if the outFile is already newer than the fileIn
					if(fileOutDate>=fileInDate)
					{
						// if the out file is newer than the clod, then don't need rebuild
						uint32	clodDate= CFile::getFileModificationDate(clodFile);
						if(fileOutDate>=clodDate)
						{
							NLMISC::InfoLog->displayRaw("Skiping %s\n", fileNameIn.c_str());
							// skip other wildcards
							skipped= true;
							break;
						}
					}

					// Ok, try to do the compute.
					NLMISC::InfoLog->displayRaw("Processing %s", fileNameIn.c_str());
					if( computeOneShape( clodFile.c_str(), pathNameIn.c_str(), pathNameOut.c_str() ) )
					{
						// succed => stop
						NLMISC::InfoLog->displayRaw(" - CLod Textured with %s\n", LodNames[j].c_str());
						break;
					}
				}
			}

			// if skip this shape
			if(skipped)
				continue;

			// if fail to find a valid filter, just do a copy
			if(j==LodFilters.size())
			{
				CFile::copyFile(pathNameOut, pathNameIn);
				NLMISC::InfoLog->displayRaw("Processing %s - Copied\n", fileNameIn.c_str());
			}
		}
	}

	return 0;
}
