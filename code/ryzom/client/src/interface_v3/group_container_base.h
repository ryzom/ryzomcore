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


#ifndef GROUP_CONTAINER_BASE_H
#define GROUP_CONTAINER_BASE_H

#include "interface_group.h"

class CGroupContainerBase : public CInterfaceGroup
{
public:
	DECLARE_UI_CLASS( CGroupContainerBase )
	CGroupContainerBase( const TCtorParam &param );
	virtual ~CGroupContainerBase();

	virtual void removeAllContainers();
	virtual void setLocked( bool locked );

	REFLECT_EXPORT_START( CGroupContainerBase, CInterfaceGroup )
	REFLECT_EXPORT_END

protected:

private:

};

#endif
