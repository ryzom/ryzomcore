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



#ifndef NL_DBGROUP_SELECT_NUMBER_H
#define NL_DBGROUP_SELECT_NUMBER_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"


namespace NLGUI
{
	class CCtrlBaseButton;
	class CViewText;
	class CViewBitmap;

	// ***************************************************************************
	/**
	 * Widget to select a number
	 * \author Lionel Berenguier
	 * \author Nevrax France
	 * \date 2002
	 */
	class CDBGroupSelectNumber : public CInterfaceGroup
	{
	public:
        DECLARE_UI_CLASS( CDBGroupSelectNumber )

		/// Constructor
		CDBGroupSelectNumber(const TCtorParam &param);
		~CDBGroupSelectNumber();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		/// CInterfaceGroup Interface
		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
		virtual void updateCoords ();
		virtual void checkCoords();
		virtual void draw ();
		virtual void clearViews ();
		virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

		// mod interface
		void	changeValue(sint delta);

		sint32	getMinValue () const { return _MinValue; }
		void	setMinValue (sint32 m) { _MinValue = m; }
		sint32	getMaxValue () const { return _MaxValue; }
		void	setMaxValue (sint32 m) { _MaxValue = m; }

		sint32	getCurrentValue () const { return _Number.getSInt32(); }
		void	setCurrentValue (sint32 val) { _Number.setSInt32(val); }

		REFLECT_EXPORT_START(CDBGroupSelectNumber, CInterfaceGroup)
			REFLECT_SINT32("min", getMinValue, setMinValue);
			REFLECT_SINT32("max", getMaxValue, setMaxValue);
		REFLECT_EXPORT_END

	protected:

		// sint32
		CInterfaceProperty	_Number;
		bool				_LoopMode;
		sint				_MinValue;
		sint				_MaxValue;
		sint				_DeltaMultiplier;

		// Children
		CViewBitmap			*_SlotNumber;
		CViewText			*_TextNumber;
		CCtrlBaseButton		*_ButtonUp;
		CCtrlBaseButton		*_ButtonDown;

	private:

		void setup();

	};

}

#endif // NL_DBGROUP_SELECT_NUMBER_H

/* End of dbgroup_select_number.h */
