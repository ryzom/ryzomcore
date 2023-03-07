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



#ifndef RY_CREATURE_H
#define RY_CREATURE_H

// game share
#include "game_share/action_nature.h"
#include "game_share/sp_type.h"
#include "game_share/timer.h"
#include "game_share/bot_chat_types.h"
#include "game_share/pvp_clan.h"
#include "game_share/fame.h"

#include "entity_manager/entity_base.h"
#include "mission_manager/ai_alias_translator.h"
#include "harvestable.h"
#include "special_trade_list.h"

#include "game_item_manager/game_item.h"
#include "nel/misc/config_file.h"

#include "egs_sheets/egs_static_game_sheet.h"

struct SMissionTradeItem;
class CBuildingPhysicalGuild;
class CBuildingPhysicalPlayer;
class COutpostBuilding;
class CMerchant;
class CCharacterShoppingList;
class IShopUnit;
class CGenNpcDescMsgImp;

/// struct representing a group of NPCS
struct CNPCGroup
{
	/// members of the group
	std::set<TAIAlias>			Members;
	/// players who killed at list an entity
	std::vector<TDataSetRow>	PlayerKillers;
	/// Teams that killed at least an entity
	std::vector<uint16>			TeamKillers;
};

/// struct for resist modifiers due to spell casting (resists increase when target by a spell)
class CCreatureResistModifiers
{
public:
	sint16	Fear;
	sint16	Sleep;
	sint16	Stun;
	sint16	Root;
	sint16	Snare;
	sint16	Slow;
	sint16	Blind;
	sint16	Madness;

	sint16	Acid;
	sint16	Cold;
	sint16	Electricity;
	sint16	Fire;
	sint16	Poison;
	sint16	Rot;
	sint16	Shockwave;

public:
	// ctor
	inline CCreatureResistModifiers()
	{ clearModifiers(); }

	inline void clearModifiers()
	{
		Fear		= 0;
		Sleep		= 0;
		Stun		= 0;
		Root		= 0;
		Snare		= 0;
		Slow		= 0;
		Blind		= 0;
		Madness		= 0;
		Acid		= 0;
		Cold		= 0;
		Electricity = 0;
		Fire		= 0;
		Poison		= 0;
		Rot			= 0;
		Shockwave	= 0;
	}
};

//////////////////////////////////////////////////////////////////////////////
// CCreatureSheet                                                           //
//////////////////////////////////////////////////////////////////////////////

class CCreatureSheet
: public CStaticCreaturesProxy
{
public:
	CCreatureSheet(IStaticCreaturesCPtr sheet);

public:
	///@name IStaticCreatures overloads
	//@{
	virtual uint32	getFaction() const { return _Faction; }
	virtual bool	getFameByKillValid() const { return _FameByKillValid; }
	virtual sint32	getFameByKill() const { return _FameByKill; }
	//@}

	///@name Setters
	//@{
	virtual void	setFaction(uint32 value) { _Faction = value; }
	virtual void	setFameByKillValid(bool value) { _FameByKillValid = value; }
	virtual void	setFameByKill(sint32 value) { _FameByKill = value; }
	//@}

	/// Proxy reset
	void reset()
	{
		if (_Sheet)
		{
			_Faction = _Sheet->getFaction();
			_FameByKillValid = _Sheet->getFameByKillValid();
			_FameByKill = _Sheet->getFameByKill();
		}
	}
	///@name Partial reset
	//@{
	virtual void	resetFaction() { if (_Sheet) _Faction = _Sheet->getFaction(); }
	virtual void	resetFameByKillValid() { if (_Sheet) _FameByKillValid = _Sheet->getFameByKillValid(); }
	virtual void	resetFameByKill() { if (_Sheet) _FameByKill = _Sheet->getFameByKill(); }
	//@}

private:
	uint32	_Faction;
	bool	_FameByKillValid;
	sint32	_FameByKill;
};
typedef NLMISC::CSmartPtr<CCreatureSheet> CCreatureSheetPtr;
typedef NLMISC::CSmartPtr<CCreatureSheet const> CCreatureSheetCPtr;

