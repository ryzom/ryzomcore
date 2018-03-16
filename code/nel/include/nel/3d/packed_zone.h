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

#ifndef _PACKED_ZONE_H
#define _PACKED_ZONE_H

#include "nel/3d/scene_group.h"
#include "nel/3d/landscape.h"
#include "nel/3d/vertex_buffer.h"
//
#include "nel/misc/stream.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/vector.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/class_registry.h"
//
#include "nel/misc/array_2d.h"
#include "nel/3d/shape_info.h"
//
#include <vector>
#include <list>


namespace NLMISC
{
	class Cmatrix;
}


namespace NL3D
{

class IDriver;
class CMaterial;


class CPackedVertex
{
public:
	uint16 X, Y, Z;
public:
	void serial(NLMISC::IStream &f)
	{
		f.serial(X, Y, Z);
	}
};

inline bool operator==(const CPackedVertex &lhs, const CPackedVertex &rhs)
{
	return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}

class CPackedTri
{
public:
	uint32 V0, V1, V2;
public:
	void serial(NLMISC::IStream &f)
	{
		f.serial(V0, V1, V2);
	}
};

class CPackedTri16
{
public:
	uint16 V0, V1, V2;
public:
	void serial(NLMISC::IStream &f)
	{
		f.serial(V0, V1, V2);
	}
};

/** A packed zone
  * Compact representation of a zone with fixed tesseletion
  * for raytarcing tests.
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2005
  */
class CPackedZoneBase : public NLMISC::IStreamable, public NLMISC::CRefCount
{
public:
	NLMISC::CAABBox				Box;
	sint32					    ZoneX;
	sint32					    ZoneY;
public:
	virtual ~CPackedZoneBase() {}
	virtual void serial(NLMISC::IStream &f) = 0;
	// TMP For debug : render position covered by a frustum
	virtual void render(CVertexBuffer &vb, IDriver &drv, CMaterial &mat, CMaterial &wiredMaterial, const NLMISC::CMatrix &camMat, uint batchSize, const NLMISC::CVector localFrustCorners[8]) = 0;
	// raytracing test
	virtual bool raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<NLMISC::CTriangle> *testedTriangles = NULL, NLMISC::CVector *normal = NULL) const = 0;
	// roughly select triangle inside 'poly' (all triangle that are at least partially  inside are guaranteed to be selected)
	virtual void appendSelection(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const = 0;
};


typedef NLMISC::CSmartPtr<CPackedZoneBase> TPackedZoneBaseSPtr;


class CPackedZone16;


class CPackedZone32 : public CPackedZoneBase
{
public:
	typedef uint32				TIndexType;
	static uint32				UndefIndex; // used as value for undefined index
	std::vector<CPackedVertex>  Verts;
	std::vector<CPackedTri>		Tris;
	std::vector<uint32>			TriLists; // lists of tri for each grid cell, all list are packed in a single vector. std::numeric_limits<uint32>::max() marks the end of a list
	NLMISC::CArray2D<uint32>	Grid;     // grid of entries into TriLists (one list for each cell)
	float						CellSize;
public:
	NLMISC_DECLARE_CLASS(CPackedZone32)
	CPackedZone32();
	void build(std::vector<const CTessFace *> &tessFaces,
		       float cellSize,
			   std::vector<CInstanceGroup *> igs,
			   const TShapeCache &shapeCache,
			   const NLMISC::CAABBox &baseZoneBBox,
			   sint32	zoneX,
			   sint32	zoneY
			  );
	void serial(NLMISC::IStream &f);
	// TMP For debug : render porition covered by a frustum
	void render(CVertexBuffer &vb, IDriver &drv, CMaterial &mat, CMaterial &wiredMaterial, const NLMISC::CMatrix &camMat, uint batchSize, const NLMISC::CVector localFrustCorners[8]);
	// try to build a 16 bit version of this packed zone to save some more place
	NLMISC::CSmartPtr<CPackedZone16> buildPackedZone16();
	// raytracing test
	virtual bool raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<NLMISC::CTriangle> *testedTriangles = NULL, NLMISC::CVector *normal = NULL) const;
	//
	void appendSelection(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const;
private:
	// for fast conversion
	NLMISC::CVector			_Origin;
	NLMISC::CVector			_WorldToLocal;
	NLMISC::CVector			_PackedLocalToWorld;
	//
	typedef NLMISC::CArray2D<std::list<uint32> > TVertexGrid;  // grid used during the build to enable vertices sharing
	typedef NLMISC::CArray2D<std::list<uint32> > TTriListGrid; // list of tri indices for each grid cell, before these lists are packed in a single vector (in 'TriLists')

private:
	void    addTri(const NLMISC::CTriangle &tri, TVertexGrid &vertexGrid, TTriListGrid &triListGrid);
	// alloc a packed vertex index, building a new vertex if necessary
	uint32  allocVertex(const NLMISC::CVector &src, TVertexGrid &vertexGrid);
	// add tris from an an instance
	void    addInstance(const CShapeInfo &si, const NLMISC::CMatrix &matrix, TVertexGrid &vertexGrid, TTriListGrid &triListGrid);
public:
	// PRIVATE : unpack a packed tri
	void	unpackTri(const CPackedTri &src, NLMISC::CVector dest[3]) const;
};


class CPackedZone16 : public CPackedZoneBase
{
public:
	typedef uint16				TIndexType;
	static uint16				UndefIndex; // used as value for undefined index
	friend class CPackedZone32;
	std::vector<CPackedVertex>  Verts;
	std::vector<CPackedTri16>	Tris;
	std::vector<uint16>			TriLists; // lists of tri for each grid cell, all lists are packed in a single vector. std::numeric_limits<uint16>::max() marks the end of a list
	NLMISC::CArray2D<uint16>	Grid;     // grid of entries into TriLists (one list for each cell)
	float						CellSize;
public:
	NLMISC_DECLARE_CLASS(CPackedZone16)
	CPackedZone16();
	//
	void serial(NLMISC::IStream &f);
	// TMP For debug : render position covered by a frustum
	void render(CVertexBuffer &vb, IDriver &drv, CMaterial &mat, CMaterial &wiredMaterial, const NLMISC::CMatrix &camMat, uint batchSize, const NLMISC::CVector localFrustCorners[8]);
		// raytracing test
	virtual bool raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<NLMISC::CTriangle> *testedTriangles = NULL, NLMISC::CVector *normal = NULL) const;
	//
	void appendSelection(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const;
private:
	// for fast conversion
	NLMISC::CVector			_Origin;
	NLMISC::CVector			_WorldToLocal;
	NLMISC::CVector			_PackedLocalToWorld;
public:
	// PRIVATE : unpack a packed tri
	void	unpackTri(const CPackedTri16 &src, NLMISC::CVector dest[3]) const;
};

} // NL3D

#endif
