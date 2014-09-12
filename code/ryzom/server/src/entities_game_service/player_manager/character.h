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


#ifndef CHARACTER_H
#define CHARACTER_H

// Game Share
#include "game_share/bot_chat_types.h"
#include "game_share/brick_flags.h"
#include "server_share/pet_interface_msg.h"
#include "game_share/power_types.h"
#include "game_share/roles.h"
#include "game_share/temp_inventory_mode.h"
#include "game_share/dyn_chat.h"
#include "game_share/timer.h"
#include "game_share/mission_desc.h"
#include "game_share/people.h"
#include "game_share/pvp_clan.h"
#include "game_share/string_manager_sender.h"
#include "game_share/r2_types.h"

#include "server_share/pet_interface_msg.h"
#include "server_share/r2_vision.h"

// Misc
#include "nel/misc/string_conversion.h"

// EGS
#include "entity_manager/entity_base.h"
#include "character_structure/character_persistant_data.h"
#include "harvest_info.h"
#include "inventory_updater.h"
#include "known_and_memorized_phrases.h"
#include "modifiers_in_db.h"
#include "powers_and_auras.h"
#include "static_action_types.h"
#include "far_position_stack.h"
#include "pvp_manager/pvp_manager_2.h"

#include "egs_sheets/egs_static_game_sheet.h"

#include "progression/progression_common.h"
#include "phrase_manager/s_effect.h"
#include "mission_manager/ai_alias_translator.h"
#include "ring_reward_points.h"
#include "persistent_effect.h"
#include "character_interface.h"
#include "database_plr.h"

namespace NLMISC
{
	class CBitMemStream;
};

namespace EGSPD
{
	class CFameContainerPD;
	class CMissionContainerPD;
	class CMissionPD;
}

namespace CHARSYNC
{
	class TCharInfo;
}

typedef std::vector< std::pair< NLMISC::CSheetId, uint8 > > TMpIdQuality;
typedef EGSPD::CMissionPD CMission;

// number of slot in a bot chat list page
#define NB_SLOT_PER_PAGE		8
// number of target coordinates in the journal
#define NB_JOURNAL_COORDS		8
// max number of context menus dynamic options
#define NB_CONTEXT_DYN_TEXTS	4

// number of buff slot (in each category)
#define NB_BUFF_SLOT			12

// max number of step in a mission
#define NB_STEP_PER_MISSION		20

// max number of historics in a mission
#define NB_HISTO_PER_MISSION	30

#define INVALID_POSITION_ID		0

// Max number of pact character can be have
#define NB_MAX_PACT				5

static const uint8 MaxSoloMissionCount = 15;
static const uint8 MaxGroupMissionCount = 4;
static const uint8 MaxGuildMissionCount = 4;

/// Frequency of character's tick update
const uint CharacterTickUpdatePeriodGc = 16;

class CCreature;
class CMissionTemplate;
class CMissionEvent;
class CMissionSolo;
class CCharacterVersionAdapter;
class CCharacterEncyclopedia;
class CCharacterGameEvent;
class CCharacterRespawnPoints;
class CCharacterShoppingList;

class CCreateCharMsg;
class CCreateCharErrorMsg;
class CPlace;
class CRegion;
class COutpost;
class CPlayerRoomInterface;
class CItemsForSale;
class CAdminProperties;
class CDeathPenalties;
class CGearLatency;

class CHarvestSource;
class CForageProgress;

class CPVPInterface;
class CStaticCharacters;

class CExchangeView;


/**
 * struct for trading phrases
 */
struct CTradePhrase
{
	/// phrase sheet id
	NLMISC::CSheetId	SheetId;

	CTradePhrase() {}
	explicit CTradePhrase(NLMISC::CSheetId sheetId) : SheetId(sheetId) {}

	/// <operator used to sort vectors of CTradePhrase
	bool operator<(const CTradePhrase &p) const
	{
		return (SheetId < p.SheetId);
	}
};


/*
 *	SGameCoordinate
 */
struct SGameCoordinate
{
	SGameCoordinate()
	{
		reset();
	}
	void reset()
	{
		X = 0;
		Y = 0;
		Z = 0;
		Heading = 0.0f;
		Cell = 0;
		Continent = 0;
	}
	sint32	X;
	sint32	Y;
	sint32	Z;
	float	Heading;
	sint32	Cell;
	uint8 Continent;

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( X );
		f.serial( Y );
		f.serial( Z );
		f.serial( Heading );
		f.serial( Cell );
		f.serial( Continent );
	}
};


/* Storage class for mission history data.
*/
struct TMissionHistory
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	// flag for successfull mission
	bool				Successfull;
	/// Last success date (used for replay player replay timer)
	NLMISC::TGameCycle	LastSuccessDate;

	TMissionHistory()
	{
		clear();
	}

	void clear()
	{
		Successfull=false;
		LastSuccessDate=0;
	}

	void serial(NLMISC::IStream &s)
	{
		s.serial(Successfull);
		s.serial(LastSuccessDate);
	}
};

struct CWelcomeMissionDesc
{
	DECLARE_PERSISTENCE_METHODS

	CWelcomeMissionDesc()
	{
		clear();
	}

	void clear()
	{
		MissionAlias = CAIAliasTranslator::Invalid;
		BotAlias = CAIAliasTranslator::Invalid;
	}

	bool isValid() const
	{
		return (MissionAlias != CAIAliasTranslator::Invalid && BotAlias != CAIAliasTranslator::Invalid);
	}

	TAIAlias MissionAlias;
	TAIAlias BotAlias;
};


typedef float TSatiety;
const TSatiety SatietyNotInit = -9999.0f;


/**
 * CPetAnimal
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 */
struct CPetAnimal
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	enum TStatus { db_unknown = -1, not_present = 0, waiting_spawn, landscape, stable, death, tp_continent };

	TStatus				PetStatus;
	NLMISC::CSheetId	TicketPetSheetId;
	NLMISC::CSheetId	PetSheetId;
	uint32				Price;
	CGameItemPtr		ItemPtr;
	NLMISC::CEntityId	OwnerId;
	TDataSetRow			SpawnedPets;
	uint32				StableId;
	sint32				Landscape_X;
	sint32				Landscape_Y;
	sint32				Landscape_Z;
	NLMISC::TGameCycle	DeathTick;
	uint32				Slot;		// used for found item ticket pointer, slot of ticket in bag
	TStatus				DbPetStatus; // value of PetStatus in the database, for optimization purpose
	TSatiety			Satiety;
	TSatiety			MaxSatiety; // not saved, got from form when TicketPetSheetId loaded 
	uint32				AnimalStatus;
	bool				IsFollowing;
	bool				IsMounted;
	bool				IsTpAllowed;
	bool				spawnFlag;
	ucstring			CustomName;
	
	// ctor
	CPetAnimal();

	void clear();
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	// init found ticket item pointer with slot
	uint32 initLinkAnimalToTicket( CCharacter * c, uint8 index);

	// get the max bulk of the animal inventory
	uint32 getAnimalMaxBulk();

	void setCustomName(const ucstring& customName) { CustomName = customName; }
};

/**
 * SBotChatMission
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date January 2005
 * 
 * Structure to hold information about bot missions on the character
 */
struct SBotChatMission
{
	TAIAlias					Mission;
	MISSION_DESC::TPreReqState	PreReqState;

	// ---------------------------------------

	SBotChatMission()
	{
		Mission = CAIAliasTranslator::Invalid;
		PreReqState = MISSION_DESC::PreReqSuccess;
	}
};

/**
 *	TCharacterLogTime
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2005
 */
/// Character time statistics in seconds
struct TCharacterLogTime
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);
	
	DECLARE_PERSISTENCE_METHODS
		
	uint32	LoginTime;
	uint32	Duration;
	uint32	LogoffTime;
	
	TCharacterLogTime()
	{
		LoginTime = 0;
		Duration = 0;
		LogoffTime = 0;
	}
};


/**
 *	CXpProgressInfos
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2005
 */
struct CXpProgressInfos
{
	double	TotalXpGain;
	double	XpBonus;
	uint32	CatalyserCount;
	uint32	CatalyserLvl;
	double	RingXpBonus;
	uint32	RingCatalyserCount;
	uint32	RingCatalyserLvl;

	CXpProgressInfos()
	{
		TotalXpGain = 0;
		XpBonus = 0;
		CatalyserCount = 0;
		CatalyserLvl = 0;
		RingXpBonus = 0;
		RingCatalyserCount = 0;
		RingCatalyserLvl = 0;
	}
};

enum TFriendVisibility
{
	VisibleToAll = 0,			// Visible to all people who have me on their friends list, even if I am ignoring them.
	VisibleToGuildAndFriends,	// Visible to people in my guild and those that have me on their friends list.
	VisibleToGuildOnly,			// Only visible to people in my guild.
	NB_FRIEND_VISIBILITY

};


/**
 * CCharacter
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CCharacter 
	:	public CCharacterPersistantData, 
		public CEntityBase, 
		public NLMISC::CEvalNumExpr,
		public ICharacter
{
private:
	/// enum for Contact List action (add, remove ...)
	enum TConctactListAction
	{
		AddedAsFriend = 0,
		RemovedFromFriends,
		AddToIgnored,
		RemovedFromIgnored,

		RemoveFriend,
		RemoveIgnored,
		
		AddedAsLeague,
		RemovedFromLeague,
		RemoveLeague,

		NB_CONTACT_LIST_ACTIONS,
		UnknownContactListAction= NB_CONTACT_LIST_ACTIONS,
	};
	static const NLMISC::CStringConversion<TConctactListAction>::CPair	ContactListActionConversionTable[];
	static NLMISC::CStringConversion<TConctactListAction>				ContactListActionConversion;
	static TConctactListAction	toContactListAction(const std::string &str);
	static const std::string	&contactListActionToString(TConctactListAction e);

	NL_INSTANCE_COUNTER_DECL(CCharacter);

public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS


	/////////////////////
	/// Api of CCharacter
	/////////////////////

	////////////////////////////////////
	// VERSION History
	// 3 : (10/08/2003) changed _KnownPhrases structure from vector<CPhraseCom> to vector<CKnownPhrase>
	// 4 : (10/10/2003) added _ClientInventoryPosition param in CGameItem class (sint16), changement managed by the CGameItem class
	//					also changed the load method of CGameItem (3rd param = CCharacter version)
	// 9 : (29/10/2003) Added the forage phrase in the starting role sheet
	// 11 : (15/11/2003) Added missions
	// 12 : (04/12/2003) added _StartingCharacteristicValues
	// 13 : (05/12/2003) players now gains 10 times more SP and phrases cost 10 times more
	// 14 : (10/12/2003) mission saves updated : new data needed for mission journal
	// 16 : (10/12/2003) added auras and powers management
	// 19 : (06/01/2004) added droppable property for items
	// 20 : (08/01/2004) game item sap load and enchantment
	// 21 : (14/01/2004) mission giver
	// 22 : (20/01/2004) added _ModifiersInDB
	// 32 : (09/02/2004) added : equipped items slots
	// 35 : (09/02/2004) increase to destroy old guild format
	// 36 : (24/02/2004) added FriendsList, IgnoreList and _IsIgnoredBy and _IsFriendOf
	// 37 : (01/03/2004) added 4 new SP types (fight, magic, craft and harvest)
	// 38 : (01/03/2004) keep number of points assigned to different type during creation
	// 39 : (02/03/2004) Changed memories behaviours (now 10 without any differences) -> need to clear memories
	// 40 : (02/03/2004) NEW SP : Wipe all phrases and bricks
	// 42 : (30/03/2004) removed _RespawnPoint 
	// 43 : binary serial
	// 44 : (28/04/2004) Craft change : wipe bricks, give SP back
	// 45 : (03/05/2004) Patch : add characteristic upgrade bricks and phrases to older saves
	// 46 : (03/05/2004) Modify self heal actions (4 roots instead of one, plus effects renamed)
	// 47 : (04/05/2004) death penalties
	// 48 : (05/05/2004) player room
	// 49 : (11/05/2004) mission steps done by player
	// 50 : (12/05/2004) wipe all actions and characteristic upgrades,give back SP
	// 51 : (12/05/2004) wipe all actions and characteristic upgrades,give back SP
	// 52 : (15/05/2004) fames
	// 53 : (03/06/2004) player room
	// 54 : (16/06/2004) serialize the number of sp types
	// 55 : (01/07/2004) serialize _LostHPremains in CGameItem
	// 56 : (01/07/2004) removed hit rate + bricks and phrases, give back Sp
	// 57 : (15/07/2004) changed self heal powers
	// 58 : (22/07/2004) patch old heal self phrase in memory
	// Warning : the following code have been incremented and merged to match live server code after a patch (version 58), invalidate daily versions but keep live version safe :)
	// 59 : (05/07/2004) guild system managed through the PDS
	//		(15/07/2004) mission system managed through the PDS
	// 60 : (05/07/2004) Fame system managed through PDS
	// 61 : (11/08/2004) player room managed through PDS
	// 62 : (25/08/2004) magic bricks SPCost have been decreased, some stun,sleep,and rot mandatory bricks have been removed
	// 63 : (25/08/2004) changed area effect bricks
	// 64 : (26/08/2004) add vector of item selled character for shop store system
	// 65 : (27/05/2004) forage RM group-prospection bricks cleaned
	// 66 : (01/09/2004) added _ScorePermanentModifiers
	// 67 : (08/09/2004) save of item for sale
	// 68 : (09/09/2004) patch old tools HP
	// 69 : (04/10/2004) patch bad timers (effects and missions, after a problem with tick save)
	////////////////////////////////////
	/**
	 * \return the current version of the class. Useful for managing old versions of saved players
	 * WARNING : the version number should be incremented when the serial method is modified
	 */
	static uint16 getCurrentVersion();

	///\return the saved version
	uint16 getSavedVersion();

	/**
	 * Default constructor
	 */
	CCharacter();
	
	/**
	 * Destructor
	 */
	~CCharacter();

	/**
	 * Init PD Struct
	 */
	void	initPDStructs();

	/**
	 * Clear the contents of the CCharacter
	 */
	void clear();
	
	/**
	 * Set the id
	 * \param id is the CEntityId OF character
	 */
	void setId( const NLMISC::CEntityId& id );

	/** Character interface forwarder */
	const NLMISC::CEntityId& getCharId() const;
	const NLMISC::CEntityId& getId() const;

	/** Fill the TCharInfo struct used to send info to SU
	 */
	void fillCharInfo(CHARSYNC::TCharInfo &charInfo) const;

	/** Set the startup instance id. 
	 *	This instance is use to put the player in after receiving READY 
	 *	instead of determining the instance from the player position.
	 */
	void setStartupInstance(uint32 instanceId);

	/** Return the startup instance. Return INVALID_INSTANCE_ID if the
	 *	instance must be computed from the player position.
	 */
	uint32 getStartupInstance();

	/** Set the name of the character */
	void setName(const ucstring &name);

	/***
	 * Set character Title
	 */
	void setTitle( CHARACTER_TITLE::ECharacterTitle title );

	/***
	 * Get character Title
	 */
	CHARACTER_TITLE::ECharacterTitle getTitle() const;

	/**
	 * set the mode without position or orientation information 
	 * \param mode the new mode
	 * \param forceUpdate if true, set mode without check gameplay rules (for EGS use only)
	 * \param disengage true to call phrase manager disengage, false otherwise
	 */
	virtual void setMode( MBEHAV::EMode mode, bool forceUpdate = false, bool disengage = true);

	/**
	 * set the mode with position or orientation information
	 * \param mode the new mode (with position or orientation)
	 */
	virtual void setMode( MBEHAV::TMode mode );

	/**
	 * Add the properties to the emiter
	 */
	void addPropertiesToMirror( const TDataSetRow& entityIndex, bool keepSheetId=true );

	/**
	 *	Set the value of a var
	 * \param var is the name of the variable
	 * \param value is the new value for the variable
	 * \return true if the value has been set, false if an error occured
	 */
	bool setValue( std::string var, std::string value );

	/**
	 *	Modify the value of a var
	 * \param var is the name of the variable
	 * \param value is the modification value
	 * \return true if the value has been changed, false if an error occured
	 */
	bool modifyValue( std::string var, std::string value );
	
	/**
	 *	Get the value of the variable
	 * \param var is the name of the variable
	 * \param value receive the current value of the variable
	 * \return true if the var has been found, false otherwise
	 */
	bool getValue( std::string var,	std::string& value );

	/// Fill version data for handshake
	void fillHandshake( NLMISC::CBitMemStream& bms );

	// update next timer tick event
	void setUpdateNextTick();
	
	/**
	 * Make all necessary update for character at timer ticks
	 */
	uint32 tickUpdate();

	/**
	 * Set the enter flag 
	 * \param b true if the player entered the game, false if he left
	 */
	void setEnterFlag( bool b );

	/**
	 * Get the enter flag 
	 * \return true if the player entered the game, false if he left
	 */
	bool getEnterFlag() const;

	/**
	 * wrapper to CEntityBase
	 */
