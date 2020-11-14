#ifndef SIMULATED_CLIENT_H
#define SIMULATED_CLIENT_H

#include "entities_spoof.h"
#include "nel/misc/vector.h"
#include "nel/misc/command.h"
#include <vector>
#include <stack>
#include <string>

class CUserCharMsg;

namespace NLNET
{
	class IGatewayTransport;
};

const uint DISABLED_CLIENT = ~0;

/**
 * An entity in the neighbourhood
 */
class CEntityDrone
{
public:
	CEntityDrone() {}
};


/**
 * A light-weight client simulated by the drone
 */
class CSimulatedClient :
	public NLMISC::ICommandsHandler
{
public:

	enum TLoginState
	{
		LSInitial,
		LSUserCharsReceiving,
		LSUserCharsReceived,
		LSNoUserChar,
		LSClientReady,
		LSQuitRequested,
		LSQuitting
	};

	enum TDirection
	{
		DNE, DSE, DSW, DNW, DMaxDir=DNW
	};

	// Main controls
	static void initNetwork(); // init network (call only once)
	CSimulatedClient( uint id=DISABLED_CLIENT );
	virtual ~CSimulatedClient();
	bool start();
	bool update();
	void requestQuit(); // can be called several times

	// Utilities
	static CSimulatedClient *const currentContext() { return _CurrentContext; } // context for non-OO functions (callbacks...)
	std::string name() const;
	virtual const std::string &getCommandHandlerName() const { return _Name; }
	void setCurrentLoginState( TLoginState ls ) { if ( _CurrentLoginState < LSQuitRequested) _CurrentLoginState = ls; }
	void requestCreateChar();
	void requestCommandA( const std::string& arg );
	TLoginState getCurrentLoginState() const { return _CurrentLoginState; }
	void setInitPos( NLMISC::CBitMemStream &impulse );
	void tp( const NLMISC::CVectorD& dest, bool ackTP );
	void setId( uint id ) { _Id = id; }
	uint id() const { return _Id; }
	void sendClientReady();
	void disconnect();
	void requestString(uint32 stringId);


	CNetworkConnection	&getNetworkConnection() { return _NetworkConnection;}

	void setGatewayTransport(NLNET::IGatewayTransport *transport){_GatewayTransport = transport; }
	NLNET::IGatewayTransport *getGatewayTransport() { return _GatewayTransport; }

	// Return a valid pointer if the slot is assigned, otherwise NULL
	CEntityDrone*	getSlot( CLFECOMMON::TCLEntityId slot );

	// Return the value of a visual property
	sint64			getVisualProp( CLFECOMMON::TCLEntityId slot, CLFECOMMON::TPropIndex prop ) const { return _NetworkConnection.getVisualProp( slot, prop ); }

	// Return the position of an entity in sight
	NLMISC::CVectorD getPosition( CLFECOMMON::TCLEntityId slot ) const;

	// Return the slot of the nearest entity (or ~0 if there is no other entity). If requiredSheet!=Unknown, only entities matching the sheet are taken in account.
	CLFECOMMON::TCLEntityId	getNearestEntity( NLMISC::CSheetId requiredSheet=NLMISC::CSheetId::Unknown ) const;

	// User entity
	CUserEntity UserEntity;

	NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CSimulatedClient)

			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, getNbUsedSlots, "Store the number of entities in vision", "")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, getNearestEntity, "Store the slot of the nearest entity, with optional positive sheet filter", "[<onlySheet>]")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, getVisualProp, "Store a visual property value", "<slot> <propIndex>")

			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, a, "Execute an admin command on you","<cmd> <arg>")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, a_value, "Execute an admin command on you, take stored value as argument","<cmd> <arg>")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, target, "Target a specified (or stored slot) ", "[<slot>]")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, continueMission, "Click the current target with an Auto Mission", "[<intId>]")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, sleep, "Sleep for seconds", "<seconds>")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, sleepUntil, "Sleep until NLMISC::getLocalTime() reaches until", "<until>")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, repeat, "Repeat the block going until endrepeat", "<count>")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, displayScript, "Display the current script list", "")
			NLMISC_COMMAND_HANDLER_ADD(CSimulatedClient, disconnect, "Request disconnection", "")

	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(getNbUsedSlots);
	NLMISC_CLASS_COMMAND_DECL(getNearestEntity);
	NLMISC_CLASS_COMMAND_DECL(getVisualProp);

	NLMISC_CLASS_COMMAND_DECL(a);
	NLMISC_CLASS_COMMAND_DECL(a_value);
	NLMISC_CLASS_COMMAND_DECL(target);
	NLMISC_CLASS_COMMAND_DECL(continueMission);
	NLMISC_CLASS_COMMAND_DECL(sleep);
	NLMISC_CLASS_COMMAND_DECL(repeat);
	NLMISC_CLASS_COMMAND_DECL(sleepUntil);
	NLMISC_CLASS_COMMAND_DECL(displayScript);
	NLMISC_CLASS_COMMAND_DECL(disconnect);

	
protected:

	friend class CClientDrone; // accessing stats

	void setIdAndName(uint id);
	bool autoLogin (const std::string &cookie, const std::string &fsaddr, bool firstConnection);
	bool updateEnterGame();
	bool updateInGame();

	void send( NLMISC::TGameCycle gameCycle );
	void send();
	sint updateNetwork(); // return the game cycle delta between the previous call
	void processVision();
	void processScript();
	void sendMsgToServer(const std::string &sMsg, uint8 u8);

	void selectCharAndEnter( uint charIndex );


protected:
	std::string				_Name;
	uint					_Id;
	uint					_UserId;
	CNetworkConnection		_NetworkConnection;
	TLoginState				_CurrentLoginState;
	//CCDBSynchronised		_IngameDbMngr;
	TDirection				_Direction;
	std::vector<std::string> _PendingCommands;
	CEntityDrone			_VisionSlots[256]; // see getSlot()
//	CLFECOMMON::TCLEntityId	_NearestSlot;
//	uint					_NbUsedSlots;
	typedef std::list<std::string> CScriptList;
	CScriptList				_Script;
	bool					_ProcessingScript;

	static CSimulatedClient *_CurrentContext;
	NLNET::TModulePtr			_ModuleGateway;
	NLNET::IGatewayTransport	*_GatewayTransport;

private:
	NLNET::TModulePtr	_ModuleTest;

};


/*
 *
 */
class CScriptStack
{
public:
	static CScriptStack * getInstance() { if (!_Instance) _Instance = new CScriptStack(); return _Instance; }

	NLMISC::CSString&		top() { if (_Stack.empty()) _Stack.push(NLMISC::CSString()); return _Stack.top(); }

private:
	std::stack<NLMISC::CSString>		_Stack;
	static CScriptStack					*_Instance;
};

#endif	// SIMULATED_CLIENT_H
