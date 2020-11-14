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


#include "std3d.h"
#include "nel/3d/skeleton_spawn_script.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/scene.h"
#include "nel/3d/particle_system_model.h"
#include "nel/misc/common.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************
CSkeletonSpawnScript::CSkeletonSpawnScript()
{
}


// ***************************************************************************
CSkeletonSpawnScript::~CSkeletonSpawnScript()
{
	// the user has not called release
	nlassert(_Instances.empty());
}


// ***************************************************************************
void	CSkeletonSpawnScript::evaluate(CSkeletonModel *skeleton)
{
	// if same cache, don't need to update parsed instances
	if(_Cache!=skeleton->getSpawnScript())
	{
		_Cache=skeleton->getSpawnScript();
		parseCache(skeleton->getOwnerScene(), skeleton);
	}

	// each frame, update PS UserMatrix
	for(uint i=0;i<_Instances.size();i++)
	{
		CInstance &inst= _Instances[i];
		if(inst.Model && inst.PS)
		{
			CMatrix	userMat;
			userMat.setRot(CVector::I, skeleton->getSSSWODir(), CVector::K);
			userMat.normalize(CMatrix::YZX);
			userMat.setPos(skeleton->getSSSWOPos());
			inst.PS->setUserMatrix(userMat);
		}
	}
}


// ***************************************************************************
void	CSkeletonSpawnScript::release(CScene *scene)
{
	// act as if the skeleton has an empty script
	if(!_Cache.empty())
	{
		_Cache.clear();
		parseCache(scene, NULL);
	}
}

// ***************************************************************************
void	CSkeletonSpawnScript::parseCache(CScene *scene, CSkeletonModel *skeleton)
{
	uint	i;
	nlassert(scene);
	nlassert(_Cache.empty() || skeleton);

	static std::vector<std::string>	newLines;
	newLines.clear();
	splitString(_Cache,"\n",newLines);

	// **** compare the 2 set of script line to know what to add, and what to remove.
	// NB: this is an O(N2), but the number of spawned objects should be small (0, 1 or 2 surely)
	static std::vector<bool>	srcToRemove;
	static std::vector<bool>	dstToAdd;
	srcToRemove.clear();
	dstToAdd.clear();
	srcToRemove.resize(_Instances.size(), true);
	dstToAdd.resize(newLines.size(), true);
	for(uint i=0;i<_Instances.size();i++)
	{
		for(uint j=0;j<newLines.size();j++)
		{
			// if the new line script is the same than the old one, then reuse!
			// NB: the script line contains an "instance number", so there is no risk of "reuse same twice"
			if(newLines[j] == _Instances[i].ScriptLine)
			{
				srcToRemove[i]= false;
				dstToAdd[j]= false;
			}
		}
	}

	// **** remove the no more used instances
	vector<CInstance>::iterator	it= _Instances.begin();
	for(i=0;i<srcToRemove.size();i++)
	{
		// if must remove this entry
		if(srcToRemove[i])
		{
			// then erase the entry
			if(it->Model)
				scene->deleteInstance(it->Model);
			it= _Instances.erase(it);
		}
		else
			it++;
	}

	// **** create the new instances
	for(i=0;i<dstToAdd.size();i++)
	{
		// if not reused
		if(dstToAdd[i])
		{
			// parse the line
			const std::string				&line= newLines[i];
			static std::vector<string>		words;
			words.clear();
			splitString(line, " ", words);
			if(words.size()>=3)
			{
				// command
				if(words[0]=="objw")
				{
					// format: "objw inst shapeName"
					// inst is a number only used to generate line difference (for cache comparison)
					_Instances.push_back(CInstance());
					CInstance	&inst= _Instances.back();
					inst.ScriptLine= line;

					// Delay the model creation at end of CScene::render()
					CSSSModelRequest	req;
					req.Skel= skeleton;
					req.InstanceId= (uint)_Instances.size()-1;
					req.Shape= words[2];
					// World Spawned Objects are sticked to the root bone (for CLod hiding behavior)
					req.BoneId= 0;
					// but have a special feature to compute their world matrix
					req.SSSWO= true;
					addModelCreationRequest(req, scene);
				}
				else if(words[0]=="objl" && words.size()>=4)
				{
					// format: "objl inst shapeName boneName"
					// inst is a number only used to generate line difference (for cache comparison)

					// get the bone name, but may have space in its name => words[3] is not the correct name
					uint	pos= 0;
					bool	inWord= false;
					uint	wordId= 0;
					// skip first spaces
					while(pos<line.size() && line[pos]==' ')
						pos++;
					// count word until reach
					while(pos<line.size())
					{
						if(line[pos]!=' ')
						{
							if(!inWord)
							{
								inWord= true;
								wordId++;
								// reach bone name! stop
								if(wordId==4)
									break;
							}
						}
						else
						{
							inWord= false;
						}

						pos++;
					}
					// and get the bone name
					string	boneName= line.substr(pos);

					// create
					_Instances.push_back(CInstance());
					CInstance	&inst= _Instances.back();
					inst.ScriptLine= line;
					// try to get the bone id from name
					sint32	boneId= skeleton->getBoneIdByName(boneName);
					// if fails, then don't create the instance
					if(boneId>=0)
					{
						// Delay the model creation at end of CScene::render()
						CSSSModelRequest	req;
						req.Skel= skeleton;
						req.InstanceId= (uint)_Instances.size()-1;
						req.Shape= words[2];
						req.BoneId= boneId;
						req.SSSWO= false;
						addModelCreationRequest(req, scene);
					}
					else
					{
						// avoid flooding (animation)
						#ifdef NL_DEBUG
						nlwarning("ERROR: SkeletonSpawnScript: boneId not found in: %s", line.c_str());
						#endif
					}
				}
				// unknown command
				else
				{
					// avoid flooding (animation)
					#ifdef NL_DEBUG
					nlwarning("ERROR: SkeletonSpawnScript: error in command: %s", line.c_str());
					#endif
				}
			}
		}
	}
}


// ***************************************************************************
void		CSkeletonSpawnScript::addModelCreationRequest(CSSSModelRequest &req, CScene *scene)
{
	// if the scene is currently rendering, then process the request at end of CScene::render()
	if(scene->isRendering())
		scene->addSSSModelRequest(req);
	// else process it now!
	else
		req.execute();
}


// ***************************************************************************
void		CSSSModelRequest::execute()
{
	// If skeleton still exist
	if(!Skel)
		return;

	CSkeletonSpawnScript	&sss= Skel->getSSSScript();
	CScene					*scene= Skel->getOwnerScene();

	// test validity of the instanceId
	if(InstanceId>=sss._Instances.size())
		return;
	// it must not have been already created (else error???)
	if(sss._Instances[InstanceId].Model)
		return;

	// OK, create the model
	CSkeletonSpawnScript::CInstance		&inst= sss._Instances[InstanceId];
	inst.Model= scene->createInstance(Shape);
	inst.PS= dynamic_cast<CParticleSystemModel*>((CTransformShape*)inst.Model);
	if(inst.Model)
	{
		Skel->stickObject(inst.Model, BoneId);
		inst.Model->setSSSWO(SSSWO);
	}
}


} // NL3D
