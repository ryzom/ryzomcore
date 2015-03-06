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
#include "nel/3d/u_driver.h"
#include "far_tp.h"
#include "net_manager.h"
#include "connection.h"
#include "interface_v3/interface_manager.h"
#include "login_patch.h"
#include "init_main_loop.h"
#include "weather.h"
#include "time_client.h"
#include "timed_fx_manager.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "user_entity.h"
#include "entities.h"
#include "interface_v3/input_handler_manager.h"
#include "interface_v3/bot_chat_page_all.h"
#include "sound_manager.h"
#include "actions_client.h"
#include "r2/editor.h"
#include "global.h"
#include "release.h"
#include "nel/misc/string_conversion.h"
#include "debug_client.h"
#include "session_browser_impl.h"
#include "game_share/security_check.h"
#include "client_chat_manager.h"
#include "bg_downloader_access.h"
#include "login_progress_post_thread.h"
#include "interface_v3/action_handler_base.h"

using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;
using namespace RSMGR;
using namespace R2;

extern CClientChatManager		ChatMngr;
extern CVariable<uint16>		SBSPortOffset;

// ***************************************************************************
// Login state machine
// ***************************************************************************

// Adapted from "nel/misc/string_conversion.h"
#define NL_BEGIN_CLASS_STRING_CONVERSION_TABLE(__class, __type)                                               \
static const NLMISC::CStringConversion<__class::__type>::CPair __type##_nl_string_conversion_table[] =          \
{
#define NL_END_CLASS_STRING_CONVERSION_TABLE(__class, __type, __tableName, __defaultValue)                    \
};                                                                                             \
NLMISC::CStringConversion<__class::__type>                                                                \
__tableName(__type##_nl_string_conversion_table, sizeof(__type##_nl_string_conversion_table)   \
			/ sizeof(__type##_nl_string_conversion_table[0]),  __class::__defaultValue);
#define NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(__class, val) { #val, __class::val},


NL_BEGIN_CLASS_STRING_CONVERSION_TABLE (CLoginStateMachine, TState)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_start)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_login)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_auto_login)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_shard_list)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_start_config)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_scan_data)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_display_eula)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_check_patch)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_display_cat)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_patch)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_close_client)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_reboot_screen)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_restart_client)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_connect)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_browser_screen)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_ingame)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_leave_shard)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_enter_far_tp_main_loop)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_disconnect)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_reconnect_fs)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_reconnect_select_char)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_reconnect_ready)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_exit_global_menu)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_reconnect_error)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_create_account)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_end)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, st_unknown)
NL_END_CLASS_STRING_CONVERSION_TABLE(CLoginStateMachine, TState, StateConversion, st_unknown)

NL_BEGIN_CLASS_STRING_CONVERSION_TABLE (CLoginStateMachine, TEvent)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_quit)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_skip_all_login)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_init_done)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_game_conf)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_data_scan)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_close_data_scan)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_login_ok)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_bad_login)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_shard_selected)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_patch_needed)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_no_patch)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_run_patch)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_close_patch)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_accept_eula)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_decline_eula)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_reboot)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_ingame_return)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_far_tp_main_loop_entered)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_connect)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_conn_failed)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_conn_dropped)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_enter_game)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_self_reconnected)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_chars_received)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_no_user_char)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_ready_received)
// 	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_reselect_char)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_reconnect_ok_received)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_global_menu_exited)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_relog)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_create_account)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_close_create_account)
	NL_CLASS_STRING_CONVERSION_TABLE_ENTRY(CLoginStateMachine, ev_unknown)
NL_END_CLASS_STRING_CONVERSION_TABLE(CLoginStateMachine, TEvent, EventConversion, ev_unknown)

//-----------------------------------------------
// toString :
//-----------------------------------------------
const std::string& CLoginStateMachine::toString(CLoginStateMachine::TState state)
{
	return StateConversion.toString(state);
}

//-----------------------------------------------
// toString :
//-----------------------------------------------
const std::string& CLoginStateMachine::toString(CLoginStateMachine::TEvent event)
{
	return EventConversion.toString(event);
}


#define SM_BEGIN_EVENT_TABLE			\
	for(;!isTerminationRequested();)	\
	{									\
		TEvent ev = waitEvent();		\

#define SM_END_EVENT_TABLE				\
	}									\

/*
#define SM_EVENT(eventId, stateId)		\
		if (ev == eventId)				\
		{								\
			try \
			{ \
				COFile outputF;				\
				string sLog = NLMISC::toString("[%s] %s -> %s\n", CLoginStateMachine::toString(ev).c_str(), CLoginStateMachine::toString(_CurrentState).c_str(), CLoginStateMachine::toString(stateId).c_str()); \
				if ( outputF.open( getLogDirectory() + "error_join.log", true, true ) ) \
				{							\
					outputF.serialBuffer( (uint8*)(&sLog[0]), (uint)sLog.size() ); \
					outputF.close();		\
				}							\
			} \
			catch (const Exception &) \
			{} \
			_CurrentState = stateId;	\
			break;						\
		}								\
*/

#define SM_EVENT(eventId, stateId)		\
		if (ev == eventId)				\
		{								\
			_CurrentState = stateId;	\
			break;						\
		}								\

extern std::string LoginLogin, LoginPassword;
extern bool noUserChar;
extern bool userChar;
extern bool serverReceivedReady;
extern bool CharNameValidArrived;
extern bool	FirstFrame;
extern bool IsInRingSession;

extern void selectTipsOfTheDay (uint tips);
#define BAR_STEP_TP 2


CLoginStateMachine::TEvent CLoginStateMachine::waitEvent()
{
	nlassert(CCoTask::getCurrentTask() == this);

	while (_NextEvents.empty() && !isTerminationRequested())
	{
		// wait until someone push an event on the state machine
		yield();

		if (isTerminationRequested())
			return ev_quit;
	}

	TEvent ev = _NextEvents.front();
	_NextEvents.pop_front();

	return ev;
}

