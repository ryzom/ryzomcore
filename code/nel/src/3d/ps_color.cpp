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

#include "nel/3d/ps_color.h"
#include "nel/3d/ps_register_color_attribs.h"


namespace NL3D {

using NLMISC::CRGBA ;

CRGBA CPSColorGradient::_DefaultGradient[] =
{
	CRGBA(0, 0, 0),
	CRGBA(255, 255, 255)
};




CPSColorGradient::CPSColorGradient()  : CPSAttribMakerRGBA<CPSValueGradientFuncRGBA>(1.f)
{
}


///======================================================================================
CPSColorGradient::CPSColorGradient(const CRGBA *colorTab, uint32 nbValues, uint32 nbStages, float nbCycles)
				: CPSAttribMakerRGBA<CPSValueGradientFuncRGBA>(nbCycles)
{
	_F.setValues(colorTab, nbValues, nbStages) ;
}

///======================================================================================
void CPSColorMemory::setColorType(CVertexBuffer::TVertexColorType colorType)
{
	if (colorType != _ColorType)
	{
		if (_T.getSize())
		{
			convertVBColor(&_T[0], _T.getSize(), CVertexBuffer::TBGRA);
		}
		_DefaultValue = convertVBColor(_DefaultValue, CVertexBuffer::TBGRA);
		_ColorType = colorType;
	}
	if (_Scheme) _Scheme->setColorType(colorType);
}

///======================================================================================
void CPSColorMemory::setDefaultValue(NLMISC::CRGBA defaultValue)
{
	CPSAttribMakerMemory<NLMISC::CRGBA>::setDefaultValue(convertVBColor(defaultValue, _ColorType));
}

///======================================================================================
NLMISC::CRGBA CPSColorMemory::getDefaultValue(void) const
{
	return convertVBColor(CPSAttribMakerMemory<NLMISC::CRGBA>::getDefaultValue(), _ColorType);
}

///======================================================================================
void CPSColorMemory::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	setColorType(CVertexBuffer::TRGBA);
	CPSAttribMakerMemory<NLMISC::CRGBA>::serial(f);
}

///======================================================================================
void CPSColorBinOp::setColorType(CVertexBuffer::TVertexColorType colorType)
{
	if (_Arg[0]) _Arg[0]->setColorType(colorType);
	if (_Arg[1]) _Arg[1]->setColorType(colorType);
}

///======================================================================================
void CPSColorBinOp::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	setColorType(CVertexBuffer::TRGBA);
	CPSAttribMakerBinOp<NLMISC::CRGBA>::serial(f);
}

///======================================================================================
void CPSValueBlendFuncRGBA::setColorType(CVertexBuffer::TVertexColorType colorType)
{
	if (colorType == _ColorType) return;
	_StartValue = convertVBColor(_StartValue, CVertexBuffer::TBGRA);
	_EndValue = convertVBColor(_EndValue, CVertexBuffer::TBGRA);
	_ColorType = colorType;
}

///======================================================================================
void CPSValueBlendSampleFuncRGBA::setColorType(CVertexBuffer::TVertexColorType colorType)
{
	if (colorType == _ColorType) return;
	convertVBColor(_Values, RGBA_BLENDER_NUM_VALUES + 1, CVertexBuffer::TBGRA);
	_ColorType = colorType;
}

///======================================================================================
void CPSValueGradientFuncRGBA::setColorType(CVertexBuffer::TVertexColorType colorType)
{
	if (colorType == _ColorType) return;
	convertVBColor(&_Tab.front(), (uint)_Tab.size(), CVertexBuffer::TBGRA);
	_ColorType = colorType;
}

void convertVBColor(NLMISC::CRGBA *array,uint numColor,CVertexBuffer::TVertexColorType format)
{
	if (!array || format == CVertexBuffer::TRGBA) return;
	for(uint k = 0; k < numColor; ++k)
	{
		std::swap(array[k].B, array[k].R);
	}
}



///======================================================================================
void CPSValueGradientFuncRGBA::getValues(NLMISC::CRGBA *tab) const
{
	if (!tab) return;
	CPSValueGradientFunc<NLMISC::CRGBA>::getValues(tab);
	convertVBColor(tab, getNumValues(), _ColorType);
}

///======================================================================================
NLMISC::CRGBA CPSValueGradientFuncRGBA::getValue(uint index)	const
{
	return convertVBColor(CPSValueGradientFunc<NLMISC::CRGBA>::getValue(index), _ColorType);
}

///======================================================================================
void CPSValueGradientFuncRGBA::setValues(const NLMISC::CRGBA *valueTab, uint32 numValues, uint32 nbStages)
{
	std::vector<NLMISC::CRGBA> convert(valueTab, valueTab + numValues);
	convertVBColor(&convert[0], numValues, _ColorType);
	CPSValueGradientFunc<NLMISC::CRGBA>::setValues(&convert[0], numValues, nbStages);
}

///======================================================================================
void CPSValueGradientFuncRGBA::setValuesUnpacked(const NLMISC::CRGBA *valueTab, uint32 numValues, uint32 nbStages)
{
	std::vector<NLMISC::CRGBA> convert(valueTab, valueTab + (numValues * nbStages + 1)) ;
	convertVBColor(&convert[0], (numValues * nbStages + 1), _ColorType);
	CPSValueGradientFunc<NLMISC::CRGBA>::setValuesUnpacked(&convert[0], numValues, nbStages);
}

///======================================================================================
void PSRegisterColorAttribs()
{
	NLMISC_REGISTER_CLASS(CPSColorBlender);
	NLMISC_REGISTER_CLASS(CPSColorMemory);
	NLMISC_REGISTER_CLASS(CPSColorBinOp);
	NLMISC_REGISTER_CLASS(CPSColorBlenderExact);
	NLMISC_REGISTER_CLASS(CPSColorGradient);
}

} // NL3D
