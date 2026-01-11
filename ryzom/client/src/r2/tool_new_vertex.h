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

#ifndef R2_TOOL_NEW_VERTEX_H
#define R2_TOOL_NEW_VERTEX_H


#include "tool_choose_pos.h"

namespace R2
{

	class CToolNewVertex : public CToolChoosePos
	{
	public:
		NLMISC_DECLARE_CLASS(R2::CToolNewVertex)
		CToolNewVertex();
	protected:
		// from CToolChoosePos
		virtual bool isValidChoosePos(const NLMISC::CVector2f &pos) const;
		virtual void commit(const NLMISC::CVector &createPosition, float createAngle);
		virtual const char *getToolUIName() const;
		// from CTool
		virtual bool onDeleteCmd();
	private:
		mutable sint			_CurrEdge;
		mutable NLMISC::CVector _CurrPos;
	};

} // R2


#endif