/**
 * Add all specific feature for creatures not assumed by CEntityBase
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CCreature : public CEntityBase, public CHarvestable
{
	NL_INSTANCE_COUNTER_DECL(CCreature);
public:
	/// Constructor
	CCreature();

	/// Destructor
	virtual ~CCreature();
private:

public:

	/// get a copy of this creature
	CCreature *  getCreatureCopy( const NLMISC::CEntityId & entityId, sint32 cellId  );

	/**
	 * Add the properties to the emiter
	 */
	void addPropertiesToMirror( const TDataSetRow& entityIndex, bool keepSheetId=true );

	/// Load George Sheet, accessing the sheet in the mirror (and initializing _SheetId)
	virtual void loadSheetCreature( const TDataSetRow& entityIndex );

	/// Load George Sheet; using the param sheetId
	void loadSheetCreature( NLMISC::CSheetId sheetId);

	void setUpdateNextTick() { _TickUpdateTimer.setRemaining( 1, _TickUpdateTimer.getEvent() ); }

	uint32 tickUpdate();

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
	virtual void setMode( MBEHAV::TMode mode )
	{
		nlwarning("setting mode %s for a creature !!! Forbidden", mode.toString().c_str());
#ifdef NL_DEBUG
		nlstopex(("set mode %s for creature", mode.toString().c_str()));
#endif
	}

	/// entity is dead
	virtual void deathOccurs();

	/// set looter entity
	virtual bool setLooter( const NLMISC::CEntityId& entity, bool forceUpdate = false );

	///\return the looter entity
	const NLMISC::CEntityId &  getLooter() { return _EntityLooter; }

	/// get loot inventory
