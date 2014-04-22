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

#include "action_handler_misc.h"
#include "interface_manager.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "people_interraction.h"
#include "nel/misc/algo.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/interface_link.h"
#include "../client_chat_manager.h"
#include "../motion/user_controls.h"
#include "../entity_cl.h"
#include "../client_cfg.h"
#include "../fog_map.h"
#include "../sky_render.h"
#include "../continent_manager.h"
#include "../main_loop.h"
#include "../misc.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

////////////
// EXTERN //
////////////

extern CClientChatManager		ChatMngr;
extern UScene					*SceneRoot;
extern UScene					*SkyScene;
extern UScene					*Scene;
extern CFogState				MainFogState;
extern CFogState				RootFogState;
extern CLightCycleManager		LightCycleManager;
extern UCamera					MainCam;
extern CContinentManager		ContinentMngr;
extern NLMISC::CLog				g_log;

////////////
// static //
////////////
//static CCDBNodeLeaf *MenuColorWidgetValue = NULL; // db entry for the color menu widget (Red)


static const string ScreenshotsDirectory("screenshots/");	// don't forget the final /

void preRenderNewSky ();

// ***************************************************************************
class CAHCommand : public IActionHandler
{
public:

	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		ICommand::execute(Params, g_log);
	}
};
REGISTER_ACTION_HANDLER (CAHCommand, "command");

// ***************************************************************************
// ***************************************************************************
// class CActionHandlerShowOne
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
REGISTER_ACTION_HANDLER (CActionHandlerShowOne, "show_one");

// ***************************************************************************
void CActionHandlerShowOne::execute (CCtrlBase * /* pCaller */, const std::string &params)
{
	CInterfaceManager	*mngr= CInterfaceManager::getInstance();
	string wndListValue= getParam(params, "list");
	string wndShow= getParam(params, "show");

	// get the list
	vector<string>	wndList;
	splitString(wndListValue, ",", wndList);

	// hide all window from the list.
	for(uint i=0;i<wndList.size();i++)
	{
		CInterfaceElement	*wnd= CWidgetManager::getInstance()->getElementFromId(wndList[i]);
		if(wnd)
			wnd->setActive(false);
	}

	// show the one needed
	CInterfaceElement	*wnd= CWidgetManager::getInstance()->getElementFromId(wndShow);
	if(wnd)
		wnd->setActive(true);
}


// ***************************************************************************
REGISTER_ACTION_HANDLER (CActionHandlerActive, "set_active");
// ***************************************************************************
void CActionHandlerActive::execute (CCtrlBase * /* pCaller */, const std::string &params)
{
	std::string active = getParam(params, "active");
	std::string target = getParam(params, "target");
	CInterfaceExprValue activeValue;
	if (CInterfaceExpr::eval(active, activeValue, NULL))
	{
		if (!activeValue.toBool())
		{
			nlwarning("<CActionHandlerActive::execute> The 'active' param must be convertible to a boolean");
			return;
		}
		CInterfaceManager	*mngr = CInterfaceManager::getInstance();
		CInterfaceElement	*wnd = CWidgetManager::getInstance()->getElementFromId(target);
		if(!wnd)
		{
			nlwarning("<CActionHandlerActive::execute> Can't get window %s", target.c_str());
			return;
		}
		wnd->setActive(activeValue.getBool());
	}
	else
	{
		nlwarning("<CActionHandlerActive::execute> can't parse the 'active' param");
		return;
	}
}

// ***************************************************************************
REGISTER_ACTION_HANDLER (CActionHandlerSetOpen, "set_open");
// ***************************************************************************
void CActionHandlerSetOpen::execute (CCtrlBase * /* pCaller */, const std::string &params)
{
	std::string open   = getParam(params, "open");
	std::string target = getParam(params, "target");
	CInterfaceExprValue activeValue;
	if (CInterfaceExpr::eval(open, activeValue, NULL))
	{
		if (!activeValue.toBool())
		{
			nlwarning("<CActionHandlerActive::execute> The 'active' param must be co,vertible to a boolean");
			return;
		}
		CInterfaceManager	*mngr = CInterfaceManager::getInstance();
		CGroupContainer		*wnd = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(target));
		if(!wnd)
		{
			nlwarning("<CActionHandlerActive::execute> Can't get window %s", target.c_str());
			return;
		}
		wnd->setOpen(activeValue.getBool());
	}
	else
	{
		nlwarning("<CActionHandlerActive::execute> can't parse the 'active' param");
		return;
	}
}

