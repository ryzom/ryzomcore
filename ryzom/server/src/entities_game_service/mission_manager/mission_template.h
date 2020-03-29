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



#ifndef RY_MISSION_TEMPLATE_H
#define RY_MISSION_TEMPLATE_H

#include "nel/ligo/primitive.h"
#include "nel/misc/string_mapper.h"

#include "game_share/character_title.h"
#include "game_share/guild_grade.h"
#include "game_share/mission_desc.h"
#include "game_share/season.h"
#include "game_share/prerequisit_infos.h"

#include "mission_step_template.h"
#include "mission_action.h"
#include "mission_manager/ai_alias_translator.h"
#include "mission_manager/mission_event.h"
#include "egs_pd.h"


class CCharacter;

// Result value for mission termination
enum TMissionResult
{
	mr_success,
	mr_fail,
	mr_abandon,
	/// forced success
	mr_forced,
	mr_result_count
};

// StatLog tag for mission result
const char *const MissionResultStatLogTag[mr_result_count] =
{
	"SUC",	// mr_success
	"FAI",	// mr_fail
	"ABD",	// mr_abandon
	"FOR"	// mr_forced
};


/* Structure for storring mission statistic data
*/
/*struct TMissionStats
{
	/// The last try date, also used for replay timer

	NLMISC::TGameCycle	LastTryDate;
	/// Numer of time the mission terminate successfully
	uint32				SuccessCount;
	/// Number of time the mission terminate with a failure
	uint32				FailCount;
	/// Number of time the mission was abandoned
	uint32				AbandonCount;

	TMissionStats()
		:	LastTryDate(0),
			SuccessCount(0),
			FailCount(0),
			AbandonCount(0)
	{}

	void serial (NLMISC::IStream &s)
	{
		s.serial(LastTryDate);
		s.serial(SuccessCount);
		s.serial(FailCount);
		s.serial(AbandonCount);
	}
};
*/
/**
 * Represents the static data of a mission. An instanciated mission references a mission template for all the static data and stores the dynamic data
 * Template can also reference its instances, it can be useful in special cases ( mono instances missions for exemple )
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CMissionTemplate
{
	NL_INSTANCE_COUNTER_DECL(CMissionTemplate);
public:

	/// ctor (just some default value)
	CMissionTemplate();

	/// dtor ( free all the static data )
	~CMissionTemplate();

	/// copy this mission
	void copy(CMissionTemplate & ref);

	/// Return the mission name.
	const std::string &getMissionName() const;

	/// build from a primitive
	bool				build(const NLLIGO::IPrimitive* prim,CMissionGlobalParsingData & globalData, CHashMap<uint,TAIAlias> & refMissions, const std::string &missionName, TAIAlias defautltNpcGiverAlias);

	/// return the highest step index among the step that are out of order with the specified index
	inline uint32		getOutOfOrderSteps(uint32 firstStep);

	/// return a SBotChatMission::TPreReqState to know the reason why it fails (if it fails of course)
	inline uint32		testPrerequisits( CCharacter * user, bool logOnFail) const
	{
		static CPrerequisitInfos prereqInfos;
		return testPrerequisits(user, prereqInfos, logOnFail, false);
	}

	/// return a SBotChatMission::TPreReqState to know the reason why it fails (if it fails of course), fill prereqInfos with infos on prerequisit
	uint32				testPrerequisits( CCharacter * user, CPrerequisitInfos &prereqInfos, bool logOnFail, bool fillPrereqInfos = true ) const;

	/// send the mission title
	uint32				sendTitleText( const TDataSetRow & userRow,const TDataSetRow & giver ) const;

	/// send the mission auto text
	uint32				sendAutoText( const TDataSetRow & userRow,const NLMISC::CEntityId & giver) const;

	/// send the mission description ( param text and addParams are used for text overloasding )
	uint32				sendDescText( const TDataSetRow & userRow, const TDataSetRow & giver, uint32 descIndex = 0xFFFFFFFF ) const;

	/// fill a vector with all the escort groups.
	inline void			getEscortGroups( std::vector< TAIAlias > & groups )const;

	/// Reset the last try date, debug only.
	void				resetGlobalReplayTimer() const
	{
		const_cast<CMissionTemplate&>(*this).LastSuccessDate = 0;
	}

	/// Return the step by indexInTemplate, or NULL
	const IMissionStepTemplate *getStepByIndexInTemplate( uint32 indexInTemplate ) const
	{
		uint32 index0 = indexInTemplate - 1;
		return getStep( index0 );
	}

	/// Return the step by index (starting at 0), or NULL
	const IMissionStepTemplate *getStep( uint32 index0 ) const
	{
		if ( index0 < Steps.size() )
			return Steps[index0];
		else
			return NULL;
	}

	/// return the default giver. Yoyo: if I have correctly understood, should be only him which can give this mission
	TAIAlias getDefaultNpcGiver() const {return _DefautltNpcGiverAlias;}

	// update the statictic data
//	void				updateMissionStats(TMissionResult result);
		
	/// descriptor of a skill prerequisit
	struct CSkillPrereq
	{
		SKILLS::ESkills	Skill;
		sint32			Min;
		sint32			Max;
	};
	/// descriptor of a jump point ( used to go to a specificc point of the mission script )
	struct CJumpPoint
	{
		std::string Name;
		uint32 Step;
		uint32 Action;
		CJumpPoint( const std::string & label,uint step, uint32 action):Name(label),Step(step),Action(action){}
	};

	/// a crash handler of the mission ( action to trigger in case of a crash )
	struct CCrashHandler
	{
		/// Names of AI instances whose crash can trigger the handler
		std::vector<std::string>		AIInstances;
		/// actions to trigger
		std::vector<IMissionAction*>	Actions;
	};

	struct CPlayerReconnectHandler
	{
		/// actions to trigger
		std::vector<IMissionAction*>	Actions;
	};

	/// the prerequisit of the mission
	struct CPrerequisits
	{
		struct TMissionReq
		{
			uint32						Line;
			std::vector<std::string>	Missions;

			TMissionReq()
				: Line(0)
			{
			}
			TMissionReq(uint32 line, const std::vector<std::string> &missions)
				: Line(line), Missions(missions)
			{}
		};
		/// skills prereq
		std::vector< std::vector<CSkillPrereq> >		Skills;
		/// needed completed missions. an entry is a vector of missions linked with an OR conditions. All vectors are linked together with AND conditions
//		std::vector< std::vector< std::string > >		NeededMissions;
		std::vector< TMissionReq >		NeededMissions;
		/// missions that must not have been picked or done. an entry is a vector of missions linked with an OR conditions. All vectors are linked together with AND conditions
//		std::vector< std::vector< std::string > >		ForbiddenMissions;
		std::vector< TMissionReq >		ForbiddenMissions;
		/// missions that must be taken by the player ( but not completed ) an entry is a vector of missions linked with an OR conditions. All vectors are linked together with AND conditions
//		std::vector< std::vector< std::string > >		RunningMissions;
		std::vector< TMissionReq >		RunningMissions;
		///an entry is a vector of missions linked with an OR conditions. All vectors are linked together with AND conditions
//		std::vector< std::vector< std::string > >		ForbiddenRunningMissions;
		std::vector< TMissionReq >		ForbiddenRunningMissions;

		/// items that must be worn when mission is picked
		std::vector< NLMISC::CSheetId >		Wear;
		/// item that must be owned
		std::vector< NLMISC::CSheetId >		Own;
		/// title that must be displayed by the player
		CHARACTER_TITLE::ECharacterTitle	Title;
		/// fame prerequisits
		NLMISC::TStringId					FameId;
		sint32								FameMin;
		/// true if the player must be in a guild
		bool								Guild;
		/// minimum guild grade displayed by the player
		EGSPD::CGuildGrade::TGuildGrade		GuildGrade;
		/// minimum team size
		uint8								TeamSize;
		/// actions that the player must know
		std::vector<NLMISC::CSheetId>		KnownActions;
		/// The season that must be current
		EGSPD::CSeason::TSeason				Season;
		/// Character minimum oldness
		uint32								CharacterMinAge;
		/// minimum account id
		uint32								MaxPlayerID;

		// Requesite a specific thema of a specific album that all of its tasks are done
		// or if TaskDone is false check if at least one is not done
		bool	EncycloReqTaskDone;
		sint32	EncycloReqAlbum;
		sint32	EncycloReqThema;

		// not empty if player must belong to a particular event faction
		std::string	EventFaction;

		/// kami or karavan type
		enum TKamiKaravan
		{
			None,
			Kami,
			Karavan,
		};

		TKamiKaravan KamiKaravan;


		// --------------------------------------------------

		CPrerequisits()
		{
			EncycloReqAlbum = -1;
			EncycloReqThema = -1;
			KamiKaravan = None;
		}
	};
	/// tags defining mission property
	struct
	{
		bool Replayable				:1;
		bool DoneOnce				:1;
		bool NoList					:1;
		bool AutoRemove				:1;
		bool NotProposed			:1;
		bool NonAbandonnable		:1;
		bool NeedValidation			:1;
		bool FailIfInventoryIsFull	:1;
		bool HideIconOnGiverNPC		:1;
	}Tags;

	std::string HashKey;


	/// auto text phrase Id
	std::string										AutoText;
	/// auto text param
	TVectorParamCheck								AutoParams;

	///title phrase_id
	std::string										TitleText;
	/// title param
	TVectorParamCheck								TitleParams;
	///description phrase_id
	std::string										DescText;
	/// description params
	TVectorParamCheck								DescParams;
	/// mission type
	MISSION_DESC::TMissionType						Type;
	/// mission category
	std::string										MissionCategory;
	/// mission icon
	NLMISC::CSheetId								Icon;
	/// instance of the mission, if the mission is MonoInstance
	std::vector<CMission*>							Instances;
	/// the steps of the mission
	std::vector< IMissionStepTemplate *>			Steps;
	/// mission alias
	TAIAlias										Alias;
	/// prerequisits of the mission
	CPrerequisits									Prerequisits;
	/// action triggered when mission is picked
	std::vector< IMissionAction* >					InitialActions;
	/// actions triggered on failure
	std::vector< std::vector< IMissionAction* > >	FailureActions;
	/// crash handlers ( special actions to be triggered in case of mission crash
	std::vector<CCrashHandler>						CrashHandlers;
	/// crash handlers ( special actions to be triggered in case of mission crash
	std::vector<CPlayerReconnectHandler>			PlayerReconnectHandlers;
	/// pairs of [first,last] step index. All steps in [first,last] are out of order ( can be done any order )
	std::vector< std::pair<uint32,uint32> >			OutOfOrderSteps;

	/// true if mission already done
	bool											AlreadyDone;

	/// the mission jump points
	std::vector< CJumpPoint >						JumpPoints;

	/// action triggered at the end of an ooo/any bloc
	std::vector< std::vector< IMissionAction* > >	OOOActions;

	/// Timer of mono missions
	NLMISC::TGameCycle								MonoTimer;

	/// Player replay timer : a player can only replay this mission
	/// after the timer elapsed (win or failed)
	NLMISC::TGameCycle								PlayerReplayTimer;
	/// Global replay timer
	NLMISC::TGameCycle								GlobalReplayTimer;

	/// Date of last success of this mission (for global replay timer)
	// TODO : persist the last try date
	NLMISC::TGameCycle								LastSuccessDate;
	/// children missions ( if this mission stops, children missions stops too )
	std::vector<TAIAlias>							ChildrenMissions;

	/// overloaded descriptions texts
	struct COverloadedDesc
	{
		std::string							Text;
		TVectorParamCheck					Params;
	};
	std::vector<COverloadedDesc> OverloadedDescs;

	std::vector<CMissionActionSetTeleport*> Teleports;

	sint32 EncycloAlbum;	// If this mission do not belongs to the encyclopedia -1
	sint32 EncycloThema;	// Thema begins at 1 not at 0
	sint32 EncycloTask;		// 0-Rite 1-7 Task
	TAIAlias EncycloNPC;

private:
	
	// This is the default NPC giver of this mission template (as parsed from the primitive).
	// Yoyo: I'm not sure I understand the whole system but it seems that a same Mission Template 
	// may be proposed by only one NPC. Hence the paradigm Mission = (MissionTemplate, Player, Giver) seems strange
	// The giver should be always hard bound to the mission template?
	// Btw, this Alias may be NULL if I didn't understand, and is used for Mission Title translation purpose only.
	TAIAlias _DefautltNpcGiverAlias;

	///\name parsing helpers
	//@{
	/// parse a mission text
	bool parseMissionText(uint32 line,  const std::vector< std::string > & preparsedParams, std::string & textId, TVectorParamCheck & textParams );
	/// parse a variable declared in script
	bool parseScriptVar( uint32 line, const std::vector< std::string > & preparsedParams, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > >& chatParams );
	/// add a script prerequisit line to the skill prerequists
	bool addSkillToList( uint32 line, const std::vector< std::string > & preparsedParams, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams );
	/// build a chat parameter list from an array of string
//	bool parseChatParamList( const std::string & separator, const std::vector< std::string > & preparsedParams, TVectorParamCheck & ret );
	/// parse a list of items
	bool parseItemList(uint32 line,  const std::string & separator, const std::vector< std::string > & preparsedParams, std::vector< NLMISC::CSheetId > & ret, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams );
	/// parse player title prereq
	bool parseTitlePrereq(uint32 line,  const std::vector< std::string > & preparsedParams );
	/// parse the minimum fame
	bool parseFamePrereq(uint32 line,  const std::vector< std::string > & preparsedParams );
	/// parse player grade prereq
	bool parseGradePrereq(uint32 line,  const std::vector< std::string > & preparsedParams );
	/// parse an int param
	bool parseInt(uint32 line,  const std::vector< std::string > & preparsedParams, int & ret );
	/// parse a action
	bool parseBrickList(uint32 line,  const std::vector< std::string > & preparsedParams, std::vector< NLMISC::CSheetId > & ret, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams );	
	/// add the mission to the mission list
	bool addMissionsToList(uint32 line,  const std::vector< std::string  > & preparsedParams, std::vector< CPrerequisits::TMissionReq  > & ret );
//	bool addMissionsToList(uint32 line,  const std::vector< std::string > & preparsedParams, std::vector< std::vector< std::string > > & ret );
	/// parse a season name
	bool parseSeason(uint32 line, const std::vector< std::string > & preparsedParams, EGSPD::CSeason::TSeason &ret);
	/// parse a mission sole param
	TAIAlias parseMissionParam(uint32 line,  const std::vector< std::string > & preparsedParams );
	/// parse a mission replay timer
	bool parseReplayTimer(uint32 line,  const std::vector< std::string > & preparsedParams , NLMISC::TGameCycle &replayTimer);
	/// parse an item price
	bool parsePrice(uint32 line,  const std::vector< std::string > & preparsedParams, CMissionSpecificParsingData& data );
	//@}

	/** Fill prerequesits with mission requirement text (with mission titles)
	 * If testAutoForbidden==true and addedPrereqTexts!=NULL, then we test if one mission template match *this. If so, add just a single prereq message and quit
	 */
	void	addMissionPrerequisitInfo(CPrerequisitInfos &prereqInfos, const char *reqText, CCharacter *user, const std::vector<std::string> &missionReqOr, bool testAutoForbidden, std::set<std::string> &addedPrereqTexts) const;
};

inline uint32 CMissionTemplate::getOutOfOrderSteps(uint32 firstStep)
{
	for ( uint i = 0; i < OutOfOrderSteps.size(); i++ )
	{
		if ( OutOfOrderSteps[i].first == firstStep )
			return OutOfOrderSteps[i].second;
	}
	return 0xFFFFFFFF;
}// CMissionTemplate::getOutOfOrderSteps

inline void CMissionTemplate::getEscortGroups( std::vector< TAIAlias > & groups )const
{
	for ( uint i = 0; i < Steps.size(); i++ )
	{
		Steps[i]->getEscortGroups(groups);
	}
}

#endif // RY_MISSION_TEMPLATE_H

/* End of mission_template.h */



