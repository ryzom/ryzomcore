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

#ifdef RZ_USE_STEAM

#include "steam_client.h"

#include <steam_api.h>

// prototypes definitions for Steam API functions we'll call
typedef bool			(__cdecl *SteamAPI_InitFuncPtr)();
typedef void			(__cdecl *SteamAPI_ShutdownFuncPtr)();
typedef ISteamClient*	(__cdecl *SteamClientFuncPtr)();
typedef ISteamFriends*	(__cdecl *SteamFriendsFuncPtr)();
typedef ISteamUser*		(__cdecl *SteamUserFuncPtr)();
typedef ISteamUtils*	(__cdecl *SteamUtilsFuncPtr)();
typedef void			(__cdecl *SteamAPI_RegisterCallbackFuncPtr)(class CCallbackBase *pCallback, int iCallback);
typedef void			(__cdecl *SteamAPI_UnregisterCallbackFuncPtr)(class CCallbackBase *pCallback);
typedef void			(__cdecl *SteamAPI_RunCallbacksFuncPtr)();

// macros to simplify dynamic functions loading
#define NL_DECLARE_SYMBOL(symbol) symbol##FuncPtr nl##symbol = NULL
#define NL_LOAD_SYMBOL(symbol) nl##symbol = (symbol##FuncPtr)NLMISC::nlGetSymbolAddress(_Handle, #symbol)

NL_DECLARE_SYMBOL(SteamAPI_Init);
NL_DECLARE_SYMBOL(SteamAPI_Shutdown);
NL_DECLARE_SYMBOL(SteamClient);
NL_DECLARE_SYMBOL(SteamFriends);
NL_DECLARE_SYMBOL(SteamUser);
NL_DECLARE_SYMBOL(SteamUtils);
NL_DECLARE_SYMBOL(SteamAPI_RegisterCallback);
NL_DECLARE_SYMBOL(SteamAPI_UnregisterCallback);
NL_DECLARE_SYMBOL(SteamAPI_RunCallbacks);

// taken from steam_api.h, we needed to change it to use our dynamically loaded functions

// Declares a callback member function plus a helper member variable which
// registers the callback on object creation and unregisters on destruction.
// The optional fourth 'var' param exists only for backwards-compatibility
// and can be ignored.
#define NL_STEAM_CALLBACK( thisclass, func, .../*callback_type, [deprecated] var*/ ) \
	_NL_STEAM_CALLBACK_SELECT( ( __VA_ARGS__, 4, 3 ), ( /**/, thisclass, func, __VA_ARGS__ ) )

