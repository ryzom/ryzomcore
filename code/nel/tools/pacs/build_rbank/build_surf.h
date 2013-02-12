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

#ifndef NL_BUILD_SURF_H
#define NL_BUILD_SURF_H

#include <vector>

#include "nel/misc/debug.h"
#include "nel/misc/file.h"

#include "nel/3d/zone.h"
#include "nel/3d/patch.h"
#include "nel/3d/mesh.h"
#include "nel/3d/landscape.h"

#include "nel/3d/quad_tree.h"
#include "nel/3d/quad_grid.h"

#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/polygon.h"

#include "nel/pacs/surface_quad.h"
#include "nel/pacs/retrievable_surface.h"

#include "prim_checker.h"



extern std::string				OutputRootPath;
extern std::string				OutputDirectory;
extern std::string				OutputPath;
extern std::string				TessellationPath;
extern std::string				IGBoxes;
extern uint						TessellateLevel;
extern bool						ReduceSurfaces;
extern bool						SmoothBorders;
extern bool						ComputeElevation;
extern bool						ComputeLevels;
extern std::vector<std::string>	ZoneNames;
extern std::string				ZoneExt;
extern std::string				ZoneNHExt;
extern std::string				ZoneLookUpPath;

extern bool						ProcessAllPasses;
extern bool						CheckPrims;
extern bool						TessellateZones;
extern bool						MoulineZones;
extern bool						TessellateAndMoulineZones;
extern bool						ProcessRetrievers;
extern std::string				PreprocessDirectory;

extern float					WaterThreshold;

extern bool						UseZoneSquare;
extern std::string				ZoneUL;
extern std::string				ZoneDR;

extern std::string				GlobalRetriever;
extern std::string				RetrieverBank;
extern std::string				GlobalUL;
extern std::string				GlobalDR;
extern bool						ProcessGlobal;
extern bool						Verbose;
extern bool						CheckConsistency;

extern CPrimChecker				PrimChecker;

std::string			getZoneNameById(uint16 id);
uint16				getZoneIdByName(std::string &name);
NLMISC::CAABBox		getZoneBBoxById(uint16 id);
uint16				getZoneIdByPos(NLMISC::CVector &pos);
NL3D::CMesh			*generateMeshFromBBox(const NLMISC::CAABBox &bbox, NLMISC::CRGBA color = NLMISC::CRGBA(255, 128, 0));

namespace NLPACS
{

class CSurfElement;
class CComputableSurfaceBorder;
class CComputableSurface;
class CPatchTessellation;
class CZoneTessellation;



/**/

const sint32	UnaffectedSurfaceId = -1;











/**
 * CSurfElement is an element of an iso-criteria surface. It is basically a CTriangle, and
 * contains the various criteria values such as incline class, landscape material ...
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CSurfElement
{
public:
	/**
	 *
	 */
	uint32							ElemId;

	/**
	 * The support of the surface element.
	 * The index to the 3 vertices of the triangle.
	 */
	uint32							Tri[3];

	/**
	 * The element normal vector
	 */
	NLMISC::CVector					Normal;

	/**
	 * The area of the element
	 */
	float							Area;

	/**
	 * The zone id
	 */
	uint16							ZoneId;

	/**
	 * The tessellation vertices
	 */
	std::vector<NLMISC::CVector>	*Vertices;


	/* Here the surface criteria.
	   Probably some normal quantization, material, flags ... */
	uint8							WaterShape;
	uint8							QuantHeight;

	uint32							ForceMerge;
	
	bool							ForceInvalid;
	bool							IsBorder;
	bool							IsValid;
	bool							IsMergable;
	bool							ClusterHint;
	bool							IsUnderWater;

	enum
	{
		NumNormalQuantas = 4,
		NumOrientationQuantas = 4
	};


	/** 
	 * The links to the neighboring elements.
	 * Each edge is related to the opposite vertex in the triangle */
	CSurfElement		*EdgeLinks[3];

