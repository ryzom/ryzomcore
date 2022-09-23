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



#ifndef RY_PROC_ITEM_PHRASE_H
#define RY_PROC_ITEM_PHRASE_H

#include "phrase_manager/s_phrase.h"


/**
 * class implemented proc item sabtrina phrases
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CProcItemPhrase : public CSPhrase
{
public:
	
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool execution = true )
	{
		// nothing to do there
		return true;
	}
	virtual void setPrimaryTarget( const TDataSetRow &entityRowId )
	{
		nlerror("Invalid overload");
		return;
	}
	virtual bool evaluate()
	{
		return true;
	}
	virtual bool validate()
	{
		return true;
	}
	virtual bool update()
	{
		nlerror("Invalid overload");
		return false;
	}
	virtual void execute()
	{
		nlerror("Invalid overload");
		return;
	}
	virtual bool launch()
	{
		nlerror("Invalid overload");
		return false;
	}
	virtual void apply()
	{
		nlerror("Invalid overload");
		return;
	}
	virtual void end()
	{
		nlerror("Invalid overload");
		return;
	}
};


#endif // RY_PROC_ITEM_PHRASE_H

/* End of proc_item_phrase.h */
