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

#include "std3d.h"
#include "nel/3d/ps_attrib_maker_helper.h"

namespace NL3D
{


// ***********************************************************************************
void CPSAttribMakerMemory<uint32>::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSAttribMakerMemoryBase<uint32>::serial(f);
	if (f.isReading())
	{
		if (_T.getSize() != 0)
		{
			_MinValue = _MaxValue = _T[0];
			for(uint k = 1; k < _T.getSize(); ++k)
			{
				_MinValue = std::min(_MinValue, _T[k]);
				_MaxValue = std::max(_MaxValue, _T[k]);
			}
		}
	}
}

// ***********************************************************************************
void CPSAttribMakerMemory<uint32>::newElement(const CPSEmitterInfo &info)
{
	CPSAttribMakerMemoryBase<uint32>::newElement(info);
	if (_T.getSize() > 1)
	{
		_MinValue = std::min(_MinValue, _T.back());
		_MaxValue = std::max(_MaxValue, _T.back());
	}
	else
	{
		_MinValue = _MaxValue = _T[0];
	}
}

// ***********************************************************************************
void CPSAttribMakerMemory<sint32>::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSAttribMakerMemoryBase<sint32>::serial(f);
	if (f.isReading())
	{
		if (_T.getSize() != 0)
		{
			_MinValue = _MaxValue = _T[0];
			for(uint k = 1; k < _T.getSize(); ++k)
			{
				_MinValue = std::min(_MinValue, _T[k]);
				_MaxValue = std::max(_MaxValue, _T[k]);
			}
		}
	}
}

// ***********************************************************************************
void CPSAttribMakerMemory<sint32>::newElement(const CPSEmitterInfo &info)
{
	CPSAttribMakerMemoryBase<sint32>::newElement(info);
	if (_T.getSize() > 1)
	{
		_MinValue = std::min(_MinValue, _T.back());
		_MaxValue = std::max(_MaxValue, _T.back());
	}
	else
	{
		_MinValue = _MaxValue = _T[0];
	}
}

// ***********************************************************************************
void CPSAttribMakerMemory<float>::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSAttribMakerMemoryBase<float>::serial(f);
	if (f.isReading())
	{
		if (_T.getSize() != 0)
		{
			_MinValue = _MaxValue = _T[0];
			for(uint k = 1; k < _T.getSize(); ++k)
			{
				_MinValue = std::min(_MinValue, _T[k]);
				_MaxValue = std::max(_MaxValue, _T[k]);
			}
		}
	}
}

// ***********************************************************************************
void CPSAttribMakerMemory<float>::newElement(const CPSEmitterInfo &info)
{
	CPSAttribMakerMemoryBase<float>::newElement(info);
	if (_T.getSize() > 1)
	{
		_MinValue = std::min(_MinValue, _T.back());
		_MaxValue = std::max(_MaxValue, _T.back());
	}
	else
	{
		_MinValue = _MaxValue = _T[0];
	}
}


} // NL3D
