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

//////////////
// Includes //
//////////////
#include "stdpch.h"

#include "login.h"
#include "login_patch.h"

#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/thread.h"
#include "nel/misc/big_file.h"
#include "nel/misc/system_utils.h"

#include "nel/net/tcp_sock.h"
#include "nel/3d/u_driver.h"
#include "nel/misc/big_file.h"

#include "interface_v3/interface_manager.h"
#include "interface_v3/input_handler_manager.h"
#include "nel/gui/group_editbox.h"
#include "interface_v3/group_quick_help.h"
#include "nel/gui/view_text.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "sound_manager.h"
#include "far_tp.h"

#include "actions_client.h"
#include "time_client.h"
#include "client_cfg.h"
#include "global.h"
#include "input.h"
#include "nel/gui/libwww.h"
#include "http_client_curl.h"
#include "login_progress_post_thread.h"

#include "init.h"
#include "release.h"
#include "bg_downloader_access.h"

#include "game_share/crypt.h"
#include "game_share/bg_downloader_msg.h"

#include "misc.h"

void ConnectToShard();

// ***************************************************************************

using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;
using namespace std;

// ***************************************************************************

vector<CShard> Shards;

string LoginLogin, LoginPassword, ClientApp, Salt;
uint32 LoginShardId = 0xFFFFFFFF;


// Completed only after that ryzom downloader has run its 'check' task :
// It is a bitfield indexed by BGDownloader::TDownloadID that indicate the parts that are available for patch
uint32 AvailablePatchs = 0;
// next wanted patch for the background downloader
BGDownloader::TDownloadID BGDownloaderWantedPatch = BGDownloader::DownloadID_RoS;



// R2 mode var ---------------

/// domain server version for patch
string	R2ServerVersion;
/// name of the version (used to alias many version under the same name),
/// the value is used to get the release not if not empty
string	VersionName;
/// Backup patch server to use in case of failure of all other servers
string	R2BackupPatchURL;
/// a list of patch server to use randomly
vector<string>	R2PatchURLs;


#define CTRL_EDITBOX_LOGIN		"ui:login:checkpass:content:eb_login:eb"
#define CTRL_EDITBOX_PASSWORD	"ui:login:checkpass:content:eb_password:eb"
#define GROUP_LIST_SHARD		"ui:login:sharddisp:content:shard_list"
#define CTRL_BUTTON_CONNECT		"ui:login:sharddisp:content:but_con"
#define GROUP_LIST_CAT			"ui:login:catdisp:content:cat_list"
#define CTRL_BUTTON_CLOSE_PATCH	"ui:login:patching:content:but_close"
#define VIEW_TOTAL_SIZE			"ui:login:catdisp:content:global_patch:size"
#define VIEW_NON_OPTIONAL_SIZE	"ui:login:catdisp:content:nonopt_patch:size"
#define VIEW_TOTAL_SIZE_PATCH	"ui:login:patching:content:global_patch:size"
#define CTRL_BUTTON_BACKTOLOGIN	"ui:login:webstart:content:back_to_login"
#define CTRL_BUTTON_RELOADTESTPAGE	"ui:login:webstart:content:reload_test_page"
#define CTRL_EDITBOX_CREATEACCOUNT_LOGIN			"ui:login:create_account:content:submit_gr:eb_login:eb"
#define CTRL_EDITBOX_CREATEACCOUNT_PASSWORD			"ui:login:create_account:content:submit_gr:eb_password:eb"
#define CTRL_EDITBOX_CREATEACCOUNT_CONFIRMPASSWORD	"ui:login:create_account:content:submit_gr:eb_confirm_password:eb"
#define CTRL_EDITBOX_CREATEACCOUNT_EMAIL			"ui:login:create_account:content:submit_gr:eb_email:eb" 

#define UI_VARIABLES_SCREEN_CHECKPASS		0
#define UI_VARIABLES_SCREEN_SHARDDISP		1
#define UI_VARIABLES_SCREEN_CHECKING		2
#define UI_VARIABLES_SCREEN_CATDISP			3
#define UI_VARIABLES_SCREEN_PATCHING		4
#define UI_VARIABLES_SCREEN_REBOOT			5
#define UI_VARIABLES_SCREEN_EULA			6
#define UI_VARIABLES_SCREEN_DATASCAN		7
#define UI_VARIABLES_SCREEN_CREATE_ACCOUNT	9


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// INTERFACE
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

bool loginFinished = false;
bool loginOK = false;
sint32 ShardSelected = -1;
CPatchManager::SPatchInfo InfoOnPatch;
uint32 TotalPatchSize;

CLoginStateMachine LoginSM;

// TODO : nico : put this in an external file, this way it isn't included by the background downloader
#ifndef RY_BG_DOWNLOADER

bool CStartupHttpClient::connectToLogin()
{
	return connect(ClientCfg.ConfigFile.getVar("StartupHost").asString(0));
}

CStartupHttpClient HttpClient;
#endif // RY_BG_DOWNLOADER


// ***************************************************************************
#define	WIN_COMBO_BOX_SELECT_MENU	"ui:interface:combo_box_select_menu"
#define	WIN_COMBO_BOX_MEASURE_MENU	"ui:interface:combo_box_measure_menu"
#define	WIN_COMBO_BOX_SELECT_MENU_OUTGAME	"ui:outgame:combo_box_select_menu"
#define	WIN_COMBO_BOX_SELECT_MENU_LOGIN	"ui:login:combo_box_select_menu"
#define	WIN_COMBO_BOX_MEASURE_MENU_LOGIN	"ui:login:combo_box_measure_menu"

bool isLoginFinished()
{
	return loginFinished;
}

void setLoginFinished( bool f )
{
	loginFinished = f;
	if( loginFinished )
	{
		CDBGroupComboBox::measureMenu.assign( WIN_COMBO_BOX_MEASURE_MENU );
		CDBGroupComboBox::selectMenu.assign( WIN_COMBO_BOX_SELECT_MENU );
	}
	else
	{
		CDBGroupComboBox::measureMenu.assign( WIN_COMBO_BOX_MEASURE_MENU_LOGIN );
		CDBGroupComboBox::selectMenu.assign( WIN_COMBO_BOX_SELECT_MENU_LOGIN );
	}
}


// ***************************************************************************
// Pop a fatal error message box, giving the option to 'quit' the client, plus a help button
static void fatalMessageBox(const ucstring &msg)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->messageBoxWithHelp(msg, "ui:login", "login_quit");
}

// ***************************************************************************
// Pop an error message box, giving the option to go back to main menu, plus a help button
static void errorMessageBox(const ucstring &msg)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->messageBoxWithHelp(msg, "ui:login", "on_back_to_login");
}

// ***************************************************************************
void createOptionalCatUI()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_CAT));
	if (pList == NULL)
	{
		nlwarning("element "GROUP_LIST_CAT" not found probably bad login_main.xml");
		return;
	}

	// Update optional categories

	CInterfaceGroup *pPrevLine = NULL;
	for(uint i = 0; i < InfoOnPatch.OptCat.size(); i++)
	{
		vector< pair < string, string > > params;
		params.clear();
		params.push_back(pair<string,string>("id", "c"+toString(i)));
		if (i>0)
			params.push_back(pair<string,string>("posref", "BL TL"));

		CInterfaceGroup *pNewLine = CWidgetManager::getInstance()->getParser()->createGroupInstance("t_cat", GROUP_LIST_CAT, params);
		if (pNewLine != NULL)
		{
			CViewText *pVT = dynamic_cast<CViewText*>(pNewLine->getView("name"));
			if (pVT != NULL) pVT->setText(InfoOnPatch.OptCat[i].Name);
			pVT = dynamic_cast<CViewText*>(pNewLine->getView("size"));
			if (pVT != NULL)
			{
				pVT->setText(BGDownloader::getWrittenSize(InfoOnPatch.OptCat[i].Size));
			}

			// Add to the list
			pNewLine->setParent(pList);
			pNewLine->setParentSize(pList);
			pNewLine->setParentPos(pPrevLine);
			pList->addGroup(pNewLine);

			pPrevLine = pNewLine;
		}
	}
	pList->invalidateCoords();
}

// ***************************************************************************
void initEula()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (!ClientCfg.SkipEULA && CFile::fileExists(getLogDirectory() + "show_eula"))
	{
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_EULA);

		// if we display the eula, it means we make a patch so we clean the cache directory
		// (clear cache after each patch)
		nlinfo("Deleting cached files");
		vector<string> cached;
		CPath::getPathContent("cache", true, false, true, cached);
		for(uint i = 0; i < cached.size(); i++)
			CFile::deleteFile(cached[i]);
	}
	else
	{
		CAHManager::getInstance()->runActionHandler("accept_eula", NULL);
	}
}

// ***************************************************************************
static void setDataScanLog(const ucstring &text)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:datascan:content:log_txt:log"));
	if (pVT != NULL)
	{
		pVT->setText(text);
	}
}

// ***************************************************************************
static void setDataScanState(const ucstring &text, ucstring progress= ucstring())
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:datascan:content:state"));
	if (pVT != NULL) pVT->setText(text);

	pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:datascan:content:progress"));
	if (pVT != NULL) pVT->setText(progress);
}

void initCatDisplay()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CPatchManager *pPM = CPatchManager::getInstance();

	// Check is good now ask the player if he wants to apply the patch
	pPM->getInfoToDisp(InfoOnPatch);

	if ((InfoOnPatch.NonOptCat.size() > 0) ||
		(InfoOnPatch.OptCat.size() > 0) ||
		(InfoOnPatch.ReqCat.size() > 0))
	{
		createOptionalCatUI();
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CATDISP);
	}
	else
	{
		LoginSM.pushEvent(CLoginStateMachine::ev_run_patch);
//		CAHManager::getInstance()->runActionHandler("login_patch", NULL);
	}
}

void initReboot()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_REBOOT);
}


// ***************************************************************************
static void setPatcherStateText(const std::string &baseUIPath, const ucstring &str)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(baseUIPath + ":content:state"));
	if (pVT != NULL)
	{
		pVT->setText(str);
	}
}

// ***************************************************************************
static void setPatcherProgressText(const std::string &baseUIPath, const ucstring &str)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(baseUIPath + ":content:progress"));
	if (pVT != NULL)
	{
		pVT->setText(str);
	}
}

// ***************************************************************************
static void updatePatchingInfoText(const std::string &baseUIPath)
{
	CPatchManager *pPM = CPatchManager::getInstance();
	CBGDownloaderAccess &bgDownloader = CBGDownloaderAccess::getInstance();
	if (isBGDownloadEnabled())
	{
		bgDownloader.update();
		if (bgDownloader.getDownloadThreadPriority() == BGDownloader::ThreadPriority_Paused)
		{
			setPatcherStateText(baseUIPath, CI18N::get("uiBGD_Paused"));
			setPatcherProgressText(baseUIPath, ucstring());
		}
		else
		{
			setPatcherStateText(baseUIPath, bgDownloader.getCurrentMessage());
			if (bgDownloader.getTotalFilesToGet() != 0)
			{
				setPatcherProgressText(baseUIPath, toString("%d/%d", (int) bgDownloader.getCurrentFilesToGet(), (int) bgDownloader.getTotalFilesToGet()));
			}
			else
			{
				setPatcherProgressText(baseUIPath, ucstring());
			}
		}
	}
	else
	{
		ucstring state;
		vector<ucstring> log;
		if(pPM->getThreadState(state, log))
		{
			setPatcherStateText(baseUIPath, state);
			if (pPM->getTotalFilesToGet() != 0)
			{
				setPatcherProgressText(baseUIPath, toString("%d/%d", pPM->getCurrentFilesToGet(), pPM->getTotalFilesToGet()));
			}
			else
			{
				setPatcherProgressText(baseUIPath, ucstring());
			}
		}
	}
}

