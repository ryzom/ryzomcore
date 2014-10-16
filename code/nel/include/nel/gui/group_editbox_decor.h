// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2014  Laszlo Kis-Adam
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

#ifndef GROUP_EDITBOX_DECOR
#define GROUP_EDITBOX_DECOR

#include "nel/gui/group_editbox.h"

namespace NLGUI
{
	/// Decorated CGroupEditBox
	class CGroupEditBoxDecor : public CGroupEditBox
	{
	public:
		DECLARE_UI_CLASS( CGroupEditBoxDecor )

		CGroupEditBoxDecor( const TCtorParam &param );
		~CGroupEditBoxDecor();

		void moveBy( sint32 x, sint32 y );

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		bool parse( xmlNodePtr cur, CInterfaceGroup *parent );
		void draw();
		void updateCoords();

		static void forceLink();

	private:
		class EBDPrivate *_Pvt;
	};
}

#endif

