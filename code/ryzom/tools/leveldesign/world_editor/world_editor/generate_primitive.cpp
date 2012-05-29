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

#include "stdafx.h"

#include "editor_primitive.h"
#include "generate_primitive.h"
#include "world_editor.h"
#include "world_editor_doc.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;
using namespace NLGEORGES;

// ***************************************************************************

/*

Implementation notes:
---------------------

- Load all the "landscape zones" overlapped by the source zone
- Build a quad tree with all primitives to check overlapping
- List all the plants used the source zone
- Sort them by size (Place first the biggest one)
- For each plants in the sorted list
	- Compute a grid position depending the plant size and the plant density
	- Jitter it with a random function
	- Test if the final position is valid
		- Check the primitive doesn't overlappe another primitive (using the quad tree)
		- Check the final position is posed over the landscape, not in a hole
		- Make some Z samples (Z_SAMPLE_COUNT, hard coded) around the position with the collision radius 
			- For each check if the delta z is behind a fixed (Z_THRESHOLD, hard coded) threshold
*/

// *** Features

// Enable / disable z position test
#define TEST_Z_POSITION

// *** todo : pass this as generation parameters

// Test N time max to place a point
#define GENERATE_PRIMITIVE_TEST_COUNT 4

// Small epsilon for triangle selection
#define SELECTION_EPSILON (0.1f)

// Z sample to do around the position to check Z
#define Z_SAMPLE_COUNT 8

// Z threshold used to validate each sample
#define Z_THRESHOLD (0.8f)

// ***************************************************************************

CGeneratePrimitive::CGeneratePrimitive ()
{
	_Landscape = NULL;
	/* todo COL_USING_VISUAL_COLLISION_MANAGER
	_VCM = NULL;
	_VCE = NULL; */
}

// ***************************************************************************

// Flora plant class, used by the flora class to reference the plants
class CFloraPlant
{
public:
	string	Name;
	float	Density;
	float	Falloff;

	// Data duplicated from the plant form to avoid a map lookup
	float	BoundingRadius;
	float	CollisionRadius;
};

// ***************************************************************************

// Flora class, filled with the flora form
class CFlora
{
public:
	// The plants references
	vector<CFloraPlant>			Plants;

	// Excludes primitives
	vector<const CPrimZoneEditor*>	ExcludePrimitives;

	// Generation parameters
	float   JitterPos;
	float   ScaleMin;
	float   ScaleMax;
	bool	PutOnWater;
	float	WaterHeight;
	uint32	RandomSeed;

	// *** Precalculation

	// Bounding min
	CVector	VMin;

	// Bounding max
	CVector	VMax;

	// *** Variables
	uint	NewPrimitiveCount;
};

// ***************************************************************************

/* Sort plant
 * This struct is used to sort plants by radius
 */
class CSortPlant
{
public:
	// Contructor
	CSortPlant (uint floraId, uint plantId, const vector<CFlora> *floraVector)
	{
		FloraId = floraId;
		PlantId = plantId;
		FloraVector = floraVector;
	}

	// Comparison
	bool operator< (const CSortPlant &other) const
	{
		return (*FloraVector)[FloraId].Plants[PlantId].BoundingRadius < (*FloraVector)[other.FloraId].Plants[other.PlantId].BoundingRadius;
	}

	// The flora id
	uint	FloraId;

	// The plant id
	uint	PlantId;

	// The flora vector
	const vector<CFlora> *FloraVector;
};

// ***************************************************************************