//	CEntityState& getState();
//	const CEntityState& getState() const;
	void setState( const COfflineEntityState& es );
	bool isDead() const;
	
	/**
	 * Serial: reading off-mirror, writing from mirror
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	// set player position to default respawn point of continent
	void setPositionToDefaultRespawnPoint();

	/**
	 * Load George Sheet with CSheetId
	 * \param sheetId is id of sheet of entity
	 * \param level if level of creature (if == 0, use level in sheet (Basics.Level))
	 */
	void loadSheetCharacter( const NLMISC::CSheetId& sheetId, uint16 level = 0 );

	/**
	 * Load George Sheet directly with filename, without using CSheetId (for testing tool)
	 * \param SheetName is filename of player sheet
	 * \param level if level of creature (if == 0, use level in sheet (Basics.Level))
	 */
	void loadSheetCharacter( const std::string& SheetName, uint16 level );

	/**
	 * Load George Sheet with static sheet pointer, common part of other loadSheetCharacter
	 * \param charactersSheet is static sheet pointer
	 * \param level if level of creature (if == 0, use level in sheet (Basics.Level))
	 */
	void loadSheetCharacter( const CStaticCharacters * charactersSheet, uint16 level, const std::string& sheetName );

	/**
	 * set the current target 
	 * \param target the new target
	 */
	virtual void setTarget( const NLMISC::CEntityId& targetId, bool sendMessage = true );

	/**
	 * set target bot-chat programm
	 */
	void setTargetBotchatProgramm( CEntityBase * target, const NLMISC::CEntityId& targetId );

	/**
	 * enable appropriate filter for seller
	 */
	void enableAppropriateFiltersForSeller( CCreature * seller );

	/**
	 * called when this entity is targeted by a player
	 * \param entity pointer on the entity targeting the current one
	 */
	void addTargetingChar( const TDataSetRow & row );

	/**
	 * called when this entity is not targeted by a specifice player anymore
	 * \param entity pointer on the entity targeting the current one
	 */
	void removeTargetingChar( const TDataSetRow & row );

	/// update target chars target
	void updateTargetingChars();

	CRingRewardPoints &getRingRewardPoints();

	/// update the target properties
	void updateTarget();

	/// add a known brick
	void addKnownBrick( const NLMISC::CSheetId& brickId );

	/// check if have brick
	bool haveBrick( const NLMISC::CSheetId& brickId );

	/// remove a known brick
	void removeKnownBrick( const NLMISC::CSheetId& brickId );

	/// get known bricks
	const std::set<NLMISC::CSheetId> &getKnownBricks() const;
		

	// return reference to visual property A
	CMirrorPropValueAlice< SPropVisualA, CPropLocationPacked<2> >& getVisualPropertyA();

	// return reference to visual property B
	CMirrorPropValueAlice< SPropVisualB, CPropLocationPacked<2> >& getVisualPropertyB();

	// return reference to visual property C
	CMirrorPropValueAlice< SPropVisualC, CPropLocationPacked<2> >& getVisualPropertyC();

	// update visual information after inventory manipulation
	void updateVisualInformation( uint16 InventoryEmpty, uint16 SlotEmpty, uint16 InventoryFull, uint16 SlotFull, const NLMISC::CSheetId& IdSheetItem, CGameItemPtr Item );

	// update visual properties after equipment inventory manipulation
	void setVisualPropertyForEquipment( uint16 slot, const CStaticItem* srcForm, uint16 quality, uint8 color );

	// tp wanted, check if tp is regular and send a server tp command
	void tpWanted( sint32 x, sint32 y, sint32 z , bool useHeading = false, float heading = 0.0f , uint8 continent = 0xFF, sint32 cell = 0);

	void teleportCharacter( sint32 x, sint32 y);

	// teleport character with or without his mount, check if tp is regular and send a server tp command
	void teleportCharacter( sint32 x, sint32 y, sint32 z, bool teleportWithMount, bool useHeading = false, float heading = 0.0f, uint8 continent = 0xFF, sint32 cell = 0, uint8 season = 0xff, const  R2::TR2TpInfos& tpInfos= R2::TR2TpInfos());

	// return the season in which is the current character
	uint8 getRingSeason() const { return _RingSeason;}

	/// get the League id
	TChanID getLeagueId() const { return _LeagueId;}
	
	
		
	// \name Team related methods
	//@{
	/// get the team Id of this player
	uint16 getTeamId() const;
	/// set the team Id of this player
	void setTeamId(uint16 id);
	/// set the League id
	void setLeagueId(TChanID id, bool removeIfEmpty=false);
	/// get team invitor
	const NLMISC::CEntityId & getTeamInvitor() const;
	/// set team invitor
	void setTeamInvitor(const NLMISC::CEntityId & invitorId);
	/// get League invitor
	const NLMISC::CEntityId & getLeagueInvitor() const;
	/// set League invitor
	void setLeagueInvitor(const NLMISC::CEntityId & invitorId);
	//@}

	/// Set fighting target
	void setFightingTarget( const NLMISC::CEntityId& targetId );

	// \name Harvest related methods
	//@{
	// harvest action
	void harvestAsked( uint16 mpIndex, uint16 quantity = 0xffff);
	void harvest( uint8 mpIndex, uint16 quantity );
	const NLMISC::CEntityId &harvestedEntity() const;
	void harvestedEntity( const NLMISC::CEntityId &id);
	const NLMISC::CSheetId &harvestedEntitySheetId() const;
	void harvestedEntitySheetId( const NLMISC::CSheetId &sheet);
	uint8 harvestedMpIndex() const;
	uint16 harvestedMpQuantity() const;
	void harvestedMpQuantity(uint16 qty);
	void resetHarvestInfos();
	void endHarvest( bool sendCloseTempImpulsion = true );
	void clearHarvestDB();
	void openHarvest();

	// get reference on deposit harvest information
	HARVEST_INFOS::CHarvestInfos& getHarvestInfos();

	// Process static actions (like harvest, faber...)
	void processStaticAction();

	///harvest deposit Result
	//void harvestDepositResult( uint16 quality );
	/// harvest corpse result
	void harvestCorpseResult( const std::vector<uint16> &qualities );

	/**
	 * Get the indices in the 'materials' vector and the quantities of the items that are currently required by a mission.
	 * If all items that match have a level tool low compared to the provided itemLevel, a message is sent to the player.
	 */
	void getMatchingMissionLootRequirements( uint16 itemLevel, const std::vector<NLMISC::CSheetId>& materials, std::vector<CAutoQuarterItemDescription>& matchingItems );

	/**
	 * Pick up a raw material in the temp inventory.
	 * \param indexInTempInv index in the temp inventory
	 * \param lastMaterial if not NULL, return true if we pick up the last material
	 * \return false if we are too encumbered to pick up
	 */
	bool pickUpRawMaterial( uint32 indexInTempInv, bool * lastMaterial = NULL );
	//@}

	/**
	 * Start a forage extraction if not already started.
	 * - If the player was not foraging, a forage progress is created.
	 * - If the player was foraging the same source, the forage progress is resumed.
	 * - If the player was foraging another source, the forage progress is reset (and the previous contents lost).
	 */
	void beginOrResumeForageSession( const NLMISC::CSheetId& materialSheetId, 
									const TDataSetRow& sourceRowId, 
									SKILLS::ESkills usedSkill, 
									bool isTheExtractor);

	/// Access the forage extraction session structure
	CForageProgress	*forageProgress();

	/// Set the current distance to prospected deposit (NULL for "not in deposit prospection mode")
	void setProspectionLocateDepositEffect( CSEffectPtr effect );

	/// Get the current distance to prospected deposit (NULL for "not in deposit prospection mode")
	CSEffectPtr getProspectionLocateDepositEffect() const;

	/// Set forage extractions counters in database
	void setExtractionProgressCounters(uint16 amountX10, uint16 qualityX10);

	/// Return the bonus extraction time coming from known passive bricks
	NLMISC::TGameCycle forageBonusExtractionTime() const;

	/**
	 * At the end of life of a forage source, let the player take the result in the temp inventory
	 * if something can be taken. Otherwise, end the forage session.
	 */
	void giveForageSessionResult( const CHarvestSource *source );

	/**
	 * If a forage session is active, reset the temp inventory
	 * and delete the forage progress structure created by beginOrResumeForageSession().
	 * Otherwise do nothing.
	 */
	void endForageSession();

	/**
	 * addXpToSkill add xpGain to a skill. 
	 * XpCatalyzer usage Enabled
	 * \param XpGain is the amount of xp added to a skill / speciality
	 * \param Skill is the name of used skill for action (or associated skill ofr specialized action used)
	 */
	void addXpToSkill( double XpGain, const std::string& Skill);

	/**
	 * addXpToSkillAndBuffer add xpGain to a skill. Do not send messages to clients, but buffer the messages
	 * XpCatalyzer usage Enabled
	 * \param XpGain is the amount of xp added to a skill / speciality
	 * \param Skill is the name of used skill for action (or associated skill ofr specialized action used)
	 */
	void addXpToSkillAndBuffer( double XpGain, const std::string& Skill, std::map<SKILLS::ESkills,CXpProgressInfos> &gainBySkill);

	/**
	 * addXpToSkillBranch add xpGain to a skill branch. If XPgain is bigger than the current skill MaxValue, then recurs.
	 * XpCatalyzer usage Disabled!
	 * \param XpGain is the amount of xp added to a skill / speciality
	 * \param Skill is the name of used skill for action (or associated skill ofr specialized action used)
	 */
	void addXpToSkillBranch( double XpGain, const std::string& Skill);

	// Set skill tree of character to max value of each skill
	void setSkillsToMaxValue();

	// Set skill tree of character to specified value
	void setSkillsToValue(const sint32& value);

	// for respawn management, need to modify _TimeDeath in cbTpAcknownledge callback
	NLMISC::TGameTime& getTimeOfDeath();
	void setTimeOfDeath( NLMISC::TGameTime t);

	// character buy a creature
	bool addCharacterAnimal( const NLMISC::CSheetId& PetTicket, uint32 Price, CGameItemPtr ptr );

	// return free slot for pet spawn or -1 if there are no free slot
	sint32 getFreePetSlot();

	// return true if can add 'delta' pets to current player pets
	bool checkAnimalCount( const NLMISC::CSheetId& PetTicket, bool sendMessage, sint32 delta );

	// CCharacter::character want spawn one of own creature
	bool spawnCharacterAnimal( uint index );

	// Specialized version of spawnCharacterAnimal() for respawnPetAfterTp()
	bool spawnWaitingCharacterAnimalNear( uint index, const SGameCoordinate& destination, uint32 destAIInstance );
		
	// Re-spawn pet of character
	void respawnPet();

	// Re-spawn pet for AI instance
	void respawnPetForInstance( uint32 InstanceNumber, const std::string& InstanceContinent );

	// Re-spawn pet after a teleport
	void respawnPetAfterTp( const SGameCoordinate& destination, uint32 destAIInstance );

	// CCharacter::character buy a creature
	void onAnimalSpawned( CPetSpawnConfirmationMsg::TSpawnError SpawnStatus, uint32 PetIdx, const TDataSetRow&	PetMirrorRow );

	/// Return the index of the animal, or ~0 if it's not an animal belonging to this character
	uint getAnimalIndex( const TDataSetRow& animalRow );

	/**
	 * Update database value (precondition: petIndex<_PlayerPets.size())
	 * Preconditions:
	 * - petIndex<_PlayerPets.size())
	 * - The current pet type has already been set in the database
	 */
	void updateAnimalHungerDb( uint petIndex );

	/// Init hunger database for all animals: needed for animals that are not spawned
	void initAnimalHungerDb();

	/// React to the hunger of the specified animal (precondition: petIndex<_PlayerPets.size()). Return true if the hunger db must be updated.
	bool onAnimalHungry( uint petIndex, bool justBecameHungry );

	/// Set satiety directly (if corresponding petCreature is NULL, will do a lookup)
	void setAnimalSatiety( uint petIndex, float value, CCreature *petCreature=NULL );

	/// Set satiety to the max (animal won't be hungry anymore)
	void setAnimalSatietyToMax(uint petIndex);

	/// Get the current and max satiety of the specified pet, or return false if the pet is not found
	bool getAnimalSatiety( uint petIndex, float& currentSatiety, float& maxSatiety );

	/// Set the race of the owner of this animal
	void setAnimalPeople( uint petIndex );

	/// Stop the specified animal if following the character and too far from him
	void checkAnimalInRange( uint petIndex );

	/// Returns true if animal is in the given stable
	bool isAnimalInStable(uint petIndex, uint16 stableId) const;

	// Player want send a command to his pet
	void sendPetCommand( CPetCommandMsg::TCommand command, uint index, bool bypassDistanceCheck=false );

	// send pack animal command
	void sendAnimalCommand( uint8 beastIndex, uint8 command );

	// Remove all spawnedpet (AIS shut down)
	void removeSpawnedPet(NLNET::TServiceId aiServiceId);

	/// Return the index of the animal corresponding to the specified ticket item, or ~0 if not found
	uint getAnimalByTicket( CGameItemPtr item );

	// \return true if corresponding pack animal is empty (return false for invalid index)
	bool checkPackAnimalEmptyInventory( uint32 petIndex );
	
	// \return true if pet animal can received a command
	bool petCommandDistance( uint32 beastIndex );
	
	// \return true if pet animal inventory is accessible
	bool petInventoryDistance( uint32 beastIndex );

	// remove pet from player corresponding to item and despawn it
	void removeAnimal( CGameItemPtr item, CPetCommandMsg::TCommand command );

	// remove pet from player corresponding to index and despawn it
	void removeAnimalIndex( uint32 beastIndex, CPetCommandMsg::TCommand command );
		
	// update coordinate for spawned pets
	void updatePetCoordinateAndDatabase();

	// update the despawn bar for dead pet animal
	void updateAnimalDespawnDb( uint petIndex );

	// remove Pet Character after his death
	void removePetCharacterAfterDeath( uint32 index );
		
	// Update database for spawned pets
	void updatePetDatabase();

	/// Update database for a spawned pet (precondition: petIndex<_PlayerPets.size())
	void updateOnePetDatabase( uint petIndex, bool mustUpdateHungerDb );
	
	// return the index of a player pet, or -1 if not found
	sint32 getPlayerPet( const TDataSetRow& petRowId ) const;

	// Set the name of the animal
	void setAnimalName( uint8 petIndex, ucstring customName );

	void sendPetCustomNameToClient(uint8 petIndex);

	// near character's pets are TP with player (continent tp)
	void allowNearPetTp();
	bool isNearPetTpIsAllowed() const;
	void forbidNearPetTp();

	// Check create parameters return false if error and set createCharErrorMsg
	static bool checkCreateParams( const CCreateCharMsg& createCharMsg, CCreateCharErrorMsg& createCharErrorMsg, uint32 userId );

	// Set start statistics and other params on character
	void setStartStatistics( const CCreateCharMsg& createCharMsg );

	// search role sheet and call start equipment and memorize actions at create character
	void searchCreateRoleSheet( EGSPD::CPeople::TPeople people, ROLES::ERole role, uint8 nbPoints, bool onlyMemorizeActions = false );

	// setStartEquipment : Set start equipment at create character
	void setStartEquipment( const SMirrorEquipment* Items );

	// memorizeStartAction : memorize start action at create character
	void memorizeStartAction( const std::vector< CStaticRole::TMemorizedSentence >& MemorizedSentences );
		
	// update regen value
	void updateRegen();

	// Same but nearly empty
	void setDummyStartCharacteristics();

	/**
	 * Eval Specialization for return Characteristics value
	 *
	 * \param value is the value to parse.
	 * \param result is the result to fill if the value has been succesfully parsed.
	 * \return UnknownValue if the value is not known, ValueError is the value evaluation failed or NoError 
	 * if it has been parsed.
	 */
	virtual TReturnState evalValue (const char *value, double &result, uint32 userData);

	/// Add a pact
	void addPact( uint8 PactNature, uint8 PactType );

	uint8 getNbPact();

	/// return true if player is exchanging
	bool isExchanging() const;

	/// invite an entity to exchange items
	void exchangeProposal();

	/// accept an exchange invitation
	void acceptExchangeInvitation();

	/// decline an exchange invitation
	void declineExchangeInvitation();

	/// the invitation is canceled for some reasons...
	void cancelExchangeInvitation();

	/// abort the exchange
	void abortExchange();

	/**
	 * start the bot chat. 
	 * \return a pointer on the current interlocutor
	 * \param chatType: type of the selected chat
	 */
	CCreature* startBotChat( BOTCHATTYPE::TBotChatFlags chatType );

	/// end the bot chat. newBotChat must be set to true if the chat is canceled because of another bot chat. closeDynChat must be true to close the current dynChat
	void endBotChat(bool newBotChat = false, bool closeDynChat = false);

	/// return the current bot chat type
	uint8 getBotChatType()const;

	///\return the current bot chat page
	uint8 getCurrentBotChatListPage();
	
	/// increment the current bot chat page
	void incCurrentBotChatListPage();

	/// check bot gift, it will unlock the 'accept' button on the client if the gift is ok
	void checkBotGift();

	/// clear bot gift
	void clearBotGift();

	/// accept the exchange (exchangeId is a counter informing the server of the information received by the client)
	void acceptExchange(uint8 exchangeId);

	///exchange money
	void exchangeMoney( const uint64& quantity);

	/// check if each character have enough room in bag for exchange
	bool validateExchange();
		
	///invalidate an exchange
	void invalidateExchange();

	/// return last tp coordinate received
	const SGameCoordinate& getTpCoordinate() const;

	/// reset TP coords
	void resetTpCoordinate();

	/// return true if player is currently being teleported
	bool teleportInProgress() const;

	// send message using string manager
	static void sendDynamicSystemMessage(const TDataSetRow &playerRowId, const std::string &msgName, const TVectorParamCheck & params = TVectorParamCheck() );
	// send message using string manager
	static void sendDynamicSystemMessage(const NLMISC::CEntityId &eid, const std::string &msgName, const TVectorParamCheck & params = TVectorParamCheck() );

	// send message using string manager
	static void sendDynamicMessageToChatGroup(const TDataSetRow &playerRowId, const std::string &msgName, CChatGroup::TGroupType type, const TVectorParamCheck & params = TVectorParamCheck() );
	// send message using string manager
	static void sendDynamicMessageToChatGroup(const NLMISC::CEntityId &eid, const std::string &msgName, CChatGroup::TGroupType type, const TVectorParamCheck & params = TVectorParamCheck() );

	// send a bit field message to client
	void sendMessageToClient( uint32 userId, NLNET::CMessage& msgout );
	
	// send the USER_CHAR message to the client (after a character is selected)
	void sendUserChar( uint32 userId, uint8 scenarioSeason, const R2::TUserRole& userRole );

	// Return the home mainland session id for a character
	TSessionId getHomeMainlandSessionId() const;

	// Set the home mainland session id
	virtual void setHomeMainlandSessionId(TSessionId homeSessionId);

	// Store the current active animation session returned by SU after char synchronisation.
	virtual void setActiveAnimSessionId(TSessionId activeAnimSessionId);
	// read the current active animation session returned by SU after char synchronisation.
	virtual TSessionId getActiveAnimSessionId();

	// check if the character is a newbie by checking the base of the position stack
	virtual bool isNewbie() const;

	// See in character_interface.h
	void applyTopOfPositionStack();

	// See in character_interface.h
	void applyAndPushNewPosition( const CFarPosition& farPos );

	// See in character_interface.h
	void applyEditorPosition( const CFarPosition& farPos );

	// See in character_interface.h
	void pushCurrentPosition();

	// See in character_interface.h
	void popAndApplyPosition();
	// See in character_interface.h
	void leavePreviousSession(TSessionId previousSesionId);

	/// See in character_interface.h
	void requestFarTP( TSessionId destSessionId, bool allowRetToMainlandIfFails=true, bool sendViaUid=false );

	// See in character_interface.h
	void returnToPreviousSession( uint32 userId, sint32 charIndex, TSessionId rejectedSessionId );

	/// spend money
	void spendMoney( const uint64 & price );

	/// get money
	const uint64 & getMoney();

	/// give money
	void giveMoney( const uint64 & money );

	/// set money
	void setMoney( const uint64 & money );
	
	/// set the number of faction point dependent of the faction
	void	setFactionPoint(PVP_CLAN::TPVPClan clan, uint32 nbPt, bool factionPVP = false);

	/// get the number of faction point given a faction
	uint32	getFactionPoint(PVP_CLAN::TPVPClan clan);

	/// set the number of pvp point
	void	setPvpPoint(uint32 nbPt);

	/// get the number of pvp point given a faction
	uint32	getPvpPoint();

	/// set the SDB path where player wins HoF points in PvP (if not empty)
	void	setSDBPvPPath(const std::string & sdbPvPPath);

	/// get the SDB path where player wins HoF points in PvP (if not empty)
	bool	getSDBPvPPath(std::string & sdbPvPPath);

	/// init faction point in client database
	void	initFactionPointDb();

	/// init pvp point in client database
	void	initPvpPointDb();

	void initOrganizationInfos();
	void setOrganization(uint32 org);
	void setOrganizationStatus(uint32 status);
	void changeOrganizationStatus(sint32 status);
	void changeOrganizationPoints(sint32 points);

	/// send faction point gain phrase to the client
	void	sendFactionPointGainMessage(PVP_CLAN::TPVPClan clan, uint32 fpGain);
	/// send faction point gain kill phrase to the client
	void	sendFactionPointGainKillMessage(PVP_CLAN::TPVPClan clan, uint32 fpGain, const NLMISC::CEntityId & victimId);
	/// send faction point lose phrase to the client
	void	sendFactionPointLoseMessage(PVP_CLAN::TPVPClan clan, uint32 fpLose);
	/// send faction point 'cannot gain yet' phrase to the client
	void	sendFactionPointCannotGainYetMessage(const NLMISC::CEntityId & victimId, uint32 remainingSeconds);

	///get the current interlocutor of the character
	const NLMISC::CEntityId & getCurrentInterlocutor();
	// set the current interlocutor
	void setCurrentInterlocutor(const NLMISC::CEntityId & interlocutor);

	///remove the entity from exchange
	void removeFromExchange();

	/**
	 * process (Memorize) pre memorized sentences
	 * \param sheetId the sheet id used by this character
	 */
	void processPreMemorizedSentences( const std::vector< CSentenceStatic > & MemorizedSentences, const std::string& SheetName );

	/**
	 * set the player in berserk state and update database
	 * \param isBerserk : the new berserk state
	 */
	void setBerserkFlag(bool isBerserk);
	
	/// return the berserk state
	bool getBerserkFlag();

	/// get the exchange view
	CExchangeView * getExchangeView();

	/// return the money in exchange
	const uint64 & getExchangeMoney() const;

	/// set exchange money amount
	void setExchangeMoney( uint64 amount );
	
	///\return the loot container
	CInventoryPtr getLootContainer();

	///\set the loot container
	void setLootContainer(CInventoryPtr lootSac);

	/// update scores infos in database
	void updateScoresInDatabase();

	/// Set database with all character information
	void setDatabase();

	/// cancel any static action in progress (ie actions canceled if the player moves like casting, harvest, faber...)
	/// type is the type of the action ( magic,...)
	void cancelStaticActionInProgress( STATIC_ACT_TYPES::TStaticActTypes type = STATIC_ACT_TYPES::Neutral, bool cancelChat = true, bool cancelLoot = true);

	/// cancel any static effect in progress (ie effects canceled if the player moves like casting, harvest, faber...)
	virtual void cancelStaticEffects();	

	/// return true if a static action is in progress
	bool staticActionInProgress() const;

	/// set the static action flag
	void staticActionInProgress(bool flag, STATIC_ACT_TYPES::TStaticActTypes type = STATIC_ACT_TYPES::Neutral);

	/// receibed emote from client
	void setEmote( MBEHAV::EBehaviour emote );

	void sendEmote( const NLMISC::CEntityId& id, MBEHAV::EBehaviour behaviour, uint16 emoteTextId, bool checkPrivilege = true );
	void sendCustomEmote( const NLMISC::CEntityId& id, MBEHAV::EBehaviour behaviour, ucstring& emoteCustomText );

	uint32 getActionsSPValue() const;
	uint32 getStartActionsSPValue() const;
	void getRoleStartActions(EGSPD::CPeople::TPeople people, ROLES::ERole role, uint8 nbPoints, std::vector<NLMISC::CSheetId> &phrases) const;
	uint32 getTotalEarnedSP() const;

	/// set the current action (fills the appropriate portion of the database)
	virtual void setCurrentAction(CLIENT_ACTION_TYPE::TClientActionType actionType,NLMISC::TGameCycle actionEndCycle );

	/// clear current action: action barre on client is no more displayed
	virtual void clearCurrentAction( );

	void setSaveDate(uint32 nTimeStamp);
	uint32 getSaveDate();

	/// set the welcome mission description (ONLY necessary when the character is created)
	/// the welcome mission description will be used by assignWelcomeMission() at the first connection
	void setWelcomeMissionDesc(TAIAlias missionAlias, TAIAlias botAlias);
	/// assign the welcome mission to the player ONCE if setWelcomeMissionDesc() have been called
	/// then this method will do nothing
	void assignWelcomeMission();

	uint getMissionsCount();
	std::map<TAIAlias, CMission*>::iterator getMissionsBegin();
	std::map<TAIAlias, CMission*>::iterator getMissionsEnd();

	///\return a mission by alias
	const CMission* getMission( TAIAlias missionId ) const;
	///\return a mission by alias
	CMission* getMission( TAIAlias missionId );

	///\return the successful missions
	const std::map< TAIAlias, TMissionHistory >& getMissionHistories();
	///add a succesful mission. DEBUG ONLY
	void addSuccessfulMissions( const CMissionTemplate & templ);

	/// check weither or not a mission has been successfully played
	bool isMissionSuccessfull(const CMissionTemplate & templ);

	/// check the last date of trying for a mission (0 if never tryied)
	NLMISC::TGameCycle getMissionLastSuccess(const CMissionTemplate & templ);

	/// build the current mission list proposed to the player
	void buildMissionList(CCreature * creature, uint16 sessionId);
	/// send mission prerequisit infos
	void sendMissionPrerequisitInfos( uint16 missionIndex );
	/// fill DB with the current mission page
	void fillMissionPage(uint16 sessionId);
	///\return the current mission list
	const std::vector<SBotChatMission> & getCurrentMissionList();
	/// add a mission to this player
	void addMission(CMissionSolo * mission);
	/// remove a mission from this player
	void removeMission(TAIAlias alias, /*TMissionResult*/ uint32 result);
	/// player abandon a mission
	void abandonMission(uint8 index);
	/// Clear the list of succesfull mission, i.e. reset the success count
	void clearSuccessfullMission();
	/// update the mission histories
	void updateMissionHistories(TAIAlias missionAlias, /*TMissionResult*/ uint32 result);

	void removeMissionFromHistories(TAIAlias missionAlias) { _MissionHistories.erase(missionAlias); }

	///process a mission event ( this function should be called each time a mission event is triggered ). 
	/// The alias parameter is useful when we want to test only a mission with a specific alias
	/// if deleteEvent is true, the first event is deleted
	bool processMissionEventList( std::list< CMissionEvent* > & eventList, bool deleteEvent ,TAIAlias alias );
	bool processMissionEvent( CMissionEvent & event, TAIAlias alias = CAIAliasTranslator::Invalid);
	// process a mission event and if not processed check if it can unblock steps for other teammate
	bool processMissionEventWithTeamMate( CMissionEvent & event, TAIAlias alias = CAIAliasTranslator::Invalid);
	// process a mission event multiple times, until all matching steps are done. Return the number of matching steps for which processMissionEvent() returned true.
	sint processMissionMultipleEvent( CMissionEvent & event, TAIAlias alias = CAIAliasTranslator::Invalid);
	/// process a mission event for the missions took by this player
	bool processMissionUserEvent(std::list< CMissionEvent* > & eventList,TAIAlias alias);
	/// process a mission event for a specific mission and optionally for a specific step
	bool processMissionStepUserEvent(std::list< CMissionEvent* > & eventList, uint missionAlias, uint32 stepIndex );
	/// make a mission advance through bot chat
	void botChatMissionAdvance( uint8 index );
	/// creat a filled exchange window BRIANCODE
	bool autoFillExchangeView();
	CMission * getMissionFromBotGift();


	/// update all previously saved missions (init of dynamic parameters)
	void updateSavedMissions();
	/// update an entry of the journal
	//void updateJournalEntry(uint8 idx,MISSION_DESC::TMissionType type);

	/// add a group with a handle (indicates when it can be despawn)
	void addHandledAIGroup(CMission *m, TAIAlias nGroupAlias, uint32 nDespawnTime);
	/// remove a group with handle (data are kept by mission for multiple handles)
	void delHandledAIGroup(CMission *m, TAIAlias nGroupAlias);
	/// remove all handled group for a specified mission
	void delAllHandledAIGroup(CMission *m);
	/// for all handles in all missions send spawn-msg to ais
	void spawnAllHandledAIGroup();
	/// for all handles in all missions send despawn-msg to ais
	void despawnAllHandledAIGroup();
	
	/// build the item trade list
	void startTradeItemSession( uint16 session );

	std::vector<CTradePhrase> & currentPhrasesTradeList();

	void addTradePageToUpdate(uint16 idx);

	/// start a trade phrase session 
	void startTradePhrases(uint16 session);

	/** 
	 * get all *Explicit* phrases offered by given rolemaster bot
	 * \param bot the rolemaster bot
	 * \param phrases the vector that will be filled with phrases sheetIds
	 */
	void getPhrasesOfferedByBot(const CCreature &bot, std::vector<CTradePhrase> &phrases);

	// return a const reference on character's item in store shop
	const CItemsForSale	&getItemInShop();

	// remove an item from shop store, character destroy it's own item
	void removeItemFromShop( uint32 identifier, uint32 quantity );

	// set filter and refresh trade list if trade occurs
	void setFilters( uint32 minQuality, uint32 maxQuality, uint32 minPrice, uint32 maxPrice, RM_CLASS_TYPE::TRMClassType minClass, RM_CLASS_TYPE::TRMClassType maxClass, RM_FABER_TYPE::TRMFType itemPartFilter, ITEM_TYPE::TItemType itemTypeFilter );
	
	// Filters for shop list
	RM_FABER_TYPE::TRMFType getRawMaterialItemPartFilter() const;
	ITEM_TYPE::TItemType getItemTypeFilter() const;
	void resetRawMaterialItemPartFilter();
	void resetItemTypeFilter();
	RM_CLASS_TYPE::TRMClassType getMinClassItemFilter() const;
	RM_CLASS_TYPE::TRMClassType getMaxClassItemFilter() const;
	uint32 getMinQualityFilter() const;
	uint32 getMaxQualityFilter() const;
	uint32 getMinPriceFilter() const;
	uint32 getMaxPriceFilter() const;
	
	/// fill a trade page
	void fillTradePage(uint16 session, bool enableBuildingLossWarning = false);

	// refresh all trade list
	void refreshTradeList();

	/// access to _CurrentTradeSession
	void	setCurrentTradeSession(uint16 session);
	uint16	getCurrentTradeSession() const;
	
	/// buy an item
	void buyItem( uint16 itemNumber, uint16 quantity );

	// destroy an item in current selected page
	void destroySaleItem( uint16 itemNumber, uint16 quantity );
		
	/// query an item price
	bool queryItemPrice( const CGameItemPtr item, uint32& price );
		
	// sell an item
	void sellItem( INVENTORIES::TInventory inv, uint32 slot, uint32 quantity, uint32 price );

	// item are sold
	void itemSolded( uint32 identifier, uint32 quantity, uint32 price, uint32 basePrice, const NLMISC::CEntityId& buyer, bool sellOffline );

	// item reach maximum time in sell store
	void itemReachMaximumSellStoreTime( uint32 identifier, uint32 quantity, bool sellOffline );

	// check sell store coherency with character, assume character is a reference
	void checkSellStore();

	/// Clear the list mission histories(for debug purpose only)
	void clearMissionHistories();

	/**
	 * test if the player has still 'n' empty slots in it's bags 
	 * \param n the number of free slots needed
	 * \param testMpConsumption tell if we should check the number of slots freed by mps consumption (default = false)
	 */
	bool testEmptySlots( uint8 n, bool testMpConsumption = false);

	/**
	 * create a beast train
	 * \param beastLeaderRow datasetRow of the train leader beast
	 * \param nbBeasts the max number of beasts in the train
	 */
	void createTrain( const TDataSetRow& beastLeaderRow, uint8 nbBeasts );

	/// get train max size
	uint8 trainMaxSize() const;

	/// add a beast in the train
	void addBeast( uint16 petIndex );

	/// get the beast train
	const std::vector<TDataSetRow> &beastTrain() const;

	/// clear the beast train
	void clearBeastTrain();

	/// increment action counter
	void incActionCounter();
	/// get action counter
	uint8 actionCounter() const;

	/// increment interface counter
	void incInterfaceCounter();
	/// get interface counter
	uint8 interfaceCounter() const;

	/// Register character name in IOS
	void registerName(const ucstring &newName = std::string(""));

	/// Mount a mount
	void mount( TDataSetRow PetRowId );

	/**
	 * Unmount a mount.
	 * If changeMountedState is false, IsMounted is not changed (useful to remember the state when teleporting).
	 * If petIndex is ~0, auto-finds the index of the first mount for which IsMounted is true.
	 */
	void unmount( bool changeMountedState=true, uint petIndex=~0 );

	// return true if the user have the privilege
	bool havePriv (const std::string &priv) const;

	//	return true if user have any privilege
	bool haveAnyPrivilege() const;
		
	///\return the continent where the player is
	CONTINENT::TContinent getCurrentContinent();

	///\return the region where the player is
	uint16 getCurrentRegion();

	bool isInPlace( uint16 place );

	///\return all the places where the player is
	const std::vector<uint16> & getPlaces();

	///\set all the places where the player is
	void setPlaces(const std::vector<const CPlace*> & places);

	///\return the stable where the player is
	uint16 getCurrentStable();
	
	///\set the continent where the player is
	void setCurrentContinent(CONTINENT::TContinent continent);

	///\set the region where the player is
	void setCurrentRegion(uint16 region);

	// \set the stable where the player is
	/// todo place
	void setCurrentStable (uint16 stable, uint16 placeId);

	// apply goo damage if character is too close than a goo path
	void applyGooDamage( float gooDistance );

	/// get the valid state of melee combat
	bool meleeCombatIsValid() const;

	/// client (in)validate melee combat
	void validateMeleeCombat(bool flag);

	/// memorize a phrase 
	void memorize(uint8 memorizationSet, uint8 index, uint16 phraseId, const std::vector<NLMISC::CSheetId> &bricks);

	//clear building phrase
	void clearCurrentPhrase();

	// get const reference on container of raw materials selected for faber 
	const std::vector<CFaberMsgItem> &getFaberRms();
	// get const reference on container of raw materials formula selected for faber 
	const std::vector<CFaberMsgItem> &getFaberRmsFormula();

	// set craft plan sheet-id
	void setCraftPlan( NLMISC::CSheetId sheet );

	// return craft plan sheet-id
	NLMISC::CSheetId getCraftPlan() const;

	// get reference on container of raw materials selected for faber 
	std::vector<CFaberMsgItem> &getFaberRmsNoConst();
	// get reference on container of raw materials formula selected for faber 
	std::vector<CFaberMsgItem> &getFaberRmsFormulaNoConst();
	
	// fill vector of const GameItem pointer with Raw material used for faber
	bool getFillFaberRms( std::vector< const CStaticItem * >& rms, std::vector< const CStaticItem * >& rmsFormula, uint16& lowerQuality ); 

	// clear Faber raw material selection
	void clearFaberRms();

	// lock Faber raw material, return true if lock success
	bool lockFaberRms();

	// unlock Faber raw material
	void unlockFaberRms();

	// consume Faber raw material
	void consumeFaberRms(bool failed = false);

	// consume an item and trigger its built-in effect
	void consumeItem( INVENTORIES::TInventory inventory, uint32 slot );

	// destroy Item currently consumed
	void destroyConsumedItem();

	// launch an admin command specified in command ticket
	void launchCommandTicket(const CStaticItem * form);

	// Exec a memorized phrase
	void executeMemorizedPhrase(uint8 memSet, uint8 index, bool cyclic, bool enchant );

	/// forget a memorizedPhrase
	void forgetPhrase(uint8 memSet, uint8 i);

	/// learn a new phrase
	void learnPhrase(const std::vector<NLMISC::CSheetId> &bricks, uint16 phraseId, const ucstring &name);

	/// delete a known phrase
	void deleteKnownPhrase(uint16 phraseId);

	///\return the index of the first free phrase slot. If no slot is free, return 0xFFFF otherwise.
	uint16 getFirstFreePhraseSlot();

	/// return const reference on known phrases
	const std::vector<CKnownPhrase>& getKnownPhrases();
	
	/// buy a phrase and add it to known phrases
	void buyPhraseByIndex( uint8 botChatIndex, uint16 knownPhraseIndex );

	/** 
	 * buy a phrase and add it to known phrases use a sheet Id to get the phrase
	 * \param phraseId the phrase sheet id
	 * \param knownPhraseIndex index of the phrase in known phrase book
	 */
	void buyPhraseBySheet( const NLMISC::CSheetId &phraseId, uint16 knownPhraseIndex );
	
	/// learn a phrase and add it's brick to known bricks
	bool learnPrebuiltPhrase( const NLMISC::CSheetId &phraseId, uint16 knownPhraseIndex , bool replace = false, bool onlyLearnBricks = false);

	/// get first empty slot in known phrase vector, increase vector size if no free slot found !
	uint16 getFirstFreeSlotInKnownPhrase();

	/// Check known and memorised phrases in cases some bricks would have been removed.
	void checkPhrases();

	///send known and memorized phrases to client
	void sendPhrasesToClient();

	/// update database for compass coordinates
	void compassDatabaseUpdate();

	/// set the compass target
	void setCompassTarget( TDataSetRow rowId);

	/// get cycle counter
	uint8 cycleCounter() const;
	/// get next counter
	uint8 nextCounter() const;
	
	/// write cycle counter in DB
	void writeCycleCounterInDB();

	/// write exec phrase in DB
	void writeExecPhraseInDB(sint16 id);

	/// write next counter in DB
	void writeNextPhraseInDB(uint8 counter);

	/// send ack on phrase execution
	void sendPhraseExecAck(bool cyclic, uint8 counter, bool execOk);

	/// get item in right hand
	virtual CGameItemPtr getRightHandItem() const;

	/// get item in left hand
	virtual CGameItemPtr getLeftHandItem() const;

	/// get ammo item
	virtual CGameItemPtr getAmmoItem() const;

	/// send custom url
	void sendUrl(const std::string &url, const std::string &salt);

	/// set custom mission param
 	void setCustomMissionParams(const std::string &missionName, const std::string &params);

 	/// add custom mission param
 	void addCustomMissionParam(const std::string &missionName, const std::string &param);

 	/// get custom mission params
 	std::vector<std::string> getCustomMissionParams(const std::string &missionName);

 	/// validate dynamic mission step sending url
 	void validateDynamicMissionStep(const std::string &url);

	/// add web command validation check
	void addWebCommandCheck(const std::string &url, const std::string &data, const std::string &salt);

	/// get web command validation check
	uint getWebCommandCheck(const std::string &url);

	/// validate web command. Return web command item index in bag if command is valid or INVENTORIES::NbBagSlots if not
	uint checkWebCommand(const std::string &url, const std::string &data, const std::string &hmac, const std::string &salt);

	/// get the available phrases
	void getAvailablePhrasesList(const std::string &brickFilter, std::vector<NLMISC::CSheetId> &selectedPhrases, EGSPD::CPeople::TPeople people = EGSPD::CPeople::Common, bool bypassBrickRequirements = false, bool includeNonRolemasterBricks = true );

	/// spend Skill points
	void spendSP(double sp, EGSPD::CSPType::TSPType type = EGSPD::CSPType::EndSPType);

	/// gain Skill points
	void addSP(double sp, EGSPD::CSPType::TSPType type);

	/// get Skill points
	double getSP(EGSPD::CSPType::TSPType type);

	/// set skill points
	void setSP(double d, EGSPD::CSPType::TSPType type );

	// dodge or parry as defense
	virtual void dodgeAsDefense( bool b);
	bool dodgeAsDefense() const;

	// used to modify defense success chance
	void parrySuccessModifier( sint32 mod );
	sint32 parrySuccessModifier() const;
	void dodgeSuccessModifier( sint32 mod );
	sint32 dodgeSuccessModifier() const;

	// used to modify craft success chance
	void craftSuccessModifier( sint32 mod );
	sint32 craftSuccessModifier() const;

	// used to modify combat action success chance
	void meleeSuccessModifier( sint32 mod );
	sint32 meleeSuccessModifier() const;
	void rangeSuccessModifier( sint32 mod );
	sint32 rangeSuccessModifier() const;
	void magicSuccessModifier( sint32 mod );
	sint32 magicSuccessModifier() const;

	// used to modify forage success chance
	void forageSuccessModifier( ECOSYSTEM::EECosystem eco, sint32 mod );
	sint32 forageSuccessModifier( ECOSYSTEM::EECosystem eco) const;

	/// set the protected slot
	virtual void protectedSlot( SLOT_EQUIPMENT::TSlotEquipment slot);
	SLOT_EQUIPMENT::TSlotEquipment protectedSlot() const;

	// set the next allowed action date
	void dateOfNextAllowedAction(NLMISC::TGameCycle date);
	// get the next allowed action date
	NLMISC::TGameCycle dateOfNextAllowedAction();

	/// player choose a re-spawn for his death character
	void respawn( uint16 index );

	/// apply respawn malus
	void applyRespawnEffects();

	// player accept resurrection by other character
	void resurrected();

	// player revives at full health at his location without death penalty
	void revive();
	
	// Buy kami or karavan pact for a respawn point
	void buyPact( const std::string& PactName ); 

	/// set forbid aura use start and end dates
	void setForbidAuraUseDates(NLMISC::TGameCycle startDate, NLMISC::TGameCycle endDate);

	/// get forbid aura use end date
	NLMISC::TGameCycle getForbidAuraUseEndDate() const;

	/// add an aura to ineffective auras
	void useAura(POWERS::TPowerType auraType, NLMISC::TGameCycle startDate, NLMISC::TGameCycle endDate, const NLMISC::CEntityId &userId);

	/// check if aura is effective, return true if effective
	bool isAuraEffective(POWERS::TPowerType auraType, NLMISC::TGameCycle &endDate, const NLMISC::CEntityId &userId);

	/// add an aura to ineffective auras
	void forbidPower(POWERS::TPowerType type, uint16 consumableFamilyId, NLMISC::TGameCycle date);
	
	/// check if aura is effective, return true if effective
	bool canUsePower(POWERS::TPowerType type, uint16 consumableFamilyId, NLMISC::TGameCycle &endDate);

	/// get nb of auras on this player
	uint8 nbAuras() const;

	/// inc nbAuras
	void incNbAura();

	/// dec nbAuras
	void decNbAura();

	/// resetPowerFlags
	void resetPowerFlags();

	void displayPowerFlags();
	
	/// reset a combat event flag 
	void resetCombatEventFlag(BRICK_FLAGS::TBrickFlag flag);

	/// set a combat event flag and it's reset date
	void setCombatEventFlag(BRICK_FLAGS::TBrickFlag flag);

	/// reset all combat flags for this character
	void resetCombatEventFlags();

	/// get combat event flags bitfield
	//uint32 getCombatEventFlags() const;

	/// return true if combat event flag is active
	bool isCombatEventFlagActive(BRICK_FLAGS::TBrickFlag flag) const;

	/// update brick flags DB value
	void updateBrickFlagsDBEntry();

	// add a link cast by this player
	virtual void addLink( CSLinkEffect * effect);

	// remove a link cast by this player
	virtual void removeLink( CSLinkEffect * effect, float factorOnSurvivalTime);

	// stop all links (a broken link last a few more seconds before being deleted)
	virtual void stopAllLinks(float factorOnSurvivalTime = 1.0f);

	/**
	 * apply the effect of the armor/shield on damage. Update the armor items if necessary
	 * \return the remaining damages
	 */
	virtual sint32 applyDamageOnArmor( DMGTYPE::EDamageType dmgType, sint32 damage, SLOT_EQUIPMENT::TSlotEquipment forcedSlot);

	/**
	 * return the malus on spell casting due to armor
	 */
	float getArmorCastingMalus();

	/// Return the damage using current armor, done by an explosion (e.g. forage source explosion)
	virtual float getActualDamageFromExplosionWithArmor( float dmg ) const;

	/// send temp inventory close impulsion to client
	void sendCloseTempInventoryImpulsion();

	/// set a fame value for the player, send info to the client.
	void setFameValuePlayer(uint32 factionIndex, sint32 playerFame, sint32 fameMax, uint16 fameTrend);
	// set the fame boundaries, send info to the client.
	//  Called when some of the CVariables are changed.
	void setFameBoundaries();
	/// reset the fame database
	void resetFameDatabase();

	/// equipment wear malus accessor
	void addWearMalus( float m );

	float wearMalus();

	/// modify current dodge modifier
	void incDodgeModifier(sint32 inc);
	/// modify current dodge modifier
	void incParryModifier(sint32 inc);

	/// current equipment adversary dodge modifier
	sint32 adversaryDodgeModifier();
	/// current equipment adversary parry modifier
	sint32 adversaryParryModifier();

	// add an effect to this entity
	virtual bool addSabrinaEffect( CSEffect *effect );
	
	/**
	 * remove an effect on this entity. The effect is not deleted nor stopped, if you
	 * want to force the effect to stop, use CSEffect::stopEffect() method
	 */
	virtual bool removeSabrinaEffect( CSEffect *effect, bool activateSleepingEffect = true );

	/** 
	 * add a bonus effect and write it in DB
	 * \param sheetId sheetId of the effect brick
	 * \param bonus true of the effectis a bonus
	 * \return index in Db, or  -1 if DB full
	 */
	sint8 addEffectInDB(const NLMISC::CSheetId &sheetId, bool bonus = true);

	/// remove bonus effect
	void removeEffectInDB(uint8 index, bool bonus = true);

	/// disable effect
	void disableEffectInDB(uint8 index, bool bonus, NLMISC::TGameCycle activationDate);

	// return the carried weight
	uint32 getCarriedWeight();
	// get the weight malus
	sint32 getWeightMalus();

	// return Pet Animal container const reference
	const std::vector< CPetAnimal >& getPlayerPets();

	/// Return the mount if the player is mounted, otherwise NULL
	CEntityBase *getMountEntity();

	// flag indicate if tp of pet near character are allowed
	bool	NearPetTpAllowed;
	
	/// return the gear latency structure
	const CGearLatency & getGearLatency();

	/// return true if the player can use an action
	virtual bool canEntityUseAction(CBypassCheckFlags bypassCheckFlags = CBypassCheckFlags::NoFlags, bool sendMessage = true) const;

	/// can entity defend ? (dodge/parry) returns true if entity can defend itself
	virtual bool canEntityDefend();

	/// get the resist value associated to effect type
	uint32 getMagicResistance(EFFECT_FAMILIES::TEffectFamily effectFamily);
	
	/// get the resist value associated to damage type
	uint32 getMagicResistance(DMGTYPE::EDamageType dmgType);

	/// get last X pos in DB (for teammates)
	sint32 getLastPosXInDB() const;
	/// get last Y pos in DB (for teammates)
	sint32 getLastPosYInDB() const;

	/// set last X pos in DB (for teammates)
	void setLastPosXInDB(sint32 x);
	/// set last Y pos in DB (for teammates)
	void setLastPosYInDB(sint32 y);
	
	/// \name Friend/Ignore lists management.

	// @{

		/// add a player to friend list by name
		void addPlayerToFriendList(const ucstring &name);

		/// add a player to ignore list by name
		void addPlayerToIgnoreList(const ucstring &name);

		/// add a player to ignore list by Id
		void addPlayerToIgnoreList(const NLMISC::CEntityId &id);
		
		/// add a player to league list by id
		void addPlayerToLeagueList(const NLMISC::CEntityId &id);

		/// add a player to friend list by id
		void addPlayerToFriendList(const NLMISC::CEntityId &id);

		/// get a player from friend or ignore list, by contact id
		const NLMISC::CEntityId	&getFriendByContactId(uint32 contactId);
		const NLMISC::CEntityId	&getIgnoreByContactId(uint32 contactId);
		
		/// remove player from friend list
		void removePlayerFromFriendListByContactId(uint32 contactId);
		void removePlayerFromFriendListByEntityId(const NLMISC::CEntityId &id);

		void setInRoomOfPlayer(const NLMISC::CEntityId &id);
		const NLMISC::CEntityId& getInRoomOfPlayer();

		/// get if player have acces to room
		bool playerHaveRoomAccess(const NLMISC::CEntityId &id);

		/// add room acces to player
		void addRoomAccessToPlayer(const NLMISC::CEntityId &id);
		
		/// remove room acces to player
		void removeRoomAccesToPlayer(const NLMISC::CEntityId &id, bool kick);

		/// remove player from league list
		void removePlayerFromLeagueListByContactId(uint32 contactId);
		void removePlayerFromLeagueListByEntityId(const NLMISC::CEntityId &id);

		/// remove player from ignore list
		void removePlayerFromIgnoreListByContactId(uint32 contactId);
		void removePlayerFromIgnoreListByEntityId(const NLMISC::CEntityId &id);
		
		/// clear friend list
		void clearFriendList();
		
		/// clear Ignore list
		void clearIgnoreList();
		
		/// clear friendOf list
		void clearFriendOfList();

		/// set player status in contact list
		void setContactOnlineStatus( const NLMISC::CEntityId &id, bool online );

		/// get num friend
		uint	getNumFriends() const;

		/// get friend by index
		const NLMISC::CEntityId &getFriend(uint16 index) const;
		
		/// get num ignored
		uint	getNumIgnored() const;
		
		/// get ignored player by index
		const NLMISC::CEntityId &getIgnoredPlayer(uint16 index) const;

		/// does the entity friend(ify) this player?
		bool hasInFriendList(const NLMISC::CEntityId &player) const;

		/// does the entity ignore this player?
		bool hasInIgnoreList(const NLMISC::CEntityId &player) const;
		
		/// player is going online or offline
		void online(bool onlineStatus);

		/// player last connection date (from SU, exactly like on mysql db)
		void setLastConnectionDate(uint32 date);

		/// player is permanently erased, unreferenced it from all contact lists
		void destroyCharacter();

	// @}

	// send message of the day to new connected player
	void sendMessageOfTheDay();

	/// called internally when the user connects
	void onConnection();

	/// called internally when the user crash or disconnect from the server
	void onDisconnection(bool bCrashed);
	
	/// change rolemaster type
	void setRolemasterType(EGSPD::CSPType::TSPType type);

	/// set the afk state of the user
	void setAfkState( bool isAfk );

	/**
	 * Return the skills of this entity
	 * \return CSkills& skills of the entity
	 */
	CSkills& getSkills();

	/// return the best skill of this entity
	SKILLS::ESkills getBestSkill() const;

	/// return the best skill of this entityfor dodge
	SKILLS::ESkills getSkillUsedForDodge() const;

	/// get skill value if used for dodge
	sint32 getSkillEquivalentDodgeValue(SKILLS::ESkills skill) const;

	/// get current dodge level
	sint32 getCurrentDodgeLevel() const;
	/// get current parry level
	sint32 getCurrentParryLevel() const;
	/// get current dodge level
	sint32 getBaseDodgeLevel() const;
	/// get current parry level
	sint32 getBaseParryLevel() const;

	/**
	 * get the value of the specified skill
	 * \param skill the skill
	 * \return skillValue (if skill not found return 0)
	 */
	sint32 getSkillValue(SKILLS::ESkills skill) const;

	/**
	 * get the base value of the specified skill
	 * \param skill the skill
	 * \return skillBaseValue (if skill not found return 0)
	 */
	sint32 getSkillBaseValue(SKILLS::ESkills skill) const;

	/**
	 * get the value of the best child skill
	 * \param skill the skill. WARNING: THIS SKILL MUST BE VALID
	 * \return skillValue (if skill not found return 0)
	 */
	sint32 getBestChildSkillValue(SKILLS::ESkills skill) const;

	/**
	 * get the best skill value between parent and chill for the asked skill
	 * \param skill we want the best value
	 * \return skillValue (0 if all parent skill are locked)
	 */
	sint32 getBestSkillValue(SKILLS::ESkills skill);

	/**
	 * return the first unlocked parent skill of the given skill
	 * \param skill the skill to look for (the leaf)
	 * \return first unlocked parent of the skill (or the skill itself if unlocked)
	 */
	SKILLS::ESkills getFirstUnlockedParentSkill(SKILLS::ESkills skill) const;

	/**
	 * change hp with a delta value
	 * \param deltaValue the delta value applied on hp
	 * \return bool true if the entity died from the changement
	 */
	virtual bool changeCurrentHp(sint32 deltaValue, TDataSetRow responsibleEntity = TDataSetRow());

	// aggroable mode
	void setAggroable(bool aggroable, bool forceUpdate = false);
	bool isAggroable();
	void setAggroableOverride(sint8 aggroableOverride);
	sint8 getAggroableOverride() const;
	void sendAggroable();
	bool isAggroableOverridden();
	
	/// set player intangible end date
	void setIntangibleEndDate(NLMISC::TGameCycle date);
	/// get player intangible end date
	NLMISC::TGameCycle getIntangibleEndDate() const;

	// get the backup of the who sees me property
	void setWhoSeesMeBeforeTP(const uint64 &whoSeesMe);
	// get the backup of the who sees me property
	const uint64 &whoSeesMeBeforeTP() const;
	// reset the backup of the who sees me property
	void resetWhoSeesMeBeforeTP();

	/// return the user admin properties
	CAdminProperties & getAdminProperties();

	/// accessors to the monitoring CSR
	const TDataSetRow&  getMonitoringCSR();
	void setMonitoringCSR(const TDataSetRow& csr);

	/// get death penalties
	const CDeathPenalties & getDeathPenalties() const;

	float nextDeathPenaltyFactor() const;
	void resetNextDeathPenaltyFactor();
	void setNextDeathPenaltyFactor(float factor);
	uint32 updateDeathPenaltyResorption();

	/// set duration (in days) needed to lose all death penalty
	void setDPLossDuration(float duration);

	/// get duration (in days) needed to lose all death penalty
	float getDPLossDuration() const;

	/* 
	 * Get persistent item services (ie services that need to be provided each time player reconnects)
	 * It is a handle used by CItemServiceManager. It should not be used by another class.
	 **/
	std::vector<NLMISC::CSheetId> & getPersistentItemServices();

	/// return the player room of this user
	CPlayerRoomInterface	&getRoomInterface();

	EGSPD::CFameContainerPD &getPlayerFamesContainer();

	bool checkCharacterStillValide( const char * msgError);

	/// get display stat flag on Xp gain for player
	bool logXpGain() const;
	/// set display stat flag on Xp gain for player
	void logXpGain(bool b);

	void decAggroCount();
	void incAggroCount();
	uint8 getAggroCount();

	/// returns true of player is in water
	bool isInWater() const;

	/// update isInWater flag
	void updateIsInWater();

	/// player enters water
	void entersWater() const;

	/// returns true if player is sitting
	bool isSitting() const;
	
	/// check to see if guild can belong to the guild ID provided.
	/// setToNone- if true will change any mismatched allegiances to "None", otherwise no change is made (default: false).
	/// returns true if allegiances are allowed, false otherwise.
	bool canBelongToGuild( uint32 guildId, bool setToNone = false );

	/// set the guild id of the player
	/// returns true if the change was allowed, returns false if the change was not allowed.
	bool setGuildId( uint32 guildId );

	/// get the guild id of the player
	uint32 getGuildId() const;

	/// updates the guild flag field in the mirror, according to character clan and preferences
	void updateGuildFlag() const;
	
	/// send FBT status to the client
	void sendBetaTesterStatus();

	// send windermeer old community status
	void sendWindermeerStatus();

	/**
	 * send a reserved title status to the client
	 * \param title			title to update
	 * \param available		true if the player can select this title on the client
	 */
	void sendReservedTitleStatus(CHARACTER_TITLE::ECharacterTitle title, bool available) const;

	/// get used TP ticket slot in bag, INVALID_INVENTORY_SLOT = unused
	uint32	getTpTicketSlot() const;

	/// reset used TP ticket slot, necessary to allow user to use another ticket
	void resetTpTicketSlot();

	/// set building exit zone
	void setBuildingExitZone(uint16 zoneIdx);

	/// get building exit zone
	uint16 getBuildingExitZone() const;

	// unlock new skill added by patch, if parent skill reach max value, children skill must be unlocked
	void checkSkillTreeForLockedSkill();


	// set the character hair. return true on success
	bool setHair(uint32 hairValue);
	// set the character tatoo return true on success
	bool setTatoo(uint32 tatooValue);
	// set the hair color of the user return true on success
	bool setHairColor(uint32 colorValue);


	typedef std::vector< TBrickParam::IIdPtr > CBrickPropertyValues;
	typedef std::map< TBrickParam::TValueType, CBrickPropertyValues > CBrickProperties;

	/// Return the values of properties of the known bricks of the specified family (useful for bonus values)
	void getPropertiesFromKnownBricks( BRICK_FAMILIES::TBrickFamily brickFamily,
									   CBrickProperties& results );


	// set pet status for debug feature
	void setPetStatus( uint32 index, CPetAnimal::TStatus status );

	void petTpAllowed( uint32 index, bool allowed );

	void setSpawnPetFlag( uint32 index );

	// return if hair cute price discount apply
	bool getHairCutDiscount() const;
	
	// reset hair cut discount price
	void resetHairCutDiscount();

	/// database update, called every 2 tick, send delta to the clients
	void databaseUpdate();

	/// HP/STA/SAP/FOCUS bar update, called every 2 tick, send fast-accurate bar info to the client
	void barUpdate();
		
	CCharacterEncyclopedia	&getEncyclopedia();
	
	CCharacterGameEvent		&getGameEvent()	;

	CCharacterRespawnPoints	&getRespawnPoints();

	const CCharacterRespawnPoints	&getRespawnPoints()	const;

	/// add player in queue id
	void addInQueue(uint32 id);

	/// remove player from queue id
	void removeFromQueue(uint32 id);

	/// get mission Queues
	const std::vector<uint32> &getMissionQueues() const;

	uint32 getEnterCriticalZoneProposalQueueId() const;
	void setEnterCriticalZoneProposalQueueId(uint32 queueId);

	/// update parry skill and level
	void updateParry(ITEMFAMILY::EItemFamily family, SKILLS::ESkills skill);

	// Jewel equipment or skill or region are changed, recompute protection and resistances
	void updateMagicProtectionAndResistance();

	// return magic protection of character (clamped to max value)
	uint32 getMagicProtection( PROTECTION_TYPE::TProtectionType magicProtectionType ) const;

	// set magic protection of character (unclamped)
	void setUnclampedMagicProtection( PROTECTION_TYPE::TProtectionType magicProtectionType, sint32 value );

	// return unclamped magic protection of a character
	uint32 getUnclampedMagicProtection( PROTECTION_TYPE::TProtectionType magicProtectionType ) const;

	// return unclamped magic resistance of a character
	uint32 getUnclampedMagicResistance( RESISTANCE_TYPE::TResistanceType magicResistanceType ) const;

	// return clamped magic resistance of a character
	uint32 getMagicResistance(RESISTANCE_TYPE::TResistanceType magicResistanceType) const;

	/// return NbNonNullClassificationTypesSkillMod
	uint8 getNbNonNullClassificationTypesSkillMod() const;
	
	/// get specific skill modifier for given race
	virtual sint32 getSkillModifierForRace(EGSPD::CPeople::TPeople people) const;

	/// get currently consumed item if any
	CGameItemPtr getConsumedItem() const;
	
	/// reset currently consumed item if any
	void resetConsumedItem(bool unlock = false);

	/// can player use consumable family ? returns true if allowed false otherwise
	bool canUseConsumableFamily(uint16 family, NLMISC::TGameCycle &endDate);

	/// disable a consumable family until 'date'
	void disableConsumableFamily(uint16 family, NLMISC::TGameCycle date);

	/// remove disabled consumable family past date
	void updateConsumableFamily();

	///\name PVP methode related
	//@{
	/// return the user PVP interface
	CPVPInterface &getPVPInterface();
	const CPVPInterface & getPVPInterface() const;
	// priviledge PVP mode
	void setPriviledgePVP( bool b );
	bool priviledgePVP();
	// full PVP mode
	void setFullPVP( bool b );
	/// set the current PVP zone where the player is
	void setCurrentPVPZone(TAIAlias alias);
	/// get the current PVP zone where the player is
	/// returns an invalid alias if the player is not in a PVP zone
	TAIAlias getCurrentPVPZone() const;
	/// set the current outpost zone where the player is
	void setCurrentOutpostZone(TAIAlias alias);
	/// get the current outpost zone where the player is
	/// returns an invalid alias if the player is not in a outpost zone
	TAIAlias getCurrentOutpostZone() const;
	/// player enters in a PVP zone, send appropriate client message
	void enterPVPZone(uint32 pvpZoneType) const;
	/// character enter in versus pvp zone, player must choose a clan
	void openPVPVersusDialog() const;
	/// get priviledgePvp
	bool getPriviledgePVP() const {return _PriviledgePvp;};
	/// get fullPvp
	bool getFullPVP() const {return _FullPvp;};
	/// get character pvp flag
	bool getPVPFlag( bool updatePVPModeInMirror = true ) const;
	/// change pvp flag
	void setPVPFlag( bool pvpFlag );
	/// reset all pvp timers and mode
	void resetPVPTimers();
	/// pvp action are made before the end of timer for remove PvP flag
	void pvpActionMade();
	/// set database pvp flag
	void setPVPFlagDatabase();
	/// set PVP recent action flag
	void setPVPRecentActionFlag(CCharacter *target = NULL);
	/// get PvP recent action flag (true if player involved in PVP action recently)
	bool getPvPRecentActionFlag() const;
	/// character are killed in PvP situation
	void killedInPVP();
	/// return the region where character is killed in PvP the last time
	uint16 getKilledPvPRegion();
	/// get allegiance of character
	/// first member of pair is religion allegiance, second member of pair is people allegiance
	std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> getAllegiance() const;
	/// Sets new declared cult, returns if parameter was within bounds, and thus if new clan as set.
	bool setDeclaredCult(PVP_CLAN::TPVPClan newClan);
	/// Sets new declared civ, returns if parameter was within bounds, and thus if new clan as set.
	bool setDeclaredCiv(PVP_CLAN::TPVPClan newClan);
	/// Set an allegiance to neutral from indetermined status, used for process message from client want set it's allegiance to neutral when it's idetermined, over case are managed by missions. 
	void setAllegianceFromIndeterminedStatus(PVP_CLAN::TPVPClan allegiance);
	/// true if pvp safe zone active for character
	bool getSafeInPvPSafeZone() const;
	/// set pvp safe zone to true
	void setPvPSafeZoneActive();
	/// clear pvp zone safe flag
	void clearSafeInPvPSafeZone();
	/// get pvp fames allies
	TYPE_PVP_CLAN getPVPFamesAllies();
	/// get pvp fames ennemys
	TYPE_PVP_CLAN getPVPFamesEnemies();
	/// update the clan in visuale property
	void updatePVPClanVP() const;
	//@}

	/// Verifies that player can retain their declared clan allegiances.  Also sends warning message if fame gets too low.
	/// returns true if allegiances were kept, false if an allegiance was ended.
	bool verifyClanAllegiance(PVP_CLAN::TPVPClan theClan, sint32 newFameValue);

	/// set outpost alias
	void setOutpostAlias( uint32 id );
	/// stores the time when player leaves outpost zone
	void startOutpostLeavingTimer();
	/// refresh the leaving timer if active
	void refreshOutpostLeavingTimer();
	/// stop the timer (happens when user re-enter the zone for instance)
	void stopOutpostLeavingTimer();
	/// set the temporary outpost alias
	void setOutpostAliasBeforeUserValidation( TAIAlias outpostId );
	/// true when timer is over
	bool outpostLeavingDurationElapsed();
	/// return true if player's guild is in conflict with outpost. attacker will get true if the guild is in conflict and the guild is attacking the outpost
	bool isGuildInConflictWithOutpost( TAIAlias outpostId, bool &guildIsAttacker );
	/// open a dialog box on the client when user enter in a different outpost zone to ask him to choose his side
	void outpostOpenChooseSideDialog( TAIAlias outpostId );
	/// user who entered into an outpost zone has choosen his side
	void outpostSideChosen( bool neutral, OUTPOSTENUMS::TPVPSide side );
	
	void setDuelOpponent( CCharacter * user );
	CCharacter * getDuelOpponent() const;
	
	/// character log stats accessors
	uint32 getFirstConnectedTime() const;
	uint32 getLastConnectedTime() const;
	uint32 getLastConnectedDate() const;
	uint32 getPlayedTime() const;

	const std::string& getLangChannel() const;
	void setLangChannel(const std::string &lang);

	const std::string& getNewTitle() const;
	void setNewTitle(const std::string &title);

	std::string getFullTitle() const;

	std::string getTagA() const;
	void setTagA(const std::string &tag);

	std::string getTagB() const;
	void setTagB(const std::string &tag);

	std::string getTagPvPA() const;
	void setTagPvPA(const std::string &tag);

	std::string getTagPvPB() const;
	void setTagPvPB(const std::string &tag);

	uint32 getOrganization() const;
	uint32 getOrganizationStatus() const;
	const std::list<TCharacterLogTime>& getLastLogStats() const;
	void updateConnexionStat();
	void setDisconnexionTime();		

	/// update outpost admin flag in client database
	void updateOutpostAdminFlagInDB();

	void haveToUpdateItemsPrerequisit( bool b );

	void channelAdded( bool b );
	bool isChannelAdded() const;

	uint8 getNbUserChannels() const { return _NbUserChannels; };
	void addUserChannel() { _NbUserChannels++; };
	void removeUserChannel() { _NbUserChannels--; };

	std::vector<SItemSpecialEffect> lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::TItemSpecialEffect effectType) const;

	/// return true if the given item is an xp catalyser which has been activated
	bool isAnActiveXpCatalyser( CGameItemPtr item );
	
	void setShowFactionChannelsMode(TChanID channel, bool s);
	bool showFactionChannelsMode(TChanID channel) const;

	// from offline command
	void contactListRefChangeFromCommand(const NLMISC::CEntityId &id, const std::string &operation);


	////////////////////////////////////////////////////////////////
	// Methods from "character_inventory_manipulation.h"

	class CItemPtr : public NLMISC::CRefCount
	{
	public:
		CItemPtr() {}
		~CItemPtr() { if(_Item != NULL) _Item.deleteItem(); }

		void setItem(CGameItemPtr item) { _Item = item; }

	private:
		CGameItemPtr _Item;
	};

	/// init player inventories
	void initInventories();

	/// init client database for player inventories
	void initInventoriesDb();

	/// release player inventories
	void releaseInventories();

	/// get player inventory from ID. Can return NULL if inventory is not available.
	CInventoryPtr getInventory(INVENTORIES::TInventory invId) const;

	// pick-up item from ground, the item taking is the targeted item
	bool pickUpItem( const NLMISC::CEntityId& entity );

	/// pick-up close
	/// close the temp inv and restore all items to the looted container
	void pickUpItemClose();

	/// pickup on entity, for loot or harvest
	void itemPickup( const NLMISC::CEntityId& entity, bool harvest = false );

	/// destroy an item (or several items in a stack)
	void destroyItem(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity, bool sendChatMessage = false);

	/// used for selectItems / destroyItems
	class CItemSlotId
	{
	public:
		INVENTORIES::TInventory		InvId;
		uint32						Slot;
		uint32						Quality;
		bool	operator<(const CItemSlotId &o) const
		{
			return Quality<o.Quality;
		}
	};

	/** check the presence of an item (or several items in a stack) by its sheetId/quality
	 *	- select only items of quality>=quality param
	 *	- can specify multiple inventory where to select the items
	 *	\param itemList if not NULL, the array is filled with the items found (nb: not cleared => allow append of different invs)
	 *	\return the number of items / stack elements that matchs
	 */
	uint selectItems(INVENTORIES::TInventory invId, NLMISC::CSheetId itemSheetId, uint32 quality, std::vector<CItemSlotId> *itemList= NULL);
	
	/** destroy a list of items (up to maxQuantity to destroy)
	 *	- if maxQuantity is not -1 (infinity code), sort itemList by quality to destroy first the lowest ones
	 *	- don't send any chat message
	 *	\return the number of items / stack element really destroyed (0 if none found)
	 */
	uint destroyItems(const std::vector<CItemSlotId> &itemSlots, uint32 maxQuantity=-1);

	/**
	 * Move an item from an inventory to another (with autostack support).
	 * \param srcInvId : source inventory
	 * \param srcSlot  : source slot
	 * \param dstInvId : destination inventory
	 * \param dstSlot  : destination slot (should be CInventoryBase::INSERT_IN_FIRST_FREE_SLOT most of the time)
	 * \param quantity : quantity to move
	 */
	void moveItem(INVENTORIES::TInventory srcInvId, uint32 srcSlot, INVENTORIES::TInventory dstInvId, uint32 dstSlot, uint32 quantity);

	/// return true if the player can put a non-dropable item in the given inventory (private inventories)
	bool canPutNonDropableItemInInventory(INVENTORIES::TInventory invId) const;

	/// equip character
	void equipCharacter(INVENTORIES::TInventory invId, uint32 dstSlot, uint32 bagSlot, bool sendChatMessage = false);

	/// unequip character
	void unequipCharacter(INVENTORIES::TInventory invId, uint32 slot, bool sendChatMessage = false);

	/// apply item modifiers(bonus etc..)
	void applyItemModifiers(const CGameItemPtr & item);
	
	/// remove item modifiers
	void removeItemModifiers(const CGameItemPtr & item);

	// return weight of equipped weapons
	uint32	getWeightOfEquippedWeapon();

	// check compatibility with slot for equip an item
	bool checkItemValidityWithSlot( const NLMISC::CSheetId& sheet, INVENTORIES::TInventory inv, uint16 slot );
	// check compatibility with equipment slot for equip an item
	bool checkItemValidityWithEquipmentSlot( const NLMISC::CSheetId& sheet, uint16 Slot );
	// check compatibility between item in left and right hand for equip an item
	bool checkRightLeftHandCompatibility( const std::vector<std::string> itemRight, const std::vector<std::string> itemLeft );
	// low level methode for check compatibility between item and slot
	bool checkIfItemCompatibleWithSlots( const std::vector<std::string> itemSlot, std::vector< uint16 > slots );
	// check ammo compatibility with weapon
	bool checkIfAmmoCompatibleWithWeapon( SKILLS::ESkills ammoType, SKILLS::ESkills weaponType );
		
	// check pre-required for equip an item
	bool checkPreRequired(const CGameItemPtr & item, bool equipCheck = false );

	/// check integrity of exchanging actors
	/// \param exchangeWithBot : if not NULL return true if player is exchanging with a bot
	bool checkExchangeActors(bool * exchangeWithBot = NULL) const;

	/// add an item from bag to exchange
	void itemBagToExchange(uint32 bagSlot, uint32 exchangeSlot, uint32 quantity);

	/// remove an item from exchange to bag
	void itemExchangeToBag(uint32 exchangeSlot);

	/// get the item in specified inventory and slot
	CGameItemPtr getItem(INVENTORIES::TInventory invId, uint32 slot) const;

	/// wear item in right hand, returns true if item has been destroyed
	bool wearRightHandItem(float wearFactor = 1.0f);
	/// wear item in left hand, returns true if item has been destroyed
	bool wearLeftHandItem(float wearFactor = 1.0f);
	/// wear armor
	void wearArmor(float wearFactor = 1.0f);
	/// wear shield
	void wearShield(float wearFactor = 1.0f);
	/// wear jewels
	void wearJewels(float wearFactor = 1.0f);

	/**
	 * Find an item or a stack of items matching the specified family. If found, the slot is set in returnedSlot.
	 * If not found, the return value is NULL.
	 * Garanties:
	 * - If not NULL, the returned item is either a stack or a food item.
	 * - If single item: item family is requested, and getStaticForm() returns non-null.
	 * - If stack: there is at least one child, and the first child is non-null and has the request item family, and getStaticForm() returns non-null on it.
	 */
	CGameItemPtr getItemByFamily(INVENTORIES::TInventory invId, ITEMFAMILY::EItemFamily family, uint32 & returnedSlot);

	/// consume the given quantity of ammo in sheath
	void consumeAmmo(uint32 quantity = 1);

	/// lock one item or some stack item
	bool lockItem(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity);

	/// unlock one item or some stack item
	void unLockItem(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity);

	/// create an item
	/// WARNING: if quantity is greater than item max stack size, max stack size will be used instead
	CGameItemPtr createItem(uint16 obtainedQuality, uint32 quantity, const NLMISC::CSheetId & obtainedItem, const NLMISC::CEntityId & creatorId = NLMISC::CEntityId::Unknown, const std::string * phraseId = NULL);

	/// add item to an inventory
	/// \return true if item was successfully added
	/// WARNING: item can be nullified in case of autostacking
	bool addItemToInventory(INVENTORIES::TInventory invId, CGameItemPtr & item, bool autoStack = true);

	/// remove an item from an inventory
	//// \return a pointer on the removed item
	CGameItemPtr removeItemFromInventory(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity);

	/// create item in the given inventory (autostack)
	/// \return true if item was successfully created
	/// WARNING: if quantity is greater than item max stack size, max stack size will be used instead
	bool createItemInInventory(INVENTORIES::TInventory invId, uint16 obtainedQuality, uint32 quantity, const NLMISC::CSheetId & obtainedItem, const NLMISC::CEntityId & creatorId = NLMISC::CEntityId::Unknown, const std::string * phraseId = NULL);

	/// create item in the given inventory and return it (no autostack)
	/// \return the item or NULL if creation failed
	/// WARNING: if quantity is greater than item max stack size, max stack size will be used instead
	CGameItemPtr createItemInInventoryFreeSlot(INVENTORIES::TInventory invId, uint16 obtainedQuality, uint32 quantity, const NLMISC::CSheetId & obtainedItem, const NLMISC::CEntityId & creatorId = NLMISC::CEntityId::Unknown, const std::string * phraseId = NULL);

	/// action on an item in the temp inventory (move it to bag)
	void itemTempInventoryToBag(uint32 scrSlot);

	/// clear temp inventory
	void clearTempInventory();

	/// get all items in temp inventory
	void getAllTempInventoryItems();

	/// return true if temp inventory is empty
	bool tempInventoryEmpty();

	/// increment the version of a slot
	void incSlotVersion(INVENTORIES::TInventory invId, uint32 slot);

	/// send item infos. For slotId (combination of inventory and slot), see explanation in CItemInfos
	void sendItemInfos( uint16 slotId );

	/// return true if the player wears an item with the specified sheetId
	bool doesWear(const NLMISC::CSheetId & sheetId) const;

	/// create a crystallized action item in temp inventory
	void createCrystallizedActionItem(const std::vector<NLMISC::CSheetId> & action);
				
	///create a sap reload item in temp inventory
	void createRechargeItem(uint32 sapRecharge);

	/// check if enchant or recharge an item 
	void enchantOrRechargeItem(INVENTORIES::TInventory invId, uint32 slot);

	/// helper for enchant and recharge item do not call directly!
	bool checkSlotsForEnchantOrRecharge(INVENTORIES::TInventory invId, uint32 slot, bool enchant);

	/// enchantItem with crystallized action in inventory
	void enchantItem(INVENTORIES::TInventory invId, uint32 slot);

	/// rechargeItem with recharge sap load item
	void rechargeItem(INVENTORIES::TInventory invId, uint32 slot);

	/// proc enchantment of an item
	void procEnchantment();

	/// use an item. Param is the slot in the bag
	void useItem(uint32 slot);

	/// stop use of an item
	void stopUseItem( bool isRingCatalyser );

	/// use a teleport
	void useTeleport(const CStaticItem & form);

	/// use an xp catalyser
	void useXpCatalyser( uint32 slot );

	/// get temp inventory mode
	TEMP_INV_MODE::TInventoryMode tempInventoryMode() const;

	/// test if we can enter in the given temp inventory mode
	/// \return true if we can
	bool canEnterTempInventoryMode(TEMP_INV_MODE::TInventoryMode mode);

	/// set temp inventory mode
	/// \return true if it successed
	bool enterTempInventoryMode(TEMP_INV_MODE::TInventoryMode mode);

	/// reset temp inventory mode
	void leaveTempInventoryMode();

	// End of methods from "character_inventory_manipulation.h"
	////////////////////////////////////////////////////////////////
	
	/// Ring stuff <=> useful to update the player DB to indicate the current speed of the incarnated creature
	void setNpcControl(const NLMISC::CEntityId& eid);
	void setStopNpcControl();

	/// return true if character has moved during a static action
	bool hasMovedDuringStaticAction()
	{
		return ( _EntityState.X() != _OldPosX || _EntityState.Y() != _OldPosY );
	}
	
	///////////////////
	// Protected methods
	///////////////////
