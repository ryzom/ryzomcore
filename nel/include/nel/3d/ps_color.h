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

#ifndef NL_PS_COLOR_H
#define NL_PS_COLOR_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_attrib_maker_template.h"
#include "nel/3d/ps_attrib_maker_bin_op.h"
#include "nel/3d/ps_attrib_maker_helper.h"
#include "nel/3d/driver.h"
#include "nel/misc/rgba.h"
#include "nel/3d/animation_time.h"






namespace NL3D {


template <>
inline const char *CPSAttribMaker<NLMISC::CRGBA>::getType() { return "CRGBA"; }


// Depending on the driver, the format of colors in vertex buffer may change. We don't want to change the format for each data that is (dynamically) in vertex buffer, so
// we modify gradients instead
inline NLMISC::CRGBA convertVBColor(NLMISC::CRGBA color, CVertexBuffer::TVertexColorType format)
{
	return format == CVertexBuffer::TBGRA ? NLMISC::CRGBA(color.B, color.G, color.R, color.A) : color;
}

void convertVBColor(NLMISC::CRGBA *array, uint numColor, CVertexBuffer::TVertexColorType format);



// helper to change color type of color blender depending on vb type
class CPSValueBlendFuncRGBA : public CPSValueBlendFunc<NLMISC::CRGBA>
{
public:
	// ctor
	CPSValueBlendFuncRGBA() : _ColorType(CVertexBuffer::TRGBA) {}
	//
	void getValues(NLMISC::CRGBA &startValue, NLMISC::CRGBA &endValue) const
	{
		CPSValueBlendFunc<NLMISC::CRGBA>::getValues(startValue, endValue);
		startValue = convertVBColor(startValue, _ColorType);
		endValue = convertVBColor(endValue, _ColorType);

	}
	virtual void setValues(NLMISC::CRGBA startValue, NLMISC::CRGBA endValue)
	{
		CPSValueBlendFunc<NLMISC::CRGBA>::setValues(convertVBColor(startValue, _ColorType), convertVBColor(endValue, _ColorType));
	}
	void serial(NLMISC::IStream &f)
	{
		setColorType(CVertexBuffer::TRGBA);
		CPSValueBlendFunc<NLMISC::CRGBA>::serial(f);
	}
	// change the color type
	void setColorType(CVertexBuffer::TVertexColorType colorType);

	virtual NLMISC::CRGBA getMaxValue(void) const { return CRGBA::Black; }
	virtual NLMISC::CRGBA getMinValue(void) const { return CRGBA::Black; }
protected:
	CVertexBuffer::TVertexColorType _ColorType;
};

const uint RGBA_BLENDER_NUM_VALUES = 64;

// helper to change color type of color blender sampled depending on vb type
class CPSValueBlendSampleFuncRGBA : public CPSValueBlendSampleFunc<NLMISC::CRGBA, RGBA_BLENDER_NUM_VALUES>
{
public:
	// ctor
	CPSValueBlendSampleFuncRGBA() : _ColorType(CVertexBuffer::TRGBA) {}
	//
	void getValues(NLMISC::CRGBA &startValue, NLMISC::CRGBA &endValue) const
	{
		CPSValueBlendSampleFunc<NLMISC::CRGBA, RGBA_BLENDER_NUM_VALUES>::getValues(startValue, endValue);
		startValue = convertVBColor(startValue, _ColorType);
		endValue = convertVBColor(endValue, _ColorType);

	}
	virtual void setValues(NLMISC::CRGBA startValue, NLMISC::CRGBA endValue)
	{
		CPSValueBlendSampleFunc<NLMISC::CRGBA, RGBA_BLENDER_NUM_VALUES>::setValues(convertVBColor(startValue, _ColorType), convertVBColor(endValue, _ColorType));
	}
	void serial(NLMISC::IStream &f)
	{
		setColorType(CVertexBuffer::TRGBA);
		CPSValueBlendSampleFunc<NLMISC::CRGBA, RGBA_BLENDER_NUM_VALUES>::serial(f);
	}
	// change the color type
	void setColorType(CVertexBuffer::TVertexColorType colorType);