void CLoginStateMachine::pushEvent(TEvent eventId)
{
	// set the next event
	_NextEvents.push_back(eventId);

	if (CCoTask::getCurrentTask() != this)
	{
		/// resume the state machine to advance state
		resume();
	}
}


void CLoginStateMachine::run()
{
	// state machine coroutine

	while (_CurrentState != st_end && !isTerminationRequested())
	{
		switch(_CurrentState)
		{
		case st_start:
			/// initial state

			if (!ClientCfg.TestBrowser)
			{
				if (LoginLogin.empty())
				{
					// standard procedure
					SM_BEGIN_EVENT_TABLE
						SM_EVENT(ev_init_done, st_login);
						SM_EVENT(ev_skip_all_login, st_ingame);
						SM_EVENT(ev_quit, st_end);
					SM_END_EVENT_TABLE
				}
				else
				{
					// login 2, bypass the login screen
					SM_BEGIN_EVENT_TABLE
						SM_EVENT(ev_init_done, st_auto_login);
						SM_EVENT(ev_skip_all_login, st_ingame);
						SM_EVENT(ev_quit, st_end);
					SM_END_EVENT_TABLE

				}
			}
//			else
//			{
//				// browser test mode
//				SM_BEGIN_EVENT_TABLE
//					SM_EVENT(ev_init_done, st_browser_screen);
//					SM_EVENT(ev_quit, st_end);
//				SM_END_EVENT_TABLE
//			}

			break;
		case st_login:
			/// display login screen and options
			{
				initLoginScreen();

				if (ClientCfg.R2Mode)
				{
					// r2 mode
					SM_BEGIN_EVENT_TABLE
						SM_EVENT(ev_login_ok, st_check_patch);
						SM_EVENT(ev_game_conf, st_start_config);
						SM_EVENT(ev_data_scan, st_scan_data);
						SM_EVENT(ev_create_account, st_create_account);
						SM_EVENT(ev_bad_login, st_login);
						SM_EVENT(ev_quit, st_end);
					SM_END_EVENT_TABLE
				}
				else
				{
					// legacy mode
					SM_BEGIN_EVENT_TABLE
						SM_EVENT(ev_login_ok, st_shard_list);
						SM_EVENT(ev_game_conf, st_start_config);
						SM_EVENT(ev_data_scan, st_scan_data);
						SM_EVENT(ev_create_account, st_create_account);
						SM_EVENT(ev_bad_login, st_login);
						SM_EVENT(ev_quit, st_end);
					SM_END_EVENT_TABLE
				}
			}
			break;
		case st_auto_login:
			initAutoLogin();

//				if (ClientCfg.R2Mode)
			{
				// r2 mode
				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_login_ok, st_check_patch);
					SM_EVENT(ev_quit, st_end);
				SM_END_EVENT_TABLE
			}
//				else
//				{
//					// legacy mode
//					SM_BEGIN_EVENT_TABLE
//						SM_EVENT(ev_login_ok, st_check_patch);
//						SM_EVENT(ev_quit, st_end);
//					SM_END_EVENT_TABLE
//				}
			break;
		case st_shard_list:
			/// display the shard list
			initShardDisplay();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_shard_selected, st_check_patch);
				SM_EVENT(ev_quit, st_end);
			SM_END_EVENT_TABLE

			break;
		case st_start_config:
			/// launch the configurator and close ryzom
			break;
		case st_scan_data:
			/// run the check data thread
			initDataScan();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_close_data_scan, st_login);
				SM_EVENT(ev_quit, st_end);
			SM_END_EVENT_TABLE

			break;
		case st_create_account:

			if (initCreateAccount())
			{
				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_login_ok, st_check_patch);
					SM_EVENT(ev_close_create_account, st_login);
					SM_EVENT(ev_quit, st_end);
				SM_END_EVENT_TABLE
			}
			else
			{
				// return to login menu if an error occured
				_CurrentState = st_login;
			}

			break;
		case st_display_eula:
		{
			/// display the eula and wait for validation

			if (ClientCfg.SkipEULA)
			{
				// we don't want to see eula, auto accept
				pushEvent(ev_accept_eula);
			}

			bool mustReboot = false;

			if (isBGDownloadEnabled())
			{
				mustReboot = CBGDownloaderAccess::getInstance().mustLaunchBatFile();
			}
			else
			{
				mustReboot = CPatchManager::getInstance()->mustLaunchBatFile();
			}

			if (mustReboot)
			{
				// skip eula and show reboot screen
				_CurrentState = st_reboot_screen;
				break;
			}

			initEula();

//			Login sequence was reordered
//			if (ClientCfg.R2Mode)
//			{
//				// ring mode
//				SM_BEGIN_EVENT_TABLE
//					SM_EVENT(ev_accept_eula, st_browser_screen);
//					SM_EVENT(ev_quit, st_end);
//				SM_END_EVENT_TABLE
//			}
//			else
//			{
//				// legacy mode
				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_accept_eula, st_connect);
					SM_EVENT(ev_quit, st_end);
				SM_END_EVENT_TABLE
