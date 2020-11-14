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



#ifndef RZ_DBVIEW_BAR_H
#define RZ_DBVIEW_BAR_H

#include "nel/misc/types_nl.h"
#include "nel/gui/view_bitmap.h"

namespace NLGUI
{

	/**
	 * class implementing a bitmap used as the front texture of a progress bar
	 * the bitmap is drawn from _X to _W * _Range/_RangeMax
	 * \author Nicolas Brigand
	 * \author Nevrax France
	 * \date 2002
	 */
	class CDBViewBar : public CViewBitmap
	{
	public:
		enum TViewBar { ViewBar_UltraMini, ViewBar_Mini, ViewBar_Normal, ViewBar_MiniThick };
	public:
        DECLARE_UI_CLASS( CDBViewBar )

		/// Constructor
		CDBViewBar(const TCtorParam &param)
			: CViewBitmap(param),
			_Slot(TCtorParam())
		{
			_Color= NLMISC::CRGBA::White;
			_ValueInt= 0;
			_RangeInt = 255;
			_ReferenceInt= 0;
			_Type = ViewBar_Normal;
		}

		void setType (TViewBar vb);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }
		virtual void updateCoords ();
		virtual void draw ();

		/// Nbs: Values by Int are not used if the Links are setuped
		void setValue (sint32 r) { _ValueInt = r; }
		void setRange (sint32 r) { _RangeInt = r; }
		void setReference (sint32 r) { _ReferenceInt = r; }
		sint32	getValue () const { return _ValueInt; }
		sint32	getRange () const { return _RangeInt; }
		sint32	getReference () const { return _ReferenceInt; }

		void setValueDbLink (const std::string &r);
		void setRangeDbLink (const std::string &r);
		void setReferenceDbLink (const std::string &r);
		std::string getValueDbLink () const;
		std::string getRangeDbLink () const;
		std::string getReferenceDbLink () const;

		// Reflect ValueInt (ie not used if the link is setuped)
		REFLECT_EXPORT_START(CDBViewBar, CViewBitmap)
			REFLECT_SINT32 ("value", getValue, setValue);
			REFLECT_SINT32 ("range", getRange, setRange);
			REFLECT_SINT32 ("reference", getReference, setReference);
			REFLECT_STRING ("value_dblink", getValueDbLink, setValueDbLink);
			REFLECT_STRING ("range_dblink", getRangeDbLink, setRangeDbLink);
			REFLECT_STRING ("reference_dblink", getReferenceDbLink, setReferenceDbLink);
		REFLECT_EXPORT_END

	protected:

		CViewBitmap		_Slot;
		TViewBar		_Type;
		sint32			_HBar;
		NLMISC::CRGBA	_ColorNegative;

		// Value of the progression in arbitrary units. should be integer
		CInterfaceProperty _Value;
		// Max range of the progression in arbitrary units. should be integer
		CInterfaceProperty _Range;
		// Reference of the progression (substracted from value and range).
		CInterfaceProperty _Reference;

		/// Nbs: Values by Int are not used if the Links are setuped. NB: not overwritten by links
		sint32	_ValueInt;
		sint32	_RangeInt;
		sint32	_ReferenceInt;

		void	parseValProp(xmlNodePtr cur, CInterfaceProperty &dbProp, sint32 &intProp, const char *name);
		sint64	getCurrentValProp(const CInterfaceProperty &dbProp, sint32 intProp);
	};

}

#endif // RZ_DBVIEW_BAR_H

/* End of dbview_bar.h */
