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


#ifndef WIDGET_MANAGER_H
#define WIDGET_MANAGER_H

#include <string>
#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/gui/interface_common.h"

class CInterfaceElement;
class CCtrlBase;
class CInterfaceGroup;
class CViewPointer;

class IParser
{
public:
	virtual void addParentPositionAssociation( CInterfaceElement *element, const std::string &parentID ) = 0;
	virtual void addParentSizeAssociation( CInterfaceElement *element, const std::string &parentID )     = 0;
	virtual void addParentSizeMaxAssociation( CInterfaceElement *element, const std::string &parentID )  = 0;
	virtual void addLuaClassAssociation( CInterfaceGroup *group, const std::string &luaScript )          = 0;
};

/// Manages the GUI widgets
class CWidgetManager{
public:

	// Master groups encapsulate all windows
	struct SMasterGroup
	{
		SMasterGroup()
		{
			Group = NULL;
			LastTopWindowPriority = WIN_PRIORITY_NORMAL;
		}

		CInterfaceGroup *Group;
		std::list< CInterfaceGroup* > PrioritizedWindows[ WIN_PRIORITY_MAX ];

		void addWindow( CInterfaceGroup *pIG, uint8 nPrio = WIN_PRIORITY_NORMAL );
		void delWindow( CInterfaceGroup *pIG );
		CInterfaceGroup *getWindowFromId( const std::string &winID );
		bool isWindowPresent( CInterfaceGroup *pIG );
		// Set a window top in its priority queue
		void setTopWindow( CInterfaceGroup *pIG );
		void setBackWindow( CInterfaceGroup *pIG );
		void deactiveAllContainers();
		void centerAllContainers();
		void unlockAllContainers();

		// Sort the world space group
		void sortWorldSpaceGroup ();

		uint8 LastTopWindowPriority;
	};


	// Infos about a modal window.
	struct SModalWndInfo
	{
		// Yoyo: store as CRefPtr in case they are deleted (can happen for instance if menu right click on a guild memeber, and guild members are udpated after)
		NLMISC::CRefPtr< CInterfaceGroup > ModalWindow; // the current modal window
		NLMISC::CRefPtr< CCtrlBase > CtrlLaunchingModal;
		bool ModalClip;
		bool ModalExitClickOut;
		bool ModalExitClickL;
		bool ModalExitClickR;
		bool ModalExitKeyPushed;
		std::string ModalHandlerClickOut;
		std::string ModalClickOutParams;

		SModalWndInfo()
		{
			ModalWindow = NULL;
			CtrlLaunchingModal = NULL;
			ModalExitClickOut = false;
			ModalExitClickL = false;
			ModalExitClickR = false;
			ModalExitKeyPushed = false;
		}
	};


	static CWidgetManager* getInstance();
	static void release();
	
	CInterfaceGroup* getMasterGroupFromId( const std::string &MasterGroupName );
	std::vector< SMasterGroup > &getAllMasterGroup(){ return _MasterGroups; }
	SMasterGroup& getMasterGroup( uint8 i ) { return _MasterGroups[ i ]; }
	CInterfaceGroup* getWindowFromId( const std::string &groupId );
	void addWindowToMasterGroup( const std::string &sMasterGroupName, CInterfaceGroup *pIG );
	void removeWindowFromMasterGroup( const std::string &sMasterGroupName, CInterfaceGroup *pIG );
	void removeAllMasterGroups();

	void activateMasterGroup (const std::string &sMasterGroupName, bool bActive);

	CInterfaceElement* getElementFromId( const std::string &sEltId );
	CInterfaceElement* getElementFromId( const std::string &sStart, const std::string &sEltId );

	/// Get the window from an element (ui:interface:###)
	CInterfaceGroup* getWindow(CInterfaceElement*);


	/**
	 * set the top window
	 * \param win : pointer to the window to be set on top
	 */
	void setTopWindow (CInterfaceGroup *pWin);

	/**
	 * set the back window
	 * \param win : pointer to the window to be set on top
	 */
	void setBackWindow (CInterfaceGroup *pWin);

	/** get the top window in the first activated masterGroup
	 */
	CInterfaceGroup* getTopWindow (uint8 nPriority = WIN_PRIORITY_NORMAL) const;

	/** get the back window in the first activated masterGroup
	 */
	CInterfaceGroup* getBackWindow (uint8 nPriority = WIN_PRIORITY_NORMAL) const;

	/** get the last escapable top window in the first activated masterGroup
	 */
	CInterfaceGroup* getLastEscapableTopWindow() const;

	void setWindowPriority (CInterfaceGroup *pWin, uint8 nPriority);

	/** return the priority of the Last Window setTopWindow()-ed.
	 */
	uint8 getLastTopWindowPriority() const;

	bool hasModal() const;

	SModalWndInfo& getModal();

	bool isPreviousModal( CInterfaceGroup *wnd ) const;

	void enableModalWindow (CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG);
	void enableModalWindow (CCtrlBase *ctrlLaunchingModal, const std::string &groupName);
	// Disable all modals windows
	void disableModalWindow ();

	/** Push a modal window that becomes the current modal window
	  */
	void pushModalWindow(CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG);
	void pushModalWindow (CCtrlBase *ctrlLaunchingModal, const std::string &groupName);
	void popModalWindow();
	// pop all top modal windows with the given category (a string stored in the modal)
	void popModalWindowCategory(const std::string &category);

	CCtrlBase *getCtrlLaunchingModal ()
	{
		if (_ModalStack.empty()) return NULL;
		return _ModalStack.back().CtrlLaunchingModal;
	}
	/// get the currently active modal window, or NULL if none
	CInterfaceGroup *getModalWindow() const
	{
		if (_ModalStack.empty()) return NULL;
		return _ModalStack.back().ModalWindow;
	}

	void setCurContextHelp( CCtrlBase *curContextHelp ){ this->curContextHelp = curContextHelp; }
	CCtrlBase* getCurContextHelp(){ return curContextHelp; }

	float _DeltaTimeStopingContextHelp;

	CViewPointer* getPointer(){ return _Pointer; }
	void setPointer( CViewPointer *pointer ){ _Pointer = pointer; }

	static IParser *parser;

private:
	CWidgetManager();
	~CWidgetManager();

	static CWidgetManager *instance;
	std::vector< SMasterGroup > _MasterGroups;
	std::vector< SModalWndInfo > _ModalStack;
	static std::string _CtrlLaunchingModalId;
	NLMISC::CRefPtr< CCtrlBase > curContextHelp;
	CViewPointer *_Pointer;
};

#endif

