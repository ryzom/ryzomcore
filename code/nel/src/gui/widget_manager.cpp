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

#include "nel/gui/db_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_pointer_base.h"
#include "nel/gui/ctrl_draggable.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/group_modal.h"
#include "nel/gui/group_editbox_base.h"
#include "nel/gui/interface_options.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_bitmap.h"

namespace NLGUI
{
	void LinkHack();
}

namespace
{
	const uint DOUBLE_CLICK_MIN = 50;
	const uint DOUBLE_CLICK_MAX = 750;
	const float ROLLOVER_MIN_DELTA_PER_MS = 0.28f;
	const float ROLLOVER_MAX_DELTA_PER_MS = 0.12f;

	void Hack()
	{
		NLGUI::LinkHack();
	}
}

namespace NLGUI
{
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
		nlassert ((nPrio!=WIN_PRIORITY_MAX) || pIG->isGroupInScene() );

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
		std::vector<CGroupContainerBase*> gcs;

		// Make first a list of all window (Warning: all group container are not window!)
		for (uint8 i = 0; i < WIN_PRIORITY_MAX; ++i)
		{
			std::list<CInterfaceGroup*>::iterator it = PrioritizedWindows[i].begin();
			while (it != PrioritizedWindows[i].end())
			{
				CGroupContainerBase *pGC = dynamic_cast<CGroupContainerBase*>(*it);
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
				CGroupContainerBase *pGC = dynamic_cast<CGroupContainerBase*>(*it);
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
				CGroupContainerBase *pGC = dynamic_cast<CGroupContainerBase*>(*it);
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
			elm.Distance = (*it)->getDepthForZSort();

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

		CGroupContainerBase *pGC = dynamic_cast<CGroupContainerBase*>(pIG);
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

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup* CWidgetManager::getWindowUnder (sint32 x, sint32 y)
	{
		H_AUTO (RZ_Interface_Window_Under )

		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
				{
					const std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
					std::list<CInterfaceGroup*>::const_reverse_iterator itw;
					for (itw = rList.rbegin(); itw != rList.rend(); itw++)
					{
						CInterfaceGroup *pIG = *itw;
						if (pIG->getActive() && pIG->getUseCursor())
						{
							if (pIG->isWindowUnder (x, y))
								return pIG;
						}
					}
				}
			}
		}
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup* CWidgetManager::getGroupUnder (sint32 x, sint32 y)
	{
		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
				{
					const std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
					std::list<CInterfaceGroup*>::const_reverse_iterator itw;
					for (itw = rList.rbegin(); itw != rList.rend(); itw++)
					{
						CInterfaceGroup *pIG = *itw;
						if (pIG->getActive() && pIG->getUseCursor())
						{
							CInterfaceGroup *pIGunder = pIG->getGroupUnder (x ,y);
							if (pIGunder != NULL)
								return pIGunder;
						}
					}
				}
			}
		}
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::getViewsUnder (sint32 x, sint32 y, std::vector<CViewBase*> &vVB)
	{
		vVB.clear ();

		// No Op if screen minimized
		if(CViewRenderer::getInstance()->isMinimized())
			return;

		uint32 sw, sh;
		CViewRenderer::getInstance()->getScreenSize(sw, sh);
		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
				{
					const std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
					std::list<CInterfaceGroup*>::const_reverse_iterator itw;
					for (itw = rList.rbegin(); itw != rList.rend(); itw++)
					{
						CInterfaceGroup *pIG = *itw;

						// Accecpt if not modal clip
						if (pIG->getActive() && pIG->getUseCursor())
						{
							if (pIG->getViewsUnder (x, y, 0, 0, (sint32) sw, (sint32) sh, vVB))
								return ;
						}
					}
				}
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::getCtrlsUnder (sint32 x, sint32 y, std::vector<CCtrlBase*> &vICL)
	{
		vICL.clear ();

		// No Op if screen minimized
		if(CViewRenderer::getInstance()->isMinimized())
			return;

		uint32 sw, sh;
		CViewRenderer::getInstance()->getScreenSize(sw, sh);
		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0 ; nPriority--)
				{
					const std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
					std::list<CInterfaceGroup*>::const_reverse_iterator itw;
					for (itw = rList.rbegin(); itw != rList.rend(); itw++)
					{
						CInterfaceGroup *pIG = *itw;

						// Accecpt if not modal clip
						if (!CWidgetManager::getInstance()->hasModal() || CWidgetManager::getInstance()->getModal().ModalWindow == pIG || CWidgetManager::getInstance()->getModal().ModalExitClickOut)
						if (pIG->getActive() && pIG->getUseCursor())
						{
							if (pIG->getCtrlsUnder (x, y, 0, 0, (sint32) sw, (sint32) sh, vICL))
								return;
						}
					}
				}
			}
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::getGroupsUnder (sint32 x, sint32 y, std::vector<CInterfaceGroup *> &vIGL)
	{
		vIGL.clear ();

		// No Op if screen minimized
		if(CViewRenderer::getInstance()->isMinimized())
			return;

		uint32 sw, sh;
		CViewRenderer::getInstance()->getScreenSize(sw, sh);
		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0 ; nPriority--)
				{
					const std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
					std::list<CInterfaceGroup*>::const_reverse_iterator itw;
					for (itw = rList.rbegin(); itw != rList.rend(); itw++)
					{
						CInterfaceGroup *pIG = *itw;

						// Accecpt if not modal clip
						if (!CWidgetManager::getInstance()->hasModal() || CWidgetManager::getInstance()->getModal().ModalWindow == pIG ||
							CWidgetManager::getInstance()->getModal().ModalExitClickOut)
						if (pIG->getActive() && pIG->getUseCursor())
						{
							if (pIG->isIn(x, y))
							{
								vIGL.push_back(pIG);
								pIG->getGroupsUnder (x, y, 0, 0, (sint32) sw, (sint32) sh, vIGL);
								return;
							}
						}
					}
				}
			}
		}
	}


	// ***************************************************************************
	void CWidgetManager::removeRefOnView( CViewBase *viewBase )
	{
		uint i;
		for (i=0; i<_ViewsUnderPointer.size(); i++)
		{
			if (_ViewsUnderPointer[i] == viewBase)
			{
				_ViewsUnderPointer.erase (_ViewsUnderPointer.begin()+i);
				i--;
			}
		}
	}

	// ***************************************************************************
	void CWidgetManager::removeRefOnCtrl(CCtrlBase *ctrlBase)
	{
		if ( getCurContextHelp()  == ctrlBase)
			setCurContextHelp( NULL );
		if (getCapturePointerLeft() == ctrlBase)
			setCapturePointerLeft(NULL);
		if (getCapturePointerRight() == ctrlBase)
			setCapturePointerRight (NULL);
		if (getCaptureKeyboard() == ctrlBase)
			setCaptureKeyboard(NULL);
		if (getOldCaptureKeyboard() == ctrlBase)
			setOldCaptureKeyboard(NULL);
		if (getDefaultCaptureKeyboard() == ctrlBase)
			setDefaultCaptureKeyboard(NULL);
		uint i;
		for (i=0; i<_CtrlsUnderPointer.size(); i++)
		{
			if (_CtrlsUnderPointer[i] == ctrlBase)
			{
				_CtrlsUnderPointer.erase (_CtrlsUnderPointer.begin()+i);
				i--;
			}
		}

		// Unregister from ClockMsgTargets
		unregisterClockMsgTarget(ctrlBase);
	}


	// ***************************************************************************
	void CWidgetManager::removeRefOnGroup (CInterfaceGroup *group)
	{
		uint i;
		for (i=0; i<_GroupsUnderPointer.size(); i++)
		{
			if (_GroupsUnderPointer[i] == group)
			{
				_GroupsUnderPointer.erase (_GroupsUnderPointer.begin()+i);
				i--;
			}
		}
	}


	void CWidgetManager::reset()
	{
		setCurContextHelp( NULL );
		
		_ViewsUnderPointer.clear();
		_CtrlsUnderPointer.clear();
		_GroupsUnderPointer.clear();

		_CaptureKeyboard = NULL;
		_OldCaptureKeyboard = NULL;
		setCapturePointerLeft(NULL);
		setCapturePointerRight(NULL);
		
		resetColorProps();

		_AlphaRolloverSpeedDB = NULL;
	}


	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::checkCoords()
	{
		H_AUTO ( RZ_Interface_validateCoords )

		uint32 nMasterGroup;

		{
			H_AUTO ( RZ_Interface_checkCoords )

			// checkCoords all the windows
			for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
			{
				CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
				if (rMG.Group->getActive())
				{
					for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
					{
						std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
						std::list<CInterfaceGroup*>::const_iterator itw;
						for (itw = rList.begin(); itw!= rList.end();)
						{
							CInterfaceGroup *pIG = *itw;
							itw++;	// since checkCoords invalidate the iterator, be sure we move to the next one before
							if (pIG->getActive())
								pIG->checkCoords ();
						}
					}
				}
			}
		}

		bool bRecomputeCtrlUnderPtr = false;
		{
			H_AUTO ( RZ_Interface_updateCoords )

			// updateCoords all the needed windows
			for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
			{
				CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
				if (rMG.Group->getActive())
				{
					for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
					{
						std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
						std::list<CInterfaceGroup*>::const_iterator itw;
						for (itw = rList.begin(); itw!= rList.end(); itw++)
						{
							CInterfaceGroup *pIG = *itw;
							bool		updateCoordCalled= false;
							// updateCoords the window only if the master group is his parent and if need it
							// do it until updateCoords() no more invalidate coordinates!!
							while (pIG->getParent()==rMG.Group && (pIG->getInvalidCoords()>0))
							{
								bRecomputeCtrlUnderPtr = true;
								// Update as many pass wanted (3 time for complex resizing, 1 for scroll for example)
								uint	numPass= pIG->getInvalidCoords();
								// reset before updateCoords
								pIG->resetInvalidCoords();
								for(uint i=0;i<numPass;i++)
								{
									pIG->updateCoords ();
								}
								updateCoordCalled= true;
							}
							// If the group need to update pos each frame (eg: CGroupInScene),
							// and updateCoords not called
							if(pIG->getParent()==rMG.Group && !updateCoordCalled && pIG->isNeedFrameUpdatePos())
							{
								// This Group will compute the delta to apply.
								pIG->onFrameUpdateWindowPos(0,0);
							}
						}
					}
				}
			}

			if ( CWidgetManager::getInstance()->getPointer() != NULL)
				CWidgetManager::getInstance()->getPointer()->updateCoords();
		}



		if (bRecomputeCtrlUnderPtr)
		{
			H_AUTO ( RZ_Interface_RecomputeCtrlUnderPtr )
			if ( CWidgetManager::getInstance()->getPointer() != NULL )
			{
				sint32 mx = _Pointer->getX();
				sint32 my = _Pointer->getY();
				getViewsUnder (mx, my, _ViewsUnderPointer);
				getCtrlsUnder (mx, my, _CtrlsUnderPointer);
				getGroupsUnder (mx, my, _GroupsUnderPointer);
				CInterfaceGroup *ptr = getWindowUnder (mx, my);
				_WindowUnder = ptr;
			}
		}
	}


	// ----------------------------------------------------------------------------
	CInterfaceGroup* CWidgetManager::getWindowForActiveMasterGroup( const std::string &window )
	{
		// Search for all elements
		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			const SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				CInterfaceElement	*pEL= getElementFromId( rMG.Group->getId() + ":" + window);
				if(pEL && pEL->isGroup())
					return (CInterfaceGroup*)pEL;
			}
		}

		return NULL;
	}


	// ***************************************************************************
	void CWidgetManager::drawOverExtendViewText()
	{
		if( getOverExtendViewText() )
		{
			CViewText	*vtSrc= dynamic_cast<CViewText*>( getOverExtendViewText() );

			CInterfaceGroup *groupOver = getWindowForActiveMasterGroup("over_extend_view_text");
			if(groupOver)
			{
				CViewText *vtDst = dynamic_cast<CViewText*>(groupOver->getView("text"));
				if (vtDst != NULL)
				{
					// Copy all aspects to the view
					vtDst->setText (vtSrc->getText());
					vtDst->setFontSize (vtSrc->getFontSize());
					vtDst->setColor (vtSrc->getColor());
					vtDst->setModulateGlobalColor(vtSrc->getModulateGlobalColor());
					vtDst->setShadow(vtSrc->getShadow());
					vtDst->setShadowColor(vtSrc->getShadowColor());
					vtDst->setCaseMode(vtSrc->getCaseMode());
					vtDst->setUnderlined(vtSrc->getUnderlined());

					// setup background
					CViewBitmap	*pBack= dynamic_cast<CViewBitmap*>(groupOver->getView("midback"));
					CViewBitmap	*pOutline= dynamic_cast<CViewBitmap*>(groupOver->getView("midoutline"));
					if(pBack)
						pBack->setColor( getOverExtendViewTextBackColor() );
					if(pOutline)
					{
						pOutline->setColor(vtSrc->getColor());
						pOutline->setModulateGlobalColor(vtSrc->getModulateGlobalColor());
					}

					// the group is the position of the overed text, but apply the delta of borders (vtDst X/Y)
					sint32 x = vtSrc->getXReal() - vtDst->getX();
					sint32 y = vtSrc->getYReal() - vtDst->getY();

					// update one time only to get correct W/H
					groupOver->updateCoords ();

					if(!vtSrc->isClampRight())
					{
						// clamped from the left part
						x += vtSrc->getWReal() - vtDst->getWReal();
					}

					// clamp to screen coords, and set
					if ((x+groupOver->getW()) > groupOver->getParent()->getWReal())
						x = groupOver->getParent()->getWReal() - groupOver->getW();
					if (x < 0)
						x = 0;
					if ((y+groupOver->getH()) > groupOver->getParent()->getHReal())
						y = groupOver->getParent()->getHReal() - groupOver->getH();
					if (y < 0)
						y = 0;

					// set pos
					groupOver->setX (x);
					groupOver->setY (y);

					// update coords 3 times is required
					groupOver->updateCoords ();
					groupOver->updateCoords ();
					groupOver->updateCoords ();

					// draw
					groupOver->draw ();
					// flush layers
					CViewRenderer::getInstance()->flush();
				}
			}

			// Reset the ptr so at next frame, won't be rendered (but if reset)
			setOverExtendViewText( NULL, getOverExtendViewTextBackColor() );
		}
	}


	uint CWidgetManager::adjustTooltipPosition( CCtrlBase *newCtrl, CInterfaceGroup *win, THotSpot ttParentRef,
												THotSpot ttPosRef, sint32 xParent, sint32 yParent,
												sint32 wParent, sint32 hParent )
	{
		CCtrlBase::TToolTipParentType	parentType= newCtrl->getToolTipParent();
		CInterfaceGroup *groupContextHelp = 
			getWindowForActiveMasterGroup(newCtrl->getContextHelpWindowName());

		uint32 _ScreenH, _ScreenW;
		CViewRenderer::getInstance()->getScreenSize( _ScreenH, _ScreenW );

		if(ttPosRef==Hotspot_TTAuto || ttParentRef==Hotspot_TTAuto)
		{
			// NB: keep the special window if type is specialwindow (defined above)
			if(!win)
				win= newCtrl->getRootWindow();
			sint32	xWin= 0;
			sint32	yWin= 0;
			sint32	wWin= 0;
			sint32	hWin= 0;
			if(win)
			{
				xWin = win->getXReal();
				yWin = win->getYReal();
				wWin = win->getWReal();
				hWin = win->getHReal();
			}
			// for Window, display top or bottom according to window pos/size
			if(parentType==CCtrlBase::TTWindow || parentType==CCtrlBase::TTSpecialWindow)
			{
				sint32	top= (sint32)_ScreenH - (yWin+hWin);
				sint32	bottom= yWin;
				if(top>bottom)
				{
					ttParentRef= Hotspot_TL;
					ttPosRef= Hotspot_BL;
				}
				else
				{
					ttParentRef= Hotspot_BL;
					ttPosRef= Hotspot_TL;
				}
			}
			// for Ctrl, display top, left or right according to window pos/size
			else if(parentType==CCtrlBase::TTCtrl)
			{
				sint32	right= (sint32)_ScreenW - (xWin+wWin);
				sint32	left= xWin;
				if(right>left)
				{
					ttParentRef= Hotspot_TR;
					ttPosRef= Hotspot_BL;
				}
				else
				{
					ttParentRef= Hotspot_TL;
					ttPosRef= Hotspot_BR;
				}
			}
			else
			{
				// default (mouse)
				ttParentRef= Hotspot_BL;
				ttPosRef= Hotspot_BL;
			}
		}

		// **** compute coordinates of the tooltip
		sint32 x= xParent;
		sint32 y= yParent;
		if (ttParentRef & Hotspot_Mx)
			y += hParent/2;
		if (ttParentRef & Hotspot_Tx)
			y += hParent;
		if (ttParentRef & Hotspot_xM)
			x += wParent/2;
		if (ttParentRef & Hotspot_xR)
			x += wParent;

		// adjust according to self posref
		if (ttPosRef & Hotspot_Mx)
			y -= groupContextHelp->getHReal()/2;
		if (ttPosRef & Hotspot_Tx)
			y -= groupContextHelp->getHReal();
		if (ttPosRef & Hotspot_xM)
			x -= groupContextHelp->getWReal()/2;
		if (ttPosRef & Hotspot_xR)
			x -= groupContextHelp->getWReal();


		// **** clamp to screen coords, and set
		uint clampCount = 0;

		if ((x+groupContextHelp->getW()) > groupContextHelp->getParent()->getWReal())
		{
			++ clampCount;
			x = groupContextHelp->getParent()->getWReal() - groupContextHelp->getW();
		}
		if (x < 0)
		{
			x = 0;
			++ clampCount;
		}
		if ((y+groupContextHelp->getH()) > groupContextHelp->getParent()->getHReal())
		{
			y = groupContextHelp->getParent()->getHReal() - groupContextHelp->getH();
			++ clampCount;
		}
		if (y < 0)
		{
			y = 0;
			++ clampCount;
		}

		// update coords 3 times is required
		groupContextHelp->setX (x);
		groupContextHelp->setY (y);
		groupContextHelp->updateCoords ();
		groupContextHelp->updateCoords ();
		groupContextHelp->updateCoords ();

		return clampCount;
	}

	// ----------------------------------------------------------------------------
	void CWidgetManager::updateTooltipCoords()
	{
		updateTooltipCoords( getCurContextHelp() );
	}

	void CWidgetManager::updateTooltipCoords( CCtrlBase *newCtrl )
	{
		if (!newCtrl) return;
		if (!newCtrl->getInvalidCoords()) return;
		
		CInterfaceGroup *groupContextHelp =
			getWindowForActiveMasterGroup(newCtrl->getContextHelpWindowName());

		if(groupContextHelp)
		{
			CViewText *pTxt = (CViewText*)groupContextHelp->getView("text");
			if (pTxt != NULL)
			{
				pTxt->setTextFormatTaged(_ContextHelpText);
				// update only to get correct W/H
				groupContextHelp->updateCoords ();


				// **** Compute parent coordinates
				CCtrlBase::TToolTipParentType	parentType= newCtrl->getToolTipParent();
				CInterfaceGroup	*win= NULL;
				// adjust to the mouse by default
				sint32		xParent= getPointer()->getX();
				sint32		yParent= getPointer()->getY();
				sint32		wParent= 0;
				sint32		hParent= 0;
				// adjust to the window
				if(parentType==CCtrlBase::TTWindow || parentType==CCtrlBase::TTSpecialWindow)
				{
					if(parentType==CCtrlBase::TTWindow)
						win= newCtrl->getRootWindow();
					else
						win =
						dynamic_cast<CInterfaceGroup*>( getElementFromId(newCtrl->getToolTipSpecialParent()));

					if(win)
					{
						xParent = win->getXReal();
						yParent = win->getYReal();
						wParent = win->getWReal();
						hParent = win->getHReal();
					}
					// Bug...: leave default to pointer
				}
				// adjust to the ctrl
				else if (parentType==CCtrlBase::TTCtrl)
				{
					xParent = newCtrl->getXReal();
					yParent = newCtrl->getYReal();
					wParent = newCtrl->getWReal();
					hParent = newCtrl->getHReal();
					// Additionaly, must clip this ctrl with its parent
					// (else animals are buggy for instance)
					CInterfaceGroup	*parent= newCtrl->getParent();
					if(parent)
					{
						sint32	xClip,yClip,wClip,hClip;
						parent->getClip(xClip,yClip,wClip,hClip);
						// clip bottom left
						xParent= std::max(xParent, xClip);
						yParent= std::max(yParent, yClip);
						// clip top right
						sint32	xrParent= std::min(xParent+ wParent, xClip+wClip);
						sint32	ytParent= std::min(yParent+ hParent, yClip+hClip);
						wParent= std::max((sint32)0, xrParent-xParent);
						hParent= std::max((sint32)0, ytParent-yParent);
					}
				}


				// **** resolve auto posref
				uint clampCount = 
					adjustTooltipPosition( newCtrl, win, newCtrl->getToolTipParentPosRef(),
										newCtrl->getToolTipPosRef(), xParent, yParent,
										wParent, hParent);

				if (clampCount != 0)
				{
					// try to fallback on alternate tooltip posref
					uint altClampCount = 
						adjustTooltipPosition( newCtrl, win, newCtrl->getToolTipParentPosRefAlt(),
												newCtrl->getToolTipPosRefAlt(), xParent, yParent,
												wParent, hParent);

					if (altClampCount > clampCount)
					{
						// worst ? resume to first posref
						adjustTooltipPosition( newCtrl, win, newCtrl->getToolTipParentPosRef(),
												newCtrl->getToolTipPosRef(), xParent, yParent,
												wParent, hParent);
					}
				}
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::disableContextHelp()
	{
		setCurContextHelp( NULL );
		_DeltaTimeStopingContextHelp = 0;
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::disableContextHelpForControl( CCtrlBase *pCtrl )
	{
		if( pCtrl == NULL )
			return;

		if( getCurContextHelp() == pCtrl )
			disableContextHelp();
	}

	// ----------------------------------------------------------------------------
	CCtrlBase* CWidgetManager::getNewContextHelpCtrl()
	{
		// get the top most ctrl under us
		CCtrlBase *best = NULL;
		sint8 bestRenderLayer = -128;

		for (sint i = (sint32)_CtrlsUnderPointer.size()-1; i>=0; i--)
		{
			CCtrlBase	*pICL = _CtrlsUnderPointer[i];
			if (pICL->getRenderLayer() > bestRenderLayer)
			{
				if ((pICL->getActive()) && (!pICL->emptyContextHelp()))
				{
					if (!getPointer()) return pICL;
					sint32 mx, my;
					getPointer()->getPointerPos(mx, my);
					if (pICL->preciseHitTest(mx, my))
					{
						best = pICL;
						bestRenderLayer	= pICL->getRenderLayer();
					}
				}
			}
		}
		if (!best)
		{
			// if a control was not found, try with the groups
			sint8 bestRenderLayer = -128;

			for (sint i = (sint32)_GroupsUnderPointer.size()-1; i>=0; i--)
			{
				CCtrlBase	*pICL = _GroupsUnderPointer[i];
				if (pICL->getRenderLayer() > bestRenderLayer)
				{
					if ((pICL->getActive()) && (!pICL->emptyContextHelp()))
					{
						if (!getPointer()) return pICL;
						sint32 mx, my;
						getPointer()->getPointerPos(mx, my);
						if (pICL->preciseHitTest(mx, my))
						{
							best = pICL;
							bestRenderLayer	= pICL->getRenderLayer();
						}
					}
				}
			}
		}
		return best;
	}

	// ----------------------------------------------------------------------------
	void CWidgetManager::drawContextHelp ()
	{
		if (!getPointer() || !_ContextHelpActive)
			return;


		sint32 x = getPointer()->getX();
		sint32 y = getPointer()->getY();


		// ***************
		// **** try to disable
		// ***************
		// test disable first, so can recheck asap if another present. see below
		CCtrlBase *_CurCtrlContextHelp = getCurContextHelp();
		if( _CurCtrlContextHelp)
		{
			if(x!=_LastXContextHelp || y!=_LastYContextHelp)
			{
				// May change of ctrl!! => disable context help
				CCtrlBase	*newCtrl= getNewContextHelpCtrl();
				if(newCtrl!=_CurCtrlContextHelp)
				{
					// disable
					disableContextHelp();
				}
			}

			// Check if _CurCtrlContextHelp  is visible
			if (_CurCtrlContextHelp == NULL)
			{
				disableContextHelp();
			}
			else
			{
				bool bVisible = true;
				if (_CurCtrlContextHelp->getActive() == false)
					bVisible = false;
				CInterfaceGroup *pParent = _CurCtrlContextHelp->getParent();
				while (pParent != NULL)
				{
					if (pParent->getActive() == false)
						bVisible = false;
					pParent = pParent->getParent();
				}
				if (!bVisible)
					disableContextHelp();
			}
		}


		// ***************
		// **** try to acquire
		// ***************
		if(!_CurCtrlContextHelp)
		{
			// get the ctrl of interset
			CCtrlBase	*newCtrl= getNewContextHelpCtrl();

			if(x==_LastXContextHelp && y==_LastYContextHelp)
				_DeltaTimeStopingContextHelp += ( interfaceTimes.frameDiffMs / 1000.0f );
			else
				_DeltaTimeStopingContextHelp = 0;

			// If reach the time limit
			if( ( _DeltaTimeStopingContextHelp > _MaxTimeStopingContextHelp )
				|| (newCtrl && newCtrl->wantInstantContextHelp()))
			{
				// if present, get the ctx help text.
				if(newCtrl)
				{
					// get the text
					//newCtrl->getContextHelpToolTip(_ContextHelpText);
					newCtrl->getContextHelp( getContextHelpText() );
					// UserDefined context help
					if( !newCtrl->getContextHelpActionHandler().empty() )
					{
						CAHManager::getInstance()->runActionHandler(newCtrl->getContextHelpActionHandler(), newCtrl, newCtrl->getContextHelpAHParams() );
					}

					// If the text is finally empty (Special AH case), abort
					if( getContextHelpText().empty() )
						newCtrl= NULL;
				}

				// not present? wait furthermore to move the mouse.
				if(!newCtrl)
					_DeltaTimeStopingContextHelp= 0;
				else
				{
					// enable
					setCurContextHelp( newCtrl );
					newCtrl->invalidateCoords();
				}
			}
		}

		updateTooltipCoords(_CurCtrlContextHelp);


		// ***************
		// **** display
		// ***************
		if(_CurCtrlContextHelp)
		{
			CInterfaceGroup *groupContextHelp =
				getWindowForActiveMasterGroup(_CurCtrlContextHelp->getContextHelpWindowName());

			if(groupContextHelp)
			{
				/** If there's a modal box around, should be sure that the context help doesn't intersect it.
				  * If this is the case, we just disable it, unless the tooltip was generated by the current modal window
				  */
				if ( hasModal() )
				{
					CInterfaceGroup *mw = getModal().ModalWindow;
					if (mw && mw->isIn(*groupContextHelp))
					{
						if (_CurCtrlContextHelp->isSonOf(mw))
						{
							groupContextHelp->executeLuaScriptOnDraw();
							groupContextHelp->draw ();
							// flush layers
							CViewRenderer::getInstance()->flush();
						}
					}
					else
					{
						groupContextHelp->executeLuaScriptOnDraw();
						groupContextHelp->draw ();
						// flush layers
						CViewRenderer::getInstance()->flush();
					}
				}
				else
				{
					groupContextHelp->executeLuaScriptOnDraw();
					groupContextHelp->draw ();
					// flush layers
					CViewRenderer::getInstance()->flush();
				}
			}
		}

		// Bkup movement
		_LastXContextHelp= x;
		_LastYContextHelp= y;
	}

	void CWidgetManager::setContextHelpActive(bool active)
	{
		if (!active)
		{
			disableContextHelp();
		}
		_ContextHelpActive = active;
	}
	
	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::movePointer (sint32 dx, sint32 dy)
	{
		if (!_Pointer)
			return;

		uint32 nScrW, nScrH;
		sint32 oldpx, oldpy, newpx, newpy, disppx, disppy, olddisppx, olddisppy;

		CViewRenderer::getInstance()->getScreenSize (nScrW, nScrH);
		_Pointer->getPointerPos (oldpx, oldpy);

		olddisppx = oldpx;
		olddisppy = oldpy;

		newpx = oldpx + dx;
		newpy = oldpy + dy;

		if (newpx < 0) newpx = 0;
		if (newpy < 0) newpy = 0;
		if (newpx > (sint32)nScrW) newpx = nScrW;
		if (newpy > (sint32)nScrH) newpy = nScrH;
		dx = newpx - oldpx;
		dy = newpy - oldpy;

		disppx = newpx;
		disppy = newpy;

		_Pointer->setPointerPos (newpx, newpy);
		_Pointer->setPointerDispPos (disppx, disppy);

		// must get back coordinates because of snapping
		sint32 mx = _Pointer->getX();
		sint32 my = _Pointer->getY();
		getViewsUnder (mx, my, _ViewsUnderPointer);
		getCtrlsUnder (mx, my, _CtrlsUnderPointer);
		getGroupsUnder (mx, my, _GroupsUnderPointer);
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::movePointerAbs(sint32 px, sint32 py)
	{
		if(!CWidgetManager::getInstance()->getPointer())
			return;

		uint32 nScrW, nScrH;
		CViewRenderer::getInstance()->getScreenSize (nScrW, nScrH);
		NLMISC::clamp(px, 0, (sint32) nScrW);
		NLMISC::clamp(py, 0, (sint32) nScrH);
		//
		_Pointer->setPointerPos (px, py);
		_Pointer->setPointerDispPos (px, py);
		//
		getViewsUnder (px, py, _ViewsUnderPointer);
		getCtrlsUnder (px, py, _CtrlsUnderPointer);
		getGroupsUnder (px, py, _GroupsUnderPointer);
	}

	// ***************************************************************************
	void CWidgetManager::setCapturePointerLeft(CCtrlBase *c)
	{
		// additionally, abort any dragging
		if( CCtrlDraggable::getDraggedSheet() != NULL )
			CCtrlDraggable::getDraggedSheet()->abortDragging();

		_CapturePointerLeft = c;
		notifyElementCaptured(c);
	}

	// ***************************************************************************
	void CWidgetManager::setCapturePointerRight(CCtrlBase *c)
	{
		_CapturePointerRight = c;
		notifyElementCaptured(c);
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::setCaptureKeyboard(CCtrlBase *c)
	{
		CGroupEditBoxBase *oldEb= dynamic_cast<CGroupEditBoxBase*>((CCtrlBase*)_CaptureKeyboard);
		CGroupEditBoxBase *newEb= dynamic_cast<CGroupEditBoxBase*>(c);

		if (_CaptureKeyboard && _CaptureKeyboard != c)
		{
			_CaptureKeyboard->onKeyboardCaptureLost();
		}
		// If the old capturedKeyboard is an editBox and allow recoverFocusOnEnter
		if ( oldEb && oldEb->getRecoverFocusOnEnter() )
		{
			_OldCaptureKeyboard = _CaptureKeyboard;
		}
		if ( newEb )
		{
			CGroupEditBoxBase::disableSelection();

			if (!newEb->getAHOnFocus().empty())
			{
				CAHManager::getInstance()->runActionHandler(newEb->getAHOnFocus(), newEb, newEb->getAHOnFocusParams());
			}

		}
		_CaptureKeyboard = c;
		notifyElementCaptured(c);
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::resetCaptureKeyboard()
	{
		CCtrlBase *captureKeyboard = _CaptureKeyboard;
		_OldCaptureKeyboard = NULL;
		_CaptureKeyboard = NULL;
		if (captureKeyboard)
		{
			captureKeyboard->onKeyboardCaptureLost();
		}
	}

	// ***************************************************************************
	void CWidgetManager::registerClockMsgTarget(CCtrlBase *vb)
	{
		if (!vb) return;
		if (isClockMsgTarget(vb))
		{
			nlwarning("<CInterfaceManager::registerClockMsgTarget> Element %s is already registered", vb->getId().c_str());
			return;
		}
		_ClockMsgTargets.push_back(vb);
	}

	// ***************************************************************************
	void CWidgetManager::unregisterClockMsgTarget(CCtrlBase *vb)
	{
		if (!vb) return;
		std::vector<CCtrlBase*>::iterator it = std::find(_ClockMsgTargets.begin(), _ClockMsgTargets.end(), vb);
		if (it != _ClockMsgTargets.end())
		{
			_ClockMsgTargets.erase(it);
		}
	}

	// ***************************************************************************
	bool CWidgetManager::isClockMsgTarget(CCtrlBase *vb) const
	{
		std::vector<CCtrlBase*>::const_iterator it = std::find(_ClockMsgTargets.begin(), _ClockMsgTargets.end(), vb);
		return it != _ClockMsgTargets.end();
	}

	void CWidgetManager::sendClockTickEvent()
	{
		NLGUI::CEventDescriptorSystem clockTick;
		clockTick.setEventTypeExtended(NLGUI::CEventDescriptorSystem::clocktick);

		if (_CapturePointerLeft)
		{
			_CapturePointerLeft->handleEvent(clockTick);
		}
		if (_CapturePointerRight)
		{
			_CapturePointerRight->handleEvent(clockTick);
		}

		// and send clock tick msg to ctrl that are registered
		std::vector<CCtrlBase*> clockMsgTarget = _ClockMsgTargets;
		for(std::vector<CCtrlBase*>::iterator it = clockMsgTarget.begin(); it != clockMsgTarget.end(); ++it)
		{
			(*it)->handleEvent(clockTick);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::notifyElementCaptured(CCtrlBase *c)
	{
		std::set<CCtrlBase *> seen;
		CCtrlBase *curr = c;
		while (curr)
		{
			seen.insert(curr);
			curr->elementCaptured(c);
			curr = curr->getParent();
		}
		// also warn the ctrl under the pointer
		for (uint i = 0; i < (uint) _CtrlsUnderPointer.size(); ++i)
		{
			if (!seen.count(_CtrlsUnderPointer[i]))
			{
				_CtrlsUnderPointer[i]->elementCaptured(c);
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::makeWindow(CInterfaceGroup *group)
	{
		if(!group)
			return;

		uint32 i = 0;
		for (i = 0; i < _MasterGroups.size(); ++i)
		{
			if (_MasterGroups[i].Group == group->getParent())
					break;
		}

		if (i == _MasterGroups.size())
		{
			std::string stmp = std::string("not found master group for window: ")+group->getId();
			nlwarning (stmp.c_str());
			return;
		}
		else
		{
			// check if group hasn't been inserted twice.
			if (_MasterGroups[i].isWindowPresent(group))
			{
				nlwarning("Window inserted twice");
			}
			else
			{
				_MasterGroups[i].addWindow(group,group->getPriority());
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::unMakeWindow(CInterfaceGroup *group, bool noWarning)
	{
		if (!group)
			return;

		uint32 i = 0;
		for (i = 0; i < _MasterGroups.size(); ++i)
		{
			if (_MasterGroups[i].Group == group->getParent())
					break;
		}

		if (i == _MasterGroups.size())
		{
			if (!noWarning)
			{
				std::string stmp = std::string("not found master group for window: ")+group->getId();
				nlwarning (stmp.c_str());
			}
			return;
		}
		else
		{
			// check if group hasn't been inserted twice.
			if (!_MasterGroups[i].isWindowPresent(group))
			{
				if (!noWarning)
					nlwarning("Window not inserted in master group");
			}
			else
			{
				_MasterGroups[i].delWindow(group);
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::setGlobalColor (NLMISC::CRGBA col)
	{
		if (!_RProp)
		{
			_RProp = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:R");
			_GProp = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:G");
			_BProp = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:B");
			_AProp = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:A");
		}
		_RProp ->setValue32 (col.R);
		_GProp ->setValue32 (col.G);
		_BProp ->setValue32 (col.B);
		_AProp ->setValue32 (col.A);

		_GlobalColor = col;

		// set the global color for content (the same with modulated alpha)
		_GlobalColorForContent = _GlobalColor;
		_GlobalColorForContent.A = (uint8) (( (uint16) _GlobalColorForContent.A * (uint16) _ContentAlpha) >> 8);
	}

	// ***************************************************************************
	void CWidgetManager::setContentAlpha(uint8 alpha)
	{
		_ContentAlpha = alpha;
		// update alpha of global color
		_GlobalColorForContent.A = alpha;/*(uint8) (( (uint16) _GlobalColor.A * (uint16) _ContentAlpha) >> 8);*/
	}

	void CWidgetManager::resetColorProps()
	{
		_RProp = NULL;
		_GProp = NULL;
		_BProp = NULL;
		_AProp = NULL;
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceOptions* CWidgetManager::getOptions( const std::string &name )
	{
		std::map< std::string, NLMISC::CSmartPtr< CInterfaceOptions > >::iterator it = _OptionsMap.find( name );
		if( it == _OptionsMap.end() )
			return NULL;
		else
			return it->second;
	}

	void CWidgetManager::addOptions( std::string name, CInterfaceOptions *options )
	{
		_OptionsMap.insert( std::map< std::string, CInterfaceOptions* >::value_type( name, options ) );
	}

	void CWidgetManager::removeOptions( std::string name )
	{
		_OptionsMap.erase( name );
	}

	void CWidgetManager::removeAllOptions()
	{
		_OptionsMap.clear();
	}
	
	
	// ***************************************************************************
	void CWidgetManager::enableMouseHandling( bool handle )
	{
		_MouseHandlingEnabled = handle;
		if(!handle)
		{
			if(!getPointer())
				return;
			
			// If Left captured, reset
			if( getCapturePointerLeft() )
				setCapturePointerLeft( NULL );

			// Same for Right
			if( getCapturePointerRight() )
				setCapturePointerRight( NULL );
			
			// Avoid any problem with modals
			disableModalWindow();
		}
	}
	
	// ***************************************************************************
	uint CWidgetManager::getUserDblClickDelay()
	{
		uint nVal = 50;
		NLMISC::CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:DOUBLE_CLICK_SPEED");
		if( pNL != NULL )
			nVal = pNL->getValue32();
		
		uint dbclickDelay = (uint)(DOUBLE_CLICK_MIN + (DOUBLE_CLICK_MAX-DOUBLE_CLICK_MIN) * (float)nVal / 100.0f);
		return dbclickDelay;
	}
	
	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::setupOptions()
	{
		// After parsing options and templates node -> init system options.
		CInterfaceOptions *opt = getOptions( "system" );
		if( opt != NULL )
		{
			// List here all Special options
			_SystemOptions[OptionCtrlSheetGrayColor]= opt->getValue("ctrl_sheet_gray_color");
			_SystemOptions[OptionCtrlTextGrayColor]= opt->getValue("ctrl_text_gray_color");
			_SystemOptions[OptionCtrlSheetRedifyColor]= opt->getValue("ctrl_sheet_redify_color");
			_SystemOptions[OptionCtrlTextRedifyColor]= opt->getValue("ctrl_text_redify_color");
			_SystemOptions[OptionCtrlSheetGreenifyColor]= opt->getValue("ctrl_sheet_greenify_color");
			_SystemOptions[OptionCtrlTextGreenifyColor]= opt->getValue("ctrl_text_greenify_color");
			_SystemOptions[OptionViewTextOverBackColor]= opt->getValue("text_over_back_color");
			_SystemOptions[OptionFont]= opt->getValue("font");
			_SystemOptions[OptionAddCoefFont]= opt->getValue("add_coef_font");
			_SystemOptions[OptionMulCoefAnim]= opt->getValue("mul_coef_anim");
			_SystemOptions[OptionTimeoutBubbles]= opt->getValue("bubbles_timeout");
			_SystemOptions[OptionTimeoutMessages]= opt->getValue("messages_timeout");
			_SystemOptions[OptionTimeoutContext]= opt->getValue("context_timeout");
			_SystemOptions[OptionTimeoutContextHtml]= opt->getValue("context_html_timeout");
		}
		
	}
	
	// Get the alpha roll over speed
	float CWidgetManager::getAlphaRolloverSpeed()
	{
		if( _AlphaRolloverSpeedDB == NULL )
			_AlphaRolloverSpeedDB = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ALPHA_ROLLOVER_SPEED");
		float fTmp = ROLLOVER_MIN_DELTA_PER_MS + (ROLLOVER_MAX_DELTA_PER_MS - ROLLOVER_MIN_DELTA_PER_MS) * 0.01f * (100 - _AlphaRolloverSpeedDB->getValue32());
		return fTmp*fTmp*fTmp;
	}

	void CWidgetManager::resetAlphaRolloverSpeed()
	{
		_AlphaRolloverSpeedDB = NULL;
	}
	
	void CWidgetManager::setContainerAlpha(uint8 alpha)
	{
		_ContainerAlpha = alpha;
		// update alpha of global color
		NLMISC::CRGBA c = getGlobalColor();
		c.A = alpha;/*(uint8) (( (uint16) _GlobalColor.A * (uint16) _ContainerAlpha) >> 8);	*/
		setGlobalColor( c );
	}

	void CWidgetManager::updateGlobalAlphas()
	{
		_GlobalContentAlpha = (uint8)NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTENT_ALPHA")->getValue32();
		_GlobalContainerAlpha = (uint8)NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTAINER_ALPHA")->getValue32();
		_GlobalRolloverFactorContent = (uint8)NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTENT_ROLLOVER_FACTOR")->getValue32();
		_GlobalRolloverFactorContainer = (uint8)NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTAINER_ROLLOVER_FACTOR")->getValue32();
	}

	CWidgetManager::CWidgetManager()
	{
		_Pointer = NULL;
		curContextHelp = NULL;
		_ContextHelpActive = true;
		_DeltaTimeStopingContextHelp = 0;
		_MaxTimeStopingContextHelp= 0.2f;
		_LastXContextHelp= -10000;
		_LastYContextHelp= -10000;

		resetColorProps();

		_GlobalColor = NLMISC::CRGBA(255,255,255,255);
		_GlobalColorForContent = _GlobalColor;
		_ContentAlpha = 255;
		_ContainerAlpha = 255;
		_GlobalContentAlpha = 255;
		_GlobalContainerAlpha = 255;
		_GlobalRolloverFactorContent = 255;
		_GlobalRolloverFactorContainer = 255;
		_AlphaRolloverSpeedDB = NULL;

		_MouseHandlingEnabled = true;
		inGame = false;
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

}