protected:
	friend class CPlayerManager;
	friend class CCharacterGameEvent;
	friend class CCharacterVersionAdapter;
	friend class CRingRewardPointsImpl;
	
	virtual void kill(TDataSetRow killerRowId = TDataSetRow());
	void setEntityRowId( const TDataSetRow& r );
	void resetPvPFlag();


	///////////////////
	// Private methods
	///////////////////
private:
	/// wear an item
	void wearItem(INVENTORIES::TInventory invId, uint32 slot, float wearFactor);

	// check return true if params match with pre requisit for wearing an equipment
	bool checkRequiredForSkill( uint32 skillRequired, SKILLS::ESkills requiredSkill );
		
	void contactListRefChange(const NLMISC::CEntityId &id, TConctactListAction actionType);
	
	/// return true if player is ignored by the given entity
	bool isIgnoredBy(const NLMISC::CEntityId &id);
	/// return true if given entity has player on friend list
	bool isFriendOf(const NLMISC::CEntityId &id);
	/// player is referenced as friend by the given entity
	void referencedAsFriendBy( const NLMISC::CEntityId &id);
	/// player is no longer referenced as friend by the given entity
	void unreferencedAsFriendBy( const NLMISC::CEntityId &id);

	/// remove player from friend list
	void removePlayerFromFriendListByIndex(uint16 index);
	/// remove player from league list
	void removePlayerFromLeagueListByIndex(uint16 index);
	/// remove player from ignore list
	void removePlayerFromIgnoreListByIndex(uint16 index);
	
	
	/// player is ignored by given player
	void addedInIgnoreListBy( const NLMISC::CEntityId &id);
	/// player no longer ignored by given player
	void removedFromIgnoreListBy( const NLMISC::CEntityId &id);	

	/// Set all character stats modifiers to initale states
	void resetCharacterModifier();

	/// recompute all Max value
	void computeMaxValue();

	/// apply regenerate and clip currents value
	void applyRegenAndClipCurrentValue();

	/// character is dead
	void deathOccurs( void );

	/// process items decay
	void processItemsDecay();

	/// reset combat flags when date reached
	void updateCombatEventFlags();

	/// update power flags tick info
	void setPowerFlagDates();

	/// update aura flags tick info
	void setAuraFlagDates();

	/// update power and aura flags
	void updatePowerAndAuraFlags();

	/**
	 * init the database  
	 */
	void initDatabase();

	/**
	 * evaluate current sentence
	 */
	void evaluateSentence();

	/**
	 * check the player has the mps used in faber and lock them in inventory
	 */