//			}
		}
		break;
		case st_check_patch:
			/// check the data to check if patch needed
			CLoginProgressPostThread::getInstance().step(CLoginStep(LoginStep_PostLogin, "login_step_post_login"));
			if (!ClientCfg.PatchWanted)
			{
				// client don't want to be patched !
				_CurrentState = st_display_eula;
				break;
			}
			initPatchCheck();
			SM_BEGIN_EVENT_TABLE
				if (isBGDownloadEnabled())
				{
					SM_EVENT(ev_patch_needed, st_patch); // no choice for patch content when background downloader is used
				}
				else
				{
					SM_EVENT(ev_patch_needed, st_display_cat);
				}
				SM_EVENT(ev_no_patch, st_display_eula);
				SM_EVENT(ev_quit, st_end);
			SM_END_EVENT_TABLE

			break;
		case st_display_cat:
			initCatDisplay();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_run_patch, st_patch);
				SM_EVENT(ev_quit, st_end);
			SM_END_EVENT_TABLE

			break;
		case st_patch:
			/// run the patch process and display progress
			initPatch();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_close_patch, st_display_eula);
				SM_EVENT(ev_quit, st_end);
			SM_END_EVENT_TABLE
			break;
		case st_close_client:
			/// terminate the client and quit
			break;
		case st_reboot_screen:
			/// display the reboot screen and wait validation
			initReboot();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_reboot, st_end);
				SM_EVENT(ev_quit, st_end);
			SM_END_EVENT_TABLE

			break;
		case st_restart_client:
			/// restart the client with login bypass params
			break;
		case st_connect:
			/// connect to the FS (start the 'in game' mode)
			ConnectToShard();

			if (ClientCfg.R2Mode)
			{
				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_enter_game, st_ingame);
				SM_END_EVENT_TABLE
			}
			else
			{
				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_enter_game, st_end);
					SM_EVENT(ev_conn_failed, st_shard_list);
				SM_END_EVENT_TABLE
			}
			break;
//		case st_browser_screen:
//			/// show the outgame browser
//
//			if (!ClientCfg.TestBrowser)
//			{
//				// in test browser mode, the browser is already init
//				initWebBrowser();
//			}
//
//			SM_BEGIN_EVENT_TABLE
//				SM_EVENT(ev_connect, st_connect);
//				SM_EVENT(ev_relog, st_login);
//				SM_EVENT(ev_quit, st_end);
//			SM_END_EVENT_TABLE
//
//			break;
		case st_ingame:
			SM_BEGIN_EVENT_TABLE
				//SM_EVENT(ev_chars_received, st_ingame); // ignored for normal login procedure
				//SM_EVENT(ev_no_user_char, st_ingame);   // "
				//SM_EVENT(ev_ready_received, st_ingame); // "
				//SM_EVENT(ev_global_menu_exited, st_ingame); "
				SM_EVENT(ev_connect, st_leave_shard); // connect to another shard
			SM_END_EVENT_TABLE
			break;
		case st_leave_shard:
			/*
			 * Far TP / Server Hop / Reselect character entry point
			 */

			// Server Hop part 1: We are not in main loop but still at character selection
			//   => make a hop to the specified server without doing anything with interface
			// Far TP part 1.1: From the ingame main loop, the admin html box gives us an event ev_connect for the destination shard.
			//   Note: the admin html box is run by CInputHandlerManager::getInstance()->pumpEvents() in the main loop.
			//   Tip: to see where a co-task is resumed from, just add a breakpoint on the end of CCoTask::resume().
			CAHManager::getInstance()->runActionHandler("quit_ryzom", NULL, "");

			if (!FarTP.isIngame()) // assumes there is no Far TP starting between char selection and main loop, see below
			{
				crashLogAddServerHopEvent();

				FarTP.onServerQuitOk(); // don't wait for onServerQuitOK() because the EGS will no longer send it if it requests a Far TP during "select char" (not sending USER_CHAR)

				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_ingame_return, st_disconnect);
				SM_END_EVENT_TABLE
			}
			else
			{
				if(FarTP.isReselectingChar())
					crashLogAddReselectPersoEvent();
				else
					crashLogAddFarTpEvent();

				if (NetMngr.getConnectionState() == CNetworkConnection::Disconnect)
					FarTP.onServerQuitOk(); // don't wait for onServerQuitOK() because the EGS is down

				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_ingame_return, st_enter_far_tp_main_loop);
					SM_EVENT(ev_enter_game, st_ingame);
				SM_END_EVENT_TABLE
			}
			break;
		case st_enter_far_tp_main_loop:
			// if bgdownloader is used, then pause it
			pauseBGDownloader();


			// Far TP part 1.2: let the main loop finish the current frame.
			// This is called when CONNECTION:SERVER_QUIT_OK is received (from NetMngr.update() in main loop).

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_far_tp_main_loop_entered, st_disconnect);
			SM_END_EVENT_TABLE
			break;
		case st_disconnect:
			// Far TP part 2: disconnect from the FS and unload shard-specific data (called from farTPmainLoop())
			FarTP.disconnectFromPreviousShard();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_connect, st_reconnect_fs);
			SM_END_EVENT_TABLE
			break;
		case st_reconnect_fs:
			// Server Hop part 3.1: connect to the new shard (called from globalMenu())
			// Far TP part 3.1: connect to the new shard (called from farTPmainLoop())
			FarTP.connectToNewShard();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_self_reconnected, st_ingame);
				SM_EVENT(ev_chars_received, st_reconnect_select_char);
				SM_EVENT(ev_no_user_char, st_reconnect_error);
				SM_EVENT(ev_conn_failed, st_reconnect_error);
				SM_EVENT(ev_conn_dropped, st_reconnect_error);
			SM_END_EVENT_TABLE
			break;
		case st_reconnect_select_char:
			// Server Hop part 3.2: bypass character selection ui & select the same character.
			// Far TP part 3.2: bypass character selection ui & select the same character.
			//   This is called from farTPmainloop(), when CONNECTION:USER_CHARS is received.
			if( !FarTP.isReselectingChar() )
			{
				FarTP.selectCharAndEnter();
			}
			// Far TP part 3.2bis: see in farTPmainLoop()

			if (!FarTP.isIngame())
			{
				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_ready_received, st_exit_global_menu);
					SM_EVENT(ev_conn_dropped, st_reconnect_error);
				SM_END_EVENT_TABLE
			}
			else
			{
				SM_BEGIN_EVENT_TABLE
					SM_EVENT(ev_ready_received, st_reconnect_ready);
					SM_EVENT(ev_conn_dropped, st_reconnect_error);
					if ( FarTP.isReselectingChar() && (ev == ev_connect) )
					{
						// Inside this Character Reselect we embed a new Server Hop
						FarTP.beginEmbeddedServerHop();
						_CurrentState = st_leave_shard;
						break;
					}
				SM_END_EVENT_TABLE
			}
			break;
		case st_exit_global_menu:
			// Server Hop part 3.3
			// Stay in Server Hop state until the global menu has been exited
			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_global_menu_exited, FarTP.isServerHopEmbedded() ? st_reconnect_ready : st_ingame);
			SM_END_EVENT_TABLE
			break;
		case st_reconnect_ready:
			// Far TP part 3.3: send ready.
			//   This is called from farTPmainloop(), when CONNECTION:READY is received.

			if ( FarTP.isServerHopEmbedded() )
				FarTP.endEmbeddedServerHop();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_enter_game, st_ingame);
				SM_EVENT(ev_conn_dropped, st_reconnect_error);
			SM_END_EVENT_TABLE
			break;
		case st_reconnect_error:
			// Far TP failed
			FarTP.onFailure();

			SM_BEGIN_EVENT_TABLE
				SM_EVENT(ev_quit, st_end);
			SM_END_EVENT_TABLE
			break;
		default:
			nlwarning("Unhandeled state");
			break;
		}
	}
}


