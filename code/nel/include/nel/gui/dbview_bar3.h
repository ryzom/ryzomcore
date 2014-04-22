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



#ifndef RZ_DBVIEW_BAR3_H
#define RZ_DBVIEW_BAR3_H

#include "nel/misc/types_nl.h"
#include "nel/gui/view_bitmap.h"

namespace NLGUI
{

	/**
	 * class implementing a 3 Bar widget
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CDBViewBar3 : public CViewBitmap
	{
	public:
        DECLARE_UI_CLASS( CDBViewBar3 )

		/// Constructor
		CDBViewBar3(const TCtorParam &param);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }
		virtual void updateCoords ();

		void setMini (bool mini);

		virtual void draw ();

		/// Nbs: Values by Int are not used if the Links are setuped
		void setValue0 (sint32 r) { _ValueInt[0] = r; }
		void setValue1 (sint32 r) { _ValueInt[1] = r; }
		void setValue2 (sint32 r) { _ValueInt[2] = r; }
		void setRange0 (sint32 r) { _RangeInt[0] = r; }
		void setRange1 (sint32 r) { _RangeInt[1] = r; }
		void setRange2 (sint32 r) { _RangeInt[2] = r; }
		sint32	getValue0 () const { return _ValueInt[0]; }
		sint32	getValue1 () const { return _ValueInt[1]; }
		sint32	getValue2 () const { return _ValueInt[2]; }
		sint32	getRange0 () const { return _RangeInt[0]; }
		sint32	getRange1 () const { return _RangeInt[1]; }
		sint32	getRange2 () const { return _RangeInt[2]; }

		// Reflect ValueInt (ie not used if the link is setuped)
		REFLECT_EXPORT_START(CDBViewBar3, CViewBitmap)
			REFLECT_SINT32 ("value1", getValue0, setValue0);
			REFLECT_SINT32 ("value2", getValue1, setValue1);
			REFLECT_SINT32 ("value3", getValue2, setValue2);
			REFLECT_SINT32 ("range1", getRange0, setRange0);
			REFLECT_SINT32 ("range2", getRange1, setRange1);
			REFLECT_SINT32 ("range3", getRange2, setRange2);
		REFLECT_EXPORT_END

		static void forceLink();

	protected:

		CViewBitmap _Slot;

		// Value of the progression in arbitrary units. should be integer
		CInterfaceProperty _Value[3];
		// Max range of the progression in arbitrary units. should be integer
		CInterfaceProperty _Range[3];

		/// Nbs: Values by Int are not used if the Links are setuped. NB: not overwritten by links
		sint32				_ValueInt[3];
		sint32				_RangeInt[3];


		NLMISC::CRGBA _Colors[3];
		NLMISC::CRGBA _ColorsNegative[3];

		bool		_Mini;

		// Height of the bitmap
		sint32		_BarH;

		void parseValProp(xmlNodePtr cur, CInterfaceProperty &dbProp, sint32 &intProp, const char *name);
		void setValProp( const std::string &value, CInterfaceProperty &dbProp, sint32 &intProp );
		sint32	getCurrentValProp(const CInterfaceProperty &dbProp, sint32 intProp);
		std::string getValProp( const CInterfaceProperty &prop, sint32 intProp  ) const;
	};

}

#endif // RZ_DBVIEW_BAR3_H

/* End of dbview_bar3.h */