//	bool checkAndLockMps();

	/**
	 * unlock mps
	 */
//	void unlockMps();

	// Apply Xp gain to skill
	void applyXpGainToSkill( SSkill *skill, float coeff, uint16 stageIdx, double XpGain, const std::string& Skill );
		
	// Apply Xp gain to skill and speciality used
	void applyXpGainToSkillAndSpeciality( SSkill *skill, SSkill *speciality, uint16 stageIdx, uint16 stageIdxSpe, double XpGain, const std::string& Skill, const std::string& Speciality );

	// remove the items given during an exchange
	void removeExchangeItems(std::vector<CGameItemPtr>& itemRemoved, std::vector< CPetAnimal >& PlayerPetsRemoved);

	// add the items gained during an exchange
	void addExchangeItems(CCharacter* trader,std::vector<CGameItemPtr>& itemToAdd, std::vector< CPetAnimal >& PlayerPetsAdded);

	/// get creator name dynamic string Id
	uint32 getCreatorNameId( const NLMISC::CEntityId &creatorId);

	/// send init impulsion for friend and ignore lists
	void sendContactListInit();

	/// Compute the 'visual' online state of a friend character
	TCharConnectionState isFriendCharVisualyOnline(const NLMISC::CEntityId &friendId);

	// From SU on char name change. sendContactListInit(), if must update
	void syncContactListWithCharNameChanges(const std::vector<NLMISC::CEntityId> &charNameChanges);
	
	/// send a message to client to remove an entry in a contact list
	void sendRemoveContactMessage(uint32 contactId, uint8 listNumber);

	/// give character all the bricks he should have after creation (according to point repartition)
	void addCreationBricks();

	// save character if save period are elapsed
	void saveCharacter();

	/// Method called at start of apply() to prepare character for loading
	void prepareToLoad();

	/// Method called at end of apply() to fixup database, visual properties, etc etc
	void postLoadTreatment();

	/// Method called for lock all ticket in bag inventory in Ring shard mode
	void lockTicketInInventory();

	/// Called by post load treatment or after EGS has received the init of the EID translator
	/// Validate coherency of the contact list against the Eid Translator content.
	/// (ie. remove any character that is no more existing)
	void validateContactList();

	/// check characteristics and scores match possessed bricks
	void checkCharacAndScoresValues();

	/// check scores match possessed bricks
	void checkScoresValues( SCORES::TScores score, CHARACTERISTICS::TCharacteristics charac );

	/// compute the best skill
	void computeBestSkill();

	/// compute skill to use for dodge
	void computeSkillUsedForDodge();

	/// compute forage bonus bricks
	void computeForageBonus();

	/// compute misc bonus bricks
	void computeMiscBonus();

	/// increment characteristic
	void changeCharacteristic( CHARACTERISTICS::TCharacteristics charac, sint16 mod );

	/// process a training brick (increase scores, charac...)
	void processTrainingBrick( const CStaticBrick *brick, bool sendChatMessage = true );

	/// process a new received bonus brick
	void processForageBonusBrick( const CStaticBrick *brick );

	/// process a new received bonus brick
	void processMiscBonusBrick( const CStaticBrick *brick );

	/// unprocess a training brick (increase scores, charac...)
	void unprocessTrainingBrick( const CStaticBrick *brick, bool sendChatMessage = true );

	/// unprocess a new received bonus brick
	void unprocessForageBonusBrick( const CStaticBrick *brick );

	/// unprocess a new received bonus brick
	void unprocessMiscBonusBrick( const CStaticBrick *brick );

	/// process items in temp inventory
	void logAndClearTempInventory();

	/** 
	 * buy a phrase and add it to known phrases use a sheet Id to get the phrase
	 * \param phraseId the phrase sheet id
	 * \param knownPhraseIndex index of the phrase in phrase book
	 * \param testRestrictions if true then test if player match phrase requirements
	 * \return true if the phrase has been bought, false if an error occured
	 */
	bool buyRolemasterPhrase( const NLMISC::CSheetId &phraseId, uint16 knownPhraseIndex, bool testRestrictions );

	/// reset exchange: clear and reset exchange views, clear exchange related members and client database
	void resetExchange();

	/// add an entity that must be saved with me when i am saved
	void	addEntityToSaveWithMe(const NLMISC::CEntityId &entityId);
	void	clearEntitiesToSaveWithMe();
	const std::set<NLMISC::CEntityId>	&getEntitiesToSaveWithMe() const;

	// fill vector of const GameItem pointer with material used for faber
	bool fillFaberMaterialArray( std::vector<CFaberMsgItem>& materialsSelectedForFaber, std::vector< const CStaticItem * >& materials, uint16& lowerMaterialQuality );

	// get region resistance modifier
	sint32 getRegionResistanceModifier( RESISTANCE_TYPE::TResistanceType resistanceType );
		
	CFarPositionStack& getPositionStack();

	/// see addXpToSkillInternal
	enum TAddXpToSkillMode
	{
		AddXpToSkillSingle= 0,		// Allow usage of XPCatalyzer and send message immediatly
		AddXpToSkillBuffer,			// Allow usage of XPCatalyzer and backup XP report in gainBySkill map
		AddXpToSkillBranch			// Do not allow usage of XPCatalyzer, send message immediatly with truncated XP gain values
	};

	// add xp bonus from catalyser ( see addXpToSkillInternal )
	bool addCatalyserXpBonus( uint32& slot, SSkill * skill, double xpGain, double& xpBonus, uint32& stackSizeToRemove, 
									 uint32& catalyserLvl, uint32& catalyserCount );

	/**
	 * addXpToSkillInternal add xpGain to a skill
	 * \param XpGain is the amount of xp added to a skill / speciality
	 * \param ContSkill is the name of used skill for action (or associated skill ofr specialized action used)
	 * \param addXpMode see TAddXpToSkillMode 
	 * \param bufferize if true enable the method to merge multiple xp gain on a same skill
	 * \param messageBuffer if bufferize is on, store infos in this map, and do not send progression messages
	 * \return the remaining XP --that can be distribued-- to same or upper skills. Warning! if allowXPCat, the returned value 
	 *	may even be bigger than the original XpGain!
	 */
	double addXpToSkillInternal( double XpGain, const std::string& ContSkill, TAddXpToSkillMode addXpMode, std::map<SKILLS::ESkills,CXpProgressInfos> &gainBySkill );

	/// Initialize the specified pet inventory, if it is valid
	bool initPetInventory(uint8 index);

	///////////////////
	// Public members
	///////////////////
	/// Property database, it's a mirror of client database
