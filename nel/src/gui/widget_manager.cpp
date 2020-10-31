// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/db_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_pointer.h"
#include "nel/gui/ctrl_draggable.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/group_modal.h"
#include "nel/gui/group_editbox_base.h"
#include "nel/gui/interface_options.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/group_container.h"
#include "nel/gui/interface_anim.h"
#include "nel/gui/proc.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/reflect_register.h"
#include "nel/gui/editor_selection_watcher.h"
#include "nel/misc/events.h"
#include "nel/gui/root_group.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

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
		LinkHack();
	}
}

namespace NLGUI
{
	CWidgetManager* CWidgetManager::instance = NULL;
	std::string CWidgetManager::_CtrlLaunchingModalId= "ctrl_launch_modal";
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
		CInterfaceGroup *pIG = getMasterGroupFromId (sMasterGroupName);
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

			while (!sZeStart.empty())
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
	CInterfaceElement* CWidgetManager::getElementFromDefine( const std::string &defineId )
	{
		return getElementFromId( _Parser->getDefine( defineId ) );
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
		_DeltaTimeStopingContextHelp = 0;
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

	// ***************************************************************************
	void CWidgetManager::hideAllWindows()
	{
		for (uint nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
				{
					std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
					std::list<CInterfaceGroup*>::const_iterator itw;
					for (itw = rList.begin(); itw!= rList.end();)
					{
						CInterfaceGroup *pIG = *itw;
						itw++;	// since setActive invalidate the iterator, be sure we move to the next one before
						pIG->setActive(false);
					}
				}
			}
		}
	}

	// ***************************************************************************
	void CWidgetManager::hideAllNonSavableWindows()
	{
		for (uint nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
				{
					std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
					std::list<CInterfaceGroup*>::const_iterator itw;
					for (itw = rList.begin(); itw!= rList.end();)
					{
						CInterfaceGroup *pIG = *itw;
						CGroupContainer *cont = dynamic_cast<CGroupContainer *>(pIG);
						itw++;	// since setActive invalidate the iterator, be sure we move to the next one before
						if (!cont || !cont->isSavable())
						{
							pIG->setActive(false);
						}
					}
				}
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
						if (!hasModal() || getModal().ModalWindow == pIG || getModal().ModalExitClickOut)
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
						if (!hasModal() || getModal().ModalWindow == pIG ||
							getModal().ModalExitClickOut)
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
		_CapturedView = NULL;

		resetColorProps();
		resetAlphaRolloverSpeedProps();
		resetGlobalAlphasProps();

		activeAnims.clear();

		editorSelection.clear();
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

							// add deadlock counter to prevent endless loop (Issue #73: web browser long scroll lockup)
							int deadlock = 10;
							while (--deadlock > 0 && pIG->getParent()==rMG.Group && (pIG->getInvalidCoords()>0))
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

			if ( getPointer() != NULL)
				getPointer()->updateCoords();
		}



		if (bRecomputeCtrlUnderPtr)
		{
			H_AUTO ( RZ_Interface_RecomputeCtrlUnderPtr )
			if ( getPointer() != NULL )
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
					groupOver->setParentPos(vtSrc);

					sint32 backupX = groupOver->getX();

					// Copy all aspects to the view
					vtDst->setLocalized (vtSrc->isLocalized());
					vtDst->setText (vtSrc->getText());
					vtDst->setFontSize (vtSrc->getFontSize());
					vtDst->setColor (vtSrc->getColor());
					vtDst->setModulateGlobalColor(vtSrc->getModulateGlobalColor());
					vtDst->setShadow(vtSrc->getShadow());
					vtDst->setShadowOutline(vtSrc->getShadowOutline());
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

					// update one time only to get correct W/H
					groupOver->updateCoords ();

					// align and clamp to screen coords
					sint32 x = -backupX;
					if (vtSrc->isClampRight())
					{
						x += std::max(0, (groupOver->getXReal() + groupOver->getWReal()) - (groupOver->getParent()->getXReal() + groupOver->getParent()->getWReal()));
					}
					else
					{
						x +=  vtDst->getWReal() - vtSrc->getWReal();
						if ( x > (groupOver->getXReal() - groupOver->getParent()->getXReal()) )
						{
							x -= x - (groupOver->getXReal() - groupOver->getParent()->getXReal());
						}
					}
					if (x != 0) groupOver->setX(-x);

					// TODO: there should be no overflow on y, unless barely visible and next to screen border

					groupOver->updateCoords();

					// draw
					groupOver->draw ();
					// flush layers
					CViewRenderer::getInstance()->flush();

					// restore backup values
					if (x != 0) groupOver->setX(backupX);
				}
			}