// ***************************************************************************
// ***************************************************************************
// CActionHandlerHideClose
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
REGISTER_ACTION_HANDLER (CActionHandlerHideClose, "hide_close");

// ***************************************************************************
void CActionHandlerHideClose::execute (CCtrlBase * /* pCaller */, const std::string &params)
{
	CInterfaceManager	*mngr= CInterfaceManager::getInstance();
	string hideValue= getParam(params, "hide");
	string closeValue= getParam(params, "close");

	// get the list
	vector<string>	hideList;
	splitString(hideValue, ",", hideList);
	vector<string>	closeList;
	splitString(closeValue, ",", closeList);

	// hide all window from the hide list.
	uint i;
	for(i=0;i<hideList.size();i++)
	{
		CInterfaceElement	*wnd= CWidgetManager::getInstance()->getElementFromId(hideList[i]);
		if(wnd)
			wnd->setActive(false);
	}

	// close all containers from the hide list.
	for(i=0;i<closeList.size();i++)
	{
		// get a container if possible
		CInterfaceElement	*wnd= CWidgetManager::getInstance()->getElementFromId(closeList[i]);
		CGroupContainer		*pIC = dynamic_cast<CGroupContainer*>(wnd);
		if(pIC)
			pIC->close();
	}

}


// ***************************************************************************
// ***************************************************************************
// Misc
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
REGISTER_ACTION_HANDLER (CActionHandlerEnterModal, "enter_modal");
REGISTER_ACTION_HANDLER (CActionHandlerPushModal, "push_modal");
REGISTER_ACTION_HANDLER (CActionHandlerLeaveModal, "leave_modal");

// ***************************************************************************
void	CActionHandlerEnterModal::execute(CCtrlBase *pCaller, const std::string &params)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get the group from param
	string	groupName= getParam(params, "group");
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(groupName) );
	if(group)
	{
		UserControls.stopFreeLook();

		// enable the modal
		CWidgetManager::getInstance()->enableModalWindow(pCaller, group);
	}
	else
	{
		nlwarning("<CActionHandlerEnterModal::execute> Couldn't find group %s", groupName.c_str());
	}
}

// ***************************************************************************
void	CActionHandlerPushModal::execute(CCtrlBase *pCaller, const std::string &params)
{
	CInterfaceManager	*mngr= CInterfaceManager::getInstance();

	// get the group from param
	string	groupName= getParam(params, "group");
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(groupName) );
	if(group)
	{
		// enable the modal
		CWidgetManager::getInstance()->pushModalWindow(pCaller, group);
	}
	else
	{
		nlwarning("<CActionHandlerPushModal::execute> Couldn't find group %s", groupName.c_str());
	}
}


// ***************************************************************************
void	CActionHandlerLeaveModal::execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
{
	CInterfaceManager	*mngr= CInterfaceManager::getInstance();

	// quit the modal
	CWidgetManager::getInstance()->popModalWindow();
}


// ***************************************************************************
// proc
// ***************************************************************************
class CActionHandlerProc : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params)
	{
		// split the parameters
		vector<string>		paramList;
		splitString(params, "|", paramList);
		if(paramList.empty())
			return;

		// execute the procedure
		CWidgetManager::getInstance()->runProcedure(paramList[0], pCaller, paramList);
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerProc, "proc");


/** Confirm that a group container can be deactivated
  */
class CActionHandlerConfirmCanDeactivate : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		CGroupContainer::validateCanDeactivate(true);
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerConfirmCanDeactivate, "confirm_can_deactivate");

/** Cancel a group container deactivation
  */
class CActionHandlerCancelCanDeactivate : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		CGroupContainer::validateCanDeactivate(false);
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerCancelCanDeactivate, "cancel_can_deactivate");





