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



#ifndef VIEW_TEXT_ID_FORMATED_H
#define VIEW_TEXT_ID_FORMATED_H

#include "nel/gui/view_text_id.h"

/** The same as a view text id, but with some display option
  * The input is a formated string, every character is copied, but subsitution is done for each character preceded by $
  * $t -> expand the value of the text id
  * $p -> expand the player name
  * $P -> expand the player name in uppercase
  * $b -> expand the current bot name ( bot with which the player is talking)
  * $s -> expand the current short bot name (with no specification/title in it)
  */

namespace NLGUI
{

	class CViewTextIDFormated : public  CViewTextID
	{
	public:
        DECLARE_UI_CLASS( CViewTextIDFormated )

		CViewTextIDFormated(const TCtorParam &param) : CViewTextID(param)
		{}

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);
		virtual void checkCoords();
		const  ucstring &getFormatString() const { return _FormatString; }
		void setFormatString(const ucstring &format);
	private:
		ucstring	_FormatString;
	};

}

#endif