	virtual NLMISC::CRGBA getMaxValue(void) const { return CRGBA::Black; }
	virtual NLMISC::CRGBA getMinValue(void) const { return CRGBA::Black; }
protected:
	CVertexBuffer::TVertexColorType _ColorType;
};
// helper to change color type of color gradient depending on vb type
class CPSValueGradientFuncRGBA : public CPSValueGradientFunc<NLMISC::CRGBA>
{
public:
	// ctor
	CPSValueGradientFuncRGBA() : _ColorType(CVertexBuffer::TRGBA) {}
	//
	void getValues(NLMISC::CRGBA *tab) const;
	NLMISC::CRGBA getValue(uint index)	const;
	void setValues(const NLMISC::CRGBA *valueTab, uint32 numValues, uint32 nbStages);
	void setValuesUnpacked(const NLMISC::CRGBA *valueTab, uint32 numValues, uint32 nbStages);
	void serial(NLMISC::IStream &f)
	{
		setColorType(CVertexBuffer::TRGBA);
		CPSValueGradientFunc<NLMISC::CRGBA>::serial(f);
	}
	// change the color type
	void setColorType(CVertexBuffer::TVertexColorType colorType);

	virtual NLMISC::CRGBA getMaxValue(void) const { return CRGBA::Black; }
	virtual NLMISC::CRGBA getMinValue(void) const { return CRGBA::Black; }
protected:
	CVertexBuffer::TVertexColorType _ColorType;
};


// helper class that helps to redefine the 'setColorType' method
// T : the functor that compute colors
template <class F>
class CPSAttribMakerRGBA : public CPSAttribMakerT<NLMISC::CRGBA, F>
{
public:
	// ctor
	CPSAttribMakerRGBA(float nbCycles) : CPSAttribMakerT<NLMISC::CRGBA, F>(nbCycles) {}
	// helps to change internal color representation
	virtual void setColorType(CVertexBuffer::TVertexColorType colorType)
	{
		this->_F.setColorType(colorType);
	}
	// serialisation should always be done in RGBA mode, so enforce that
	virtual void serial(NLMISC::IStream &f)
	{
		setColorType(CVertexBuffer::TRGBA);
		CPSAttribMakerT<NLMISC::CRGBA, F>::serial(f);
	}
};


/// This is a int blender class. It just blend between 2 values. The blending is exact, and thus slow...
class CPSColorBlenderExact : public CPSAttribMakerRGBA<CPSValueBlendFuncRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlenderExact);
	CPSColorBlenderExact(NLMISC::CRGBA startColor = NLMISC::CRGBA::White , NLMISC::CRGBA endColor = NLMISC::CRGBA::Black, float nbCycles = 1.0f) : CPSAttribMakerRGBA<CPSValueBlendFuncRGBA>(nbCycles)
	{
		_F.setValues(startColor, endColor);
	}
	CPSAttribMakerBase *clone() const { return new CPSColorBlenderExact(*this); }

};



// an int blender class that perform 64 color sample between colors, it is faster than CPSColorBlenderExact
class CPSColorBlender : public CPSAttribMakerRGBA<CPSValueBlendSampleFuncRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlender);
	CPSColorBlender(NLMISC::CRGBA startColor = NLMISC::CRGBA::White , NLMISC::CRGBA endColor = NLMISC::CRGBA::Black, float nbCycles = 1.0f) : CPSAttribMakerRGBA<CPSValueBlendSampleFuncRGBA>(nbCycles)
	{
		_F.setValues(startColor, endColor);
	}
	CPSAttribMakerBase *clone() const { return new CPSColorBlender(*this); }
};



/** This is a color gradient class
  * NB: a non null gradient must be set before use
  */
