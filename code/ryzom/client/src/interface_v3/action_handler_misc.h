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



#ifndef NL_ACTION_HANDLER_MISC_H
#define NL_ACTION_HANDLER_MISC_H

#include "nel/misc/types_nl.h"
#include "nel/gui/action_handler.h"
#include "interface_manager.h"

namespace NLGUI
{
	class CInterfaceGroup;
}


// ***************************************************************************
/**
 * Show only one window and hide others
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CActionHandlerShowOne : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};


// ***************************************************************************
/**
 * hide some windows and close some group containers
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CActionHandlerHideClose : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};


// ***************************************************************************
/**
 * Activate a modal window after emptying the modal window stack.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CActionHandlerEnterModal : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};


// ***************************************************************************
/**
 * Activate a modal window, but keep previous modal windows (push on the modal windows stack)
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CActionHandlerPushModal : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};



// ***************************************************************************
/**
 * Exit from modal
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CActionHandlerLeaveModal : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};

// ***************************************************************************
/**
 * Activate a window from a parsed boolean value
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CActionHandlerActive : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};


// ***************************************************************************
/**
 * Open/Activate a container from a parsed boolean value
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CActionHandlerSetOpen : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};

// ***************************************************************************
/**
 * Eval an interface expression. The result isn't used, but the expression can contain functions that are in fact, procedures.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CActionHandlerEvalExpr : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params);
};



/** Build a menu color widget with the given tooltip and chooser title (ccdTitle)
  * (matching action handlers defined in action_handler_misc.cpp)
  */
CInterfaceGroup *createMenuColorWidget(const std::string &colDbEntry, const std::string &toolTipTextID, const std::string &ccdTitle);

// ***************************************************************************
// callback used by set_server_string and set_server_id action handlers and by addServerString and addServerId
// methods from CInterfaceManager to remove name from a received string like entityName$entityTitle$
class CStringPostProcessRemoveName : public CInterfaceManager::IStringProcess
{
public:
	CStringPostProcessRemoveName():Woman(false) {}
	bool Woman;
	bool cbIDStringReceived(ucstring &inOut);
};

// ***************************************************************************
// same as above but for title
class CStringPostProcessRemoveTitle : public CInterfaceManager::IStringProcess
{
public:
	bool cbIDStringReceived(ucstring &inOut);
};

// ***************************************************************************
// remove title except if the npc has only a title
class CStringPostProcessNPCRemoveTitle : public CInterfaceManager::IStringProcess
{
public:
	bool cbIDStringReceived(ucstring &inOut);
};


/** Capture current content of framebuffer and save the result. If a custom size is asked in ClientCfg, then the scene is rendered again
  * instead (possibly multiple time)
  */
void initScreenshot();
void screenShotTGA();
void screenShotPNG();
void screenShotJPG();


#endif // NL_ACTION_HANDLER_MISC_H

/* End of action_handler_misc.h */
