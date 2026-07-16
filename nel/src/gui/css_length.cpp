// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2022  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2021  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdpch.h"

#include <string>
#include "nel/gui/css_length.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

bool CSSLength::parseValue(const std::string &value, bool allowPercent, bool allowNegative)
{
	static const std::string knownUnitsArr[] = {
		"%", "rem", "em", "px", "pt", "vw", "vh", "vi", "vb", "vmin", "vmax"
	};
	static const std::set<std::string> knownUnits(knownUnitsArr, &knownUnitsArr[sizeof(knownUnitsArr) / sizeof(knownUnitsArr[0])]);

	std::string::size_type pos = 0;
	std::string::size_type len = value.size();
	if (len == 0)
		return false;

	if (len == 1 && value[0] == '0')
	{
		m_Value = 0;
		m_Kind = Auto;
		return true;
	}

	// +100px; -100px
	if (value[0] == '+')
		pos++;
	else if (allowNegative && value[0] == '-')
		pos++;

	while(pos < len)
	{
		bool isNumeric = (value[pos] >= '0' && value[pos] <= '9')
			|| (pos == 0 && value[pos] == '.')
			|| (pos > 0 && value[pos] == '.' && value[pos-1] >= '0' && value[pos-1] <= '9');

		if (!isNumeric)
			break;

		pos++;
	}

	std::string unit = toLowerAscii(value.substr(pos));
	if (!allowPercent && unit == "%")
		return false;

	if (knownUnits.count(unit))
	{
		std::string tmpstr = value.substr(0, pos);
		if (fromString(tmpstr, m_Value))
		{
			setUnit(unit);
			return true;
		}
	}

	return false;
}

float CSSLength::getValue() const
{
	if (m_Unit == CSS_UNIT_PERCENT)
		return m_Value / 100.f;

	return m_Value;
}
void CSSLength::setFloatValue(float f, const std::string &unit)
{
	m_Value = f;
	setUnit(unit);
}

void CSSLength::setUnit(const std::string &unit)
{
	if (unit.empty())
	{
		m_Unit = CSS_UNIT_NONE;
		m_Kind = Fixed;
	}
	else if (unit == "px")
	{
		m_Unit = CSS_UNIT_PX;
		m_Kind = Fixed;
	} else if (unit == "pt")
	{
		m_Unit = CSS_UNIT_PT;
		m_Kind = Fixed;
	} else if (unit == "%")
	{
		m_Unit = CSS_UNIT_PERCENT;
		m_Kind = Relative;
	} else if (unit == "em")
	{
		m_Unit = CSS_UNIT_EM;
		m_Kind = Relative;
	} else if (unit == "rem")
	{
		m_Unit = CSS_UNIT_REM;
		m_Kind = Relative;
	} else if (unit == "vw")
	{
		m_Unit = CSS_UNIT_VW;
		m_Kind = Relative;
	} else if (unit == "vh")
	{
		m_Unit = CSS_UNIT_VH;
		m_Kind = Relative;
	} else if (unit == "vi")
	{
		m_Unit = CSS_UNIT_VI;
		m_Kind = Relative;
	} else if (unit == "vb")
	{
		m_Unit = CSS_UNIT_VB;
		m_Kind = Relative;
	} else if (unit == "vmin")
	{
		m_Unit = CSS_UNIT_VMIN;
		m_Kind = Relative;
	} else if (unit == "vmax")
	{
		m_Unit = CSS_UNIT_VMAX;
		m_Kind = Relative;
	} else if (unit == "auto")
	{
		m_Unit = CSS_UNIT_NONE;
		m_Kind = Auto;
	} else
	{
		// fallback to auto
		m_Unit = CSS_UNIT_NONE;
		m_Kind = Auto;
	}
}

float CSSLength::calculate(uint32 relValue, uint32 emSize, uint32 remSize, uint32 vwSize, uint32 vhSize = 0) const
{
	if (m_Kind == Auto)
		return 0;

	float value = getValue();
	switch(m_Unit)
	{
		case CSS_UNIT_EM:
			return emSize * value;
		case CSS_UNIT_REM:
			return remSize * value;
		case CSS_UNIT_PERCENT:
			return relValue * value;
		case CSS_UNIT_PX:
		case CSS_UNIT_PT:
			return value;
		case CSS_UNIT_VW:
		case CSS_UNIT_VI:
			// Vi for horizontal writing mode only
			return (float)vwSize*0.01f;
		case CSS_UNIT_VH:
		case CSS_UNIT_VB:
			// Vb for horizontal writing mode only
			return (float)vhSize*0.01f;
		case CSS_UNIT_VMIN:
			return (float)std::min(vwSize, vhSize)*0.01f;
		case CSS_UNIT_VMAX:
			return (float)std::max(vwSize, vhSize)*0.01f;
	}

	nldebug("Unknown CSS unit '%s'", toString().c_str());
	return value;
}

std::string CSSLength::toString() const
{
	if (m_Kind == Auto)
		return "auto";

	std::string ret;
	ret += NLMISC::toString("%f", m_Value);

	size_t pos = ret.find(".");
	for( ; pos < ret.size(); ++pos)
	{
		if (ret[pos] != '0')
			break;
	}
	if (pos == ret.size())
		ret = ret.substr(0, ret.find("."));

	switch(m_Unit)
	{
		case CSS_UNIT_NONE: break;
		case CSS_UNIT_EM: ret += "em"; break;
		case CSS_UNIT_REM: ret += "rem"; break;
		case CSS_UNIT_PERCENT: ret += "%"; break;
		case CSS_UNIT_PX: ret += "px"; break;
		case CSS_UNIT_PT: ret += "pt"; break;
		case CSS_UNIT_VW: ret += "vw"; break;
		case CSS_UNIT_VH: ret += "vh"; break;
		case CSS_UNIT_VI: ret += "vi"; break;
		case CSS_UNIT_VB: ret += "vb"; break;
		case CSS_UNIT_VMIN: ret += "vmin"; break;
		case CSS_UNIT_VMAX: ret += "vmax"; break;
	}

	return ret;
}

} // namespace

