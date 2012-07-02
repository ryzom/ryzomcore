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



#ifndef RY_GAME_SHARE_PROPERTIES_H
#define RY_GAME_SHARE_PROPERTIES_H

#include "nel/misc/types_nl.h"


/**
 * Class to manage some properties of an entity.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CProperties
{
public:
	typedef struct
	{
		union
		{
			uint16 properties;

			struct
			{
				uint16	isSelectable	: 1; // The entity is selectable.
				uint16	isGivable		: 1; // It'possible to give something to the entity.
				uint16	isTalkableTo	: 1; // It's possible to talk to the entity.
				uint16	isUsable		: 1; // The entity can be used.
				uint16	isLiftable		: 1; // The entity is liftable.
				uint16	isLookableAt	: 1; // It's possible to look at the entity.
				uint16	isAttackable	: 1; // It's possible to attack the entity.
				uint16	isCurativable	: 1; // It's possible to heal or cure the entity
				uint16	isInvitable		: 1; // It'possible to invite the entity in a team.
				uint16	isHarvestable	: 1; // It'possible to loot the entity.
				uint16  canExchangeItem : 1; // It is possible to exchange items with the entity
				uint16  isMountable		: 1; // It is possible to mount the creature
				uint16	isLootable		: 1; // It'possible to loot the entity.
				uint16	isAfk			: 1; // entity is away from keyboard
				uint16	isInvulnerable	: 1; // entity is invulnerable (traders, rolemasters...)
				uint16  freeProperty	: 1; // free 1 bits
			} prop;
		};

	} TProperties;

protected:
	/// Entity properties.
	TProperties _Properties;

public:
	/// Constructor
	CProperties()				{_Properties.properties = 0;}
	CProperties(uint16 p)		{_Properties.properties = p;}
	CProperties(TProperties p)	{_Properties = p;}

	// Operator (uint16)
	operator uint16() const { return _Properties.properties; }

	// Operator ()
	const TProperties& operator () () const { return _Properties; }

	// Operator ==
	bool operator == ( const CProperties& p ) const { return p().properties == _Properties.properties; }

	// Operator !=
	bool operator != ( const CProperties& p ) const { return p().properties != _Properties.properties; }

	// Operator !=
	bool operator [] ( uint32 nBitNbWanted ) const
	{
		switch( nBitNbWanted )
		{
		case 0:
			return selectable();
		case 1:
			return givable();
		case 2:
			return talkableTo();
		case 3:
			return usable();
		case 4:
			return liftable();
		case 5:
			return lookableAt();
		case 6:
			return attackable();
		case 7:
			return curativable();
		case 8:
			return invitable();
		case 9:
			return harvestable();
		case 10:
			return canExchangeItem();
		case 11:
			return mountable();
		case 12:
			return lootable();
		case 13:
			return afk();
		case 14:
			return invulnerable();
		default:
			return false;
		}
		//return _Properties.properties & (1<<nBitNbWanted);}
	}

	/// Return true if the entity can be selected.
	bool selectable() const {return _Properties.prop.isSelectable;}
	/// Set if the entity can be selected.
	void selectable(bool p) {_Properties.prop.isSelectable = p;}

	/// Return true if it's possible to talk to the entity.
	bool talkableTo() const {return _Properties.prop.isTalkableTo;}
	/// Set if it's possible to talk to the entity.
	void talkableTo(bool p) {_Properties.prop.isTalkableTo = p;}

	/// Return true if it's possible to talk to the entity.
	bool usable() const {return _Properties.prop.isUsable;}
	/// Set if it's possible to talk to the entity.
	void usable(bool p) {_Properties.prop.isUsable = p;}

	/// Return true if it's possible to talk to the entity.
	bool liftable() const {return _Properties.prop.isLiftable;}
	/// Set if it's possible to talk to the entity.
	void liftable(bool p) {_Properties.prop.isLiftable = p;}

	/// Return true if it's possible to talk to the entity.
	bool lookableAt() const {return _Properties.prop.isLookableAt;}
	/// Set if it's possible to talk to the entity.
	void lookableAt(bool p) {_Properties.prop.isLookableAt = p;}

	/// Return true if it's possible to talk to the entity.
	bool givable() const {return _Properties.prop.isGivable;}
	/// Set if it's possible to talk to the entity.
	void givable(bool p) {_Properties.prop.isGivable = p;}

	/// Return true if it's possible to attack the entity.
	bool attackable() const {return _Properties.prop.isAttackable;}
	/// Set if it's possible to attack the entity.
	void attackable(bool p) {_Properties.prop.isAttackable = p;}

	/// Return true if it's possible to heal or cure the entity.
	bool curativable() const {return _Properties.prop.isCurativable;}
	/// Set if it's possible to heal or cure the entity.
	void curativable(bool p) {_Properties.prop.isCurativable = p;}

	/// Return true if it's possible to invite the entity. in a team
	bool invitable() const {return _Properties.prop.isInvitable;}
	/// Set if it's possible to invite the entity. in a team
	void invitable(bool p) {_Properties.prop.isInvitable = p;}

	/// Return true if it's possible to loot the entity.
	bool lootable() const {return _Properties.prop.isLootable;}
	/// Set if it's possible to loot the entity.
	void lootable(bool p) {_Properties.prop.isLootable = p;}

	/// Return true if it's possible to harvest the entity.
	bool harvestable() const {return _Properties.prop.isHarvestable;}
	/// Set if it's possible to harvest the entity.
	void harvestable(bool p) {_Properties.prop.isHarvestable = p;}

	/// Return true if it's possible to exchange items with the entity.
	bool canExchangeItem() const {return _Properties.prop.canExchangeItem;}
	/// Set if it's possible to to exchange items with the entity.
	void canExchangeItem(bool p) {_Properties.prop.canExchangeItem = p;}

	/// Return true if it's possible to mount the entity.
	bool mountable() const {return _Properties.prop.isMountable;}
	/// Set if it's possible to to exchange items with the entity.
	void mountable(bool p) {_Properties.prop.isMountable = p;}

	/// Return true if entity is AFK
	bool afk() const {return _Properties.prop.isAfk;}
	/// set afk state
	void afk(bool p) {_Properties.prop.isAfk = p;}

	/// Return true if entity is invulnerable
	bool invulnerable() const {return _Properties.prop.isInvulnerable;}
	/// set afk state
	void invulnerable(bool p) {_Properties.prop.isInvulnerable = p;}

	/// Set all flags
	void setAllFlags() { _Properties.properties = 0xffff; }

	/// serial
	void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		f.serial( _Properties.properties );
	}

	std::string toString() const { return NLMISC::toString( _Properties.properties ); }
};


#endif // RY_GAME_SHARE_PROPERTIES_H

/* End of properties.h */
