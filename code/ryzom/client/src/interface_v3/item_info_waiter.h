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



#ifndef RZ_ITEM_INFO_WAITER_H
#define RZ_ITEM_INFO_WAITER_H

class	IItemInfoWaiter
{
public:
	IItemInfoWaiter() {ItemSlotId= 0; ItemSheet= 0;}
	virtual ~IItemInfoWaiter() {}
	// The item SheetId. If differ from current sheet in the SlotId, the infos are not updated / requested
	uint			ItemSheet;
	// The item SlotId to retrieve info.
	uint			ItemSlotId;

	// Called when the info is received for this slot.
	virtual void	infoReceived() =0;
};

#endif // RZ_DBCTRL_SHEET_INFO_WAITER_H

/* End of item_info_waiter.h */

