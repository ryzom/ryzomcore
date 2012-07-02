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

#include "nel/3d/zone.h"
#include "nel/3d/instance_lighter.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/landscape.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/shape.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/register_3d.h"

#include "../zone_lib/zone_utility.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

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

// My zone lighter
class CMyIgZoneLighter : public CInstanceLighter
{
	// Progress bar
	virtual void progress (const char *message, float progress)
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
};




int main(int argc, char* argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter ("adding the path");

	// Register 3d
	registerSerial3d ();

	// Good number of args ?
	if (argc<5)
	{
		// Help message
		printf ("%s [zonein.zonel] [igout.ig] [parameter_file] [dependancy_file]\n", argv[0]);
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
			string zoneName=toLower (string ("zone_"+getName (argv[1])));

			// Load the zone
			try
			{
				// Read the config file
				CConfigFile parameter;

				// Load and parse the parameter file
				parameter.load (argv[3]);

				// **********
				// *** Build the lighter descriptor
				// **********

				CInstanceLighter::CLightDesc lighterDesc;

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

				// For ig of Zones, never disable Sun contrib !!!
				lighterDesc.DisableSunContribution= false;

				// Get the search pathes
				CConfigFile::CVar &search_pathes = parameter.getVar ("search_pathes");
				uint path;
				for (path = 0; path < (uint)search_pathes.size(); path++)
				{
					// Add to search path
					CPath::addSearchPath (search_pathes.asString(path));
				}

				// A landscape allocated with new: it is not delete because destruction take 3 secondes more!
				CLandscape *landscape=new CLandscape;
				landscape->init();

				// A zone lighter
				CMyIgZoneLighter lighter;
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
				string igName = getName (argv[1])+".ig";
				string igNameLookup = CPath::lookup (igName, false, false);
				if (!igNameLookup.empty())
					igName = igNameLookup;

				bool zoneIgLoaded;

				// Try to open the file
				CInstanceGroup *centerInstanceGroup= NULL;
				if (inputFile.open (igName))
				{
					// load the center ig
					centerInstanceGroup=new CInstanceGroup;

					// Serial it
					centerInstanceGroup->serial (inputFile);
					inputFile.close();

					// Add to the list
					instanceGroup.push_back (centerInstanceGroup);
					zoneIgLoaded = true;
				}
				else
				{
					// Warning
					fprintf (stderr, "Warning: can't load instance group %s\n", igName.c_str());
					zoneIgLoaded = false;
				}

				// If can't load the center instanceGroup, skip it.
				if(!zoneIgLoaded)
					return 0;

				// Get bank path
				CConfigFile::CVar &bank_name_var = parameter.getVar ("bank_name");
				string bank_name = bank_name_var.asString ();
				string bank_name_lookup = CPath::lookup (bank_name);
				if (!bank_name_lookup.empty())
					bank_name = bank_name_lookup;

				// Load the bank
				if (inputFile.open (bank_name))
				{
					try
					{
						// Load
						landscape->TileBank.serial (inputFile);
						landscape->initTileBanks();
					}
					catch (const Exception &e)
					{
						// Error
						nlwarning ("ERROR error loading tile bank %s\n%s\n", bank_name.c_str(), e.what());
					}
				}
				else
				{
					// Error
					nlwarning ("ERROR can't load tile bank %s\n", bank_name.c_str());
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
					try
					{
						CConfigFile::CVar &additionnal_ig = parameter.getVar ("additionnal_ig");									
						for (uint add=0; add<(uint)additionnal_ig.size(); add++)
						{
							// Input file
							CIFile inputFile;

							// Name of the instance group
							string name = additionnal_ig.asString(add);
							string nameLookup = CPath::lookup (name, false, false);
							if (!nameLookup.empty())
								name = nameLookup;

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
					catch (const NLMISC::EUnknownVar &)
					{
						nlinfo("No additionnal ig's to load");
					}
				}
				
				// Shadow ?
				if (lighterDesc.Shadow)
				{
					// Load and parse the dependency file
					CConfigFile dependency;
					dependency.load (argv[4]);

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
							string name = zoneName+".ig";
							string nameLookup = CPath::lookup (name, false, false);
							if (!nameLookup.empty())
								name = nameLookup;

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
				}

				// A vector of CInstanceLighter::CTriangle
				vector<CInstanceLighter::CTriangle> vectorTriangle;

				// **********
				// *** Build triangle array
				// **********

				landscape->checkBinds ();

				// Add triangles from landscape, for pointLight lighting.
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
						string name=group->getShapeName (instance);

						// Skip it?? use the DontCastShadowForExterior flag. See doc of this flag
						if(group->getInstance(instance).DontCastShadow || group->getInstance(instance).DontCastShadowForExterior)
							continue;

						// Add a .shape at the end ?
						if (!name.empty())
						{
							if (name.find('.') == std::string::npos)
								name += ".shape";

							// Find the file
							string nameLookup = CPath::lookup (name, false, false);
							if (!nameLookup.empty())
								name = nameLookup;

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

								// If centerInstanceGroup, take good instanceId, to avoid selfShadowing
								sint	instanceId;
								if(group == centerInstanceGroup)
									instanceId= instance;
								else
									instanceId= -1;

								// Add triangles
								lighter.addTriangles (*iteMap->second, mt, vectorTriangle, instanceId);
							}
						}
					}

					// For each point light of the ig
					const std::vector<CPointLightNamed>	&pointLightList= group->getPointLightList();
					for (uint plId=0; plId<pointLightList.size(); plId++)
					{
						// Add it to the Ig.
						lighter.addStaticPointLight(pointLightList[plId], igName.c_str ());
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

					// Output ig
					CInstanceGroup	output;

					// Light the zone
					lighter.light (*centerInstanceGroup, output, lighterDesc, vectorTriangle, landscape);

					// Compute time
					printf ("\rCompute time: %d ms                                                      \r", 
						(uint)(CTime::getLocalTime ()-time));

					// Save the zone
					COFile outputFile;

					// Open it
					if (outputFile.open (argv[2]))
					{
						try
						{
							// Save the new ig
							outputFile.serial(output);
						}
						catch (const Exception& except)
						{
							// Error message
							nlwarning ("ERROR writing %s: %s\n", argv[2], except.what());
						}
					}
					else
					{
						// Error can't open the file
						nlwarning ("ERROR Can't open %s for writing\n", argv[2]);
					}
				}
				else
				{
					// Error
					nlwarning ("ERROR Abort: files are missing.\n");
				}
			}
			catch (const Exception& except)
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
	

	// Landscape is not deleted, nor the instanceGroups, for faster quit.
	// Must disalbe BlockMemory checks (for pointLights).
	NL3D_BlockMemoryAssertOnPurge= false;

	// exit.
	return 0;
}