public:

	/// Player-bank database
//	CCDBSynchronised				_PropertyDatabase;
	CBankAccessor_PLR				_PropertyDatabase;

	CInventoryUpdaterForCharacter	_InventoryUpdater;

	////////////////////////////
	// For debug
	NLMISC::TGameCycle XpGainRate;
	std::string TestProgressSkill;
	std::string TestProgressSpeciality;

	// Only for testing tool
	std::string					TestSheetName;
	CStaticCharacters			TestcharactersSheet;

	/// Stack of normal positions (mainland, outlands) (mutable because of store() that pushes/pops _EntityState)
	mutable CFarPositionStack		PositionStack;


	// Obsolete:
	// Editor position (mutable because of store() that pushes/pops _EntityState)
	//mutable CFarPosition			EditorPosition;

	void			setSessionUserRole( R2::TUserRole mode );
	R2::TUserRole	sessionUserRole() const;

	/** If the session id is SessionLockPositionStack, the current position won't be saved.
	 * It should be the case in ur_editor mode (see setSessionUserRole()).
	 */
	void			setSessionId( TSessionId sessionId );
	TSessionId		sessionId() const;

	// Same as set SessionId but works event if current sessin is a edition session
	void			setCurrentSessionId( TSessionId sessionId );
	TSessionId		currentSessionId() const;

	// get the position stack
	const CFarPositionStack& getPositionStack() const;

	bool TestProgression;

	bool isShopingListInProgress() const { return _ShoppingList != 0; };

	void setFinalized(bool isFinalized) { _LoadingFinish = isFinalized; };
	bool isFinalized() const { return _LoadingFinish; };

	void setFriendVisibility(TFriendVisibility val) { _FriendVisibility = val; }
	const TFriendVisibility& getFriendVisibility() const { return _FriendVisibility; }

	void setFriendVisibilitySave(uint8 val) { if (val < NB_FRIEND_VISIBILITY) _FriendVisibility = (TFriendVisibility)val; }
	uint8 getFriendVisibilitySave() const { return (uint8)_FriendVisibility; }

	//////////////////
	// Private members
	//////////////////
