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

#include "../zone_lib/zone_utility.h"

#include "nel/3d/zone.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/shape.h"
#include "nel/3d/water_shape.h"
#include "nel/3d/register_3d.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/path.h"



#include <stdio.h>
#include <map>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

typedef pair<sint, sint> CZoneDependenciesValue;


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

void progress (const char *message, float progress)
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

class CZoneDependencies
{
public:
	// Default Ctor
	CZoneDependencies ()
	{
		// Not loaded
		Loaded=false;
	}

	// Zone coordinates
	sint		X;
	sint		Y;

	// BBox
	CAABBoxExt	BBox;

	// Loaded
	bool							Loaded;

	// List 
	set<CZoneDependenciesValue>		Dependences;
};

class CZoneDescriptorBB
{
public:
	// Zone coordinates
	sint		X;
	sint		Y;

	// BBox
	CAABBoxExt	BBox;
};



// a bbox with a 'NULL' flad added
class CPotentialBBox 
{
public:
	CPotentialBBox() : IsVoid(true) {}
	CPotentialBBox(const CAABBox &b) : Box(b), IsVoid(false) {}
	CAABBox Box; 
	bool	IsVoid;
	void makeUnion(const CPotentialBBox &other)
	{
		if (IsVoid && other.IsVoid) return;
		if( IsVoid)
		{
			Box = other.Box;
		}
		else if (!other.IsVoid)
		{
			Box = CAABBox::computeAABBoxUnion(Box, other.Box);
		}
		IsVoid = false;
	}
	void transform(const NLMISC::CMatrix &matrix)
	{
		if (!IsVoid)
		{
			Box = NLMISC::CAABBox::transformAABBox(matrix, Box);			
		}
	}
	// If the bbox has a null size, then mark it void
	void removeVoid() 
	{ 
		if (!IsVoid && Box.getHalfSize() == CVector::Null)
		{
			IsVoid = true;
		}
	}
};


// a couple of bbox to identity occluding / receiving bbox
struct CLightingBBox
{
public:
	CPotentialBBox OccludingBox;
	CPotentialBBox ReceivingBox;
	void makeUnion(const CLightingBBox &other)
	{
		OccludingBox.makeUnion(other.OccludingBox);
		ReceivingBox.makeUnion(other.ReceivingBox);
	}
	void transform(const NLMISC::CMatrix &matrix)
	{
		OccludingBox.transform(matrix);
		ReceivingBox.transform(matrix);
	}
	// If the bbox has a null size, then mark it void
	void removeVoid() 
	{ 
		OccludingBox.removeVoid();
		ReceivingBox.removeVoid();
	}
};

typedef std::map<std::string, CLightingBBox> TString2LightingBBox;
/// A map to cache the shapes bbox's
typedef TString2LightingBBox TShapeMap;



// compute the bbox of the igs in a zone
static void computeZoneIGBBox(const char *zoneName, CLightingBBox &result, TShapeMap &shapeMap, const TString2LightingBBox &additionnalIG);
// ryzom specific, see definition
static void computeIGBBoxFromContinent(NLMISC::CConfigFile &parameter,									   
									   TShapeMap &shapeMap,
									   TString2LightingBBox &zone2BBox
							          );