// ***************************************************************************
// ***************************************************************************
// EditBox
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
class CActionHandlerEditBoxNumber : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &params)
	{
		CGroupEditBox		*pEditBox= dynamic_cast<CGroupEditBox*>(pCaller);
		if(!pEditBox)
			return;



		// get the dblink dest
		string	dblink= getParam(params, "value");
		if(dblink.empty())
			return;

		// get the value
		sint64	val= pEditBox->getInputStringAsInt64();

		string valueStr;
		// see if a max value has been set
		valueStr = getParam(params, "max_value");
		if (!valueStr.empty())
		{
			CInterfaceExprValue maxValue;
			if (!CInterfaceExpr::eval(valueStr, maxValue) || !maxValue.toInteger())
			{
				nlwarning("<CActionHandlerEditBoxNumber::execute> Can't eval maxValue, or can't convert to integer : %s", valueStr.c_str());
				return;
			}
			val = std::min(maxValue.getInteger(), val);
		}
		// see if a min value has been set
		valueStr = getParam(params, "min_value");
		if (!valueStr.empty())
		{
			CInterfaceExprValue minValue;
			if (!CInterfaceExpr::eval(valueStr, minValue) || !minValue.toInteger())
			{
				nlwarning("<CActionHandlerEditBoxNumber::execute> Can't eval minValue, or can't convert to integer : %s", valueStr.c_str());
				return;
			}
			val = std::max(minValue.getInteger(), val);
		}
		// set in the database
		CInterfaceProperty	prop;
		prop.link(dblink.c_str());
		prop.setSInt64(val);
		string	updateText = getParam(params, "update_text");
		bool    mustUpdate = updateText.empty() ? true : CInterfaceElement::convertBool(updateText.c_str());

		if (mustUpdate)
		{
			// replace the editbox string
			pEditBox->setInputStringAsInt64(val);
			pEditBox->setSelectionAll();
		}
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerEditBoxNumber, "editbox_number");

// ***************************************************************************
// Dynamic creation of interface links
// ***************************************************************************
/** Add a link to an interface element
  */
class CActionHandlerAddLink : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const std::string &params)
	{
		std::string expr	= getParam(params, "expr");
		std::string targets = getParam(params, "target");
		std::string id		= getParam(params, "id");
		std::string ah		= getParam(params, "action");
		std::string ahparam	= getParam(params, "params");
		std::string ahcond	= getParam(params, "cond");
		if (id.empty())
		{
			nlwarning("<CActionHandlerAddLink> Must specify a link's id");
			return;
		}
		CInterfaceGroup *parentGroup = dynamic_cast<CInterfaceGroup *>(pCaller);
		if (!parentGroup)
		{
			if (pCaller) parentGroup = pCaller->getParent();
		}

		std::vector<CInterfaceLink::CTargetInfo> targetsVect;
		std::vector<CInterfaceLink::CCDBTargetInfo> cdbTargetsVect;
		bool result = CInterfaceLink::splitLinkTargetsExt(targets, parentGroup, targetsVect, cdbTargetsVect);
		if (!result)
		{
			nlwarning("<CActionHandlerAddLink> Couldn't parse all links");
		}
		// add the link
		CInterfaceLink *il = new CInterfaceLink;
		il->init(targetsVect, cdbTargetsVect, expr, ah, ahparam, ahcond, parentGroup);
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CWidgetManager::getInstance()->getParser()->addLink(il, id);
		il->update();
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerAddLink, "add_link");

/** Remove a link from an interface element
  */
class CActionHandlerRemoveLink : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &params)
	{
		std::string id     = getParam(params, "id");
		if (id.empty())
		{
			nlwarning("<CActionHandlerRemoveLink> Must specify a link's id");
			return;
		}
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CWidgetManager::getInstance()->getParser()->removeLink(id);
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerRemoveLink, "remove_link");

// ***************************************************************************
REGISTER_ACTION_HANDLER (CActionHandlerEvalExpr, "eval_expr");
// ***************************************************************************
void CActionHandlerEvalExpr::execute(CCtrlBase * /* pCaller */, const std::string &params)
{
	std::string expr = getParam(params, "expr");
	if (expr.empty())
	{
		nlwarning("<CActionHandlerEvalExpr::execute> 'expr' parameter not found or empty.");
		return;
	}
	CInterfaceExprValue dummyResult; // result not used
	if (!CInterfaceExpr::eval(expr, dummyResult))
	{
		nlwarning("<CActionHandlerEvalExpr::execute> Couldn't eval expression");
	}
	return;
}

// ***************************************************************************
CInterfaceGroup *createMenuColorWidget(const string &colDbEntry,
									   const string &toolTipTextID,
									   const string &ccdTitle)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	pair<string, string> params[3] =
	{
		make_pair(string("col_db_entry"), colDbEntry),
		make_pair(string("tooltip"), toolTipTextID),
		make_pair(string("ccd_title"), ccdTitle),
	};
	return CWidgetManager::getInstance()->getParser()->createGroupInstance("menu_color_widget", "", params, 3);
}

// ***************************************************************************
struct CCameraBackup
{
	CViewport Viewport;
	CFrustum  Frustum;
};