bool CGeneratePrimitive::generate (std::vector< std::vector<IPrimitive*> > &dest, const std::vector<const IPrimitive*> &source, 
								   NLMISC::IProgressCallback &callback, const CLigoConfig &config, const char *dataDirectory, 
								   const char *primClassToGenerate)
{
	// Get the doc
	CWorldEditorDoc *doc = getDocument ();
	
	// Erase dest
	dest.clear ();
	dest.resize (source.size ());

	// Clear
	clear ();

	// Create a landscape
	_Landscape = new CLandscape ();
	_Landscape->init ();

	// Get the prim class to generate
	const CPrimitiveClass *genPrimClass = theApp.Config.getPrimitiveClass (primClassToGenerate);
	nlassert (genPrimClass);

	// Visual collision manager
	/* todo COL_USING_VISUAL_COLLISION_MANAGER
	_VCM = new CVisualCollisionManager;
	_VCE = _VCM->createEntity ();
	_VCM->setLandscape (_Landscape);
	_VCE->setSnapToRenderedTesselation (false);
	*/

	// Load the landscape
	if (loadLandscape (source, callback, config, dataDirectory))
	{
		// Generate vegetation
		uint i, j;

		// Load the .Flora file (georges file) and load all associated .plant
		vector<CFlora> flora (source.size ());

		// Create a loader
		UFormLoader *loader = UFormLoader::createLoader ();

		// Keep last form alive
		CSmartPtr<UForm> form;
		CSmartPtr<UForm> form2;

		// For each source primitives
		for (i=0; i<source.size (); i++)
		{
			// Get the flora form name
			string floraName;
			if (source[i]->getPropertyByName ("form", floraName))
			{
				// Progress bar
				callback.DisplayString = "Reading "+(floraName+".flora")+"...";
				callback.progress ((float)i/(float)source.size ());
				callback.pushCropedValues ((float)i/(float)source.size (), (float)(i+1)/(float)source.size ());

				// Get the path name
				string filename = CPath::lookup (floraName+".flora", false, false, false);
				if (!filename.empty ())
				{
					// Load the form
					form = loader->loadForm (filename.c_str ());
					if (form == NULL)
					{
						// One more error
						_FileNotFound.push_back (filename);
					}
					else
					{
						// Get the root node
						UFormElm &rootNode = form->getRootNode ();

						// Read the Plants field
						UFormElm *pElt;
						if (rootNode.getNodeByName (&pElt, "Plants") && pElt)
						{
							uint size;
							nlverify (pElt->getArraySize (size));
							for (j=0; j<size; j++)
							{
								// Get the struct
								UFormElm *pArrayElt;
								if (pElt->getArrayNode (&pArrayElt, j) && pArrayElt)
								{
									// Read the plant values
									CFloraPlant plant;
									pArrayElt->getValueByName (plant.Name, "File name");
									pArrayElt->getValueByName (plant.Density, "Density");
									pArrayElt->getValueByName (plant.Falloff, "Falloff");
									plant.CollisionRadius = 0.f;
									plant.BoundingRadius = 1;
									plant.Name = NLMISC::CFile::getFilenameWithoutExtension (plant.Name);

									// Progress bar
									callback.DisplayString = "Reading "+plant.Name+"...";
									callback.progress ((float)j/(float)size);

									// Get the path name
									string filename = CPath::lookup (plant.Name+".plant", false, false, false);
									if (!filename.empty ())
									{
										// Read the plant file
										form2 = loader->loadForm (filename.c_str());
										if (form2)
										{
											// Read the plant
											float CollisionRadius = 0;
											float BoundingRadius = 1;

											// Get root node
											UFormElm &rootNode2 = form2->getRootNode ();

											rootNode2.getValueByName (plant.CollisionRadius, "3D.Collision Radius");
											rootNode2.getValueByName (plant.BoundingRadius, "3D.Bounding Radius");

											// Add it
											flora[i].Plants.push_back (plant);
										}
										else
										{
											_FileNotFound.push_back (filename);
										}								
									}
									else
									{
										_FileNotFound.push_back (plant.Name);
									}
								}
							}
						}

						// Read the flora values
						flora[i].NewPrimitiveCount = 0;
						rootNode.getValueByName (flora[i].JitterPos, "Jitter_Pos");
						rootNode.getValueByName (flora[i].ScaleMin, "Scale_Min");
						rootNode.getValueByName (flora[i].ScaleMax, "Scale_Max");
						rootNode.getValueByName (flora[i].PutOnWater, "Put_On_Water");
						rootNode.getValueByName (flora[i].WaterHeight, "Water_Height");
						rootNode.getValueByName (flora[i].RandomSeed, "Random_Seed");

						// For each vertices
						uint numVertices = source[i]->getNumVector ();
						if (numVertices)
						{
							// Get the vector position
							const CPrimVector *pos = source[i]->getPrimVector ();

							// Get the bounding of the zone
							flora[i].VMin = flora[i].VMax = pos[0];
							for (j = 0; j < numVertices; ++j)
							{
								// Get the vector position
								if (flora[i].VMin.x > pos[j].x) 
									flora[i].VMin.x = pos[j].x;
								if (flora[i].VMin.y > pos[j].y) 
									flora[i].VMin.y = pos[j].y;
								if (flora[i].VMin.z > pos[j].z) 
									flora[i].VMin.z = pos[j].z;
								if (flora[i].VMax.x < pos[j].x) 
									flora[i].VMax.x = pos[j].x;
								if (flora[i].VMax.y < pos[j].y) 
									flora[i].VMax.y = pos[j].y;
								if (flora[i].VMax.z < pos[j].z) 
									flora[i].VMax.z = pos[j].z;
							}
						}
											
						// ** Get exclude zone
						uint numFloraChildren = source[i]->getNumChildren ();
						for (uint k=0; k<numFloraChildren; k++)
						{
							// The child
							const IPrimitive *child;
							nlverify (source[i]->getChild (child, k));

							// Is it a zone ?
							const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor*> (child);
							if (zone)
							{
								// Is it an explude zone ?
								string className;
								if (zone->getPropertyByName ("class", className) && (className == "flora_exclude"))
								{
									// Add it
									flora[i].ExcludePrimitives.push_back (zone);
								}
							}
						}
					}
				}
				else
				{
					_FileNotFound.push_back (floraName+".flora");
				}

				// Progress bar
				callback.popCropedValues ();
			}
		}

		// Error ?
		if (!_FileNotFound.empty ())
		{
			// Continue ?
			char message[2048];
			message[0] = 0;
			for (uint i=0; i<_FileNotFound.size (); i++)
			{
				strcat (message, _FileNotFound[i].c_str ());
				strcat (message, "\n");
			}

			if (!theApp.yesNoMessage ("Can't load some files:\n%s\nContinue ?", message))
			{
				return false;
			}
		}

		// Insert plants
		vector<CSortPlant> sortedPlants;
		for (i=0; i<source.size (); i++)
		for (j=0; j<flora[i].Plants.size (); j++)
		{
			// Inserted
			sortedPlants.push_back (CSortPlant (i, j, &flora));
		}

		// Some plants ?
		if (!sortedPlants.empty ())
		{
			// Sort it
			sort (sortedPlants.begin (), sortedPlants.end ());

			// *** Generating

			// ** Build the quad grid

			// Quad grid to check primitive overlapping
			CQuadGrid<CQuadGridElement> quadGrid;
			quadGrid.create (256, 10.f);

			// Progress bar
			callback.DisplayString = "Build quad grid";

			// Insert all the primitive found
			uint numPrimitive = doc->getNumDatabaseElement ();
			for (i=0; i<numPrimitive; i++)
			{
				// Progress bar
				callback.progress ((float)i/(float)numPrimitive);
				callback.pushCropedValues ((float)i/(float)numPrimitive, (float)(i+1)/(float)numPrimitive);

				// Insert the primitive
				insertPrimitive (quadGrid, *(doc->getDatabaseElements (i).RootNode), &callback);

				// Progress bar
				callback.popCropedValues ();
			}

			// ** Take a srand found in selected primitives

			uint randSeed = 0;
			for (i=0; i<flora.size (); i++)
				randSeed += flora[i].RandomSeed;
			srand (randSeed);

			// ** Pose plant

			// Progress bar
			callback.DisplayString = "Generate primitives";

			// For each sorted plants, bigger first
			nlassert (sortedPlants.size ());
			sint sortedPlant;
			for (sortedPlant = sortedPlants.size ()-1; sortedPlant >= 0; sortedPlant--)
			{
				// Progress bar
				callback.progress ((float)(sortedPlants.size ()-sortedPlant-1)/(float)(sortedPlants.size ()));
				callback.pushCropedValues ((float)(sortedPlants.size ()-sortedPlant-1)/(float)sortedPlants.size (), (float)(sortedPlants.size ()-sortedPlant)/(float)sortedPlants.size ());

				// Ref on the flora
				CFlora &theFlora = flora[sortedPlants[sortedPlant].FloraId];

				// Ref on the plant
				const CFloraPlant &thePlant = theFlora.Plants[sortedPlants[sortedPlant].PlantId];

				// Get a valid jitter
				float jitter = theFlora.JitterPos;
				clamp (jitter, 0.0f, 1.0f);

				// Is it a non empty zone or path ?
				const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> (source[sortedPlants[sortedPlant].FloraId]);
				if (zone)
				{
					// Some vector
					if (((const IPrimitive*)zone)->getNumVector ())
					{
						// Distance between primitives
						float squareLength = (float)sqrt (Pi*thePlant.BoundingRadius*thePlant.BoundingRadius / thePlant.Density);

						// Number of primitive on X and Y axis to try to place
						uint32 nNbPlantX = 1+(int)floor ((theFlora.VMax.x-theFlora.VMin.x) / squareLength);
						uint32 nNbPlantY = 1+(int)floor ((theFlora.VMax.y-theFlora.VMin.y) / squareLength);

						// Pose plants
						uint k, l;
						for (l = 0; l < nNbPlantY; ++l)
						for (k = 0; k < nNbPlantX; ++k)
						{
							// Progress bar
							callback.progress ((float)(l*nNbPlantX+k)/(float)(nNbPlantY*nNbPlantX));

							// Try sveral times times
							uint m;
							for (m = 0; m < GENERATE_PRIMITIVE_TEST_COUNT; ++m)
							{
								// Generate a position
								CVector pos;
								pos.x = theFlora.VMin.x + squareLength * k + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
								pos.y = theFlora.VMin.y + squareLength * l + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
								pos.z = 0.0f;

								// Does the zone contains the position generated ?
								if (zone->contains(pos))
								{
									// Add the primitive
									if (addPrimitive (dest[sortedPlants[sortedPlant].FloraId], pos, primClassToGenerate, *zone, theFlora, thePlant, quadGrid, config))
									{
										theFlora.NewPrimitiveCount++;
										break;
									}
								}
							}
						}
					}
				}
				else
				{
					// Is it a path ?
					const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> (source[sortedPlants[sortedPlant].FloraId]);
					if (path)
					{
						// Total length of the broken line
						float length = 0.0f; 
						for (j = 0; j < path->VPoints.size()-1; ++j)
							length += (path->VPoints[j]-path->VPoints[j+1]).norm();

						// Number of primitives on this path
						float squareLength = (float)(2*thePlant.BoundingRadius / thePlant.Density);
						uint nNbPlant = 1+(int)floor (length / squareLength);

						// Pose plants
						uint k;
						for (k = 0; k < nNbPlant; ++k)
						{
							// Progress bar
							callback.progress ((float)(k)/(float)(nNbPlant));

							bool bExists = false;
							CVector pos;

							// Try sveral times times
							uint m;
							for (m = 0; m < GENERATE_PRIMITIVE_TEST_COUNT; ++m)
							{
								// Calculate the curviline abscisse
								float curvAbs = squareLength * k + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
								if ((curvAbs>=0) && (curvAbs<length))
								{
									float TempLength = 0.0f;

									// Convert to a real point along the curve (broken line)
									uint l;
									for (l = 0; l < path->VPoints.size()-1; ++l)
									{
										float newSize = (path->VPoints[l]-path->VPoints[l+1]).norm();
										if (curvAbs < (TempLength+newSize))
										{
											curvAbs -= TempLength;
											break;
										}
										TempLength += newSize;
									}

									// Placed ?
									if (l != (path->VPoints.size()-1))
									{
										// Calculate the coord
										curvAbs = curvAbs / (path->VPoints[l]-path->VPoints[l+1]).norm();
										pos = path->VPoints[l] + (path->VPoints[l+1]-path->VPoints[l])*curvAbs;
										pos.z = 0.0f;

										// Add the primitive
										if (addPrimitive (dest[sortedPlants[sortedPlant].FloraId], pos, primClassToGenerate, *path, theFlora, thePlant,quadGrid, config))
										{
											theFlora.NewPrimitiveCount++;
											break;
										}
									}
								}
							}
						}
					}
				}

				// Progress bar
				callback.popCropedValues ();
			}
		}

		// Done
		return true;
	}

	// Failed
	return false;
}

