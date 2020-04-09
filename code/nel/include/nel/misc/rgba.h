// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_RGBA_H
#define NL_RGBA_H


#include "types_nl.h"
#include "common.h"

#include <algorithm>

namespace NLMISC
{

class	IStream;

/**
 * Class pixel RGBA
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CRGBA
{
public:

	/// Default constructor. do nothing
	CRGBA() {}

	/**
	 * Constructor.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	CRGBA(uint8 r, uint8 g, uint8 b, uint8 a=255) :
		R(r), G(g), B(b), A(a) {}

	/**
	 * setup as a packed pixel
	 */
	void	setPacked(uint packed)
	{
		R= (packed>>24)&255;
		G= (packed>>16)&255;
		B= (packed>>8)&255;
		A= packed & 255;
	}

	/**
	 * Return a packed pixel
	 */
	uint	getPacked() const {return ((uint)R<<24) + ((uint)G<<16) + ((uint)B<<8) + A;}

	/**
	 * Comparison operator.
	 */
	bool	operator<(CRGBA c) const {return getPacked()<c.getPacked();}
	/**
	 * Comparison operator.
	 */
	bool	operator!=(CRGBA c) const {return !(*this==c);}

	/**
	 * Equality operator.
	 */
	bool	operator==(CRGBA c) const
		{return R==c.R && G==c.G && B==c.B && A==c.A;}

	/**
	 * Serialisation.
	 * \param f Stream used for serialisation.
	 */
	void    serial (NLMISC::IStream &f);

	/**
	 * Blend two colors.
	 * \param c0 Color 0.
	 * \param c1 Color 1.
	 * \param coef Blend factor. 0~256. 0 return c0 and 256 return c1.
	 */
	void blendFromui(CRGBA c0, CRGBA c1, uint coef) // coef must be in [0,256]
	{
		uint	a1 = coef;
		uint	a2 = 256-a1;
		R = uint8((c0.R*a2 + c1.R*a1) >>8);
		G = uint8((c0.G*a2 + c1.G*a1) >>8);
		B = uint8((c0.B*a2 + c1.B*a1) >>8);
		A = uint8((c0.A*a2 + c1.A*a1) >>8);
	}

	/**
	 * Modulate colors with a constant.
	 * \param c0 Color 0.
	 * \param a E [0,256]. c0*a returned into this.
	 */
	void	modulateFromui (CRGBA c0, uint a)
	{
		R = uint8((c0.R*a) >>8);
		G = uint8((c0.G*a) >>8);
		B = uint8((c0.B*a) >>8);
		A = uint8((c0.A*a) >>8);
	}


	/**
	 * Modulate colors with another color.
	 * \param c0 Color 0.
	 * \param c1 Color 1. c0*c1 returned into this.
	 */
	void	modulateFromColor (CRGBA c0, CRGBA c1)
	{
		R = (c0.R*c1.R) >>8;
		G = (c0.G*c1.G) >>8;
		B = (c0.B*c1.B) >>8;
		A = (c0.A*c1.A) >>8;
	}


	/**
	 * Set colors.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	void	set (uint8 r, uint8 g, uint8 b, uint8 a=255);

	/**
	 * Convert to gray value
	 */
	uint8 toGray() const;

	/**
	 * Color is gray
	 */
	bool isGray() const
	{
		return R == G && G == B;
	}

	/**
	 * Get a 16 bits 565 pixel.
	 */
	uint16 get565 () const
	{
		return ((uint16)(R&0xf8)<<8) | ((uint16)(G&0xfc)<<3) | (uint16)(B>>3);
	}

	/**
	 * Set the RGB fields with a 16 bits 565 pixel.
	 */
	void	set565(uint16 col)
	{
		// unpack.
		R= col>>11;
		G= (col>>5)&0x3F;
		B= (col)&0x1F;
		// to 8 bits.
		R= (R<<3) + (R>>2);
		G= (G<<2) + (G>>4);
		B= (B<<3) + (B>>2);
	}

