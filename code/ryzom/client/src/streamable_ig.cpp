// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#include "stdpch.h"



#include "nel/3d/u_instance_group.h"
#include "nel/misc/path.h"
#include "nel/misc/progress_callback.h"

#include "ig_enum.h"
#include "streamable_ig.h"
#include "continent_manager.h"

extern CContinentManager ContinentMngr;

H_AUTO_DECL(RZ_StremableIG)

//=================================================================================
CStreamableIG::CStreamableIG()	: _Scene(NULL), _Linked(false), _IGMap(NULL)
{
	H_AUTO_USE(RZ_StremableIG)
}

//=================================================================================
void CStreamableIG::init(NL3D::UScene *owningScene, const NLMISC::CVector &pos,float forceLoadRadius,float loadRadius,float unloadRadius)
{
	H_AUTO_USE(RZ_StremableIG)
	nlassert(_Scene == NULL); // init should be called once
	nlassert(owningScene);
	_Scene = owningScene;
	if (!(unloadRadius >= loadRadius && loadRadius >= forceLoadRadius && forceLoadRadius >= 0))
	{
		nlwarning("CStreamableIG::init : invalid radius have been used !");
	}
	CStreamableEntity::setForceLoadRadius(forceLoadRadius);
	CStreamableEntity::setLoadRadius(loadRadius);
	CStreamableEntity::setUnloadRadius(unloadRadius);
	CStreamableEntity::setPos(pos);
}

//=================================================================================
CStreamableIG::~CStreamableIG()
{
	H_AUTO_USE(RZ_StremableIG)
	if (_Scene)
	{
		removeLoadedIGFromMap();
		for(uint k = 0; k < _IGs.size(); ++k)
		{
			if (_IGs[k].Loading)
			{
				_Scene->stopCreatingAndAddingIG(&_IGs[k].IG);
			}
			else if(_IGs[k].IG && _IGs[k].IG != (NL3D::UInstanceGroup *)-1)
			{
				notifyIGRemoved(_IGs[k].IG);
			}

			if (_IGs[k].IG && _IGs[k].IG != (NL3D::UInstanceGroup *)-1)
			{
				_IGs[k].IG->removeFromScene(*_Scene);
				_Scene->deleteInstanceGroup(_IGs[k].IG);
				_IGs[k].IG= NULL;
			}
		}
	}
}

//=================================================================================
/*virtual*/ void CStreamableIG::loadAsync()
{
	H_AUTO_USE(RZ_StremableIG)
	if (!_Linked)
	{
		if(!ClientCfg.Light)
			nlwarning("Loading async %p", this);
		#ifdef NL_DEBUG
				//nlinfo("Loading async : %s", Name.c_str());
		#endif
		bool canLinkNow = true;
		for(uint k = 0; k < _IGs.size(); ++k)
		{
			if (!_IGs[k].Loading && !_IGs[k].IG)
			{
				// Current continent season
				EGSPD::CSeason::TSeason season = ContinentMngr.cur()->Season;

				//nlinfo("started load async");
				_IGs[k].Loading = true;
				_Callback.Owner = this;
				_Scene->createInstanceGroupAndAddToSceneAsync(_IGs[k].Name + ".ig", &_IGs[k].IG, _IGs[k].Pos, _IGs[k].Rot, season, &_Callback);
			}
			else
			{
				if (_IGs[k].Loading && _IGs[k].IG)
				{
					//nlinfo("loading finiched");
					// loading has finished
					_IGs[k].Loading = false;

					if (_IGs[k].IG != (NL3D::UInstanceGroup *)-1 && _IGMap)
					{
						(*_IGMap)[_IGs[k].Name] = _IGs[k].IG;
						this->notifyIGAdded(_IGs[k].IG);
					}
				}
			}

			// Load is not finished
			canLinkNow &= !_IGs[k].Loading;
		}

		if (canLinkNow)
		{
			linkInstances();
		}
	}
}