// ***************************************************************************

bool CGeneratePrimitive::loadLandscape (const std::vector<const NLLIGO::IPrimitive*> &source, NLMISC::IProgressCallback &callback, 
								const NLLIGO::CLigoConfig &config, const char *dataDirectory)
{
	// Get the bbox of the primitives
	CAABBox bbox;
	bool firstPoint = true;
	for (uint i=0; i<source.size (); i++)
	{
		// Extand the bbox
		uint nbVert = source[i]->getNumVector ();
		if (nbVert)
		{
			// Get the vectors
			const CPrimVector *vectors = source[i]->getPrimVector ();
			for (uint j=0; j<nbVert; j++)
			{
				// Extand the bbox
				if (firstPoint)
				{
					bbox.setCenter (vectors[j]);
					firstPoint = false;
				}
				else
				{
					bbox.extend (vectors[j]);
				}
			}
		}
	}

	if (!firstPoint)
	{
		// Get the bounding values
		int minx = (int)floor (bbox.getMin ().x / config.CellSize);
		int miny = (int)floor (bbox.getMin ().y / config.CellSize);
		int maxx = (int)floor (bbox.getMax ().x / config.CellSize);
		int maxy= (int)floor (bbox.getMax ().y / config.CellSize);

		// Load the landscape zones
		for (int x=minx; x<=maxx; x++)
		for (int y=miny; y<=maxy; y++)
		{
			// Get the zone name
			std::string zoneName;
			if (getZoneNameFromXY (x, y, zoneName))
			{
				// Load the zone
				CIFile file;
				bool opened = true;
				string filename;
				if (!file.open (filename = standardizePath (dataDirectory)+"zones_generated/"+zoneName+".zone"))
				{
					if (!file.open (filename = standardizePath (dataDirectory)+"zones_generated/"+zoneName+".zonew"))
					{
						if (!file.open (filename = standardizePath (dataDirectory)+"zones_generated/"+zoneName+".zonel"))
						{
							opened = false;
						}
					}
				}

				// Opened ?
				if (opened)
				{
					// Message
					callback.DisplayString = "Reading zone "+NLMISC::CFile::getFilename (filename)+"...";

					// Progress bar
					callback.progress ((float)((x-minx)*(maxy-miny+1)+(y-miny))/(float)((maxx-minx+1)*(maxy-miny+1)));
					
					// Serial it, and add it into the landscape
					CZone zone;
					zone.serial (file);
					_Landscape->addZone (zone);
				}
			}
		}
	}

	// Done
	return true;
}

