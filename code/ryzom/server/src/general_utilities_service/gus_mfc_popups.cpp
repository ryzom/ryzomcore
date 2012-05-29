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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_mfc_select_with_text.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// static routines
	//-----------------------------------------------------------------------------

	bool selectFromList(const CVectorSString& options,CSString& selectResult)
	{ 
		CGusMfcSelect dlg(options);
		if (dlg.DoModal ()== IDOK)
		{
			selectResult= dlg.ComboValue;
			return true;
		}
		return false;
	}

	bool selectFromListWithText(const CVectorSString& options,CSString& selectResult,CSString& textResult)
	{ 
		CGusMfcSelectWithText dlg(options);
		if (dlg.DoModal ()== IDOK)
		{
			selectResult= dlg.ComboValue;
			textResult= dlg.TextValue;
			return true;
		}
		return false;
	}
}