// ***************************************************************************
// Far TP
// ***************************************************************************

CFarTP FarTP;

extern std::string CurrentCookie;
extern string ClientApp;


namespace R2
{
	extern bool ReloadUIFlag;
}


/*
 * requestFarTPToSession
 */
bool CFarTP::requestFarTPToSession(TSessionId sessionId, uint8 charSlot, CFarTP::TJoinMode joinMode, bool bailOutIfSessionVanished)
{
	if (_HookedForEditor)
	{
		// Redirect Far TP to editor instead of following instructions
		_HookedForEditor = false;
		return FarTP.requestFarTPToSession((TSessionId)0, charSlot, CFarTP::LaunchEditor, true); // allow bailing out in case of edition session not reachable
	}

	// call the join session using the session browser interface
	uint32 charId = (NetMngr.getLoginCookie().getUserId()<<4)+(0xf&charSlot);

	// Clean out for next Far TP
	_SessionIdToJoinFast = 0;

	CSessionBrowserImpl	&sb = CSessionBrowserImpl::getInstance();
	sb.init(NULL);
//	sb.setAuthInfo(NetMngr.getLoginCookie());
//	sb.connectItf(CInetAddress("borisb", 80));

	sb.CurrentJoinMode = joinMode;
	// send the join session
//	_JoinSessionResultReceived = false;
	try
	{
		switch (joinMode)
		{
		case LaunchEditor:
			{
retryJoinEdit:
				TSessionId sessionId(0);
				sb.joinEditSession(charId, ClientApp);

				bool ret = sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_joinSessionResult"));

				if (!ret)
					throw "Protocol error";
				if (sb._LastJoinSessionResult == 2)
				{
					// the edit session did not exist, create a new one
					sb.scheduleSession(charId, TSessionType::st_edit, "", "", TSessionLevel::sl_a, /*TAccessType::at_private, */TRuleType::rt_strict, TEstimatedDuration::et_long, 0, TAnimMode(), TRaceFilter(), TReligionFilter(), TGuildFilter(), TShardFilter(), TLevelFilter(), std::string(), TSessionOrientation(), true, true);
					ret = sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_scheduleSessionResult"));
					if (!ret || sb._LastScheduleSessionResult!= 0)	throw "schedule error";
					sessionId = sb._LastScheduleSessionId;

					// invite the char in the session
					sb.inviteCharacter(charId, sessionId, charId, TSessionPartStatus::sps_edit_invited);
					ret = sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_invokeResult"));
					if (!ret || sb._LastInvokeResult != 0)	throw "Invitation Error";

					// now, start the edit session
					sb.startSession(charId, sessionId);
					ret = sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_invokeResult"));
					if (!ret || sb._LastInvokeResult != 0)	throw "start session error";

					// retry to join
					goto retryJoinEdit;
				}
				else if (sb._LastJoinSessionResult == 15)
				{
					sessionId = sb._LastJoinSessionId;
					// the session was closed, the SU has put it in planned state, start it
					sb.startSession(charId, sessionId);
					ret = sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_invokeResult"));
					if (!ret || sb._LastInvokeResult != 0)	throw "start session error (2), no DSS available";
					// retry to join
					goto retryJoinEdit;
				}
				else if (sb._LastJoinSessionResult != 0)
				{
					// any other error are not recoverable from here
					throw "join session error";
				}

				// ok, the join is accepted !, the other far TP work is done in the callback
			}
			break;
		case JoinSession:
			{
				sb.joinSession(charId, sessionId, ClientApp);

				bool ret = sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_joinSessionResult"));

				if (!ret)
					throw "Protocol error";
				if (sb._LastJoinSessionResult == 16)
				{
//	#pragma message (NL_LOC_WRN "inform the player that he is banned from the ring")
					throw "User ban from the ring";
				}
				if (sb._LastJoinSessionResult != 0)
				{
					throw "Join session error";
				}
				// ok, the join is accepted !, the other far TP work is done in the callback
			}
			break;
		case JoinMainland:
//retryJoinMainland:
			{
				TSessionId sessionId(0);
				sb.joinMainland(charId, ClientApp);

				bool ret = sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_joinSessionResult"));

				if (!ret)
					throw "Protocol error";
				if (sb._LastJoinSessionResult != 0)
				{
					// some error during the join
					throw "Error joining session";
				}
				// ok, the join is accepted !, the other far TP work is done in the callback
			}
			break;
		default:
			nlstop;
		}

		return true;
	}

//
//	const string url = _URLBase + string(_URLTable[joinMode]);
//	string res;
//	try
//	{
//		// Get the address of a front-end service via http
//		if (!HttpClient.connect(url))
//			throw 1;
//		switch (joinMode)
//		{
//		case CFarTP::LaunchEditor:
//			if (!HttpClient.sendGetWithCookie(url, "ryzomId", CurrentCookie, toString("charSlot=%u", charSlot)))
//				throw 2;
//			break;
//		case CFarTP::JoinSession:
//			if (!HttpClient.sendPostWithCookie(url, "ryzomId", CurrentCookie, toString("sessionId=%u&charSlot=%u", sessionId, charSlot)))
//				throw 2;
//			break;
//		case CFarTP::JoinMainland:
//			if (!HttpClient.sendPostWithCookie(url, "ryzomId", CurrentCookie, "ml="))
//				throw 2;
//			break;
//		}
//		if (!HttpClient.receive(res))
//			throw 3;
//		HttpClient.disconnect();
//		string::size_type luaPos = res.find("<lua>");
//		if (luaPos == string::npos)
//			throw 4;
//		res = res.substr(luaPos + 5);
//		res = res.substr(0, res.find("</lua>"));
//		nlinfo("Found server for %ssession %u", joinMode==CFarTP::LaunchEditor ? "editing " : "", sessionId);
//
//		// Begin Far TP
//		CInterfaceManager *pIM = CInterfaceManager::getInstance();
//		pIM->executeLuaScript(res, true);
//		return true;
//	}
	catch ( char *errorMsg )
	{
//		// Get more details on the error
//		string httpErrorStr;
//		if (errorNum < 4) // we didn't receive an answer
//			httpErrorStr = toString(" (HTTP error %u)", errorNum);
//		if ( errorNum == 4 )
//		{
//			string::size_type posEndOfFirstLine = res.find( "\n" );
//			if ( posEndOfFirstLine == string::npos )
//			{
//				// Invalid http answer
//				httpErrorStr = toString(" (HTTP invalid)");
//			}
//			else
//			{
//				// Http status not OK (e.g. "404 Not Found")
//				httpErrorStr = res.substr( 0, posEndOfFirstLine );
//				if ( httpErrorStr.find( "200 OK" ) == string::npos )
//					httpErrorStr = " (" + httpErrorStr + ")";
//				else
//					httpErrorStr.clear();
//			}
//		}
		bool requestRetToMainland = /*httpErrorStr.empty() &&*/ bailOutIfSessionVanished;
		bool isAttemptingEmbeddedServerHop = isReselectingChar() && (joinMode != CFarTP::JoinSession);
		bool letReturnToCharSelect = (!isIngame()) || isAttemptingEmbeddedServerHop;

		// User-friendly message
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if ( letReturnToCharSelect )
		{
//			// Hide all buttons except Quit. If !requestRetToMainland, we will show them back at the end of connectToNewShard().
//			CAHManager::getInstance()->runActionHandler( "proc", NULL, "charsel_disable_buttons" );
//			CAHManager::getInstance()->runActionHandler( "set", NULL, "target_property=ui:outgame:charsel:quit_but:active|value=1" );

			CInterfaceElement *btnOk = CWidgetManager::getInstance()->getElementFromId("ui:outgame:charsel:message_box:ok");
			if (btnOk)
				btnOk->setActive( ! requestRetToMainland );

			// Hide the black screen i.e. force showing the interface
			CInterfaceElement *charSelBlackScreen = CWidgetManager::getInstance()->getElementFromId("ui:outgame:charsel:black_screen");
			if (charSelBlackScreen)
			{
				CViewBase *charSelBlackScreenBitmap = dynamic_cast<CViewBase*>(charSelBlackScreen);
				if (charSelBlackScreenBitmap)
					charSelBlackScreenBitmap->setAlpha(0);
			}
		}
		pIM->messageBoxWithHelp(
			CI18N::get(requestRetToMainland ? "uiSessionVanishedFarTP" : "uiSessionUnreachable") + ucstring(errorMsg),
			letReturnToCharSelect ? "ui:outgame:charsel" : "ui:interface");

		// Info in the log
		string outErrorMsg = toString("Could not join %ssession %u: '%s' error\n", joinMode==CFarTP::LaunchEditor ? "editing " : "", sessionId.asInt(), errorMsg );
		nlwarning( outErrorMsg.c_str() );
//		if ( httpErrorStr.empty() )
//		{
//			string delimiter = "Content-Type: text/html";
//			string::size_type delimPos = res.find( delimiter );
//			if ( delimPos != string::npos )
//				res = res.substr( delimPos + delimiter.size() );
//		}
//		uint pos = 0;
//		do
//		{
//			WarningLog->displayRaw( res.substr( pos, 200 ).c_str() ); // truncates long strings
//			pos += 200;
//		}
//		while ( pos < res.size() );
		WarningLog->displayRawNL( "" );

		// Save this error (regular log file is deleted at every startup)
//		res = res + "\n\n";
		/*try
		{
			COFile outputF;
			if ( outputF.open( getLogDirectory() + "error_join.log", true, true ) )
			{
				time_t currentTime;
				time( &currentTime );
				string headerS = NLMISC::toString( "\n\n%s%s\n\n", asctime(localtime(&currentTime)), outErrorMsg.c_str() );
				outputF.serialBuffer( (uint8*)(&headerS[0]), (uint)headerS.size() );
//				outputF.serialBuffer( (uint8*)(&res[0]), res.size() );
				outputF.close();
			}
		}
		catch (const Exception &)
		{}
		*/

		// If the session is not a permanent session and has vanished, pop the position
		if ( requestRetToMainland )
		{
			requestReturnToPreviousSession( sessionId );
			LastGameCycle = NetMngr.getCurrentServerTick();
			NetMngr.send(NetMngr.getCurrentServerTick());
		}
		else if ( letReturnToCharSelect )
		{
			// Show all buttons except 'New character' so that the character can retry entering game or choose another character.
			CAHManager::getInstance()->runActionHandler( "proc", NULL, "charsel_enable_buttons" );
			CAHManager::getInstance()->runActionHandler( "set", NULL, "target_property=ui:outgame:charsel:create_new_but:active|value=0" );

			CInterfaceGroup* charselGroup = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:outgame:charsel"));
			if(charselGroup)
				CAHManager::getInstance()->runActionHandler( "proc", charselGroup, "charsel_init_buttons" );
		}

		return false;
	}
}


