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

#ifndef NL_SHAPE_INFO_H
#define NL_SHAPE_INFO_H


#include "nel/misc/triangle.h"
#include "nel/misc/aabbox.h"
//
#include <vector>


namespace NL3D
{

class CMeshGeom;
class CMeshMRMGeom;
class IShape;
class CMeshBase;

// Mesh or mrm mesh as a bunch of triangles
class CShapeInfo
{
public:
	NLMISC::CAABBox					LocalBBox;
	std::vector<NLMISC::CTriangle>	Tris;
public:
	void build(const NL3D::IShape &shape);
	void swap(CShapeInfo &other);
private:
	void build(const NL3D::CMeshBase &meshBase, const NL3D::CMeshGeom &meshGeom);
	void build(const NL3D::CMeshBase &meshBase, const NL3D::CMeshMRMGeom &meshGeom);
};

// a shape cache, sorted by names
typedef std::map<std::string, CShapeInfo> TShapeCache;

// format a shape name to have lowercase + extension, for lookup in shape cache
std::string standardizeShapeName(const std::string &name);

} // NL3D


#endif