// ***************************************************************************

void CGeneratePrimitive::clear ()
{
	/* todo COL_USING_VISUAL_COLLISION_MANAGER
	// Delete the entity
	if (_VCE)
	{
		nlassert (_VCM);
		_VCM->deleteEntity;
		_VCE = NULL;
	}

	// Delete the manager
	if (_VCM)
	{
		delete _VCM;
		_VCM = NULL;
	}
	*/

	// Delete the landscape
	if (_Landscape)
	{
		delete (_Landscape);
		_Landscape = NULL;
	}

	_FileNotFound.clear ();
}

// ***************************************************************************

CGeneratePrimitive::~CGeneratePrimitive ()
{
	clear ();
}

// ***************************************************************************

void CGeneratePrimitive::insertPrimitive (CQuadGrid<CQuadGridElement> &quadGrid, const IPrimitive &primitive, NLMISC::IProgressCallback *callback)
{
	// is it a point ?
	const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor*> (&primitive);
	if (point)
	{
		// Insert it
		CQuadGridElement element;

		// * Is it collisionnable ?

		// Get the primitive class
		string className;
		nlverify (primitive.getPropertyByName ("class", className));
		const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (className.c_str ());
		if (primClass)
		{
			// Get the position
			element.Position = point->Point;
			element.Position.z = 0;

			// Default radius
			element.Radius = 0;
			
			// This class make collision during generation ?
			if (primClass->Collision)
			{
				// Get the radius
				string radius;
				if (point->getPropertyByName ("radius", radius))
				{
					// Get the value
					sscanf (radius.c_str (), "%f", &element.Radius);
				}

				// Insert BB
				NLMISC::CVector bboxmin = element.Position - CVector (element.Radius, element.Radius, element.Radius);
				NLMISC::CVector bboxmax = element.Position + CVector (element.Radius, element.Radius, element.Radius);

				// Insert it
				quadGrid.insert (bboxmin, bboxmax, element);
			}
		}
	}

	// Scan children
	uint numChildren = primitive.getNumChildren ();
	for (uint i=0; i<numChildren; i++)
	{
		// Progress bar
		if (callback)
		{
			callback->progress ((float)i/(float)numChildren);
			callback->pushCropedValues ((float)i/(float)numChildren, (float)(i+1)/(float)numChildren);
		}

		// Insert child
		const IPrimitive *child;
		nlverify (primitive.getChild (child, i));
		insertPrimitive (quadGrid, *child, callback);

		// Progress bar
		if (callback)
			callback->popCropedValues ();
	}
}