//=================================================================================
/*virtual*/ void CStreamableIG::load(NLMISC::IProgressCallback &progress)
{
	H_AUTO_USE(RZ_StremableIG)
	if (!_Linked)
	{
		nlwarning("Load %p", this);
		#ifdef NL_DEBUG
			//nlinfo("Loading : %s", Name.c_str());
		#endif
		std::vector<bool> waitForIg;
		waitForIg.resize(_IGs.size());
		for(uint k = 0; k < _IGs.size(); ++k)
		{
			#ifdef NL_DEBUG
				//nlinfo("Loading ig %s", _IGs[k].Name.c_str());
			#endif
			progress.progress((float)k/((float)_IGs.size()*2.f));

			if (!_IGs[k].IG)
			{
				//nlinfo("blocking load");
				if (!_IGs[k].Loading)
				{
					// Current continent season
					EGSPD::CSeason::TSeason season = ContinentMngr.cur()->Season;

					//nlinfo("start blocking load");
					// blocking load
					// block after queueing all
					_Callback.Owner = this;
					_Scene->createInstanceGroupAndAddToSceneAsync(_IGs[k].Name + ".ig", &_IGs[k].IG, _IGs[k].Pos, _IGs[k].Rot, season, &_Callback);
					_IGs[k].Loading = true;
				}
				
				_Scene->updateWaitingInstances(1000); /* set a high value to upload texture at a fast rate */

				waitForIg[k] = true;
			}
			else
			{
				if (_IGs[k].Loading && _IGs[k].IG)
				{
					_IGs[k].Loading = false;
				}

				waitForIg[k] = false;
			}
		}
		for(uint k = 0; k < _IGs.size(); ++k)
		{
			progress.progress(((float)k + (float)_IGs.size())/((float)_IGs.size()*2.f));

			if (waitForIg[k])
			{
				//nlinfo("wait for end of blockin load");
				// blocking call
				while (!_IGs[k].IG)
				{
					NLMISC::nlSleep(1);
					// wait till loaded...
					_Scene->updateWaitingInstances(1000); /* set a high value to upload texture at a fast rate */
				}
				_IGs[k].Loading = false;
			}
		}
		linkInstances();
		addLoadedIGToMap();
	}
}

//=================================================================================
/*virtual*/ void CStreamableIG::unload()
{
	H_AUTO_USE(RZ_StremableIG)
	#ifdef NL_DEBUG
		// nlinfo("Unloading : %s", Name.c_str());
	#endif
	if (_Linked)
	{
		nlwarning("Unloading %p", this);
	}
	removeLoadedIGFromMap();
	for(uint k = 0; k < _IGs.size(); ++k)
	{
		if (_IGs[k].Loading)
		{
			if (_IGs[k].IG)
			{
				if (_IGs[k].IG != (NL3D::UInstanceGroup *)-1)
				{
					// the ig has just finished loading, and loading hasn't failed
					_IGs[k].IG->removeFromScene(*_Scene);
					// notifyIGAdded has not been called yet, so no need to call notifyIGRemoved
				}
			}
			else
			{
				_Scene->stopCreatingAndAddingIG(&_IGs[k].IG);
			}
			_IGs[k].Loading = false;
			_IGs[k].IG = NULL;
		}
		else
		{
			if (_IGs[k].IG && _IGs[k].IG != (NL3D::UInstanceGroup *)-1) // -1 signal that async loading failed
			{
				//nlinfo("unload 2");
				nlassert(_Scene);
				_IGs[k].IG->removeFromScene(*_Scene);
				_Scene->deleteInstanceGroup (_IGs[k].IG);
				this->notifyIGRemoved(_IGs[k].IG);
			}
			_IGs[k].IG = NULL;
		}
	}
	_Linked = false;
}

//=================================================================================
void CStreamableIG::forceUnload()
{
	H_AUTO_USE(RZ_StremableIG)
	unload();
}


//===================================================================================
bool CStreamableIG::setIG(uint ig, const std::string &name, const std::string &parentName)
{
	H_AUTO_USE(RZ_StremableIG)
	if (ig<_IGs.size())
	{
		// Destroy this IG
		if (_IGs[ig].Loading)
		{
			if (!_IGs[ig].IG)
			{
				_Scene->stopCreatingAndAddingIG(&_IGs[ig].IG);
				this->notifyIGRemoved(_IGs[ig].IG);
				_IGs[ig].Loading = false;
				_IGs[ig].IG = NULL;
			}
		}
		else
		{
			if (_IGs[ig].IG && _IGs[ig].IG != (NL3D::UInstanceGroup *)-1) // -1 signal that async loading failed
			{
				nlassert(_Scene);
				_IGs[ig].IG->removeFromScene(*_Scene);
				_Scene->deleteInstanceGroup (_IGs[ig].IG);
				this->notifyIGRemoved(_IGs[ig].IG);
			}
			_IGs[ig].IG = NULL;
		}

		// Load this IG
		_IGs[ig].Name = NLMISC::CFile::getFilenameWithoutExtension(name);
		_IGs[ig].ParentName = NLMISC::CFile::getFilenameWithoutExtension(parentName);
		NLMISC::strlwr(_IGs[ig].Name);
		NLMISC::strlwr(_IGs[ig].ParentName);
		_IGs[ig].IG = NULL;
		_IGs[ig].Loading = false;
		_Linked = false;
		return true;
	}
	return false;
}

