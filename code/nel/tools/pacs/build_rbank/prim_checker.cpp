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

#include "prim_checker.h"

// NeL Misc includes
#include "nel/misc/vectord.h"
#include "nel/misc/path.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"

// NeL Ligo includes
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"

// NeL 3d
#include "nel/3d/scene_group.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/water_model.h"
#include "nel/3d/water_shape.h"
#include "nel/3d/quad_grid.h"

// STL includes
#include <vector>
#include <set>

using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;
using namespace std;

NLLIGO::CLigoConfig LigoConfig;
extern bool						Verbose;
extern float					WaterThreshold;

/*
 * Constructor
 */
CPrimChecker::CPrimChecker()
{
}



/*
 *		init()
 */
bool	CPrimChecker::build(const string &primitivesPath, const string &igLandPath, const string &igVillagePath, const string &outputDirectory, bool forceRebuild)
{
	if (Verbose)
		nlinfo("Checking pacs.packed_prims consistency");

	NLLIGO::Register();

	// Init ligo
	if (!LigoConfig.readPrimitiveClass ("world_editor_classes.xml", false))
	{
		// Should be in l:\leveldesign\world_edit_files
		nlwarning ("Can't load ligo primitive config file world_editor_classes.xml");
		return false;
	}

	uint	i, j;
	string	outputfname = CPath::standardizePath(outputDirectory)+"pacs.packed_prims";

	_Grid.clear();

	vector<string>	files;
	CPath::getPathContent(primitivesPath, true, false, true, files);

	for (i=0; i<files.size(); ++i)
	{
		if (CFile::getExtension(files[i]) == "primitive")
		{
			readFile(files[i]);
		}
	}

	files.clear();
	CPath::getPathContent(igLandPath, true, false, true, files);
	CPath::getPathContent(igVillagePath, true, false, true, files);

	set<string>		noWaterShapes;

	for (i=0; i<files.size(); ++i)
	{
		try
		{
			// load ig associated to the zone
			string	igname = files[i];

			string	ignamelookup = CPath::lookup(igname);
			//nlinfo("Reading ig '%s'", ignamelookup.c_str());
			CIFile			igStream(ignamelookup);
			CInstanceGroup	ig;
			igStream.serial(ig);

			// search in group for water instance
			for (j=0; j<ig._InstancesInfos.size(); ++j)
			{
				string	shapeName = ig._InstancesInfos[j].Name;
				if (CFile::getExtension (shapeName) == "")
					shapeName += ".shape";

				if (noWaterShapes.find(shapeName) != noWaterShapes.end())
					continue;

				string	shapeNameLookup = CPath::lookup (shapeName, false, false);
				if (!shapeNameLookup.empty())
				{
					CIFile			f;
					if (f.open (shapeNameLookup))
					{
						CShapeStream	shape;
						shape.serial(f);

						CWaterShape	*wshape = dynamic_cast<CWaterShape*>(shape.getShapePointer());
						if (wshape == NULL)
						{
							noWaterShapes.insert(shapeName);
							continue;
						}

						//nlinfo("Render water shape '%s'", shapeNameLookup.c_str());

						CMatrix	matrix;
						ig.getInstanceMatrix(j, matrix);

						CPolygon			wpoly;
						//wshape->getShapeInWorldSpace(wpoly);
						CPolygon2D			wpoly2d = wshape->getShape();

						uint	k;
						for (k=0; k<wpoly2d.Vertices.size(); ++k)
						{
							wpoly.Vertices.push_back(matrix * wpoly2d.Vertices[k]);
						}

						float	zwater = wpoly.Vertices[0].z - WaterThreshold;
						uint16	idx = (uint16)_WaterHeight.size();
						_WaterHeight.push_back(zwater);
						render(wpoly, idx);

						if (Verbose)
							nlinfo("Rendered water shape '%s' in instance '%s'", CFile::getFilenameWithoutExtension(shapeName).c_str(), CFile::getFilenameWithoutExtension(igname).c_str());
					}
					else if (Verbose)
					{
						noWaterShapes.insert(shapeName);
						nlwarning ("Can't load shape %s", shapeNameLookup.c_str());
					}
				}
				else if (Verbose)
				{
					noWaterShapes.insert(shapeName);
					nlwarning ("Can't find shape %s", shapeName.c_str());
				}
			}
		}
		catch (const Exception &e)
		{
			nlwarning("%s", e.what());
		}
	}

	COFile	f;
	if (f.open(outputfname))
	{
		f.serial(_Grid);
		f.serialCont(_WaterHeight);
	}
	else
	{
		nlwarning("Couldn't save pacs.packed_prims file '%s'", outputfname.c_str());
	}

	return true;
}



/*
 *		load()
 */
bool	CPrimChecker::load(const string &outputDirectory)
{
	string	outputfname = CPath::standardizePath(outputDirectory)+"pacs.packed_prims";

	CIFile	f;
	if (f.open(outputfname))
	{
		f.serial(_Grid);
		f.serialCont(_WaterHeight);
	}
	else
	{
		nlwarning("Couldn't load pacs.packed_prims file '%s'", outputfname.c_str());
		return false;
	}

	return true;
}





/*
 *		readFile()
 */
