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
#include "shard_names.h"

using namespace std;
using namespace NLMISC;


// ***************************************************************************
void CShardNames::init(NLMISC::CConfigFile &configFile)
{
	// read the mainland session name table
	CConfigFile::CVar *sessionNames = configFile.getVarPtr("HomeMainlandNames");
	if (sessionNames == NULL)
	{
		nlwarning("No variable 'HomeMainlandNames', domain unified character names will not work correctly !");
	}
	else
	{
		uint32 sessionId;

		for (uint i=0; i<sessionNames->size()/3; ++i)
		{
			TSessionName sn;
			fromString(sessionNames->asString(i*3), sessionId);
			sn.SessionId = (TSessionId)sessionId;
			sn.DisplayName = sessionNames->asString(i*3+1);
			sn.ShortName = sessionNames->asString(i*3+2);
			sn.DisplayNameId = CStringMapper::map(sn.DisplayName);


			_SessionNames.push_back(sn);
		}

		nlinfo("Read %u home session names from config files", _SessionNames.size());
	}

	// read the var to append or not (). Use DontUseSU one for simplicity (but this is for server only...)
	CConfigFile::CVar	*dontUseSU = configFile.getVarPtr("DontUseSU");
	_AppendParenthesisWhenSessionNotFound= true;	// default
	if(dontUseSU!=NULL)
	{
		// do not append () if DontUseSU==1
		_AppendParenthesisWhenSessionNotFound= dontUseSU->asInt()==0;
	}
}

// ***************************************************************************
void CShardNames::saveShardNames(std::vector<string> &outData) const
{
	// backup a vector<string> version of the data
	outData.resize(_SessionNames.size()*3);
	for(uint i=0;i<_SessionNames.size();i++)
	{
		const TSessionName &sn= _SessionNames[i];
		outData[i*3+0]= toString(sn.SessionId);
		outData[i*3+1]= sn.DisplayName;
		outData[i*3+2]= sn.ShortName;
	}
}

// ***************************************************************************
void CShardNames::loadShardNames(const std::vector<string> &inData)
{
	// reset
	_SessionNames.clear();

	uint32 sessionId;

	// parse the vector of string
	for (uint i=0; i<inData.size()/3; ++i)
	{
		TSessionName sn;
		fromString(inData[i*3+0], sessionId);
		sn.SessionId = (TSessionId)sessionId;
		sn.DisplayName = inData[i*3+1];
		sn.ShortName = inData[i*3+2];
		sn.DisplayNameId = CStringMapper::map(sn.DisplayName);

		_SessionNames.push_back(sn);
	}
}

// ***************************************************************************
const std::string &CShardNames::getShardName(TSessionId shardId)
{
	for (uint i=0; i<_SessionNames.size(); ++i)
	{
		if (_SessionNames[i].SessionId == shardId)
			return _SessionNames[i].DisplayName;
	}

	// not found
	static string empty;
	return empty;
}


/** Return the index in the shard names table of the shard. */
uint32 CShardNames::getShardIndex(TSessionId shardId)
{
	for (uint i=0; i<_SessionNames.size(); ++i)
	{
		if (_SessionNames[i].SessionId == shardId)
			return i;
	}

	// not found
	return 0xffffffff;
}

// ***************************************************************************
TSessionId CShardNames::getShardId(const std::string &shardName)
{
	for (uint i=0; i<_SessionNames.size(); ++i)
	{
		if (_SessionNames[i].DisplayName == shardName)
			return _SessionNames[i].SessionId;
	}

	// not found
	return TSessionId(0);
}

// ***************************************************************************
void CShardNames::parseRelativeName(TSessionId contextSessionId, const string &inputCharName, string &outCharName, TSessionId &outSessionId)
{
	// by default, if nothing match elsewhere, we return the context session id
	outSessionId = contextSessionId;

	string::size_type pos = inputCharName.find('.');
	if (pos != string::npos)
	{
		// we have a short name !
		string shortName = inputCharName.substr(0, pos);
		outCharName = inputCharName.substr(pos+1);

		// look in the session name table
		for (uint i=0; i<_SessionNames.size(); ++i)
		{
			if (nlstricmp(_SessionNames[i].ShortName, shortName) == 0)
			{
				outSessionId = _SessionNames[i].SessionId;
				break;
			}
		}
	}
	else
	{
		// no short name spec, check for a '('
		pos = inputCharName.find('(');
		if (pos == string::npos)
		{
			// no session name at all, set the out name with input
			outCharName = inputCharName;
		}
		else
		{
			// we have the full session name
			string sessionName = inputCharName.substr(pos+1, inputCharName.size()-pos-2);
			outCharName = inputCharName.substr(0, pos);;
			// look in the session name table
			for (uint i=0; i<_SessionNames.size(); ++i)
			{
				if (nlstricmp(_SessionNames[i].DisplayName, sessionName) == 0)
				{
					outSessionId = _SessionNames[i].SessionId;
					break;
				}
			}
		}
	}
}


// ***************************************************************************
std::string CShardNames::makeFullName(const std::string &charName, TSessionId homeSessionId)
{
	// look in the session name table
	for (uint i=0; i<_SessionNames.size(); ++i)
	{
		if (_SessionNames[i].SessionId == homeSessionId)
		{
			return charName+"("+_SessionNames[i].DisplayName+")";
		}
	}

	// session not found
	if(_AppendParenthesisWhenSessionNotFound)
		return charName+toString("(%u)", homeSessionId.asInt());
	else
		return charName;
}


// ***************************************************************************
std::string CShardNames::makeFullNameFromRelative(TSessionId contextSessionId, const std::string &inputcharName)
{
	std::string outName;
	TSessionId outSessionId;

	parseRelativeName(contextSessionId, inputcharName, outName, outSessionId);
	return makeFullName(outName, outSessionId);
}