//=================================================================================
void CStreamableIG::addIG(const std::string &name,const std::string &parentName, const NLMISC::CVector &pos, const NLMISC::CQuat &rot)
{
	H_AUTO_USE(RZ_StremableIG)
	_IGs.push_back(CIGNode ());
	_IGs.back().Name = NLMISC::CFile::getFilenameWithoutExtension(name);
	_IGs.back().ParentName = NLMISC::CFile::getFilenameWithoutExtension(parentName);
	NLMISC::strlwr(_IGs.back().Name);
	NLMISC::strlwr(_IGs.back().ParentName);
	_IGs.back().IG = NULL;
	_IGs.back().Loading = false;
	_IGs.back().Pos = pos;
	_IGs.back().Rot = rot;
}

//=================================================================================
void CStreamableIG::linkInstances()
{
	H_AUTO_USE(RZ_StremableIG)
	if (_Linked)
		return;
	for(uint k = 0; k < _IGs.size(); ++k)
	{
		/** There are few igs at the same time, so a linear search should suffice for now
		  */
		if (_IGs[k].IG != (NL3D::UInstanceGroup *)-1)
		{
			// search the parent
			if (!_IGs[k].ParentName.empty())
			{
				for(uint l = 0; l < _IGs.size(); ++l)
				{
					if (l == k) continue; // can't be a parent of itself
					if (_IGs[l].IG != (NL3D::UInstanceGroup *)-1 && _IGs[k].ParentName == _IGs[l].Name)
					{
						if (!_IGs[k].IG->linkToParentCluster(_IGs[l].IG))
						{
							nlwarning("Failed to link cluster %s to its parent %s", _IGs[k].Name.c_str(), _IGs[k].ParentName.c_str());
						}
					}
				}
			}
		}
	}
	_Linked = true;
}

//=================================================================================
void CStreamableIG::reserve(uint size)
{
	H_AUTO_USE(RZ_StremableIG)
	_IGs.reserve(size);
}

//=================================================================================
void CStreamableIG::setLoadedIGMap(CStreamableIG::TString2IG *igMap)
{
	H_AUTO_USE(RZ_StremableIG)
	removeLoadedIGFromMap();
	_IGMap = igMap;
	addLoadedIGToMap();
}

//=================================================================================
void CStreamableIG::addLoadedIGToMap()
{
	H_AUTO_USE(RZ_StremableIG)
	if (!_IGMap) return;
	for(uint k = 0; k < _IGs.size(); ++k)
	{
		if (_IGs[k].IG && _IGs[k].IG != (NL3D::UInstanceGroup *)-1) // is this a successfully loaded ig ?
		{
			// insert the new ig if it hasn't before..
			if( _IGMap->insert(std::make_pair(NLMISC::strlwr(_IGs[k].Name), _IGs[k].IG)).second )
				// if inserted, must notify IG Added, else already notifiyed by loadAsync()
				this->notifyIGAdded(_IGs[k].IG);
		}
	}
}

//=================================================================================
void CStreamableIG::removeLoadedIGFromMap()
{
	H_AUTO_USE(RZ_StremableIG)
	if (!_IGMap) return;
	for(uint k = 0; k < _IGs.size(); ++k)
	{
		if (_IGs[k].IG && _IGs[k].IG != (NL3D::UInstanceGroup *)-1) // is this a successfully loaded ig ?
		{
			TString2IG::iterator it = _IGMap->find(_IGs[k].Name);
			if (it != _IGMap->end())
			{
				_IGMap->erase(it);
			}
		}
	}
}


//=================================================================================
bool CStreamableIG::enumIGs(IIGEnum *callback)
{
	H_AUTO_USE(RZ_StremableIG)
	bool continueEnum = true;
	for(TIGArray::iterator it = _IGs.begin(); it != _IGs.end() && continueEnum; ++it)
	{
		if (it->IG && it->IG != (NL3D::UInstanceGroup *)-1)
			continueEnum = callback->enumIG(it->IG);
	}
	return continueEnum;
}

