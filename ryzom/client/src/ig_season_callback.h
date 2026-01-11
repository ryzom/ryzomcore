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



#ifndef CL_IG_SEASON_CAll_BACK_H
#define CL_IG_SEASON_CAll_BACK_H

#include "ig_enum.h"

class CIGSeasonCallback : public IIGObserver
{
public:
	EGSPD::CSeason::TSeason	Season;
private:
	virtual void instanceGroupLoaded(NL3D::UInstanceGroup * /* ig */) { }
	virtual void instanceGroupAdded(NL3D::UInstanceGroup *ig);
	virtual void instanceGroupRemoved(NL3D::UInstanceGroup * /* ig */) { }
};

extern CIGSeasonCallback IGSeasonCallback;

#endif // CL_IG_SEASON_CAll_BACK_H
