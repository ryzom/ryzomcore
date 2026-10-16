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
#include "nel/misc/path.h"


#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"


#include "nel/3d/zone.h"
#include "zone_lighter_2002.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/landscape.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/shape.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/water_shape.h"

#include "../zone_lib/zone_utility.h"

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
class CMyZoneLighter : public CZoneLighter2002
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
		for (i=strlen(msg); i<79; i++)
			msg[i]=' ';
		msg[i]=0;
		printf ("%s", msg);
		printf ("\r");
	}
};


//=======================================================================================
// load additionnal ig from a village (ryzom specific)
static void loadIGFromVillage(const NLGEORGES::UFormElm *villageItem, const std::string &continentName,
							  uint villageIndex, std::list<CInstanceGroup *> &instanceGroups)
{	
	const NLGEORGES::UFormElm *igNamesItem;
	if (! (villageItem->getNodeByName (&igNamesItem, "IgList") && igNamesItem) )
	{
		nlwarning("No list of IGs was found in the continent form %s, village #%d", continentName.c_str(), villageIndex);
		return;
	}

	// Get number of village
	uint numIgs;
	nlverify (igNamesItem->getArraySize (numIgs));
	const NLGEORGES::UFormElm *currIg;
	for(uint l = 0; l < numIgs; ++l)
	{														
		if (!(igNamesItem->getArrayNode (&currIg, l) && currIg))
		{
			nlwarning("Couldn't get ig #%d in the continent form %s, in village #%d", l, continentName.c_str(), villageIndex);
			continue;
		}			
		const NLGEORGES::UFormElm *igNameItem;
		currIg->getNodeByName (&igNameItem, "IgName");
		std::string igName;
		if (!igNameItem->getValue (igName))
		{
			nlwarning("Couldn't get ig name of ig #%d in the continent form %s, in village #%d", l, continentName.c_str(), villageIndex);
			continue;
		}
		if (igName.empty())
		{
			nlwarning("Ig name of ig #%d in the continent form %s, in village #%d is an empty string", l, continentName.c_str(), villageIndex);
			continue;
		}

		igName = CFile::getFilenameWithoutExtension(igName) + ".ig";
		string nameLookup = CPath::lookup (igName, false, true);
		if (!nameLookup.empty())
		{		
			CIFile inputFile;
			// Try to open the file
			if (inputFile.open (nameLookup))
			{
				// New ig
				std::auto_ptr<CInstanceGroup> group(new CInstanceGroup);
				try
				{
					group->serial (inputFile);
				}
				catch(NLMISC::Exception &)
				{
					nlwarning ("Error while loading instance group %s\n", igName.c_str());	
					continue;
				}								
				inputFile.close();
				// Add to the list
				instanceGroups.push_back (group.release());
			}
			else
			{
				// Error
				nlwarning ("Can't open instance group %s\n", igName.c_str());
			}
		}								
	}	
}


//=======================================================================================
// load additionnal ig from a continent (ryzom specific)
static void loadIGFromContinent(NLMISC::CConfigFile &parameter, std::list<CInstanceGroup *> &instanceGroups,
								const std::vector<std::string> &zoneNameArray
							   )
{
		
	try
	{
		CConfigFile::CVar &continent_name_var = parameter.getVar ("continent_name");
		CConfigFile::CVar &level_design_directory = parameter.getVar ("level_design_directory");
		CConfigFile::CVar &level_design_world_directory = parameter.getVar ("level_design_world_directory");						
		CConfigFile::CVar &level_design_dfn_directory = parameter.getVar ("level_design_dfn_directory");
		CPath::addSearchPath(level_design_dfn_directory.asString(), true, false);
		CPath::addSearchPath(level_design_world_directory.asString(), true, false);

		std::string continentName = continent_name_var.asString();
		if (CFile::getExtension(continentName).empty())
			continentName += ".continent";
		// Load the form
		NLGEORGES::UFormLoader *loader = NLGEORGES::UFormLoader::createLoader();
		//
		std::string pathName = level_design_world_directory.asString() + "/" + continentName;
		if (pathName.empty())
		{		
			nlwarning("Can't find continent form : %s", continentName.c_str());
			return;
		}		
		NLGEORGES::UForm *villageForm;
		villageForm = loader->loadForm(pathName.c_str());
		if(villageForm != NULL)
		{
			NLGEORGES::UFormElm &rootItem = villageForm->getRootNode();
			// try to get the village list
			// Load the village list
			NLGEORGES::UFormElm *villagesItem;
			if(!(rootItem.getNodeByName (&villagesItem, "Villages") && villagesItem))
			{
				nlwarning("No villages where found in %s", continentName.c_str());
				return;
			}

			// Get number of village
			uint numVillage;
			nlverify (villagesItem->getArraySize (numVillage));

			// For each village
			for(uint k = 0; k < numVillage; ++k)
			{				
				NLGEORGES::UFormElm *currVillage;
				if (!(villagesItem->getArrayNode (&currVillage, k) && currVillage))
				{
					nlwarning("Couldn't get village %d in continent %s", continentName.c_str(), k);
					continue;
				}
				// check that this village is in the dependency zones
				NLGEORGES::UFormElm *zoneNameItem;
				if (!currVillage->getNodeByName (&zoneNameItem, "Zone") && zoneNameItem)
				{
					nlwarning("Couldn't get zone item of village %d in continent %s", continentName.c_str(), k);
					continue;
				}
				std::string zoneName;
				if (!zoneNameItem->getValue(zoneName))
				{
					nlwarning("Couldn't get zone name of village %d in continent %s", continentName.c_str(), k);
					continue;
				}

				zoneName = CFile::getFilenameWithoutExtension(zoneName);
				for(uint l = 0; l < zoneNameArray.size(); ++l)
				{					
					if (NLMISC::nlstricmp(CFile::getFilenameWithoutExtension(zoneNameArray[l]), zoneName) == 0)																  
					{										
						// ok, it is in the dependant zones
						loadIGFromVillage(currVillage, continentName, k, instanceGroups);
						break;
					}
				}					
			}				
		}
		else 
		{
			nlwarning("Can't load continent form : %s", continentName.c_str());
		}				
	}	
	catch (NLMISC::EUnknownVar &e)
	{
		nlinfo(e.what());
	}
}

