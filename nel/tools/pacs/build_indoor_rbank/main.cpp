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


#include <vector>

#include <stdlib.h>

#include "nel/misc/config_file.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"

#include "nel/pacs/collision_mesh_build.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/retriever_instance.h"
#include "nel/pacs/global_retriever.h"
#include "nel/pacs/retriever_bank.h"

#include "nel/pacs/u_global_position.h"

#include "mouline.h"

using namespace std;
using namespace NLPACS;
using namespace NLMISC;

#ifndef NL_BIRB_CFG
#define NL_BIRB_CFG "."
#endif // NL_BIB_CFG

bool			AddToRetriever = true;
bool			Merge = true;
string			MergePath = "c:/data_test/";
string			MergeInputPrefix = "fyros";
string			MergeOutputPrefix = "city_landscape_fyros";
string			MeshPath = "c:/data_test/";
vector<string>	Meshes;
string			OutputPath = "c:/data_test/";
string			OutputPrefix = "city_fyros";


CVector	getZoneCenterById(uint16 id)
{
	CAABBox		bbox;
	uint		x, y;
	const float	zdim = 160.0f;

	x = id%256;
	y = id/256;

	return CVector(zdim*((float)x+0.5f), -zdim*((float)y+0.5f), 0.0f);
}

uint32	getIdByCoord(uint x, uint y)
{
	return y*256+x;
}

uint16 getZoneIdByName(string &name)
{
	sint		y = 0, x = 0;
	const char	*str = name.c_str();

	while (*str != '_')
		y = y*10 + *(str++)-'0';

	++str;

	x = (str[0]-'A')*26+(str[1]-'A');

	return (y-1)*256+x;
}

template<class A>
void serialAndSave(A &a, string fn)
{
	COFile	f;
	f.open(fn);
	f.serial(a);
	f.close();
}

template<class A>
void openAndSerial(A &a, string fn)
{
	CIFile	f;
	f.open(fn);
	f.serial(a);
	f.close();
}



// config management
int		getInt(CConfigFile &cf, const string &varName)
{
	CConfigFile::CVar &var = cf.getVar(varName);
	return var.asInt();
}

string	getString(CConfigFile &cf, const string &varName)
{
	CConfigFile::CVar &var = cf.getVar(varName);
	return var.asString();
}

void	initConfig()
{
	try
	{
#ifdef NL_OS_UNIX
		NLMISC::CPath::addSearchPath(NLMISC::CPath::getApplicationDirectory("NeL"));
#endif // NL_OS_UNIX

		NLMISC::CPath::addSearchPath(NL_BIRB_CFG);

		CConfigFile cf;
	
		cf.load("build_indoor_rbank.cfg");
	
		Merge = getInt(cf, "Merge") != 0;
		AddToRetriever = getInt(cf, "AddToRetriever") != 0;
		MergePath = getString(cf, "MergePath");
		MergeInputPrefix = getString(cf, "MergeInputPrefix");
		MergeOutputPrefix = getString(cf, "MergeOutputPrefix");
		MeshPath = getString(cf, "MeshPath");

		OutputPath = getString(cf, "OutputPath");
		OutputPrefix = getString(cf, "OutputPrefix");

		CConfigFile::CVar	&meshes = cf.getVar("Meshes");
		uint				i;
		for (i=0; i<(uint)meshes.size(); i++)
			Meshes.push_back(meshes.asString(i));
	}
	catch (const EConfigFile &e)
	{
		printf ("Problem in config file : %s\n", e.what ());
	}
}



//
void makeGlobalRetriever(vector<CVector> &translation)
{
	CGlobalRetriever	gr;
	CRetrieverBank		rb;

	if (Merge)
	{
		openAndSerial(rb, MergePath+MergeInputPrefix+".rbank");
		gr.setRetrieverBank(&rb);
		openAndSerial(gr, MergePath+MergeInputPrefix+".gr");
	}
	else
	{
		gr.setRetrieverBank(&rb);
		gr.init();
	}

	TTime		start, end;
	
	start = CTime::getLocalTime();

	vector<uint>	ninst;

	uint	i;
	for (i=0; i<Meshes.size(); ++i)
	{
		CLocalRetriever		lr;
		try
		{
			openAndSerial(lr, OutputPath+Meshes[i]+".lr");

			uint	rid = rb.addRetriever(lr);

			if (AddToRetriever)
			{
				uint	iid = (gr.makeInstance(rid, 0, -translation[i])).getInstanceId();

				ninst.push_back(iid);
			}
		}
		catch (const Exception &e)
		{
			nlwarning("WARNING: can't merge lr '%s.lr': %s", Meshes[i].c_str(), e.what());
		}
	}

	gr.initQuadGrid();

	for (i=0; i<ninst.size(); ++i)
		gr.makeLinks(ninst[i]);


	end = CTime::getLocalTime();
	nlinfo("instance making and linking work time: %.3f", (double)(end-start)/1000.0);

	COFile	output;

	/*
	 * This way, at the end of the build process, 
	 * the rbank should be 5 bytes only and lr should be saved as well next to the rbank
	 */

	if (Merge)
	{
		serialAndSave(gr, OutputPath+MergeOutputPrefix+".gr");
		rb.saveShortBank(OutputPath, MergeOutputPrefix, true);	// save 5 bytes rbank and lr as well

//		serialAndSave(rb, OutputPath+MergeOutputPrefix+".rbank");
	}
	else
	{
		serialAndSave(gr, OutputPath+OutputPrefix+".gr");
		rb.saveShortBank(OutputPath, OutputPrefix, true);		// save 5 bytes rbank and lr as well

//		serialAndSave(rb, OutputPath+OutputPrefix+".rbank");
	}

	gr.check();

/*
	UGlobalPosition	gpos;

	gpos = gr.retrievePosition(CVector(18630, -24604, 0));
	const CLocalRetriever	&lr = gr.getRetriever(gr.getInstance(gpos.InstanceId).getRetrieverId());
	lr.dumpSurface(gpos.LocalPosition.Surface, gr.getInstance(gpos.InstanceId).getOrigin());
*/
}