/*
 * hookNextFarTPForEditor
 */
void CFarTP::hookNextFarTPForEditor()
{
	_HookedForEditor = true;
}

/*
 * requestReturnToPreviousSession
 */
void CFarTP::requestReturnToPreviousSession(TSessionId rejectedSessionId)
{
	const string msgName = "CONNECTION:RET_MAINLAND";
	CBitMemStream out;
	nlverify(GenericMsgHeaderMngr.pushNameToStream(msgName, out));
	out.serial(PlayerSelectedSlot);
	out.serial(rejectedSessionId);
	NetMngr.push(out);
	nlinfo("%s sent", msgName.c_str());
}

/*
 * requestReconnection
 */
void CFarTP::requestReconnection()
{
	_ReselectingChar = true;
	if (!requestFarTPToSession(TSessionId(std::numeric_limits<uint16>::max()), std::numeric_limits<uint8>::max(), CFarTP::JoinMainland, false))
		_ReselectingChar = false;
}


const char * CFarTP::_URLTable[CFarTP::NbJoinModes] =
{
	"edit_session.php",
	"join_session.php",
	"join_shard.php"
};


/*
 * States
 */

bool CFarTP::isFarTPInProgress() const
{
	CLoginStateMachine::TState state = LoginSM.getCurrentState();
	return (state == CLoginStateMachine::st_leave_shard ||
			state == CLoginStateMachine::st_enter_far_tp_main_loop ||
			state == CLoginStateMachine::st_disconnect ||
			state == CLoginStateMachine::st_reconnect_fs ||
			state == CLoginStateMachine::st_reconnect_select_char ||
			state == CLoginStateMachine::st_reconnect_ready ||
			state == CLoginStateMachine::st_reconnect_error);
}

