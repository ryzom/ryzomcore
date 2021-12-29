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


#ifndef SHARD_NAMES_H
#define SHARD_NAMES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/singleton.h"
#include "game_share/r2_basic_types.h"


/** This class provide a standard way to deal with shard names.
 *	It allow to convert from shardID to shard name and vice versa
 *	as well as to build or parse full character name (ie. character names
 *	that include the shard name).
 */
class CShardNames : public NLMISC::CSingleton<CShardNames>
{
public:
	struct TSessionName
	{
		/// The home mainland session Id for this shard
		TSessionId			SessionId;
		/// Display name, as displayed in user interface and appended to player character names
		std::string			DisplayName;
		/// pre mapped name (in string mapper)
		NLMISC::TStringId	DisplayNameId;
		/// short name used in user commands like "/tell [<shortName>.]<userName>"
		std::string			ShortName;
	};

	// This container is just a vector because it is very small and brute force parsing will be faster
	typedef std::vector<TSessionName>	TSessionNames;
private:
	/// Table of home session names
	TSessionNames	_SessionNames;

	/// Typically For Shard when no SU, do not append () in makeFullName() if the session is not found
	bool			_AppendParenthesisWhenSessionNotFound;

public:

	/** ctor */
	CShardNames()
	{
		_AppendParenthesisWhenSessionNotFound= true;
	}

	/** init the shard names by reading the content of "HomeMainlandNames" var
	*/
	void init(NLMISC::CConfigFile &configFile);

	/** Build as a vector of string, for serial or message of the shardname configuration
	*/
	void saveShardNames(std::vector<std::string> &outData) const;

	/** Load as a vector of string, for serial or message of the shardname configuration
	*/
	void loadShardNames(const std::vector<std::string> &inData);

	/** Return the vector of names session */
	const TSessionNames &getSessionNames() const
	{
		return _SessionNames;
	}

	/** Return the name of the shard, empty string if no match is found */
	const std::string &getShardName(TSessionId shardId);

	/** Return the index in the shard names table of the shard. Return 0xffffffff if shard is not found */
	uint32 getShardIndex(TSessionId shardId);

	/** Return the shard/session id from i the shard name. Return 0 if no match is found*/
	TSessionId getShardId(const std::string &shardName);

	/** Parse the submitted character name using the submitted context and
	 *	return the deduced character short name and character home session Id
	 *	Input name can be :
	 *		- a short name, session id is deduced to be the same as context session id
	 *		- a located name (<shortShardName>.<characterName> : the session is
	 *			is deduced from the short shard name
	 *		- a full name (<characterName>(<shardName>) : the session is deduced
	 *			from the shard name.
	 */
	void parseRelativeName(TSessionId contextSessionId, const std::string &inputCharName, std::string &outCharName, TSessionId &outSessionId);

	/** Build a full player name according to it's name and home session id
	 *	The returned name as the following format :
	 *		<characterName>'('<sessionName>')'
	 *	If no shard name is found for the given session id, then
	 *	an empty parenthesys pair is returned (e.g. "toto()" )
	 */
	std::string makeFullName(const std::string &charName, TSessionId homeSessionId);

	/** Build a full name by parsing a relative name (a helper that make
	 *	use of parseRelativeName() and makeFullName() to build
	 *	a full name from a relative name in only 1 call.
	 */
	std::string makeFullNameFromRelative(TSessionId contextSessionId, const std::string &inputcharName);

};



#endif // SHARD_NAMES_H