	/**
	 * Set the RGBA fields with a 32 bits 8888 pixel.
	 */
	void	set8888(uint32 col)
	{
		R = col & 255;
		G = (col >> 8) & 255;
		B = (col >> 16) & 255;
		A = (col >> 24) & 255;
	}


	/**
	 * Compute in this the average of 2 RGBA.
	 */
	void	avg2(CRGBA a, CRGBA b)
	{
		R= ((uint)a.R+(uint)b.R)>>1;
		G= ((uint)a.G+(uint)b.G)>>1;
		B= ((uint)a.B+(uint)b.B)>>1;
		A= ((uint)a.A+(uint)b.A)>>1;
	}

	/**
	 * Compute in this the average of 4 RGBA.
	 * The average is "correct": +1 is added to the four color, to make a "round" like average.
	 */
	void	avg4(CRGBA a, CRGBA b, CRGBA c, CRGBA d)
	{
		R= ((uint)a.R+(uint)b.R+(uint)c.R+(uint)d.R+ 1)>>2;
		G= ((uint)a.G+(uint)b.G+(uint)c.G+(uint)d.G+ 1)>>2;
		B= ((uint)a.B+(uint)b.B+(uint)c.B+(uint)d.B+ 1)>>2;
		A= ((uint)a.A+(uint)b.A+(uint)c.A+(uint)d.A+ 1)>>2;
	}

	/**
	 *	Do the sum of 2 rgba, clamp, and store in this
	 */
	void	add(CRGBA c0, CRGBA c1)
	{
		uint	r,g,b,a;
		r= c0.R + c1.R;	r= std::min(r, 255U);	R= (uint8)r;
		g= c0.G + c1.G;	g= std::min(g, 255U);	G= (uint8)g;
		b= c0.B + c1.B;	b= std::min(b, 255U);	B= (uint8)b;
		a= c0.A + c1.A;	a= std::min(a, 255U);	A= (uint8)a;
	}

	/**
	 *	Compute c0 - c1, and store in this
	 */
	void	sub(CRGBA c0, CRGBA c1)
	{
		sint	r,g,b,a;
		r= c0.R - c1.R;	r= std::max(r, 0);	R= (uint8)r;
		g= c0.G - c1.G;	g= std::max(g, 0);	G= (uint8)g;
		b= c0.B - c1.B;	b= std::max(b, 0);	B= (uint8)b;
		a= c0.A - c1.A;	a= std::max(a, 0);	A= (uint8)a;
	}


	/// \name RGBOnly methods. Same f() as their homonym, but don't modify A component.
	// @{

	/// see blendFromui()
	void	blendFromuiRGBOnly(CRGBA c0, CRGBA c1, uint coef) // coef must be in [0,256]
	{
		uint	a1 = coef;
		uint	a2 = 256-a1;
		R = uint8((c0.R*a2 + c1.R*a1) >>8);
		G = uint8((c0.G*a2 + c1.G*a1) >>8);
		B = uint8((c0.B*a2 + c1.B*a1) >>8);
	}
	/// see modulateFromui()
	void	modulateFromuiRGBOnly(CRGBA c0, uint a)
	{
		R = uint8((c0.R*a) >>8);
		G = uint8((c0.G*a) >>8);
		B = uint8((c0.B*a) >>8);
	}
	/// see modulateFromColor()
	void	modulateFromColorRGBOnly(CRGBA c0, CRGBA c1)
	{
		R = (c0.R*c1.R) >>8;
		G = (c0.G*c1.G) >>8;
		B = (c0.B*c1.B) >>8;
	}
	/// see avg2()
	void	avg2RGBOnly(CRGBA a, CRGBA b)
	{
		R= ((uint)a.R+(uint)b.R)>>1;
		G= ((uint)a.G+(uint)b.G)>>1;
		B= ((uint)a.B+(uint)b.B)>>1;
	}
	/// see avg4()
	void	avg4RGBOnly(CRGBA a, CRGBA b, CRGBA c, CRGBA d)
	{
		R= ((uint)a.R+(uint)b.R+(uint)c.R+(uint)d.R+ 1)>>2;
		G= ((uint)a.G+(uint)b.G+(uint)c.G+(uint)d.G+ 1)>>2;
		B= ((uint)a.B+(uint)b.B+(uint)c.B+(uint)d.B+ 1)>>2;
	}
	/// see add()
	void	addRGBOnly(CRGBA c0, CRGBA c1)
	{
		uint	r,g,b;
		r= c0.R + c1.R;	r= std::min(r, 255U);	R= (uint8)r;
		g= c0.G + c1.G;	g= std::min(g, 255U);	G= (uint8)g;
		b= c0.B + c1.B;	b= std::min(b, 255U);	B= (uint8)b;
	}
	/// see sub()
	void	subRGBOnly(CRGBA c0, CRGBA c1)
	{
		sint	r,g,b;
		r= c0.R - c1.R;	r= std::max(r, 0);	R= (uint8)r;
		g= c0.G - c1.G;	g= std::max(g, 0);	G= (uint8)g;
		b= c0.B - c1.B;	b= std::max(b, 0);	B= (uint8)b;
	}



