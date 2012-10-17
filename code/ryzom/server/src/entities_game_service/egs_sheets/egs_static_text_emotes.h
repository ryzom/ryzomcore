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



#ifndef EGS_STATIC_TEXT_EMOTES_H
#define EGS_STATIC_TEXT_EMOTES_H

//Nel georges
#include "nel/georges/u_form.h"
#include "game_share/mode_and_behaviour.h"

/**
 * class used to store emote phrases
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CStaticTextEmotes
{
public:
	struct CTextEmotePhrases
	{
		/// emote id in consummable items
		std::string EmoteId;
		///\name string ids of phrases that must be displayed when the actor has a target.
		//@{
		/// string for the crowd
		std::string TargetCrowd;
		/// string for the actor
		std::string TargetActor;
		/// string for the target
		std::string TargetTarget;
		//@}
		
		///\name string ids of phrases that must be displayed when the actor has no target.
		//@{
		/// string for the crowd
		std::string NoTargetCrowd;
		/// string for the actor
		std::string NoTargetActor;
		
		///\name string ids of phrases that must be displayed when the actor targets himshelf
		//@{
		/// string for the actor
		std::string SelfActor;
		/// string for the crowd
		std::string SelfCrowd;
		//@}
		
		/// animation id
		std::string Animation;
		/// true if the emote is reserved for FBT
		bool OnlyForFBT;

		/// true if this emote can be launched from client UI
		bool UsableFromClientUI;
		
		void serial(class NLMISC::IStream &f)
		{
			f.serial( EmoteId );
			f.serial( TargetCrowd );
			f.serial( TargetActor );
			f.serial( TargetTarget );
			f.serial( NoTargetCrowd );
			f.serial( NoTargetActor );
			f.serial( SelfActor );
			f.serial( SelfCrowd );
			f.serial( Animation );
			f.serial( OnlyForFBT );
			f.serial( UsableFromClientUI );
		}
	};
	/// ctor
	CStaticTextEmotes() { }

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 2; }
	/// Serial
	void serial(class NLMISC::IStream &f)
	{
		f.serialCont( _Phrases );
		if (f.isReading())
			buildEmoteIdMap();
	}

	const CTextEmotePhrases * getPhrase(uint idx)const
	{
		if ( idx < _Phrases.size() )
			return &_Phrases[idx];
		return NULL;
	}
	const CTextEmotePhrases * getPhrase(const std::string& emoteId)const
	{
		std::map<std::string, size_t>::const_iterator it = _EmoteIdMap.find(emoteId);
		if (it!=_EmoteIdMap.end() && it->second<_Phrases.size())
			return &_Phrases[it->second];
		return NULL;
	}
	uint16 getEmoteIndex(const std::string& emoteId) const
	{
		std::map<std::string, size_t>::const_iterator it = _EmoteIdMap.find(emoteId);
		if (it!=_EmoteIdMap.end())
			return (uint16)it->second;
		return (uint16)~0;
	}
	MBEHAV::EBehaviour getEmoteBehav(const std::string& emoteId) const;
	
	/// Removed
	void removed() { }
	
private:
	void buildEmoteIdMap();
	
private:		
	/// all emotes, ordered by integer id
	std::vector< CTextEmotePhrases > _Phrases;
	/// mapping of string id to integer id (index in _Phrases)
	std::map<std::string, size_t> _EmoteIdMap;
};



#endif
