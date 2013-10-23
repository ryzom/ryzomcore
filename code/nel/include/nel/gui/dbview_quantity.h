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



#ifndef NL_DBVIEW_QUANTITY_H
#define NL_DBVIEW_QUANTITY_H

#include "nel/misc/types_nl.h"
#include "nel/gui/view_text.h"

namespace NLGUI
{

	// ***************************************************************************
	/**
	 * Display a text in the form of val / max or "empty"
	 * \author Lionel Berenguier
	 * \author Nevrax France
	 * \date 2002
	 */
	class CDBViewQuantity : public CViewText
	{
	public:
        DECLARE_UI_CLASS( CDBViewQuantity )

		/// Constructor
		CDBViewQuantity(const TCtorParam &param);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		virtual bool parse (xmlNodePtr cur, CInterfaceGroup * parentGroup);
		virtual void draw ();

		static void forceLink();


	protected:
		CInterfaceProperty		_Number;
		CInterfaceProperty		_NumberMax;
		sint32					_Cache;
		sint32					_CacheMax;
		ucstring			_EmptyText;

		void	buildTextFromCache();
	};

}

#endif // NL_DBVIEW_QUANTITY_H

/* End of dbview_quantity.h */