// *********************************************************
CCameraBackup setupCameraForScreenshot(UScene &scene, uint left, uint right, uint top, uint bottom, uint screenShotWidth, uint screenShotHeight)
{
	CCameraBackup cb;
	cb.Frustum = scene.getCam().getFrustum();
	cb.Viewport = scene.getViewport();
	// Build a frustum
	CFrustum frustumPart;
	frustumPart.Left = cb.Frustum.Left+(cb.Frustum.Right-cb.Frustum.Left)*((float)left/(float)screenShotWidth);
	frustumPart.Right = cb.Frustum.Left+(cb.Frustum.Right-cb.Frustum.Left)*((float)right/(float)screenShotWidth);
	frustumPart.Top = cb.Frustum.Top+(cb.Frustum.Bottom-cb.Frustum.Top)*((float)top/(float)screenShotHeight);
	frustumPart.Bottom = cb.Frustum.Top+(cb.Frustum.Bottom-cb.Frustum.Top)*((float)bottom/(float)screenShotHeight);
	frustumPart.Near = cb.Frustum.Near;
	frustumPart.Far = cb.Frustum.Far;
	frustumPart.Perspective = cb.Frustum.Perspective;

	// Build a viewport
	CViewport viewport;
	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();
	viewport.init (0, 0, (float)(right-left)/Driver->getWindowWidth(),(float)(bottom-top)/Driver->getWindowHeight());

	// Activate all this
	scene.getCam().setFrustum (frustumPart);
	scene.setViewport (viewport);

	return cb;
}


// *********************************************************
static void restoreCamera(UScene &scene, const CCameraBackup &backup)
{
	scene.getCam().setFrustum (backup.Frustum);
	scene.setViewport(backup.Viewport);
}

// ***************************************************************************
void renderSceneScreenShot (uint left, uint right, uint top, uint bottom, uint screenShotWidth, uint screenShotHeight)
{
	CCameraBackup cbScene = setupCameraForScreenshot(*Scene, left, right, top, bottom, screenShotWidth, screenShotHeight);
	CCameraBackup cbCanopy = setupCameraForScreenshot(*SceneRoot, left, right, top, bottom, screenShotWidth, screenShotHeight);
	commitCamera();
	// sky setup are copied from main scene before rendering so no setup done here
	renderScene(ClientCfg.ScreenShotFullDetail, ClientCfg.Bloom);
	restoreCamera(*Scene, cbScene);
	restoreCamera(*SceneRoot, cbCanopy);
	commitCamera();
}

// ***************************************************************************

void getBuffer (CBitmap &btm)
{
	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();
	//
	if (ClientCfg.ScreenShotWidth && ClientCfg.ScreenShotHeight)
	{
		// Destination image
		CBitmap temp;
		btm.resize (ClientCfg.ScreenShotWidth, ClientCfg.ScreenShotHeight, CBitmap::RGBA);

		uint top;
		uint bottom = std::min (Driver->getWindowHeight (), ClientCfg.ScreenShotHeight);
		for (top=0; top<ClientCfg.ScreenShotHeight; top+=Driver->getWindowHeight ())
		{
			uint left;
			uint right = std::min (Driver->getWindowWidth (), ClientCfg.ScreenShotWidth);
			for (left=0; left<ClientCfg.ScreenShotWidth; left+=Driver->getWindowWidth ())
			{
				Driver->clearBuffers (CRGBA::Black);
				renderSceneScreenShot (left, right, top, bottom, ClientCfg.ScreenShotWidth, ClientCfg.ScreenShotHeight);
				// Get the bitmap
				Driver->getBuffer (temp);
				Driver->swapBuffers ();
				btm.blit (temp, 0, Driver->getWindowHeight ()-(bottom-top), right-left, bottom-top, left, top);

				// Next
				right = std::min (right+Driver->getWindowWidth (), ClientCfg.ScreenShotWidth);
			}

			// Next
			bottom = std::min (bottom+Driver->getWindowHeight (), ClientCfg.ScreenShotHeight);
		}
	}
	else
	{
		Driver->getBuffer(btm);
	}
}

void displayScreenShotSavedInfo(const string &filename)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	ucstring msg("'" + filename + "' " + CI18N::get("uiScreenshotSaved"));
	pIM->displaySystemInfo(msg);
}

void initScreenshot()
{
	if (!CFile::isExists(ScreenshotsDirectory)) CFile::createDirectory(ScreenshotsDirectory);
}