///=========================================================
int main (int argc, char* argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter ("adding the path");

	TShapeMap shapeMap;

	// Check number of args
	if (argc<5)
	{
		// Help message
		printf ("zone_dependencies [properties.cfg] [firstZone.zone] [lastzone.zone] [output_dependencies.cfg]\n");
	}
	else
	{
		NL3D::registerSerial3d();
		// Light direction
		CVector lightDirection;

		// Config file handler
		try
		{
			// Read the properties file
			CConfigFile properties;
			
			// Load and parse the properties file
			properties.load (argv[1]);

			// Get the light direction
			CConfigFile::CVar &sun_direction = properties.getVar ("sun_direction");
			lightDirection.set (sun_direction.asFloat(0), sun_direction.asFloat(1), sun_direction.asFloat(2));
			lightDirection.normalize();


			// Get the search pathes
			CConfigFile::CVar &search_pathes = properties.getVar ("search_pathes");
			uint path;
			for (path = 0; path < (uint)search_pathes.size(); path++)
			{
				// Add to search path
				CPath::addSearchPath (search_pathes.asString(path));
			}
/*
			CConfigFile::CVar &ig_path = properties.getVar ("ig_path");
			NLMISC::CPath::addSearchPath(ig_path.asString(), true, true);

			CConfigFile::CVar &shapes_path = properties.getVar ("shapes_path");
			NLMISC::CPath::addSearchPath(shapes_path.asString(), true, true);
*/
			CConfigFile::CVar &compute_dependencies_with_igs = properties.getVar ("compute_dependencies_with_igs");
			bool computeDependenciesWithIgs = compute_dependencies_with_igs.asInt() != 0;								


			// Get the file extension
			string ext=getExt (argv[2]);

			// Get the file directory
			string dir=getDir (argv[2]);

			// Get output extension
			string outExt=getExt (argv[4]);

			// Get output directory
			string outDir=getDir (argv[4]);

			// Get the first and last name
			string firstName=getName (argv[2]);
			string lastName=getName (argv[3]);

			// Get the coordinates
			uint16 firstX, firstY;
			uint16 lastX, lastY;
			if (getZoneCoordByName (firstName.c_str(), firstX, firstY))
			{
				// Last zone
				if (getZoneCoordByName (lastName.c_str(), lastX, lastY))
				{
					// Take care
					if (lastX<firstX)
					{
						uint16 tmp=firstX;
						firstX=lastX;
						lastX=firstX;
					}
					if (lastY<firstY)
					{
						uint16 tmp=firstY;
						firstY=lastY;
						lastY=firstY;
					}

					// Min z
					float minZ=FLT_MAX;

					// Make a quad grid
					CQuadGrid<CZoneDescriptorBB>	quadGrid;
					quadGrid.create (256, 100);

					// The dependencies list
					vector< CZoneDependencies > dependencies;
					dependencies.resize ((lastX-firstX+1)*(lastY-firstY+1));

					// Ryzom specific: build bbox for villages
					TString2LightingBBox villagesBBox;					
					computeIGBBoxFromContinent(properties, shapeMap, villagesBBox);		


					// Insert each zone in the quad tree
					sint y, x;
					for (y=firstY; y<=lastY; y++)
					for (x=firstX; x<=lastX; x++)
					{
						

						// Progress
						progress ("Build bounding boxes", (float)(x+y*lastX)/(float)(lastX*lastY));

						// Make a zone file name
						string zoneName;
						getZoneNameByCoord (x, y, zoneName);

						// Open the file
						CIFile file;
						if (file.open (dir+zoneName+ext))
						{							
							// The zone
							CZone zone;

							try
							{
								// Serial the zone
								file.serial (zone);

								/// get bbox from the ig of this zone
								CLightingBBox igBBox;								
								if (computeDependenciesWithIgs)
								{
									computeZoneIGBBox(zoneName.c_str(), igBBox, shapeMap, villagesBBox);
								}								
								// Create a zone descriptor
								

								
								NLMISC::CAABBox zoneBox;
								zoneBox.setCenter(zone.getZoneBB().getCenter());
								zoneBox.setHalfSize(zone.getZoneBB().getHalfSize());

								CLightingBBox zoneLBox;
								zoneLBox.OccludingBox = zoneLBox.ReceivingBox = zoneBox; // can't be void
								zoneLBox.makeUnion(igBBox);								
								nlassert(!zoneLBox.ReceivingBox.IsVoid);
								//
								CZoneDescriptorBB zoneDesc;
								zoneDesc.X=x;
								zoneDesc.Y=y;
								zoneDesc.BBox=zoneLBox.ReceivingBox.Box;
								//
								if (!zoneLBox.OccludingBox.IsVoid)
								{
									quadGrid.insert (zoneLBox.ReceivingBox.Box.getMin(), zoneLBox.ReceivingBox.Box.getMax(), zoneDesc);
								}
																								
								// Insert in the dependencies
								// Index 
								uint index=(x-firstX)+(y-firstY)*(lastX-firstX+1);
								

								// Loaded
								dependencies[index].Loaded=true;
								dependencies[index].X=x;
								dependencies[index].Y=y;
								dependencies[index].BBox=zoneLBox.OccludingBox.Box;

								// ZMin
								float newZ=zoneLBox.ReceivingBox.Box.getMin().z;
								if (newZ<minZ)
									minZ=newZ;
							}
							catch (const Exception& e)
							{
								// Error handling
								nlwarning ("ERROR in file %s, %s", (dir+zoneName+ext).c_str(), e.what ());
							}
						}
					}

					// Now select each zone in others and make a depencies list
					for (y=firstY; y<=lastY; y++)
					for (x=firstX; x<=lastX; x++)
					{
						// Progress
						progress ("Compute dependencies", (float)(x+y*lastX)/(float)(lastX*lastY));

						// Index 
						uint index=(x-firstX)+(y-firstY)*(lastX-firstX+1);

						// Loaded ?
						if (dependencies[index].Loaded)
						{
							// Min max vectors
							CVector vMin (dependencies[index].BBox.getMin());
							CVector vMax (dependencies[index].BBox.getMax());

							// Make a corner array 
							CVector corners[4] = 
							{
								CVector (vMin.x, vMin.y, vMax.z), CVector (vMax.x, vMin.y, vMax.z), 
								CVector (vMax.x, vMax.y, vMax.z), CVector (vMin.x, vMax.y, vMax.z)
							};

							// Extended bbox
							CAABBox	bBox=dependencies[index].BBox.getAABBox();

							// For each corner
							uint corner;
							for (corner=0; corner<4; corner++)
							{
								// Target position
								CVector target;
								if (lightDirection.z!=0)
								{
									// Not horizontal target
									target=corners[corner]+(lightDirection*((minZ-corners[corner].z)/lightDirection.z));
								}
								else
								{
									// Horizontal target, select 500 meters around.
									target=(500*lightDirection)+corners[corner];
								}

								// Extend the bbox
								bBox.extend (target);
							}

							// Clear quad tree selection
							quadGrid.clearSelection ();

							// Select
							quadGrid.select (bBox.getMin(), bBox.getMax());

							// Check selection
							CQuadGrid<CZoneDescriptorBB>::CIterator it=quadGrid.begin();
							while (it!=quadGrid.end())
							{
								// Index 
								uint targetIndex=((*it).X-firstX)+((*it).Y-firstY)*(lastX-firstX+1);

								// Not the same
								if (targetIndex!=index)
								{
									// Target min z
									float targetMinZ=dependencies[targetIndex].BBox.getMin().z;
									if (targetMinZ<vMax.z)
									{
										// Min z inf to max z ?
										// Target optimized bbox
										CAABBox	bBoxOptimized=dependencies[index].BBox.getAABBox();

										// For each corner
										for (corner=0; corner<4; corner++)
										{
											// Target position
											CVector target;
											if (lightDirection.z!=0)
											{
												// Not horizontal target
												target=corners[corner]+(lightDirection*((targetMinZ-corners[corner].z)
													/lightDirection.z));
											}
											else
											{
												// Horizontal target, select 500 meters around.
												target=(500*lightDirection)+corners[corner];
											}

											// Extend the bbox
											bBoxOptimized.extend (target);
										}

										// Check it more presisly
										//if ((*it).BBox.intersect (bBoxOptimized))
										if ((*it).BBox.intersect (bBox))
										{
											// Insert in the set
											dependencies[targetIndex].Dependences.insert (CZoneDependenciesValue (x, y));
										}
									}
								}

								// Next selected
								it++;
							}
						}
					}

					// For each zone
					for (y=firstY; y<=lastY; y++)
					for (x=firstX; x<=lastX; x++)
					{
						// Progress
						progress ("Save depend files", (float)(x+y*lastX)/(float)(lastX*lastY));

						// Index 
						uint index=(x-firstX)+(y-firstY)*(lastX-firstX+1);

						// Loaded ?
						if (dependencies[index].Loaded)
						{
							// Make a file name
							string outputFileName;
							getZoneNameByCoord(x, y, outputFileName);
							outputFileName=outDir+outputFileName+outExt;

							// Write the dependencies file
							FILE *outputFile;
							if ((outputFile=fopen (toLower (outputFileName).c_str(), "w")))
							{
								// Add a dependency entry
								fprintf (outputFile, "dependencies =\n{\n");

								// Add dependent zones
								set<CZoneDependenciesValue>::iterator ite=dependencies[index].Dependences.begin();
								while (ite!=dependencies[index].Dependences.end())
								{
									// Name of the dependent zone
									std::string zoneName;
									getZoneNameByCoord(ite->first, ite->second, zoneName);

									// Write it
									string message="\t\""+zoneName+"\"";
									fprintf (outputFile, "%s", toLower (message).c_str());

									// Next ite;
									ite++;
									if (ite!=dependencies[index].Dependences.end())
										fprintf (outputFile, ",\n");
								}

								// Close the variable
								fprintf (outputFile, "\n};\n\n");
							}
							else
							{
								nlwarning ("ERROR can't open %s for writing.\n", outputFileName.c_str());
							}

							// Close the file
							fclose (outputFile);
						}
					}

				}
				else
				{
					// Not valid
					nlwarning ("ERROR %s is not a valid zone name.\n", lastName.c_str());
				}
			}
			else
			{
				// Not valid
				nlwarning ("ERROR %s is not a valid zone name.\n", firstName.c_str());
			}
		}
		catch (const Exception &ee)
		{
			nlwarning ("ERROR %s\n", ee.what());
		}
	}

	return 0;
}