// ***************************************************************************
// Main loop of the login step
void loginMainLoop()
{
	CDBGroupComboBox::selectMenuOut.assign( WIN_COMBO_BOX_SELECT_MENU_OUTGAME );
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CPatchManager *pPM = CPatchManager::getInstance();

	CBGDownloaderAccess &bgDownloader = CBGDownloaderAccess::getInstance();

	bool windowBlinkDone = false;
	bool fatalMessageBoxShown = false;

	while (LoginSM.getCurrentState() != CLoginStateMachine::st_connect
			&& LoginSM.getCurrentState() != CLoginStateMachine::st_end
			&& LoginSM.getCurrentState() != CLoginStateMachine::st_ingame)
//	while (loginFinished == false)
	{
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();

		// Update the DT T0 and T1 global variables
		updateClientTime();



		CInputHandlerManager::getInstance()->pumpEvents();
		Driver->clearBuffers(CRGBA::Black);
		Driver->setMatrixMode2D11();

		// Update sound
		if (SoundMngr != NULL)
			SoundMngr->update();

		// Interface handling & displaying
		pIM->updateFrameEvents();
		pIM->updateFrameViews(NULL);
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();



//		if (::GetAsyncKeyState(VK_SPACE))
//		{
//			pIM->displayUIViewBBoxs("");
//			pIM->displayUICtrlBBoxs("");
//			pIM->displayUIGroupBBoxs("");
//		}

		// Force the client to sleep a bit.
		if(ClientCfg.Sleep >= 0)
		{
			nlSleep(ClientCfg.Sleep);
		}
		// Display
		Driver->swapBuffers();

//		sint32 screen = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->getValue32();
		if (LoginSM.getCurrentState() == CLoginStateMachine::st_check_patch)
//		if (screen == UI_VARIABLES_SCREEN_CHECKING) // If we are in checking mode
		{
			nlSleep(10); // For the checking thread
			bool res = false;
			BGDownloader::TTaskResult taskResult = BGDownloader::TaskResult_Unknown;
			bool finished = false;
			ucstring bgDownloaderError;
			if (isBGDownloadEnabled())
			{
				finished = bgDownloader.isTaskEnded(taskResult, bgDownloaderError);
			}
			else
			{
				finished = pPM->isCheckThreadEnded(res);
			}

			if (finished)
			{
				setPatcherStateText("ui:login:checking", ucstring());
				setPatcherProgressText("ui:login:checking", ucstring());

				if (isBGDownloadEnabled())
				{
					AvailablePatchs = bgDownloader.getAvailablePatchs();
					#ifdef NL_OS_WINDOWS
					{
						// Get the window
						HWND hWnd = Driver->getDisplay();
						nlassert (hWnd);
						// Show the window, unless it has been minimized, in
						// which case we don't pop it unexpectedly
						if (!windowBlinkDone)
						{
							bgDownloader.hideDownloader();
							ShowWindow (hWnd, SW_RESTORE);
							windowBlinkDone = true;
						}

					}
					#endif
					switch(taskResult)
					{
						case BGDownloader::TaskResult_Success:
							if (AvailablePatchs & (1 << BGDownloaderWantedPatch)) // is there a patch for what we want now ?
							{
								LoginSM.pushEvent(CLoginStateMachine::ev_patch_needed);
							}
							else
							{
								LoginSM.pushEvent(CLoginStateMachine::ev_no_patch);
							}
						break;
						case BGDownloader::TaskResult_Error:
						{
							if (!fatalMessageBoxShown)
							{
								fatalMessageBox(bgDownloaderError);
								fatalMessageBoxShown = true;
							}
						}
						break;
						default:
							if (!fatalMessageBoxShown)
							{
								fatalMessageBox(CI18N::get("uiErrChecking"));
								fatalMessageBoxShown = true;
							}
						break;
					}

				}
				else
				{
					if(res)
					{
						// Check is good now ask the player if he wants to apply the patch
						pPM->getInfoToDisp(InfoOnPatch);

						AvailablePatchs = InfoOnPatch.getAvailablePatchsBitfield();

						if ((InfoOnPatch.NonOptCat.size() > 0) ||
							(InfoOnPatch.OptCat.size() > 0) ||
							(InfoOnPatch.ReqCat.size() > 0))
						{
							LoginSM.pushEvent(CLoginStateMachine::ev_patch_needed);
							//	createOptionalCatUI();
							//	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CATDISP);
						}
						else
						{
							LoginSM.pushEvent(CLoginStateMachine::ev_no_patch);
							//	CAHManager::getInstance()->runActionHandler("login_patch", NULL);
						}
					}
					else
					{
						ucstring errMsg = CI18N::get("uiErrChecking");
						if (!pPM->getLastErrorMessage().empty())
						{
							errMsg = pPM->getLastErrorMessage();
						}
						if (!fatalMessageBoxShown)
						{
							fatalMessageBox(errMsg);
							fatalMessageBoxShown = true;
						}
					}
				}
			}
			else
			{
				// update interface content
				updatePatchingInfoText("ui:login:checking");
			}
		}
//		else if (screen == UI_VARIABLES_SCREEN_DATASCAN) // If we are in ScanData mode
		else if (LoginSM.getCurrentState() == CLoginStateMachine::st_scan_data)
		{
			nlSleep(10); // For the checking thread

			bool	res;
			if (pPM->isScanDataThreadEnded(res))
			{
				// if interface consider it was running before
				if(NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DATASCAN_RUNNING")->getValue32()!=0)
				{
					// no more running
					NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DATASCAN_RUNNING")->setValue32(0);

					if(res)
					{
						// touch any file with checksum error, and get their number
						uint	numFiles= pPM->applyScanDataResult();

						// Report result
						if(numFiles==0)
							setDataScanState(CI18N::get("uiScanDataSucess"));
						else
						{
							ucstring	fmt= CI18N::get("uiScanDataErrors");
							strFindReplace(fmt, "%d", toString(numFiles));
							setDataScanState(fmt);
						}
					}
					else
					{
						ucstring errMsg = CI18N::get("uiErrDataScanning");
						if (!pPM->getLastErrorMessage().empty())
						{
							errMsg = pPM->getLastErrorMessage();
						}
						pIM->messageBoxWithHelp(errMsg, "ui:login");
					}

					// the log may have changed
					ucstring	dsLog;
					if(pPM->getDataScanLog(dsLog))
						setDataScanLog(dsLog);
				}
			}
			else
			{
				// update inteface content
				ucstring state;
				vector<ucstring> log;
				// get state
				if(pPM->getThreadState(state, log))
				{
					// set state
					setDataScanState(state, toString("%d/%d", pPM->getCurrentFilesToGet(), pPM->getTotalFilesToGet()));
				}
				// set special data scan log
				ucstring	dsLog;
				if(pPM->getDataScanLog(dsLog))
					setDataScanLog(dsLog);
			}
		}
//		else if (screen == UI_VARIABLES_SCREEN_PATCHING) // If we are in patching mode
		else if (LoginSM.getCurrentState() == CLoginStateMachine::st_patch)
		{
			nlSleep(30); // For the patching thread

			int currentPatchingSize;
			int totalPatchSize;


			if (isBGDownloadEnabled())
			{
				currentPatchingSize = bgDownloader.getPatchingSize();
				totalPatchSize = bgDownloader.getTotalSize();
				BGDownloader::TTaskResult taskResult;
				bool finished = false;
				ucstring bgDownloaderError;
				finished = bgDownloader.isTaskEnded(taskResult, bgDownloaderError);
				if (finished)
				{
					//bgDownloader.hideDownloader();
					// restore the search paths (all bnp files were removed from CPath to allows
					// the patcher to overwrite them)

					// create a file to prompt eula next time
					CFile::createEmptyFile(getLogDirectory() + "show_eula");

					if (taskResult == BGDownloader::TaskResult_Error)
					{
						setPatcherStateText("ui:login:patching", ucstring());
						setPatcherProgressText("ui:login:patching", ucstring());

						if (!fatalMessageBoxShown)
						{
							fatalMessageBox(bgDownloaderError);
							fatalMessageBoxShown = true;
						}
					}
					else
					{
						bgDownloader.getPatchCompletionFlag(true /* clear flag */);
						LoginSM.pushEvent(CLoginStateMachine::ev_close_patch);
					}
				}
				else
				{
					updatePatchingInfoText("ui:login:patching");
				}
			}
			else
			{
				totalPatchSize = TotalPatchSize;
				currentPatchingSize = pPM->getPatchingSize();
				bool res;
				if (pPM->isPatchThreadEnded(res))
				{
					if(res)
					{
						LoginSM.pushEvent(CLoginStateMachine::ev_close_patch);
					}
					else
					{
						ucstring errMsg = CI18N::get("uiErrPatchApply");
						if (!pPM->getLastErrorMessage().empty())
						{
							errMsg = pPM->getLastErrorMessage();
						}
						if (!fatalMessageBoxShown)
						{
							fatalMessageBox(errMsg);
							fatalMessageBoxShown = true;
						}
					}
				}
				else
				{
					updatePatchingInfoText("ui:login:patching");
				}
			}

			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(VIEW_TOTAL_SIZE_PATCH));
			ucstring sTmp;
			sTmp = BGDownloader::getWrittenSize(currentPatchingSize);
			sTmp += " / " + BGDownloader::getWrittenSize(totalPatchSize);
			if (pVT != NULL) pVT->setText(sTmp);
		}
//		else if (screen == UI_VARIABLES_SCREEN_CATDISP) // If we are displaying patch info
		else if (LoginSM.getCurrentState() == CLoginStateMachine::st_display_cat)
		{
			// Non optional stuff (non opt cat and req cat)

			// Add req cat size : given the optional cat we determines the required cat
			uint32 nNonOptSize = 0;
			TotalPatchSize = 0;
			vector<sint32> ReqCat;
			CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_CAT));
			if (pList != NULL)
			{
				for(uint i = 0; i < InfoOnPatch.OptCat.size(); i++)
				{
					CInterfaceGroup *pLine = pList->getGroup("c"+toString(i));
					if (pLine != NULL)
					{
						CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(pLine->getCtrl("on_off"));
						if ((pCB != NULL) && (pCB->getPushed()))
						{
							TotalPatchSize += InfoOnPatch.OptCat[i].Size;
							if (InfoOnPatch.OptCat[i].Req != -1)
							{
								uint32 j;
								for (j = 0; j < ReqCat.size(); ++j)
									if (ReqCat[j] == InfoOnPatch.OptCat[i].Req)
										break;
								// Add just once the req cat size to the non optional size
								if (j == ReqCat.size())
								{
									ReqCat.push_back(InfoOnPatch.OptCat[i].Req);
									nNonOptSize += InfoOnPatch.ReqCat[InfoOnPatch.OptCat[i].Req].Size;
								}
							}
						}
					}
				}
			}

			// Add non optional cats
			for (uint32 i = 0; i < InfoOnPatch.NonOptCat.size(); ++i)
				nNonOptSize += InfoOnPatch.NonOptCat[i].Size;

			TotalPatchSize += nNonOptSize;
			// Total size of the patches is optional cats + required cat (f(optCat)) + non opt cat

			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(VIEW_TOTAL_SIZE));
			if (pVT != NULL) pVT->setText(BGDownloader::getWrittenSize(TotalPatchSize));

			pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(VIEW_NON_OPTIONAL_SIZE));
			if (pVT != NULL) pVT->setText(BGDownloader::getWrittenSize(nNonOptSize));
		}
	}
}