private:

	R2::TUserRole						_SessionUserRole;
	sint32								_HPB; //backup hp of player for mainland, for be independant of what happend in ring session
	
	// If SessionLockPositionStack, the current position won't be saved (ex: _SessionUserRole == ur_editor)
	TSessionId							_SessionId;
	// the value of the current session (valid even when _SessionUserRole == ur_editor)
	TSessionId							_CurrentSessionId;

	/// The active anim session returned by SU after syncUserChars
	TSessionId							_ActiveAnimSessionId;

	CTimer								_TickUpdateTimer;
	CTimer								_DbUpdateTimer;
	CTimer								_DeathPenaltyTimer;
	CTimer								_BarUpdateTimer;
	
	struct CBotGift
	{
		NL_INSTANCE_COUNTER_DECL(CBotGift);
	public:

		MISSION_DESC::TMissionType	Type;
		TAIAlias					MissionAlias;
		std::set<uint32>			StepsIndex; // when step "any" we must consider several steps
	};
	
	/// true if the player entered the game
	bool								_Enter;

	/// initial ai instance for ring
	uint32								_StartupInstance;

	/// The temporary fame container
	EGSPD::CFameContainerPD				*_Fames;

	// The declared Cult and Civilization information for the character for fame purposes.
	PVP_CLAN::TPVPClan					_DeclaredCult;
	PVP_CLAN::TPVPClan					_DeclaredCiv;

	/// Time in gametime until the behaviour not change, using has temporizing deaths sequence management
	NLMISC::TGameTime					_TimeDeath;

	// money
	uint64								_Money;

	uint32								_FactionPoint[PVP_CLAN::EndClans-PVP_CLAN::BeginClans+1];

	uint32								_PvpPoint;

	uint32								_Organization;
	uint32								_OrganizationStatus;
	uint32								_OrganizationPoints;

	std::string							_LangChannel;

	std::string							_NewTitle;
	std::string							_TagPvPA;
	std::string							_TagPvPB;
	std::string							_TagA;
	std::string							_TagB;

	/// SDB path where player wins HoF points in PvP (if not empty)
	std::string							_SDBPvPPath;
	
	// Keep pointer on the container being looted
	CInventoryPtr						_LootContainer;
	NLMISC::CEntityId					_EntityLoot;

	/// if this player has an invitation for another team, keep the team here
	NLMISC::CEntityId 					_TeamInvitor;

	/// if this player has an invitation for League, keep the invitor here
	NLMISC::CEntityId 					_LeagueInvitor;

	// id of the current team
	CMirrorPropValueAlice< uint16, CPropLocationPacked<2> >	_TeamId;
	
	TChanID								_LeagueId;

	/// temp values used to test if team bars need an update or not
	mutable uint8						_OldHpBarSentToTeam;
	mutable uint8						_OldSapBarSentToTeam;
	mutable uint8						_OldStaBarSentToTeam;

	/// temp values used to test if Players bars need an update or not
	uint8								_BarSentToPlayerMsgNumber;

	sint32								_OldHpBarSentToPlayer;
	sint32								_OldSapBarSentToPlayer;
	sint32								_OldStaBarSentToPlayer;
	sint32								_OldFocusBarSentToPlayer;
	
	NLMISC::TGameCycle					_LastTickSaved;
	NLMISC::TGameCycle					_LastTickCompassUpdated;

	/// permanent score modifiers (given by bricks)
	sint32								_ScorePermanentModifiers[SCORES::NUM_SCORES];