///===========================================================================
/** Load and compute the bbox of the models that are contained in a given instance group
  * \return true if the computed bbox is valid
  */
static void computeIGBBox(const NL3D::CInstanceGroup &ig, CLightingBBox &result, TShapeMap &shapeMap)
{
	result = CLightingBBox(); // starts with void result
	bool firstBBox = true;	
	/// now, compute the union of all bboxs
	for (CInstanceGroup::TInstanceArray::const_iterator it = ig._InstancesInfos.begin(); it != ig._InstancesInfos.end(); ++it)
	{		
		CLightingBBox currBBox;
		
		bool validBBox = false;
		/// get the bbox from file or from map
		if (shapeMap.count(it->Name)) // already loaded ?
		{
			currBBox = shapeMap[it->Name];
			validBBox = true;
		}
		else // must load the shape to get its bbox
		{		
			std::string shapePathName;
			std::string toLoad = it->Name;
			if (getExt(toLoad).empty()) toLoad += ".shape";
			shapePathName = NLMISC::CPath::lookup(toLoad, false, false);

			if (shapePathName.empty())
			{
				nlwarning("Unable to find shape '%s'", it->Name.c_str());				
			}
			else
			{
				CIFile shapeInputFile;
				
				if (shapeInputFile.open (shapePathName.c_str()))
				{					
					NL3D::CShapeStream shapeStream;
					try
					{
						shapeStream.serial (shapeInputFile);
						// NB Nico :
						// Deal with water shape -> their 'Receiving' box is set to 'void'
						// this prevent the case where a huge surface of water will cause the zone it is attached to (the 'Zone'
						// field in the villages sheets) to load all the zones that the water surface cover. (This caused
						// an 'out of memory error' in the zone lighter due to too many zone being loaded)
						
						// FIXME : test for water case hardcoded for now						
						CWaterShape *ws = dynamic_cast<CWaterShape *>(shapeStream.getShapePointer());
						if (ws)
						{
							CAABBox bbox;
							shapeStream.getShapePointer()->getAABBox(bbox);
							currBBox.OccludingBox = CPotentialBBox(bbox); // occluding box is used, though the water shape
																		 // doesn't cast shadow -> the tiles flag ('above', 'intersect', 'below water')
																		 // are updated inside the zone_lighter
							currBBox.ReceivingBox.IsVoid = true; // no lighted by the zone lighter !!!
							currBBox.removeVoid();
							shapeMap[it->Name] = currBBox;							
						}
						else
						{

							CAABBox bbox;
							shapeStream.getShapePointer()->getAABBox(bbox);
							currBBox.OccludingBox = CPotentialBBox(bbox);
							currBBox.ReceivingBox = CPotentialBBox(bbox);
							currBBox.removeVoid();
							shapeMap[it->Name] = currBBox;
						}
						validBBox = true;
					}
					
					catch (const NLMISC::Exception &e)
					{
						nlwarning("Error while loading shape %s. \n\t Reason : %s ", it->Name.c_str(), e.what());
					}				
				}
				else
				{
					nlwarning("Unable to open shape file %s to get its bbox", it->Name.c_str());
				}
			}
		}


		if (validBBox)
		{
			/// build the model matrix
			NLMISC::CMatrix mat;
			mat.scale(it->Scale);
			NLMISC::CMatrix rotMat;
			rotMat.setRot(it->Rot);
			mat = rotMat * mat;
			mat.setPos(it->Pos);

			/// transform the bbox
			currBBox.transform(mat);
			currBBox.removeVoid();			
			if (firstBBox)
			{
				result = currBBox;
				firstBBox = false;
			}
			else // add to previous one
			{						
				result.makeUnion(currBBox);
			}			
		}		
	}	
}

