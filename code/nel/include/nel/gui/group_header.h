// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef CL_GROUP_HEADER_H
#define CL_GROUP_HEADER_H

#include "nel/gui/group_list.h"

namespace NLGUI
{

	class CGroupHeaderEntry;

	// *****************************************************************************************************************
	/** Display a header with movable entries.
	  * Usually used with a table to change the size of each column (much like the windows file explorer in 'details' mode)
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 2006
	  */
	class CGroupHeader : public CGroupList
	{
	public:
        DECLARE_UI_CLASS( CGroupHeader )

		REFLECT_EXPORT_START(CGroupHeader, CGroupList)
			REFLECT_LUA_METHOD("enlargeColumns", luaEnlargeColumns);
			REFLECT_LUA_METHOD("resizeColumnsAndContainer", luaResizeColumnsAndContainer);
		REFLECT_EXPORT_END

		CGroupHeader(const TCtorParam &param);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		// from CInterfaceGroup
		virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);
		sint32	getHeaderMaxSize() const { return _HeaderMaxSize; }
		// get the entries in this header
		void getEntries(std::vector<CGroupHeaderEntry *> &dest);
		// ensure that max. content of columns is visible (without the total width becoming more than 'getHeaderMaxSize()'
		void enlargeColumns(sint32 margin);
		// ensure that content of each column is visible
		void resizeColumnsAndContainer(sint32 margin);
	private:
		sint32	_HeaderMaxSize;
		int luaEnlargeColumns(CLuaState &ls);
		int luaResizeColumnsAndContainer(CLuaState &ls);
	};

	// *****************************************************************************************************************
	// an entry in a header, includes a "mover control" to move it inside its parent header
	// NOTE : when not used inside a CGroupHeader, will work, but there will be no 'max_size'
	class CGroupHeaderEntry : public CInterfaceGroup
	{
	public:
        DECLARE_UI_CLASS( CGroupHeaderEntry )

		CGroupHeaderEntry(const TCtorParam &param);
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		// from CInterfaceGroup
		virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);
		sint32 getMinSize() const { return _MinSize; }
		virtual void updateCoords();
		CInterfaceGroup *getTargetColumn() const;

		const	std::string &getAHOnResize() const { return _AHOnResize; }
		const	std::string &getAHOnResizeParams() const { return _AHOnResizeParams; }

	private:
		sint32 _MinSize;
		sint32 _ResizerSize;
		std::string _TargetColumnId;
		std::string _AHOnResize;
		std::string _AHOnResizeParams;
	};


}

#endif