//	inline CGameItemPtr& getLootInventory() { return _LootInventory; }
	inline CInventoryPtr& getLootInventory() { return _LootInventory; }

	/**
	 * computeSkillProgress calulate the experience gain after an action
	 * \param Target is the target of action (uknown if action ne need target or target is implicite )
	 * \param DeltaLvl is the difference of level between maker and target
	 * \param OffensiveAction is true if action is offensive (target can resist)
	 * \param Skill is the name of used skill for action (or associated skill ofr specialized action used)
	 * \param Speciality is the name of specialized skill used for action ( empty if no specialized skill used )
	 */
	virtual void computeSkillProgress( CEntityBase *Target, sint32 DeltaLvl, ACTNATURE::TActionNature ActionNature, const std::string& Skill, const std::string& Speciality ) { }

	/**
	 * setBotDescription Set all params describe bot relative to bot chat (what selling, what mission proposed etc...)
	 * \param description is a transport class message reference for update description
	 */
	void setBotDescription( const CGenNpcDescMsgImp& description );

	/**
	 * Set the alias of the creature AI group
	 * \param alias: the new group alias
	 */
	inline void setAIGroupAlias( TAIAlias alias ){_AIGroupAlias = alias;}

	/**
	 * Get the AI Group Alias
	 / \return the group alias of the bot
	 */
	inline TAIAlias getAIGroupAlias() const { return _AIGroupAlias;}

	///\return the alias
	inline TAIAlias getAlias() const  { return _AIAlias;	}

	///set the bot alias alias
	inline TAIAlias setAlias( TAIAlias alias ) {return _AIAlias = alias;	}

	/**
	 * get a reference on mission vector of creature (bot)
	 * \return reference on mission vector
	 */
	inline const std::vector< uint32 >& getMissionVector() const { return _MissionsProposed; }

	// return Non Null CMerchant ptr
	NLMISC::CSmartPtr<CMerchant>& getMerchantPtr();

	/**
	 * Update merchant trade list
	 */
	void updateMerchantTradeList();

	/**
	 * get a reference on shop unit of bot
	 * \return reference on tradelist vector
	 */
	const std::vector< const IShopUnit * >&getMerchantTradeList();

	/// get bot chat program validation flags reference
	uint32 getBotChatProgram() const { return _BotChatProgram; }

	/// set bot chat program
	void setBotChatProgram( uint32 program ) { _BotChatProgram = program; }

	/// return true if bot selling fight action
	inline bool isSellingFightAction() const { return _FightAction; }
	/// return true if bot selling magic action
	inline bool isSellingMagicAction() const { return _MagicAction; }
	/// return true if bot selling craft action
	inline bool isSellingCraftAction() const { return _CraftAction; }
	/// return true if bot selling harvest action
	inline bool isSellingHarvestAction() const { return _HarvestAction; }
	/// return true if bot selling characteristics
	inline bool isSellingCharacteristics() const { return _CharacteristicsSeller; }

	/// Reassign _SheetId to server sheet (_SheetId must have been initialized in mirror before)
	void setServerSheet();

	/// set creature sheath items
	void setItemsInSheath( uint8 sheath, const NLMISC::CSheetId &rightHandItem, uint8 qualityR, const NLMISC::CSheetId &leftHandItem, uint8 qualityL, const NLMISC::CSheetId &ammoItem, uint8 qualityA);

	/// get the character leader index
	inline TDataSetRow getCharacterLeaderIndex() const { return _CharacterLeaderIndex; }

	/// set the character leader index
	inline void setCharacterLeaderIndex( const TDataSetRow& row) { _CharacterLeaderIndex = row; }

	/// reset the character leader index
	inline void resetCharacterLeaderIndex() { _CharacterLeaderIndex = TDataSetRow(); }

	/// reset the character leader index
	void disbandTrain();

	/// get the welcome chat message
	inline const std::string & getWelcomeMessage() { return _WelcomePhrase; }

	/**
	 * apply the effect of the armor/shield on damage. Update the armor items if necessary
	 * \return the remaining damages
	 */
	virtual sint32 applyDamageOnArmor( DMGTYPE::EDamageType dmgType, sint32 damage, SLOT_EQUIPMENT::TSlotEquipment forcedSlot);

	/// Return the damage using current armor, done by an explosion (e.g. forage source explosion)
	virtual float getActualDamageFromExplosionWithArmor( float dmg ) const;

	inline const std::vector< std::pair< std::string , std::string > > & getContextTexts() { return _ContextTexts; }

	void addContextText(const std::string & title, const std::string & detail) { _ContextTexts.push_back(std::make_pair(title,detail)); }


	/// get ItemPtr from it's sheet and quality (all items are created at service loading, and not copied for every npc using it)
	CGameItemPtr getNpcItem( const NLMISC::CSheetId &sheet, uint16 quality);

	/// get item in right hand
	inline virtual CGameItemPtr getRightHandItem()
	{
		return _RightHandItem;
	}

	/// get item in left hand
	inline virtual CGameItemPtr getLeftHandItem()
	{
		return _LeftHandItem;
	}

	/// get ammo item
	inline virtual CGameItemPtr getAmmoItem()
	{
		return _LeftHandItem;
	}

	inline void setItems( const NLMISC::CSheetId &rightHandItem, uint8 qualityR, const NLMISC::CSheetId &leftHandItem, uint8 qualityL)
	{
		_RightHandItem = getNpcItem(rightHandItem, qualityR);
		_LeftHandItem = getNpcItem(leftHandItem, qualityL);
	}

	// tp wanted for an entity
	void tpWanted( sint32 x, sint32 y, sint32 z , bool useHeading = false, float heading = 0.0f , uint8 continent = 0xFF, sint32 cell = 0);

	/// request a despawn after a number of game cycles
	void requestDespawn(NLMISC::TGameCycle waitCycles = 0);

	/// try to abort a despawn request, returns true if despawn has been aborted
	bool abortDespawn();

	// return reference on loot right
	inline std::vector< TDataSetRow >& getLootRight() { return _LootRight; }

	/// enable loot rights for given entity
	inline void enableLootRights(const TDataSetRow &entityRowId)
	{
		_LootRight.clear();
		_LootRight.push_back(entityRowId);
		_LootRightDuration = CTickEventHandler::getGameCycle() + 900;
	}

	/// enable loot rights for given team
	void enableLootRights(uint16 teamId);

	/// get the resist value associated to effect type
	uint32 getMagicResistance(EFFECT_FAMILIES::TEffectFamily effectFamily);

	/// get the resist value associated to damage type
	uint32 getMagicResistance(DMGTYPE::EDamageType dmgType);

	/// get creature resist modifiers
	const CCreatureResistModifiers	&getResistModifiers() const { return _ResistModifiers; }

	/// inc creature resist modifier
	virtual void incResistModifier(EFFECT_FAMILIES::TEffectFamily effectFamily, float factor);
	/// inc creature resist modifier
	virtual void incResistModifier(DMGTYPE::EDamageType dmgType, float factor);

	/// return the guild building linked to this bot
	const CBuildingPhysicalGuild * getGuildBuilding() const { return _GuildBuilding; }

	/// return the player building linked to this bot
	const CBuildingPhysicalPlayer * getPlayerBuilding() const { return _PlayerBuilding; }

	COutpostBuilding*		getOutpostBuilding() const { return _OutpostBuilding; }
	void					setOutpostBuilding(COutpostBuilding *);

	/// get the bot rolemaster type
	EGSPD::CSPType::TSPType getGuildRoleMasterType() const { return _GuildRoleMasterType; }

	/// return origin shop selector
	inline const std::vector< uint32 >& getOriginShopSelector() const { return _OriginShopSelector; }