// ***************************************************************************

bool CGeneratePrimitive::addPrimitive (std::vector<NLLIGO::IPrimitive*> &dest, const NLMISC::CVector &pos, 
						const char *primClassToGenerate, const NLLIGO::IPrimitive &parent, 
						const class CFlora &flora, const class CFloraPlant &plant,
						NL3D::CQuadGrid<CQuadGridElement> &quadGrid, const CLigoConfig &config)
{
	// Generate a scale
	float scale = (flora.ScaleMax-flora.ScaleMin)*frand(1.0)+flora.ScaleMin;

	// Test collision
	// note : i'm not sure the layer is really usefull here because primitive are always generated on the layer 0
	if (testPosition (pos, plant, flora, scale, 0, quadGrid, config))
	{
		// Test finally with the exclude primitive
		uint m;
		for (m = 0; m < flora.ExcludePrimitives.size(); m++)
		{
			// Contains in the exclude primitive ?
			if (flora.ExcludePrimitives[m]->contains(pos))
				break;
		}

		// No collision ?
		if (m == flora.ExcludePrimitives.size())
		{
			// Create a new primitive
			CPrimPointEditor *newPoint = new CPrimPointEditor;

			// Position and angle
			newPoint->Point = pos;
			newPoint->Angle = (float)Pi * frand (2.0);

			// Add its class
			newPoint->addPropertyByName ("class", new CPropertyString (primClassToGenerate));

			// * Add a name

			// Get its parent name
			string parentName;
			parent.getPropertyByName ("name", parentName);

			// Get number of brothers
			uint numBrother = parent.getNumChildren () + flora.NewPrimitiveCount;

			// Set the name
			newPoint->addPropertyByName ("name", new CPropertyString ((parentName+" "+toString (numBrother)).c_str ()));

			// Set the form
			newPoint->addPropertyByName ("form", new CPropertyString (plant.Name.c_str ()));

			// Set the radius
			newPoint->addPropertyByName ("radius", new CPropertyString ((toString (scale*plant.BoundingRadius).c_str ())));

			// Set the scale
			newPoint->addPropertyByName ("scale", new CPropertyString ((toString (scale).c_str ())));

			// Set the layer
			newPoint->addPropertyByName ("layer", new CPropertyString ("0"));

			// Put on water ?
			if (flora.PutOnWater)
			{
				// Unset the flag to don't snap over the landscape
				newPoint->addPropertyByName ("snap", new CPropertyString ("false"));

				// 
				newPoint->addPropertyByName ("height", new CPropertyString (toString(flora.WaterHeight).c_str()));

				// Align position
				if (newPoint->Point.z < flora.WaterHeight)
					newPoint->Point.z = flora.WaterHeight;
			}
			
			// Add the primitive
			dest.push_back (newPoint);

			// Add it in the grid
			insertPrimitive (quadGrid, *newPoint, NULL);

			// Posed
			return true;
		}
	}
	
	// Not posed
	return false;
}