	/**
	 * A flag for each edge, set if the edge has already been evaluated (in
	 * the surface border computation.
	 */
	bool				EdgeFlag[3];

	/**
	 * The Id of the surface container.
	 */
	sint32				SurfaceId;

public:
	/**
	 * Constructor.
	 * Creates a simple CSurfElement.
	 */
	CSurfElement()
	{
		ElemId = 0;
		EdgeLinks[0] = NULL;
		EdgeLinks[1] = NULL;
		EdgeLinks[2] = NULL;
		EdgeFlag[0] = false;
		EdgeFlag[1] = false;
		EdgeFlag[2] = false;
		SurfaceId = UnaffectedSurfaceId;
		IsBorder = false;
		IsValid = false;
		IsMergable = true;
		ClusterHint = false;
		ForceInvalid = false;
		IsUnderWater = false;
		WaterShape = 255;
		QuantHeight = 0;
		ForceMerge = 0;
		ZoneId = 0;
	}

	/// Computes the bbox of the surface element.
	NLMISC::CAABBox	getBBox() const;


	/**
	 * Computes the various criteria values (associated to quantas)
	 */
	void	computeQuantas(CZoneTessellation *zoneTessel);

	/**
	 * Removes properly all links to the CSurfElement.
	 */
	void	removeLinks()
	{
		uint	i, j;
		for (i=0; i<3; ++i)
		{
			if (EdgeLinks[i] != NULL)
				for (j=0; j<3; ++j)
					if (EdgeLinks[i]->EdgeLinks[j] == this)
						EdgeLinks[i]->EdgeLinks[j] = NULL;
			EdgeLinks[i] = NULL;
		}

	}

	/**
	 * Get zone Id on edge
	 */
	sint32	getZoneIdOnEdge(uint edge) const
	{
		return (EdgeLinks[edge] != NULL ? EdgeLinks[edge]->ZoneId : -1);
	}

	void	serial(NLMISC::IStream &f, std::vector<CSurfElement> &tessellation)
	{
		f.serial(ElemId);
		f.serial(Tri[0], Tri[1], Tri[2]);
		f.serial(Normal);
		f.serial(ZoneId);

		if (f.isReading())
		{
			sint32	s;
			uint	i;
			for (i=0; i<3; ++i)
			{
				f.serial(s);
				EdgeLinks[i] = (s >= 0 ? &tessellation[s] : NULL);
			}
		}
		else
		{
			sint32	s;
			uint	i;
			for (i=0; i<3; ++i)
			{
				s = (EdgeLinks[i] != NULL ? EdgeLinks[i]->ElemId : -1);
				f.serial(s);
			}
		}
	}
};