void initLoginScreen()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CPatchManager *pPM = CPatchManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKPASS);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DISPLAY_ACCOUNT_BUTTONS")->setValue32(ClientCfg.DisplayAccountButtons);

	// Active inputs
	Actions.enable(true);
	EditActions.enable(true);

	if(ClientCfg.ConfigFile.exists("VerboseLog"))
		pPM->setVerboseLog(ClientCfg.ConfigFile.getVar("VerboseLog").asInt() == 1);
	if(pPM->isVerboseLog()) nlinfo("Using verbose log mode");

	ClientApp = ClientCfg.ConfigFile.getVar("Application").asString(0);

	string l = ClientCfg.LastLogin;

	if(!l.empty())
	{
		CGroupEditBox *pGEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_LOGIN));
		if (pGEB != NULL && (pGEB->getInputString().empty()))
		{
			pGEB->setInputString(l);
		}
		CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_PASSWORD "|select_all=false");
	}
	else
	{
		CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_LOGIN "|select_all=false");
	}


	CCtrlTextButton *pCB = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId(CTRL_BUTTON_CONNECT));
	if (pCB != NULL) pCB->setActive(false);

	setLoginFinished( false );
	loginOK = false;
}

void initAutoLogin()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupEditBox *pGEBLog = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_LOGIN));
	CGroupEditBox *pGEBPwd = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_PASSWORD));
	pGEBLog->setInputString(LoginLogin);
	pGEBPwd->setInputString(LoginPassword);
	CAHManager::getInstance()->runActionHandler("on_login", NULL, "");

	if (ClientCfg.R2Mode)
	{
		LoginSM.pushEvent(CLoginStateMachine::ev_login_ok);
	}
	else
	{
		// Select good shard
		ShardSelected = -1;
		for (uint32 i = 0; i < Shards.size(); ++i)
		{
			if (Shards[i].ShardId == LoginShardId)
			{
				ShardSelected = i;
				break;
			}
		}

		if (ShardSelected == -1)
		{
			pIM->messageBoxWithHelp(CI18N::get("uiErrServerLost"), "ui:login");
			LoginSM.pushEvent(CLoginStateMachine::ev_quit);
		}
		else
		{
			LoginSM.pushEvent(CLoginStateMachine::ev_login_ok);
			//		CAHManager::getInstance()->runActionHandler("login_connect_2", NULL);
		}
	}
}


// ***************************************************************************
// Called from client.cpp
bool login()
{
	CLoginProgressPostThread::getInstance().step(CLoginStep(LoginStep_LoginScreen, "login_step_login_screen"));

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CPatchManager *pPM = CPatchManager::getInstance();

	if (LoginLogin.empty())
		loginIntro();

	pIM->initLogin();
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	bool tmpDI = ClientCfg.DisableDirectInput;
	ClientCfg.DisableDirectInput = true;
	InitMouseWithCursor(false);
	Driver->showCursor (false);
	SetMouseFreeLook ();
	SetMouseCursor (false);
	SetMouseSpeed (ClientCfg.CursorSpeed);
	SetMouseAcceleration (ClientCfg.CursorAcceleration);
	InitMouseWithCursor (ClientCfg.HardwareCursor);
	ClientCfg.DisableDirectInput = tmpDI;

//	if (ClientCfg.TestBrowser)
//	{
//		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_WEBSTART);
//
//		// hide 'back to login' button
//		CInterfaceElement *backToLogin = CWidgetManager::getInstance()->getElementFromId(CTRL_BUTTON_BACKTOLOGIN);
//		if (backToLogin)
//			backToLogin->setActive(false);
//
//		// show 'reload test page' button
//		CInterfaceElement *reloadTest = CWidgetManager::getInstance()->getElementFromId(CTRL_BUTTON_RELOADTESTPAGE);
//		if (reloadTest)
//			reloadTest->setActive(true);
//
//		// start the browser
//		CGroupHTML *pGH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(GROUP_BROWSER));
//
//		pGH->browse(ClientCfg.TestBrowserUrl.c_str());
//
//	}
//	else
//	{
////		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKPASS);
////		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DISPLAY_ACCOUNT_BUTTONS")->setValue32(ClientCfg.DisplayAccountButtons);
//	}

	// Active inputs
	Actions.enable(true);
	EditActions.enable(true);

	if(ClientCfg.ConfigFile.exists("pPM->isVerboseLog()"))
		pPM->setVerboseLog(ClientCfg.ConfigFile.getVar("VerboseLog").asInt() == 1);
	if(pPM->isVerboseLog()) nlinfo("Using verbose log mode");

	ClientApp = ClientCfg.ConfigFile.getVar("Application").asString(0);

//	string l = getRegKeyValue("Login").c_str();
//
//	if(!l.empty())
//	{
//		CGroupEditBox *pGEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_LOGIN));
//		if (pGEB != NULL)
//			pGEB->setInputString(l);
//		CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_PASSWORD "|select_all=false");
//	}
//	else
//	{
//		CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_LOGIN "|select_all=false");
//	}

	ShardSelected = -1;
//	CCtrlTextButton *pCB = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId(CTRL_BUTTON_CONNECT));
//	if (pCB != NULL) pCB->setActive(false);
//
//  setLoginFinished( false );
//	loginFinished = false;
//	loginOK = false;

	// Comes from a current patch
//	if (!LoginLogin.empty())
//	{
//		CInterfaceManager *pIM = CInterfaceManager::getInstance();
//		CGroupEditBox *pGEBLog = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_LOGIN));
//		CGroupEditBox *pGEBPwd = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_PASSWORD));
//		pGEBLog->setInputString(LoginLogin);
//		pGEBPwd->setInputString(LoginPassword);
//		CAHManager::getInstance()->runActionHandler("on_login", NULL, "");
//		// Select good shard
//		ShardSelected = -1;
//		for (uint32 i = 0; i < Shards.size(); ++i)
//		{
//			if (Shards[i].ShardId == LoginShardId)
//			{
//				ShardSelected = i;
//				break;
//			}
//		}
//
//		if (ShardSelected == -1)
//			pIM->messageBox(CI18N::get("uiErrServerLost"), "ui:login");
//		else
//			CAHManager::getInstance()->runActionHandler("login_connect_2", NULL);
//	}

	// start the login state machine
	LoginSM.pushEvent(CLoginStateMachine::ev_init_done);

	// run the main loop
	loginMainLoop();

	// Uninit all
	pIM->uninitLogin();

	// Disable inputs
	Actions.enable(false);
	EditActions.enable(false);

	return loginOK;
}

// ***************************************************************************
// INTERFACE HELPERS
// ***************************************************************************

// ***************************************************************************
void removeSpace(string &s)
{
	uint i = 0;
	while (s.size()>0)
	{
		if (s[i] == ' ')
			s.erase(i, 1);
		else
			i++;
		if (i >= s.size()) break;
	}
}

// ***************************************************************************
static void getPatchParameters(std::string &url, std::string &ver, std::vector<std::string> &patchURIs)
{
	if (ClientCfg.R2Mode)
	{
		patchURIs = R2PatchURLs;
		url = R2BackupPatchURL;
		ver = R2ServerVersion;
	}
	else
	{
		nlassert(ShardSelected != -1);
		patchURIs = Shards[ShardSelected].PatchURIs;
		url = Shards[ShardSelected].EmergencyPatchURL;
		ver = Shards[ShardSelected].Version;

		if (!ClientCfg.PatchUrl.empty())
			url = ClientCfg.PatchUrl;

		if (!ClientCfg.PatchVersion.empty())
			ver = ClientCfg.PatchVersion;
	}
}

// ***************************************************************************
std::string getBGDownloaderCommandLine()
{
	#ifdef NL_DEBUG
		CConfigFile::CVar *bgdCommandLine = ClientCfg.ConfigFile.getVarPtr("BackgroundDownloaderCommandLine");
		if (bgdCommandLine != NULL && !bgdCommandLine->asString().empty())
		{
			return bgdCommandLine->asString();
		}
	#endif
	string url;
	string ver;
	std::vector<std::string> patchURIs;
	getPatchParameters(url, ver, patchURIs);
	std::string commandLine = /*R2ServerVersion + " " + VersionName + " " + */ url + " " + ver;
	for (uint i = 0; i < patchURIs.size(); ++i)
	{
		commandLine += " " + patchURIs[i];
	}
	return commandLine;
}

// ***************************************************************************
void initPatchCheck()
{
	// start the patching system
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CPatchManager *pPM = CPatchManager::getInstance();

	string url;
	string ver;
	std::vector<std::string> patchURIs;

	if (!ClientCfg.R2Mode)
	{
		// nb Nico : refactored this code.
		// In previous code, the following was not set in R2Mode, possible bug ?...let as before anyway ...
		// store the selected shard for restarting after patch
		LoginShardId = Shards[ShardSelected].ShardId;
	}

	if (!isBGDownloadEnabled())
	{
		getPatchParameters(url, ver, patchURIs);
		pPM->init(patchURIs, url, ver);
		pPM->startCheckThread(true /* include background patchs */);
	}
	else
	{
		BGDownloader::CTaskDesc taskDesc(BGDownloader::DLState_CheckPatch);
		CBGDownloaderAccess::getInstance().requestDownloadThreadPriority(BGDownloader::ThreadPriority_Normal, false);
		CBGDownloaderAccess::getInstance().startTask(taskDesc, getBGDownloaderCommandLine(), true /* showDownloader */);
	}
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKING);

	setPatcherStateText("ui:login:checking", ucstring());
	setPatcherProgressText("ui:login:checking", ucstring());
}

void initShardDisplay()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_SHARDDISP);

	CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_SHARD));
	if (pList == NULL)
	{
		nlwarning("element "GROUP_LIST_SHARD" not found probably bad login_main.xml");
		return;
	}
	/* // To test more servers
	for (uint fff = 0; fff < 20; ++fff)
	{
		CShard s (	toString("%05d",fff), fff%3, fff+32, toString("%s%d","pipo",fff),
					32*fff%46546, "32.32.32.32", "http://www.ryzomcore.org" );
		Shards.push_back(s);
	}*/

	CInterfaceGroup *pPrevLine = NULL;
	for(uint i = 0; i < Shards.size(); i++)
	{
		vector< pair < string, string > > params;
		params.clear();
		params.push_back(pair<string,string>("id", "s"+toString(i)));
		if (i>0)
			params.push_back(pair<string,string>("posref", "BL TL"));

		CInterfaceGroup *pNewLine = CWidgetManager::getInstance()->getParser()->createGroupInstance("t_shard", GROUP_LIST_SHARD, params);
		if (pNewLine != NULL)
		{
			CViewText *pVT = dynamic_cast<CViewText*>(pNewLine->getView("name"));
			if (pVT != NULL) pVT->setText(Shards[i].Name);

			pVT = dynamic_cast<CViewText*>(pNewLine->getView("version"));
			if (pVT != NULL) pVT->setText(Shards[i].Version);

			CViewBase *pVBon = pNewLine->getView("online");
			CViewBase *pVBoff = pNewLine->getView("offline");
			if ((pVBon != NULL) && (pVBoff != NULL))
			{
				pVBon->setActive (Shards[i].Online);
				pVBoff->setActive (!Shards[i].Online);
			}

			pVT = dynamic_cast<CViewText*>(pNewLine->getView("nbplayer"));
			if (pVT != NULL) pVT->setText(toString(Shards[i].NbPlayers));


			// Add to the list
			pNewLine->setParent(pList);
			pNewLine->setParentSize(pList);
			pNewLine->setParentPos(pPrevLine);
			pList->addGroup(pNewLine);

			pPrevLine = pNewLine;
		}
	}
	// UI Patch
	if (!Shards.empty())
	{
		CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_SHARD ":s0:but"));
		if (pCB != NULL)
			pCB->setPushed(true);
		CAHManager::getInstance()->runActionHandler (pCB->getActionOnLeftClick(), pCB, pCB->getParamsOnLeftClick());

	}
	pList->invalidateCoords();
}

