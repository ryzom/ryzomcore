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

// Skeleton Weight Template exportation

#include "std_afx.h"
#include "nel_export.h"
#include "resource.h"
#include "nel/3d/skeleton_weight.h"
#include "nel/misc/file.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_appdata.h"

using namespace NL3D;
using namespace NLMISC;

bool CNelExport::exportSWT(const char *sPath, std::vector<INode*>& vectNode)
{
	float rPosValue;
	float rRotValue;
	float rScaleValue;
	CSkeletonWeight::TNodeArray aSWNodes; // Array of Skeleton Weight Node
	int nNumNode = 0;

	aSWNodes.empty();

	// Build the array of node
	std::vector<INode*>::iterator it = vectNode.begin();

	for(int i=0; i<(int)vectNode.size(); ++i,++it)
	{
		// Get the SWT Modifier
		INode *pNode = *it;

		// SWT active ?
		if (CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_SWT, BST_UNCHECKED) != BST_UNCHECKED)
		{
			// Get the value
			rPosValue = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, 0.f);
			rRotValue = rPosValue;
			rScaleValue = rPosValue;

			// Store them in the temporary list
			aSWNodes.resize(nNumNode+3);
			aSWNodes[nNumNode].Name = pNode->GetName();
			aSWNodes[nNumNode].Name += std::string (".")+ITransformable::getRotQuatValueName();
			aSWNodes[nNumNode].Weight = rRotValue;
			++nNumNode;
			aSWNodes[nNumNode].Name = pNode->GetName();
			aSWNodes[nNumNode].Name += std::string (".")+ITransformable::getPosValueName ();
			aSWNodes[nNumNode].Weight = rPosValue;
			++nNumNode;
			aSWNodes[nNumNode].Name = pNode->GetName();
			aSWNodes[nNumNode].Name += std::string (".")+ITransformable::getScaleValueName();
			aSWNodes[nNumNode].Weight = rScaleValue;
			++nNumNode;
		}
	}

	if (aSWNodes.size())
	{
		CSkeletonWeight sw;
		COFile file;
		
		sw.build( aSWNodes );

		if (file.open (sPath))
		{
			try
			{							
				// Serial the skeleton
				sw.serial (file);
				// All is good
				return true;
			}
			catch (Exception &e)
			{
				nlwarning (e.what());
			}
		}
	}
	else
	{
		// No node found with a SWT Modifier
		nlwarning ("No node found with a SWT Modifier");
	}
	return false;
}