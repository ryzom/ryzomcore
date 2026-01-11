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

#ifndef TOTEM_BASE_H
#define TOTEM_BASE_H

#include "game_share/pvp_clan.h"
#include "game_share/effect_families.h"
#include "creature_manager/creature.h"

class CSEffect;
class CCharacter;
class CTotemEffect;
class CFileDescription;

extern NLMISC::CVariable<uint32> TotemBuildTime;

/**
 * A Totem Base with its effects on faction PVP
 * \author Gregorie Diaconu
 * \author Nevrax France
 * \date 2005
 */
class CTotemBase
{
public :
	DECLARE_PERSISTENCE_METHODS

	///\ctor
	CTotemBase( std::string const& name );
	
	/// Return all effects a given character gets on this totem
	void	getTotemEffect( CCharacter* user, std::vector<CSEffect*>& outEffects ) const;
	
	/// Start building this totem
	void	startBuilding( CCharacter* builder );

	/// Update building of the totem. Returns true if the building was updated.
	bool	tickUpdate();

	/// Destroy this totem
	void	destroyTotem();
	
	/// Change the effect family this totem give
	inline void setTotemEffectFamily( EFFECT_FAMILIES::TEffectFamily effect )	{ _TotemEffect = effect; }

	/// Get the effect family this totem give
	inline EFFECT_FAMILIES::TEffectFamily getTotemEffectFamily() const			{ return _TotemEffect; }

	/// Set the region alias of this totem
	inline void		setRegionAlias( TAIAlias alias )	{ _RegionAlias = alias; }

	/// Get the region alias of this totem
	inline uint32	getRegionAlias() const				{ return _RegionAlias; }
	
	/// Add a neighbour totem
	inline void		addNeighbour( TAIAlias neighbour )	{ _Neighbours.push_back( neighbour ); }

	/// Get the number of neighbours
	inline uint		getNumNeighbours() const			{ return (uint)_Neighbours.size(); }

	/// Get the list of neighbours
	inline TAIAlias	getNeighbour( uint index ) const	{ return _Neighbours[index]; }


	/// Change the owner of this totem.
	inline void					setOwnerFaction( PVP_CLAN::TPVPClan faction )	{ _OwnerFaction = faction; }

	/// Get the faction which currently owns this totem
	inline PVP_CLAN::TPVPClan	getOwnerFaction() const	{ return _OwnerFaction; }


	/// Return the bot object associated to this totem
	inline CCreature*	getBotObject() const	{ return _BotObject; }

	/// Change the bot object associated to this totem
	void				setBotObject( CCreature* botObject );

	/// Return true if the totem building is completed, or if there is no totem
	inline bool	isBuildingFinished() const { return _IsBuildingFinished; }

	/// Load data from PDR
	void loadFromPDR();
	void totemFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);


	/// Can this character build a totem there ?
	bool canStartBuilding( CCharacter* actor );

	/// time left for building spire is finished
	NLMISC::TGameCycle buildTimeLeft() const { return _IsBuildingFinished ? (NLMISC::TGameCycle)0 : TotemBuildTime - (CTickEventHandler::getGameCycle() - _BuildingStartTime); }

	/// return builder of totem
	TDataSetRow  getBuilder() { return _Builder; }

	/// return tick when attack message are sended for last time
	inline NLMISC::TGameCycle getLastTickAttackMessageSended() const { return _LastTickAttackMessageSended; }

	// set tick for last time an attack message are sended
	inline void setLastTickAttackMessageSended( NLMISC::TGameCycle tick ) { _LastTickAttackMessageSended = tick; }

	std::string getName() const { return _Name; }

private :
	/// Alias of the region
	TAIAlias	_RegionAlias;

	std::string	_Name;
	
	/// Faction which currently owns this totem (Neutral if no one)
	PVP_CLAN::TPVPClan	_OwnerFaction;
	
	/// Neighbor regions
	std::vector<TAIAlias>	_Neighbours;

	/// Effect of the totem base
	EFFECT_FAMILIES::TEffectFamily	_TotemEffect;

	/// Used BotObject
	NLMISC::CRefPtr<CCreature>	_BotObject;

	/// Is the building finished ? True if no totem
	bool				_IsBuildingFinished;

	/// Tick when the totem has last started being build
	NLMISC::TGameCycle	_BuildingStartTime;

	/// Tick when totem has last been updated
	NLMISC::TGameCycle	_LastTickUpdate;

	/// The last time a faction lost this totem
	NLMISC::TGameCycle	_LastTimeOwned[PVP_CLAN::NbClans];

	/// Tick when last time attack message for totem has sended
	NLMISC::TGameCycle	_LastTickAttackMessageSended;

	// these information must be kept or they will be lost
	// while changing the bot object sheetId
	/// HP Gained during building
	float	_BuildHpGain;
	
	/// Max HP for Totem
	float	_TotemMaxHP;

	/// Current HP for Totem
	float	_TotemCurrentHP;

	/// Builder of totem
	TDataSetRow  _Builder;

	/// Category of rewards a character can get
	enum TRewardCategory
	{
		/// the character is a member of the faction which owns the totem
		FACTION_MEMBER, 
		/// the character is an enemy of the faction which owns the totem
		FACTION_ENEMY,
		/**
		 * the character is neutral, but is a member of a guild which
		 * is aligned with the faction which owns the totem 
		 */
		NEUTRAl_GUILD,
		/// the character is strictly neutral
		NEUTRAL_STRICT,
	};
	
	/// Returns the reward category a given player will get on this totem
	TRewardCategory _GetRewardCatergory( CCharacter* user ) const;
};

#endif // TOTEM_BASE_H