// ***************************************************************************

void onlogin(bool vanishScreen = true)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// Remove space before and after each string login & password
	removeSpace(LoginLogin);
	removeSpace(LoginPassword);

	if(!LoginLogin.empty())
	{
		ClientCfg.LastLogin = LoginLogin;
		ClientCfg.writeString("LastLogin", ClientCfg.LastLogin, true);
	}

	if(vanishScreen)
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(-1);

	// Check the login/pass

	// main menu page for r2mode
	string res = checkLogin(LoginLogin, LoginPassword, ClientApp);
	if (res.empty())
	{
		// if not in auto login, push login ok event
		if (LoginSM.getCurrentState() != CLoginStateMachine::st_auto_login)
			LoginSM.pushEvent(CLoginStateMachine::ev_login_ok);

		return;
		// Ok there is something ! Display next window

		if (ClientCfg.R2Mode)
		{
//			if (ClientCfg.PatchWanted)
//			{
//				// start the patching system
//				CPatchManager *pPM = CPatchManager::getInstance();
//
//				pPM->init(R2PatchURLs, R2BackupPatchURL, R2ServerVersion);
//
//				pPM->startCheckThread();
//				NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKING);
//			}
//			else
//			{
//				// go directly to web browser
//				NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_WEBSTART);
//
//				CInterfaceManager *pIM = CInterfaceManager::getInstance();
//				// start the browser
//				CGroupHTML *pGH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(GROUP_BROWSER));
//
//				pGH->browse(RingMainURL.c_str());
//			}
//			return;
		}
		else
		{
//			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_SHARDDISP);
		}

//		CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_SHARD));
//		if (pList == NULL)
//		{
//			nlwarning("element "GROUP_LIST_SHARD" not found probably bad login_main.xml");
//			return;
//		}
//		/* // To test more servers
//		for (uint fff = 0; fff < 20; ++fff)
//		{
//			CShard s (	toString("%05d",fff), fff%3, fff+32, toString("%s%d","pipo",fff),
//						32*fff%46546, "32.32.32.32", "http://www.ryzomcore.org" );
//			Shards.push_back(s);
//		}*/
//
//		CInterfaceGroup *pPrevLine = NULL;
//		for(uint i = 0; i < Shards.size(); i++)
//		{
//			vector< pair < string, string > > params;
//			params.clear();
//			params.push_back(pair<string,string>("id", "s"+toString(i)));
//			if (i>0)
//				params.push_back(pair<string,string>("posref", "BL TL"));
//
//			CInterfaceGroup *pNewLine =pIM->createGroupInstance("t_shard", GROUP_LIST_SHARD, params);
//			if (pNewLine != NULL)
//			{
//				CViewText *pVT = dynamic_cast<CViewText*>(pNewLine->getView("name"));
//				if (pVT != NULL) pVT->setText(Shards[i].Name);
//
//				pVT = dynamic_cast<CViewText*>(pNewLine->getView("version"));
//				if (pVT != NULL) pVT->setText(Shards[i].Version);
//
//				CViewBase *pVBon = pNewLine->getView("online");
//				CViewBase *pVBoff = pNewLine->getView("offline");
//				if ((pVBon != NULL) && (pVBoff != NULL))
//				{
//					pVBon->setActive (Shards[i].Online);
//					pVBoff->setActive (!Shards[i].Online);
//				}
//
//				pVT = dynamic_cast<CViewText*>(pNewLine->getView("nbplayer"));
//				if (pVT != NULL) pVT->setText(toString(Shards[i].NbPlayers));
//
//
//				// Add to the list
//				pNewLine->setParent(pList);
//				pNewLine->setParentSize(pList);
//				pNewLine->setParentPos(pPrevLine);
//				pList->addGroup(pNewLine);
//
//				pPrevLine = pNewLine;
//			}
//		}
//		// UI Patch
//		if (!Shards.empty())
//		{
//			CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_SHARD ":s0:but"));
//			if (pCB != NULL)
//				pCB->setPushed(true);
//			CAHManager::getInstance()->runActionHandler (pCB->getActionOnLeftClick(), pCB, pCB->getParamsOnLeftClick());
//
//		}
//		pList->invalidateCoords();
	}
	else
	{
		// close the socket in case of error
		HttpClient.disconnect();

		pIM->messageBoxWithHelp("Error : " + res, "ui:login");

		LoginSM.pushEvent(CLoginStateMachine::ev_bad_login);
//		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKPASS);
//
//		if (LoginLogin.empty())
//			CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_LOGIN "|select_all=false");
//		else
//			CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_PASSWORD "|select_all=false");

	}
}

class CAHOnLogin : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		//nlinfo("CAHOnLogin called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CGroupEditBox *pGEBLog = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_LOGIN));
		CGroupEditBox *pGEBPwd = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_PASSWORD));
		if ((pGEBLog == NULL) || (pGEBPwd == NULL))
		{
			nlwarning("element "CTRL_EDITBOX_LOGIN" or "CTRL_EDITBOX_PASSWORD" not found probably bad login_main.xml");
			return;
		}

		LoginLogin = pGEBLog->getInputStringAsStdString();
		LoginPassword = pGEBPwd->getInputStringAsStdString();

		onlogin();
	}
};
REGISTER_ACTION_HANDLER (CAHOnLogin, "on_login");


// ***************************************************************************
class CAHOnGameConfiguration : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHOnGameConfiguration called");

		static string Configurator = "ryzom_configuration_rd.exe";

		if (CFile::fileExists (Configurator))
		{
			// launch the ryzom configurator
			launchProgram(Configurator, "");
			setLoginFinished( true );
			loginOK = false;

			LoginSM.pushEvent(CLoginStateMachine::ev_quit);
		}
		else
		{
			nlwarning("<CAHOnGameConfiguration::execute> can't find ryzom configurator : %s",Configurator.c_str());
		}
	}
};
REGISTER_ACTION_HANDLER (CAHOnGameConfiguration, "on_game_configuration");


// ***************************************************************************
class CAHLoginQuit : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHLoginQuit called");

		setLoginFinished( true );
		loginOK = false;

		LoginSM.pushEvent(CLoginStateMachine::ev_quit);
	}
};
REGISTER_ACTION_HANDLER (CAHLoginQuit, "login_quit");


// ***************************************************************************
class CAHLoginTab : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHLoginTab called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		if (NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->getValue32() == UI_VARIABLES_SCREEN_CHECKPASS)
		{
			CCtrlBase *pCB = CWidgetManager::getInstance()->getCaptureKeyboard();
			if (pCB != NULL)
			{
				CCtrlBase *pNewCB;
				string sID = pCB->getId();
				if (sID == CTRL_EDITBOX_LOGIN)
					pNewCB = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_PASSWORD));
				else
					pNewCB = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_LOGIN));
				CWidgetManager::getInstance()->setCaptureKeyboard(pNewCB);
			}
		}
		else if (NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->getValue32() == UI_VARIABLES_SCREEN_CREATE_ACCOUNT)
		{
			CCtrlBase *pCB = CWidgetManager::getInstance()->getCaptureKeyboard();
			if (pCB != NULL)
			{
				CCtrlBase *pNewCB;
				string sID = pCB->getId();
				if (sID == CTRL_EDITBOX_CREATEACCOUNT_LOGIN)
					pNewCB = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_CREATEACCOUNT_PASSWORD));
				else if (sID == CTRL_EDITBOX_CREATEACCOUNT_PASSWORD)
					pNewCB = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_CREATEACCOUNT_CONFIRMPASSWORD));
				else if (sID == CTRL_EDITBOX_CREATEACCOUNT_CONFIRMPASSWORD)
					pNewCB = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_CREATEACCOUNT_EMAIL));
				else
					pNewCB = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_CREATEACCOUNT_LOGIN));
				CWidgetManager::getInstance()->setCaptureKeyboard(pNewCB);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHLoginTab, "login_tab");


// ***************************************************************************
class CAHShardSelect : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		nlinfo("CAHShardSelect called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CCtrlButton *pCB = NULL;
		// Unselect
		if (ShardSelected != -1)
		{
			pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_SHARD ":s"+toString(ShardSelected)+":but"));
			if (pCB != NULL)
				pCB->setPushed(false);
		}

		pCB = dynamic_cast<CCtrlButton*>(pCaller);
		if (pCB != NULL)
		{
			string name = pCB->getId();
			name = name.substr(0,name.rfind(':'));
			name = name.substr(name.rfind(':')+2,name.size());
			fromString(name, ShardSelected);

			pCB->setPushed(true);
		}

		CCtrlTextButton *pCTB = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId(CTRL_BUTTON_CONNECT));
		if (pCTB != NULL)
			pCTB->setActive(true);
	}
};
REGISTER_ACTION_HANDLER (CAHShardSelect, "shard_select");

// ***************************************************************************
void ConnectToShard()
{
	//nlinfo("ConnectToShard called");

	if (ClientCfg.R2Mode)
	{
		// r2 mode
		setLoginFinished( true );
		loginOK = true;

		LoginSM.pushEvent(CLoginStateMachine::ev_enter_game);
	}
	else
	{
		// legacy mode
		nlassert(ShardSelected != -1);

		string res = selectShard(Shards[ShardSelected].ShardId, Cookie, FSAddr);

		if(res.empty())
		{
			setLoginFinished( true );
			loginOK = true;

			LoginSM.pushEvent(CLoginStateMachine::ev_enter_game);
		}
		else
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			pIM->messageBoxWithHelp("Error :" + res, "ui:login");

			LoginSM.pushEvent(CLoginStateMachine::ev_conn_failed);
		}

	}

}

// ***************************************************************************
class CAHLoginConnect : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHLoginConnect called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		if (ShardSelected == -1)
			return;

		if (!Shards[ShardSelected].Online)
		{
			pIM->messageBoxWithHelp(CI18N::get("uiErrOffLineShard"), "ui:login");
			return;
		}

		LoginSM.pushEvent(CLoginStateMachine::ev_shard_selected);

//		std::vector<std::string>	patchURIs = Shards[ShardSelected].PatchURIs;
//		string url = Shards[ShardSelected].EmergencyPatchURL;
//		string ver = Shards[ShardSelected].Version;
//
//		if (!ClientCfg.PatchUrl.empty())
//			url = ClientCfg.PatchUrl;
//
//		if (!ClientCfg.PatchVersion.empty())
//			ver = ClientCfg.PatchVersion;
//
//		pPM->init(patchURIs, url, ver);
//
//		LoginShardId = Shards[ShardSelected].ShardId;
//
//		if (ClientCfg.PatchWanted)
//		{
//			pPM->startCheckThread();
//			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKING);
//		}
//		else
//		{
//			CAHManager::getInstance()->runActionHandler("login_patch",NULL);
//		}
	}
};
REGISTER_ACTION_HANDLER (CAHLoginConnect, "login_connect");

// ***************************************************************************
// Can be called after a patching process (ryzom relaunch and call this AH to
// see if we have to continue patching or directly go ingame)
class CAHLoginConnect2 : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHLoginConnect2 called");
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		if (Shards[ShardSelected].PatchURIs.empty() && Shards[ShardSelected].EmergencyPatchURL.empty())
		{
			pIM->messageBoxWithHelp(CI18N::get("uiErrCantPatch"), "ui:login");
			return;
		}

		LoginSM.pushEvent(CLoginStateMachine::ev_shard_selected);

