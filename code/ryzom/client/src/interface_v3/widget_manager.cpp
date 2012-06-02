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

#include "widget_manager.h"
#include "interface_group.h"
#include "group_container.h"
#include "group_in_scene.h"
#include "view_pointer.h"

CWidgetManager* CWidgetManager::instance = NULL;
std::string CWidgetManager::_CtrlLaunchingModalId= "ctrl_launch_modal";
IParser* CWidgetManager::parser = NULL;

// ----------------------------------------------------------------------------
// SMasterGroup
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
void CWidgetManager::SMasterGroup::addWindow(CInterfaceGroup *pIG, uint8 nPrio)
{
	nlassert(nPrio<WIN_PRIORITY_MAX);

	// Priority WIN_PRIORITY_WORLD_SPACE is only for CGroupInScene !
	// Add this group in another priority list
	nlassert ((nPrio!=WIN_PRIORITY_MAX) || (dynamic_cast<CGroupInScene*>(pIG)!=NULL));

	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			// If the element already exists in the list return !
			if (*it == pIG)
				return;
			it++;
		}
	}
	PrioritizedWindows[nPrio].push_back(pIG);
}

// ----------------------------------------------------------------------------
void CWidgetManager::SMasterGroup::delWindow(CInterfaceGroup *pIG)
{
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			if ((*it) == pIG)
			{
				PrioritizedWindows[i].erase(it);
				return;
			}
			it++;
		}
	}
}

// ----------------------------------------------------------------------------
CInterfaceGroup* CWidgetManager::SMasterGroup::getWindowFromId(const std::string &winID)
{
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			if ((*it)->getId() == winID)
				return *it;
			it++;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
bool CWidgetManager::SMasterGroup::isWindowPresent(CInterfaceGroup *pIG)
{
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			if ((*it) == pIG)
				return true;
			it++;
		}
	}
	return false;
}

// Set a window top in its priority queue
// ----------------------------------------------------------------------------
void CWidgetManager::SMasterGroup::setTopWindow(CInterfaceGroup *pIG)
{
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			if (*it == pIG)
			{
				PrioritizedWindows[i].erase(it);
				PrioritizedWindows[i].push_back(pIG);
				LastTopWindowPriority= i;
				return;
			}
			it++;
		}
	}
	// todo hulud interface syntax error
	nlwarning("window %s do not exist in a priority list", pIG->getId().c_str());
}

// ----------------------------------------------------------------------------
void CWidgetManager::SMasterGroup::setBackWindow(CInterfaceGroup *pIG)
{
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			if (*it == pIG)
			{
				PrioritizedWindows[i].erase(it);
				PrioritizedWindows[i].push_front(pIG);
				return;
			}
			it++;
		}
	}
	// todo hulud interface syntax error
	nlwarning("window %s do not exist in a priority list", pIG->getId().c_str());
}

// ----------------------------------------------------------------------------
void CWidgetManager::SMasterGroup::deactiveAllContainers()
{
	std::vector<CGroupContainer*> gcs;

	// Make first a list of all window (Warning: all group container are not window!)
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(*it);
			if (pGC != NULL)
				gcs.push_back(pGC);
			it++;
		}
	}

	// Then hide them. Must do this in 2 times, because setActive(false) change PrioritizedWindows,
	// and hence invalidate its.
	for (uint32 i = 0; i < gcs.size(); ++i)
	{
		gcs[i]->setActive(false);
	}
}

// ----------------------------------------------------------------------------
void CWidgetManager::SMasterGroup::centerAllContainers()
{
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(*it);
			if ((pGC != NULL) && (pGC->getParent() != NULL))
			{
				sint32 wParent = pGC->getParent()->getW(false);
				sint32 w = pGC->getW(false);
				pGC->setXAndInvalidateCoords((wParent - w) / 2);
				sint32 hParent = pGC->getParent()->getH(false);
				sint32 h = pGC->getH(false);
				pGC->setYAndInvalidateCoords(h+(hParent - h) / 2);
			}

			it++;
		}
	}
}

// ----------------------------------------------------------------------------
void CWidgetManager::SMasterGroup::unlockAllContainers()
{
	for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
	{
		std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
		while (it != PrioritizedWindows[i].end())
		{
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(*it);
			if (pGC != NULL)
				pGC->setLocked(false);

			it++;
		}
	}
}

