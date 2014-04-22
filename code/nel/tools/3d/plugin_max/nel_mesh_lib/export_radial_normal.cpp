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

#include "stdafx.h"
#include "export_radial_normal.h"
#include "export_nel.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************

CRadialVertices::CRadialVertices ()
{
	_NodePtr = NULL;
	_MeshPtr = NULL;
}

// ***************************************************************************

void CRadialVertices::init (INode *node, Mesh *mesh, TimeValue time, Interface &ip, CExportNel &nelExport)
{
	// Reset the mask
	_SmoothingGroupMask = 0;

	// Set the pointer
	_NodePtr = node;
	_MeshPtr = mesh;

	// Get the node object matrix
	Matrix3 objectMatrix = _NodePtr->GetObjectTM (time);

	// Get a NeL matrix world to object space
	CMatrix nelMatrix;
	CExportNel::convertMatrix (nelMatrix, objectMatrix);
	nelMatrix.invert ();

	// For each appdata
	uint app;
	for (app=NEL3D_RADIAL_FIRST_SM; app<NEL3D_RADIAL_FIRST_SM+NEL3D_RADIAL_NORMAL_COUNT; app++)
	{
		// Get the appvalue
		string pivotName = CExportNel::getScriptAppData (_NodePtr, NEL3D_APPDATA_RADIAL_NORMAL_SM+app-NEL3D_RADIAL_FIRST_SM, "");

		// Active ?
		if (pivotName != "")
		{
			// Add the mask
			_SmoothingGroupMask |= (1<<app);

			// Get the node by name
			INode *pivotNode = ip.GetINodeByName(pivotName.c_str());
			if (pivotNode)
			{
				// Get the world Pivot point
				Point3 pivot = pivotNode->GetNodeTM (time).GetTrans ();

				// Nel vector
				CVector &nelPivot = _Pivot[app-NEL3D_RADIAL_FIRST_SM];
				nelPivot.set (pivot.x, pivot.y, pivot.z);

				// Object space pivot
				nelPivot = nelMatrix * nelPivot;
			}
			else
			{
				// Output error message
				char msg[512];
				smprintf (msg, 512, "Can't find pivot node named %s", pivotName);
				nelExport.outputErrorMessage (msg);
			}
		}
	}
}

// ***************************************************************************

bool CRadialVertices::isUsingRadialNormals (uint face)
{
	if (_MeshPtr)
	{
		// Get the smoothing group for this face
		uint32 sm = (uint32)_MeshPtr->faces[face].smGroup;

		// Does it use a radial normal smoothing group ?
		return (sm & _SmoothingGroupMask) != 0;
	}
	else
	{
		nlwarning ("CRadialVertices not initialized");
		return false;
	}
}

// ***************************************************************************

Point3 CRadialVertices::getLocalNormal (uint vertex, uint face)
{
	if (_MeshPtr)
	{
		// Result
		Point3 result (0, 0, 0);

		// For each smoothing group
		uint app;
		for (app=NEL3D_RADIAL_FIRST_SM; app<NEL3D_RADIAL_FIRST_SM+NEL3D_RADIAL_NORMAL_COUNT; app++)
		{
			// This smoothing group use radial normal ?
			if (_SmoothingGroupMask & (1<<app))
			{
				// Does this face belong to this smoothing group ?
				if ((uint32)_MeshPtr->faces[face].smGroup & (1<<app))
				{
					// Get the object vertex
					CVector vertex (_MeshPtr->verts[vertex].x, _MeshPtr->verts[vertex].y, _MeshPtr->verts[vertex].z);

					// Compute a normal
					vertex -= _Pivot[app-NEL3D_RADIAL_FIRST_SM];
					vertex.normalize ();

					// Add the vector
					result += Point3 (vertex.x, vertex.y, vertex.z);
				}
			}
		}

		// Return a normalize the result
		if (Length (result) > 0)
			return Normalize (result);
		else
			nlwarning ("No radial normal found for this vertex");
	}
	else
	{
		nlwarning ("CRadialVertices not initialized");
	}
	return Point3 (0,0,0);
}

// ***************************************************************************
