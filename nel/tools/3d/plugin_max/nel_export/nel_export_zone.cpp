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

#include "std_afx.h"
#include "nel_export.h"
#include "nel/3d/zone.h"
#include "nel/3d/zone_symmetrisation.h"
#include "../nel_patch_lib/rpo.h"

using namespace NL3D;
using namespace NLMISC;

// --------------------------------------------------

bool CNelExport::exportZone (const std::string &sPath, INode& node, TimeValue time)
{
	// Result to return
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Object can convert itself to NeL patchmesh ?
		RPO* pPatchObject = (RPO*) os.obj->ConvertToType(time, RYKOLPATCHOBJ_CLASS_ID);

		// Conversion success ?
		if (pPatchObject)
		{
			// Build the zone
			CZone zone;
			CZoneSymmetrisation zoneSymmetry;
			if (pPatchObject->rpatch->exportZone (&node, &pPatchObject->patch, zone, zoneSymmetry, 0, 160, 1, false))
			{
				// Open a file
				COFile file;
				if (file.open (sPath))
				{
					try
					{
						// Serial the zone
						zone.serial (file);

						// All is good
						bRet=true;
					}
					catch (...)
					{
					}
				}
				file.close ();
			}

#ifdef NL_DEBUG
			// load the zone
			CZone checkZone;
			CIFile inputFile;
			if (inputFile.open (sPath))
			{
				checkZone.serial (inputFile);

				// Check zone
				std::vector<CPatchInfo> patchs;
				std::vector<CBorderVertex> borderVertices;
				checkZone.retrieve(patchs, borderVertices);

				// Get Center and scal
				float fScale=checkZone.getPatchScale();
				CVector vCenter=checkZone.getPatchBias();

				// Watch points
				for (sint nPatch=0; nPatch<(sint)patchs.size(); nPatch++)
				{
					for (sint nVert=0; nVert<4; nVert++)
					{
						CVector v=patchs[nPatch].Patch.Vertices[nVert];
					}
				}
			}
#endif // NL_DEBUG


		}
	}
	return bRet;
}