class CElementToSort
{
public:
	CInterfaceGroup *pIG;
	float	Distance;
	bool operator< (const CElementToSort& other) const
	{
		// We want first farest views
		return Distance > other.Distance;
	}
};

void CWidgetManager::SMasterGroup::sortWorldSpaceGroup ()
{
	static std::vector<CElementToSort> sortTable;
	sortTable.clear ();

	// Fill the sort table
	std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[WIN_PRIORITY_WORLD_SPACE].begin();
	while (it != PrioritizedWindows[WIN_PRIORITY_WORLD_SPACE].end())
	{
		sortTable.push_back (CElementToSort ());
		CElementToSort &elm = sortTable.back();
		elm.pIG = *it;
		elm.Distance = (static_cast<CGroupInScene*>(*it))->getDepthForZSort();

		it++;
	}

	// Sort the table
	std::sort (sortTable.begin(), sortTable.end());

	// Fill the final table
	uint i = 0;
	it = PrioritizedWindows[WIN_PRIORITY_WORLD_SPACE].begin();
	while (it != PrioritizedWindows[WIN_PRIORITY_WORLD_SPACE].end())
	{
		*it = sortTable[i].pIG;

		it++;
		i++;
	}
}


CWidgetManager* CWidgetManager::getInstance()
{
	if( instance == NULL )
		instance = new CWidgetManager;

	return instance;
}

void CWidgetManager::release()
{
	delete instance;
	instance = NULL;
}

// ----------------------------------------------------------------------------
CInterfaceGroup* CWidgetManager::getMasterGroupFromId (const std::string &MasterGroupName)
{
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		if (_MasterGroups[i].Group->getId() == MasterGroupName)
			return _MasterGroups[i].Group;
	}
	return NULL;
}

// ----------------------------------------------------------------------------
CInterfaceGroup* CWidgetManager::getWindowFromId (const std::string & groupId)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		CInterfaceGroup *pIG = rMG.getWindowFromId(groupId);
		if (pIG != NULL)
			return pIG;
	}
	return NULL;
}

// ----------------------------------------------------------------------------
void CWidgetManager::addWindowToMasterGroup (const std::string &sMasterGroupName, CInterfaceGroup *pIG)
{
	// Warning this function is not smart : its a o(n) !
	if (pIG == NULL) return;
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); ++nMasterGroup)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getId() == sMasterGroupName)
		{
			rMG.addWindow(pIG, pIG->getPriority());
		}
	}
}

// ----------------------------------------------------------------------------
void CWidgetManager::removeWindowFromMasterGroup(const std::string &sMasterGroupName,CInterfaceGroup *pIG)
{
	// Warning this function is not smart : its a o(n) !
	if (pIG == NULL) return;
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); ++nMasterGroup)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getId() == sMasterGroupName)
		{
			rMG.delWindow(pIG);
		}
	}
}

void unlinkAllContainers (CInterfaceGroup *pIG)
{
	const std::vector<CInterfaceGroup*> &rG = pIG->getGroups();
	for(uint i = 0; i < rG.size(); ++i)
		unlinkAllContainers (rG[i]);

	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(pIG);
	if (pGC != NULL)
		pGC->removeAllContainers();
}

// ***************************************************************************
void CWidgetManager::removeAllMasterGroups()
{
	uint i;

	for (i = 0; i < _MasterGroups.size(); ++i)
		unlinkAllContainers (_MasterGroups[i].Group);

	// Yoyo: important to not Leave NULL in the array, because of CGroupHTML and LibWWW callback
	// that may call CInterfaceManager::getElementFromId() (and this method hates having NULL in the arrays ^^)
	while(!_MasterGroups.empty())
	{
		delete _MasterGroups.back().Group;
		_MasterGroups.pop_back();
	}

}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::activateMasterGroup (const std::string &sMasterGroupName, bool bActive)
{
	CInterfaceGroup *pIG = CWidgetManager::getInstance()->getMasterGroupFromId (sMasterGroupName);
	if (pIG != NULL)
	{
		pIG->setActive(bActive);
		pIG->invalidateCoords();
	}
}