//		std::vector<std::string>	patchURIs = Shards[ShardSelected].PatchURIs;
//		string url = Shards[ShardSelected].EmergencyPatchURL;
//		string ver = Shards[ShardSelected].Version;
//
//		if (!ClientCfg.PatchUrl.empty())
//			url = ClientCfg.PatchUrl;
//
//		if (!ClientCfg.PatchVersion.empty())
//			ver = ClientCfg.PatchVersion;
//
//		pPM->init(patchURIs, url, ver);
//
//		if ((ClientCfg.PatchWanted) &&
//			(!Shards[ShardSelected].Version.empty()) &&
//			(Shards[ShardSelected].Version != pPM->getClientVersion()))
//		{
//			pPM->startCheckThread();
//			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKING);
//		}
//		else
//		{
//			// Version is good, eula then connect and launch the client
//			showEULA();
//		}
	}
};
REGISTER_ACTION_HANDLER (CAHLoginConnect2, "login_connect_2");

void initPatch()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CPatchManager *pPM = CPatchManager::getInstance();


	if (!isBGDownloadEnabled())
	{
		// Get the list of optional categories to patch
		vector<string> vCategories;

		CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_CAT));
		if (pList == NULL)
		{
			nlwarning("element "GROUP_LIST_CAT" not found probably bad login_main.xml");
			return;
		}

		for(uint i = 0; i < InfoOnPatch.OptCat.size(); i++)
		{
			// Ok for the moment all optional categories must be patched even if the player
			// does not want it. Because we cant detect that a continent have to be patched ingame.
			vCategories.push_back(InfoOnPatch.OptCat[i].Name);

			/*
			// Code to check if the player wants an optional category or not
			CInterfaceGroup *pLine = pList->getGroup("c"+toString(i));
			if (pLine != NULL)
			{
				CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(pLine->getCtrl("on_off"));
				if (pCB != NULL)
				{
					if (pCB->getPushed())
						vCategories.push_back(rAllCats[i]);
				}
			}
			*/

		}
		pPM->startPatchThread(vCategories, true);
	}
	else
	{
		// NB : here we only do a part of the download each time
		BGDownloader::CTaskDesc taskDesc(BGDownloader::DLState_GetAndApplyPatch, (1 << BGDownloaderWantedPatch));
		CBGDownloaderAccess::getInstance().startTask(taskDesc, string(), true /* showDownloader */); // no command line since bg downloader should already be started
		// release lock on bnp, so that they can be written
		NLMISC::CBigFile::getInstance().removeAll();
	}
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_PATCHING);

	CInterfaceElement *closeBtn = CWidgetManager::getInstance()->getElementFromId(CTRL_BUTTON_CLOSE_PATCH);
	if (closeBtn)
		closeBtn->setActive(false);

	setPatcherStateText("ui:login:patching", ucstring());
	setPatcherProgressText("ui:login:patching", ucstring());
}

// ***************************************************************************
// Called after the check has been done. The page is full of optional categories that must be selected for patching
class CAHLoginPatch : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHLoginPatch called");

		LoginSM.pushEvent(CLoginStateMachine::ev_run_patch);

//		CInterfaceManager *pIM = CInterfaceManager::getInstance();
//		CPatchManager *pPM = CPatchManager::getInstance();
//
//		// Get the list of optional categories to patch
//		vector<string> vCategories;
//
//		CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_CAT));
//		if (pList == NULL)
//		{
//			nlwarning("element "GROUP_LIST_CAT" not found probably bad login_main.xml");
//			return;
//		}
//
//		for(uint i = 0; i < InfoOnPatch.OptCat.size(); i++)
//		{
//			// Ok for the moment all optional categories must be patched even if the player
//			// does not want it. Because we cant detect that a continent have to be patched ingame.
//			vCategories.push_back(InfoOnPatch.OptCat[i].Name);
//
//			/*
//			// Code to check if the player wants an optional category or not
//			CInterfaceGroup *pLine = pList->getGroup("c"+toString(i));
//			if (pLine != NULL)
//			{
//				CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(pLine->getCtrl("on_off"));
//				if (pCB != NULL)
//				{
//					if (pCB->getPushed())
//						vCategories.push_back(rAllCats[i]);
//				}
//			}
//			*/
//		}
//
//		// Start patching
//		if (ClientCfg.PatchWanted)
//		{
//			pPM->startPatchThread(vCategories);
//			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_PATCHING);
//		}
//		else
//		{
//			ConnectToShard();
//		}
	}
};
REGISTER_ACTION_HANDLER (CAHLoginPatch, "login_patch");

// ***************************************************************************
// Called after the check has been done. The page is full of optional categories that must be selected for patching
class CAHClosePatch : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHClosePatch called");

		LoginSM.pushEvent(CLoginStateMachine::ev_close_patch);
	}
};
REGISTER_ACTION_HANDLER (CAHClosePatch, "close_patch");


// ***************************************************************************
// Called after pushing the read note at the opening of the modal window
class CAHSetReleaseNote : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &sParams)
	{
		nlinfo("CAHSetReleaseNote called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		string sShard = getParam(sParams, "shard");
		string sGroupHtml = getParam(sParams, "group");

		CGroupHTML *pQH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(sGroupHtml));
		if (pQH == NULL)
			return;

		string sURL;
		if (ClientCfg.R2Mode)
		{
			// ring release note
			sURL = ClientCfg.RingReleaseNotePath +
				"?version=" + (VersionName.empty() ? R2ServerVersion : VersionName)+
				"&lang=" + ClientCfg.LanguageCode +
				"&ca=" + ClientCfg.ConfigFile.getVar("Application").asString(0);
				"&startPage="+RingMainURL;
		}
		else
		{
			// legacy ryzom release note
			uint32 nShardId;
			if (sShard == "selected")
				nShardId = ShardSelected;
			else
				fromString(sShard.substr(1), nShardId);

			sURL = ClientCfg.ReleaseNotePath +
				"?version=" + Shards[nShardId].Version +
				"&lang=" + ClientCfg.LanguageCode +
				"&id=" + toString(Shards[nShardId].ShardId) +
				"&ca=" + ClientCfg.ConfigFile.getVar("Application").asString(0);

		}

		pQH->browse(sURL.c_str());
	}
};
REGISTER_ACTION_HANDLER (CAHSetReleaseNote, "set_release_note");

// ***************************************************************************
// Called after pushing the read note at the opening of the modal window
class CAHReboot : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		nlinfo("CAHReboot called");

		// create a file to prompt eula next time
		CFile::createEmptyFile(getLogDirectory() + "show_eula");

		CInterfaceManager *im = CInterfaceManager::getInstance();
		try
		{
			if (isBGDownloadEnabled())
			{
				CBGDownloaderAccess::getInstance().reboot();
			}
			else
			{
				CPatchManager::getInstance()->reboot();
			}
			LoginSM.pushEvent(CLoginStateMachine::ev_reboot);
		}
		catch (const NLMISC::EDiskFullError &)
		{
			im->messageBoxWithHelp(CI18N::get("uiPatchDiskFull"), "ui:login");
		}
		catch (const NLMISC::EWriteError &)
		{
			im->messageBoxWithHelp(CI18N::get("uiPatchWriteError"), "ui:login");
		}
		catch (const std::exception &e)
		{
			im->messageBoxWithHelp(ucstring(e.what()), "ui:login", "login_quit");
		}
	}
};
REGISTER_ACTION_HANDLER (CAHReboot, "reboot");



// ***************************************************************************
class CAHAcceptEula : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		//nlinfo("CAHAcceptEula called");
		if(CFile::fileExists(getLogDirectory() + "show_eula"))
			CFile::deleteFile(getLogDirectory() + "show_eula");
		LoginSM.pushEvent(CLoginStateMachine::ev_accept_eula);

//		if (ClientCfg.R2Mode)
//		{
//			// open web browser
//			CInterfaceManager *pIM = CInterfaceManager::getInstance();
//			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_WEBSTART);
//
//			// start the browser
//			CGroupHTML *pGH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(GROUP_BROWSER));
//
//			pGH->browse(RingMainURL.c_str());
//		}
//		else
//		{
//			ConnectToShard();
//		}
	}
};
REGISTER_ACTION_HANDLER (CAHAcceptEula, "accept_eula");

// ***************************************************************************
class CAHOpenURL : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &sParams)
	{
		nlinfo("CAHOpenURL called");

		string url;

		string installTag;

#ifdef NL_OS_WINDOWS

		// Check for special install tag
		const char *KeyName = "InstallTag";

		installTag = CSystemUtils::getRegKey(KeyName);

		if (installTag.length() > 1)
		{
			nldebug("Found install tag '%s'", url.c_str());
		}
		else
		{
			DWORD ret = 0;
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				ret,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL
			);
			// Process any inserts in lpMsgBuf.
			// ...
			// Display the string.
			nlwarning("RegQueryValue for '%s' : %s", KeyName, lpMsgBuf);
			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
#else
		// TODO: for Linux and Mac OS
#endif

		/*
		if (sParams == "cfg_CreateAccountURL")
		{
			url = ClientCfg.CreateAccountURL;

			if (!installTag.empty())
			{
				url += string("/?from=")+installTag;
			}
		}
		else */if (sParams == "cfg_EditAccountURL")
		{
			url = ClientCfg.EditAccountURL;
		}
		else if (sParams == "cfg_BetaAccountURL")
		{
			url = ClientCfg.BetaAccountURL;
		}
		else if (sParams == "cfg_ForgetPwdURL")
		{
			url = ClientCfg.ForgetPwdURL;
		}
		else if (sParams == "cfg_LoginSupportURL")
		{
			url= ClientCfg.LoginSupportURL;
		}
		else if (sParams == "cfg_FreeTrialURL")
		{
			url = ClientCfg.FreeTrialURL;

			if (!installTag.empty())
			{
				url += string("&from=")+installTag;
			}
		}
		else if (sParams == "cfg_ConditionsTermsURL")
		{
			url = ClientCfg.ConditionsTermsURL;
		}
		else
		{
			nlwarning("no URL found");
			return;
		}

		string::size_type pos_lang = url.find("/en/");

		if(pos_lang!=string::npos)
			url.replace(pos_lang+1, 2, ClientCfg.getHtmlLanguageCode());

		if(url.find('?')!=string::npos)
			url += "&";
		else
			url += "?";
		url += "language=" + ClientCfg.LanguageCode;
		openURL(url.c_str());

		nlinfo("openURL %s",url.c_str());
	}
};
REGISTER_ACTION_HANDLER (CAHOpenURL, "open_url");

static vector<UDriver::CMode> VideoModes;
vector<string> StringModeList;
vector<string> StringPresetList;
vector< pair<string, bool> > CfgPresetList;
sint CurrentMode = -1;
sint CurrentPreset = -1;