bool CFarTP::isServerHopInProgress() const
{
	CLoginStateMachine::TState state = LoginSM.getCurrentState();
	return (state == CLoginStateMachine::st_leave_shard ||
			state == CLoginStateMachine::st_reconnect_fs ||
			state == CLoginStateMachine::st_reconnect_select_char ||
			state == CLoginStateMachine::st_exit_global_menu ||
			state == CLoginStateMachine::st_reconnect_error); // TODO: error handling
}


// from begin of Far TP to disconnection
bool CFarTP::isLeavingShard() const
{
	CLoginStateMachine::TState state = LoginSM.getCurrentState();
	return (state == CLoginStateMachine::st_leave_shard ||
			state == CLoginStateMachine::st_enter_far_tp_main_loop ||
			state == CLoginStateMachine::st_disconnect);
}

// from reconnection to game entering
bool CFarTP::isJoiningShard() const
{
	CLoginStateMachine::TState state = LoginSM.getCurrentState();
	return (state == CLoginStateMachine::st_reconnect_fs ||
			state == CLoginStateMachine::st_reconnect_select_char ||
			state == CLoginStateMachine::st_reconnect_ready ||
			state == CLoginStateMachine::st_reconnect_error);
}


/*
 * Events
 */

void CFarTP::onServerQuitOk()
{
	game_exit_request = false;
	ryzom_exit_request = false;

	if (LoginSM.getCurrentState() == CLoginStateMachine::st_leave_shard)
		LoginSM.pushEvent(CLoginStateMachine::ev_ingame_return);
}

void CFarTP::onServerQuitAbort()
{
	game_exit_request = false;
	ryzom_exit_request = false;

	if (LoginSM.getCurrentState() == CLoginStateMachine::st_leave_shard)
		LoginSM.pushEvent(CLoginStateMachine::ev_enter_game);
	_ReselectingChar = false;
}

void CFarTP::disconnectFromPreviousShard()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (isIngame())
	{
		// Display background (TODO: not Kami)
		beginLoading (StartBackground);
		UseEscapeDuringLoading = false;

		// Play music and fade out the Game Sound
		if (SoundMngr)
		{
			// Loading Music Loop.ogg
			LoadingMusic = ClientCfg.SoundOutGameMusic;
			SoundMngr->playEventMusic(LoadingMusic, CSoundManager::LoadingMusicXFade, true);
			SoundMngr->fadeOutGameSound(ClientCfg.SoundTPFade);
		}

		// Change the tips
		selectTipsOfTheDay (rand());

		// Start progress bar and display background
		ProgressBar.reset (BAR_STEP_TP);
		ucstring nmsg("Loading...");
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		ProgressBar.progress(0);
	}

	// Disconnect from the FS
	NetMngr.disconnect();

	if (isIngame())
	{
		// Save the R2EDEnabled flag to know if we are switching it when we reconnect
		_PreviousR2EdEnabled = ClientCfg.R2EDEnabled;

		// Release R2 editor if applicable
		R2::getEditor().autoConfigRelease(IsInRingSession);
	}

	if( isReselectingChar() )
	{
		releaseMainLoopReselect();
	}
	else
	{
		// String manager: remove all waiting callbacks and removers
		// (if some interface stuff has not received its string yet, its remover will get useless)
		STRING_MANAGER::CStringManagerClient::release( false );

		// Ugly globals
		userChar = false;
		noUserChar = false;
		serverReceivedReady = false;
		CharNameValidArrived = false;
		UserCharPosReceived = false;
		SabrinaPhraseBookLoaded = false;
	}

	// Restart the network manager, the interface sync counter and the entities
	pIM->resetShardSpecificData();

	/*
		ServerToLocal autocopy stuff:
		When reinit will reset server counters to 0 in the server database, onServerChange()
		will be called and data will be copied since CInterfaceManager::_LocalSyncActionCounter==0
		(done in resetShardSpecificData() above)
		If we do the resetShardSpecificData() after, scenari could arise where Local database could not be reseted
	*/
	NetMngr.reinit();

	if (isIngame() && !isReselectingChar())
	{
		nlinfo("FarTP: calling EntitiesMngr.reinit()");
		EntitiesMngr.reinit();
	}
	LoginSM.pushEvent(CLoginStateMachine::ev_connect);
}

