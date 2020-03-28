// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// Scene Exportation

// A scene is made of virtual object instance
// An object instance is 
// - A reference to a mesh (refered by the name of the node)
// - The transformations to get it to the world
// - The parent


#include "std_afx.h"
#include "nel_export.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_patch_lib/rpo.h"
#include "nel/3d/scene_group.h"

#include <vector>

using namespace std;
using namespace NL3D;
using namespace NLMISC;

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
bool CNelExport::exportInstanceGroup(string filename, vector<INode*>& vectNode)
{
	vector<INode*> resultInstanceNode;
	CInstanceGroup *pIG = _ExportNel->buildInstanceGroup (vectNode, resultInstanceNode, _Ip->GetTime());

	if (pIG != NULL)
	{
		COFile file;
		
		if (file.open (filename))
		{
			try
			{
				// Serial the skeleton
				pIG->serial (file);
				// All is good
			}
			catch (const Exception &c)
			{
				// Cannot save the file
				MessageBox(NULL, MaxTStrFromUtf8(c.what()).data(), _T("NeL export"), MB_OK|MB_ICONEXCLAMATION);
				return false;
			}
		}

		delete	pIG;
	}
	else
	{
		// No node found with a SWT Modifier
	}
	return true;
}