//#ifdef NL_DEBUG
	/// has death report been sent ???
	bool deathReportHasBeenPushed() const { return _DeathReportHasBeenPushed; }
	bool deathReportHasBeenSent() const { return _DeathReportHasBeenSent; }
	void deathReportSent() { _DeathReportHasBeenSent = true; }
//#endif

	/// keep aggressiveness	of a creature against player character
	void addAggressivenessAgainstPlayerCharacter( TDataSetRow PlayerRowId );

	///	remove aggressiveness of a creature against player character
	void removeAggressivenessAgainstPlayerCharacter( TDataSetRow PlayerRowId );

	// get number of player in aggro list
	uint16 getNbOfPlayersInAggroList() const { return _NbOfPlayersInAggroList;	}

	/// return true if creature is agressive against player character
	bool isAgressive( TDataSetRow PlayerRowId ) { return _Agressiveness.find( PlayerRowId ) != _Agressiveness.end(); }

	/// get aggro list
	inline const std::set<TDataSetRow> &getAggroList() const { return _Agressiveness; }

	/// get the guild charges proposed by the bot
	const std::vector<uint16> & getGuildCharges() { return _GuildCharges; }

	/// get creature static form
	const CStaticCreatures* getForm() const { return _Form; }

	/// get trade special price factor
	bool getSpecialPlayerSellPriceFactor ( const NLMISC::CSheetId & sheet, float & factor ) const
	{
		return _SpecialTradeList.getBuyPriceFactor( sheet, factor );
	}

	// return true if creature sell player item
	bool sellPlayerItem();

	const std::vector< uint32 >& getBotChatCategory() const { return _BotChatCategory; }

	bool getIsAPet() { return _IsAPet; }

	void setIsAPet(bool isAPet) { _IsAPet = isAPet; }

	// Display shop selectors
	void displayShopSelectors( NLMISC::CLog& log );

	/// used to know if creature has still money on it
	bool moneyHasBeenLooted() { return _MoneyHasBeenLooted; }
	void moneyHasBeenLooted(bool m) { _MoneyHasBeenLooted = m; }

	/// add a Faction to faction attackable vector
	inline void addFactionAttackable(uint32 factionIndex, sint32 fame, bool above)
	{
		if (above)
			_FactionAttackableAbove.push_back( std::make_pair(factionIndex, fame) );
		else
			_FactionAttackableBelow.push_back( std::make_pair(factionIndex, fame) );
	}

	/// check creature can be attacked by given player according to it's faction fames
	bool checkFactionAttackable(const NLMISC::CEntityId &playerId) const;

	uint32	getFaction() const { return _Faction; }
	bool	getFameByKillValid() const { return _FameByKillValid; }
	sint32	getFameByKill() const { return _FameByKill; }

	void	setFaction(uint32 value) { _Faction = value; }
	void	setFameByKillValid(bool value) { _FameByKillValid = value; }
	void	setFameByKill(sint32 value) { _FameByKill = value; }

	void reset()
	{
		if (_Form)
		{
			_Faction = _Form->getFaction();
			_FameByKillValid = _Form->getFameByKillValid();
			_FameByKill = _Form->getFameByKill();
		}
	}

	void	resetFaction() { if (_Form) _Faction = _Form->getFaction(); }
	void	resetFameByKillValid() { if (_Form) _FameByKillValid = _Form->getFameByKillValid(); }
	void	resetFameByKill() { if (_Form) _FameByKill = _Form->getFameByKill(); }

	/// get webPage info
	const std::string &getWebPage() const {return _WebPage;}
	const std::string &getWebPageName() const {return _WebPageName;}

	const uint32 getOrganization() const { return _Organization; }

	const NLMISC::CSheetId &getBotChatOutpost() const {return _BotChatOutpost;}

	const std::vector<NLMISC::CSheetId> &getExplicitActionTradeList() const {return _ExplicitActionTradeList;}
	bool		getFilterExplicitActionTradeByPlayerRace() const {return _FilterExplicitActionTradeByPlayerRace;}
	EGSPD::CSPType::TSPType		getExplicitActionSPType() const {return _ExplicitActionSPType;}
	bool		getFilterExplicitActionTradeByBotRace() const {return _FilterExplicitActionTradeByBotRace;}

	// accessors for altar ticket tp restriction
	inline PVP_CLAN::TPVPClan getAltarClanRestriction() { return _TicketClanRestriction; }
	inline bool getAltarForNeutral() { return _TicketForNeutral; }
	inline sint32 getAltarFameRestriction() { return _TicketFameRestriction; }
	inline sint32 getAltarFameValueRestriction() { return _TicketFameRestrictionValue; }

	inline void clearAltarFameRestriction() { _TicketFameRestriction = CStaticFames::INVALID_FACTION_INDEX; }
	inline void clearAltarFameRestrictionValue() { _TicketFameRestrictionValue = 250; }

	NLMISC::CSheetId getLootTable(uint i) { if (i<_LootTables.size()) return _LootTables[i]; else return NLMISC::CSheetId::Unknown; };

	float getMaxHitRangeForPC() { return _MaxHitRangeForPC; }