// ------------------------------------------------------------------------------------------------
CInterfaceGroup* CWidgetManager::getWindow(CInterfaceElement *pIE)
{
	CInterfaceGroup *pIG = pIE->getParent();
	if (pIG == NULL) return NULL;
	if (pIG->getParent() == NULL) return NULL;
	while (pIG->getParent()->getParent() != NULL)
	{
		pIG = pIG->getParent();
	}
	return pIG;
}


// ------------------------------------------------------------------------------------------------
CInterfaceElement* CWidgetManager::getElementFromId (const std::string &sEltId)
{
	// System special
	if(sEltId == _CtrlLaunchingModalId)
		return getCtrlLaunchingModal();

	// Search for all elements
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		CInterfaceElement *pIEL = rMG.Group->getElement (sEltId);
		if (pIEL != NULL)
			return pIEL;
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
CInterfaceElement* CWidgetManager::getElementFromId (const std::string &sStart, const std::string &sEltId)
{
	CInterfaceElement *pIEL = getElementFromId (sEltId);
	if (pIEL == NULL)
	{
		std::string sZeStart = sStart, sTmp;
		if (sZeStart[sZeStart.size()-1] == ':')
			sZeStart = sZeStart.substr(0, sZeStart.size()-1);

		while (sZeStart != "")
		{
			if (sEltId[0] == ':')
				sTmp = sZeStart	+ sEltId;
			else
				sTmp = sZeStart	+ ":" + sEltId;
			pIEL = getElementFromId (sTmp);
			if (pIEL != NULL)
				return pIEL;
			std::string::size_type nextPos = sZeStart.rfind(':');
			if (nextPos == std::string::npos) break;
			sZeStart = sZeStart.substr(0, nextPos);
		}
	}
	return pIEL;
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::setTopWindow (CInterfaceGroup* win)
{
	//find the window in the window list
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
			rMG.setTopWindow(win);
	}
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::setBackWindow(CInterfaceGroup* win)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
			rMG.setBackWindow(win);
	}
}

// ------------------------------------------------------------------------------------------------
CInterfaceGroup* CWidgetManager::getTopWindow (uint8 nPriority) const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			// return the first.
			if(rMG.PrioritizedWindows[nPriority].empty())
				return NULL;
			else
				return rMG.PrioritizedWindows[nPriority].back();
		}
	}
	return NULL;
}


// ------------------------------------------------------------------------------------------------
CInterfaceGroup* CWidgetManager::getBackWindow (uint8 nPriority) const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			// return the first.
			if(rMG.PrioritizedWindows[nPriority].empty())
				return NULL;
			else
				return rMG.PrioritizedWindows[nPriority].front();
		}
	}
	return NULL;
}

// ***************************************************************************
CInterfaceGroup* CWidgetManager::getLastEscapableTopWindow() const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
			{
				const std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				std::list<CInterfaceGroup*>::const_reverse_iterator	it;
				it= rList.rbegin();
				for(;it!=rList.rend();it++)
				{
					if((*it)->getActive() && (*it)->getEscapable())
						return *it;
				}
			}
		}
	}
	return NULL;
}

// ***************************************************************************
void CWidgetManager::setWindowPriority (CInterfaceGroup *pWin, uint8 nNewPriority)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			if (rMG.isWindowPresent(pWin))
			{
				rMG.delWindow(pWin);
				rMG.addWindow(pWin, nNewPriority);
			}
		}
	}
}

// ***************************************************************************
uint8 CWidgetManager::getLastTopWindowPriority() const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			return rMG.LastTopWindowPriority;
		}
	}
	return 0;
}

bool CWidgetManager::hasModal() const
{
	if( !_ModalStack.empty() )
		return true;
	else
		return false;
}

CWidgetManager::SModalWndInfo& CWidgetManager::getModal()
{
	return _ModalStack.back();
}

