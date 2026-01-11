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




#ifndef NL_DEBUG_CLIENT_H
#define NL_DEBUG_CLIENT_H


/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
// Game Share
#include "game_share/entity_types.h"


////////////
// GLOBAL //
////////////
// Verbose enum
enum TVerbose
{
	VerboseNone   = 0x00000000,
	VerboseMagic  = 0x00000001,
	VerboseAnim   = 0x00000002,
	VerboseAll    = 0xFFFFFFFF
};
extern uint32 Verbose;
extern bool VerboseAnimSelection;
extern bool VerboseAnimUser;
extern bool VerboseVP;
extern CLFECOMMON::TCLEntityId WatchedEntitySlot;
extern uint64 IngameEnterTime;


///////////////
// FUNCTIONS //
///////////////
void initDebugMemory();
double memoryUsedSinceLastCall();

/// Set an output file to log debugs. (empty to as nlwarning).
void setDebugOutput(const std::string &filename);
/// Push a string in a debug stack but will display only if there is a debug string in the stack when flush.
void pushInfoStr(const std::string &str);
/// Push a string in a debug stack.
void pushDebugStr(const std::string &str);
/// Display 'title' and a warning for each element in the Debug Stack.
void flushDebugStack(const std::string &title);

/**
 * <Class description>
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CDebugClient
{
public:

	/// Constructor
	CDebugClient();

};

// Display streaming debug page
void displayStreamingDebug ();

// Display Net debug page
void displayNetDebug ();


// return a string with \n separated to get information about the client at the moment 't'
// it is used by the bug_report and when the client crashs
std::string getDebugInformation();

// Start the ingame time counter
void resetIngameTime ();

// Get ingame time
uint64 ingameTime0 ();
uint64 ingameTime1 ();

// Must verbose VP?
bool verboseVPAdvanceTest(class CEntityCL *en, uint32 form);
inline bool verboseVP(class CEntityCL * /* en */, uint32 /* form */=0)
{
	return VerboseVP;
	// TestYoyo
	//return verboseVPAdvanceTest(en, form);
}

// Debug report
void	crashLogAddServerHopEvent();
void	crashLogAddFarTpEvent();
void	crashLogAddReselectPersoEvent();

#endif // NL_DEBUG_CLIENT_H

/* End of debug_client.h */
