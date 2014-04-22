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

#ifndef NL_EXPORT_RADIAL_NORMAL_H
#define NL_EXPORT_RADIAL_NORMAL_H

#include "export_appdata.h"

class CExportNel;

class CRadialVertices
{
public:
	CRadialVertices ();
	void init (INode *node, Mesh *mesh, TimeValue time, Interface &ip, CExportNel &nelExport);
	bool isUsingRadialNormals (uint face);
	Point3 getLocalNormal (uint vertex, uint face);
private:
	uint32				_SmoothingGroupMask;
	NLMISC::CVector		_Pivot[NEL3D_RADIAL_NORMAL_COUNT];
	INode				*_NodePtr;
	Mesh				*_MeshPtr;
};

#endif // NL_EXPORT_RADIAL_NORMAL_H