//-----------------------------------------------------------------------------
// The following macros are implementation details, not intended for public use
//-----------------------------------------------------------------------------
#define _NL_STEAM_CALLBACK_AUTO_HOOK( thisclass, func, param )
#define _NL_STEAM_CALLBACK_HELPER( _1, _2, SELECTED, ... )		_NL_STEAM_CALLBACK_##SELECTED
#define _NL_STEAM_CALLBACK_SELECT( X, Y )						_NL_STEAM_CALLBACK_HELPER X Y
#define _NL_STEAM_CALLBACK_3( extra_code, thisclass, func, param ) \
	struct CCallbackInternal_ ## func : private CSteamCallbackImpl< sizeof( param ) > { \
		CCallbackInternal_ ## func () { extra_code nlSteamAPI_RegisterCallback( this, param::k_iCallback ); } \
		CCallbackInternal_ ## func ( const CCallbackInternal_ ## func & ) { extra_code nlSteamAPI_RegisterCallback( this, param::k_iCallback ); } \
		CCallbackInternal_ ## func & operator=( const CCallbackInternal_ ## func & ) { return *this; } \
		private: virtual void Run( void *pvParam ) { _NL_STEAM_CALLBACK_AUTO_HOOK( thisclass, func, param ) \
			thisclass *pOuter = reinterpret_cast<thisclass*>( reinterpret_cast<char*>(this) - offsetof( thisclass, m_steamcallback_ ## func ) ); \
			pOuter->func( reinterpret_cast<param*>( pvParam ) ); \
		} \
	} m_steamcallback_ ## func ; void func( param *pParam )
#define _NL_STEAM_CALLBACK_4( _, thisclass, func, param, var ) \
	CSteamCallback< thisclass, param > var; void func( param *pParam )

//-----------------------------------------------------------------------------
// Purpose: templated base for callbacks - internal implementation detail
//-----------------------------------------------------------------------------
template< int sizeof_P >
class CSteamCallbackImpl : protected CCallbackBase
{
public:
	~CSteamCallbackImpl() { if ( m_nCallbackFlags & k_ECallbackFlagsRegistered ) nlSteamAPI_UnregisterCallback( this ); }
	void SetGameserverFlag() { m_nCallbackFlags |= k_ECallbackFlagsGameServer; }

protected:
	virtual void Run( void *pvParam ) = 0;
	virtual void Run( void *pvParam, bool /*bIOFailure*/, SteamAPICall_t /*hSteamAPICall*/ ) { Run( pvParam ); }
	virtual int GetCallbackSizeBytes() { return sizeof_P; }
};

//-----------------------------------------------------------------------------
// Purpose: maps a steam callback to a class member function
//			template params: T = local class, P = parameter struct,
//			bGameserver = listen for gameserver callbacks instead of client callbacks
//-----------------------------------------------------------------------------
template< class T, class P, bool bGameserver = false >
class CSteamCallback : public CSteamCallbackImpl< sizeof( P ) >
{
public:
	typedef void (T::*func_t)(P*);

	// NOTE: If you can't provide the correct parameters at construction time, you should
	// use the CCallbackManual callback object (STEAM_CALLBACK_MANUAL macro) instead.
	CSteamCallback( T *pObj, func_t func ) : m_pObj( NULL ), m_Func( NULL )
	{
		if ( bGameserver )
		{
			this->SetGameserverFlag();
		}
		Register( pObj, func );
	}

	// manual registration of the callback
	void Register( T *pObj, func_t func )
	{
		if ( !pObj || !func )
			return;

		if ( this->m_nCallbackFlags & CCallbackBase::k_ECallbackFlagsRegistered )
			Unregister();

		m_pObj = pObj;
		m_Func = func;
		// SteamAPI_RegisterCallback sets k_ECallbackFlagsRegistered
		nlSteamAPI_RegisterCallback( this, P::k_iCallback );
	}

	void Unregister()
	{
		// SteamAPI_UnregisterCallback removes k_ECallbackFlagsRegistered
		nlSteamAPI_UnregisterCallback( this );
	}

protected:
	virtual void Run( void *pvParam )
	{
		(m_pObj->*m_Func)( (P *)pvParam );
	}

	T *m_pObj;
	func_t m_Func;
};

// listener called by Steam when AuthSessionTicket is available
class CAuthSessionTicketListener
{
public:
	CAuthSessionTicketListener():_AuthSessionTicketResponse(this, &CAuthSessionTicketListener::OnAuthSessionTicketResponse)
	{
		_AuthSessionTicketHandle = 0;
		_AuthSessionTicketSize = 0;

		_AuthSessionTicketCallbackCalled = false;
		_AuthSessionTicketCallbackError = false;;
		_AuthSessionTicketCallbackTimeout = false;
	}

	// wait until a ticket is available or return if no ticket received after specified ms
	bool waitTicket(uint32 ms)
	{
		// call Steam method
		_AuthSessionTicketHandle = nlSteamUser()->GetAuthSessionTicket(_AuthSessionTicketData, sizeof(_AuthSessionTicketData), &_AuthSessionTicketSize);

		nldebug("GetAuthSessionTicket returned %u bytes, handle %u", _AuthSessionTicketSize, _AuthSessionTicketHandle);

		nlinfo("Waiting for Steam GetAuthSessionTicket callback...");

		// define expiration time
		NLMISC::TTime expirationTime = NLMISC::CTime::getLocalTime() + ms;

		// wait until callback method is called or expiration
		while(!_AuthSessionTicketCallbackCalled && !_AuthSessionTicketCallbackTimeout)
		{
			// call registered callbacks
			nlSteamAPI_RunCallbacks();

			// check if expired
			if (NLMISC::CTime::getLocalTime() > expirationTime)
				_AuthSessionTicketCallbackTimeout = true;
		}

		// expired
		if (_AuthSessionTicketCallbackTimeout)
		{
			nlwarning("GetAuthSessionTicket callback never called");
			return false;
		}

		nlinfo("GetAuthSessionTicket called");

		// got an error
		if (_AuthSessionTicketCallbackError)
		{
			nlwarning("GetAuthSessionTicket callback returned error");
			return false;
		}

		return true;
	}

	// return ticket if available in hexadecimal
	std::string getTicket() const
	{
		// if expired or error, ticket is not available
		if (!_AuthSessionTicketCallbackCalled || _AuthSessionTicketCallbackError || _AuthSessionTicketCallbackTimeout) return "";

		std::string authSessionTicket;

		// optimize string by allocating the final string size
		authSessionTicket.reserve(_AuthSessionTicketSize*2);

		// convert buffer to hexadecimal string
		for (uint32 i = 0; i < _AuthSessionTicketSize; ++i)
		{
			authSessionTicket += NLMISC::toString("%02x", _AuthSessionTicketData[i]);
		}

		return authSessionTicket;
	}

private:
	// ticket handle
	HAuthTicket _AuthSessionTicketHandle;

	// buffer of ticket data
	uint8 _AuthSessionTicketData[1024];

	// size of buffer
	uint32 _AuthSessionTicketSize;

	// different states of callback
	bool _AuthSessionTicketCallbackCalled;
	bool _AuthSessionTicketCallbackError;
	bool _AuthSessionTicketCallbackTimeout;

	// callback declaration
	NL_STEAM_CALLBACK(CAuthSessionTicketListener, OnAuthSessionTicketResponse, GetAuthSessionTicketResponse_t, _AuthSessionTicketResponse);
};

// method called by Steam
void CAuthSessionTicketListener::OnAuthSessionTicketResponse(GetAuthSessionTicketResponse_t *inCallback)
{
	_AuthSessionTicketCallbackCalled = true;

	if (inCallback->m_eResult != k_EResultOK)
	{
		_AuthSessionTicketCallbackError = true;
	}
}        

CSteamClient::CSteamClient():_Handle(NULL), _Initialized(false)
{
}

CSteamClient::~CSteamClient()
{
	release();
}

static void SteamWarningMessageHook(int severity, const char *message)
{
	switch(severity)
	{
		case 1: // warning
		nlwarning("%s", message);
		break;

		case 0: // message
		nlinfo("%s", message);
		break;

		default: // unknown
		nlwarning("Unknown severity %d: %s", severity, message);
		break;
	}
}

bool CSteamClient::init()
{
	std::string filename;

#if defined(NL_OS_WIN64)
	filename = "steam_api64.dll";
#elif defined(NL_OS_WINDOWS)
	filename = "steam_api.dll";
#elif defined(NL_OS_MAC)
	filename = "libsteam_api.dylib";
#else
	filename = "libsteam_api.so";
#endif

	// try to load library
	_Handle = NLMISC::nlLoadLibrary(filename);

	if (!_Handle)
	{
		nlwarning("Unable to load Steam client");
		return false;
	}

	// load Steam functions
	NL_LOAD_SYMBOL(SteamAPI_Init);
	NL_LOAD_SYMBOL(SteamAPI_Shutdown);

	// check if function was found
	if (!nlSteamAPI_Init)
	{
		nlwarning("Unable to get a pointer on SteamAPI_Init");
		return false;
	}

	// initialize Steam API
	if (!nlSteamAPI_Init())
	{
		nlwarning("Unable to initialize Steam client");
		return false;
	}

	_Initialized = true;

	// load more Steam functions
	NL_LOAD_SYMBOL(SteamClient);
	NL_LOAD_SYMBOL(SteamFriends);
	NL_LOAD_SYMBOL(SteamUser);
	NL_LOAD_SYMBOL(SteamUtils);

	// set warning messages hook
	nlSteamClient()->SetWarningMessageHook(SteamWarningMessageHook);

	bool loggedOn = nlSteamUser()->BLoggedOn();

	nlinfo("Steam AppID: %u", nlSteamUtils()->GetAppID());
	nlinfo("Steam login: %s", nlSteamFriends()->GetPersonaName());
	nlinfo("Steam user logged: %s", loggedOn ? "yes":"no");

	// don't need to continue, if not connected
	if (!loggedOn) return false;

	// load symbols used by AuthSessionTicket
	NL_LOAD_SYMBOL(SteamAPI_RegisterCallback);
	NL_LOAD_SYMBOL(SteamAPI_UnregisterCallback);
	NL_LOAD_SYMBOL(SteamAPI_RunCallbacks);

	CAuthSessionTicketListener listener;

	// wait 5 seconds to get ticket
	if (!listener.waitTicket(5000)) return false;

	// save ticket
	_AuthSessionTicket = listener.getTicket();

	nldebug("Auth ticket: %s", _AuthSessionTicket.c_str());

	return true;
}

bool CSteamClient::release()
{
	if (!_Handle) return false;

	if (_Initialized)
	{
		// only shutdown Steam if initialized
		nlSteamAPI_Shutdown();

		_Initialized = false;
	}

	// free Steam library from memory
	bool res = NLMISC::nlFreeLibrary(_Handle);

	_Handle = NULL;

	return res;
}


#endif