			// Reset the ptr so at next frame, won't be rendered (but if reset)
			setOverExtendViewText( NULL, getOverExtendViewTextBackColor() );
		}
	}

	// ----------------------------------------------------------------------------
	void CWidgetManager::snapIfClose(CInterfaceGroup *group)
	{
		if (!group || _WindowSnapDistance == 0 || _WindowSnapInvert != lastKeyEvent.isShiftDown())
			return;

		uint hsnap = _WindowSnapDistance;
		uint vsnap = _WindowSnapDistance;

		sint32 newX = group->getX();
		sint32 newY = group->getY();

		// new coords for window without snap
		// used to calculate distance from target
		sint gLeft   = newX;
		sint gRight  = newX + group->getWReal();
		sint gTop    = newY;
		sint gBottom = newY - group->getHReal();

		// current window coords as if already snaped
		// used to calculate target for snap
		sint gLeftR   = group->getXReal();
		sint gRightR  = gLeftR + group->getWReal();
		sint gBottomR = group->getYReal();
		sint gTopR    = gBottomR + group->getHReal();

		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (!rMG.Group->getActive()) continue;

			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0 ; nPriority--)
			{
				const std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				std::list<CInterfaceGroup*>::const_reverse_iterator itw;
				for (itw = rList.rbegin(); itw != rList.rend(); itw++)
				{
					CInterfaceGroup *pIG = *itw;
					// do not snap to self, inactive, or not using mouse interaction
					if (group == pIG || !(pIG->getActive() && pIG->getUseCursor()))
						continue;

					// target
					sint wLeft   = pIG->getXReal();
					sint wRight  = pIG->getXReal() + pIG->getWReal();
					sint wTop    = pIG->getYReal() + pIG->getHReal();
					sint wBottom = pIG->getYReal();
					sint delta;

					if (gTopR >= wBottom && gBottomR <= wTop)
					{
						delta = abs(gRight - wLeft);
						if (delta <= hsnap)
						{
							hsnap = delta;
							newX = wLeft - group->getWReal();
						}

						delta = abs(gLeft - wRight);
						if (delta <= hsnap)
						{
							hsnap = delta;
							newX = wRight;
						}

						delta = abs(gLeft - wLeft);
						if (delta <= hsnap)
						{
							hsnap = delta;
							newX = wLeft;
						}

						delta = abs(gRight - wRight);
						if (delta <= hsnap)
						{
							hsnap = delta;
							newX = wRight - group->getWReal();
						}
					}

					if (gLeftR <= wRight && gRightR >= wLeft)
					{
						delta = abs(gTop - wBottom);
						if (delta <= vsnap)
						{
							vsnap = delta;
							newY = wBottom;
						}

						delta = abs(gBottom - wTop);
						if (delta <= vsnap)
						{
							vsnap = delta;
							newY = wTop + group->getHReal();
						}

						delta = abs(gTop - wTop);
						if (delta <= vsnap)
						{
							vsnap = delta;
							newY = wTop;
						}

						delta = abs(gBottom - wBottom);
						if (delta <= vsnap)
						{
							vsnap = delta;
							newY = wBottom + group->getHReal();
						}
					}
				}//windows
			}//priority
		}//master group

		group->setX(newX);
		group->setY(newY);
	}

	// ----------------------------------------------------------------------------
	uint CWidgetManager::adjustTooltipPosition( CCtrlBase *newCtrl, CInterfaceGroup *win, THotSpot ttParentRef,
												THotSpot ttPosRef, sint32 xParent, sint32 yParent,
												sint32 wParent, sint32 hParent )
	{
		CCtrlBase::TToolTipParentType	parentType= newCtrl->getToolTipParent();
		CInterfaceGroup *groupContextHelp =
			getWindowForActiveMasterGroup(newCtrl->getContextHelpWindowName());

		uint32 _ScreenH, _ScreenW;
		CViewRenderer::getInstance()->getScreenSize( _ScreenW, _ScreenH );

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
	void CWidgetManager::getNewWindowCoordToNewScreenSize( sint32 &x, sint32 &y, sint32 w, sint32 h,
														sint32 newScreenW, sint32 newScreenH) const
	{
		// NB: x is relative to Left of the window (and Left of screen)
		// NB: y is relative to Top of the window  (but Bottom of screen)

		/*
			The goal here is to move the window so it fit the new resolution
			But we don't want to change its size (because somes windows just can't)
			We also cannot use specific code according to each window because user may completly modify his interface
			So the strategy is to dectect on which "side" (or center) the window is the best sticked,
			and then just move the window according to this position
		*/

		// *** First detect from which screen position the window is the more sticked (borders or center)
		// In X: best hotspot is left, middle or right?
		sint32	posXToLeft= x;
		sint32	posXToMiddle= x+w/2-_ScreenW/2;
		sint32	posXToRight= _ScreenW-(x+w);
		sint32	bestXHotSpot= Hotspot_xL;
		sint32	bestXPosVal= posXToLeft;
		if(abs(posXToMiddle) < bestXPosVal)
		{
			bestXHotSpot= Hotspot_xM;
			bestXPosVal= abs(posXToMiddle);
		}
		if(posXToRight < bestXPosVal)
		{
			bestXHotSpot= Hotspot_xR;
			bestXPosVal= posXToRight;
		}

		// Same In Y: best hotspot is bottom, middle or top?
		// remember here that y is the top of window (relative to bottom of screen)
		sint32	posYToBottom= y-h;
		sint32	posYToMiddle= y-h/2-_ScreenH/2;
		sint32	posYToTop= _ScreenH-y;
		sint32	bestYHotSpot= Hotspot_Bx;
		sint32	bestYPosVal= posYToBottom;
		const	sint32	middleYWeight= 6;		// Avoid default Mission/Team/Map/ContactList positions to be considered as "middle"
		if(abs(posYToMiddle)*middleYWeight < bestYPosVal)
		{
			bestYHotSpot= Hotspot_Mx;
			bestYPosVal= abs(posYToMiddle)*middleYWeight;
		}
		if(posYToTop < bestYPosVal)
		{
			bestYHotSpot= Hotspot_Tx;
			bestYPosVal= posYToTop;
		}

		// *** According to best matching hotspot, and new screen resolution, move the window
		// x
		if(bestXHotSpot==Hotspot_xM)
			x= newScreenW/2 + posXToMiddle - w/2;
		else if(bestXHotSpot==Hotspot_xR)
			x= newScreenW - posXToRight - w;
		// y
		if(bestYHotSpot==Hotspot_Mx)
			y= newScreenH/2 + posYToMiddle + h/2;
		else if(bestYHotSpot==Hotspot_Tx)
			y= newScreenH - posYToTop;
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::moveAllWindowsToNewScreenSize(uint32 newScreenW, uint32 newScreenH, bool fixCurrentUI)
	{
		std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = getAllMasterGroup();
		// If resolutions correctly setuped, and really different from new setup
		if( _ScreenW >0 && _ScreenH>0 &&
			newScreenW >0 && newScreenH>0 &&
			( _ScreenW != newScreenW || _ScreenH != newScreenH)
			)
		{
			// *** Do it for the Active Desktop (if wanted)
			if(fixCurrentUI)
			{
				// only for ui:interface (not login, nor outgame)
				for (uint nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
				{
					CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
					if (!rMG.Group || rMG.Group->getId() != "ui:interface")
						continue;

					// For all priorities, but the worldspace one
					for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
					{
						if (nPriority==WIN_PRIORITY_WORLD_SPACE)
							continue;

						// For All windows (only layer 0 group container)
						std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
						std::list<CInterfaceGroup*>::const_iterator itw;
						for (itw = rList.begin(); itw != rList.end(); itw++)
						{
							CInterfaceGroup *pIG = *itw;
							if(!pIG->isGroupContainer())
								continue;
							CGroupContainer	*gc= dynamic_cast<CGroupContainer*>(pIG);
							if(gc->getLayerSetup()!=0)
								continue;
							// should all be BL / TL
							if(gc->getParentPosRef()!=Hotspot_BL || gc->getPosRef()!=Hotspot_TL)
								continue;

							// Get current window coordinates
							sint32	x= pIG->getX();				// x is relative to Left of the window
							sint32	y= pIG->getY();				// y is relative to Top of the window
							sint32	w= pIG->getW(false);		// the window may be hid, still get the correct(or estimated) W
							sint32	h= pIG->getH(false);		// the window may be hid, still get the correct(or estimated) H

							// Compute the new coordinate
							getNewWindowCoordToNewScreenSize(x, y, w, h, newScreenW, newScreenH);

							// Change
							pIG->setX(x);
							pIG->setY(y);
						}
					}
				}
			}

			std::vector< INewScreenSizeHandler* >::iterator itr;
			for( itr = newScreenSizeHandlers.begin(); itr != newScreenSizeHandlers.end(); ++itr )
			{
				INewScreenSizeHandler *handler = *itr;
				handler->process( newScreenW, newScreenH );
			}
		}

		// Now those are the last screen coordinates used for window position correction
		if(newScreenW >0 && newScreenH>0)
		{
			_ScreenW = newScreenW;
			_ScreenH = newScreenH;
		}
	}

	class InvalidateTextVisitor : public CInterfaceElementVisitor
	{
	public:
		InvalidateTextVisitor( bool reset)
		{
			this->reset = reset;
		}

		void visitGroup( CInterfaceGroup *group )
		{
			const std::vector< CViewBase* > &vs = group->getViews();
			for( std::vector< CViewBase* >::const_iterator itr = vs.begin(); itr != vs.end(); ++itr )
			{
				CViewText *vt = dynamic_cast< CViewText* >( *itr );
				if( vt != NULL )
				{
					if( reset )
						vt->resetTextIndex();
					vt->updateTextContext();
				}
			}
		}

	private:
		bool reset;
	};

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::updateAllLocalisedElements()
	{

		uint32 nMasterGroup;

		uint32 w, h;
		CViewRenderer::getInstance()->checkNewScreenSize ();
		CViewRenderer::getInstance()->getScreenSize (w, h);

		// Update ui:* (limit the master containers to the height of the screen)
		for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			rMG.Group->setW (w);
			rMG.Group->setH (h);
		}
		CViewRenderer::getInstance()->setClipWindow(0, 0, w, h);

		bool scaleChanged = _InterfaceScale != CViewRenderer::getInstance()->getInterfaceScale();
		if (scaleChanged)
		{
			_InterfaceScale = CViewRenderer::getInstance()->getInterfaceScale();
			notifyInterfaceScaleWatchers();
		}

		// If all conditions are OK, move windows so they fit correctly with new screen size
		// Do this work only InGame when Config is loaded
		moveAllWindowsToNewScreenSize(w,h,true);

		// Invalidate coordinates of all Windows of each MasterGroup
		for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];

			InvalidateTextVisitor inv( false);

			rMG.Group->visitGroupAndChildren( &inv );
			rMG.Group->invalidateCoords ();
			for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
			{
				std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
				std::list<CInterfaceGroup*>::const_iterator itw;
				for (itw = rList.begin(); itw != rList.end(); itw++)
				{
					CInterfaceGroup *pIG = *itw;
					pIG->visitGroupAndChildren( &inv );
					pIG->invalidateCoords ();
				}
			}
		}

		// setup for all
		for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			bool bActive = rMG.Group->getActive ();
			rMG.Group->setActive (true);
			rMG.Group->updateCoords ();
			rMG.Group->setActive (bActive);
		}

		// update coords one
		checkCoords();

		// Action by default (container opening
		for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			rMG.Group->launch ();
		}

	}

	void CWidgetManager::drawViews( NL3D::UCamera camera )
	{
		CViewRenderer::getInstance()->activateWorldSpaceMatrix (false);
		NL3D::UDriver *driver = CViewRenderer::getInstance()->getDriver();

		// If an element has captured the keyboard, make sure it is alway visible (all parent windows active)
		if( getCaptureKeyboard() != NULL)
		{
			CCtrlBase *cb = getCaptureKeyboard();
			do
			{
				if (!cb->getActive())
				{
					setCaptureKeyboard(NULL);
					break;
				}
				cb = cb->getParent();
			}
			while (cb);
		}
		
		// Check if screen size changed
		uint32 w, h;
		CViewRenderer::getInstance()->checkNewScreenSize ();
		CViewRenderer::getInstance()->getScreenSize (w, h);
		if ((w != _ScreenW) || (h != _ScreenH))
		{
			// No Op if screen minimized
			if(w!=0 && h!=0 && !CViewRenderer::getInstance()->isMinimized())
			{
				updateAllLocalisedElements ();
				setScreenWH(w, h);
			}
		}
		
		// Update global color from database
		if (!_RProp)
		{
			_RProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:R");
			_GProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:G");
			_BProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:B");
			_AProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:A");
		}

		setGlobalColor(NLMISC::CRGBA(
			(uint8)_RProp->getValue32(),
			(uint8)_GProp->getValue32(),
			(uint8)_BProp->getValue32(),
			(uint8)_AProp->getValue32()));

		NLMISC::CRGBA c  = getGlobalColorForContent();
		NLMISC::CRGBA gc = getGlobalColor();
		c.R = gc.R;
		c.G = gc.G;
		c.B = gc.B;
		c.A = (uint8) (( (uint16) c.A * (uint16) getContentAlpha() ) >> 8);
		setGlobalColorForContent( c );

		// Update global alphaS from database
		updateGlobalAlphas();

		/*  Draw all the windows
			To minimize texture swapping, we first sort per Window, then we sort per layer, then we render per Global Texture.
			Computed String are rendered in on big drawQuads at last part of each layer
		*/
		CDBManager::getInstance()->flushObserverCalls();

		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				// Sort world space windows
				rMG.sortWorldSpaceGroup ();

				for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
				{
					if ( (nPriority == WIN_PRIORITY_WORLD_SPACE) && !camera.empty())
					{
						driver->setViewMatrix( NL3D::CMatrix::Identity);
						driver->setModelMatrix( NL3D::CMatrix::Identity);
						driver->setFrustum(camera.getFrustum());
						CViewRenderer::getInstance()->activateWorldSpaceMatrix (true);
					}

					std::list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
					std::list<CInterfaceGroup*>::const_iterator itw;

					for (itw = rList.begin(); itw != rList.end(); itw++)
					{
						CInterfaceGroup *pIG = *itw;
						if( pIG ) // TODO: debug null pointer in PrioritizedWindows list
						{
							if (pIG->getActive())
							{
								// Draw all the elements of this window in the layers in ViewRendered
								pIG->draw ();
								// flush the layers
								CViewRenderer::getInstance()->flush ();
							}
						}
					}

					if( draggedElement != NULL )
					{
						CInterfaceElement *e = draggedElement;
						static_cast< CViewBase* >( e )->draw();
					}

					if ( (nPriority == WIN_PRIORITY_WORLD_SPACE) && !camera.empty())
					{
						driver->setMatrixMode2D11();
						CViewRenderer::getInstance()->activateWorldSpaceMatrix (false);
					}
				}
			}
		}

		CDBManager::getInstance()->flushObserverCalls();

		// draw the special over extend text
		drawOverExtendViewText();

		// draw the context help
		drawContextHelp ();

		std::vector< IOnWidgetsDrawnHandler* >::iterator itr;
		for( itr = onWidgetsDrawnHandlers.begin(); itr != onWidgetsDrawnHandlers.end(); ++itr )
		{
			IOnWidgetsDrawnHandler *handler = *itr;
			handler->process();
		}

		// Draw the pointer and DND Item
		if (getPointer() != NULL)
		{
			if (getPointer()->getActive())
				getPointer()->draw ();
		}

		if (CInterfaceElement::getEditorMode())
		{
			for(uint i = 0; i < editorSelection.size(); ++i)
			{
				CInterfaceElement *e = getElementFromId(editorSelection[i]);
				if (e != NULL)
					e->drawHighlight();
			}
		}

		// flush layers
		CViewRenderer::getInstance()->flush();

		// todo hulud remove Return in 2d world
		driver->setMatrixMode2D11();

		CDBManager::getInstance()->flushObserverCalls();
	}

	bool CWidgetManager::handleEvent( const CEventDescriptor &evnt )
	{
		// Check if we can receive events (no anims!)
		for( uint32 i = 0; i < activeAnims.size(); ++i )
			if( activeAnims[i]->isDisableButtons() )
				return false;

		bool handled = false;

		if( evnt.getType() == CEventDescriptor::system )
		{
			handleSystemEvent( evnt );
		}
		else
		if (evnt.getType() == CEventDescriptor::key)
		{
			handled = handleKeyboardEvent( evnt );
		}
		else if (evnt.getType() == CEventDescriptor::mouse )
		{
			handled = handleMouseEvent( evnt );
		}

		CDBManager::getInstance()->flushObserverCalls();

		return handled;
	}

	bool CWidgetManager::handleSystemEvent( const CEventDescriptor &evnt )
	{
		const CEventDescriptorSystem &systemEvent = reinterpret_cast< const CEventDescriptorSystem& >( evnt );
		if( systemEvent.getEventTypeExtended() == CEventDescriptorSystem::setfocus )
		{
			if( getCapturePointerLeft() != NULL )
			{
				getCapturePointerLeft()->handleEvent( evnt );
				setCapturePointerLeft( NULL );
			}

			if( getCapturePointerRight() != NULL )
			{
				getCapturePointerRight()->handleEvent( evnt );
				setCapturePointerRight( NULL );
			}

			if( _CapturedView != NULL )
			{
				_CapturedView->handleEvent( evnt );
				_CapturedView = NULL;
			}
		}

		return true;
	}

	bool CWidgetManager::handleKeyboardEvent( const CEventDescriptor &evnt )
	{
		bool handled = false;

		CEventDescriptorKey &eventDesc = (CEventDescriptorKey&)evnt;

		//_LastEventKeyDesc = eventDesc;

		// Any Key event disable the ContextHelp
		disableContextHelp();

		// Hide menu if the key is pushed
//		if ((eventDesc.getKeyEventType() == CEventDescriptorKey::keydown) && !_ModalStack.empty() && !eventDesc.getKeyAlt() && !eventDesc.getKeyCtrl() && !eventDesc.getKeyShift())
		// Hide menu (or popup menu) is ESCAPE pressed
		if( eventDesc.getKeyEventType() == CEventDescriptorKey::keydown && eventDesc.getKey() == NLMISC::KeyESCAPE )
		{
			if( hasModal() )
			{
				SModalWndInfo mwi = getModal();
				if (mwi.ModalExitKeyPushed)
					disableModalWindow();
			}
		}

		// Manage "quit window" If the Key is ESCAPE, no captureKeyboard
		if( eventDesc.getKeyEventType() == CEventDescriptorKey::keydown && eventDesc.getKey() == NLMISC::KeyESCAPE )
		{
			// Get the last escapable active top window. NB: this is ergonomically better.
			CInterfaceGroup	*win= getLastEscapableTopWindow();
			if( win )
			{
				// If the window is a modal, must pop it.
				if( dynamic_cast<CGroupModal*>(win) )
				{
					if(!win->getAHOnEscape().empty())
						CAHManager::getInstance()->runActionHandler(win->getAHOnEscape(), win, win->getAHOnEscapeParams());
					popModalWindow();
					handled= true;
				}
				// else just disable it.
				// Special case: leave the escape Key to the CaptureKeyboard .
				else if( !getCaptureKeyboard() )
				{
					if(!win->getAHOnEscape().empty())
						CAHManager::getInstance()->runActionHandler(win->getAHOnEscape(), win, win->getAHOnEscapeParams());
					win->setActive(false);
					handled= true;
				}
			}
		}

		// Manage complex "Enter"
		if( eventDesc.getKeyEventType() == CEventDescriptorKey::keychar && eventDesc.getChar() == NLMISC::KeyRETURN && !eventDesc.getKeyCtrl() )
		{
			// If the  top window has Enter AH
			CInterfaceGroup	*tw= getTopWindow();
			if(tw && !tw->getAHOnEnter().empty())
			{
				// if the captured keyboard is in this Modal window, then must handle him in priority
				if( getCaptureKeyboard() && getCaptureKeyboard()->getRootWindow()==tw)
				{
					bool result = getCaptureKeyboard()->handleEvent(evnt);
					CDBManager::getInstance()->flushObserverCalls();
					return result;
				}
				else
				{
					// The window or modal control the OnEnter. Execute, and don't go to the chat.
					CAHManager::getInstance()->runActionHandler(tw->getAHOnEnter(), tw, tw->getAHOnEnterParams());
					handled= true;
				}
			}

			// else the 'return' key bring back to the last edit box (if possible)
			CCtrlBase *oldCapture = getOldCaptureKeyboard() ? getOldCaptureKeyboard() : getDefaultCaptureKeyboard();
			if ( getCaptureKeyboard() == NULL && oldCapture && !handled)
			{
				/* If the editbox does not want to recover focus, then abort. This possibility is normaly avoided
					through setCaptureKeyboard() which already test getRecoverFocusOnEnter(), but it is still possible
					for the default capture (main chat) or the old captured window to not want to recover
					(temporary Read Only chat for instance)
				*/
				if(!dynamic_cast<CGroupEditBoxBase*>(oldCapture) ||
					dynamic_cast<CGroupEditBoxBase*>(oldCapture)->getRecoverFocusOnEnter())
				{
					setCaptureKeyboard( oldCapture );
					notifyElementCaptured(getCaptureKeyboard() );
					// make sure all parent windows are active
					CCtrlBase *cb = getCaptureKeyboard();
					CGroupContainer *lastContainer = NULL;
					nlassert(cb);
					for(;;)
					{
						CGroupContainer *gc = dynamic_cast<CGroupContainer *>(cb);
						if (gc) lastContainer = gc;
						cb->forceOpen();
						if (cb->getParent())
						{
							cb = cb->getParent();
						}
						else
						{
							cb->invalidateCoords();
							break;
						}
					}
					if (lastContainer)
					{
						setTopWindow(lastContainer);
						lastContainer->enableBlink(1);
					}
					handled= true;
				}
			}
		}

		// General case: handle it in the Captured keyboard
		if ( getCaptureKeyboard() != NULL && !handled)
		{
			bool result = getCaptureKeyboard()->handleEvent(evnt);
			CDBManager::getInstance()->flushObserverCalls();
			return result;
		}

		lastKeyEvent = eventDesc;

		return handled;
	}

	bool CWidgetManager::handleMouseEvent( const CEventDescriptor &evnt )
	{
		bool handled = false;

		CEventDescriptorMouse &eventDesc = (CEventDescriptorMouse&)evnt;

		if( eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown )
			_Pointer->setButtonState( static_cast< NLMISC::TMouseButton >( _Pointer->getButtonState() | NLMISC::leftButton ) );
		else
		if( eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown )
			_Pointer->setButtonState( static_cast< NLMISC::TMouseButton >( _Pointer->getButtonState() | NLMISC::rightButton ) );
		else
		if( eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftup )
			_Pointer->setButtonState( static_cast< NLMISC::TMouseButton >( _Pointer->getButtonState() & ~NLMISC::leftButton ) );
		if( eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightup )
			_Pointer->setButtonState( static_cast< NLMISC::TMouseButton >( _Pointer->getButtonState() & ~NLMISC::rightButton ) );

		if( eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mousemove )
			handleMouseMoveEvent( eventDesc );

		eventDesc.setX( _Pointer->getX() );
		eventDesc.setY( _Pointer->getY() );

		if( CInterfaceElement::getEditorMode() )
		{
			// Let's pretend we've handled the event... or actually we have!
			if( ( eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown ) ||
				( eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightup ) )
				return true;
		}

		if( isMouseHandlingEnabled() )
		{
			// First thing to do : Capture handling
			if ( getCapturePointerLeft() != NULL)
				handled|= getCapturePointerLeft()->handleEvent(evnt);

			if ( getCapturePointerRight() != NULL &&
				getCapturePointerLeft() != getCapturePointerRight() )
				handled|= getCapturePointerRight()->handleEvent(evnt);

			if( _CapturedView != NULL &&
				_CapturedView != getCapturePointerLeft() &&
				_CapturedView != getCapturePointerRight() )
				_CapturedView->handleEvent( evnt );

			CInterfaceGroup *ptr = getWindowUnder (eventDesc.getX(), eventDesc.getY());
			setCurrentWindowUnder( ptr );

			// Any Mouse event but move disable the ContextHelp
			if(eventDesc.getEventTypeExtended() != CEventDescriptorMouse::mousemove)
			{
				disableContextHelp();
			}

			// get the group under the mouse
			CInterfaceGroup *pNewCurrentWnd = getCurrentWindowUnder();
			setMouseOverWindow( pNewCurrentWnd != NULL );


			NLMISC::CRefPtr<CGroupModal>	clickedOutModalWindow;

			// modal special features
			if ( hasModal() )
			{
				CWidgetManager::SModalWndInfo mwi = getModal();
				if(mwi.ModalWindow)
				{
					// If we are not in "click out" mode so we dont handle controls other than those of the modal
					if (pNewCurrentWnd != mwi.ModalWindow && !mwi.ModalExitClickOut)
					{
						pNewCurrentWnd = NULL;
					}
					else
					{
						// If there is a handler on click out launch it
						if (pNewCurrentWnd != mwi.ModalWindow)
							if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown ||
								(eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown))
								if (!mwi.ModalHandlerClickOut.empty())
									CAHManager::getInstance()->runActionHandler(mwi.ModalHandlerClickOut,NULL,mwi.ModalClickOutParams);

						// If the current window is not the modal and if must quit on click out
						if(pNewCurrentWnd != mwi.ModalWindow && mwi.ModalExitClickOut)
						{
							// NB: don't force handle==true because to quit a modal does not avoid other actions

							// quit if click outside
							if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown ||
								(eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown))
							{
								clickedOutModalWindow = dynamic_cast<CGroupModal *>((CInterfaceGroup*)mwi.ModalWindow);
								// disable the modal
								popModalWindow();
								if ( hasModal() )
								{
									// don't handle event unless it is a previous modal window
									if( !isPreviousModal( pNewCurrentWnd ) )
										pNewCurrentWnd = NULL; // can't handle event before we have left all modal windows
								}
								movePointer (0,0); // Reget controls under pointer
							}
						}
					}
				}
			}

			// Manage LeftClick.
			if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown)
			{
				if ((pNewCurrentWnd != NULL) && (!hasModal()) && (pNewCurrentWnd->getOverlappable()))
				{
					CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(pNewCurrentWnd);
					if (pGC != NULL)
					{
						if (!pGC->isGrayed()) setTopWindow(pNewCurrentWnd);
					}
					else
					{
						setTopWindow(pNewCurrentWnd);
					}
				}

				bool captured = false;

				// must not capture a new element if a sheet is currentlty being dragged.
				// This may happen when alt-tab has been used => the sheet is dragged but the left button is up
				if (!CCtrlDraggable::getDraggedSheet())
				{
					if( CInterfaceElement::getEditorMode() && _GroupSelection )
					{
						for( sint32 i = _GroupsUnderPointer.size() - 1; i >= 0; i-- )
						{
							CInterfaceGroup *g = _GroupsUnderPointer[ i ];
							if( ( g != NULL ) && ( g->isInGroup( pNewCurrentWnd ) ) )
							{
								_CapturedView = g;
								captured = true;
								break;
							}
						}
					}

					if( !captured )
					{
						// Take the top most control.
						uint nMaxDepth = 0;
						const std::vector< CCtrlBase* >& _CtrlsUnderPointer = getCtrlsUnderPointer();
						for (sint32 i = (sint32)_CtrlsUnderPointer.size()-1; i >= 0; i--)
						{
							CCtrlBase	*ctrl= _CtrlsUnderPointer[i];
							if (ctrl && ctrl->isCapturable() && ctrl->isInGroup( pNewCurrentWnd ) )
							{
								if( CInterfaceElement::getEditorMode() && !ctrl->isEditorSelectable() )
									continue;

								uint d = ctrl->getDepth( pNewCurrentWnd );
								if (d > nMaxDepth)
								{
									nMaxDepth = d;
									setCapturePointerLeft( ctrl );
									captured = true;
								}
							}
						}
					}

					if( CInterfaceElement::getEditorMode() && !captured )
					{
						for( sint32 i = _ViewsUnderPointer.size()-1; i >= 0; i-- )
						{
							CViewBase *v = _ViewsUnderPointer[i];
							if( ( v != NULL ) && v->isInGroup( pNewCurrentWnd ) )
							{
								if( CInterfaceElement::getEditorMode() && !v->isEditorSelectable() )
									continue;

								_CapturedView = v;
								captured = true;
								break;
							}
						}
					}

					notifyElementCaptured( getCapturePointerLeft() );
					if (clickedOutModalWindow && !clickedOutModalWindow->OnPostClickOut.empty())
					{
						CAHManager::getInstance()->runActionHandler(clickedOutModalWindow->OnPostClickOut, getCapturePointerLeft(), clickedOutModalWindow->OnPostClickOutParams);
					}
				}
				//if found
				if ( captured )
				{
					// consider clicking on a control implies handling of the event.
					handled= true;

					if( getCapturePointerLeft() != NULL )
						_CapturedView = getCapturePointerLeft();

					// handle the capture
					_CapturedView->handleEvent( evnt );
				}
				else
				{
					if( CInterfaceElement::getEditorMode() )
						clearEditorSelection();
				}
			}

			// Manage RightClick
			if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown)
			{
				if ((pNewCurrentWnd != NULL) && (!hasModal()) && (pNewCurrentWnd->getOverlappable()))
				{
					CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(pNewCurrentWnd);
					if (pGC != NULL)
					{
						if (!pGC->isGrayed()) setTopWindow(pNewCurrentWnd);
					}
					else
					{
						setTopWindow(pNewCurrentWnd);
					}
				}

				// Take the top most control.
				{
					uint nMaxDepth = 0;
					const std::vector< CCtrlBase* >& _CtrlsUnderPointer = getCtrlsUnderPointer();
					for (sint32 i = (sint32)_CtrlsUnderPointer.size()-1; i >= 0; i--)
					{
						CCtrlBase	*ctrl= _CtrlsUnderPointer[i];
						if (ctrl && ctrl->isCapturable() && ctrl->isInGroup( pNewCurrentWnd ) )
						{
							uint d = ctrl->getDepth( pNewCurrentWnd );
							if (d > nMaxDepth)
							{
								nMaxDepth = d;
								setCapturePointerRight( ctrl );
							}
						}
					}
					notifyElementCaptured( getCapturePointerRight() );
					if (clickedOutModalWindow && !clickedOutModalWindow->OnPostClickOut.empty())
					{
						CAHManager::getInstance()->runActionHandler(clickedOutModalWindow->OnPostClickOut, getCapturePointerRight(), clickedOutModalWindow->OnPostClickOutParams);
					}
				}
				//if found
				if ( getCapturePointerRight() != NULL)
				{
					// handle the capture
					handled |= getCapturePointerRight()->handleEvent(evnt);
				}
			}


			if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightup)
			{
				if (!handled)
					if (pNewCurrentWnd != NULL)
						pNewCurrentWnd->handleEvent(evnt);
				if ( getCapturePointerRight() != NULL)
				{
					setCapturePointerRight(NULL);
					handled= true;
				}
			}

			// window handling. if not handled by a control
			if (!handled)
			{
				if (((pNewCurrentWnd != NULL) && !hasModal()) ||
					((hasModal() && getModal().ModalWindow == pNewCurrentWnd)))
				{
					CEventDescriptorMouse ev2 = eventDesc;
					sint32 x= eventDesc.getX(), y = eventDesc.getY();
					if (pNewCurrentWnd)
					{
						pNewCurrentWnd->absoluteToRelative (x, y);
						ev2.setX (x); ev2.setY (y);
						handled|= pNewCurrentWnd->handleEvent (ev2);
					}

					// After handle event of a left click, may set window Top if movable (infos etc...)
					//if( (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown) && pNewCurrentWnd->isMovable() )
					//	setTopWindow(pNewCurrentWnd);
				}
			}

			// Put here to let a chance to the window to handle if the capture dont
			if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftup)
			{
				if ( getCapturePointerLeft() != NULL)
				{
					if( !handled )
					{
						CCtrlBase *c = getCapturePointerLeft();
						c->handleEvent( evnt );
					}

					setCapturePointerLeft(NULL);
					handled = true;
				}

				_CapturedView = NULL;

				if( CInterfaceElement::getEditorMode() )
					stopDragging();
			}


			// If the current window is the modal, may Modal quit. Do it after standard event handle
			if(hasModal() && pNewCurrentWnd == getModal().ModalWindow)
			{
				// NB: don't force handle==true because to quit a modal does not avoid other actions
				CWidgetManager::SModalWndInfo mwi = getModal();
				//  and if must quit on click right
				if(mwi.ModalExitClickR)
				{
					// quit if click right
					if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightup)
						// disable the modal
						disableModalWindow();
				}

				//  and if must quit on click left
				if(mwi.ModalExitClickL)
				{
					// quit if click right
					if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftup)
						// disable the modal
						disableModalWindow();
				}
			}

			// If the mouse is over a window, always consider the event is taken (avoid click behind)
			handled|= isMouseOverWindow();

			// If mouse click was not on interface and we have keyboard captured, then release keyboard
			if (!handled && getCaptureKeyboard() != NULL && eventDesc.getEventTypeExtended() != CEventDescriptorMouse::mousemove)
				CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
		}

		return handled;
	}

	bool CWidgetManager::handleMouseMoveEvent( const CEventDescriptor &eventDesc )
	{
		if( getPointer() == NULL )
			return false;

		if( eventDesc.getType() != CEventDescriptor::mouse )
			return false;

		const CEventDescriptorMouse &e = static_cast< const CEventDescriptorMouse& >( eventDesc );

		if( e.getEventTypeExtended() != CEventDescriptorMouse::mousemove )
			return false;

		uint32 screenW, screenH;
		CViewRenderer::getInstance()->getScreenSize( screenW, screenH );
		sint32 oldX = getPointer()->getX();
		sint32 oldY = getPointer()->getY();

		sint32 x = e.getX();
		sint32 y = e.getY();

		// These are floats packed in the sint32 from the NEL events that provide them as float
		// see comment in CInputHandler::handleMouseMoveEvent
		sint32 newX = static_cast< sint32 >( std::floor( *reinterpret_cast< float* >( &x ) * screenW + 0.5f ) );
		sint32 newY = static_cast< sint32 >( std::floor( *reinterpret_cast< float* >( &y ) * screenH + 0.5f ) );

		if( ( oldX != newX ) || ( oldY != newY ) )
		{
			movePointerAbs( newX, newY );
			CEventDescriptorMouse &ve = const_cast< CEventDescriptorMouse& >( e );
			ve.setX( getPointer()->getX() );
			ve.setY( getPointer()->getY() );
		}

		if( CInterfaceElement::getEditorMode() )
		{
			if( ( _CapturedView != NULL ) && ( draggedElement == NULL ) )
			{
				startDragging();
			}
			else
			if( draggedElement != NULL )
			{
				sint32 dx = newX - oldX;
				sint32 dy = newY - oldY;

				draggedElement->moveBy( dx, dy );
			}
		}

		return true;
	}

	// ------------------------------------------------------------------------------------------------
	bool CWidgetManager::startDragging()
	{
		CInterfaceElement *e = NULL;

		CInterfaceGroup *g = _CapturedView->getParent();
		if( g != NULL )
		{
			e = g->takeElement( _CapturedView );
			if( e == NULL )
			{
				nlinfo( "Something went horribly wrong :(" );
				return false;
			}
		}
		else
			e = _CapturedView;

		e->setParent( NULL );
		draggedElement = e;

		return true;
	}

	void CWidgetManager::stopDragging()
	{
		if( draggedElement != NULL )
		{
			CInterfaceGroup *g = getGroupUnder( draggedElement->getXReal(), draggedElement->getYReal() );
			CInterfaceElement *e = draggedElement;
			CInterfaceGroup *tw = getTopWindow();

			if( g == NULL )
				g = tw;

			std::string oldid = e->getId();

			e->setParent( g );
			e->setIdRecurse( e->getShortId() );
			e->setParentPos( g );
			e->setParentSize( g );
			g->addElement( e );

			e->alignTo( g );
			//e->setName( "==MARKED==" );

			draggedElement = NULL;

			onWidgetMoved( oldid, e->getId() );
		}
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
		if(!getPointer())
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
		_CapturedView = NULL;

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
		std::list<CCtrlBase*>::iterator it = std::find(_ClockMsgTargets.begin(), _ClockMsgTargets.end(), vb);
		if (it != _ClockMsgTargets.end())
		{
			// instead of deleting, just mark as deleted incase we are inside iterating loop,
			// it will be removed in sendClockTickEvent
			(*it) = NULL;
		}
	}

	// ***************************************************************************
	bool CWidgetManager::isClockMsgTarget(CCtrlBase *vb) const
	{
		std::list<CCtrlBase*>::const_iterator it = std::find(_ClockMsgTargets.begin(), _ClockMsgTargets.end(), vb);
		return it != _ClockMsgTargets.end();
	}

	void CWidgetManager::sendClockTickEvent()
	{
		CEventDescriptorSystem clockTick;
		clockTick.setEventTypeExtended(CEventDescriptorSystem::clocktick);

		if (_CapturePointerLeft)
		{
			_CapturePointerLeft->handleEvent(clockTick);
		}
		if (_CapturePointerRight)
		{
			_CapturePointerRight->handleEvent(clockTick);
		}

		// and send clock tick msg to ctrl that are registered
		for(std::list<CCtrlBase*>::iterator it = _ClockMsgTargets.begin(); it != _ClockMsgTargets.end();)
		{
			CCtrlBase* ctrl = *it;
			if (ctrl)
			{
				ctrl->handleEvent(clockTick);
				++it;
			}
			else
				it = _ClockMsgTargets.erase(it);
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
			_RProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:R");
			_GProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:G");
			_BProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:B");
			_AProp = CDBManager::getInstance()->getDbProp("UI:SAVE:COLOR:A");
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


	bool CWidgetManager::serializeOptions( xmlNodePtr parentNode ) const
	{
		if( parentNode == NULL )
			return false;

		std::map< std::string, NLMISC::CSmartPtr< CInterfaceOptions > >::const_iterator itr;
		for( itr = _OptionsMap.begin(); itr != _OptionsMap.end(); ++itr )
		{
			if( itr->second->serialize( parentNode, itr->first ) == NULL )
				return false;
		}

		return true;
	}


	bool CWidgetManager::serializeTreeData( xmlNodePtr parentNode ) const
	{
		if( parentNode == NULL )
			return false;

		std::vector< SMasterGroup >::size_type i;
		for( i = 0; i < _MasterGroups.size(); i++ )
		{
			const SMasterGroup &mg = _MasterGroups[ i ];

			std::vector< CInterfaceGroup* >::size_type j;
			for( j = 0; j < mg.Group->getNumGroup(); j++ )
			{
				CInterfaceGroup *g = mg.Group->getGroup( j );
				nlassert(g);

				if( dynamic_cast< CGroupModal* >( g ) != NULL )
					continue;

				if( g->serializeTreeData( parentNode ) == NULL )
					return false;
			}
		}

		return true;
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
		NLMISC::CCDBNodeLeaf *pNL = CDBManager::getInstance()->getDbProp("UI:SAVE:DOUBLE_CLICK_SPEED");
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
			_SystemOptions[OptionMonospaceFont]= opt->getValue("monospace_font");
		}

	}

	// Get the alpha roll over speed
	float CWidgetManager::getAlphaRolloverSpeed()
	{
		if( _AlphaRolloverSpeedDB == NULL )
			_AlphaRolloverSpeedDB = CDBManager::getInstance()->getDbProp("UI:SAVE:ALPHA_ROLLOVER_SPEED");
		float fTmp = ROLLOVER_MIN_DELTA_PER_MS + (ROLLOVER_MAX_DELTA_PER_MS - ROLLOVER_MIN_DELTA_PER_MS) * 0.01f * (100 - _AlphaRolloverSpeedDB->getValue32());
		return fTmp*fTmp*fTmp;
	}

	void CWidgetManager::resetAlphaRolloverSpeedProps()
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
		if (!_GlobalContentAlphaDB)
		{
			_GlobalContentAlphaDB = CDBManager::getInstance()->getDbProp("UI:SAVE:CONTENT_ALPHA");
			nlassert(_GlobalContentAlphaDB);
			_GlobalContainerAlphaDB = CDBManager::getInstance()->getDbProp("UI:SAVE:CONTAINER_ALPHA");
			nlassert(_GlobalContainerAlphaDB);
			_GlobalContentRolloverFactorDB = CDBManager::getInstance()->getDbProp("UI:SAVE:CONTENT_ROLLOVER_FACTOR");
			nlassert(_GlobalContentRolloverFactorDB);
			_GlobalContainerRolloverFactorDB = CDBManager::getInstance()->getDbProp("UI:SAVE:CONTAINER_ROLLOVER_FACTOR");
			nlassert(_GlobalContainerRolloverFactorDB);
		}
		_GlobalContentAlpha = (uint8)_GlobalContentAlphaDB->getValue32();
		_GlobalContainerAlpha = (uint8)_GlobalContainerAlphaDB->getValue32();
		_GlobalRolloverFactorContent = (uint8)_GlobalContentRolloverFactorDB->getValue32();
		_GlobalRolloverFactorContainer = (uint8)_GlobalContainerRolloverFactorDB->getValue32();
	}

	void CWidgetManager::resetGlobalAlphasProps()
	{
		_GlobalContentAlphaDB = NULL;
		_GlobalContainerAlphaDB = NULL;
		_GlobalContentRolloverFactorDB = NULL;
		_GlobalContainerRolloverFactorDB = NULL;
	}

	void CWidgetManager::registerNewScreenSizeHandler( INewScreenSizeHandler *handler )
	{
		std::vector< INewScreenSizeHandler* >::iterator itr =
			std::find( newScreenSizeHandlers.begin(), newScreenSizeHandlers.end(), handler );

		if( itr != newScreenSizeHandlers.end() )
			return;

		newScreenSizeHandlers.push_back( handler );
	}

	void CWidgetManager::removeNewScreenSizeHandler( INewScreenSizeHandler *handler )
	{
		std::vector< INewScreenSizeHandler* >::iterator itr =
			std::find( newScreenSizeHandlers.begin(), newScreenSizeHandlers.end(), handler );

		if( itr == newScreenSizeHandlers.end() )
			return;

		newScreenSizeHandlers.erase( itr );
	}

	void CWidgetManager::registerOnWidgetsDrawnHandler( IOnWidgetsDrawnHandler* handler )
	{
		std::vector< IOnWidgetsDrawnHandler* >::iterator itr =
			std::find( onWidgetsDrawnHandlers.begin(), onWidgetsDrawnHandlers.end(), handler );

		if( itr != onWidgetsDrawnHandlers.end() )
			return;

		onWidgetsDrawnHandlers.push_back( handler );
	}

	void CWidgetManager::removeOnWidgetsDrawnHandler( IOnWidgetsDrawnHandler* handler )
	{
		std::vector< IOnWidgetsDrawnHandler* >::iterator itr =
			std::find( onWidgetsDrawnHandlers.begin(), onWidgetsDrawnHandlers.end(), handler );

		if( itr == onWidgetsDrawnHandlers.end() )
			return;

		onWidgetsDrawnHandlers.erase( itr );
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::startAnim( const std::string &animId )
	{
		CInterfaceAnim *pIT = _Parser->getAnim( animId );
		if( pIT == NULL )
			return;

		stopAnim( animId );
		pIT->start();
		activeAnims.push_back( pIT );
	}

	void CWidgetManager::removeFinishedAnims()
	{
		sint32 i = 0;
		for( i = 0; i < (sint32)activeAnims.size(); ++i )
		{
			CInterfaceAnim *pIA = activeAnims[i];
			if (pIA->isFinished())
			{
				activeAnims.erase( activeAnims.begin() + i );
				--i;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::stopAnim( const std::string &animId )
	{
		CInterfaceAnim *pIT = _Parser->getAnim( animId );

		for( uint i = 0; i < activeAnims.size(); ++i )
			if( activeAnims[ i ] == pIT )
			{
				activeAnims.erase( activeAnims.begin() + i );
				if( !pIT->isFinished() )
					pIT->stop();
				return;
			}
	}

	void CWidgetManager::updateAnims()
	{
		for( std::vector< CInterfaceAnim* >::size_type i = 0; i < activeAnims.size(); i++ )
			activeAnims[ i ]->update();
	}


	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::runProcedure( const std::string &procName, CCtrlBase *pCaller,
		const std::vector< std::string> &paramList )
	{
		CProcedure *procp = _Parser->getProc( procName );
		if( procp == NULL )
			return;

		CProcedure &proc = *procp;

		// Run all actions
		for( uint i = 0; i < proc.Actions.size(); i++ )
		{
			const CProcAction &action = proc.Actions[i];
			// test if the condition for the action is valid
			if (!action.CondBlocks.empty())
			{
				CInterfaceExprValue result;
				result.setBool( false );
				std::string cond;
				action.buildCond( paramList, cond );
				CInterfaceExpr::eval( cond, result, NULL );

				if( result.toBool() )
					if( !result.getBool() )
						continue;
			}
			// build the params sting
			std::string params;
			action.buildParams( paramList, params );
			// run
			//nlwarning("step %d : %s, %s", (int) i, action.Action.c_str(), params.c_str());
			CAHManager::getInstance()->runActionHandler( action.Action, pCaller, params );
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::setProcedureAction( const std::string &procName, uint actionIndex,
		const std::string &ah, const std::string &params )
	{
		CProcedure *procp = _Parser->getProc( procName );
		if( procp == NULL )
			return;

		CProcedure &proc = *procp;

		// set wanted action
		if( actionIndex<proc.Actions.size() )
		{
			CProcAction &action = proc.Actions[ actionIndex ];
			action.Action = ah;
			action.ParamBlocks.clear();
			action.ParamBlocks.resize( 1 );
			action.ParamBlocks[ 0 ].String = params;
		}
	}

	void CWidgetManager::getEditorSelection( std::vector< std::string > &selection )
	{
		selection.clear();
		for(uint i = 0; i < editorSelection.size(); ++i)
			selection.push_back(editorSelection[i]);
	}

	void CWidgetManager::selectWidget( const std::string &name )
	{
		std::vector< std::string >::iterator itr
			= std::find( editorSelection.begin(), editorSelection.end(), name );

		CInterfaceElement *e = getElementFromId( name );

		if( itr != editorSelection.end() )
		{
			// If multiselection is on unselect if already selected
			if( multiSelection )
			{
				editorSelection.erase( itr );
				if( e != NULL )
					e->setEditorSelected( false );
			}
		}
		else
		{
			// Select if not yet selected
			if( e != NULL )
			{
				// If multiselection is off, we can only have 1 widget selected
				if( !multiSelection )
				{
					editorSelection.clear();
				}

				e->setEditorSelected( true );
				editorSelection.push_back( name );
			}

		}

		notifySelectionWatchers();
	}

	void CWidgetManager::clearEditorSelection()
	{
		editorSelection.clear();
		notifySelectionWatchers();
	}

	void CWidgetManager::notifySelectionWatchers()
	{
		std::vector< IEditorSelectionWatcher* >::iterator itr = selectionWatchers.begin();
		while( itr != selectionWatchers.end() )
		{
			(*itr)->selectionChanged();
			++itr;
		}
	}

	void CWidgetManager::registerSelectionWatcher( IEditorSelectionWatcher *watcher )
	{
		std::vector< IEditorSelectionWatcher* >::iterator itr =
			std::find( selectionWatchers.begin(), selectionWatchers.end(), watcher );

		// We already have this watcher
		if( itr != selectionWatchers.end() )
			return;

		selectionWatchers.push_back( watcher );
	}

	void CWidgetManager::unregisterSelectionWatcher( IEditorSelectionWatcher *watcher )
	{
		std::vector< IEditorSelectionWatcher* >::iterator itr =
			std::find( selectionWatchers.begin(), selectionWatchers.end(), watcher );

		// We don't have this watcher
		if( itr == selectionWatchers.end() )
			return;

		selectionWatchers.erase( itr );
	}

	void CWidgetManager::onWidgetAdded( const std::string &id )
	{
		std::vector< IWidgetWatcher* >::const_iterator itr = widgetWatchers.begin();
		while( itr != widgetWatchers.end() )
		{
			(*itr)->onWidgetAdded( id );
			++itr;
		}
	}

	void CWidgetManager::onWidgetMoved( const std::string &oldid, const std::string &newid )
	{
		std::vector< IWidgetWatcher* >::const_iterator itr = widgetWatchers.begin();
		while( itr != widgetWatchers.end() )
		{
			(*itr)->onWidgetMoved( oldid, newid );
			++itr;
		}
	}

	void CWidgetManager::registerWidgetWatcher( IWidgetWatcher *watcher )
	{
		std::vector< IWidgetWatcher* >::const_iterator itr
			= std::find( widgetWatchers.begin(), widgetWatchers.end(), watcher );
		// already exists
		if( itr != widgetWatchers.end() )
			return;

		widgetWatchers.push_back( watcher );
	}

	void CWidgetManager::unregisterWidgetWatcher( IWidgetWatcher *watcher )
	{
		std::vector< IWidgetWatcher* >::iterator itr
			= std::find( widgetWatchers.begin(), widgetWatchers.end(), watcher );
		// doesn't exist
		if( itr == widgetWatchers.end() )
			return;

		widgetWatchers.erase( itr );
	}

	CInterfaceElement* CWidgetManager::addWidgetToGroup( std::string &group, std::string &widgetClass, std::string &widgetName )
	{
		// Check if this group exists
		CInterfaceElement *e = getElementFromId( group );
		if( e == NULL )
			return NULL;
		CInterfaceGroup *g = dynamic_cast< CInterfaceGroup* >( e );
		if( g == NULL )
			return NULL;

		// Check if an element already exists with that name
		if( g->getElement( widgetName ) != NULL )
			return NULL;

		// Create and add the new widget
		CViewBase *v = getParser()->createClass( widgetClass );
		if( v == NULL )
			return NULL;

		v->setId( std::string( g->getId() + ":" + widgetName ) );
		v->setParent( g );

		if( v->isGroup() )
			g->addGroup( dynamic_cast< CInterfaceGroup* >( v ) );
		else
		if( v->isCtrl() )
			g->addCtrl( dynamic_cast< CCtrlBase* >( v ) );
		else
			g->addView( v );

		onWidgetAdded( v->getId() );

		return v;
	}

	bool CWidgetManager::groupSelection()
	{
		std::vector< CInterfaceElement* > elms;

		// Resolve the widget names
		for(uint i = 0; i < editorSelection.size(); ++i)
		{
			CInterfaceElement *e = getElementFromId(editorSelection[i]);
			if (e != NULL)
				elms.push_back(e);
		}

		editorSelection.clear();

		if( elms.empty() )
			return false;

		// Create the group as the subgroup of the top window
		CInterfaceGroup *g = static_cast< CInterfaceGroup* >( getParser()->createClass( "interface_group" ) );
		getTopWindow()->addGroup( g );
		g->setParent( getTopWindow() );
		g->setIdRecurse( std::string( "group" ) + NLMISC::toString( _WidgetCount ) );
		_WidgetCount++;
		onWidgetAdded( g->getId() );

		std::string oldId;

		// Reparent the widgets to the new group
		for(uint i = 0; i < elms.size(); ++i)
		{
			CInterfaceElement *e = elms[i];
			oldId = e->getId();
			CInterfaceGroup *p = e->getParent();
			if (p != NULL)
				p->takeElement(e);

			g->addElement(e);
			e->setParent(g);
			e->setParentPos(g);
			e->setParentSize(g);
			e->setIdRecurse(e->getShortId());

			onWidgetMoved(oldId, e->getId());
		}
		elms.clear();

		// Make sure widgets aren't clipped because the group isn't big enough
		g->spanElements();
		// Make sure widgets are aligned
		g->alignElements();
		// Align the new group to the top window
		g->alignTo( getTopWindow() );

		g->setActive( true );

		return true;
	}

	bool CWidgetManager::unGroupSelection()
	{
		if( editorSelection.size() != 1 )
			return false;

		// Does the element exist?
		CInterfaceElement *e = getElementFromId( editorSelection[ 0 ] );
		if( e == NULL )
			return false;

		// Is the element a group?
		CInterfaceGroup *g = dynamic_cast< CInterfaceGroup* >( e );
		if( g == NULL )
			return false;

		// Can't blow up a root group :(
		CInterfaceGroup *p = g->getParent();
		if( p == NULL )
			return false;

		// KABOOM!
		bool ok = g->explode();
		if( !ok )
			return false;

		p->delElement( g );

		clearEditorSelection();

		p->updateCoords();

		return true;
	}


	bool CWidgetManager::createNewGUI( const std::string &project, const std::string &window )
	{
		reset();

		for(uint i = 0; i < _MasterGroups.size(); ++i)
			delete _MasterGroups[i].Group;
		_MasterGroups.clear();

		// First create the master group
		CRootGroup *root = new CRootGroup( CViewBase::TCtorParam() );

		SMasterGroup mg;
		mg.Group = root;

		root->setIdRecurse( project );
		root->setW( 1024 );
		root->setH( 768 );
		root->setActive( true );

		// Create the first / main window
		CInterfaceGroup *wnd = new CInterfaceGroup( CViewBase::TCtorParam() );
		wnd->setW( 1024 );
		wnd->setH( 768 );
		wnd->setParent( root );
		wnd->setParentPos( root );
		wnd->setParentSize( root );
		wnd->setPosRef( Hotspot_MM );
		wnd->setParentPosRef( Hotspot_MM );
		wnd->setIdRecurse( window );
		wnd->setActive( true );

		// Add the window
		root->addElement( wnd );
		mg.addWindow( wnd, wnd->getPriority() );
		_MasterGroups.push_back( mg );

		_Pointer = new CViewPointer( CViewBase::TCtorParam() );

		IParser *parser = getParser();


		// Set base color to white		
		VariableData v;
		v.type = "sint32";
		v.value = "255";

		v.entry = "UI:SAVE:COLOR:R";
		parser->setVariable( v );

		v.entry = "UI:SAVE:COLOR:G";
		parser->setVariable( v );

		v.entry = "UI:SAVE:COLOR:B";
		parser->setVariable( v );

		v.entry = "UI:SAVE:COLOR:A";
		parser->setVariable( v );

		return true;
	}


	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::notifyInterfaceScaleWatchers()
	{
		std::vector< IInterfaceScaleWatcher* >::iterator itr = scaleWatchers.begin();
		while( itr != scaleWatchers.end() )
		{
			(*itr)->onInterfaceScaleChanged();
			++itr;
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::registerInterfaceScaleWatcher( IInterfaceScaleWatcher *watcher )
	{
		std::vector< IInterfaceScaleWatcher* >::const_iterator itr
			= std::find( scaleWatchers.begin(), scaleWatchers.end(), watcher );

		if( itr != scaleWatchers.end() )
			return;

		scaleWatchers.push_back( watcher );
	}

	// ------------------------------------------------------------------------------------------------
	void CWidgetManager::unregisterInterfaceScaleWatcher( IInterfaceScaleWatcher *watcher )
	{
		std::vector< IInterfaceScaleWatcher* >::iterator itr
			= std::find( scaleWatchers.begin(), scaleWatchers.end(), watcher );

		if( itr == scaleWatchers.end() )
			return;

		scaleWatchers.erase( itr );
	}

	// ------------------------------------------------------------------------------------------------
	CWidgetManager::CWidgetManager()
	{
		LinkHack();
		CStringShared::createStringMapper();

		CReflectableRegister::registerClasses();

		_Parser = IParser::createParser();

		_Pointer = NULL;
		curContextHelp = NULL;
		_ContextHelpActive = true;
		_DeltaTimeStopingContextHelp = 0;
		_MaxTimeStopingContextHelp= 0.2f;
		_LastXContextHelp= -10000;
		_LastYContextHelp= -10000;

		resetColorProps();
		resetAlphaRolloverSpeedProps();
		resetGlobalAlphasProps();

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
		_MouseOverWindow = false;
		inGame = false;

		setScreenWH(0, 0);
		_InterfaceScale = 1.0f;

		_WindowSnapDistance = 10;
		_WindowSnapInvert = false;

		_GroupSelection = false;
		multiSelection = false;
		_WidgetCount = 0;
	}

	CWidgetManager::~CWidgetManager()
	{
		for (uint32 i = 0; i < _MasterGroups.size(); ++i)
		{
			delete _MasterGroups[i].Group;
		}

		delete _Parser;
		_Parser = NULL;

		_Pointer = NULL;
		curContextHelp = NULL;

		CStringShared::deleteStringMapper();

		editorSelection.clear();
	}

}