///\todo (nico): bot chat list should be regrouped in a structure deleted when bot chat ends
	// Trading structures
	CCharacterShoppingList				*_ShoppingList;
	// special trading struct for phrases
	std::vector<CTradePhrase>			_CurrentPhrasesTradeList;
	// current trade session
	uint16								_CurrentTradeSession;
	// list of trade pages waiting update
	std::list<uint16>					_TradePagesToUpdate;
	// if trader is a rolemaster, keep rolemaster main type for SP spending
	EGSPD::CSPType::TSPType				_RolemasterType;
	
	// current page in the displayed list
	uint8								_CurrentBotChatListPage;
	// type of the current bot chat
	uint8								_CurrentBotChatType;
	// current mission list
	std::vector<SBotChatMission>		_CurrentMissionList;
	
	// the missions took by the player
	EGSPD::CMissionContainerPD			*_Missions;

	std::map<TAIAlias, TMissionHistory>	_MissionHistories;

	// temporary data to keep the welcome mission description between the character creation and the first connection
	CWelcomeMissionDesc					_WelcomeMissionDesc;

	// Save date is used to know if we have to launch crash or player_reconnect handler
	uint32								_SaveDate;

	//the entity the character is talking to
	NLMISC::CEntityId					_CurrentInterlocutor;

	//entity who proposed an exchange
	NLMISC::CEntityId					_ExchangeAsker;

	// exchange view
	NLMISC::CSmartPtr<CExchangeView>	_ExchangeView;

	//id of the last exchange action
	uint8								_ExchangeId;
	
	//has the character accepted the exchange
	bool								_ExchangeAccepted;
	
	// money in exchange
	uint64								_ExchangeMoney;
	
	/// current bot gift of the player
	CBotGift							*_BotGift;
	
	
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	SGameCoordinate				_TpCoordinate;

	// currently used TP ticket bag slot, INVALID_SLOT_INDEX = unused
//	sint16						_TpTicketSlot;
	uint32						_TpTicketSlot;

	// is the player berserk
	bool						_IsBerserk;

	/// Number of bits currently available for sending database impulsions to the client
	CMirrorPropValueBase< uint16 > _AvailImpulseBitsize;

	/// old pos X
	mutable sint32				_OldPosX;
	/// old pos Y
	mutable sint32				_OldPosY;

	/// last X position written in DB
	sint32						_LastPosXInDB;
	/// last Y position written in DB
	sint32						_LastPosYInDB;

	/// set to true if the character is doing an action canceled by any movement (X or Y)
	mutable bool				_StaticActionInProgress;
	/// the current type of static action if any
	mutable STATIC_ACT_TYPES::TStaticActTypes	_StaticActionType;

	// \name Harvest related attributes
	//@{
	/// bool indicating if the harvest interface is opened
	bool						_HarvestOpened;
	/// the mp source id
	NLMISC::CEntityId			_MpSourceId;
	/// the mp source sheetId
	NLMISC::CSheetId			_MpSourceSheetId;
	/// index of the harvested Mp
	uint8						_MpIndex;
	/// quantity of mp harvested
	uint16						_HarvestedQuantity;
	/// flag indicating if the player is harvesting a deposit
	bool						_HarvestDeposit;
	/// if harvesting a deposit, the skill used
	SKILLS::ESkills				_DepositSearchSkill;
	/// Harvest deposit information
	HARVEST_INFOS::CHarvestInfos _DepositHarvestInformation;
	//@}

	/// the tick value of the nex decay of player items
//	NLMISC::TGameCycle			_NextDecayTickTime;

	// character fames
//	CCharacterFames				_Fames;

	// nb of activestatic effects 
	uint8						_NbStaticActiveEffects;

	// the stable where the player is and Place/Region where stable is
	uint16						_CurrentStable;
	uint16						_PlaceOfCurrentStable; 
	uint16						_RegionOfCurrentStable;

	// Pet Animal of player (Mektoub mount and packer, other pets
	std::vector< CPetAnimal >	_PlayerPets;

	/// vector of the creatures in the current player train
	std::vector<TDataSetRow>	_BeastTrain;
	/// max number of beasts in the train
	uint8						_TrainMaxSize;

	/// counter for the current action of the player (used for macros)
	uint8						_ActionCounter;

	/// counter for interface actions (used for inventory and building_sentence)
	uint8						_InterfaceCounter;

	/// the continent where the player is
	CONTINENT::TContinent		_CurrentContinent;
	/// the region where the player is
	uint16						_CurrentRegion;
	/// the places where the player is
	std::vector< uint16 >		_Places;

	// Last tick send message for inform player is suffer goo damage
	NLMISC::TGameCycle			_LastTickSufferGooDamage;
	
	/// is melee combat valid or not
	bool						_MeleeCombatIsValid;
	
	/// memorized phrases
	CPlayerPhraseMemory			_MemorizedPhrases;

	/// known bricks
	std::set<NLMISC::CSheetId>	_KnownBricks;

	/// known Phrases
	std::vector<CKnownPhrase>	_KnownPhrases;

	/// known Phrases
	std::set<NLMISC::CSheetId>	_BoughtPhrases;

	/// cycle counter
	uint8						_CycleCounter;
	/// next phrase counter
	uint8						_NextCounter;

	// sheet id of craft plan
	NLMISC::CSheetId			_CraftPlan;

	// vector of inventory and index and quantity of raw material selected for faber
	std::vector<CFaberMsgItem>	_RmSelectedForFaber;
	// vector of inventory and index and quantity of raw material formula selected for faber
	std::vector<CFaberMsgItem>	_RmFormulaSelectedForFaber;

	uint16						_SavedVersion;

	// keep starting charac values, use them as an offset of current charac values when listing charac bricks player can buy
	std::vector<uint8>			_StartingCharacteristicValues;

	/** 
	 * date when the player will be able to start a new action, before this date new actions will be refused
	 * this is set when a player disengage from a combat before the combat action latency has ended (anti exploit)
	 */
	NLMISC::TGameCycle			_DateOfNextAllowedAction;

	/// Structure for forage sessions (NULL if there is no forage in progress)
	CForageProgress				*_ForageProgress;
	
	/// Distance to current prospected deposit
	CSEffectPtr					_ProspectionLocateDepositEffect;

	//// Bonus extraction time coming from known passive bricks (not saved but computed by computeForageBonus() at loading)
	NLMISC::TGameCycle			_ForageBonusExtractionTime;

	/// date since when this player can't use auras
	NLMISC::TGameCycle			_ForbidAuraUseStartDate;
	/// date when this player will be able to use auras again
	NLMISC::TGameCycle			_ForbidAuraUseEndDate;

	/// vector of power types and the date when player will be able to use them again
	CPowerActivationDateVector	_ForbidPowerDates;

	/// vector of auras ineffective on this player
	CAuraActivationDateVector	_IneffectiveAuras;

	/// ctecor of disabled consumables families
	CConsumableOverdoseTimerVector	_ConsumableOverdoseEndDates;

	/// disabled modifiers (such as ineffective auras)
	CModifiersInDB				_ModifiersInDB;

	/// nb of auras affecting this player
	uint8						_NbAuras;

	/// nb of users channels
	uint8						_NbUserChannels;

	/// last webcommand index
	uint32						_LastWebCommandIndex;

 	std::map<std::string, std::string>	_CustomMissionsParams;

	// for a power/combat event, stores start and end ticks
	struct CFlagTickRange {

		uint32 StartTick;
		uint32 EndTick;

		uint32 OldStartTick;
		uint32 OldEndTick;

		CFlagTickRange()
		{
			StartTick = 0;
			EndTick = 0;
			OldStartTick = 0;
			OldEndTick = 0;
		}
	};
	std::vector<CFlagTickRange>	_CombatEventFlagTicks;
	std::vector<CFlagTickRange>	_PowerFlagTicks;
	
	// class used for apply/store for persistent effects
	CPersistentEffect			_PersistentEffects;

	/// timer for each flag, when date is reached, reset flag
