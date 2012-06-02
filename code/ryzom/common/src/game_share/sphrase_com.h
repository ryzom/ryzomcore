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



#ifndef RY_SPHRASE_H
#define RY_SPHRASE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/stream.h"
#include "persistent_data.h"
#include "persistent_data.h"
#include "inventories.h"


// ***************************************************************************
/**
 * Description of a Sabrina Phrase. (ie set of brick, and other client side infos)
 *	For communication Client/Server (NB: CSPhrase name already exist...)
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CSPhraseCom
{
public:
	static const	CSPhraseCom	EmptyPhrase;

public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	/// Constructor
	CSPhraseCom(){}

	// clear out the contents before filling with new data...
	void clear()
	{
		Bricks.clear();
		Name.clear();
	}

	// List Of SBricks composing the phrase.
	std::vector<NLMISC::CSheetId>	Bricks;

	// Name Of the Phrase. Saved on server, read on client.
	ucstring						Name;

	/// The comparison is made only on Bricks
	bool	operator==(const CSPhraseCom &p) const;
	bool	operator!=(const CSPhraseCom &p) const {return !operator==(p);}
	bool	operator<(const CSPhraseCom &p) const;

	/// consider empty if Bricks.empty() or if brick 0 is 0.
	bool	empty() const {return Bricks.empty() || Bricks[0].asInt()==0;}

	/// This serial is made for server->client com. NB: SheetId must be init.
	void	serial(NLMISC::IStream &impulse);
};


// ***************************************************************************
/**
 * Tuple Sabrina / Known Slot.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CSPhraseSlot
{
public:
	CSPhraseCom			Phrase;
	uint16				KnownSlot;
	NLMISC::CSheetId	PhraseSheetId;

	/// This serial is made for server->client com.
	void	serial(NLMISC::IStream &impulse);
};


// ***************************************************************************
class CSPhraseMemorySlot
{
public:
	uint8		MemoryLineId;
	uint8		MemorySlotId;
	uint16		PhraseId;

	/// This serial is made for server->client com.
	void	serial(NLMISC::IStream &impulse);
};

// ***************************************************************************
class CFaberMsgItem
{
	uint8	InvId;			// matchs INVENTORIES::EInventory
public:
	uint16	IndexInInv;		// index in the inventory
	uint16	Quantity;		// quantity of mp selected

	void setInvId(INVENTORIES::TInventory invId)
	{
		InvId = invId;
	}
	INVENTORIES::TInventory getInvId()
	{
		return INVENTORIES::TInventory(InvId);
	}
	/// This serial is made for server->client com.
	void	serial(NLMISC::IStream &impulse);
};

#endif // NL_SPHRASE_H

/* End of sphrase_com.h */
