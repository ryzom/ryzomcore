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

#ifndef FAR_TP_H
#define FAR_TP_H

#include "nel/misc/co_task.h"
#include <list>
#include <string>
#include "game_share/security_check.h"


class CLoginStateMachine : private NLMISC::CCoTask
{

public:
	enum TState
	{
		/// initial state
		st_start,
		/// display login screen and options
		st_login,
		/// auto login using cmd lien parameters (used with patch reboot)
		st_auto_login,
		/// display the shard list
		st_shard_list,
		/// lauch the configurator and close ryzom
		st_start_config,
		/// run the scan data thread
		st_scan_data,
		/// display the eula and wait for validation
		st_display_eula,
		/// check the data to determine need for patch
		st_check_patch,
		/// display the list of optional patch category for patching
		st_display_cat,
		/// run the patch process and display progress
		st_patch,
		/// terminate the client and quit
		st_close_client,
		/// display the reboot screen and wait validation
		st_reboot_screen,
		/// restart the client with login bypass params
		st_restart_client,
		/// connect to the FS (start the 'in game' mode)
		st_connect,
		/// show the outgame browser
		st_browser_screen,
		/// ingame state
		st_ingame,
		/// leave the current shard (the exit action progress, Far TP part 1.1; Server Hop part 1-2)
		st_leave_shard,
		/// let the main loop finish the current frame and leave it (Far TP part 1.2)
		st_enter_far_tp_main_loop,
		/// disconnect from the FS (Far TP part 2)
		st_disconnect,
		/// connect to a new FS (Far TP & Server Hop part 3.1)
		st_reconnect_fs,
		/// after reconnecting, bypass character selection ui & select the same character (Far TP & Server Hop part 3.2)
		st_reconnect_select_char,
		/// after reconnecting and receiving ready, send ready (Far TP part 3.3)
		st_reconnect_ready,
		/// between global menu exit and sending ready (Server Hop part 3.3)
		st_exit_global_menu,
		/// error while reconnecting
		st_reconnect_error,
		/// Rate a ring session. should pop a web windows pointing the rate session page
		st_rate_session,
		/// create account
		st_create_account,
		/// pseudo state to leave the state machine
		st_end,
		///
		st_unknown
	};

	/// For displaying
	static const std::string& toString(TState state);

	enum TEvent
	{
		/// the client click the 'quit' button
		ev_quit,
		/// the client is connecting without authentication (dev mode)
		ev_skip_all_login,
		/// the init is done
		ev_init_done,
		/// the client push the 'config' button
		ev_game_conf,
		/// the client push the 'scan data' button
		ev_data_scan,
		/// the client push the 'scan data' button
		ev_close_data_scan,
		/// the login is validated and cookie is received
		ev_login_ok,
		/// the login check as failed
		ev_bad_login,
		/// the client has selected a shard
		ev_shard_selected,
		/// the patch checker says we need to ev_patch_needed
		ev_patch_needed,
		/// the patch check says we are clean, no patch needed
		ev_no_patch,
		/// the user validate the category screen, patch can begin
		ev_run_patch,
		/// the client validate the patch result
		ev_close_patch,
		/// Client accept the eula screen
		ev_accept_eula,
		/// Client decline the eula screen
		ev_decline_eula,
		/// the client valide the reboot message box
		ev_reboot,
		/// the ingame session terminates (CONNECTION:SERVER_QUIT_OK msg)
		ev_ingame_return,
		/// the far tp main loop is entered
		ev_far_tp_main_loop_entered,
		/// the lua script (from web usually) starts the client connection to a shard
		ev_connect,
		/// the connection fail for some reason
		ev_conn_failed,
		/// the connection was dropped by the server
		ev_conn_dropped,
		/// the client enters the game main loop
		ev_enter_game,
		// a self reconnetion is complete, get back to outgame menu
		ev_self_reconnected,
		/// characters received (CONNECTION:USER_CHARS msg)
		ev_chars_received,
		/// characters received but empty (CONNECTION:NO_USER_CHAR msg)
		ev_no_user_char,
		/// ready received (CONNECTION:READY msg)
		ev_ready_received,
		/// reselect character (CONNECTION:RECO
// 		ev_reselect_char,
		/// reconnect ok received (CONNECTION:SERVER_RECONNECT_OK msg)
		ev_reconnect_ok_received,
		/// global menu exited
		ev_global_menu_exited,
		/// The client click the 'relog' button
		ev_relog,
		/// the client push the 'create account' button
		ev_create_account,
		/// the client push the 'create account' button
		ev_close_create_account,
		///
		ev_unknown
	};

	/// For displaying
	static const std::string& toString(TEvent event);


private:
	TState				_CurrentState;
	std::list<TEvent>	_NextEvents;
public:

	// Create the state machine with the wanted start.
	// Note that, because it is a co-task, restarting the state machine after it has finished
	// require to delete then recreate this object
	CLoginStateMachine() :
		_CurrentState(st_start)
	{

	}

	TState getCurrentState()
	{
		return _CurrentState;
	}

	void pushEvent(TEvent eventId);

	TEvent waitEvent();


private:

	void run();
};

extern CLoginStateMachine LoginSM;

void initLoginScreen();
void initShardDisplay();
void initDataScan();
bool initCreateAccount();
void initEula();
void initPatchCheck();
void initCatDisplay();
void initAutoLogin();
void initPatch();
//void initWebBrowser();
void initReboot();

void ConnectToShard();

void waitForUserCharReceived();


/**
 */
class CFarTP
{
public:

	// See definition of _URLTable for url binding
	enum TJoinMode
	{
		LaunchEditor,
		JoinSession,
		JoinMainland,
		NbJoinModes
	};

	CFarTP() : _Reason(NULL), _SessionIdToJoinFast(0), _InGameMode(false), _PreviousR2EdEnabled(false), _ReselectingChar(false), _IsServerHopEmbedded(false), _HookedForEditor(false), _DSSDown(false) /*, _SelfReconnecting(false)*/ {}

	// Get the address of a front-end service via http, then begin Far TP
	bool requestFarTPToSession(TSessionId sessionId, uint8 charSlot, CFarTP::TJoinMode joinMode, bool bailOutIfSessionVanished);

	// Ask the EGS to pop the top position if applicable
	void requestReturnToPreviousSession(TSessionId rejectedSessionId=TSessionId(0));

	// Store the result of a JoinSession request for the next disconnection request
	void setJoinSessionResult(TSessionId sessionId, const CSecurityCode& securityCode);

	// Tell to ignore the next Far TP request, it will be replaced by a Far TP to the edition session (allows to shortcut anim-mainland-edition by anim-edition)
	void hookNextFarTPForEditor();

	/// Begin a Character Reselect sequence
	void requestReconnection();

	// These states can overlap
	bool isIngame() const { return _InGameMode; }
	bool isFarTPInProgress() const;
	bool isServerHopInProgress() const;
	bool isLeavingEGS() const { return (LoginSM.getCurrentState() == CLoginStateMachine::st_leave_shard); }
	bool isLeavingShard() const; // from begin of Far TP to disconnection
	bool isJoiningShard() const; // from reconnection to game entering
	bool isReselectingChar() const { return _ReselectingChar; }
	bool isServerHopEmbedded() const { return _IsServerHopEmbedded; }
	bool isFastDisconnectGranted() const { return _SessionIdToJoinFast != 0; }

	// Far TP from the main loop
	void farTPmainLoop();
	void writeSecurityCodeForDisconnection(NLMISC::IStream& msgout);
	void onServerQuitOk();
	void onServerQuitAbort();
	void disconnectFromPreviousShard();
	void connectToNewShard();
	bool reselectCharacter();
	void selectCharAndEnter();
	void sendReady();
	void onFailure();
	void onDssDown(bool forceReturn=false);

	void setIngame() { _InGameMode = true; }
	void setOutgame() { _InGameMode = false; }
	void setMainLoopEntered() { setIngame(); _ReselectingChar = false; }
	void beginEmbeddedServerHop() { _IsServerHopEmbedded = true; setOutgame(); _ReselectingChar = false; }
	void endEmbeddedServerHop() { _IsServerHopEmbedded = false; setIngame(); _ReselectingChar = true; }

	void setURLBase(const std::string& urlBase) { _URLBase = urlBase; }

private:

	// called back from session browser after a far tp request
	void joinSessionResult(uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const std::string &participantStatus);


	std::string *_Reason;

	static const char *_URLTable[NbJoinModes];

	std::string _URLBase;

	TSessionId _SessionIdToJoinFast; // if non 0, fast disconnection allowed to session
	CSecurityCode _SecurityCodeForDisconnection;

	bool _InGameMode; // formerly known as EnterWorld
	bool _PreviousR2EdEnabled;
	bool _ReselectingChar;
	bool _IsServerHopEmbedded; // allows to embed a server hop inside a char reselect (e.g. click on Launch Editor of PLAY with a character on a remote shard)
	bool _HookedForEditor; // allows to Far TP to editor instead of following the next Far TP request
	bool _DSSDown; // remembers a DSSDown event if occurs during a FarTP
//	bool _SelfReconnecting; // allows to disconnect after character selection and reconnect until the player can choose another character

};

extern CFarTP FarTP;

#endif // FAR_TP_H