/**
 * CComputableSurfaceBorder separates geometrically 2 distinct CComputableSurface objects
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CComputableSurfaceBorder
{
public:
	std::vector<NLMISC::CVector>	Vertices;

	sint32							Left;
	sint32							Right;

	float							Length;

	sint8							Edge;

	bool							DontSmooth;

public:
	/// Constructor.
	CComputableSurfaceBorder(sint32 left = 0, sint32 right = 0, sint edge=-1) : Left(left), Right(right), Edge(edge), DontSmooth(false) {}

	/// Dump the vertices that constitue the border.
	void	dump();

	/// Smoothes the border (and so reduces the number of vertices).
	void	smooth(float val);

	/// Computes the length of the border
	void	computeLength()
	{
		sint	n;
		Length = 0.0;
		for (n=0; n<(sint)Vertices.size()-1; ++n)
		{
			Length += (Vertices[n+1]-Vertices[n]).norm();
		}
	}
};










/**
 * CComputableSurface is a compact connex set of CSurfElement.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CComputableSurface
{
public:

public:
	/// The Id of the surface
	sint32									SurfaceId;

	/// The references on the elements that belong to the surface
	std::vector<CSurfElement *>				Elements;

	/// The object that stores all the borders used in the computed area
	std::vector<CComputableSurfaceBorder>	*BorderKeeper;

	/// The border in the surface, by id
	std::vector<uint16>						BorderIds;

	bool									IsUnderWater;
	bool									ClusterHint;

	float									Area;
	float									WaterHeight;
	uint8									QuantHeight;

	/// The BBox of the whole zone (in which the surface should be contained.)
	NLMISC::CAABBox							BBox;

	/// The height storage quad tree
	CSurfaceQuadTree						HeightQuad;

	/// The center of the surface
	NLMISC::CVector							Center;

public:
	/**
	 * Constructor.
	 * Builds an empty surface.
	 */
	CComputableSurface() : SurfaceId(UnaffectedSurfaceId), BorderKeeper(NULL), ClusterHint(false)	{}

	/**
	 * Flood fills the surface elements to find iso-criteria surfaces.
	 * Every linked surface element which has the same quantas values and a surfaceid == -1
	 * are marked and recursively called.
	 */
	template<class A>
	void	floodFill(CSurfElement *first, sint32 surfId, const A &cmp, CZoneTessellation *zoneTessel)
	{
		if (Verbose)
		{
			nldebug("flood fill surface %d", surfId);
		}

		std::vector<CSurfElement *>	stack;
		sint					i;

		stack.push_back(first);
		first->SurfaceId = surfId;

		SurfaceId = surfId;
		ClusterHint = first->ClusterHint;
		QuantHeight = first->QuantHeight;
		uint	waterShape = first->WaterShape;

		IsUnderWater = first->IsUnderWater;

		//WaterHeight = IsUnderWater ? zoneTessel->WaterShapes[first->WaterShape].Vertices[0].z : 123456.0f;
		bool	tamere;
		WaterHeight = IsUnderWater ? PrimChecker.waterHeight(first->WaterShape, tamere)+WaterThreshold : 123456.0f;


		uint32	currentZoneId = first->ZoneId;

		Area = 0.0;

		while (!stack.empty())
		{
			CSurfElement	*pop = stack.back();
			stack.pop_back();
			Elements.push_back(pop);
			Area += pop->Area;

			for (i=0; i<3; ++i)
			{
				if (pop->EdgeLinks[i] != NULL && pop->EdgeLinks[i]->SurfaceId == UnaffectedSurfaceId && cmp.equal(first, pop->EdgeLinks[i]))
				{
					pop->EdgeLinks[i]->SurfaceId = SurfaceId;
					stack.push_back(pop->EdgeLinks[i]);
				}
			}
		}

		if (Verbose)
		{
			nldebug("%d elements added", Elements.size());
		}

		Center = NLMISC::CVector::Null;
		for (i=0; i<(sint)Elements.size(); ++i)
		{
			std::vector<NLMISC::CVector>	&vertices = *Elements[i]->Vertices;
			Center += (vertices[Elements[i]->Tri[0]]+vertices[Elements[i]->Tri[1]]+vertices[Elements[i]->Tri[2]]);
		}
		Center /= (float)(Elements.size()*3);
	}


	/// Builds the border of the CComputableSurface.
	void	buildBorders(CZoneTessellation *zoneTessel);

	/// Check Surface Consistency
	bool	checkConsistency();

private:
	void	followBorder(CZoneTessellation *zoneTessel, CSurfElement *first, uint edge, uint sens, std::vector<NLMISC::CVector> &vstore, bool &loop);
};














