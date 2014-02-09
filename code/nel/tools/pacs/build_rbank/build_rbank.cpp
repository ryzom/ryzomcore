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


#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/polygon.h"
#include "nel/misc/smart_ptr.h"

#include "nel/3d/scene_group.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/water_model.h"
#include "nel/3d/water_shape.h"
#include "nel/3d/quad_grid.h"

#include "build_rbank.h"
#include "build_surf.h"

#include "surface_splitter.h"

#include "nel/pacs/global_retriever.h"
#include "nel/pacs/retriever_bank.h"
#include "nel/pacs/surface_quad.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/retriever_instance.h"
#include "nel/pacs/chain.h"
#include "nel/pacs/collision_mesh_build.h"

#include <string>
#include <deque>
#include <map>

using namespace std;
using namespace NLMISC;
using namespace NL3D;


class CIGBox
{
public:
	CIGBox() {}
	CIGBox(const string &name, const CAABBox &bbox) : Name(name), BBox(bbox) {}
	string			Name;
	CAABBox			BBox;
	void			serial(NLMISC::IStream &f) { f.serial(Name, BBox); }
};

/*
string getZoneNameById(uint16 id)
{
	uint	x = id%256;
	uint	y = id/256;

	char ych[32];
	sprintf(ych,"%d_%c%c", y+1, 'A'+x/26, 'A'+x%26);
	return string(ych);
}
*/
string getZoneNameByCoord(float x, float y)
{
	const float zoneDim = 160.0f;

	float xcount = x/zoneDim;
	float ycount = -y/zoneDim + 1;

	char ych[32];
	sprintf(ych,"%d_%c%c",(sint)ycount, 'A'+(sint)xcount/26, 'A'+(sint)xcount%26);
	return string(ych);
}

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

string	changeExt(string name, const string &ext)
{
	string::iterator	it, last;
	last = name.end();

	for (it=name.begin(); it!=name.end(); ++it)
		if (*it == '.')
			last = it;

	name.erase(last, name.end());
	name.append(".");
	name.append(ext);
	return name;
}



void processAllPasses(string &zoneName)
{
	uint	/*i,*/ j;

	NLPACS::CZoneTessellation		tessellation;
	vector<NLPACS::COrderedChain3f>	fullChains;
	string							name;
	string							filename;

	try
	{
		uint16	zid = getZoneIdByName(zoneName);
		CAABBox	box = getZoneBBoxById(zid);

		CVector		translation = -box.getCenter();
		if (tessellation.setup(zid, 4, translation))
		{
			tessellation.build();

			CAABBox	tbox = tessellation.computeBBox();

			tessellation.compile();
			tessellation.generateBorders(1.0);

			NLPACS::CLocalRetriever	retriever;

			CAABBox	rbbox = tessellation.BestFittingBBox;
			CVector hs = rbbox.getHalfSize();
			hs.z = 10000.0f;
			rbbox.setHalfSize(hs);
			retriever.setBBox(rbbox);
			retriever.setType(NLPACS::CLocalRetriever::Landscape);

			for (j=0; j<tessellation.Surfaces.size(); ++j)
			{
				retriever.addSurface(0,
									 0,
									 0,
									 0,
									 0,
									 tessellation.Surfaces[j].IsUnderWater,
									 tessellation.Surfaces[j].WaterHeight,
									 tessellation.Surfaces[j].ClusterHint,
									 tessellation.Surfaces[j].Center,
									 tessellation.Surfaces[j].HeightQuad,
									 tessellation.Surfaces[j].QuantHeight);

				if (Verbose)
				{
					nlinfo("Added surface %d: water=%d", j, (tessellation.Surfaces[j].IsUnderWater ? 1 : 0));
				}
			}

			for (j=0; j<tessellation.Borders.size(); ++j)
			{
				if (tessellation.Borders[j].Right < -1)
				{
					retriever.addChain(tessellation.Borders[j].Vertices,
									   tessellation.Borders[j].Left,
									   NLPACS::CChain::getDummyBorderChainId());

				}
				else
				{
					retriever.addChain(tessellation.Borders[j].Vertices,
									   tessellation.Borders[j].Left,
									   tessellation.Borders[j].Right);
				}
			}

			retriever.computeLoopsAndTips();

			retriever.findBorderChains();
			retriever.updateChainIds();
			retriever.computeTopologies();

			retriever.computeCollisionChainQuad();

			retriever.setType(NLPACS::CLocalRetriever::Landscape);

			// and save it...

			if (!retriever.checkSurfacesIntegrity(translation))
			{
				nlwarning("retriever '%s' has a surface issue (self covering surface...)", zoneName.c_str());
			}

			COFile	outputRetriever;
			name = changeExt(zoneName, string("lr"));
			filename = OutputPath+name;
			if (Verbose)
				nlinfo("save file %s", filename.c_str());
			outputRetriever.open(filename);
			retriever.serial(outputRetriever);
		}
	}
	catch(const Exception &e)
	{
		printf("%s\n", e.what ());
	}
}