// ***************************************************************************
class CAHInitResLod : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		//nlinfo("CAHInitResLod called");
		if (Driver == NULL) return;

		VideoModes.clear();
		StringModeList.clear();

		CurrentMode = getRyzomModes(VideoModes, StringModeList);

		// getRyzomModes() expects empty list, so we need to insert 'Windowed' after mode list is filled
		StringModeList.insert(StringModeList.begin(), "uiConfigWindowed");

		// If the client is in windowed mode, still in windowed mode and do not change anything
		if (ClientCfg.Windowed)
			CurrentMode = 0;
		// If we have not found the mode so it can be an error or machine change, so propose the first available
		else if (CurrentMode == -1)
			CurrentMode = 1;
		// We inserted 'Windowed' as first mode, so index needs to move too
		else
			++CurrentMode;

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:checkpass:content:res_value"));
		if (pVT != NULL)
			pVT->setHardText(StringModeList[CurrentMode]);

		StringPresetList.clear();
		StringPresetList.push_back("uiLodValueLow");
		StringPresetList.push_back("uiLodValueMedium");
		StringPresetList.push_back("uiLodValueNormal");
		StringPresetList.push_back("uiLodValueHigh");
		StringPresetList.push_back("uiLodValueCustom");
		CurrentPreset = 4; // CInterfaceDDX::CustomPreset

		// first indicates the preset-able cfg-variable
		// second indicates if its a double variable (else it's an int)
		CfgPresetList.clear();
		CfgPresetList.push_back(pair<string,bool>("LandscapeTileNear",		true));
		CfgPresetList.push_back(pair<string,bool>("LandscapeThreshold",		true));
		CfgPresetList.push_back(pair<string,bool>("Vision",					true));
		CfgPresetList.push_back(pair<string,bool>("MicroVeget",				false));
		CfgPresetList.push_back(pair<string,bool>("MicroVegetDensity",		true));
		CfgPresetList.push_back(pair<string,bool>("FxNbMaxPoly",			false));
		CfgPresetList.push_back(pair<string,bool>("Cloud",					false));
		CfgPresetList.push_back(pair<string,bool>("CloudQuality",			true));
		CfgPresetList.push_back(pair<string,bool>("CloudUpdate",			false));
		CfgPresetList.push_back(pair<string,bool>("Shadows",				false));
		CfgPresetList.push_back(pair<string,bool>("SkinNbMaxPoly",			false));
		CfgPresetList.push_back(pair<string,bool>("NbMaxSkeletonNotCLod",	false));
		CfgPresetList.push_back(pair<string,bool>("CharacterFarClip",		true));

		CfgPresetList.push_back(pair<string,bool>("Bloom",					false));
		CfgPresetList.push_back(pair<string,bool>("SquareBloom",			false));
		CfgPresetList.push_back(pair<string,bool>("DensityBloom",			true));

		// Check if all the preset-able cfg-variable are in a preset mode
		sint nPreset = -1;
		for (uint32 i = 0; i < CfgPresetList.size(); ++i)
		{
			CConfigFile::CVar *cfgVarPtr = ClientCfg.ConfigFile.getVarPtr(CfgPresetList[i].first);
			if (cfgVarPtr == NULL) continue;
			// Get the preset of the variable i
			sint nVarPreset = 0;
			for (uint32 j = 0; j < 4; ++j) // CInterfaceDDX::NumPreset
			{
				string sPresetName = CfgPresetList[i].first + "_ps" + toString(j);
				CConfigFile::CVar *presetVarPtr = ClientCfg.ConfigFile.getVarPtr(sPresetName);
				if(presetVarPtr)
				{
					if (CfgPresetList[i].second) // Is it a double ?
					{
						if (cfgVarPtr->asDouble() == presetVarPtr->asDouble())
							nVarPreset |= (1 << j);
					}
					else
					{
						if (cfgVarPtr->asInt() == presetVarPtr->asInt())
							nVarPreset |= (1 << j);
					}
				}
			}

			if (nPreset == -1)
				nPreset = nVarPreset;
			else
				nPreset &= nVarPreset;

			if (nPreset == 0)
				break;
		}

		if (nPreset != 0)
		{
			if (nPreset&1)		CurrentPreset = 0;
			else if (nPreset&2)	CurrentPreset = 1;
			else if (nPreset&4)	CurrentPreset = 2;
			else if (nPreset&8)	CurrentPreset = 3;
		}

		pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:checkpass:content:lod_value"));
		if (pVT != NULL)
			pVT->setHardText(StringPresetList[CurrentPreset]);
	}
};
REGISTER_ACTION_HANDLER (CAHInitResLod, "init_res_lod");

// ***************************************************************************
class CAHMoreRes : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		nlinfo("CAHMoreRes called");
		if (CurrentMode < ((sint)StringModeList.size()-1))
			CurrentMode++;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:checkpass:content:res_value"));
		if (pVT != NULL)
			pVT->setHardText(StringModeList[CurrentMode]);
	}
};
REGISTER_ACTION_HANDLER (CAHMoreRes, "more_res");

// ***************************************************************************
class CAHLessRes : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		nlinfo("CAHLessRes called");
		if (CurrentMode > 0)
			CurrentMode--;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:checkpass:content:res_value"));
		if (pVT != NULL)
			pVT->setHardText(StringModeList[CurrentMode]);
	}
};
REGISTER_ACTION_HANDLER (CAHLessRes, "less_res");

// ***************************************************************************
class CAHMoreLod : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		nlinfo("CAHMoreLod called");
		if (CurrentPreset < ((sint)StringPresetList.size()-1))
			CurrentPreset++;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:checkpass:content:lod_value"));
		if (pVT != NULL)
			pVT->setHardText(StringPresetList[CurrentPreset]);
	}
};
REGISTER_ACTION_HANDLER (CAHMoreLod, "more_lod");

// ***************************************************************************
class CAHLessLod : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		nlinfo("CAHMoreLod called");
		if (CurrentPreset > 0)
			CurrentPreset--;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:login:checkpass:content:lod_value"));
		if (pVT != NULL)
			pVT->setHardText(StringPresetList[CurrentPreset]);
	}
};
REGISTER_ACTION_HANDLER (CAHLessLod, "less_lod");

// ***************************************************************************
class CAHUninitResLod : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* sParams */)
	{
		//nlinfo("CAHUninitResLod called");

		// If the mode requested is a windowed mode do nothnig
		if (CurrentMode == 0)
		{
			ClientCfg.Windowed = true;
			ClientCfg.writeBool("FullScreen", false);
		}
		else
		{
			ClientCfg.Windowed = false;
			// Get W, H
			uint16 w = 0, h = 0;
			{
				string vidModeStr = StringModeList[CurrentMode];
				string tmp = vidModeStr.substr(0,vidModeStr.find('x')-1);
				fromString(tmp, w);
				tmp = vidModeStr.substr(vidModeStr.find('x')+2,vidModeStr.size());
				fromString(tmp, h);
			}
			ClientCfg.Width = w;
			ClientCfg.Height = h;

			ClientCfg.writeBool("FullScreen", true);
			ClientCfg.writeInt("Width", w);
			ClientCfg.writeInt("Height", h);
		}

		if (CurrentPreset != 4) // CInterfaceDDX::CustomPreset
		{
			for (uint32 i = 0; i < CfgPresetList.size(); ++i)
			{
				CConfigFile::CVar *cfgVarPtr = ClientCfg.ConfigFile.getVarPtr(CfgPresetList[i].first);
				if (cfgVarPtr == NULL) continue;

				string sPresetName = CfgPresetList[i].first + "_ps" + toString(CurrentPreset);
				CConfigFile::CVar *presetVarPtr = ClientCfg.ConfigFile.getVarPtr(sPresetName);
				if(presetVarPtr)
				{
					if (CfgPresetList[i].second) // Is it a double ?
						cfgVarPtr->setAsDouble(presetVarPtr->asDouble());
					else
						cfgVarPtr->setAsInt(presetVarPtr->asInt());
				}
			}
		}

		// **** Save the config
		if (ClientCfg.SaveConfig)
			ClientCfg.ConfigFile.save ();
		ClientCfg.IsInvalidated = true;
	}
};
REGISTER_ACTION_HANDLER (CAHUninitResLod, "uninit_res_lod");


void initDataScan()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CPatchManager *pPM = CPatchManager::getInstance();

	// reset the log
	setDataScanLog(ucstring());

	// Start Scanning
	pPM->startScanDataThread();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_DATASCAN);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DATASCAN_RUNNING")->setValue32(1);

}

// ***************************************************************************
// Called after the check has been done. The page is full of optional categories that must be selected for patching
class CAHOnScanDataStart : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHOnScanDataStart called");

		LoginSM.pushEvent(CLoginStateMachine::ev_data_scan);

	}
};
REGISTER_ACTION_HANDLER (CAHOnScanDataStart, "on_scan_data_start");

// ***************************************************************************
// Called when the user cancel the scan
class CAHOnScanDataClose : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHOnScanDataClose called");
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CPatchManager *pPM = CPatchManager::getInstance();

		// if the scan is still running
		if(NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DATASCAN_RUNNING")->getValue32()!=0)
		{
			// request to stop the thread
			pPM->askForStopScanDataThread();

			// Log.
			setDataScanState(CI18N::get("uiCancelingScanData"));
		}
		else
		{
			LoginSM.pushEvent(CLoginStateMachine::ev_close_data_scan);
			// Come Back to Login Screen.
//			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKPASS);
//
//			// Give focus to password if some login entered
//			string	loginEB;
//			CGroupEditBox *pGEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_EDITBOX_LOGIN));
//			if(pGEB)
//				loginEB= pGEB->getInputStringAsStdString();
//			// if none entered
//			if (loginEB.empty())
//				CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_LOGIN "|select_all=false");
//			else
//				CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_PASSWORD "|select_all=false");
		}
	}
};
REGISTER_ACTION_HANDLER (CAHOnScanDataClose, "on_scan_data_close");

// ***************************************************************************

inline string parseTooltip(const string & initString, const string & tagName)
{
	string tooltip;

	string::size_type tooltipPos = initString.find(tagName);
	if(tooltipPos != string::npos)
	{
		tooltip = initString.substr(tooltipPos);

		// start of tooltip
		tooltip = tooltip.substr(tooltip.find(">")+1);

		// end of tooltip
		tooltip = tooltip.substr(0, tooltip.find("<"));
	}

	ucstring uc;
	uc.fromUtf8(tooltip);;
	tooltip = uc.toString();

	return tooltip;
}

inline string parseCommentError(const string & initString, const string & tagName)
{
	string error;

	string::size_type errorPos = initString.find(tagName);
	if(errorPos != string::npos)
	{
		error = initString.substr(errorPos);

		// start of comment
		error = error.substr(error.find(">")+1);

		// end of comment
		error = error.substr(0, error.find("<"));
	}

	ucstring uc;
	uc.fromUtf8(error);;
	error = uc.toString();

	return error;
}