/**
 * CZoneTessellation is the whole tessellation of a given CZone.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CZoneTessellation
{
private:
	std::vector<CSurfElement>				_Tessellation;
	std::vector<NLMISC::CVector>			_Vertices;

protected:

	std::vector<uint16>						_ZoneIds;
	std::vector<const NL3D::CZone*>			_ZonePtrs;

public:
	class CMergeForceBox
	{
	public:
		NLMISC::CAABBox						MergeBox;
		uint32								MergeId;
		void	serial(NLMISC::IStream &f)		{ f.serial(MergeBox, MergeId); }
	};

public:
	/// The zone valid tessellation elements.
	std::vector<CSurfElement *>				Elements;

	///
	NLMISC::CAABBox							BBox;
	NLMISC::CAABBox							OriginalBBox;
	NLMISC::CAABBox							BestFittingBBox;
	// Yoyo: if zone is empty, we must not apply the Translation delta to zone
	bool									BestFittingBBoxSetuped;

	///
	NLMISC::CVector							Translation;

	///
	sint32									CentralZoneId;

	std::vector<NLMISC::CPolygon>			WaterShapes;
	NL3D::CQuadGrid<uint32>					WaterGrid;

	/** 
	 * The tessellation refinement. The size of the tessellation is equal to 2m/Refinement
	 * (say, for instance, a refinement of 2 means a 1m large tessellation.)
	 */
	sint16									Refinement;

	/**
	 * The surfaces composing the tessellation.
	 */
	std::vector<CComputableSurface>			Surfaces;
	std::vector<CComputableSurface>			ExtSurfaces;

	/**
	 * The borders for the whole CZone.
	 */
	std::vector<CComputableSurfaceBorder>	Borders;

	/**
	 * The box that force merge into surface
	 */
	std::vector<CMergeForceBox>				ForceMerge;

	/**
	 * Flags
	 */
	std::vector<uint8>						VerticesFlags;

public:
	/**
	 * Constructor
	 * Creates an empty tessellation.
	 */
	CZoneTessellation() {}

	/**
	 * Clear
	 */
	void	clear();

	/**
	 * Sets a zone tessellation up for building later.
	 */
	bool	setup(uint16 zoneId, sint16 refinement, const NLMISC::CVector &translation);

	/**
	 * Adds a zone light tessellation to the quad tree container.
	 */
	void	addToContainer(const NL3D::CZone &zone);
	NL3D::CMesh	*generateCollisionMesh();

	/**
	 * Builds the whole zone tessellation (with linkage) from the given zone.
	 */
	void	build();

	/**
	 * Sets the water polygons up.
	 */
	void	addWaterShape(const NLMISC::CPolygon &poly)
	{
		WaterShapes.push_back(poly);
	}

	/**
	 * Compile the whole zone tessellation and creates surfaces
	 */
	void	compile();

	/**
	 * Generates a CMesh from the tessellation.
	 */
	NL3D::CMesh	*generateMesh();

	/**
	 * Generates borders for the whole zone tessellation.
	 * \param smooth how much to smooth the borders
	 */
	void	generateBorders(float smooth);

	/**
	 *
	 */
	NLMISC::CAABBox	computeBBox() const;

	/**
	 * Save tessellation
	 */
	void	saveTessellation(NLMISC::COFile &output);

	/**
	 * Load tessellation
	 */
	void	loadTessellation(NLMISC::CIFile &input);

private:
	void	checkSameLandscapeHmBinds(const NL3D::CLandscape &landscape, const NL3D::CLandscape &landscapeNoHm);
};



class CSurfElemCompareSimple
{
public:

	bool	equal(const CSurfElement *a, const CSurfElement *b) const
	{
		return	a->IsValid == b->IsValid &&
				a->ForceInvalid == b->ForceInvalid;
	}
};

class CSurfElemCompareNormal
{
public:

	bool	equal(const CSurfElement *a, const CSurfElement *b) const
	{
		return	b->IsValid &&
				a->ClusterHint == b->ClusterHint &&
				a->ZoneId == b->ZoneId &&
				a->IsUnderWater == b->IsUnderWater &&
				a->WaterShape == b->WaterShape &&
				a->QuantHeight == b->QuantHeight;
	}
};


}; // NLPACS

#endif // NL_BUILD_SURF_H

/* End of build_surf.h */
