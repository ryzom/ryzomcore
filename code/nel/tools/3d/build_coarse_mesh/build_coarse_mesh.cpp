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

#include "nel/3d/coarse_mesh_build.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/register_3d.h"

#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/bsphere.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;

// Shape file descriptor
class CShapeFileDesc
{
public:
	IShape			*Shape;
	std::string		FileName;
};

int main(int argc, char* argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter ("adding the path");

	// Register 3d
	registerSerial3d();

	// Get start time
	sint64 startTime=CTime::getLocalTime ();
	if (argc!=2)
	{
		// Doc..
		nlinfo("build_coarse_mesh [config_file.cfg]\n");
	}
	else
	{
		// Read the config file
		try
		{
			CConfigFile cf;

			// Load and parse "test.txt" file
			cf.load (argv[1]);

			// Get the texture_output variable (a string)
			CConfigFile::CVar &texture_output = cf.getVar ("output_textures");
			std::vector<std::string> texture_output_path;
			uint outputTextureCount = texture_output.size();
			texture_output_path.resize (outputTextureCount);
			uint i;
			for (i=0; i<outputTextureCount; i++)
			{
				// Get the i texture
				texture_output_path[i] = texture_output.asString (i);
			}

			// Get the texture_mul_size variable (a float)
			CConfigFile::CVar &texture_mul_size = cf.getVar ("texture_mul_size");
			float texture_mul_size_float = texture_mul_size.asFloat ();

			// Get the list_mesh variable (an array of string)
			CConfigFile::CVar &search_path = cf.getVar ("search_path");
			for (uint path=0; path<(uint)search_path.size(); path++)
			{
				// Add search pathes
				CPath::addSearchPath (search_path.asString (path));
			}

			// Get the list_mesh variable (an array of string)
			CConfigFile::CVar &list_mesh = cf.getVar ("list_mesh");

			// Vector of shapes
			std::vector<CShapeFileDesc> shapes;
			std::vector<CCoarseMeshBuild::CCoarseMeshDesc> coarseMeshes;
			coarseMeshes.reserve (list_mesh.size ());
			shapes.reserve (list_mesh.size ());

			// Load all the shapes
			uint list_mesh_count=list_mesh.size ()/2;
			for (i = 0; i < list_mesh_count; i++)
			{
				// Shape path
				std::string intputPath=list_mesh.asString (i*2);
				std::string outputPath=list_mesh.asString (i*2+1);

				// Output
				nlinfo ("Loading %s...\n", intputPath.c_str());

				// File
				CIFile inputFile;
				if (inputFile.open (CPath::lookup (intputPath.c_str())))
				{
					// Load a shape stream
					CShapeStream shapeStr;
					inputFile.serial (shapeStr);

					// Cast pointer
					CMeshMultiLod *multiLod=dynamic_cast<CMeshMultiLod*>(shapeStr.getShapePointer ());
					if (multiLod)
					{
						// Coarse mesh found
						bool found=false;

						// Get the coarse meshes
						uint count=multiLod->getNumSlotMesh ();
						uint j;
						for (j=0; j<count; j++)
						{
							// Is a coarse mesh
							bool coarseMesh;

							// Get a mesh
							IMeshGeom *iMeshGeom=multiLod->getSlotMesh (j, coarseMesh);

							// Is a coarse mesh ?
							if (coarseMesh)
							{
								// Cast
								CMeshGeom *meshGeom=safe_cast<CMeshGeom *> (iMeshGeom);

								// Shape not used ?
								if (!found)
								{
									// Build an info class
									CShapeFileDesc fileDesc;
									fileDesc.Shape=multiLod;
									fileDesc.FileName=outputPath;

									// Add it
									shapes.push_back (fileDesc);
								}

								// Found one
								found=true;

								// Add it in the list
								coarseMeshes.push_back (CCoarseMeshBuild::CCoarseMeshDesc (meshGeom, multiLod));
							}
						}

						// No coarse mesh ?
						if (!found)
						{
							// Output error message
							nlwarning ("ERROR this shape has no coarse mesh. %s\n", intputPath.c_str());

							// Delete the shape
							delete multiLod;
						}
					}
					else
					{
						// not good type
						nlwarning ("ERROR This mesh is not a multilod ! %s...\n", intputPath.c_str());
					}
				}
				else
				{
					// Output
					nlwarning ("ERROR file not found: %s\n", intputPath.c_str());
				}
			}

			// Some coarse mesh ?
			if (coarseMeshes.size()>0)
			{
				// Output text
				nlinfo ("Compute the coarse meshes...\n");

				// A bitmap
				vector<CBitmap> coarseBitmap;
				coarseBitmap.resize (texture_output_path.size ());
				
				// Build the coarse meshes
				CCoarseMeshBuild::CStats stats; 
				CCoarseMeshBuild builder;
				if (builder.build (coarseMeshes, coarseBitmap, stats, texture_mul_size_float))
				{
					// Save the shapes
					for (i=0; i<shapes.size(); i++)
					{
						// Output text
						nlinfo ("Write %s...\n", shapes[i].FileName.c_str());

						// File output
						COFile outputFile;
						if (outputFile.open(shapes[i].FileName))
						{
							// Load a shape stream
							CShapeStream shapeStr;
							shapeStr.setShapePointer (shapes[i].Shape);

							// Write
							outputFile.serial (shapeStr);
						}
						else
						{
							nlwarning ("ERROR can't write file %s\n", shapes[i].FileName.c_str());
						}
					}

					// Write the bitmaps
					for (i=0; i<texture_output_path.size (); i++)
					{
						nlinfo ("Write textures %s...\n", texture_output_path[i].c_str());

						// File for the texture
						COFile outputFile;
						if (outputFile.open (texture_output_path[i]))
						{
							// Write a TGA file
							if (!coarseBitmap[i].writeTGA (outputFile, 32))
							{
								nlwarning ("ERROR can't write the file %s\n", texture_output_path[i].c_str());
							}
						}
						else
						{
							nlwarning ("ERROR can't write file %s\n", texture_output_path[i].c_str());
						}
					}

					// Show stats
					nlinfo ("\nCoarse meshes computed: %d\nTexture size: %dx%d\nArea of the texture used: %f%%\nCompute time (s): %f\n", 
						shapes.size(), coarseBitmap[0].getWidth(), coarseBitmap[0].getHeight(), stats.TextureUsed*100.f, 
						((float)(uint32)CTime::getLocalTime ()-startTime)/1000.f);
				}
				else
				{
					nlwarning ("ERROR global texture too small to contain all textures.\nPlease increase the texture_mul_size parameter.\n");
				}
			}
			else
			{
				nlwarning ("WARNING no coarse mesh to compute, abort.\n");
			}
		}
		catch (const EConfigFile &e)
		{
			// Something goes wrong... catch that
			nlwarning ("ERROR %s\n", e.what ());
		}
		catch (const Exception &e)
		{
			// Something goes wrong... catch that
			nlwarning ("ERROR %s\n", e.what ());
		}
	}
	
	return 0;
}