bool screenshotZBuffer(const std::string &filename)
{
	std::string::size_type pos = filename.find(".");

	if (pos == std::string::npos)
		return false;

	std::string filename_z = filename.substr(0, pos) + "_z" + filename.substr(pos);
	std::string ext = filename.substr(pos+1);

	std::vector<float> z;
	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();

	Driver->getZBuffer(z);

	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();

	// get min and max values
	for(uint i = 0; i < z.size(); ++i)
	{
		float value = z[i];
		if (value > max)
		{
			max = value;
		}
		else if (value < min)
		{
			min = value;
		}
	}

	max = max - min;

	CBitmap zi;
	zi.resize(Driver->getWindowWidth(), Driver->getWindowHeight());
	CRGBA *dest = (CRGBA *) &zi.getPixels()[0];

	for(uint k = 0; k < z.size(); ++k)
	{
		// normalize values
		uint8 i = (uint8) ((z[k] - min) * 255.f / max);
		dest->set(i, i, i, i);
		++ dest;
	}

	try
	{
		COFile f;
		f.open(filename_z);
		if (ext == "png")
			zi.writePNG(f, 32);
		else
			zi.writeTGA(f, 32);
	}
	catch(...)
	{
		return false;
	}

	return true;
}

void screenShotTGA()
{
	CBitmap btm;
	getBuffer (btm);

	string filename = CFile::findNewFile (ScreenshotsDirectory+"screenshot.tga");
	COFile fs(filename);

	if (!btm.writeTGA(fs, 24, false))
	{
		fs.close();
		CFile::deleteFile(filename);
		return;
	}

	if (ClientCfg.ScreenShotZBuffer)
		screenshotZBuffer(filename);

	nlinfo("Screenshot '%s' saved in tga format (%ux%u)", filename.c_str(), ClientCfg.ScreenShotWidth, ClientCfg.ScreenShotHeight);
	displayScreenShotSavedInfo(filename);
}

void screenShotPNG()
{
	CBitmap btm;
	getBuffer (btm);

	string filename = CFile::findNewFile (ScreenshotsDirectory+"screenshot.png");
	COFile fs(filename);

	if (!btm.writePNG(fs, 24))
	{
		fs.close();
		CFile::deleteFile(filename);
		return;
	}

	if (ClientCfg.ScreenShotZBuffer)
		screenshotZBuffer(filename);

	nlinfo("Screenshot '%s' saved in png format (%ux%u)", filename.c_str(), ClientCfg.ScreenShotWidth, ClientCfg.ScreenShotHeight);
	displayScreenShotSavedInfo(filename);
}

void screenShotJPG()
{
	CBitmap btm;
	getBuffer (btm);

	string filename = CFile::findNewFile (ScreenshotsDirectory+"screenshot.jpg");
	COFile fs(filename);

	if (!btm.writeJPG(fs))
	{
		fs.close();
		CFile::deleteFile(filename);
		return;
	}

	if (ClientCfg.ScreenShotZBuffer)
		screenshotZBuffer(filename);

	nlinfo("Screenshot '%s' saved in jpg format (%ux%u)", filename.c_str(), ClientCfg.ScreenShotWidth, ClientCfg.ScreenShotHeight);
	displayScreenShotSavedInfo(filename);
}

// ***************************************************************************

class CAHScreenShot : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		// if custom screenshot size is asked, then do it right now
		if (ClientCfg.ScreenShotWidth && ClientCfg.ScreenShotHeight)
		{
			// Custom screen shot ?
			screenShotTGA();
		}
		else
		{
			// post screenshot request
			ScreenshotRequest = ScreenshotRequestTGA;
		}
	}
};

// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHScreenShot, "screen_shot");

// ***************************************************************************
class CAHScreenShotJPG : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		// if custom screenshot size is asked, then do it right now
		if (ClientCfg.ScreenShotWidth && ClientCfg.ScreenShotHeight)
		{
			screenShotJPG();
		}
		else
		{
			// post screenshot request
			ScreenshotRequest = ScreenshotRequestJPG;
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHScreenShotJPG, "screen_shot_jpg");

// ***************************************************************************
class CAHScreenShotPNG : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		// if custom screenshot size is asked, then do it right now
		if (ClientCfg.ScreenShotWidth && ClientCfg.ScreenShotHeight)
		{
			screenShotPNG();
		}
		else
		{
			// post screenshot request
			ScreenshotRequest = ScreenshotRequestPNG;
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHScreenShotPNG, "screen_shot_png");