//
//
//
//
//
//

/*
void tessellateAndMoulineZone(string &zoneName)
{
	uint	i, j;

	NLPACS::CZoneTessellation		tessellation;
	vector<NLPACS::COrderedChain3f>	fullChains;
	string							name;
	string							filename;

	try
	{
		uint16	zid = getZoneIdByName(zoneName);
		CAABBox	box = getZoneBBoxById(zid);

		CVector		translation = -box.getCenter();
		if (tessellation.setup(zid, 4, translation))
		{
			tessellation.build();

			CAABBox	tbox = tessellation.computeBBox();

			vector<CIGBox>				boxes;
			try
			{
				if (CFile::fileExists (IGBoxes))
				{
					CIFile		binput(IGBoxes);
					binput.serialCont(boxes);
				}
				else
				{
					nlinfo("WARNING: IG list no found");
				}
			}
			catch (const Exception &) { nlinfo("WARNING: IG list no found"); }

			for (i=0; i<boxes.size(); ++i)
			{
				if (tbox.intersect(boxes[i].BBox))
				{
					try
					{
						// load ig associated to the zone
						string	igname = boxes[i].Name;
						CIFile			monStream(CPath::lookup(igname));
						CInstanceGroup	ig;
						monStream.serial(ig);

						// search in group for water instance
						for (j=0; j<ig._InstancesInfos.size(); ++j)
						{
							string	shapeName = ig._InstancesInfos[j].Name;
							if (CFile::getExtension (shapeName) == "")
								shapeName += ".shape";

							string	shapeNameLookup = CPath::lookup (shapeName, false, false);
							if (!shapeNameLookup.empty())
							{
								CIFile			f;
								if (f.open (shapeNameLookup))
								{
									CShapeStream	shape;
									shape.serial(f);

									CWaterShape	*wshape = dynamic_cast<CWaterShape *>(shape.getShapePointer());
									if (wshape == NULL)
										continue;

									CMatrix	matrix;
									ig.getInstanceMatrix(j, matrix);

									CPolygon			wpoly;
									wshape->getShapeInWorldSpace(wpoly);

									uint	k;
									for (k=0; k<wpoly.Vertices.size(); ++k)
									{
										//wpoly.Vertices[k].z = 0.0f;
										wpoly.Vertices[k] = matrix * wpoly.Vertices[k];
									}

									tessellation.addWaterShape(wpoly);
								}
								else
								{
									nlwarning ("Can't load shape %s", shapeNameLookup.c_str());
								}
							}
						}
					}
					catch (const Exception &e)
					{
						nlwarning("%s", e.what());
					}
				}
			}

			tessellation.compile();
			tessellation.generateBorders(1.0);

			NLPACS::CLocalRetriever	retriever;

			CAABBox	rbbox = tessellation.BestFittingBBox;
			CVector hs = rbbox.getHalfSize();
			hs.z = 10000.0f;
			rbbox.setHalfSize(hs);
			retriever.setBBox(rbbox);
			retriever.setType(NLPACS::CLocalRetriever::Landscape);

			for (j=0; j<(sint)tessellation.Surfaces.size(); ++j)
			{
				retriever.addSurface(0,
									 0,
									 0,
									 0,
									 0,
									 tessellation.Surfaces[j].IsUnderWater,
									 tessellation.Surfaces[j].WaterHeight,
									 tessellation.Surfaces[j].ClusterHint,
									 tessellation.Surfaces[j].Center,
									 tessellation.Surfaces[j].HeightQuad,
									 tessellation.Surfaces[j].QuantHeight);
			}

			for (j=0; j<(sint)tessellation.Borders.size(); ++j)
			{
				if (tessellation.Borders[j].Right < -1)
				{
					retriever.addChain(tessellation.Borders[j].Vertices,
									   tessellation.Borders[j].Left,
									   NLPACS::CChain::getDummyBorderChainId());

				}
				else
				{
					retriever.addChain(tessellation.Borders[j].Vertices,
									   tessellation.Borders[j].Left,
									   tessellation.Borders[j].Right);
				}
			}

			fullChains = retriever.getFullOrderedChains();

			// save raw retriever
			COFile	outputRetriever;
			name = changeExt(zoneName, string("lr"));
			filename = OutputPath+PreprocessDirectory+name;
			if (Verbose)
				nlinfo("save file %s", filename.c_str());
			outputRetriever.open(filename);
			retriever.serial(outputRetriever);

			// save raw chains
			COFile	outputChains;
			name = changeExt(zoneName, string("ochain"));
			filename = OutputPath+name;
			if (Verbose)
				nlinfo("save file %s", filename.c_str());
			outputChains.open(filename);
			outputChains.serialCont(fullChains);
		}
	}
	catch(const Exception &e)
	{
		printf(e.what ());
	}
}




void processRetriever(string &zoneName)
{
	string							name;
	string							filename;

	try
	{
		uint16	zid = getZoneIdByName(zoneName);
		CAABBox	box = getZoneBBoxById(zid);

		NLPACS::CLocalRetriever	retriever;

		// load raw retriever
		CIFile	inputRetriever;
		name = changeExt(zoneName, string("lr"));
		filename = OutputPath+PreprocessDirectory+name;
		if (Verbose)
			nlinfo("load file %s", filename.c_str());

		if (CFile::fileExists(filename))
		{
			inputRetriever.open(filename);
			retriever.serial(inputRetriever);

			// compute the retriever

			retriever.computeLoopsAndTips();

			retriever.findBorderChains();
			retriever.updateChainIds();
			retriever.computeTopologies();

			retriever.computeCollisionChainQuad();

			retriever.setType(NLPACS::CLocalRetriever::Landscape);

			//
			CSurfaceSplitter	splitter;
			//splitter.build(retriever);

			// and save it...

			COFile	outputRetriever;
			name = changeExt(zoneName, string("lr"));
			filename = OutputPath+name;
			if (Verbose)
				nlinfo("save file %s", filename.c_str());
			outputRetriever.open(filename);
			retriever.serial(outputRetriever);
		}
	}
	catch(const Exception &e)
	{
		printf(e.what ());
	}
}
*/




