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



#ifndef NL_PROPERTY_DECODER_H
#define NL_PROPERTY_DECODER_H

#include <vector>
#include <deque>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"

#include "game_share/entity_types.h"
#include "game_share/action.h"

/**
 * An engine that allows to encode/decode continuous properties using delta values.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CPropertyDecoder
{
private:
/*
	/// A property entry, containing Garanted, ToGaranty and LastReceived values of action
	class CPropertyEntry
	{
	public:
		CLFECOMMON::CAction::TValue						LastReceived;
		CLFECOMMON::TPacketNumber						Packet;
		void	init() { Packet = 0xFFFFFFFF; LastReceived = 0; }
	};
*/

	/// An entity entry, containing properties for this entity
	class CEntityEntry
	{
	public:
		CEntityEntry() : AssociationBits(0), EntryUsed(false), PosIsRelative(false), PosIsInterior(false) {}
		CLFECOMMON::TSheetId							Sheet;
		uint16											AssociationBits;
		bool											EntryUsed;
		bool											PosIsRelative;
		bool											PosIsInterior;
//		CPropertyEntry									Properties[CLFECOMMON::LAST_CONTINUOUS_PROPERTY+1];
	};

	/// The entity entries
	std::vector<CEntityEntry>							_Entities;

	//
	uint16												_RefBitsX,
														_RefBitsY,
														_RefBitsZ;
	//
	sint32												_RefPosX,
														_RefPosY;
	//													_RefPosZ;

public:
	/// Constructor
	CPropertyDecoder();

	//
	void	init(uint maximum) { setMaximumEntities(maximum); clear (); }
	void	setMaximumEntities(uint maximum);
	void	clear();
	void	addEntity(CLFECOMMON::TCLEntityId entity, CLFECOMMON::TSheetId sheet);
	bool	removeEntity(CLFECOMMON::TCLEntityId entity);
	uint16&	associationBits( CLFECOMMON::TCLEntityId entity ) { return _Entities[entity].AssociationBits; }

	bool	isUsed(CLFECOMMON::TCLEntityId entity) const { return _Entities[entity].EntryUsed; }
	CLFECOMMON::TSheetId	sheet(CLFECOMMON::TCLEntityId entity) const { return _Entities[entity].Sheet; }


	/** Receives actions from the front end. Actually transmits actions received
	 *  by the client to the property decoder.
	 *  \param packetNumber the number of the packet received
	 *  \param ack the number of the acknowledged packet by the front end
	 *  \param actions the actions sent to the client by the front end
	 */
	void	receive(CLFECOMMON::TPacketNumber packetNumber, CLFECOMMON::TPacketNumber ack, std::vector<CLFECOMMON::CAction *> &actions);

	/** Receives single action from the front end. Actually transmits action received
	 *  by the client to the property decoder.
	 *  \param packetNumber the number of the packet received
	 *  \param ack the number of the acknowledged packet by the front end
	 *  \param action the action sent to the client by the front end
	 */
	void	receive(CLFECOMMON::TPacketNumber packetNumber, CLFECOMMON::CAction *action);

	/// Decode x and y
	void	decodeAbsPos2D( sint32& x, sint32& y, uint16 x16, uint16 y16 )
	{
		x = _RefPosX + (((sint32)((sint16)(x16 - _RefBitsX))) << 4);
		y = _RefPosY + (((sint32)((sint16)(y16 - _RefBitsY))) << 4);
	}

	/// Set player's reference position
	void	setReferencePosition(const NLMISC::CVectorD &position);


	/// Retrieves a property value.
//	const CLFECOMMON::CAction::TValue	&getProperty(CLFECOMMON::TCLEntityId entity, CLFECOMMON::TProperty property) const;

	/// Retrieves an entity position.
//	void					getPosition(CLFECOMMON::TCLEntityId entity, CLFECOMMON::CAction::TValue &posx, CLFECOMMON::CAction::TValue &posy, CLFECOMMON::CAction::TValue &posz) const;
};


#endif // NL_PROPERTY_DECODER_H

/* End of property_decoder.h */
