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



#ifndef RYZOM_STATIC_DEPOSIT_H
#define RYZOM_STATIC_DEPOSIT_H

//Nel georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
// std
#include <vector>

// TEMP
typedef uint32 TTerrain;


/*
 * Struct deposit parameters
 */
class CStaticDepositRawMaterial
{
public:
	NLMISC::CSheetId	MaterialSheet;
	//uint16				MaxAmount;

	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( MaterialSheet );
		//f.serial( MaxAmount );
	}
};


/**
 * CStaticDeposit
 *
 * \author Alain Saffray, Olivier Cado
 * \author Nevrax France
 * \date 2002-2003
 */
/*class CStaticDeposit
{
public :
		
	/// read sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );
	
	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 3; }
	
	/// destructor
	virtual ~CStaticDeposit() {}
	
	/// called when the sheet is removed
	void removed() {}
	
	/// Serialize deposit
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{}		
};*/

#endif // RYZOM_STATIC_DEPOSIT_H

/* End of static_deposit.h */
