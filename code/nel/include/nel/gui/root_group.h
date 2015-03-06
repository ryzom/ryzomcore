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


#ifndef ROOT_GROUP_H
#define ROOT_GROUP_H

#include <string>
#include <map>

#include "nel/gui/interface_group.h"

namespace NLGUI
{

	class CRootGroup : public CInterfaceGroup
	{
	public:
		CRootGroup(const TCtorParam &param);
		virtual ~CRootGroup();

		virtual CInterfaceElement* getElement (const std::string &id);
		virtual void addGroup (CInterfaceGroup *child, sint eltOrder = -1);
		virtual bool delGroup (CInterfaceGroup *child, bool dontDelete = false);

	private:
		std::map< std::string, CInterfaceGroup* > _Accel;
	};

}

#endif

