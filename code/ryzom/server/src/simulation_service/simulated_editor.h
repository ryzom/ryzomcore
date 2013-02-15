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

#ifndef SIMULATED_EDITOR_H
#define SIMULATED_EDITOR_H

#include "../client_drone/simulated_client.h"
#include "simulated_dmc.h"

class R2::CDynamicMapClient;
class R2::CObject;

// AJM in work
enum SimEditorState
{
	sesUninitialized = 0,
	sesInitialized,

	sesStartingSession,
	sesAwaitingSessionStart,
	sesSessionStarted,

	sesJoiningSession,
	sesAwaitingSessionJoin,
	sesSessionJoined,

	sesScheduledForLogin,
	sesLoggingIn,
	sesAwaitingLogin,
	sesLoggedIn,

	sesScheduledForConnection,
	sesConnecting,
	sesAwaitingConnection,
	sesConnected,

	sesLoadingScenario,
	sesScheduledForUpload,
	sesUploadingScenario,
	sesScenarioUploaded,

	sesRunningAsEditor,
	sesRunningAsAnimator,

	sesEndingScenario,
	sesJoiningScenario,

	sesNotRunning,
	sesQuitting,
	sesLASTSTATE
};

const std::string SimEditorStateNames[] =
{
	"Uninitialized",
	"Initialized",

	"StartingSession",
	"AwaitingSessionStart",
	"SessionStarted",

	"JoiningSession",
	"AwaitingSessionJoin",
	"SessionJoined",

	"ScheduledForLogin",
	"LoggingIn",
	"AwaitingLogin",
	"LoggedIn",

	"ScheduledForConnection",
	"Connecting",
	"AwaitingConnection",
	"Connected",

	"LoadingScenario",
	"ScheduledForUpload",
	"UploadingScenario",
	"ScenarioUploaded",

	"RunningAsEditor",
	"RunningAsAnimator",

	"EndingScenario",
	"JoiningScenario",
	
	"NotRunning",
	"Quitting",
	"Invalid State!"
};


class CSimulatedEditor : public CSimulatedClient
{
public:

	CSimulatedEditor( uint id=DISABLED_CLIENT );
	virtual ~CSimulatedEditor();

	static void initImpulseCallbacks();
	void init();
	void login();
	void connectToDSS();

	// console interface
	void test();
	void testCreateScenario( const std::string &fileName );
	void testRunScenario();

	// simulation service interface
	R2::CObject *loadScenarioStub( const std::string &filename );
	R2::CObject *loadRtData( const std::string &filename );
	uint getNumActs( R2::CObject *rtData );
	void uploadScenario( R2::CObject *pStub, R2::CObject *pRtData );
	void runScenario();	// start test
	void startAct( uint actId );
	void endScenario();	// stop test

	// are we logged in to the Frontend Service?
	bool isLoggedIn() /*const*/; // calls nonconst CSimulatedClient method

	// are we connected to the Dynamic Scenario Service?
	bool isConnected() const;

	// state machine
	SimEditorState getState() const	{ return _ses; }
	void setState( SimEditorState ses ) { _ses = ses; }

protected:
	R2::CObject *loadScenarioData( const std::string &fileName ) const;
	R2::CObject *loadScenarioRtData( const std::string &fileName ) const;
	R2::CObject *createScenarioStub( const R2::CObject *pData ) const;
	bool checkScenarioRtData( R2::CObject *rtData ) const;

private:
	R2::CDynamicMapClient		*_DMC;
	SimEditorState				 _ses;
	bool						 _bLoggedIn;
};
#endif	// SIMULATED_EDITOR_H