bool CWidgetManager::isPreviousModal( CInterfaceGroup *wnd ) const
{
	std::vector< SModalWndInfo >::size_type s = _ModalStack.size();
	for( std::vector< SModalWndInfo >::size_type i = 0; i < s; i++ )
		if( _ModalStack[ i ].ModalWindow == wnd )
			return true;

	return false;
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::enableModalWindow (CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG)
{
	// disable any modal before. release keyboard
	disableModalWindow();
	pushModalWindow(ctrlLaunchingModal, pIG);
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::enableModalWindow (CCtrlBase *CtrlLaunchingModal, const std::string &groupName)
{
	CInterfaceGroup	*group= dynamic_cast<CGroupModal*>( getElementFromId(groupName) );
	if(group)
	{
		// enable the modal
		enableModalWindow(CtrlLaunchingModal, group);
	}
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::disableModalWindow ()
{
	while (!_ModalStack.empty())
	{
		SModalWndInfo winInfo = _ModalStack.back();
		_ModalStack.pop_back(); // must pop back as early as possible because 'setActive' may trigger another 'popModalWindow', leading to a crash
		// disable old modal window
		if(winInfo.ModalWindow)
		{
			setBackWindow(winInfo.ModalWindow);
			winInfo.ModalWindow->setActive(false);
		}
	}

	// disable any context help
	setCurContextHelp( NULL );
	CWidgetManager::getInstance()->_DeltaTimeStopingContextHelp = 0;
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::pushModalWindow(CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG)
{
	// enable the wanted modal
	if(pIG)
	{
		SModalWndInfo mwi;
		mwi.ModalWindow = pIG;
		mwi.CtrlLaunchingModal = ctrlLaunchingModal;
		// setup special group
		CGroupModal		*groupModal= dynamic_cast<CGroupModal*>(pIG);
		if(groupModal)
		{
			mwi.ModalExitClickOut = groupModal->ExitClickOut;
			mwi.ModalExitClickL = groupModal->ExitClickL;
			mwi.ModalExitClickR = groupModal->ExitClickR;
			mwi.ModalHandlerClickOut = groupModal->OnClickOut;
			mwi.ModalClickOutParams = groupModal->OnClickOutParams;
			mwi.ModalExitKeyPushed = groupModal->ExitKeyPushed;
			// update coords of the modal
			if(groupModal->SpawnOnMousePos)
			{
				groupModal->SpawnMouseX = _Pointer->getX();
				groupModal->SpawnMouseY = _Pointer->getY();
			}
		}
		else
		{
			// default for group not modal. Backward compatibility
			mwi.ModalExitClickOut = false;
			mwi.ModalExitClickL = false;
			mwi.ModalExitClickR = false;
			mwi.ModalExitKeyPushed = false;
		}

		_ModalStack.push_back(mwi);

		// update coords and activate the modal
		mwi.ModalWindow->invalidateCoords();
		mwi.ModalWindow->setActive(true);
		setTopWindow(mwi.ModalWindow);
	}
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::pushModalWindow(CCtrlBase *ctrlLaunchingModal, const std::string &groupName)
{
	CInterfaceGroup	*group= dynamic_cast<CGroupModal*>( getElementFromId(groupName) );
	if(group)
	{
		// enable the modal
		enableModalWindow(ctrlLaunchingModal, group);
	}
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::popModalWindow()
{
	if (!_ModalStack.empty())
	{
		SModalWndInfo winInfo = _ModalStack.back();
		_ModalStack.pop_back(); // must pop back as early as possible because 'setActive' may trigger another 'popModalWindow', leading to a crash
		if(winInfo.ModalWindow)
		{
			setBackWindow(winInfo.ModalWindow);
			winInfo.ModalWindow->setActive(false);
		}
		if (!_ModalStack.empty())
		{
			if(_ModalStack.back().ModalWindow)
			{
				_ModalStack.back().ModalWindow->setActive(true);
				setTopWindow(_ModalStack.back().ModalWindow);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CWidgetManager::popModalWindowCategory(const std::string &category)
{
	for(;;)
	{
		if (_ModalStack.empty()) break;
		if (!_ModalStack.back().ModalWindow) break;
		CGroupModal *gm = dynamic_cast<CGroupModal *>((CInterfaceGroup*)(_ModalStack.back().ModalWindow));
		if (gm && gm->Category == category)
		{
			_ModalStack.back().ModalWindow->setActive(false);
			_ModalStack.pop_back();
		}
		else
		{
			break;
		}
	}
}


CWidgetManager::CWidgetManager()
{
	_Pointer = NULL;
	curContextHelp = NULL;
}

CWidgetManager::~CWidgetManager()
{
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		delete _MasterGroups[i].Group;
	}

	_Pointer = NULL;
	curContextHelp = NULL;
}