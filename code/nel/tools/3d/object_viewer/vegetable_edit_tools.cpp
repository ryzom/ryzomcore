// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_edit_tools.h"


CDirectEditableRangeFloat::CDirectEditableRangeFloat(const std::string &id, float defaultMin, float defaultMax, const std::string &title)
	: CEditableRangeFloat(id, NULL, defaultMin, defaultMax), _Title(title)
{
}


void	CDirectEditableRangeFloat::init(uint32 x, uint32 y, CWnd *pParent)
{
	uint	dx= 60;

	CEditableRangeFloat::init(x+dx, y, pParent);

	CRect	rect;
	rect.SetRect(x, y+10, x+dx, y+25);
	_StaticText.Create(_Title.c_str(), WS_CHILD | WS_VISIBLE, rect, pParent);
	_StaticText.SetFont(pParent->GetFont());
}


void		CDirectEditableRangeFloat::setFloat(float *value, IVegetableRefresh *vegetRefresh)
{
	nlassert(value);
	_Wrapper.Value= value;
	_Wrapper.VegetableRefresh= vegetRefresh;
	setWrapper(&_Wrapper);

	// update view.
	updateValueFromReader();
}


void		CDirectEditableRangeFloat::enableWrite(bool enb)
{
	_Wrapper.WriteEnabled= enb;
}

float		CDirectEditableRangeFloat::CDirectFloatWrapper::get(void) const
{
	return *Value;
}
void		CDirectEditableRangeFloat::CDirectFloatWrapper::set(const float &f)
{
	if(WriteEnabled)
	{
		*Value= f;
		VegetableRefresh->refreshVegetableDisplay();
	}
}