bool initCreateAccount()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// reset UI
	CInterfaceGroup *createAccountUI = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:login:create_account"));
	if(createAccountUI)
	{
		// show "submit interface", hide "login interface"
		CInterfaceGroup * grSubmit = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("submit_gr"));
		if(grSubmit)
			grSubmit->setActive(true);

		CInterfaceGroup * grLogin = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("login_gr"));
		if(grLogin)
			grLogin->setActive(false);

		// empty edit boxes
		std::vector< std::string > editBoxes(4);
		editBoxes[0] = "eb_login";
		editBoxes[1] = "eb_password";
		editBoxes[2] = "eb_confirm_password";
		editBoxes[3] = "eb_email";

		for(uint i=0; i<editBoxes.size(); i++)
		{
			CGroupEditBox * eb = dynamic_cast<CGroupEditBox*>(createAccountUI->findFromShortId(editBoxes[i] + ":eb"));
			if(eb)
				eb->setInputString(ucstring(""));
		}

		// conditions button
		CCtrlBaseButton * but = dynamic_cast<CCtrlBaseButton*>(createAccountUI->findFromShortId("accept_cond"));
		if(but)
			but->setPushed(true);

		// get rules from url
		string url = ClientCfg.CreateAccountURL;
		CPatchManager *pPM = CPatchManager::getInstance();

		if (!CurlHttpClient.connect(url))
		{
			nlwarning("Can't connect");
			return false;
		}

		std::string lang = ClientCfg.LanguageCode;
		if(lang=="wk") lang = "uk";

		CurlHttpClient.verifyServer(true); // set this to false if you need to connect to the test environment

		std::string params = "language=" + lang;
		if(!CurlHttpClient.sendGet(url, params, pPM->isVerboseLog()))
		{
			ucstring errorMessage("Can't send (error code 60)");
			errorMessageBox(errorMessage);
			nlwarning(errorMessage.toString().c_str());
			return false;
		}

		string res;
		if(!CurlHttpClient.receive(res, pPM->isVerboseLog()))
		{
			ucstring errorMessage("Can't receive (error code 61)");
			errorMessageBox(errorMessage);
			nlwarning(errorMessage.toString().c_str());
			return false;
		}

		if(res.empty())
		{
			ucstring errorMessage("Empty result (error code 13)");
			errorMessageBox(errorMessage);
			nlwarning(errorMessage.toString().c_str());
			return false;
		}

		CurlHttpClient.disconnect();

		// initialize rules in interface
		std::vector< std::pair< std::string , std::string > > rules(5);
		rules[0] = std::pair<std::string , std::string>("rules_login", "id=tooltip-Username");
		rules[1] = std::pair<std::string , std::string>("rules_password", "id=tooltip-Password");
		rules[2] = std::pair<std::string , std::string>("rules_password_conf", "id=tooltip-ConfirmPass");
		rules[3] = std::pair<std::string , std::string>("rules_email", "id=tooltip-Email");
		rules[4] = std::pair<std::string , std::string>("rules_conditions", "id=tooltip-TaC");

		for(uint i=0; i<rules.size(); i++)
		{
			CViewText * text = dynamic_cast<CViewText*>(createAccountUI->findFromShortId(rules[i].first));
			if(text)
			{
				string tooltip = parseTooltip(res, rules[i].second);
				text->setHardText(tooltip);
				text->setActive(false);
			}
		}

		// welcome text
		CViewText * text = dynamic_cast<CViewText*>(createAccountUI->findFromShortId("errors_list"));
		if(text)
		{
			text->setHardText(toString(CI18N::get("uiCreateAccountWelcome")));
			text->setColor(CRGBA(255, 255, 255, 255));

			CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("erros_txt"));
			if(group)
			{
				group->updateCoords();

				CInterfaceGroup * groupScroll = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("err_back_scrollbar"));
				if(groupScroll) groupScroll->setActive(group->getHReal() > group->getMaxHReal());
				CCtrlScroll * scroll = dynamic_cast<CCtrlScroll*>(createAccountUI->findFromShortId("err_scroll_bar"));
				if(scroll)
					scroll->setTrackPos(scroll->getHReal());
			}
		}

		// hide rules
		CInterfaceGroup * rulesGr = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("rules_gr"));
		if(rulesGr)
			rulesGr->setActive(false);

		// must be done after hide rules
		CAHManager::getInstance()->runActionHandler("set_keyboard_focus", NULL, "target=" CTRL_EDITBOX_CREATEACCOUNT_LOGIN "|select_all=false"); 
	}


	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CREATE_ACCOUNT);

	return true;
}

// ***************************************************************************
// Called when the user focus one of the edit boxes during the account creation
class CAHCreateAccountRules : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		nlinfo("CAHCreateAccountRules called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup *createAccountUI = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:login:create_account"));
		if(createAccountUI)
		{
			CInterfaceGroup * rulesGr = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("rules_gr"));
			if(rulesGr)
				rulesGr->setActive(false);

			std::vector< std::string > rules(4);
			rules[0] = "rules_login";
			rules[1] = "rules_password";
			rules[2] = "rules_password_conf";
			rules[3] = "rules_email";

			for(uint i=0; i<rules.size(); i++)
			{
				CViewText * text = dynamic_cast<CViewText*>(createAccountUI->findFromShortId(rules[i]));
				if(text)
				{
					text->setActive(Params==rules[i]);
					if(Params==rules[i])
					{
						if(rulesGr)
							rulesGr->setActive(text->getText() != ucstring(""));
					}
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHCreateAccountRules, "create_account_rules");

// ***************************************************************************
// Called when the user choose the account creation
class CAHOnCreateAccount : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHOnCreateAccount called");

		LoginSM.pushEvent(CLoginStateMachine::ev_create_account);
	}
};
REGISTER_ACTION_HANDLER (CAHOnCreateAccount, "on_create_account");

// ***************************************************************************
// Called when the user submit the account creation
class CAHOnCreateAccountSubmit : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHOnCreateAccountSubmit called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CInterfaceGroup *createAccountUI = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:login:create_account"));
		if(createAccountUI)
		{
			// recover data from UI
			std::vector< std::string > editBoxes(4);
			editBoxes[0] = "eb_login";
			editBoxes[1] = "eb_password";
			editBoxes[2] = "eb_confirm_password";
			editBoxes[3] = "eb_email";
			std::vector< std::string > results(4);

			for(uint i=0; i<editBoxes.size(); i++)
			{
				CGroupEditBox * eb = dynamic_cast<CGroupEditBox*>(createAccountUI->findFromShortId(editBoxes[i] + ":eb"));
				if(eb)
					results[i] = eb->getInputString().toUtf8();
			}

			// text
			CViewText * text = dynamic_cast<CViewText*>(createAccountUI->findFromShortId("email_adress"));
			if(text)
				text->setHardText(results[3]);

			// conditions button
			bool conditionsPushed = false;
			CCtrlBaseButton * but = dynamic_cast<CCtrlBaseButton*>(createAccountUI->findFromShortId("accept_cond"));
			if(but)
				conditionsPushed = !but->getPushed();

			string url = ClientCfg.CreateAccountURL;
			CPatchManager *pPM = CPatchManager::getInstance();

			if (!CurlHttpClient.connect(url))
			{
				ucstring errorMessage("Can't connect");
				errorMessageBox(errorMessage);
				nlwarning(errorMessage.toString().c_str());
				return;
			}

			std::string params = "Username=" + results[0] + "&Password=" + results[1]
				+ "&ConfirmPass=" + results[2] + "&Email=" + results[3];

			if(conditionsPushed)
				params += "&TaC=1";

			std::string md5 = results[0] + results[1] + "" + results[3];
			md5 = NLMISC::getMD5((uint8*)md5.data(), (uint32)md5.size()).toString();

			params += "&SC=" + md5;
			std::string lang = ClientCfg.LanguageCode;
			if(lang=="wk") lang = "uk";
			params += "&Language=" + lang;

			CurlHttpClient.verifyServer(true); // set this to false if you need to connect to the test environment

			if(!CurlHttpClient.sendPost(url, params, pPM->isVerboseLog()))
			{
				ucstring errorMessage("Can't send (error code 60)");
				errorMessageBox(errorMessage);
				nlwarning(errorMessage.toString().c_str());
				return;
			}

			string res;
			if(!CurlHttpClient.receive(res, pPM->isVerboseLog()))
			{
				ucstring errorMessage("Can't receive (error code 61)");
				errorMessageBox(errorMessage);
				nlwarning(errorMessage.toString().c_str());
				return;
			}

			if(res.empty())
			{
				ucstring errorMessage("Empty result (error code 13)");
				errorMessageBox(errorMessage);
				nlwarning(errorMessage.toString().c_str());
				return;
			}

			CurlHttpClient.disconnect();

			// parse html
			string::size_type okPos = res.find("email_sent");
			if(okPos != string::npos)
			{
				// show "submit interface", hide "login interface"
				CInterfaceGroup * grSubmit = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("submit_gr"));
				if(grSubmit)
					grSubmit->setActive(false);

				CInterfaceGroup * grLogin = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("login_gr"));
				if(grLogin)
					grLogin->setActive(true);
			}
			else
			{
				// initialize error comments in interface
				CViewText * text = dynamic_cast<CViewText*>(createAccountUI->findFromShortId("errors_list"));
				if(text)
				{
					text->setColor(CRGBA(250, 30, 30, 255));

					std::vector< std::string > errors(5);
					errors[0] = "id=\"comment-Username\"";
					errors[1] = "id=\"comment-Password\"";
					errors[2] = "id=\"comment-ConfirmPass\"";
					errors[3] = "id=\"comment-Email\"";
					errors[4] = "id=\"comment-TaC\"";

					string error;
					for(uint i=0; i<errors.size(); i++)
					{
						string comment = parseCommentError(res, errors[i]);
						if(comment!="")
							error += "- " + comment + "\n";
					}

					text->setHardText(error);

					CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("erros_txt"));
					if(group)
					{
						group->updateCoords();

						CInterfaceGroup * groupScroll = dynamic_cast<CInterfaceGroup*>(createAccountUI->findFromShortId("err_back_scrollbar"));
						if(groupScroll) groupScroll->setActive(group->getHReal() > group->getMaxHReal());
						CCtrlScroll * scroll = dynamic_cast<CCtrlScroll*>(createAccountUI->findFromShortId("err_scroll_bar"));
						if(scroll)
							scroll->setTrackPos(scroll->getHReal());
					}
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHOnCreateAccountSubmit, "on_create_account_submit");

// ***************************************************************************
// Called when the user cancel the account creation
class CAHOnCreateAccountClose : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHOnCreateAccountClose called");

		LoginSM.pushEvent(CLoginStateMachine::ev_close_create_account);
	}
};
REGISTER_ACTION_HANDLER (CAHOnCreateAccountClose, "on_create_account_close");

// ***************************************************************************
class CAHCreateAccountLogin : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHCreateAccountLogin called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CInterfaceGroup *createAccountUI = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:login:create_account"));
		if(createAccountUI)
		{
			CGroupEditBox * eb = dynamic_cast<CGroupEditBox*>(createAccountUI->findFromShortId("eb_login:eb"));
			if(eb)
				LoginLogin = eb->getInputString().toUtf8();

			eb = dynamic_cast<CGroupEditBox*>(createAccountUI->findFromShortId("eb_password:eb"));
			if(eb)
				LoginPassword = eb->getInputString().toUtf8();

			onlogin(false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHCreateAccountLogin, "create_account_login");

// ***************************************************************************
// Called by html embeded lua script
class CAHOnConnectToShard: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// warning : pCaller is null when event come from lua scrip embeded in HTML
		Cookie = getParam(Params, "cookie");
		FSAddr = getParam(Params, "fsAddr");

		// replace the '_' with '|' in the cookie string
		for (uint i=0; i<Cookie.size(); ++i)
		{
			if (Cookie[i] == '_')
				Cookie[i] = '|';
		}

		setLoginFinished( true );
		loginOK = true;

		LoginSM.pushEvent(CLoginStateMachine::ev_connect);
	}
};
REGISTER_ACTION_HANDLER (CAHOnConnectToShard, "on_connect_to_shard");

// ***************************************************************************
// Called to return to login screen in case of error
class CAHOnBackToLogin: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		setLoginFinished( false );
		loginOK = false;
		LoginSM.pushEvent(CLoginStateMachine::ev_relog);