class CPSColorGradient : public CPSAttribMakerRGBA<CPSValueGradientFuncRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorGradient);

	/** default ctor
	  * NB: a non null gradient must be set before use
	  */
	CPSColorGradient();

	/**
	 *	Construct the value gradient blender by passing a pointer to a color table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSColorGradient(const NLMISC::CRGBA *colorTab, uint32 nbValues, uint32 nbStages, float nbCycles = 1.0f);
	static NLMISC::CRGBA _DefaultGradient[];
	CPSAttribMakerBase *clone() const { return new CPSColorGradient(*this); }
};



/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSColorMemory : public CPSAttribMakerMemory<NLMISC::CRGBA>
{
public:
	CPSColorMemory() : _ColorType(CVertexBuffer::TRGBA) { setDefaultValue(NLMISC::CRGBA::White); }
	NLMISC_DECLARE_CLASS(CPSColorMemory);
	CPSAttribMakerBase *clone() const { return new CPSColorMemory(*this); }
	virtual void setColorType(CVertexBuffer::TVertexColorType colorType);
	virtual void setDefaultValue(NLMISC::CRGBA defaultValue);
	virtual NLMISC::CRGBA getDefaultValue(void) const;
	virtual void serial(NLMISC::IStream &f);
protected:
	CVertexBuffer::TVertexColorType _ColorType;
};


/** An attribute maker whose output if the result of a binary op on colors
  *
  */
class CPSColorBinOp : public CPSAttribMakerBinOp<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBinOp);
	CPSAttribMakerBase *clone() const { return new CPSColorBinOp(*this); }
	virtual void setColorType(CVertexBuffer::TVertexColorType colorType);
	virtual void serial(NLMISC::IStream &f);
};



#if 0

/**
 * Here, we got color maker
 * \see ps_attrib_maker.h, ps_attrib_maker_template.h
 */

/// these are some attribute makers for colors

/// This is a int blender class. It just blend between 2 values. The blending is exact, and thus slow...
class CPSColorBlenderExact : public CPSValueBlender<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlenderExact);
	CPSColorBlenderExact(NLMISC::CRGBA startColor = NLMISC::CRGBA::White , NLMISC::CRGBA endColor = NLMISC::CRGBA::Black, float nbCycles = 1.0f) : CPSValueBlender<NLMISC::CRGBA>(nbCycles)
	{
		_F.setValues(startColor, endColor);
	}
	CPSAttribMakerBase *clone() const { return new CPSColorBlenderExact(*this); }

};

// an int blender class that perform 64 color sample between colors, it is faster than CPSColorBlenderExact
class CPSColorBlender : public CPSValueBlenderSample<NLMISC::CRGBA, 64>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlender);
	CPSColorBlender(NLMISC::CRGBA startColor = NLMISC::CRGBA::White , NLMISC::CRGBA endColor = NLMISC::CRGBA::Black, float nbCycles = 1.0f) : CPSValueBlenderSample<NLMISC::CRGBA, 64>(nbCycles)
	{
		_F.setValues(startColor, endColor);
	}
	CPSAttribMakerBase *clone() const { return new CPSColorBlender(*this); }
};

/** This is a color gradient class
  * NB: a non null gradient must be set before use
  */
class CPSColorGradient : public CPSValueGradient<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorGradient);

	/** default ctor
	  * NB: a non null gradient must be set before use
	  */
//	CPSColorGradient();

	/**
	 *	Construct the value gradient blender by passing a pointer to a color table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */
	CPSColorGradient(const NLMISC::CRGBA *colorTab, uint32 nbValues, uint32 nbStages, float nbCycles = 1.0f);
	static NLMISC::CRGBA _DefaultGradient[];
	CPSAttribMakerBase *clone() const { return new CPSColorGradient(*this); }
};

/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSColorMemory : public CPSAttribMakerMemory<NLMISC::CRGBA>
{
public:
	CPSColorMemory() { setDefaultValue(NLMISC::CRGBA::White); }
	NLMISC_DECLARE_CLASS(CPSColorMemory);
	CPSAttribMakerBase *clone() const { return new CPSColorMemory(*this); }
};

/** An attribute maker whose output if the result of a binary op on colors
  *
  */
class CPSColorBinOp : public CPSAttribMakerBinOp<NLMISC::CRGBA>
{
	public:
	NLMISC_DECLARE_CLASS(CPSColorBinOp);
	CPSAttribMakerBase *clone() const { return new CPSColorBinOp(*this); }
};

#endif // #if 0


} // NL3D
#endif // NL_PS_COLOR_H

/* End of ps_color.h */
