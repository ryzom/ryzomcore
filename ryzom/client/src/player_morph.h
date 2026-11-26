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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef CL_PLAYER_MORPH_H
#define CL_PLAYER_MORPH_H

#include "nel/misc/sheet_id.h"

/// Answer the server's morph request once the creature can be displayed
void prepareMorph(uint32 sheet);

/// Store the abilities of the user's morph and show them when the user has that creature form
void setMorphAbilities(uint32 sheet, const std::vector<NLMISC::CSheetId> &actions, const std::vector<std::string> &labels, bool attackable);

/// Show or hide the abilities panel after the user's form changed
void updateMorphPanel();

/// The user's morph leaves its creature mood to move
void endMorphMood();

#endif // CL_PLAYER_MORPH_H
