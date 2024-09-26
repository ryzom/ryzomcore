// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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

#ifndef CL_CSS_LENGTH_H
#define CL_CSS_LENGTH_H

#include "nel/misc/types_nl.h"
#include "nel/gui/css_types.h"

namespace NLGUI
{
	/**
	 * \brief CSS types used in GUI classes
	 * \date 2021-07-02 11:36 GMT
	 * \author Meelis Mägi (Nimetu)
	 */

	class CSSLength
	{
	public:
		enum Kind {
			Auto, Relative, Fixed
		};

		CSSLength(float value = 0, CSSUnitType unit = CSS_UNIT_NONE, Kind kind = Auto)
		: m_Value(value), m_Unit(unit), m_Kind(Auto)
		{}

		void setAuto() { m_Kind = Auto; }
		bool parseValue(const std::string &value, bool allowPercent = true, bool allowNegative = false);
		void setFloatValue(float f, const std::string &unit);

		float getValue() const;
		float getFloat() const { return m_Value; }

		bool isPercent() const { return m_Unit == CSS_UNIT_PERCENT; }

		bool isAuto() const { return m_Kind == Auto; }
		bool isRelative() const { return m_Kind == Relative; }

		// % uses relValue
		// em uses emSize
		// rem uses remSize
		// vw,vh,vi,vb,vmin,vmax uses vwSize/vhSize
		float calculate(uint32 relValue, uint32 emSize, uint32 remSize, uint32 vwSize, uint32 whSize) const;

		CSSUnitType getUnit() const { return m_Unit; }

		std::string toString() const;

	private:
		void setUnit(const std::string &unit);

		float m_Value;
		CSSUnitType m_Unit;
		Kind m_Kind;
	};

}//namespace

#endif // CL_CSS_LENGTH_H


