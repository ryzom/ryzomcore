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



#ifndef NL_BASE_DIALOG_H
#define NL_BASE_DIALOG_H

#include "nel/misc/types_nl.h"


/**
 * Base dialog for option dialogs
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2003
 */
class CBaseDialog : public CDialog
{
public:

	/// Constructor
	CBaseDialog (uint id, CWnd* pParent = NULL) : CDialog(id, pParent) {}

	/// On ok
	virtual void OnOK ();
	virtual void OnCancel ();
};


#endif // NL_BASE_DIALOG_H

/* End of base_dialog.h */