//		CInterfaceManager *pIM = CInterfaceManager::getInstance();
//		// need to reset password and current screen
//		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SCREEN")->setValue32(UI_VARIABLES_SCREEN_CHECKPASS);
	}
};
REGISTER_ACTION_HANDLER (CAHOnBackToLogin, "on_back_to_login");

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// NETWORK CONNECTION
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
string checkLogin(const string &login, const string &password, const string &clientApp)
{
	CPatchManager *pPM = CPatchManager::getInstance();
	Shards.clear();

	if(ClientCfg.ConfigFile.exists("VerboseLog"))
		pPM->setVerboseLog(ClientCfg.ConfigFile.getVar("VerboseLog").asInt() == 1);
	if(pPM->isVerboseLog()) nlinfo("Using verbose log mode");

	if(!HttpClient.connectToLogin())
		return "Can't connect (error code 1)";

	if(pPM->isVerboseLog()) nlinfo("Connected");

	string res;

	// ask server for salt
	if(!HttpClient.sendGet(ClientCfg.ConfigFile.getVar("StartupPage").asString()+"?cmd=ask&login="+login+"&lg="+ClientCfg.LanguageCode, "", pPM->isVerboseLog()))
		return "Can't send (error code 60)";

	if(pPM->isVerboseLog()) nlinfo("Sent request for password salt");

	if(!HttpClient.receive(res, pPM->isVerboseLog()))
		return "Can't receive (error code 61)";

	if(pPM->isVerboseLog()) nlinfo("Received request login check");

	if(res.empty())
		return "Empty answer from server (error code 62)";

	if(res[0] == '0')
	{
		// server returns an error
		nlwarning("server error: %s", res.substr(2).c_str());
		return res.substr(2);
	}
	else if(res[0] == '1')
	{
		Salt = res.substr(2);
	}
	else
	{
		// server returns ???
		nlwarning("%s", res.c_str());
		return res;
	}

	// send login + crypted password + client app and cp=1 (as crypted password)
	if(!HttpClient.connectToLogin())
		return "Can't connect (error code 63)";

	if(pPM->isVerboseLog()) nlinfo("Connected");

	if (ClientCfg.R2Mode)
	{
		// R2 login sequence
		std::string	cryptedPassword = CCrypt::crypt(password, Salt);
		if(!HttpClient.sendGet(ClientCfg.ConfigFile.getVar("StartupPage").asString()+"?cmd=login&login="+login+"&password="+cryptedPassword+"&clientApplication="+clientApp+"&cp=1"+"&lg="+ClientCfg.LanguageCode))
			return "Can't send (error code 2)";

		// the response should contains the result code and the cookie value
		if(pPM->isVerboseLog()) nlinfo("Sent request login check");

		if(!HttpClient.receive(res, pPM->isVerboseLog()))
			return "Can't receive (error code 3)";

		if(pPM->isVerboseLog()) nlinfo("Received request login check");

		if(res.empty())
			return "Empty answer from server (error code 4)";

		if(res[0] == '0')
		{
			// server returns an error
			nlwarning("server error: %s", res.substr(2).c_str());
			return res.substr(2);
		}
		else if(res[0] == '1')
		{
			//nlwarning(res.c_str());
			vector<string>	lines;
			explode(res, std::string("\n"), lines, false);
			if (lines.size() != 2)
			{
				return toString("Invalid server return, found %u lines, want 2", lines.size());
			}

			vector<string>	parts;
			explode(lines[0], std::string("#"), parts, false);
			if (parts.size() < 5)
				return "Invalid server return, missing cookie and/or Ring URLs";

			// server returns ok, we have the cookie

			// store the cookie value and FS address for next page request
			CurrentCookie = parts[1];
			Cookie = CurrentCookie;
			FSAddr = parts[2];

			// store the ring startup page
			RingMainURL = parts[3];
			FarTP.setURLBase(parts[4]);

			if(parts.size() >= 6 && parts[5] == "1")
			{
				// we want stats
				extern bool startStat;
				startStat = true;
			}

			// parse the second line (contains the domain info)
			parts.clear();
			explode(lines[1], std::string("#"), parts, false);
			if (parts.size() < 3)
				return "Invalid server return, missing patch URLs";

			R2ServerVersion = parts[0].c_str();
			R2BackupPatchURL = parts[1];
			explode(parts[2], std::string(" "), R2PatchURLs, true);
		}
		else
		{
			// unexpected content
#if !FINAL_VERSION
			string ret = toString("DEV : Invalid server return, missing return code in \n%s", res.c_str());
			return ret;
#else
			return "Invalid server return, missing return code";
#endif
		}

	}
	else
	{
		// standard ryzom login sequence
		std::string	cryptedPassword = CCrypt::crypt(password, Salt);
		if(!HttpClient.sendGet(ClientCfg.ConfigFile.getVar("StartupPage").asString()+"?login="+login+"&password="+cryptedPassword+"&clientApplication="+clientApp+"&cp=1"))
			return "Can't send (error code 2)";
	/*
		if(!send(ClientCfg.ConfigFile.getVar("StartupPage").asString()+"?login="+login+"&password="+password+"&clientApplication="+clientApp))
			return "Can't send (error code 2)";
	*/
		if(pPM->isVerboseLog()) nlinfo("Sent request login check");

		if(!HttpClient.receive(res, pPM->isVerboseLog()))
			return "Can't receive (error code 3)";

		if(pPM->isVerboseLog()) nlinfo("Received request login check");

		if(res.empty())
			return "Empty answer from server (error code 4)";

		if(res[0] == '0')
		{
			// server returns an error
			nlwarning("server error: %s", res.substr(2).c_str());
			return res.substr(2);
		}
		else if(res[0] == '1')
		{
			// server returns ok, we have the list of shard
			uint nbs;
			fromString(res.substr(2), nbs);
			vector<string> lines;

			explode(res, std::string("\n"), lines, true);

			if(pPM->isVerboseLog())
			{
				nlinfo ("Exploded, with nl, %u res", (uint)lines.size());
	/*			for (uint i = 0; i < lines.size(); i++)
				{
					nlinfo (" > '%s'", lines[i].c_str());
				}*/
			}

			if(lines.size() != nbs+1)
			{
				nlwarning("bad shard lines number %u != %d", (uint)lines.size(), nbs+1);
				nlwarning("'%s'", res.c_str());
				return "bad lines numbers (error code 5)";
			}

			for(uint i = 1; i < lines.size(); i++)
			{
				vector<string> res;
				explode(lines[i], std::string("|"), res);

				if(pPM->isVerboseLog())
				{
					nlinfo ("Exploded with '%s', %u res", "|", (uint)res.size());
	/*				for (uint i = 0; i < res.size(); i++)
					{
						nlinfo (" > '%s'", res[i].c_str());
					}*/
				}

				if (res.size() < 7 && res.size() > 8)
				{
					nlwarning("bad | numbers %u != %d", (uint)res.size(), 8);
					nlwarning("'%s'", lines[i].c_str());
					return "bad pipe numbers (error code 6)";
				}
				bool online;
				fromString(res[1], online);
				uint32 shardId;
				fromString(res[2], shardId);
				uint32 nbPlayers;
				fromString(res[4], nbPlayers);
				Shards.push_back(CShard(res[0], online, shardId, res[3], nbPlayers, res[5], res[6]));
				nlinfo("Shard %u, addr = %s, id = %u, name = %s, PatchURIs = %s", i, res[5].c_str(), shardId, res[3].c_str(), res[6].c_str());
				if (res.size() == 8)
				{
					explode(res[7], std::string(" "), Shards.back().PatchURIs);
				}
			}
		}
		else
		{
			// server returns ???
			nlwarning("%s", res.c_str());
			return res;
		}
	}

	return "";
}

// ***************************************************************************
string selectShard(uint32 shardId, string &cookie, string &addr)
{
	cookie = addr = "";

	if(!HttpClient.connectToLogin()) return "Can't connect (error code 7)";

	if(LoginLogin.empty()) return "Empty Login (error code 8)";
	if(LoginPassword.empty()) return "Empty Password (error code 9)";
	if(ClientApp.empty()) return "Empty Client Application (error code 10)";

	// send login + crypted password + client app and cp=1 (as crypted password)
	std::string	cryptedPassword = CCrypt::crypt(LoginPassword, Salt);
	if(!HttpClient.sendGet(ClientCfg.ConfigFile.getVar("StartupPage").asString()+"?cmd=login&shardid="+toString(shardId)+"&login="+LoginLogin+"&password="+cryptedPassword+"&clientApplication="+ClientApp+"&cp=1"))
		return "Can't send (error code 11)";

	string res;

	CPatchManager *pPM = CPatchManager::getInstance();
	if(!HttpClient.receive(res, pPM->isVerboseLog()))
		return "Can't receive (error code 12)";

	if(res.empty())
		return "Empty result (error code 13)";

	if(res[0] == '0')
	{
		// server returns an error
		nlwarning("server error: %s", res.substr(2).c_str());
		return res.substr(2);
	}
	else if(res[0] == '1')
	{
		// server returns ok, we have the access

		vector<string> line;
		explode(res, std::string(" "), line, true);

		if (line.size() < 2 || line.size() > 3)
		{
			nlwarning("bad launch lines number %d != %d", line.size(), 2);
			return "bad launch line number (error code 14)";
		}

		cookie = line[0].substr(2);
		addr = line[1];

		std::vector<std::string>	patchURIs;

		CShard*	shard = NULL;
		uint	i;
		for (i=0; i<Shards.size(); ++i)
		{
			if (Shards[i].ShardId == shardId)
			{
				shard = &Shards[i];
				break;
			}
		}
/*
		if (shard != NULL && line.size() >= 3)
		{
			explode(line[2], "|", shard->PatchURIs, true);

			nlinfo("received %d main patch server URIs:", shard->PatchURIs.size());
			uint	i;
			for (i=0; i<shard->PatchURIs.size(); ++i)
				nlinfo("%d: '%s'", i, shard->PatchURIs[i].c_str());
		}
*/
	}
	else
	{
		// server returns ???
		nlwarning("%s", res.c_str());
		return res;
	}

	return "";
}


/*void mainLandPatch()
{
	if (!AvailablePatchs) return;
	nlassert(AvailablePatchs & (1 << BGDownloader::DownloadID_MainLand)); // only handled case for now
	// no-op
	BGDownloaderWantedPatch = BGDownloader::DownloadID_MainLand;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->initLogin();
	// login machine should be in the 'end' state !!
	nlassert(LoginSM.getCurrentState() == CLoginStateMachine::st_end);
	LoginSM.pushEvent(CLoginStateMachine::ev_mainland_patch);
	loginMainLoop(); // patch is handled in the login mainloop
	// there should have been a reboot there, so quit if something went wrong...
	release();
	exit(0);
}
*/



// ***************************************************************************
// ***************************************************************************
// INTRO HANDLING
// ***************************************************************************
// ***************************************************************************

#include "init_main_loop.h"

bool loginIntroSkip;

void loginIntro()
{
	// Display of nevrax logo is done at init time (see init.cpp) just before addSearchPath (second one)
	for (uint i = 0; i < 1; i++) // previously display nevrax then nvidia
	{
		if (i != 0)
		{
			beginLoading(IntroNVidia);
			ucstring nmsg("");
			ProgressBar.newMessage (nmsg);
		}

		Driver->AsyncListener.reset();
		updateClientTime();
		CInputHandlerManager::getInstance()->pumpEventsNoIM();
		updateClientTime();

		loginIntroSkip = false;

		sint64 CurTime = T0;

		while (loginIntroSkip == false)
		{
			updateClientTime();
			if ((T0 - CurTime) > 5000) // 5s before quiting
				break;
			// Update messages
			CInputHandlerManager::getInstance()->pumpEventsNoIM();

			// Exit ?
			if (Driver->AsyncListener.isKeyPushed (KeyESCAPE) || Driver->AsyncListener.isKeyPushed (KeyRETURN) ||
				Driver->AsyncListener.isKeyPushed (KeySPACE))
				break;

			const ucstring nmsg("");
			ProgressBar.newMessage (nmsg);
			IngameDbMngr.flushObserverCalls();
			NLGUI::CDBManager::getInstance()->flushObserverCalls();
		}
	}
	beginLoading(StartBackground);
	ProgressBar.finish();
}
