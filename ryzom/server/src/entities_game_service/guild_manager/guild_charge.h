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



#ifndef RY_GUILD_CHARGE_H
#define RY_GUILD_CHARGE_H

#include "nel/misc/string_mapper.h"
#include "mission_manager/ai_alias_translator.h"

class CGuild;
class COutpost;

/// todo charge

/**
 * a charge that can be acquired by a guild 
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildCharge
{
	NL_INSTANCE_COUNTER_DECL(CGuildCharge);
public:

	/// Constructor /// /// todo charge
	CGuildCharge(const std::string & name)
		:/*_Owner( CGuild::InvalidGuildPtr ),*/_Outpost(NULL),_Mission(CAIAliasTranslator::Invalid),_Giver( CAIAliasTranslator::Invalid )
	{
		_Name =  NLMISC::strupr( name );
		_TextTitle = _Name + "_TITLE";
		_TextDetails = _Name + "_DESC";
	}
	/// versionning
	static uint32 getCurrentVersion(){ return 1;}
	/// build the charge
	bool build( NLMISC::TStringId civ, const std::vector<std::string> & params );
	/// xml serialization of dynamic data
	void xmlSerial( NLMISC::IStream & f );
	/// save the charge
	void save();
	/// update the duty : it is a new cycle
	void cycleUpdate();
	/// return the owner of the guild
	CGuild * getOwner(){ return _Owner; }
	/// set the owner of the guild
	void setOwner(CGuild * guild){ _Owner = guild; }
	/// add an applicant
	void addApplicant( CGuild * guild ){ _Applicants.push_back(guild); }
	/// remove the specified guild
	inline void removeApplicant( CGuild * guild);
	/// send the charge txt to the client
	void sendTexts( const TDataSetRow &userId, uint32 & title, uint32& details );
	/// get the title text of the guild
	const std::string & getTitleText(){ return _TextTitle; };
	/// set the charge outpost
	void setOutpost( COutpost * outpost ){ _Outpost = outpost; }
	/// get the charge outpost
	COutpost * getOutpost(){ return _Outpost; }
	/// get the charge name
	const std::string & getName(){ return _Name; }
	/// get the mission alias linked with the charge
	TAIAlias getMissionAlias(){ return _Mission; }
	/// set the giver of the charges.
	void setGiver( TAIAlias bot )
	{
		if ( _Giver != CAIAliasTranslator::Invalid && _Giver != bot)
			nlwarning("charge %s. Cant set bot %u as giver : %u is already giver",_Name.c_str(),bot,_Giver );
		else
			_Giver = bot;
	}

private:
	///\name static data ( sent by AI or computed from AI data )
	//@{
	/// name of the charge
	std::string									_Name;
	/// file to save
	std::string									_FileName;
	/// famed of the charge
//	NLMISC::TStringId							_FameId;
	/// Faction of the charge (replace _FameId)
	uint32										_Faction;
	/// title text
	std::string									_TextTitle;
	/// details text
	std::string									_TextDetails;
	/// outpost of the charge
	COutpost*									_Outpost;
	/// alias of the mission linked with this charge
	TAIAlias									_Mission;
	/// alias of the bot giving the charge
	TAIAlias									_Giver;
	//@}

	///\name dynamic data ( must be saved )
	//@{
	/// todo charge c etait un smart pointer
	CGuild*										_Owner;
	// todo charge c etait un smart pointer
	std::vector< CGuild* >						_Applicants;
	//@}

	NLMISC_COMMAND_FRIEND(displayCharges);
};

inline void CGuildCharge::removeApplicant( CGuild * guild)
{
	/// todo charge
	/*
	nlassert(guild);
	for ( uint i = 0; i < _Applicants.size(); i++ )
	{
		if  ( _Applicants[i] == guild )
		{
			_Applicants[i]->removeAppliedCharge(this);
			_Applicants[i] = _Applicants.back();
			_Applicants.pop_back();
			return;
		}
	}
	nlwarning("<removeApplicant> in charge %s : guild %u not found",_FileName.c_str(),guild->getId());
	*/
}// CGuildCharge::removeApplicant

#endif // RY_GUILD_CHARGE_H

/* End of guild_charge.h */
