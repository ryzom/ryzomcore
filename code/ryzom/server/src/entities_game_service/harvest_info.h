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



#ifndef RY_HARVEST_INFO_H
#define RY_HARVEST_INFO_H


namespace HARVEST_INFOS
{

struct CHarvestInfos
{
	NLMISC::TGameCycle EndCherchingTime;
	NLMISC::CSheetId Sheet;
	uint32	DepositIndex;
	uint32	DepositIndexContent;
	uint16	Quantity;
	uint16	MinQuality;
	uint16	MaxQuality;

	CHarvestInfos() : EndCherchingTime(0xffffffff), DepositIndex(0xffffffff), DepositIndexContent(0), Quantity(0),MinQuality(0),MaxQuality(0)
	{}

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( EndCherchingTime );
		f.serial( Sheet );
		f.serial( DepositIndex );
		f.serial( DepositIndexContent );
		f.serial( Quantity );
		f.serial( MinQuality );
		f.serial( MaxQuality );
	}
};

}
#endif // RY_HARVEST_INFO_H
/* End of harvest_info.h */