//	bool isMissionStepIconDisplayable() const { return _MissionIconFlags.IsMissionStepIconDisplayable; }
//	bool isMissionGiverIconDisplayable() const { return _MissionIconFlags.IsMissionGiverIconDisplayable; }

	void setUserModelId(const std::string &id);
	const std::string getUserModelId() const { return _UserModelId; }

	void setCustomLootTableId(const std::string &id);
	const std::string getCustomLootTableId() const { return _CustomLootTableId;}

	void setPrimAlias(uint32 alias);
	uint32 getPrimAlias() const { return _PrimAlias; }

	void initFormPointer(NLMISC::CSheetId sheetId);
	void displayModifiedAttributes(NLMISC::CEntityId id, NLMISC::CLog &log);

protected:
	/**
	 * kill entity (when it's hit points reach 0)
	 * \param killerRowId if valid, the TDataSetRow of the entity which has killed the current entity
	 */
	virtual void kill(TDataSetRow killerRowId = TDataSetRow());

private :
	void fillLootInventory( NLMISC::CSheetId& lootTable, const CStaticItem * bagForm, bool& haveLoot );

private:
	CTimer                          _TickUpdateTimer;

	std::vector<NLMISC::CSheetId>	_LootTables;
//	CGameItemPtr					_LootInventory;
	CInventoryPtr					_LootInventory;
	NLMISC::CEntityId				_EntityLooter;

	/// keep pointer on creature form
	const CStaticCreatures *		_Form;

	std::string						_SheetName;

	uint32	_Faction;
	bool	_FameByKillValid;
	sint32	_FameByKill;

	// Alias of the parent group
	TAIAlias						_AIGroupAlias;

	// Bot chat parameters part of members
	TAIAlias						_AIAlias;					// Alias for identitfy creature for mission system
	std::string						_WebPage;					// Web Page accessible from the NPC
	std::string						_WebPageName;				// Web Page Name (if any)
	std::vector< uint32 >			_MissionsProposed;			// Mission ids proposed by NPC
	std::vector< uint32 >			_BotChatCategory;			// vector of active selector of bot chat category
	std::vector< uint32 >			_RmShopSelector;			// Rm shop selector (ie: for rm ecosystem list)
	std::vector< uint32 >			_OriginShopSelector;		// Item Origin shop selector (ie: for item by species list)
	std::vector< uint32 >			_QualityShopSelector;		// Item Quality shop selector (ie: for item by quality list)
	std::vector< uint32 >			_LevelShopSelector;			// Item Level shop selector (ie: for item by level list)
	std::vector< uint32 >			_ShopTypeSelector;			// Shop type selector (static or dynamic)

	NLMISC::TGameCycle				_LastCycleUpdateSelectors;	// Last cycle received a update message from AI
	NLMISC::TGameCycle				_LastCycleUpdateTradeList;	// Last cycle message processed for update trade list

	std::vector<NLMISC::CSheetId>	_ExplicitActionTradeList;	// explicit list of action to sell
	bool							_FilterExplicitActionTradeByPlayerRace;
	EGSPD::CSPType::TSPType			_ExplicitActionSPType;
	bool							_FilterExplicitActionTradeByBotRace;
	bool							_FightAction;				// shop fight action
	bool							_MagicAction;				// shop magic action
	bool							_CraftAction;				// shop craft action
	bool							_HarvestAction;				// shop harvest action
	bool							_CharacteristicsSeller;		// shop characteristics
	bool							_GuildCreator;				// true if the bot enable player to create guilds

	NLMISC::CSmartPtr<CMerchant> 	_Merchant;					// smart pointer on CMerchant class of creature

	uint32							_BotChatProgram;			// Program enabled for bot chat
	uint32							_Organization;				// Organization for bot

	TDataSetRow						_CharacterLeaderIndex;		// The data set row of the beast's leader character if it's in a pack/train
	std::string						_WelcomePhrase;				// welcome message id of the bot

	/// texts displayed through the context menu. First element : title, second : detail
	std::vector< std::pair< std::string , std::string > > _ContextTexts;

	/// Right hand item
	CGameItemPtr					_RightHandItem;
	/// Left hand item
	CGameItemPtr					_LeftHandItem;

	bool							_DespawnRequested;			// despawn has been requested
	bool							_DespawnSentToAI;			// despawn has been sent to AI (too late to abort)
	NLMISC::TGameCycle				_DespawnDate;				// despawn date

	// loot rights management
	std::vector< TDataSetRow >		_LootRight;					// list of CCharacter have loot right after creature death, if emty all have loot right
	NLMISC::TGameCycle				_LootRightDuration;			// Duration of loot right before all have loot right