// ***************************************************************************

bool CGeneratePrimitive::testPosition (const CVector &pos, const CFloraPlant &plant, const CFlora &flora, float scale, uint layer,
									   NL3D::CQuadGrid<CQuadGridElement> &quadGrid, const CLigoConfig &config)
{
	// Clear the selection
	quadGrid.clearSelection ();

	// Selection BB
	CVector pos2 = pos;
	pos2.z = 0;
	CVector bboxmin = pos2 - CVector (plant.BoundingRadius, plant.BoundingRadius, plant.BoundingRadius);
	CVector bboxmax = pos2 + CVector (plant.BoundingRadius, plant.BoundingRadius, plant.BoundingRadius);

	// Select
	quadGrid.select (bboxmin, bboxmax);

	// Test selection
	CQuadGrid<CQuadGridElement>::CIterator ite = quadGrid.begin();
	while (ite != quadGrid.end())
	{
		// Distance between the 2 primitives
		CVector dist = pos - (*ite).Position;
		dist.z = 0;

		// Distance max
		float distMax = (*ite).Radius + plant.BoundingRadius;

		// Test
		if (dist.sqrnorm () < (distMax*distMax))
			return false;

		// Next
		ite++;
	}

#ifdef TEST_Z_POSITION
	// *** Check the position is good, ie, the collision circle is over the landscape at a small delta Z of the center

	// Approximate the z with patch bounding boxes
	sint32 zoneX = (sint32)floor (pos.x/config.CellSize);
	sint32 zoneY = (sint32)floor (-pos.y/config.CellSize);
	sint32 zoneId = zoneY * 256 +  zoneX;
	CZone *zone = _Landscape->getZone (zoneId);

	// Zone exist ?
	if (zone)
	{
		// Get the zone bbox
		CAABBoxExt zoneBBox = zone->getZoneBB ();

		// The bbox used to select triangles
		CAABBox bbox;
		bbox.setCenter (CVector (pos.x + SELECTION_EPSILON, pos.y + SELECTION_EPSILON, zoneBBox.getMax ().z + SELECTION_EPSILON));
		bbox.extend (CVector (pos.x - SELECTION_EPSILON, pos.y - SELECTION_EPSILON, zoneBBox.getMin ().z - SELECTION_EPSILON));

		// *** Extend the box

		// Get some Z around to see if we can put the Flora on the ground
		vector<CVector> base;
		base.resize (Z_SAMPLE_COUNT);
		uint i;
		for (i = 0; i < Z_SAMPLE_COUNT; ++i)
		{
			// Ref on the new position
			CVector &destPos = base[i];

			destPos = pos;
			destPos.x += scale * plant.CollisionRadius * cosf((2.0f*(float)Pi*i)/(float)Z_SAMPLE_COUNT);
			destPos.y += scale * plant.CollisionRadius * sinf((2.0f*(float)Pi*i)/(float)Z_SAMPLE_COUNT);

			// Extend the bbox
			bbox.extend (CVector (destPos.x - SELECTION_EPSILON, destPos.y - SELECTION_EPSILON, 0));
			bbox.extend (CVector (destPos.x + SELECTION_EPSILON, destPos.y + SELECTION_EPSILON, 0));
		}

		// Min and max
		float zMin = bbox.getMin ().z;
		float zMax = bbox.getMax ().z;

		// Select some triangles
		vector<CTrianglePatch> triangles;
		_Landscape->buildTrianglesInBBox (bbox, triangles, 0);

		// *** Get the real Z
		float centerZ;
		if (!getZFromXY (centerZ, pos.x, pos.y, zMin, zMax, layer, config, triangles))
		{
			// Primitive placed in a hole
			return false;
		}

		// Get some Z around to see if we can put the Flora on the ground
		for (i = 0; i < Z_SAMPLE_COUNT; ++i)
		{
			// Ref on the new position
			CVector &destPos = base[i];

			// Z under us ?
			if (!getZFromXY (destPos.z, destPos.x, destPos.y, zMin, zMax, layer, config, triangles))
			{
				// Primitive placed in a hole
				return false;
			}

			// todo : pass this threshold in parameter. hard coded delta z here..
			if (fabs(destPos.z - centerZ) > Z_THRESHOLD)
			{
				return false;
			}
		}
	}
#endif // TEST_Z_POSITION

	// All good
	return true;
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

bool CGeneratePrimitive::getZFromXY (float &result, float x, float y, float zMin, float zMax, uint layer, 
									 const NLLIGO::CLigoConfig &config, const vector<CTrianglePatch> &triangles)
{
	/* todo COL_USING_VISUAL_COLLISION_MANAGER
	// The position
	CVector pos = CVector(x, y, 0);
	float z, zmin, zmax;

	// Zone BBox
	CAABBoxExt bb = pZone->getZoneBB();
	zmin = bb.getMin().z;
	zmax = bb.getMax().z;
	pos.z = zmin;
	z = zmin;

	// Hulud: I'm not sure to understand that code..
	// I don't chage anything because, i need the same result
	{
		while (z < zmax)
		{
			// Result normal
			CVector normal;
			if (_VCE->snapToGround(pos, normal))
				break;

			// Super sampling due to max frequency on radiosity
			z += CVisualCollisionEntity::BBoxRadiusZ / 2.0f; 
			pos.z = z;
		}

		if (z >= zmax)
			return false;
	}

	// Done
	result = pos.z;
	return true;
	*/

	// The position
	CVector position (x, y, 0);

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
			// The min max position
			CVector minPos (x, y, zMin);
			CVector maxPos (x, y, zMax);

			// build the plane
			CPlane plane;
			plane.make (triangle.V0, triangle.V1, triangle.V2);

			// Get the final height
			CVector intersect = plane.intersect (minPos, maxPos);
			selectedHeight.insert (intersect.z);
		}
	}
	
	// Look for the triangle in the good layer
	uint currentLayer = 0;

	// Number of layer found
	uint layerCount = selectedHeight.size ();

	// Layer exist ?
	if (layer < layerCount)
	{
		// Invert the layer number
		layer = layerCount - layer - 1;

		// Look for the good layer
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
		result = *ite;

		// Collision found
		return true;
	}

	// Not found 
	return false;
}

// ***************************************************************************

