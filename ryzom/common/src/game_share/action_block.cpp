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



#include "stdpch.h"

#include "action_block.h"
#include "action_factory.h"

using namespace NLMISC;

namespace CLFECOMMON {


CActionBlock::~CActionBlock()
{
	uint	i;
	for (i=0; i<Actions.size(); ++i)
		CActionFactory::getInstance()->remove(Actions[i]);
}

void	CActionBlock::serial(CBitMemStream &msg)
{
	if (!msg.isReading() && Cycle == 0)
		nlwarning("Packing action block (%d actions) with unset date", Actions.size());

	msg.serial(Cycle);

	uint	i;

	uint8	num = (uint8)Actions.size();
	msg.serial(num);

	//static char	buff[1024], cat[128];

	if (msg.isReading())
	{
		//sprintf(buff, "Unpack[%d]:", Cycle);
		for (i=0; i<num; ++i)
		{
			CAction	*action = CActionFactory::getInstance()->unpack(msg, false);
			if ( ! action )
			{
				Success = false; // reject an incorrect block
			}
			else
			{
				//sprintf(cat, " %d(%d bits)", action->Code, action->size());
				//strcat(buff, cat);
				Actions.push_back(action);
			}
		}
	}
	else
	{
		//sprintf(buff, "Pack[%d]:", Cycle);
		for (i=0; i<num; ++i)
		{
			uint	msgPosBefore = msg.getPosInBit();
			CActionFactory::getInstance()->pack(Actions[i], msg, false);
			uint	msgPosAfter = msg.getPosInBit();

			uint	actionSize = CActionFactory::getInstance()->size(Actions[i]);

			if (actionSize < msgPosAfter-msgPosBefore)
				nlwarning("Action %d declares a lower size (%d bits) from what it actually serialises (%d bits)", Actions[i]->Code, actionSize, msgPosAfter-msgPosBefore);
			//sprintf(cat, " %d(%d bits)", Actions[i]->Code, Actions[i]->size());
			//strcat(buff, cat);
		}
	}
	//nlinfo("Block: %s", buff);
}

} // CLFECOMMON




