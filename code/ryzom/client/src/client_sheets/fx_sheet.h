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




#ifndef CL_FX_SHEET_H
#define CL_FX_SHEET_H


/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
#include "entity_sheet.h"
#include <vector>
#include <string>


///////////
// USING //
///////////


///////////
// CLASS //
///////////
namespace NLGEORGES
{
	class UFormElm;
}
/**
 * Class to manage the FX sheet.
 *
 * CFxCL used a part of this sheet class:
 * In the first item in PSList:
 * - PSName
 * - 'Standard' user params 0 and 1 (see CFXSheet::CPSStruct::StandardIndex)
 *
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2001
 */
class CFXSheet : public CEntitySheet
{
public:
	class CPSStruct
	{
	public:
		class CPSUserParamStruct
		{
		public:
			/// Value for the USer Param 0
			float		UserParam0;
			/// Value for the USer Param 1
			float		UserParam1;
			/// Value for the USer Param 2
			float		UserParam2;
			/// Value for the USer Param 3
			float		UserParam3;

			/// Constructor
			CPSUserParamStruct()
			{
				UserParam0 = 0.f;
				UserParam1 = 0.f;
				UserParam2 = 0.f;
				UserParam3 = 0.f;
			}

			/// Serialize character sheet into binary data file.
			virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
			{
				f.serial(UserParam0);
				f.serial(UserParam1);
				f.serial(UserParam2);
				f.serial(UserParam3);
			}
		};

		/// PS Filename.
		std::string						PSName;
		/// PS launch according the animation or at the beginning.
		bool							Anim;
		/// Parameters by Power.
		std::vector<CPSUserParamStruct> Power;
		/// Index of the 'Standard' power
		static uint StandardIndex;


		/// Constructor
		CPSStruct()
		{
			Anim = false;
		}

		/// Serialize character sheet into binary data file.
		virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(PSName);
			f.serial(Anim);
			f.serialCont(Power);
		}
	};

	std::vector<std::string>	TrailList;
	std::vector<CPSStruct>		PSList;

	/// Constructor
	CFXSheet();

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize character sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};


#endif // CL_FX_SHEET_H

/* End of fx_sheet.h */
