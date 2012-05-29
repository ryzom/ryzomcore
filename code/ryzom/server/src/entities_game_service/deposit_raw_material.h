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



#ifndef RY_DEPOSIT_RAW_MATERIAL_H
#define RY_DEPOSIT_RAW_MATERIAL_H

// Struct deposit parameters
struct CDepositRawMaterial
{
	NLMISC::CSheetId	MaterialSheet;
	//uint16				MaxAmount;

	// defined operator = to fill structure with CStaticDeposit::CStaticDepositRawMaterial
	const CDepositRawMaterial &operator = (const CStaticDeposit::CStaticDepositRawMaterial& form )
	{
		MaterialSheet = form.MaterialSheet;
		//MaxAmount = form.MaxAmount;
		return *this;
	}

	/// serialize
//	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
//	{
//	}
};

#endif // RY_DEPOSIT_RAW_MATERIAL_H

/* End of deposit_raw_material.h */
