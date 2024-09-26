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

#ifndef PVP_FACTION_REWARD_MANAGER_H
#define PVP_FACTION_REWARD_MANAGER_H

#include "game_share/pvp_clan.h"
#include "totem_base.h"
#include "nel/ligo/primitive.h"
#include "game_share/effect_families.h"

class CCharacter;
class CSEffect;
class CTotemBase;

/// What a faction owns
struct TFactionPossessions
{
	DECLARE_PERSISTENCE_METHODS

	/// Number of totems owned by the faction
	uint32	NbTotems;
	/// Level of the totems
	uint8	Level;
	/// Number of faction points owned by all the faction members
	sint32	FactionPointsPool;
};

/**
 * Manager for Rewards of the Faction PVP
 * \author Gregorie Diaconu
 * \author Nevrax France
 * \date 2005
 */
class CPVPFactionRewardManager
{
	DECLARE_PERSISTENCE_METHODS
public :
	typedef std::vector<int> TPossessionsPerEffect;
	
	/// Totems levels with the number of totems needed to reach them
	enum TotemLevel
	{
		LEVEL_0 =  1,
		LEVEL_1 =  5,
		LEVEL_2 = 15,
		LEVEL_3 = 25,
		LEVEL_4 = 35,
	};

	/// Values of each effects
	static sint32 EffectValues[ EFFECT_FAMILIES::EndTotemEffects+1 ];

	/// Returns the totem base for a given region using its ID
	const CTotemBase* getTotemBaseFromId( uint16 regionId ) { return _GetTotemBaseFromId( regionId ); }

	/// Singleton declaration
	NLMISC_SAFE_SINGLETON_DECL(CPVPFactionRewardManager);

private :
	///\ctor
	CPVPFactionRewardManager();

	/// Returns the totem base for a given region using its alias
	CTotemBase*	_GetTotemBase( TAIAlias regionAlias );

	/// Returns the totem base for a given region using its ID
	CTotemBase* _GetTotemBaseFromId( uint16 regionId );

	/// Update totems level for a given faction
	void	_UpdateLevel( PVP_CLAN::TPVPClan faction );
	
	/// Get effect bonus for each totem level
	sint32	_GetLevelBonus( EFFECT_FAMILIES::TEffectFamily, uint8 level );

	/// Get the effects a character would get on a totem
	void	_GetTotemsEffectsRec( CCharacter* user, CTotemBase* pTotem, 
		                          std::vector<CSEffect*>& outEffects, std::vector<CTotemBase*>& processed );
	/// Remove all totem effects from a given player
	void	_removeTotemsEffects( CCharacter* user );

	/// Totem bases sorted by region
	std::map<TAIAlias, CTotemBase*>	_TotemBasesPerRegion;

	/// Number of totems
	uint32						_NbTotems;

	/// Possessions for each faction
	TFactionPossessions			_FactionsPossessions[PVP_CLAN::NbClans];

	/// Build the totem bases using the a LIGO primitive
	void	_BuildTotemBasesRec( const NLLIGO::IPrimitive* prim,
		                         std::map<CTotemBase*, std::set<std::string> >& neighboursNames,
							     std::map<std::string, CTotemBase*>& totemBasesPerName );

	/// Have the totem bases been setup ?
	bool				_InitDone;

	/// Has data been load from database ?
	bool				_DBLoaded;

	/// Is there some updates we could save ?
	bool				_DataUpdated;

	/// Date of last save
	NLMISC::TGameCycle	_LastSave;

	/**
	 * Load from database factions and totems data
	 * Return false if there is nothing to load, true elsewhere
	 */
	bool _LoadFromPDR();
	// callback from BS
	void _totemFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);

	friend struct TTotemFileCallback;

public :
	/// Initialize totem bases and effect values (called only once)
	void init();

	/**
	 * Start building a totem in a given region. This totem will be owned
	 * by the builder's faction. 
	 * Return true if the building has started, false elsewhere.
	 */
	bool startTotemBuilding( uint16 regionIndex, CCharacter* builder );

	/// Destroy the totem in a given region 
	bool destroyTotem( uint16 regionIndex, TDataSetRow killerRowId );

	/// Get the list of reward effects for a character depending on its coordinates
	std::vector<CSEffect*> getTotemsEffects( CCharacter* user, std::vector<CTotemBase*>& processed );

	/// Give totem rewards to a player
	void giveTotemsEffects( CCharacter* user );

	/// Remove all totem effects from a given player and update player database
	void removeTotemsEffects( CCharacter* user );

	/// Update totems building
	void tickUpdate();

	/// Update the faction points pool of a faction
	void updateFactionPointPool( PVP_CLAN::TPVPClan faction, sint32 fpDelta );

	/// get faction points in pool of a faction
	sint32 getFactionPointPool( PVP_CLAN::TPVPClan faction );

	/// Returns true if this botObject is linked to a CTotemBase
	bool isATotem( CCreature* botObject );

	/// Returns true if the targeted botObject can be attacked by the actor
	bool isAttackable( CCharacter* actor, CEntityBase* target );

	/// Returns true if the actor can build a totem in this region
	bool canBuildTotem( CCharacter* actor );

	/// Returns the faction which currently owns a region
	PVP_CLAN::TPVPClan getRegionOwner( uint16 regionId );

	/// send event message to ai
	void sendEventToAI( const CTotemBase * totem, const std::string& event );

	/// send message when spire is attacked
	void spireAttacked( CCharacter * actor, CCreature * spire );

	/**
	 * Add a new totem bot object. This bot object will be linked
	 * to the corresponding CTotemBase depending on its coordinates
	 */
	void addBotObject( CCreature* botObject );

	inline bool isInit() { return _InitDone; }
};

#endif // PVP_FACTION_REWARD_MANAGER_H
