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
#include "string_manager.h"
#include "nel/misc/i18n.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
#include <time.h>

// load the values using the george sheet
void CStringManager::TSheetInfo::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	if (form)
	{
		SheetName = sheetId.toString();

		std::string ext = NLMISC::CSheetId::fileExtensionFromType(sheetId.getSheetType());

		SheetName = SheetName.substr(0, SheetName.find(ext));
		// remove ending '.'
		if (!SheetName.empty() && *SheetName.rbegin() == '.')
			SheetName.resize(SheetName.size()-1);

		std::string gender;

		if (sheetId.getSheetType() == NLMISC::CSheetId::typeFromFileExtension("creature"))
		{
			form->getRootNode ().getValueByName (gender, "Basics.Gender");
			sint genderId;
			NLMISC::fromString(gender, genderId);
			Gender = GSGENDER::EGender(genderId);

			form->getRootNode ().getValueByName (Race, "Basics.Race");

//			form->getRootNode ().getValueByName (DisplayName, "Basics.First Name");
//			std::string s;
//			form->getRootNode ().getValueByName (s, "Basics.CharacterName");
//			if (!DisplayName.empty())
//				DisplayName+=' ';
//			DisplayName+=s;

			form->getRootNode ().getValueByName (Profile, "Basics.Profile");
			form->getRootNode ().getValueByName (ChatProfile, "Basics.ChatProfile");
		}
		else if (sheetId.getSheetType() == NLMISC::CSheetId::typeFromFileExtension("race_stats"))
		{
			form->getRootNode ().getValueByName (Race, "Race");
		}
/*		else if (sheetId.getType() == NLMISC::CSheetId::typeFromFileExtension("sitem"))
		{
			// read any item specific data
		}
*/		else
		{
			nlwarning("CStringManager::TEntityInfo : Do not know the type of the sheet '%s'.", sheetId.toString().c_str());
			return;
		}
	}
}