//
class CFaultyChain
{
public:
	uint		Chain;
	CVectorD	Start, End;
	sint		PreviousChain, NextChain;
};

class CReconstructed
{
public:
	CReconstructed() : FrontInstance(-1), FrontChain(-1) {}
	vector<uint>			Chains;
	CVectorD				Start, End;
	sint					FrontInstance, FrontChain;
};

class CFaultyInstance
{
public:
	uint					Instance;
	vector<CFaultyChain>	Chains;
	vector<CReconstructed>	Reconstructed;
};

class CChainRef
{
public:
	uint					Chain, Previous;
	uint					FrontChain;
	uint					BorderId;
	uint					From, To;
};

class CFullChain
{
public:
	uint					Instance;
	vector<CVector>			Vertices;
	vector<CChainRef>		Chains;
};


void	fixFaultyLinks(map<uint, CFaultyInstance> &faultyInstances, 
					   const vector<NLPACS::CRetrieverInstance> &instances,
					   const vector<NLPACS::CLocalRetriever> &retrievers)
{
	map<uint, CFaultyInstance>::iterator	ifi;
	uint									i, j, k, l;

	// first
	// rebuild full chains
	// -- join all chains that are missing a link
	for (ifi=faultyInstances.begin(); ifi!=faultyInstances.end(); ++ifi)
	{
		CFaultyInstance	&inst = (*ifi).second;

		// for each chain, find best matching ending chain
		for (k=0; k<inst.Chains.size(); ++k)
		{
			sint	best = -1;
			double	bestDist = 1.0e10;
			for (l=0; l<inst.Chains.size(); ++l)
			{
				if (l == k || (best != -1 && inst.Chains[best].PreviousChain != -1))
					continue;

				CVectorD	diff = inst.Chains[k].End - inst.Chains[l].Start;
				double		dist = diff.norm();

				if (dist < 0.1 && dist < bestDist)
				{
					best = l;
					bestDist = dist;
				}
			}

			if (best != -1)
			{
				inst.Chains[best].PreviousChain = k;
				inst.Chains[k].NextChain = best;
			}
		}

		//
		for (k=0; k<inst.Chains.size(); ++k)
		{
			if (inst.Chains[k].PreviousChain == -1)
			{
				l = k;
				inst.Reconstructed.push_back(CReconstructed());
				do
				{
					inst.Reconstructed.back().Chains.push_back(l);
				}
				while ((sint)(l=inst.Chains[l].NextChain) != -1);
				inst.Reconstructed.back().Start = inst.Chains[inst.Reconstructed.back().Chains.front()].Start;
				inst.Reconstructed.back().End   = inst.Chains[inst.Reconstructed.back().Chains.back()].End;
			}
		}
	}

	// second
	// match reconstructed chains
	// -- for each reconstructed chain in an instance, find best matching reconstructed chain in neighbour instances
	for (ifi=faultyInstances.begin(); ifi!=faultyInstances.end(); ++ifi)
	{
		CFaultyInstance	&inst = (*ifi).second;

		const NLPACS::CRetrieverInstance	&instance = instances[inst.Instance];
		const NLPACS::CLocalRetriever		&retriever = retrievers[instance.getRetrieverId()];
		vector<sint32>						neighbs = instance.getNeighbors();

		for (i=0; i<neighbs.size(); ++i)
		{
			map<uint, CFaultyInstance>::iterator	ifn = faultyInstances.find(neighbs[i]);
			if (ifn == faultyInstances.end())
				continue;

			CFaultyInstance	&neighb = (*ifn).second;

			for (j=0; j<inst.Reconstructed.size(); ++j)
			{
				if (inst.Reconstructed[j].FrontInstance != -1)
					continue;

				CVectorD		&astart = inst.Reconstructed[j].Start,
								&aend = inst.Reconstructed[j].End;

				const NLPACS::CRetrieverInstance	&ninstance = instances[neighb.Instance];
				const NLPACS::CLocalRetriever		&nretriever = retrievers[ninstance.getRetrieverId()];

				for (k=0; k<neighb.Reconstructed.size(); ++k)
				{
					if (neighb.Reconstructed[k].FrontInstance != -1)
						continue;

					CVectorD	&bstart = neighb.Reconstructed[j].Start,
								&bend = neighb.Reconstructed[j].End;

					if ((astart-bend).norm() < 0.1 && (aend-bstart).norm() < 0.1)
					{
						// ok, found missing match !
						inst.Reconstructed[j].FrontInstance = neighb.Instance;
						inst.Reconstructed[j].FrontChain = k;
						neighb.Reconstructed[k].FrontInstance = inst.Instance;
						neighb.Reconstructed[k].FrontChain = j;

						CFullChain		fci, fcn;
						uint			m;

						CVector			ori = instance.getOrigin(),
										orn = ninstance.getOrigin();

						fci.Instance = inst.Instance;
						fcn.Instance = neighb.Instance;

						// build full chains
						for (l=0; l<inst.Reconstructed[j].Chains.size(); ++l)
						{
							uint								chain = inst.Chains[inst.Reconstructed[j].Chains[l]].Chain;
							NLPACS::CLocalRetriever::CIterator	it(&retriever, chain);

							CChainRef	cr;
							cr.Chain = chain;
							cr.Previous = chain;
							cr.From = (uint)fci.Vertices.size();
							cr.BorderId = NLPACS::CChain::convertBorderChainId(retriever.getChain(chain).getRight());

							while (!it.end())
							{
								fci.Vertices.push_back(it.get3d()+ori);
								++it;
							}

							cr.To = (uint)fci.Vertices.size()-1;

							if (l < inst.Reconstructed[j].Chains.size()-1)
								fci.Vertices.pop_back();

							fci.Chains.push_back(cr);
						}

						for (l=0; l<neighb.Reconstructed[k].Chains.size(); ++l)
						{
							uint								chain = neighb.Chains[neighb.Reconstructed[k].Chains[l]].Chain;
							NLPACS::CLocalRetriever::CIterator	it(&nretriever, chain);

							CChainRef	cr;
							cr.Chain = chain;
							cr.Previous = chain;
							cr.From = (uint)fcn.Vertices.size();
							cr.BorderId = NLPACS::CChain::convertBorderChainId(nretriever.getChain(chain).getRight());

							while (!it.end())
							{
								fcn.Vertices.push_back(it.get3d()+orn);
								++it;
							}

							cr.To = (uint)fcn.Vertices.size()-1;

							if (l < neighb.Reconstructed[k].Chains.size()-1)
								fcn.Vertices.pop_back();

							fcn.Chains.push_back(cr);
						}

						if (fcn.Vertices.size() != fci.Vertices.size())
						{
							nlwarning("Couldn't reconstruct link between %d and %d, mismatching number of vertices", inst.Instance, neighb.Instance);
							break;
						}

						for (l=0; l<fci.Vertices.size(); ++l)
						{
							if ((fci.Vertices[l] - fcn.Vertices[fci.Vertices.size()-1-l]).norm() > 0.2f)
							{
								nlwarning("Couldn't reconstruct link between %d and %d, some vertices don't match", inst.Instance, neighb.Instance);
								break;
							}

							fci.Vertices[l] -= ori;
							fcn.Vertices[fci.Vertices.size()-1-l] -= orn;
						}

						if (l<fci.Vertices.size())
							break;

						uint	newChaini = (uint)retriever.getChains().size(),
								newChainn = (uint)nretriever.getChains().size();

						// save free border ids in order to renumerate them after splits
						vector<uint>	ifreeBorderIds, nfreeBorderIds;
						uint			inextBorderId, nnextBorderId;

						for (l=0; l<fci.Chains.size(); ++l)
							ifreeBorderIds.push_back(fci.Chains[l].BorderId);
						inextBorderId = (uint)retriever.getBorderChains().size();

						for (l=0; l<fcn.Chains.size(); ++l)
							nfreeBorderIds.push_back(fcn.Chains[l].BorderId);
						nnextBorderId = (uint)nretriever.getBorderChains().size();

						// generate splits from first chain on second chain
						for (l=0; l<fci.Chains.size()-1; ++l)
						{
							uint	splitAt = (uint)fci.Vertices.size()-1 - fci.Chains[l].To;

							for (m=(uint)fcn.Chains.size()-1; (sint)m>=0 && fcn.Chains[m].From>splitAt; --m)
								;

							// no split ?
							if ((sint)m < 0 || fcn.Chains[m].From == splitAt)
								continue;

							// insert split in second chain
							fcn.Chains.insert(fcn.Chains.begin()+m+1, fcn.Chains[m]);
							fcn.Chains[m].To = splitAt;
							fcn.Chains[m+1].From = splitAt;
							fcn.Chains[m+1].Chain = newChainn++;
						}

						// generate splits from second chain on first chain
						for (l=0; l<fcn.Chains.size()-1; ++l)
						{
							uint	splitAt = (uint)fcn.Vertices.size()-1 - fcn.Chains[l].To;

							for (m=(uint)fci.Chains.size()-1; (sint)m>=0 && fci.Chains[m].From>splitAt; --m)
								;

							// no split ?
							if ((sint)m < 0 || fci.Chains[m].From == splitAt)
								continue;

							// insert split in first chain
							fci.Chains.insert(fci.Chains.begin()+m+1, fci.Chains[m]);
							fci.Chains[m].To = splitAt;
							fci.Chains[m+1].From = splitAt;
							fci.Chains[m+1].Chain = newChaini++;
						}

						if (fci.Chains.size() != fcn.Chains.size())
						{
							nlwarning("Couldn't reconstruct link between %d and %d, chain splitting failed", inst.Instance, neighb.Instance);
							break;
						}

						// renumerate border ids after splits
						for (l=0; l<fci.Chains.size(); ++l)
						{
							if (!ifreeBorderIds.empty())
							{
								fci.Chains[l].BorderId = ifreeBorderIds.back();
								ifreeBorderIds.pop_back();
							}
							else
							{
								fci.Chains[l].BorderId = inextBorderId++;
							}

							(const_cast<NLPACS::CLocalRetriever&>(retriever)).forceBorderChainId(fci.Chains[l].Chain, fci.Chains[l].BorderId);
						}

						for (l=0; l<fcn.Chains.size(); ++l)
						{
							if (!nfreeBorderIds.empty())
							{
								fcn.Chains[l].BorderId = nfreeBorderIds.back();
								nfreeBorderIds.pop_back();
							}
							else
							{
								fcn.Chains[l].BorderId = nnextBorderId++;
							}

							(const_cast<NLPACS::CLocalRetriever&>(nretriever)).forceBorderChainId(fcn.Chains[l].Chain, fcn.Chains[l].BorderId);
						}

						// insert/replace new chains in instances

						vector<NLPACS::CLocalRetriever::CChainReplacement>	replacement;
						vector<uint>										newIds;

						l=0;
						while (l<fci.Chains.size())
						{
							sint	previous=-1;

							newIds.clear();
							replacement.clear();

							for (; l<fci.Chains.size(); ++l)
							{
								if (previous != -1 && previous != (sint)fci.Chains[l].Previous)
									break;

								previous = fci.Chains[l].Previous;

								NLPACS::CLocalRetriever::CChainReplacement	cr;

								cr.Chain = fci.Chains[l].Chain;
								cr.Left = retriever.getChain(previous).getLeft();
								cr.Right = NLPACS::CChain::convertBorderChainId(fci.Chains[l].BorderId);
								cr.Vertices.clear();
								cr.Vertices.insert(cr.Vertices.begin(),
												   fci.Vertices.begin()+fci.Chains[l].From, 
												   fci.Vertices.begin()+fci.Chains[l].To+1);

								replacement.push_back(cr);
								newIds.push_back(fci.Chains[l].BorderId);
							}

							if (replacement.size() >= 2)
							{
								(const_cast<NLPACS::CLocalRetriever&>(retriever)).replaceChain(previous, replacement);
								(const_cast<NLPACS::CRetrieverInstance&>(instance)).resetBorderChainLinks(newIds);
							}
						}

						l=0;
						while (l<fcn.Chains.size())
						{
							sint	previous=-1;

							newIds.clear();
							replacement.clear();

							for (; l<fcn.Chains.size(); ++l)
							{
								if (previous != -1 && previous != (sint)fcn.Chains[l].Previous)
									break;

								previous = fcn.Chains[l].Previous;

								NLPACS::CLocalRetriever::CChainReplacement	cr;

								cr.Chain = fcn.Chains[l].Chain;
								cr.Left = nretriever.getChain(previous).getLeft();
								cr.Right = NLPACS::CChain::convertBorderChainId(fcn.Chains[l].BorderId);
								cr.Vertices.clear();
								cr.Vertices.insert(cr.Vertices.begin(),
												   fcn.Vertices.begin()+fcn.Chains[l].From, 
												   fcn.Vertices.begin()+fcn.Chains[l].To+1);

								replacement.push_back(cr);
								newIds.push_back(fcn.Chains[l].BorderId);
							}

							if (replacement.size() >= 2)
							{
								(const_cast<NLPACS::CLocalRetriever&>(nretriever)).replaceChain(previous, replacement);
								(const_cast<NLPACS::CRetrieverInstance&>(ninstance)).resetBorderChainLinks(newIds);
							}
						}

						// force links between instances (border chain links)
						for (l=0; l<fci.Chains.size(); ++l)
						{
							m = (uint)fci.Chains.size()-1-l;
							(const_cast<NLPACS::CRetrieverInstance&>(instance)).forceBorderChainLink(fci.Chains[l].BorderId, 
																									 neighb.Instance,
																									 fcn.Chains[m].BorderId,
																									 fcn.Chains[m].Chain,
																									 nretriever.getChain(fcn.Chains[m].Chain).getLeft());
							(const_cast<NLPACS::CRetrieverInstance&>(ninstance)).forceBorderChainLink(fcn.Chains[m].BorderId, 
																									 inst.Instance,
																									 fci.Chains[l].BorderId,
																									 fci.Chains[l].Chain,
																									 retriever.getChain(fci.Chains[l].Chain).getLeft());

							if (Verbose)
								nlinfo("Fixed: link between %d/%d and %d/%d => %d/%d - %d/%d", fci.Instance, fci.Chains[l].Previous, fcn.Instance, fcn.Chains[m].Previous, fci.Instance, fci.Chains[l].Chain, fcn.Instance, fcn.Chains[m].Chain);
						}

						break;
					}
				}
			}
		}

		(const_cast<NLPACS::CLocalRetriever&>(retriever)).computeCollisionChainQuad();
	}
}