void CFarTP::connectToNewShard()
{
	// TODO: start commands?

	// Connect to the next FS
	NetMngr.initCookie(Cookie, FSAddr);

	// connect the session browser to the new shard
	NLNET::CInetAddress sbsAddress(CSessionBrowserImpl::getInstance().getFrontEndAddress());
	sbsAddress.setPort(sbsAddress.port()+SBSPortOffset);
	CSessionBrowserImpl::getInstance().connectItf(sbsAddress);

	string result;
	NetMngr.connect(result);
	if (!result.empty())
	{
		_Reason = new string(result);
		LoginSM.pushEvent(CLoginStateMachine::ev_conn_failed);
		return;
	}

	// Reinit the string manager cache.
	STRING_MANAGER::CStringManagerClient::instance()->initCache(FSAddr, ClientCfg.LanguageCode);

	// reset the chat mode
	ChatMngr.resetChatMode();

	// The next step will be triggered by the CONNECTION:USER_CHARS msg from the server
}

// return to character selection screen
bool CFarTP::reselectCharacter()
{
	if ( ! reconnection() )
	{
		// The user clicked the Quit button
		releaseOutGame();
		return false;
	}
	return true;
}

void CFarTP::selectCharAndEnter()
{
	CBitMemStream out;
	nlverify (GenericMsgHeaderMngr.pushNameToStream("CONNECTION:SELECT_CHAR", out));
	CSelectCharMsg	SelectCharMsg;
	SelectCharMsg.c = (uint8)PlayerSelectedSlot; // PlayerSelectedSlot has not been reset
	out.serial(SelectCharMsg);
	NetMngr.push(out);
	/*//Obsolete
	CBitMemStream out2;
	nlverify(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:ENTER", out2));
	NetMngr.push(out2);
	*/
	LastGameCycle = NetMngr.getCurrentServerTick();
	NetMngr.send(NetMngr.getCurrentServerTick());

	// if (!isIngame())
	//   globalMenu will exit when WaitServerAnswer and serverReceivedReady (triggered by the CONNECTION:READY msg) are true
	// else
	//   Next step will be triggered by the CONNECTION:READY msg from the server
}

void CFarTP::sendReady()
{
	if ( isReselectingChar() )
	{
		initMainLoop();
	}
	else
	{
		// Set season
		RT.updateRyzomClock(NetMngr.getCurrentServerTick(), ryzomGetLocalTime() * 0.001);
		DayNightCycleHour	= (float)RT.getRyzomTime();
		CurrSeason = RT.getRyzomSeason();
		RT.updateRyzomClock(NetMngr.getCurrentServerTick(), ryzomGetLocalTime() * 0.001);
		DayNightCycleHour	= (float)RT.getRyzomTime();
		ManualSeasonValue = RT.getRyzomSeason();

		// Reset all fx (no need to CTimedFXManager::getInstance().reset() and EntitiesMngr.getGroundFXManager().reset() because the entities have been removed)
		CProjectileManager::getInstance().reset();
		FXMngr.reset(); // (must be done after EntitiesMngr.release())
		EntitiesMngr.getGroundFXManager().init(Scene, ClientCfg.GroundFXMaxDist, ClientCfg.GroundFXMaxNB, ClientCfg.GroundFXCacheSize);

		// Get the sheet for the user from the CFG.
		// Initialize the user and add him into the entity manager.
		// DO IT AFTER: Database, Collision Manager, PACS, scene, animations loaded.
		CSheetId userSheet(ClientCfg.UserSheet);
		nlinfo("FarTP: calling EntitiesMngr.create(0, userSheet.asInt())");
		TNewEntityInfo emptyEntityInfo;
		emptyEntityInfo.reset();
		EntitiesMngr.create(0, userSheet.asInt(), emptyEntityInfo);

		// Wait for the start position (USER_CHAR) and set the continent
		waitForUserCharReceived();


		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if ( ClientCfg.R2EDEnabled != _PreviousR2EdEnabled )
		{
			// Reload textures, keys and interface config if we are switch between playing and r2 mode
			// Must be done after receiving the current R2EDEnabled flag (USER_CHAR)
			pIM->loadIngameInterfaceTextures();

			// Unload (and save if leaving normal playing mode) keys and interface config
			// Instead of doing it in disconnectFromPreviousShard(), we do it here, only when it's needed
			ClientCfg.R2EDEnabled = ! ClientCfg.R2EDEnabled;
			pIM->uninitInGame0();
			ClientCfg.R2EDEnabled = ! ClientCfg.R2EDEnabled;
			ActionsContext.removeAllCombos();

			if ( ! ClientCfg.R2EDEnabled )
			{
				// Remove all existing keys and load them back, and load new interface config
				pIM->loadKeys();
				CWidgetManager::getInstance()->hideAllWindows();
				pIM->loadInterfaceConfig();
			}
			else
			{
				R2::ReloadUIFlag = true; // in R2ED mode the CEditor class deals with it
			}
		}
		pIM->configureQuitDialogBox(); // must be called after waitForUserCharReceived() to know the ring config

		ContinentMngr.select(UserEntity->pos(), ProgressBar); // IMPORTANT : must select continent after ui init, because ui init also load landmarks (located in the icfg file)
															  // landmarks would be invisible else (RT 12239)



		// Update Network until current tick increase.
		LastGameCycle = NetMngr.getCurrentServerTick();
		while (LastGameCycle == NetMngr.getCurrentServerTick())
		{
			// Event server get events
			CInputHandlerManager::getInstance()->pumpEventsNoIM();
			// Update Network.
			NetMngr.update();
			IngameDbMngr.flushObserverCalls();
			NLGUI::CDBManager::getInstance()->flushObserverCalls();
			// Be nice to the system
			nlSleep(100);
		}
		LastGameCycle = NetMngr.getCurrentServerTick();
		ProgressBar.progress(1);

		// Create the message for the server to create the character.
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:READY", out))
		{
			out.serial(ClientCfg.LanguageCode);
			NetMngr.push(out);
			NetMngr.send(NetMngr.getCurrentServerTick());
		}

		// To be sure server crash is not fault of client
		ConnectionReadySent = true; // must be called before BotChatPageAll->initAfterConnectionReady()

		// To reset the inputs.
		CInputHandlerManager::getInstance()->pumpEventsNoIM();

		if(BotChatPageAll && (! ClientCfg.R2EDEnabled))
			BotChatPageAll->initAfterConnectionReady();

		// Transition from background to game
		FirstFrame = true;
	}

	ProgressBar.finish();

	LoginSM.pushEvent(CLoginStateMachine::ev_enter_game);

	if (_DSSDown)
	{
		_DSSDown = false;
		onDssDown(true);
	}
}

