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



#ifndef TRADE_H
#define TRADE_H

// Misc
#include "nel/misc/vectord.h"

#include "game_item_manager/game_item.h"
#include "game_share/constants.h"

namespace RYZOM_TRADE
{


//type of a trade
enum TTradeType
{
	unknown = 0,
	item,
	teleport,
	brick,
	phrase,
	pact,
	plan,
};


// Specialized part of trading struct for item
struct SItemTrade
{
	NL_INSTANCE_COUNTER_DECL(SItemTrade);
public:

	// constructor
	SItemTrade() { ItemPtr = 0; Quality = 0; Level = 0; }

	uint16			Quality;
	uint16			Level;
	CGameItemPtr	ItemPtr;
	
	void copy( SItemTrade * i )
	{
		if (!i)
			return;

		Quality = i->Quality;
		Level = i->Level;
		ItemPtr = i->ItemPtr;
	}
};


///////////////////////////////////////////////////////
// User interface : Common interface for trading struct
///////////////////////////////////////////////////////
class CTradeBase
{
public:
	uint32				Price;
	uint8				Type;
	NLMISC::CSheetId	Sheet;
	void*				Specialized;

	CTradeBase() { Type = unknown; Specialized = 0; Price = 0; }

	CTradeBase( TTradeType type )
	{	
		Type = type; 
		if ( Type == item || Type == plan )
		{
			Specialized = new SItemTrade();
		}
		else
			Specialized = 0;
	}

	// destructor
	virtual ~CTradeBase()
	{
		if( (Specialized && Type == item || Type == plan ) && Specialized != 0 )
		{
			delete ( (SItemTrade *) Specialized);
		}
	}
	
	// Copy constructor
	CTradeBase( const CTradeBase& t )
	{
		Price = t.Price;
		Type = t.Type;
		Sheet = t.Sheet;
		if ( (Type == item || Type == plan ) && t.Specialized != 0)
		{
			Specialized = new SItemTrade();
			( (SItemTrade *) Specialized)->copy( (SItemTrade *) t.Specialized );
		}
		else
			Specialized = 0;
	}
	
	// = operator
	const CTradeBase &operator = (const CTradeBase &a)
	{
		Price = a.Price;
		Type = a.Type;
		Sheet = a.Sheet;
		if ( ( Type == item || Type == plan ) && a.Specialized != 0 )
		{
			if (Specialized)
				delete  (SItemTrade *) Specialized ;

			Specialized = new SItemTrade();
			( (SItemTrade *) Specialized)->copy( (SItemTrade *) a.Specialized );
		}
		return *this;
	}
private:
	// Common part of CTrade for serial management
	struct CTradeUnserial
	{
		uint32	Price;
		uint8	Type;
		NLMISC::CSheetId Sheet;
	};

	void init( CTradeUnserial& s )
	{
		Price = s.Price;
		Type = s.Type;
		Sheet = s.Sheet;
		if ( Type == item || Type == plan )
		{
			Specialized = new SItemTrade();
		}
	}
};

typedef std::vector< RYZOM_TRADE::CTradeBase * > TTradeList;

}

#endif
