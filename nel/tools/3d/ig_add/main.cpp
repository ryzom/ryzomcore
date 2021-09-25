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

#include <iostream>
#include <vector>

#include "nel/misc/file.h"

#include "nel/3d/scene_group.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// ***************************************************************************
CInstanceGroup* LoadInstanceGroup (const char* sFilename)
{
	CIFile file;
	CInstanceGroup *newIG = new CInstanceGroup;

	if( file.open( sFilename ) )
	{
		try
		{
			newIG->serial (file);
		}
		catch (const Exception &)
		{
			delete newIG;
			return NULL;
		}
	}
	else
	{
		delete newIG;
		return NULL;
	}
	return newIG;
}

// ***************************************************************************
bool SaveInstanceGroup (const char* sFilename, CInstanceGroup *pIG)
{
	COFile file;

	if( file.open( sFilename ) )
	{
		try
		{
			pIG->serial (file);
		}
		catch (const Exception &)
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

// ***************************************************************************
int main(int nNbArg, char**ppArgs)
{

	if (nNbArg != 4)
	{
		// Display help information
		cout << "USAGE : ig_add out in1 in2" << endl;
		cout << "Make out = in1 + in2 (out, in1 and in2 are *.IG files" << endl;
		return 1;
	}

	// Load in1 and in2
	// ****************
	CInstanceGroup *pIG1 = LoadInstanceGroup (ppArgs[2]);
	if (pIG1 == NULL)
	{
		cerr << "File " << ppArgs[2] << " not found" << endl;
		return -1;
	}
	CInstanceGroup *pIG2 = LoadInstanceGroup (ppArgs[3]);
	if (pIG2 == NULL)
	{
		cerr << "File " << ppArgs[3] << " not found" << endl;
		delete pIG1;
		return -1;
	}

	// Add in1 and in2
	// ***************
	CInstanceGroup *pIGout = new CInstanceGroup;

	CVector vGP1, vGP2, vGPout;
	CInstanceGroup::TInstanceArray IA1, IA2, IAout;
	vector<CCluster> Clusters1, Clusters2, Clustersout;
	vector<CPortal> Portals1, Portals2, Portalsout;
	vector<CPointLightNamed> PLN1, PLN2, PLNout;

	pIG1->retrieve (vGP1, IA1, Clusters1, Portals1, PLN1);
	pIG2->retrieve (vGP2, IA2, Clusters2, Portals2, PLN2);

	uint32 i;

	// -----------------------------------------------------------------------
	vGPout = (vGP1+vGP2) / 2.0f;

	// -----------------------------------------------------------------------
	for (i = 0; i < IA1.size(); ++i)
	{
		CInstanceGroup::CInstance iTmp = IA1[i];
		iTmp.Pos += (vGP1 - vGPout);
		IAout.push_back(iTmp);
	}
	for (i = 0; i < IA2.size(); ++i)
	{
		CInstanceGroup::CInstance iTmp = IA2[i];
		iTmp.Pos += (vGP2 - vGPout);
		IAout.push_back(iTmp);
	}

	// -----------------------------------------------------------------------
	Clustersout = Clusters1;
	for (i = 0; i < Clusters2.size(); ++i)
		Clustersout.push_back(Clusters2[i]);

	// -----------------------------------------------------------------------
	Portalsout = Portals1;
	for (i = 0; i < Portals2.size(); ++i)
		Portalsout.push_back(Portals2[i]);

	// -----------------------------------------------------------------------
	PLNout = PLN1;
	for (i = 0; i < PLN2.size(); ++i)
		PLNout.push_back(PLN2[i]);


	pIGout->build (vGPout, IAout, Clustersout, Portalsout, PLNout);

	// Save out
	// ********
	if (!SaveInstanceGroup (ppArgs[1], pIGout))
	{
		cerr << "Cannot save to file " << ppArgs[1] << endl;

		// Delete the ig
		delete pIG1;
		delete pIG2;
		delete pIGout;

		return -1;
	}

	// Delete the ig
	delete pIG1;
	delete pIG2;
	delete pIGout;

	return 1;
}

// NOTES : For the moment the behaviour of the build with cluster system is 
// strange because the instance of the instance group are not relinked to
// all the clusters... But this is not a big deal because we just want to
// add group of instances without any cluster system