void CFarTP::onFailure()
{
	// Display message
	string reason;
	if (_Reason)
	{
		reason += ": " + (*_Reason);
		delete _Reason;
		_Reason = NULL;
	}
	else if (noUserChar)
	{
		reason += ": no characters found!";
	}
	Driver->systemMessageBox(("Unable to join shard"+reason).c_str(), "Error", UDriver::okType, UDriver::exclamationIcon);

	// TODO: recover from error

	LoginSM.pushEvent(CLoginStateMachine::ev_quit);
}

void CFarTP::onDssDown(bool forceReturn)
{
	if (!forceReturn)
	{
		// If leaving shard, don't bother with DSS "downitude"
		if (isLeavingShard())
			return;
		// If joining shard, store event and launch it at the end of reconnection
		if (isJoiningShard())
		{
			_DSSDown = true; // note: still, some cases will make the client hang or abort, e.g. DSS down before the client receives the start pos (in ring shard)
			return;
		}
	}

	CInterfaceManager *pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(CI18N::get("uiDisconnected"));
	requestReturnToPreviousSession();
}

extern bool loginFinished;
void setLoginFinished( bool f );
extern bool loginOK;

void CFarTP::joinSessionResult(uint32 /* userId */, TSessionId /* sessionId */, uint32 /* result */, const std::string &/* shardAddr */, const std::string &/* participantStatus */)
{
//	_LastJoinSessionResultMsg = result;
//
//	_JoinSessionResultReceived = true;
//
//	if (result == 0)
//	{
//		// ok, the join is successful
//
//		FSAddr = shardAddr;
//
//		setLoginFinished( true );
//		loginOK = true;
//
//		LoginSM.pushEvent(CLoginStateMachine::ev_connect);
//
//	}
//	else
//	{
//		// TODO : display the error message on client screen and log
//		nlstop;
//	}
}


void CFarTP::setJoinSessionResult(TSessionId sessionId, const CSecurityCode& securityCode)
{
	_SessionIdToJoinFast = sessionId;
	_SecurityCodeForDisconnection = securityCode;
}

void CFarTP::writeSecurityCodeForDisconnection(NLMISC::IStream& msgout)
{
	CSecurityCheckForFastDisconnection::forwardSecurityCode(msgout, _SessionIdToJoinFast, _SecurityCodeForDisconnection);
}

// Not run by the cotask but within the main loop
void CFarTP::farTPmainLoop()
{
	ConnectionReadySent = false;
	LoginSM.pushEvent(CLoginStateMachine::ev_far_tp_main_loop_entered);
	uint nbRecoSelectCharReceived = 0;

	bool welcomeWindow = true;

	// Update network until the end of the FarTP process, before resuming the main loop
	while (!ConnectionReadySent)
	{
		// Event server get events
		CInputHandlerManager::getInstance()->pumpEventsNoIM();

		// Update Network.
		NetMngr.update();
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();

		// TODO: resend in case the last datagram sent was lost?
//		// check if we can send another dated block
//		if (NetMngr.getCurrentServerTick() != serverTick)
//		{
//			//
//			serverTick = NetMngr.getCurrentServerTick();
//			NetMngr.send(serverTick);
//		}
//		else
//		{
//			// Send dummy info
//			NetMngr.send();
//		}

		if (LoginSM.getCurrentState() == CLoginStateMachine::st_reconnect_select_char)
		{
			// Far TP part 3.2bis: go to character selection dialog
			// This is done outside the co-routine because it may need to co-routine to do an embedded server hop
			if ( FarTP.isReselectingChar() )
			{
				++nbRecoSelectCharReceived;
				if ( nbRecoSelectCharReceived <= 1 )
				{
					ClientCfg.SelectCharacter = -1;	// turn off character autoselection
					if ( ! FarTP.reselectCharacter() ) // it should not return here in farTPmainLoop() in the same state otherwise this would be called twice
						return;
				}
				else
					nlwarning( "Received more than one st_reconnect_select_char event" );
			}
		}
		else if (LoginSM.getCurrentState() == CLoginStateMachine::st_reconnect_ready)
		{
			// Don't call sendReady() within the cotask but within the main loop, as it contains
			// event/network loops that could trigger a global exit().
			sendReady();
			welcomeWindow = !isReselectingChar();
		}

		// Be nice to the system
		nlSleep(100);
	}

	// active/desactive welcome window
	if(welcomeWindow)
		initWelcomeWindow();
}

