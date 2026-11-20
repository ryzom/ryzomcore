// Ryzom - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef CL_CLIENT_CTRL_H
#define CL_CLIENT_CTRL_H

#include "nel/misc/types_nl.h"

#include <string>

namespace NLMISC {
class CBitMemStream;
class CCmdArgs;
class CCtrlEndpoint;
class CJsonValue;
}

/**
 * The control endpoint of the client: with --ctrl-listen=<name> on the command line, the client
 * listens on a local pipe, and a tool on this machine (the MCP front, nel_mcp) drives and
 * inspects it through it. See nel/misc/ctrl_endpoint.h for the channel and who can open it.
 *
 * Whoever opens the endpoint acts as the user: it runs Lua with what the interface's Lua state
 * has, and commands. So it exists only in builds that are not final, or with the build option
 * WITH_RYZOM_CLIENT_CTRL, and only the command line switches it on: not client.cfg, which is
 * re-read while the client runs and comes with patches, nor a variable or a command, which
 * the server (REMOTE_ADMIN) and Lua (runCommand) can reach. Without it, these do nothing.
 */
#if !FINAL_VERSION || defined(RYZOM_CLIENT_CTRL)
#define RYZOM_HAS_CLIENT_CTRL 1
#endif

namespace ClientCtrl {

#ifdef RYZOM_HAS_CLIENT_CTRL

/// Declares --ctrl-listen
void addArgs(NLMISC::CCmdArgs &args);
/// Starts the endpoint when the command line asks for it; after the log is set up
void start(const NLMISC::CCmdArgs &args);
bool isActive();
/// A tool holds the lease of the endpoint: the client takes its input as if it had the focus, and is not
/// paced for the lack of it (a client on a desktop that is not shown never has the focus)
bool isDriven();
NLMISC::CCtrlEndpoint *getEndpoint();

/// Serves the calls, from the event pump of every loop of the client, after the events are pumped.
/// interfaceAvailable is false in the phases that skip the interface manager (loading screens).
void update(bool interfaceAvailable);
/// Whether the interface handles events in the current pump
bool isInterfaceAvailable();
/// At every place a frame is shown, just before the swap
void beforeSwap();
/// In the main loop, when the scene is drawn and the interface is not yet
void sceneRendered();
/// The phase of the client: login, connecting, character_selection, entering_game, in_game, leaving_game
void setPhase(const char *phase);
const std::string &getPhase();
/// Why the client is about to exit, for the record of the endpoint
void setExitReason(const std::string &reason);
/// Why the client may exit soon: an exit that follows gives this reason, but the state does not turn to quitting
void setRequestedExitReason(const std::string &reason);
/// Stops the endpoint with the reason it exits for; generic for the ordinary end, which a quit a tool asked for explains
void release(const std::string &reason, bool generic = false);

/// A system message, as the chat shows it
void showSystemInfo(const std::string &category, const std::string &text);
/// An impulse of msg.xml received (before its handler) or sent (as it is pushed), for net.record
void netImpulse(bool out, const NLMISC::CBitMemStream &message);
void addEvent(const std::string &kind, const NLMISC::CJsonValue &data);
/// Identity values of the record: the account, the shard, the character
void setIdentity(const std::string &key, const std::string &value);

/// Around the event pump of a loading screen: the state is loading, with its message
class CLoadingScope
{
public:
	CLoadingScope(const std::string &message, float value);
	~CLoadingScope();
};

#else

inline void addArgs(NLMISC::CCmdArgs &) { }
inline void start(const NLMISC::CCmdArgs &) { }
inline bool isActive() { return false; }
inline bool isDriven() { return false; }
inline NLMISC::CCtrlEndpoint *getEndpoint() { return nullptr; }
inline void update(bool) { }
inline bool isInterfaceAvailable() { return true; }
inline void beforeSwap() { }
inline void sceneRendered() { }
inline void setPhase(const char *) { }
inline void setExitReason(const std::string &) { }
inline void release(const std::string &, bool = false) { }
inline void showSystemInfo(const std::string &, const std::string &) { }
inline void netImpulse(bool, const NLMISC::CBitMemStream &) { }
inline void addEvent(const std::string &, const NLMISC::CJsonValue &) { }
inline void setIdentity(const std::string &, const std::string &) { }

class CLoadingScope
{
public:
	CLoadingScope(const std::string &, float) { }
};

#endif

} // ClientCtrl

#endif // CL_CLIENT_CTRL_H

/* End of client_ctrl.h */