///===========================================================================
/** Load and compute the bbox of the models that are located in a given zone
  * \param the zone whose bbox must be computed
  * \param result the result bbox
  * \param shapeMap for speedup (avoid loading the same shape twice)
  * \param additionnalIG a map that gives an additionnal ig for a zone from its name (ryzom specific : used to compute village bbox)
  * \return true if the computed bbox is valid
  */
static void computeZoneIGBBox(const char *zoneName, CLightingBBox &result, TShapeMap &shapeMap, const TString2LightingBBox &additionnalIG)
{
	result = CLightingBBox(); // starts with a void box	
	std::string lcZoneName = NLMISC::toLower(std::string(zoneName));
	TString2LightingBBox::const_iterator zoneIt = additionnalIG.find(lcZoneName);
	if (zoneIt != additionnalIG.end())
	{		
		result = zoneIt->second;		
	}

	std::string igFileName = zoneName + std::string(".ig");
	std::string pathName = CPath::lookup(igFileName, false, false);

	if (pathName.empty())
	{
		// nlwarning("unable to find instance group of zone : %s", zoneName);
		return;
	}


	/// Load the instance group of this zone
	CIFile igFile;
	if (!igFile.open(pathName))
	{
		nlwarning("unable to open file : %s", pathName.c_str());
		return;
	}

	NL3D::CInstanceGroup ig;
	try
	{		
		ig.serial(igFile);
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning("Error while reading an instance group file : %s \n reason : %s", pathName.c_str(), e.what());
		return;
	}
	CLightingBBox tmpBBox;
	computeIGBBox(ig, tmpBBox, shapeMap);		
	result.makeUnion(tmpBBox);	
}