	// @}

	///\name Color group manipulation
	//@{
		/** Add a group of colors with saturation, using mmx instructions when present.
		  * \param dest The destination color buffer, encoded as CRGBA's.
		  * \param src1 The first source color buffer, encoded as CRGBA's.
		  * \param src2 The second source color buffer, encoded as CRGBA's.
		  * \param numColors The number of colors to compute
		  * \param Stride between each source color.
		  * \param Stride between each destination color.
		  * \param Dup the number of time the result must be duplicated in the destination.
		  */
		static void addColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride = sizeof(CRGBA), uint destStride = sizeof(CRGBA), uint dup = 1);

		/** Modulate a group of colors with saturation, using mmx instructions when present.
		  * \param dest The destination color buffer, encoded as CRGBA's.
		  * \param src1 The first source color buffer, encoded as CRGBA's.
		  * \param src2 The second source color buffer, encoded as CRGBA's.
		  * \param numColors The number of colors to compute
		  * \param Stride between each color.  It is the same for sources and destination.
		  */
		static void modulateColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride = sizeof(CRGBA), uint destStride = sizeof(CRGBA), uint dup = 1);

		/** Subtract a group of colors with saturation (src1 - src2), using mmx instructions when present.
		  * \param dest The destination color buffer, encoded as CRGBA's.
		  * \param src1 The first source color buffer, encoded as CRGBA's.
		  * \param src2 The second source color buffer, encoded as CRGBA's.
		  * \param numColors The number of colors to compute
		  * \param Stride between each color.  It is the same for sources and destination.
		  */
		static void subtractColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride = sizeof(CRGBA), uint destStride = sizeof(CRGBA), uint dup = 1);
	//@}

	/// \name Color space conversions RGB only
	//@{
			/** Convert to HLS color space.
			  * Lightness and satuation ranges from 0 to 1
			  * There's no range for hue, (all hues colors range from 0 to 360, from 360 to 720 and so on)
			  * \return true if the color is achromatic
			  */
			bool convertToHLS(float &h, float &l, float &S) const;

			/** Build from HLS valued
			  *	Each component ranges from 0 to 1.f
			  */
			void buildFromHLS(float h, float l, float s);
	//@}

	static CRGBA stringToRGBA( const char *ptr );

	std::string toString() const;
	bool fromString( const std::string &s );


	/// Swap the B and R components, to simulate a CBRGA
	void	swapBR()
	{
		std::swap(R,B);
	}


	/// Red componant.
	uint8	R;
	/// Green componant.
	uint8	G;
	/// Blue componant.
	uint8	B;
	/// Alpha componant.
	uint8	A;


	/// some colors
	static const CRGBA Black ;
	static const CRGBA Red ;
	static const CRGBA Green ;
	static const CRGBA Yellow ;
	static const CRGBA Blue ;
	static const CRGBA Magenta ;
	static const CRGBA Cyan ;
	static const CRGBA White ;
	static const CRGBA Transparent ;
};


