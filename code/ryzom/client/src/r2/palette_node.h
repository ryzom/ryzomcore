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

#ifndef R2_PALETTE_NODE_H
#define R2_PALETTE_NODE_H


#if 0
//#include "interface_user_data.h"
#include "nel/gui/group_tree.h"
#include "nel/gui/lua_object.h"
//
#include "nel/misc/smart_ptr.h"


namespace R2
{

/**
  * A palette node, to be attached to a CInterfaceElement. Makes a reference to a lua table describing the instance to be copied
  */

class CPaletteNode : public NLMISC::CRefCount
{
public:
	typedef NLMISC::CSmartPtr<CPaletteNode> TSmartPtr;
	//
	virtual ~CPaletteNode() {}
	// Set the instance for this palette node
	void        setInstance(CLuaObject &instance) { _Instance = instance; }
	// Retrieve instance
	const CLuaObject &getInstance() const { return _Instance; }
	// Retrieve lua class definition for that object
	const CLuaObject getClass() const;
private:
	CLuaObject	_Instance;
};



} // R2
#endif

#endif