//	NLMISC::TGameCycle			_CombatEventResetDate[BRICK_FLAGS::NbCombatFlags];

	/// total malus due to wearing equipment
	float						_WearEquipmentMalus;

	/// compass current target (none, target, teammates...)
	TDataSetRow					_CompassTarget;

	/// bulk of the character
//	sint32						_Bulk;
	/// total carried weight
//	sint32						_CarriedWeight;

	CGearLatency				*_GearLatency;

	// modifiers due to equipment
	sint32						_DodgeModifier;
	sint32						_ParryModifier;
	sint32						_AdversaryDodgeModifier;
	sint32						_AdversaryParryModifier;

	// is character in a coma ("dead" but can still be healed and resurrected)
	bool						_IsInAComa;

	// Aggroable override (-1=computed, 0=not aggroable, 1=aggroable)
	sint8							_AggroableOverride;
	bool							_Aggroable;
	
	// Contact Structure
	struct CContactId
	{
		NLMISC::CEntityId			EntityId;		// Id used for server/storage
		uint32						ContactId;		// Id used for client/server communication
	};
	uint32							_ContactIdPool;

	std::vector<NLMISC::CEntityId>	_RoomersList; // Players who have acces to player's room
	NLMISC::CEntityId				_inRoomOfPlayer;

	// friends list
	std::vector<CContactId>	_FriendsList;
	// league list
	std::vector<CContactId>	_LeagueList;
	// ignore list
	std::vector<CContactId>	_IgnoreList;
	// list of players for whom this player is in the friendlist (to update online status and to remove from players list if this char is permanently deleted)
	// Yoyo: WARNING!! this list may be false (may contain too much entities), see contactListRefRefChange()
	std::vector<NLMISC::CEntityId>	_IsFriendOf;
	// list of players for whom this player is in the friendlist (to remove from players list if this char is permanently deleted)
	// Yoyo: WARNING!! this list may be false (may contain too much entities), see contactListRefRefChange()
	std::vector<NLMISC::CEntityId>	_IsIgnoredBy;

	// specialized skill points (not in CSkills for version management !)
	double							_SpType[EGSPD::CSPType::EndSPType];
	uint32							_SpentSpType[EGSPD::CSPType::EndSPType];

	// keep points repartition among different types (fight, magic, craft, harvest) during creation (2bits per type)
	uint8							_CreationPointsRepartition;

	/// Intangible state end date
	NLMISC::TGameCycle				_IntangibleEndDate;
	
	/// who sees me property, before TP
	uint64							_WhoSeesMeBeforeTP;

	/// special properties for GM ( 4 bytes )
	CAdminProperties				*_AdminProperties;

	/// CSR monitoring this player mission
	TDataSetRow						_MonitoringCSR;

	/// death penalties
	CDeathPenalties					*_DeathPenalties;

	/// part of xp gain used to update death penalty
	float							_DeathPenaltyXpGainFactor;

	/// if true, next death penalty will not be applied
	float							_NextDeathPenaltyFactor;

	/// duration (in days) needed to lose all death penalty
	float							_DPLossDuration;

	/// player room
	CPlayerRoomInterface			*_PlayerRoom;

	/// display stats on Xp gain for player
	bool							_LogXpGain;

	/// number of creatures attacking the player
	uint8							_AggroCount;

	/// guild id of the player
	uint32							_GuildId;
	/// guild id of the player
	bool							_UseFactionSymbol;
	
	/// bool, set to true if player is in water
	mutable bool					_PlayerIsInWater;

	/// best skill
	SKILLS::ESkills					_BestSkill;
	/// best skill used for dodge
	SKILLS::ESkills					_SkillUsedForDodge;

	/// current dodge level
	sint32							_CurrentDodgeLevel;
	/// base dodge level
	sint32							_BaseDodgeLevel;
	/// current parry level
	sint32							_CurrentParryLevel;
	/// base parry level
	sint32							_BaseParryLevel;
	/// current parry skill
	SKILLS::ESkills					_CurrentParrySkill;

	/// current protection for each type of magic damage and max magic damage absorption gived by jewels
	uint32							_MagicProtection[PROTECTION_TYPE::NB_PROTECTION_TYPE];
	uint32							_MaxAbsorption;

	// current resistance for each type of magic resistance
	uint32							_MagicResistance[RESISTANCE_TYPE::NB_RESISTANCE_TYPE];

	sint32							_BaseResistance;

	/// currently consumed item slot
	sint32							_ConsumedItemSlot;
	/// currently consumed item inventory
	INVENTORIES::TInventory			_ConsumedItemInventory;

	/// slot of xp catalyser used
	uint32							_XpCatalyserSlot;
	/// slot of ring xp catalyser used
	uint32							_RingXpCatalyserSlot;

	/// backup last used weight malus
	sint32							_LastAppliedWeightMalus;

	std::vector<TDataSetRow>		_TargetingChars;

	// items placed in shop store by character
	CItemsForSale					*_ItemsInShopStore;

	// shop list filters
	RM_FABER_TYPE::TRMFType			_RawMaterialItemPartFilter;
	ITEM_TYPE::TItemType			_ItemTypeFilter;
	RM_CLASS_TYPE::TRMClassType		_MinClass;
	RM_CLASS_TYPE::TRMClassType		_MaxClass;
	uint32							_MinQualityFilter;
	uint32							_MaxQualityFilter;
	uint32							_MinPriceFilter;
	uint32							_MaxPriceFilter;

	uint16							_BuildingExitZone;

	// used for force respawn player who are in a mainland in town of this mainland
	bool							_RespawnMainLandInTown;

	CCharacterEncyclopedia			*_EncycloChar;

	CCharacterGameEvent				*_GameEvent;

	CCharacterRespawnPoints			*_RespawnPoints;

	/// persistent item services, ie services that need to be provided each time player reconnects
	std::vector<NLMISC::CSheetId>	_PersistentItemServices;

	/// number of specific type modifiers on skill
	uint8							_NbNonNullClassificationTypesSkillMod;

	/// specific types skill modifiers
	sint32							_ClassificationTypesSkillModifiers[EGSPD::CClassificationType::EndClassificationType];

	/// to know if item pre-requisits have to be recomputed (as after a skill/charac update)
	bool							_HaveToUpdateItemsPrerequisit;

	///\name PVP related members
	//@{
	/// interface between player and PVP system
	CPVPInterface					*_PVPInterface;
	// set pvp mode for priviledge player
	bool							_PriviledgePvp;
	// set full pvp mode for player
	bool							_FullPvp;
	// flag PVP, true for player involved in Faction PVP and other PVP
	bool							_PVPFlag;
	// time of last change in pvp safe
	NLMISC::TGameCycle				_PVPSafeLastTimeChange;
	bool							_PVPSafeLastTime;
	bool							_PVPInSafeZoneLastTime;
	// time of last change in pvp flag (for prevent change PVP flag exploits)
	NLMISC::TGameCycle				_PVPFlagLastTimeChange;
	// time of pvp flag are setted to on (for prevent change PVP flag exploits)
	NLMISC::TGameCycle				_PVPFlagTimeSettedOn;
	// time of the last PVP action made (for prevent PVE / PVP exploits)
	NLMISC::TGameCycle				_PVPRecentActionTime;
	// all pvp flags (ally and enemy) when players do a pvp curative action
	uint32							_PVPFlagAlly;
	uint32							_PVPFlagEnemy;
	// character safe if is in pvp safe zone
	bool							_PvPSafeZoneActive;
	// player changed his faction tag, we have to update pvp mode
	bool							_HaveToUpdatePVPMode;
	// counter for database management
	uint32							_PvPDatabaseCounter;
	// last civ points written in database
	sint32							_LastCivPointWriteDB;
	// last cult points written in database
	sint32							_LastCultPointWriteDB;
	/// the PVP zone where the player is
	TAIAlias						_CurrentPVPZone;
	/// the outpost zone where the player is
	TAIAlias						_CurrentOutpostZone;
	/// region where player character are killed in PvP situation
	uint16							_RegionKilledInPvp;
	//@}

	bool							_ChannelAdded;

	bool							_LoadingFinish;

	/// if true, enable display of channel faction for users with priviledge
	std::map<TChanID, bool>	_FactionChannelsMode;

	/// time when user left outpost zone (0 means not valid)
	NLMISC::TGameCycle				_OutpostLeavingTime;
	/// temporary outpost id(user doesn't have chosen his side yet)
	TAIAlias						_OutpostIdBeforeUserValidation;

	// duel opponent
	CCharacter						* _DuelOpponent;

	TAIAlias						_LastCreatedNpcGroup;

	TFriendVisibility               _FriendVisibility;

	// :KLUDGE: ICDBStructNode non-const 'coz getName and getParent are not
	// const methods. See CCDBSynchronised::getICDBStructNodeFromName for more
	// info.
//	typedef ICDBStructNode* TDbReminderData;
//	struct CCharacterDbReminder
//	{
//		NL_INSTANCE_COUNTER_DECL(CCharacterDbReminder);
//	public:
//
//		/*
//		 * PackAnimal
//		 */
//		struct CBeast
//		{
//			TDbReminderData TYPE;
//			TDbReminderData UID;
//			TDbReminderData STATUS;
//			TDbReminderData HP;
//			TDbReminderData BULK_MAX;
//			TDbReminderData POS;
//			TDbReminderData HUNGER;
//			TDbReminderData DESPAWN;
//		};
//
//		struct CPackAnimal
//		{
//			CBeast			BEAST[4];
//		};
//
//		CPackAnimal		PACK_ANIMAL;
//
//
//
//		/*
//		 * CharacterInfo
//		 */
//		struct CScores
//		{
//			TDbReminderData BaseRegen[SCORES::NUM_SCORES];
//			TDbReminderData Regen[SCORES::NUM_SCORES];
//			TDbReminderData BaseScore[SCORES::NUM_SCORES];
//			TDbReminderData MaxScore[SCORES::NUM_SCORES];
//		};
//
//		struct CSkills
//		{
//			TDbReminderData	BaseSkill[SKILLS::NUM_SKILLS];
//			TDbReminderData	Skill[SKILLS::NUM_SKILLS];
//			TDbReminderData	ProgressBar[SKILLS::NUM_SKILLS];
//		};
//
//		struct CCharacterInfo
//		{
//			CScores		SCORES;
//			CSkills		SKILLS;
//			
//			TDbReminderData	DodgeBase;
//			TDbReminderData	DodgeCurrent;
//			TDbReminderData	ParryBase;
//			TDbReminderData	ParryCurrent;
//		};
//
//		CCharacterInfo	CHARACTER_INFO;
//
//
//		/*
//		 * Target
//		 */
//		struct CTarget
//		{
//			TDbReminderData UID;
//			TDbReminderData HP;
//			TDbReminderData SAP;
//			TDbReminderData STA;
//			TDbReminderData FOCUS;
//			TDbReminderData CONTEXT_VAL;
//			TDbReminderData PLAYER_LEVEL;
//		};
//
//		CTarget			TARGET;
//
//		struct CDebugPing
//		{
//			TDbReminderData PING;
//		};
//
//		CDebugPing		DATABASE_PING;
//
//		TDbReminderData KnownBricksFamilies[BRICK_FAMILIES::NbFamilies];
//
//		/*
//		 * Modifier
//		 */
//		struct CBonus
//		{
//			TDbReminderData Sheet[12];
//			TDbReminderData Disable[12];
//			TDbReminderData DisableTime[12];
//		};
//
//		struct CMalus
//		{
//			TDbReminderData Sheet[12];
//			TDbReminderData Disable[12];
//			TDbReminderData DisableTime[12];
//		};
//
//		struct CModifier
//		{
//			TDbReminderData TotalMalusEquip;
//			CBonus	Bonus;
//			CMalus	Malus;
//		};
//
//		CModifier	Modifiers;
//
//		struct CDisableConsumable
//		{
//			TDbReminderData Family[12];
//			TDbReminderData DisableTime[12];
//		};
//
//		CDisableConsumable DisableConsumable;
//
//		/// Constructor
//		CCharacterDbReminder();		
//	};

//	static void ensureDbReminderReady();

//	static CCharacterDbReminder* _DataIndexReminder;

	// temporary boolean for allow buy one hair cute for one dapper for correct bug of haircut of patch 1_2_1
	bool				_HairCuteDiscount;

	/// keep the Ids of the mission queues in which is this player
	std::vector<uint32>	_MissionsQueues;

	/// keep here the queue for which this player currently has an enter critical zone proposal
	uint32				_EnterCriticalZoneProposalQueueId;

	/// List of entities that must be saved at same time than me
	std::set<NLMISC::CEntityId>		_EntitiesToSaveWithMe;

	uint32 _FirstConnectedTime;						//first connected time in second since midnight (00:00:00), January 1, 1970
	uint32 _LastConnectedTime;						//last connected time in second since midnight (00:00:00), January 1, 1970 (change each tick update)
	uint32 _LastConnectedDate;						//last connected time in second since midnight (00:00:00), January 1, 1970 (never change after login, exactly like in mysql db)
	uint32 _PlayedTime;								//cumulated played time in second
	mutable std::list<TCharacterLogTime> _LastLogStats;		//keep n login/duration/logoff time

	sint64 _BrickFamilyBitField[BRICK_FAMILIES::NbFamilies]; // bit fields for known bricks
	sint64 _InterfacesFlagsBitField;

	uint8 _RingSeason;
	// Ring info used for updating client DB on the speed of the curently controlled creature
	NLMISC::CEntityId _NpcControlEid;	
	uint32 _LastTickNpcControlUpdated;

	/// success modifiers from consumables
	sint32 _ParrySuccessModifier;
	sint32 _DodgeSuccessModifier;
	sint32 _CraftSuccessModifier;
	sint32 _MeleeSuccessModifier;
	sint32 _RangeSuccessModifier;
	sint32 _MagicSuccessModifier;
	std::vector<sint32> _ForageSuccessModifiers;

	
public:
	uint32 getLastDisconnectionDate();


private:
	TAIAlias _SelectedOutpost;
public:
	TAIAlias getSelectedOutpost() const;
	void setSelectedOutpost(TAIAlias alias);
	
public:
//	static CCharacterDbReminder* getDataIndexReminder();
	// Add/Update  a channel in the user DB. For private usage by dynchat
	void setDynChatChan(TChanID id, uint32 name, bool readOnly);
	// Remove a channel in the user DB. For private usage by dynchat
	void removeDynChatChan(TChanID id);
	// change the weather seen by a player (0 for auto-weather, locally computed, which is the default)
	void setWeatherValue(uint16 value);

public:
	// The following 'property' is public because it is in fact an abstract interface, allowing access to
	// a well protected private object that manages the ring reward points implementation
	CRingRewardPoints RingRewardPoints;

private:
	/// general invisibility flag for super users (GMs...)
	bool			_Invisibility;
	/// General aggroable flag for persistence
	sint8			_AggroableSave;
	/// General god flag for persistence
	bool			_GodModeSave;
public:

	void			setWebCommandIndex(uint32 index) { _LastWebCommandIndex = index;}
	uint32			getWebCommandIndex() const { return _LastWebCommandIndex;}


	bool			getInvisibility() const	{ return _Invisibility;}
	/// Set the invisibility flag, NB : just for persistence, do not change nothing.
	void			setInvisibility(bool invisible)
	{ 
		_Invisibility = invisible;
		CBankAccessor_PLR::getUSER().setIS_INVISIBLE(_PropertyDatabase, _Invisibility);
	}

	sint8			getAggroableSave() const	{ return _AggroableSave;}
	/// Set the aggroable save flag, NB : just for persistence, do not change nothing.
	void			setAggroableSave(sint8 aggroable)		{ _AggroableSave = aggroable;}

	bool			getGodModeSave() const	{ return _GodModeSave;}
	/// Set the gobmode save flag, NB : just for persistence, do not change nothing.
	void			setGodModeSave(bool godMode)	{ _GodModeSave = godMode;}

	/// Test the character against mission prerequisits for the specified list of mission giver NPCs
	void			sendNpcMissionGiverIconDesc( const std::vector<uint32>& npcKeys );

	/// Inform the client that an event that might change mission availability just occured
	void			sendEventForMissionAvailabilityCheck();

	/// Send the current timer period if is different from the default or force is true
	void			sendNpcMissionGiverTimer(bool force);
};


typedef NLMISC::CSmartPtr<CCharacter> CCharacterPtr;
typedef NLMISC::CRefPtr<CCharacter> CCharacterRefPtr;

#include "character_inlines.h"

#endif // CHARACTER_H