//=======================================================================================
// ryzom specific build bbox of a village in a zone
static void computeBBoxFromVillage(const NLGEORGES::UFormElm *villageItem, 
								   const std::string &continentName,
								   uint villageIndex,
								   TShapeMap &shapeMap,
								   CLightingBBox &result
								  )
{	
	result = CLightingBBox();
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
				CInstanceGroup group;
				try
				{
					CLightingBBox currBBox;
					group.serial (inputFile);
					computeIGBBox(group, currBBox, shapeMap);											
					result.makeUnion(currBBox);					
				}
				catch(const NLMISC::Exception &)
				{
					nlwarning ("Error while loading instance group %s\n", igName.c_str());	
					continue;
				}								
				inputFile.close();				
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
/** Load additionnal ig from a continent (ryzom specific)
  * \param parameter a config file that contains the name of the continent containing the zones we are processing
  * \param zone2bbox This will be filled with the name of a zone and the bbox of the village it contains
  * \param a map of shape
  * \param a vector that will be filled with a zone name and the bbox of the village it contains
  */
static void computeIGBBoxFromContinent(NLMISC::CConfigFile &parameter,									   
									   TShapeMap &shapeMap,
									   TString2LightingBBox &zone2BBox									   
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
				zoneName = NLMISC::toLower(CFile::getFilenameWithoutExtension(zoneName));				
				CLightingBBox result;				
				// ok, it is in the dependant zones
				computeBBoxFromVillage(currVillage, continentName, k, shapeMap, result);
				if (!result.OccludingBox.IsVoid || result.ReceivingBox.IsVoid)
				{
					zone2BBox[zoneName] = result;					
				}										
			}				
		}
		else 
		{
			nlwarning("Can't load continent form : %s", continentName.c_str());
		}				
	}	
	catch (const NLMISC::EUnknownVar &e)
	{
		nlinfo(e.what());
	}
}
