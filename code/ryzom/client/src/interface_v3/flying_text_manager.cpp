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

#include "flying_text_manager.h"
#include "interface_manager.h"
#include "group_in_scene.h"
#include "nel/gui/view_text.h"


using namespace std;
using namespace NLMISC;


// 500 seems to add at max 200Ko in memory (estimation done with the WindowTaskManager, so not really accurate)
const	uint	MaxFlyingText= 500;


// ***************************************************************************
CFlyingTextManager::CFlyingTextManager()
{
	_CharacterWindowOffsetX= 0;
}

// ***************************************************************************
CFlyingTextManager::~CFlyingTextManager()
{
	// must have been released
	nlassert(_InScenePool.empty() && _InSceneCurrent.empty());
}

// ***************************************************************************
void	CFlyingTextManager::initInGame()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	nlassert (pIM);

	// must have been released
	nlassert(_InScenePool.empty() && _InSceneCurrent.empty());

	// get the root ptr
	_Root= "ui:interface";

	// create all
	_InScenePool.reserve(MaxFlyingText);
	for(uint i=0;i<MaxFlyingText;i++)
	{
		// create the in scene group
		string id = "in_scene_flying_text_"+toString (i);
		std::vector<std::pair<std::string,std::string> > templateParams;
		templateParams.push_back (std::pair<std::string,std::string>("id", id));
		CInterfaceGroup *groupInfo = CWidgetManager::getInstance()->getParser()->createGroupInstance ("in_scene_flying_text",
			"ui:interface", templateParams);
		// if ok
		if(groupInfo)
		{
			CGroupInfo		gi;
			gi.GroupInScene= dynamic_cast<CGroupInScene*>(groupInfo);
			gi.ViewText= dynamic_cast<CViewText*>(groupInfo->getView("text"));
			// if ok append, else delete and abort
			if(gi.GroupInScene && gi.GroupInScene)
			{
				_InScenePool.push_back(gi);
				/* don't link to the interface now. Prefer do this to avoid big number of window to parse
					each frame for nothing because they are hid.
				   This save 1ms per frame for 1000 window created but not displayed....
				   NB: the cost of link/unlink instead of a setActive() is negligible in effect
				*/
				gi.GroupInScene->setActive(true);
				nlassert(gi.GroupInScene->getParent()==NULL);
			}
			else
			{
				delete groupInfo;
			}
		}
	}

	// misc
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("in_scene_flying_char_offsetx"), _CharacterWindowOffsetX);
}

// ***************************************************************************
void	CFlyingTextManager::releaseInGame()
{
	// **** unlink all nodes from interface
	TInSceneCurrentMap::iterator	it= _InSceneCurrent.begin();
	for(;it!=_InSceneCurrent.end();it++)
	{
		it->second.UsedThisFrame= false;
	}
	releaseNotUsedFlyingText();
	nlassert(_InSceneCurrent.empty());

	// **** delete the pool
	for(uint i=0;i<_InScenePool.size();i++)
	{
		// since unlinked from window list and group list, just delete
		delete _InScenePool[i].GroupInScene;
	}
	_InScenePool.clear();
}

// ***************************************************************************
void	CFlyingTextManager::addFlyingText(void *key, const ucstring &text, const NLMISC::CVector &pos, CRGBA color, float scale, sint offsetX)
{
	// key exist in the map?
	TInSceneCurrentMap::iterator	it= _InSceneCurrent.find(key);
	// no, try to allocate a new entry
	if(it==_InSceneCurrent.end())
	{
		// no more free group, abort
		if(_InScenePool.empty())
			return;
		// else take an entry
		it= _InSceneCurrent.insert(make_pair(key, _InScenePool.back())).first;
		// and no more free
		_InScenePool.pop_back();
		// link to interface
		linkToInterface(it->second);
	}

	// this entry is used
	CGroupInfo	&gi= it->second;
	gi.UsedThisFrame= true;

	// update infos
	gi.ViewText->setText(text);
	gi.ViewText->setColor(color);
	gi.GroupInScene->Position= pos;
	gi.GroupInScene->Scale= scale;
	gi.GroupInScene->setOffsetX(offsetX);
}

// ***************************************************************************
void	CFlyingTextManager::releaseNotUsedFlyingText()
{
	// for all elements of inscene current
	TInSceneCurrentMap::iterator	it= _InSceneCurrent.begin();
	for(;it!=_InSceneCurrent.end();)
	{
		// if used this frame, don't remove
		if(it->second.UsedThisFrame)
		{
			it->second.UsedThisFrame= false;
			it++;
		}
		// else new free slot
		else
		{
			// erase from interface
			unlinkToInterface(it->second);
			// back to pool
			_InScenePool.push_back(it->second);
			// erase from map
			TInSceneCurrentMap::iterator	itTmp= it++;
			_InSceneCurrent.erase(itTmp);
		}
	}
}


// ***************************************************************************
void	CFlyingTextManager::linkToInterface(CGroupInfo &gi)
{
	if(!_Root)
		return;

	// should not be linked
	nlassert(gi.GroupInScene->getParent()==NULL);

	// add to group and window list
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", gi.GroupInScene);
	gi.GroupInScene->setParent(_Root);
	_Root->addGroup (gi.GroupInScene);
}

// ***************************************************************************
void	CFlyingTextManager::unlinkToInterface(CGroupInfo &gi)
{
	if(!_Root)
		return;

	// should be linked
	nlassert(gi.GroupInScene->getParent()!=NULL);

	// remove from group and window list
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CWidgetManager::getInstance()->unMakeWindow(gi.GroupInScene);
	// remove but don't delete
	gi.GroupInScene->getParent()->delGroup(gi.GroupInScene, true);
	gi.GroupInScene->setParent(NULL);
}