void	CPrimChecker::readFile(const string &filename)
{
	string	fullpath = CPath::lookup(filename, false);

	if (fullpath.empty())
		return;

	// lookup for primitive file
	CIFile		f(fullpath);
	CIXml		xml;

	CPrimitives	prims;

	// load xml file
	xml.init(f);
	if (Verbose)
		nlinfo("Loaded prim file '%s'", filename.c_str());

	// read nodes
	if (!prims.read(xml.getRootNode(), filename.c_str(), LigoConfig))
	{
		nlwarning("Can't use primitive file '%s', xml parse error",  filename.c_str());
		return;
	}

	// get CPrimNode
	CPrimNode	*primRootNode = prims.RootNode;

	// read recursive node
	readPrimitive(primRootNode);
}

/*
 *		readPrimitive()
 */
void	CPrimChecker::readPrimitive(IPrimitive *primitive)
{
	string	className;

	// check good class and check primitive has a class name
	if (dynamic_cast<CPrimZone*>(primitive) != NULL && primitive->getPropertyByName("class", className))
	{
		if (className == "pacs_include")
			render(static_cast<CPrimZone*>(primitive), Include);
		else if (className == "pacs_exclude")
			render(static_cast<CPrimZone*>(primitive), Exclude);
		else if (className == "pacs_cluster_hint")
			render(static_cast<CPrimZone*>(primitive), ClusterHint);
	}

	// parse children
	uint	i;
	for (i=0; i<primitive->getNumChildren(); ++i)
	{
		IPrimitive	*child;

		if (!primitive->getChild(child, i))
			continue;

		readPrimitive(child);
	}
}


/*
 *		render()
 */
void	CPrimChecker::render(CPrimZone *zone, uint8 bits)
{
	if (zone->VPoints.size() < 3)
		return;

	string	name;
	if (zone->getPropertyByName("name", name) && Verbose)
		nlinfo("Rendering CPrimZone '%s'", name.c_str());

	// get the bouding box of the CPrimZone
	CAABBox	box;

	box.setCenter(zone->VPoints[0]);
	box.setHalfSize(CVector::Null);

	uint	i;
	for (i=1; i<zone->VPoints.size(); ++i)
		box.extend(zone->VPoints[i]);

	sint32	xmin, ymin, xmax, ymax;

	xmin = (sint32)(floor(box.getMin().x));
	ymin = (sint32)(floor(box.getMin().y));

	xmax = (sint32)(ceil(box.getMax().x));
	ymax = (sint32)(ceil(box.getMax().y));

	// Fill grid with points that belong to the CPrimZone
	sint32	x, y;
	for (y=ymin; y<=ymax; ++y)
		for (x=xmin; x<=xmax; ++x)
			if (zone->contains(CVector((float)x, (float)y, 0.0f)))
				_Grid.set(x, y, bits);
}


/*
 * Render a water shape, as a CPolygon
 */
void	CPrimChecker::render(const CPolygon &poly, uint16 value)
{
	list<CPolygon>		convex;

	// divide poly in convex polys
	if (!poly.toConvexPolygons(convex, CMatrix::Identity))
	{
		convex.clear();
		CPolygon	reverse = poly;
		std::reverse(reverse.Vertices.begin(), reverse.Vertices.end());
		if (!reverse.toConvexPolygons(convex, CMatrix::Identity))
			return;
	}

	list<CPolygon>::iterator	it;
	for (it=convex.begin(); it!=convex.end(); ++it)
	{
		CPolygon2D					convex2d(*it);

		CPolygon2D::TRasterVect		rasterized;
		sint						ymin;

		convex2d.computeBorders(rasterized, ymin);

		sint	dy;
		for (dy=0; dy<(sint)rasterized.size(); ++dy)
		{
			sint	x;

			for (x=rasterized[dy].first; x<=rasterized[dy].second; ++x)
			{
				uint8	prevBits = _Grid.get((uint)x, (uint)(ymin+dy));

				// only set if there was not a water shape there or if previous was lower
				if ((prevBits & Water) != 0)
				{
					uint16	prevWS = _Grid.index((uint)x, (uint)(ymin+dy));

					if (_WaterHeight[value] < _WaterHeight[prevWS])
						continue;
				}

				_Grid.index((uint)x, (uint)(ymin+dy), value);
				_Grid.set((uint)x, (uint)(ymin+dy), Water);
			}
		}
	}

}

/*
 * Render a CPolygon of bit value
 */
void	CPrimChecker::renderBits(const CPolygon &poly, uint8 bits)
{
	list<CPolygon>		convex;

	// divide poly in convex polys
	if (!poly.toConvexPolygons(convex, CMatrix::Identity))
	{
		convex.clear();
		CPolygon	reverse = poly;
		std::reverse(reverse.Vertices.begin(), reverse.Vertices.end());
		if (!reverse.toConvexPolygons(convex, CMatrix::Identity))
			return;
	}

	list<CPolygon>::iterator	it;
	for (it=convex.begin(); it!=convex.end(); ++it)
	{
		CPolygon2D					convex2d(*it);

		CPolygon2D::TRasterVect		rasterized;
		sint						ymin;

		convex2d.computeBorders(rasterized, ymin);

		sint	dy;
		for (dy=0; dy<(sint)rasterized.size(); ++dy)
		{
			sint	x;

			for (x=rasterized[dy].first; x<=rasterized[dy].second; ++x)
			{
				_Grid.set((uint)x, (uint)(ymin+dy), bits);
			}
		}
	}

}


