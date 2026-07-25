/** \file zone_lighter.cpp
 * zone_lighter.cpp : Very simple zone lighter
 *
 * $Id$
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "nel/misc/vector.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/app_context.h"
#include "nel/misc/path.h"

#include "nel/3d/zone.h"
#include "zone_lighter_2001.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/landscape.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/shape.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/register_3d.h"

#include "../zone_lib/zone_utility.h"	// load a header file from zone_welder project

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#define BAR_LENGTH 21

char *progressbar[BAR_LENGTH]=
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

// My zone lighter
class CMyZoneLighter : public CZoneLighter2001
{
	// Progress bar
	virtual void progress (const char *message, float progress) NL_OVERRIDE
	{
		// Progress bar
		char msg[512];
		sprintf (msg, "\r%s: %s", message, progressbar[(uint)(progress*(float)BAR_LENGTH)]);
		uint i;
		for (i=strlen(msg); i<79; i++)
			msg[i]=' ';
		msg[i]=0;
		printf ("%s", msg);
		printf ("\r");
	}
};

int main(int argc, char* argv[])
{
	// Modern NLMISC requires an application context (did not exist in 2001)
	NLMISC::CApplicationContext applicationContext;

	// Register 3d
	registerSerial3d ();

	// Good number of args ?
	if (argc<5)
	{
		// Help message
		printf ("zone_lighter [zonein.zone] [zoneout.zone] [parameter_file] [dependancy_file]\n");
	}
	else
	{
		// Ok, read the zone
		CIFile inputFile;

		// Get extension
		string ext=getExt (argv[1]);
		string dir=getDir (argv[1]);

		// Open it for reading
		if (inputFile.open (argv[1]))
		{
			// Zone name
			string zoneName=strlwr (string ("zone_"+getName (argv[1])));

			// Load the zone
			try
			{
				// Read the config file
				CConfigFile parameter;
				CConfigFile dependency;

				// Load and parse the dependency file
				parameter.load (argv[3]);

				// Load and parse the dependency file
				dependency.load (argv[4]);

				// Fill the pathes
				string ig_path = parameter.getVar ("ig_path").asString();
				if ((ig_path[ig_path.length()-1]!='\\')&&(ig_path[ig_path.length()-1]!='/'))
					ig_path+="/";
				string shapes_path = parameter.getVar ("shapes_path").asString();
				if ((shapes_path[shapes_path.length()-1]!='\\')&&(shapes_path[shapes_path.length()-1]!='/'))
					shapes_path+="/";

				// Get the maps path
				string maps_path = parameter.getVar ("maps_path").asString();

				// Add it to the current path
				CPath::addSearchPath (maps_path);

				// A landscape allocated with new: it is not delete because destruction take 3 secondes more!
				CLandscape *landscape=new CLandscape;
				landscape->init();

				// Era reproduction: the 2001 bank has only empty displacement maps; the
				// modern getTileNoiseMap substitutes a placeholder pattern for missing
				// bitmaps, so disable noise to reproduce the era's clean no-displacement
				// tessellation (also available on the modern tool as the same env).
				if (getenv ("NL_ZONE_LIGHTER_NO_NOISE"))
					landscape->setNoiseMode (false);

				// A zone lighter
				CMyZoneLighter lighter;
				lighter.init ();

				// A vector of zone id
				vector<uint> listZoneId;

				// The zone
				CZone zone;

				// List of ig
				std::list<CInstanceGroup*> instanceGroup;

				// Load
				zone.serial (inputFile);
				inputFile.close();

				// Load ig of the zone
				string igName=ig_path+getName (argv[1])+".ig";

				// Try to open the file
				if (inputFile.open (igName))
				{
					// New ig
					CInstanceGroup *group=new CInstanceGroup;

					// Serial it
					group->serial (inputFile);
					inputFile.close();

					// Add to the list
					instanceGroup.push_back (group);
				}
				else
				{
					// Warning
					fprintf (stderr, "Warning: can't load instance group %s\n", igName.c_str());
				}

				// Get bank path
				CConfigFile::CVar &bank_path = parameter.getVar ("bank_path");

				// Load the bank
				if (inputFile.open (bank_path.asString()))
				{
					try
					{
						// Load
						landscape->TileBank.serial (inputFile);
						landscape->initTileBanks();
					}
					catch (Exception &e)
					{
						// Error
						nlwarning ("ERROR error loading tile bank %s\n%s\n", bank_path.asString().c_str(), e.what());
					}
				}
				else
				{
					// Error
					nlwarning ("ERROR can't load tile bank %s\n", bank_path.asString().c_str());
				}

				// Add the zone
				landscape->addZone (zone);
				listZoneId.push_back (zone.getZoneId());

				// Load instance group ?
				CConfigFile::CVar &load_ig= parameter.getVar ("load_ig");
				bool loadInstanceGroup = load_ig.asInt ()!=0;

				// Continue to build ?
				bool continu=true;

				// Try to load additionnal instance group.
				if (loadInstanceGroup)
				{
					// Additionnal instance group
					CConfigFile::CVar &additionnal_ig= parameter.getVar ("additionnal_ig");
					for (uint add=0; add<(uint)additionnal_ig.size(); add++)
					{
						// Input file
						CIFile inputFile;

						// Name of the instance group
						string name=ig_path+additionnal_ig.asString(add);

						// Try to open the file
						if (inputFile.open (name))
						{
							// New ig
							CInstanceGroup *group=new CInstanceGroup;

							// Serial it
							group->serial (inputFile);
							inputFile.close();

							// Add to the list
							instanceGroup.push_back (group);
						}
						else
						{
							// Error
							nlwarning ("ERROR can't load instance group %s\n", name.c_str());

							// Stop before build
							continu=false;
						}
					}
				}
				
				// *** Scan dependency file
				CConfigFile::CVar &dependant_zones = dependency.getVar ("dependencies");
				for (uint i=0; i<(uint)dependant_zones.size(); i++)
				{
					// Get zone name
					string zoneName=dependant_zones.asString(i);

					// Load the zone
					CZone zoneBis;

					// Open it for reading
					if (inputFile.open (dir+zoneName+ext))
					{
						// Read it
						zoneBis.serial (inputFile);
						inputFile.close();

						// Add the zone
						landscape->addZone (zoneBis);
						listZoneId.push_back (zoneBis.getZoneId());
					}
					else
					{
						// Error message and continue
						nlwarning ("ERROR can't load zone %s\n", (dir+zoneName+ext).c_str());
					}

					// Try to load an instance group.
					if (loadInstanceGroup)
					{
						string name=ig_path+zoneName+".ig";

						// Name of the instance group
						if (inputFile.open (name))
						{
							// New ig
							CInstanceGroup *group=new CInstanceGroup;

							// Serial it
							group->serial (inputFile);
							inputFile.close();

							// Add to the list
							instanceGroup.push_back (group);
						}
						else
						{
							// Error message and continue
							nlwarning ("WARNING can't load instance group %s\n", name.c_str());
						}
					}
				}

				// **********
				// *** Build the lighter descriptor
				// **********

				CZoneLighter2001::CLightDesc lighterDesc;

				// Grid size
				CConfigFile::CVar &quad_grid_size = parameter.getVar ("quad_grid_size");
				lighterDesc.GridSize=quad_grid_size.asInt();

				// Grid size
				CConfigFile::CVar &quad_grid_cell_size = parameter.getVar ("quad_grid_cell_size");
				lighterDesc.GridCellSize=quad_grid_cell_size.asFloat();

				// Heightfield cell size
				CConfigFile::CVar &global_illumination_cell_size = parameter.getVar ("global_illumination_cell_size");
				lighterDesc.HeightfieldCellSize=global_illumination_cell_size.asFloat();

				// Heightfield size
				CConfigFile::CVar &global_illumination_length = parameter.getVar ("global_illumination_length");
				lighterDesc.HeightfieldSize=global_illumination_length.asFloat();

				// Light direction
				CConfigFile::CVar &sun_direction = parameter.getVar ("sun_direction");
				lighterDesc.LightDirection.x=sun_direction.asFloat(0);
				lighterDesc.LightDirection.y=sun_direction.asFloat(1);
				lighterDesc.LightDirection.z=sun_direction.asFloat(2);
				lighterDesc.LightDirection.normalize ();

				// Oversampling
				CConfigFile::CVar &oversampling = parameter.getVar ("oversampling");
				sint oversmaplingValue=oversampling.asInt();
				switch (oversmaplingValue)
				{
				case 0:
					lighterDesc.Oversampling=CZoneLighter2001::CLightDesc::NoOverSampling;
					break;
				case 2:
					lighterDesc.Oversampling=CZoneLighter2001::CLightDesc::OverSamplingx2;
					break;
				case 8:
					lighterDesc.Oversampling=CZoneLighter2001::CLightDesc::OverSamplingx8;
					break;
				case 32:
					lighterDesc.Oversampling=CZoneLighter2001::CLightDesc::OverSamplingx32;
					break;
				case 128:
					lighterDesc.Oversampling=CZoneLighter2001::CLightDesc::OverSamplingx128;
					break;
				default:
					// Error message
					nlwarning ("ERROR oversampling value not supported. Must be 0, 2, 8, 32 or 128. Forced to 0.\n");
					lighterDesc.Oversampling=CZoneLighter2001::CLightDesc::NoOverSampling;
					break;
				}

				// Number of CPU
				CConfigFile::CVar &cpu_num = parameter.getVar ("cpu_num");
				lighterDesc.NumCPU=cpu_num.asInt ();

				// Shadow bias
				CConfigFile::CVar &shadow_bias = parameter.getVar ("shadow_bias");
				lighterDesc.ShadowBias=shadow_bias.asFloat ();

				// Softshadow
				CConfigFile::CVar &softshadow = parameter.getVar ("softshadow");
				lighterDesc.Softshadow=softshadow.asInt ()!=0;

				// Softshadow blur size
				CConfigFile::CVar &softshadow_blur_size = parameter.getVar ("softshadow_blur_size");
				lighterDesc.SoftshadowBlurSize=softshadow_blur_size.asFloat ();

				// Softshadow fall
				CConfigFile::CVar &softshadow_fallof = parameter.getVar ("softshadow_fallof");
				lighterDesc.SoftshadowFallof=softshadow_fallof.asFloat ();

				// Softshadow fall
				CConfigFile::CVar &softshadow_shape_vertex_count = parameter.getVar ("softshadow_shape_vertex_count");
				lighterDesc.SoftshadowShapeVertexCount=softshadow_shape_vertex_count.asInt ();

				// Sun contribution
				CConfigFile::CVar &sun_contribution = parameter.getVar ("sun_contribution");
				lighterDesc.SunContribution=sun_contribution.asInt ()!=0;

				// Shadows enabled ?
				CConfigFile::CVar &shadow = parameter.getVar ("shadow");
				lighterDesc.Shadow=shadow.asInt ()!=0;

				// Sky contribution
				CConfigFile::CVar &sky_contribution = parameter.getVar ("sky_contribution");
				lighterDesc.SkyContribution=sky_contribution.asInt ()!=0;

				// Sky contribution
				CConfigFile::CVar &sky_intensity = parameter.getVar ("sky_intensity");
				lighterDesc.SkyIntensity=sky_intensity.asFloat ();

				// A vector of CZoneLighter2001::CTriangle
				vector<CZoneLighter2001::CTriangle> vectorTriangle;

				// **********
				// *** Build triangle array
				// **********

				landscape->checkBinds ();

				// Add triangles from landscape
				landscape->enableAutomaticLighting (false);
				lighter.addTriangles (*landscape, listZoneId, 0, vectorTriangle);

				// Load and add shapes

				// Map of shape
				std::map<string, IShape*> shapeMap;

				// For each instance group
				std::list<CInstanceGroup*>::iterator ite=instanceGroup.begin();
				while (ite!=instanceGroup.end())
				{
					// Instance group
					CInstanceGroup *group=*ite;

					// For each instance
					for (uint instance=0; instance<group->getNumInstance(); instance++)
					{
						// Get the instance shape name
						string name=shapes_path+group->getShapeName (instance);

						// Add a .shape at the end ?
						if (name.find('.') == std::string::npos)
							name += ".shape";

						// Find the shape in the bank
						std::map<string, IShape*>::iterator iteMap=shapeMap.find (name);
						if (iteMap==shapeMap.end())
						{
							// Input file
							CIFile inputFile;

							if (inputFile.open (name))
							{
								// Load it
								CShapeStream stream;
								stream.serial (inputFile);

								// Get the pointer
								iteMap=shapeMap.insert (std::map<string, IShape*>::value_type (name, stream.getShapePointer ())).first;
							}
							else
							{
								// Error
								nlwarning ("WARNING can't load shape %s\n", name.c_str());
							}
						}
						
						// Loaded ?
						if (iteMap!=shapeMap.end())
						{
							// Build the matrix
							CMatrix scale;
							scale.identity ();
							scale.scale (group->getInstanceScale (instance));
							CMatrix rot;
							rot.identity ();
							rot.setRot (group->getInstanceRot (instance));
							CMatrix pos;
							pos.identity ();
							pos.setPos (group->getInstancePos (instance));
							CMatrix mt=pos*rot*scale;

							// Add triangles
							lighter.addTriangles (*iteMap->second, mt, vectorTriangle);
						}
					}

					// Next instance group
					ite++;
				}

				// Continue ?
				if (continu)
				{
					// **********
					// *** Light!
					// **********

					// Start time
					TTime time=CTime::getLocalTime ();

					// Output zone
					CZone output;

					// Light the zone
					lighter.light (*landscape, output, zone.getZoneId(), lighterDesc, vectorTriangle, listZoneId);

					// Compute time
					printf ("\rCompute time: %d ms                                                      \r", 
						(uint)(CTime::getLocalTime ()-time));

					// Save the zone
					COFile outputFile;

					// Open it
					if (outputFile.open (argv[2]))
					{
						// Save the new zone
						try
						{
							// Save it
							output.serial (outputFile);
						}
						catch (Exception& except)
						{
							// Error message
							nlwarning ("ERROR writing %s: %s\n", argv[2], except.what());
						}
					}
					else
					{
						// Error can't open the file
						nlwarning ("ERROR Can't open %s for writing\n", argv[1]);
					}
				}
				else
				{
					// Error
					nlwarning ("ERROR Abort: files are missing.\n");
				}
			}
			catch (Exception& except)
			{
				// Error message
				nlwarning ("ERROR %s\n", except.what());
			}
		}
		else
		{
			// Error can't open the file
			nlwarning ("ERROR Can't open %s for reading\n", argv[1]);
		}

	}
	
	// exit.
	return 0;
}