// ***************************************************************************
// Reply to the last people who talked in the chat -> this change the target of the main chat to the name of the last teller
class CAHReplyTeller : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		if (!PeopleInterraction.LastSenderName.empty())
		{
			CChatWindow *w = PeopleInterraction.ChatGroup.Window;
			if (w)
			{
				w->setKeyboardFocus();
				w->enableBlink(1);
				PeopleInterraction.ChatGroup.Filter.setTargetPlayer(CEntityCL::removeTitleAndShardFromName(PeopleInterraction.LastSenderName));
				CGroupEditBox *eb = w->getEditBox();
				if (eb != NULL)
				{
					eb->bypassNextKey();
				}

			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHReplyTeller, "reply_teller")

// ***************************************************************************
// Reply to the last people who talked in the chat only once (display '/tell name' in the last activated chat window)
class CAHReplyTellerOnce : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		// display a /tell command in the main chat
		if (!PeopleInterraction.LastSenderName.empty())
		{
			CChatWindow *w = PeopleInterraction.ChatGroup.Window;
			if (w)
			{
				w->setKeyboardFocus();
				w->enableBlink(1);
				w->setCommand(ucstring("tell ") + CEntityCL::removeTitleAndShardFromName(PeopleInterraction.LastSenderName) + ucstring(" "), false);
				CGroupEditBox *eb = w->getEditBox();
				if (eb != NULL)
				{
					eb->bypassNextKey();
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHReplyTellerOnce, "reply_teller_once")

// ***************************************************************************
/** Cycle through the last people on which a 'tell' has been done.
  * Focus must be in a window with a target (main chat or user chat), otherwise the main chat is used
  */
class CAHCycleTell : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (!im->isInGame()) return;
		const ucstring *lastTellPeople = ChatMngr.cycleLastTell();
		if (!lastTellPeople) return;
		// just popup the main chat
		//CChatWindow *w = PeopleInterraction.MainChat.Window;
		CChatWindow *w = PeopleInterraction.ChatGroup.Window;
		if (w)
		{
			w->setKeyboardFocus();
			w->enableBlink(1);
			//PeopleInterraction.MainChat.Filter.setTargetPlayer(*lastTellPeople);
			PeopleInterraction.ChatGroup.Filter.setTargetPlayer(*lastTellPeople);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHCycleTell, "cycle_tell")




// temp for test : set last sender name
NLMISC_COMMAND(slsn, "Temp : set the name of the last sender.", "<name>")
{
	if (args.size() != 1) return false;
	PeopleInterraction.LastSenderName = ucstring(args[0]);
	return true;
}

// ***************************************************************************
bool CStringPostProcessRemoveName::cbIDStringReceived(ucstring &inOut)
{
	// extract the replacement id
	ucstring strNewTitle = CEntityCL::getTitleFromName(inOut);

	// retrieve the translated string
	if (!strNewTitle.empty())
	{
		inOut = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(strNewTitle, Woman);
		{
			// Sometimes translation contains another title
			ucstring::size_type pos = inOut.find('$');
			if (pos != ucstring::npos)
			{
				inOut = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(inOut), Woman);
			}
		}
	}
	else
		inOut = "";

	return true;
}

// ***************************************************************************
bool CStringPostProcessRemoveTitle::cbIDStringReceived(ucstring &inOut)
{
	inOut = CEntityCL::removeTitleAndShardFromName(inOut);
	return true;
}

// ***************************************************************************
bool CStringPostProcessNPCRemoveTitle::cbIDStringReceived(ucstring &inOut)
{
	ucstring sOut = CEntityCL::removeTitleAndShardFromName(inOut);
	if (sOut.empty())
	{
		CStringPostProcessRemoveName SPPRM;
		SPPRM.cbIDStringReceived(inOut);
	}
	else
	{
		inOut = sOut;
	}
	return true;
}



// ***************************************************************************
class CAHAnimStart : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &Params)
	{
		string sAnim = getParam(Params, "anim");
		CWidgetManager::getInstance()->startAnim(sAnim);
	}
};
REGISTER_ACTION_HANDLER (CAHAnimStart, "anim_start");

// ***************************************************************************
class CAHAnimStop : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &Params)
	{
		string sAnim = getParam(Params, "anim");
		CWidgetManager::getInstance()->stopAnim(sAnim);
	}
};
REGISTER_ACTION_HANDLER (CAHAnimStop, "anim_stop");