void createRetriever(vector<CVector> &translation)
{
	uint	i;

	translation.resize(Meshes.size());

	for (i=0; i<Meshes.size(); ++i)
	{
		CLocalRetriever			lr;
		CCollisionMeshBuild		cmb;

		string					meshName = Meshes[i];

		nlinfo("compute retriever %s", meshName.c_str());

		try
		{
			openAndSerial(cmb, MeshPath+meshName+".cmb");

			{
				vector<bool>	usedMaterials;
				uint			j;
				uint			maxMat = 0;

				for (j=0; j<cmb.Faces.size(); ++j)
					if (cmb.Faces[j].Material > (sint)maxMat)
						maxMat = cmb.Faces[j].Material;

				usedMaterials.resize(maxMat+1);

				for (j=0; j<usedMaterials.size(); ++j)
					usedMaterials[j] = false;
				for (j=0; j<cmb.Faces.size(); ++j)
					usedMaterials[cmb.Faces[j].Material] = true;

				for (j=0; j<usedMaterials.size(); ++j)
					if (usedMaterials[j])
						nlinfo("Material %d used", j);
			}


			computeRetriever(cmb, lr, translation[i], true);
/*
			// TEMP
			uint	tt;
			for (tt=0; tt<lr.getSurfaces().size(); ++tt)
				lr.dumpSurface(tt);
			//
*/
			
			// Compute an identifier name
			string indentifier = meshName;
			string::size_type sharpPos = indentifier.rfind ('#');
			if (sharpPos != string::npos)
				indentifier = indentifier.substr (0, sharpPos);
			lr.setIdentifier(indentifier);

			// Save the lr file
			serialAndSave(lr, OutputPath+meshName+".lr");
		}
		catch (const Exception &e)
		{
			nlwarning("WARNING: can compute lr '%s.lr': %s", meshName.c_str(), e.what());
		}
	}
}


// pacs test

void	serialGPos(UGlobalPosition &gp, NLMISC::IStream &f)
{
	f.serial(gp.InstanceId, gp.LocalPosition.Surface, gp.LocalPosition.Estimation);
}

struct CGlobalPositionCompare
{
	UGlobalPosition		ClientPosition,
						RetrievedPosition;
	bool				Compare;

	void				serial(NLMISC::IStream &f)
	{
		serialGPos(ClientPosition, f);
		serialGPos(RetrievedPosition, f);
		f.serial(Compare);
	}
};

vector<CGlobalPositionCompare>	StoredPosition;


void	loadAndDumpPositions()
{
	CIFile	f;

	f.open("backup.position");
	f.serialCont(StoredPosition);
	f.close();

	uint	i;
	for (i=0; i<StoredPosition.size(); ++i)
	{
		CGlobalPositionCompare	&c = StoredPosition[i];
		nlinfo("cmp=%s cl(%4d,%4d,%.8f,%.8f,%.8f) gr(%4d,%4d,%.8f,%.8f,%.8f)",
				c.Compare ? "true" : "false",
				c.ClientPosition.InstanceId, c.ClientPosition.LocalPosition.Surface, c.ClientPosition.LocalPosition.Estimation.x, c.ClientPosition.LocalPosition.Estimation.y, c.ClientPosition.LocalPosition.Estimation.z,
				c.RetrievedPosition.InstanceId, c.RetrievedPosition.LocalPosition.Surface, c.RetrievedPosition.LocalPosition.Estimation.x, c.RetrievedPosition.LocalPosition.Estimation.y, c.RetrievedPosition.LocalPosition.Estimation.z);
	}
}

//

/*
int main(int argc, char **argv)
{
	CRetrieverBank	bank;
	CIFile			f("R:/code/ryzom/data/3d/continents/matis/pacs/matis.rbank");

	f.serial(bank);

	bank.saveShortBank("R:/code/ryzom/data/3d/continents/matis/pacs", "matis");
}
*/


int main(int argc, char **argv)
{
	vector<CVector>	translation;
	TTime			start, end;

	createDebug ();
	ErrorLog->removeDisplayer("DEFAULT_MBD");

	initConfig();

	start = CTime::getLocalTime();
	createRetriever(translation);

//	translation[0] = CVector(-4670.0f, 3579.0f, 0.0f);

	makeGlobalRetriever(translation);
	end = CTime::getLocalTime();

	nlinfo("%.3f seconds work", (double)(end-start)/1000.0);

	uint	i;
	for (i=0; i<translation.size(); ++i)
		nlinfo("CVector\ttranslation_%d = CVector(%.1ff, %.1ff, %.1ff);", i, translation[i].x, translation[i].y, translation[i].z);

	return 0;
}