//=======================================================================================
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
		printf ("%s [zonein.zone] [zoneout.zone] [parameter_file] [dependancy_file]\n", argv[0]);
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

				// **********
				// *** Build the lighter descriptor
				// **********

				CZoneLighter2002::CLightDesc lighterDesc;

				// Get bank name
				CConfigFile::CVar &bank_name = parameter.getVar ("bank_name");

				// Load instance group ?
				CConfigFile::CVar &load_ig= parameter.getVar ("load_ig");
				bool loadInstanceGroup = load_ig.asInt ()!=0;

				CConfigFile::CVar &additionnal_ig = parameter.getVar ("additionnal_ig");

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

				// Water rendering parameters
				CConfigFile::CVar &water_zbias = parameter.getVar ("water_shadow_bias");
				lighterDesc.WaterShadowBias = water_zbias.asFloat();

				CConfigFile::CVar &water_ambient = parameter.getVar ("water_ambient");
				lighterDesc.WaterAmbient = water_ambient.asFloat();

				CConfigFile::CVar &water_diffuse = parameter.getVar ("water_diffuse");
				lighterDesc.WaterDiffuse = water_diffuse.asFloat();
				
				CConfigFile::CVar &modulate_water_color = parameter.getVar ("modulate_water_color");
				lighterDesc.ModulateWaterColor = modulate_water_color.asInt() != 0;

				CConfigFile::CVar &sky_contribution_for_water = parameter.getVar ("sky_contribution_for_water");
				lighterDesc.SkyContributionForWater = sky_contribution_for_water.asInt() != 0;

				// Oversampling
				CConfigFile::CVar &oversampling = parameter.getVar ("oversampling");
				sint oversmaplingValue=oversampling.asInt();
				switch (oversmaplingValue)
				{
				case 0:
					lighterDesc.Oversampling=CZoneLighter2002::CLightDesc::NoOverSampling;
					break;
				case 2:
					lighterDesc.Oversampling=CZoneLighter2002::CLightDesc::OverSamplingx2;
					break;
				case 8:
					lighterDesc.Oversampling=CZoneLighter2002::CLightDesc::OverSamplingx8;
					break;
				case 32:
					lighterDesc.Oversampling=CZoneLighter2002::CLightDesc::OverSamplingx32;
					break;
				case 128:
					lighterDesc.Oversampling=CZoneLighter2002::CLightDesc::OverSamplingx128;
					break;
				default:
					// Error message
					nlwarning ("ERROR oversampling value not supported. Must be 0, 2, 8, 32 or 128. Forced to 0.\n");
					lighterDesc.Oversampling=CZoneLighter2002::CLightDesc::NoOverSampling;
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

				// Vegetable Height
				CConfigFile::CVar &vegetable_height = parameter.getVar ("vegetable_height");
				lighterDesc.VegetableHeight=vegetable_height.asFloat ();

				
				// Shadow are enabled ?
				if (lighterDesc.Shadow)
				{
					// Load and parse the dependency file
					dependency.load (argv[4]);
				}

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

				bool zoneIgLoaded = false;

				// Load ig of the zone
				string igName = getName (argv[1])+".ig";
				string igNameLookup = CPath::lookup (igName, false, false);
				if (!igNameLookup.empty())
					igName = igNameLookup;

				if (inputFile.open (igName))
				{
					// New ig
					CInstanceGroup *group=new CInstanceGroup;

					// Serial it
					group->serial (inputFile);
					inputFile.close();

					// Add to the list
					instanceGroup.push_back (group);
					zoneIgLoaded = true;
				}
				else
				{
					// Warning
					fprintf (stderr, "Warning: can't load instance group %s\n", igName.c_str());
					zoneIgLoaded = false;
				}

				// Load the bank
				string bankName = bank_name.asString();
				string bankNameLookup = CPath::lookup (bankName, false, false);
				if (!bankNameLookup.empty())
					bankName = bankNameLookup;
				if (inputFile.open (bankName))
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
						nlwarning ("ERROR error loading tile bank %s\n%s\n", bankName.c_str(), e.what());
					}
				}
				else
				{
					// Error
					nlwarning ("ERROR can't load tile bank %s\n", bankName.c_str());
				}

				// Add the zone
				landscape->addZone (zone);
				listZoneId.push_back (zone.getZoneId());

				// Continue to build ?
				bool continu=true;

				// Try to load additionnal instance group.

				// Additionnal instance group
				if (loadInstanceGroup)
				{
					try
					{
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
					catch (NLMISC::EUnknownVar &)
					{
						nlinfo("No additionnal ig's to load");
					}
				}				
				
				// *** Scan dependency file

				if (lighterDesc.Shadow)
				{
					CConfigFile::CVar &dependant_zones = dependency.getVar ("dependencies");
					std::vector<std::string> zoneNameArray;
					zoneNameArray.reserve(1 + (uint)dependant_zones.size());
					zoneNameArray.push_back(argv[1]);

					for (uint i=0; i<(uint)dependant_zones.size(); i++)
					{
						// Get zone name
						string zoneName=dependant_zones.asString(i);
						zoneNameArray.push_back(zoneName);
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

					if (loadInstanceGroup)
					{
						// Ryzom specific : additionnal villages from a continent form
						loadIGFromContinent(parameter, instanceGroup, zoneNameArray);
					}
				}

				// A vector of CZoneLighter2002::CTriangle
				vector<CZoneLighter2002::CTriangle> vectorTriangle;

				// **********
				// *** Build triangle array
				// **********

				landscape->checkBinds ();

				// Add triangles from landscape
				landscape->enableAutomaticLighting (false);
				lighter.addTriangles (*landscape, listZoneId, 0, vectorTriangle);

				// Map of shape
				std::map<string, IShape*> shapeMap;

				// For each instance group
				std::list<CInstanceGroup*>::iterator ite=instanceGroup.begin();
				while (ite!=instanceGroup.end())
				{
					// Instance group
					CInstanceGroup *group=*ite;

					// Load and add shapes
					if (lighterDesc.Shadow)
					{
						// For each instance
						for (uint instance=0; instance<group->getNumInstance(); instance++)
						{
							// Get the instance shape name
							string name=group->getShapeName (instance);

							// Skip it??
							if(group->getInstance(instance).DontCastShadow)
								continue;

							// Add a .shape at the end ?
							if (name.find('.') == std::string::npos)
								name += ".shape";

							// Add path
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

								// Add triangles
								lighter.addTriangles (*iteMap->second, mt, vectorTriangle);

								/** If it is a lightable shape and we are dealing with the ig of the main zone,
								  * add it to the lightable shape list
								  */
								IShape *shape = iteMap->second;
								if (ite == instanceGroup.begin()  /* are we dealing with main zone */ 
									&& zoneIgLoaded               /* ig of the main zone loaded successfully (so its indeed the ig of the first zone) ? */								
									&& CZoneLighter2002::isLightableShape(*shape)
								   )
								{
									lighter.addLightableShape(shape, mt);
								}

								/** If it is a water shape, add it to the lighter, so that it can check
								  * which tiles are above / below water for this zone. The result is saved in the flags of tiles.
								  * A tile that have their flags set to VegetableDisabled won't get setupped
								  */
								if (dynamic_cast<NL3D::CWaterShape *>(shape))
								{
									lighter.addWaterShape(static_cast<NL3D::CWaterShape *>(shape), mt);
								}
							}
						}
					}

					// For each point light of the ig
					const std::vector<CPointLightNamed>	&pointLightList= group->getPointLightList();
					for (uint plId=0; plId<pointLightList.size(); plId++)
					{
						// Add it to the Ig.
						lighter.addStaticPointLight(pointLightList[plId]);
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
	

	// Landscape is not deleted, nor the instanceGroups, for faster quit.
	// Must disalbe BlockMemory checks (for pointLights).
	NL3D_BlockMemoryAssertOnPurge= false;

	// exit.
	return 0;
}