void	processGlobalRetriever()
{
	NLPACS::CRetrieverBank		retrieverBank;
	NLPACS::CGlobalRetriever	globalRetriever;

	uint						ULid = getZoneIdByName(GlobalUL),
								DRid = getZoneIdByName(GlobalDR);
	
	CAABBox						ULbbox = getZoneBBoxById(ULid);
	CAABBox						DRbbox = getZoneBBoxById(DRid);
	CAABBox						bbox;

	CVector						vmin, vmax;

	vmin.minof(ULbbox.getMin(), DRbbox.getMin());
	vmax.maxof(ULbbox.getMax(), DRbbox.getMax());
	bbox.setMinMax(vmin, vmax);

	uint						x0 = ULid%256, 
								y0 = ULid/256,
								x1 = DRid%256, 
								y1 = DRid/256;
	
	globalRetriever.setRetrieverBank(&retrieverBank);
	globalRetriever.init();

	uint						x, y;

	if (Verbose)
		nlinfo("make all instances");

	for (y=y0; y<=y1; ++y)
	{
		for (x=x0; x<=x1; ++x)
		{
			try
			{
				string filename = OutputPath+getZoneNameById(x+y*256)+".lr";
				if (CFile::fileExists (filename))
				{
					uint	retrieverId = retrieverBank.addRetriever(filename);
					globalRetriever.makeInstance(retrieverId, 0, getZoneCenterById((uint16)getIdByCoord(x, y))); 
				}
			}
			catch (const Exception &e)
			{
				printf("%s\n", e.what ());
			}
		}
	}

	if (Verbose)
		nlinfo("make all links");
	globalRetriever.makeAllLinks();

//	if (Verbose)
//		nlinfo("clean retriever bank up");
//	retrieverBank.clean();

	map<uint, CFaultyInstance>				faultyInstances;

	const vector<NLPACS::CRetrieverInstance>	&instances = globalRetriever.getInstances();
	const vector<NLPACS::CLocalRetriever>		&retrievers = retrieverBank.getRetrievers();
	uint	i, j;
	uint	totalUnlinked = 0, totalLink = 0;
	for (i=0; i<instances.size(); ++i)
	{
		const vector<NLPACS::CRetrieverInstance::CLink>	&links = instances[i].getBorderChainLinks();
		CVector	pos = instances[i].getBBox().getCenter();
		string	unlinkstr = "instance "+toString(i)+":"+getZoneNameById(getZoneIdByPos(pos))+":";
		bool	unlinkerr = false;

		const NLPACS::CLocalRetriever			&retriever = retrievers[instances[i].getRetrieverId()];

		CFaultyInstance		fi;
		fi.Instance = i;

		for (j=0; j<links.size(); ++j)
		{
			++totalLink;
			if (links[j].Instance == 0xffff)
			{
				unlinkstr += (string(" ")+toString(j));
				++totalUnlinked;
				unlinkerr = true;

				CFaultyChain	fc;

				fc.Chain = retriever.getBorderChain(j);
				fc.Start = retriever.getStartVector(fc.Chain) + instances[i].getOrigin();
				fc.End   = retriever.getStopVector(fc.Chain) + instances[i].getOrigin();
				fc.PreviousChain = -1;
				fc.NextChain = -1;

				fi.Chains.push_back(fc);
			}
		}
		if (unlinkerr)
		{
			if (Verbose)
				nlinfo("unlink: %s", unlinkstr.c_str());
			faultyInstances.insert(make_pair<uint, CFaultyInstance>(i, fi));
		}
	}

	if (Verbose)
		nlinfo("%d are still unlinked (%d links total)", totalUnlinked, totalLink);

	// rebuild full chains
	if (totalUnlinked > 0)
	{
		if (Verbose)
			nlinfo("Fixing faulty links...");
		fixFaultyLinks(faultyInstances, instances, retrievers);

		// recheck

		const vector<NLPACS::CRetrieverInstance>	&instances = globalRetriever.getInstances();
		const vector<NLPACS::CLocalRetriever>		&retrievers = retrieverBank.getRetrievers();
		uint	i, j;
		uint	totalUnlinked = 0, totalLink = 0;
		for (i=0; i<instances.size(); ++i)
		{
			const vector<NLPACS::CRetrieverInstance::CLink>	&links = instances[i].getBorderChainLinks();
			CVector	pos = instances[i].getBBox().getCenter();
			string	unlinkstr = "instance "+toString(i)+":"+getZoneNameById(getZoneIdByPos(pos))+":";
			bool	unlinkerr = false;

			const NLPACS::CLocalRetriever			&retriever = retrievers[instances[i].getRetrieverId()];

			CFaultyInstance		fi;
			fi.Instance = i;

			for (j=0; j<links.size(); ++j)
			{
				++totalLink;
				if (links[j].Instance == 0xffff)
				{
					unlinkstr += (string(" ")+toString(j));
					++totalUnlinked;
					unlinkerr = true;

					CFaultyChain	fc;

					fc.Chain = retriever.getBorderChain(j);
					fc.Start = retriever.getStartVector(fc.Chain) + instances[i].getOrigin();
					fc.End   = retriever.getStopVector(fc.Chain) + instances[i].getOrigin();
					fc.PreviousChain = -1;
					fc.NextChain = -1;

					fi.Chains.push_back(fc);
				}
			}
			if (unlinkerr)
			{
				if (Verbose)
					nlinfo("after fix: unlink: %s", unlinkstr.c_str());
				faultyInstances.insert(make_pair<uint, CFaultyInstance>(i, fi));
			}
		}
	}

	if (Verbose)
		nlinfo("init the quad grid");
	globalRetriever.initQuadGrid();

	string	filename;

	COFile	outputRetriever;
	filename = OutputPath+GlobalRetriever;
	if (Verbose)
		nlinfo("save file %s", filename.c_str());
	outputRetriever.open(filename);
	globalRetriever.serial(outputRetriever);

	COFile	outputBank;
	filename = OutputPath+RetrieverBank;
	if (Verbose)
		nlinfo("save file %s", filename.c_str());
	outputBank.open(filename);
	retrieverBank.serial(outputBank);

	retrieverBank.saveRetrievers(OutputPath, CFile::getFilenameWithoutExtension(RetrieverBank));
}

///

void	updateRetrieverBank()
{
	NLPACS::CRetrieverBank		retrieverBank;

	string	filename;
	filename = OutputPath+RetrieverBank;

	CIFile	inputBank;
	if (Verbose)
		nlinfo("load file %s", filename.c_str());
	inputBank.open(filename);
	retrieverBank.serial(inputBank);
	inputBank.close();

	COFile	outputBank;
	if (Verbose)
		nlinfo("save file %s", filename.c_str());
	outputBank.open(filename);
	retrieverBank.serial(outputBank);
	outputBank.close();
}


