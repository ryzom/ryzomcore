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



#ifndef RZ_CTRL_COL_PICK_H
#define RZ_CTRL_COL_PICK_H

#include "nel/misc/types_nl.h"
#include "nel/gui/ctrl_base.h"


namespace NLGUI
{

	/**
	 * Class handling a Color Picker
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2003
	 */
	class CCtrlColPick : public CCtrlBase
	{

	public:
        DECLARE_UI_CLASS( CCtrlColPick )

		CCtrlColPick(const TCtorParam &param);
		~CCtrlColPick();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);

		virtual void updateCoords();
		virtual void draw();
		virtual bool handleEvent (const NLGUI::CEventDescriptor &event);

		sint32 getColorR () const { return _ColorSelect.R; }
		sint32 getColorG () const { return _ColorSelect.G; }
		sint32 getColorB () const { return _ColorSelect.B; }
		sint32 getColorA () const { return _ColorSelect.A; }

		void setColorR (sint32 r) { _ColorSelect.R = (uint8)r; }
		void setColorG (sint32 g) { _ColorSelect.G = (uint8)g; }
		void setColorB (sint32 b) { _ColorSelect.B = (uint8)b; }
		void setColorA (sint32 a) { _ColorSelect.A = (uint8)a; }


		std::string getColor () const;			// Get Color Selected
		void setColor (const std::string &col);	// Set Color Selected

		std::string getColorOver () const;			// Get Color Over
		void setColorOver (const std::string &col);	// Set Color Over

		REFLECT_EXPORT_START(CCtrlColPick, CCtrlBase)
			REFLECT_SINT32("r", getColorR, setColorR);
			REFLECT_SINT32("g", getColorG, setColorG);
			REFLECT_SINT32("b", getColorB, setColorB);
			REFLECT_SINT32("a", getColorA, setColorA);
			REFLECT_STRING("color", getColor, setColor);
			REFLECT_STRING("color_over", getColorOver, setColorOver);
		REFLECT_EXPORT_END


	protected:

		void selectColor (sint32 x, sint32 y);
		NLMISC::CRGBA getColor (sint32 x, sint32 y);

	protected:

		bool _MouseDown;

		sint32 _Texture;

		NLMISC::CRGBA _ColorSelect;		// Last Color selected
		NLMISC::CRGBA _ColorOver;		// Color Under Mouse Pointer

		std::string _AHOnChange;
		std::string _AHOnChangeParams;

		CInterfaceProperty _ColSelR;
		CInterfaceProperty _ColSelG;
		CInterfaceProperty _ColSelB;
		CInterfaceProperty _ColSelA;
	};

}


#endif // RZ_CTRL_COL_PICK_H

/* End of ctrl_col_pick.h */