/**
 * Class pixel BGRA, Windows style pixel.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CBGRA
{
public:

	/// Default constructor. do nothing
	CBGRA() {}

	/**
	 * Constructor from a CRGBA
	 * \param c CRGBA color.
	 */
	CBGRA(CRGBA c)
	{
		R=c.R;
		G=c.G;
		B=c.B;
		A=c.A;
	};

	/**
	 * Constructor.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	CBGRA(uint8 r, uint8 g, uint8 b, uint8 a=255) :
		B(b), G(g), R(r), A(a) {}

	/**
	 * Cast operator to CRGBA.
	 */
	operator CRGBA()
	{
		return CRGBA (R, G, B, A);
	}

	/**
	 * Return a packed pixel
	 */
	uint	getPacked() const
	{
		return ((uint)B<<24) + ((uint)G<<16) + ((uint)R<<8) + A;
	}

	/**
	 * Comparison operator.
	 */
	bool	operator<(const CBGRA &c) const
	{
		return getPacked()<c.getPacked();
	}

	/**
	 * Equality operator.
	 */
	bool	operator==(const CBGRA &c) const
	{
		return R==c.R && G==c.G && B==c.B && A==c.A;
	}

	/**
	 * Serialisation.
	 * \param f Stream used for serialisation.
	 */
	void    serial(NLMISC::IStream &f);

	/**
	 * Blend two colors.
	 * \param c0 Color 0.
	 * \param c1 Color 1.
	 * \param factor Blend factor. 0~256. 0 return c0 and 256 return c1.
	 */
	void blendFromui(CBGRA &c0, CBGRA &c1, uint factor);

	/**
	 * Set colors.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	void set(uint8 r, uint8 g, uint8 b, uint8 a);

	/// Blue componant.
	uint8	B;
	/// Green componant.
	uint8	G;
	/// Red componant.
	uint8	R;
	/// Alpha componant.
	uint8	A;
};

// specialisation of the 'blend' function found in algo.h
template <class U>
inline CRGBA blend(CRGBA c0, CRGBA c1, U blendFactor)
{
	CRGBA result;
	result.blendFromui(c0, c1, (uint) ((float) blendFactor * 256.f));
	return result;
}

/**
 * Class pixel float RGBA
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CRGBAF
{
public:
	/// Default constructor. do nothing
	CRGBAF ()
	{}

	/**
	 * Constructor.
	 * \param _r Red componant.
	 * \param _g Green componant.
	 * \param _b Blue componant.
	 * \param _a Alpha componant.
	 */
	CRGBAF (float _r, float _g, float _b, float _a=1.f)
	{
		R=_r;
		G=_g;
		B=_b;
		A=_a;
	}

	/**
	 * Constructor with a CRGBA.
	 * \param c CRGBA color.
	 */
	CRGBAF (CRGBA c)
	{
		R=(float)c.R/255.f;
		G=(float)c.G/255.f;
		B=(float)c.B/255.f;
		A=(float)c.A/255.f;
	}

	/**
	 * Cast operator to CRGBA.
	 */
	operator CRGBA() const
	{
		uint8 _r=(uint8)(R*255.f);
		uint8 _g=(uint8)(G*255.f);
		uint8 _b=(uint8)(B*255.f);
		uint8 _a=(uint8)(A*255.f);
		return CRGBA (_r, _g, _b, _a);
	}

	/**
	 * Normalize component between [0.f,1.f]
	 */
	void normalize ()
	{
		R= (R>1.f) ? 1.f : (R<0.f) ? 0.f : R;
		G= (G>1.f) ? 1.f : (G<0.f) ? 0.f : G;
		B= (B>1.f) ? 1.f : (B<0.f) ? 0.f : B;
		A= (A>1.f) ? 1.f : (A<0.f) ? 0.f : A;
	}

	/**
	 * Add operator. Sum components.
	 * \param c CRGBA color.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator+ (const CRGBAF& c) const
	{
		return CRGBAF (R+c.R, G+c.G, B+c.B, A+c.A);
	}

	/**
	 * Sub operator. Substract components.
	 * \param c CRGBA color.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator- (const CRGBAF& c) const
	{
		return CRGBAF (R-c.R, G-c.G, B-c.B, A-c.A);
	}

	/**
	 * Mul operator. Mul components.
	 * \param c CRGBA color.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator* (const CRGBAF& c) const
	{
		return CRGBAF (R*c.R, G*c.G, B*c.B, A*c.A);
	}

	/**
	 * Mul float operator. Mul each component by f.
	 * \param f Float factor.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator* (float f) const
	{
		return CRGBAF (R*f, G*f, B*f, A*f);
	}

	/**
	 * Div float operator. Div each component by f.
	 * \param f Float factor.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator/ (float f) const
	{
		return CRGBAF (R/f, G/f, B/f, A/f);
	}

	/**
	 * Add operator. Add each component.
	 * \param c CRGBA color.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator+= (const CRGBAF& c)
	{
		R+=c.R;
		G+=c.G;
		B+=c.B;
		A+=c.A;
		return *this;
	}

	/**
	 * Sub operator. Substract each component.
	 * \param c CRGBA color.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator-= (const CRGBAF& c)
	{
		R-=c.R;
		G-=c.G;
		B-=c.B;
		A-=c.A;
		return *this;
	}

	/**
	 * Mul operator. Multiplate each component.
	 * \param c CRGBA color.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator*= (const CRGBAF& c)
	{
		R*=c.R;
		G*=c.G;
		B*=c.B;
		A*=c.A;
		return *this;
	}

	/**
	 * Mul float operator. Multiplate each component by f.
	 * \param f Float factor.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator*= (float f)
	{
		R*=f;
		G*=f;
		B*=f;
		A*=f;
		return *this;
	}

	/**
	 * Div float operator. Divide each component by f.
	 * \param f Float factor.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator/= (float f)
	{
		R/=f;
		G/=f;
		B/=f;
		A/=f;
		return *this;
	}

	/**
	 * Serialisation.
	 * \param f Stream used for serialisation.
	 */
	void    serial(NLMISC::IStream &f);

	/**
	 * Set colors.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	void set(float r, float g, float b, float a);

	/// Red componant.
	float	R;
	/// Green componant.
	float	G;
	/// Blue componant.
	float	B;
	/// Alpha componant.
	float	A;
};

/**
 * Mul float operator. Multiplate each component by f.
 * \param f Float factor.
 * \return Return the result
 */
inline CRGBAF operator* (float f, const CRGBAF& c)
{
	return CRGBAF (c.R*f, c.G*f, c.B*f, c.A*f);
}

#ifdef NL_LITTLE_ENDIAN
#define NL_RGBA_R_DWORD_MASK (0x000000ff)
#define NL_RGBA_G_DWORD_MASK (0x0000ff00)
#define NL_RGBA_B_DWORD_MASK (0x00ff0000)
#define NL_RGBA_A_DWORD_MASK (0xff000000)
#else // NL_LITTLE_ENDIAN
#define NL_RGBA_R_DWORD_MASK (0xff0000)
#define NL_RGBA_G_DWORD_MASK (0x00ff0000)
#define NL_RGBA_B_DWORD_MASK (0x0000ff00)
#define NL_RGBA_A_DWORD_MASK (0x000000ff)
#endif // NL_LITTLE_ENDIAN

} // NLMISC


#endif // NL_RGBA_H

/* End of rgba.h */