//#ifdef NL_DEBUG
	// Looking for a 'Zombie' bug (creature dead on EGS, but not on AIS)
	bool	_DeathReportHasBeenSent;
	bool	_DeathReportHasBeenPushed;
	// keep death date to send death report every 5 ticks if not recived by AIS
	NLMISC::TGameCycle _DeathDate;
//#endif

	/// guild rooms managed by this bot
	const CBuildingPhysicalGuild*			_GuildBuilding;
	/// guild rooms managed by this bot
	const CBuildingPhysicalPlayer*			_PlayerBuilding;
	/// if this bot is a outpost building bot so we have a pointer on the structure
	COutpostBuilding*						_OutpostBuilding;

	/// type of the bot as a guild rolemaster ( invalid value if none )
	EGSPD::CSPType::TSPType				_GuildRoleMasterType;

	// creature agressiveness against player character
	std::set< TDataSetRow >			_Agressiveness;

	// keep nb of players in creature aggro list (as aggro list also contains npcs or creatures)
	uint16						_NbOfPlayersInAggroList;

	/// charges proposed by this bot
	std::vector<uint16>		_GuildCharges;
	/// special trade list of the list
	CSpecialTradeList				_SpecialTradeList;
	/// resits modifiers
	CCreatureResistModifiers	_ResistModifiers;

	/// Flag : this creature is a pet
	bool							_IsAPet;

	/// if true the next time this creature is looted money will not be given to looter
	bool							_MoneyHasBeenLooted;

	/// vector of faction that can attack this creature (when > 0)
	std::vector< std::pair<uint32, sint32> >	_FactionAttackableAbove;
	/// vector of faction that can attack this creature (when < 0)
	std::vector< std::pair<uint32, sint32> >	_FactionAttackableBelow;

	NLMISC::CSheetId				_BotChatOutpost;

	/// Altar selector for TP tickets restrictions
	PVP_CLAN::TPVPClan				_TicketClanRestriction;
	bool							_TicketForNeutral;
	sint32							_TicketFameRestriction;
	sint32							_TicketFameRestrictionValue;
//	} _MissionIconFlags;

	float							_MaxHitRangeForPC;
	std::string						_UserModelId;
	std::string						_CustomLootTableId;
	//if the creature has a user model, this is the alias of the primitive where the model is defined
	uint32							_PrimAlias;
};

typedef NLMISC::CSmartPtr<CCreature> CCreaturePtr;
typedef NLMISC::CRefPtr<CCreature> CCreatureRefPtr;


#endif // RY_CREATURE_H

/* End of creature.h */
