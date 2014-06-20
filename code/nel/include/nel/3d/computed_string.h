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

#ifndef NL_COMPUTED_STRING_H
#define NL_COMPUTED_STRING_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/u_text_context.h"
#include <vector>
#include <limits>

namespace NLMISC {

class CMatrix;

}


namespace NL3D {

class CTextureFont;
struct CComputedString;

// ***************************************************************************
/**
 *	A Buffer to render batch of computed string.
 */
class	CRenderStringBuffer : public URenderStringBuffer
{
public:
	CVertexBuffer	Vertices;
	uint			NumQuads;

public:
	CRenderStringBuffer();
	virtual ~CRenderStringBuffer();

	/// render and make empty the render string buffer. see CComputedString::render2DClip()
	void	flush(IDriver& driver, CMaterial *fontMat);

	/** render and make empty the render string buffer. see CComputedString::render2DProjected()
	 *  The driver view and model matrices have to be setuped as material zbuffer flags by the user.
	 *	This method only render string quads.
	 */
	void	flushUnProjected(IDriver& driver, CMaterial *fontMat, bool zwrite);
};



class CLetterColors : public ULetterColors
{

public:

	struct SLetterColor
	{
		uint Index;
		NLMISC::CRGBA Color;

		SLetterColor(uint index, const NLMISC::CRGBA & color)
		{
			Index = index;
			Color = color;
		}

		bool operator == ( const SLetterColor lc ) const
		{
			return (Index==lc.Index && Color==lc.Color);
		}
	};

	CLetterColors() {}
	virtual ~CLetterColors() {}

	void clear()
	{
		_indexedColors.clear();
	}

	bool empty() const
	{
		return _indexedColors.empty();
	}

	uint size() const
	{
		return (uint)_indexedColors.size();
	}

	uint getIndex(uint i) const
	{
		if(i<_indexedColors.size())
			return _indexedColors[i].Index;

        return std::numeric_limits<uint>::max();
	}

	const CRGBA & getColor(uint i) const
	{
		if(i<_indexedColors.size())
			return _indexedColors[i].Color;

		return CRGBA::Black;
	}

	const SLetterColor & getLetterColor(uint i) const
	{
		if(i<_indexedColors.size())
			return _indexedColors[i];

		static SLetterColor defaultLetterColor(0, CRGBA::Black);
		return defaultLetterColor;
	}

	bool isSameLetterColors(ULetterColors * letterColors)
	{
		CLetterColors * letterCol = static_cast<CLetterColors*>(letterColors);
		bool	sameLetterColors = false;
		if(_indexedColors.size()==letterCol->size())
		{
			sameLetterColors= true;
			for(uint i=0;i<_indexedColors.size();i++)
			{
				if(!(_indexedColors[i] == letterCol->getLetterColor(i)))
				{
					sameLetterColors = false;
					break;
				}
			}
		}

		return sameLetterColors;
	}

	void pushLetterColor(uint index, const NLMISC::CRGBA & color)
	{
		_indexedColors.push_back(SLetterColor(index, color));
	}

private:
	std::vector< SLetterColor > _indexedColors;
};


// ***************************************************************************
/**
 * CComputedString
 * A CComputedString is a structure which permits to render a string
 * in a driver. It computes 4 vertices per char the renderer draw quads from them.
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
struct CComputedString
{

public:

	CVertexBuffer Vertices;
	CMaterial	*Material;
	CRGBA Color;
	/// The width of the string, in pixels (eg: 30)
	float StringWidth;
	/// The height of the string, in pixels (eg: 10)
	float StringHeight;
	/// The BBox of all vertices. used for render2DClip()
	float XMin, ZMin, XMax, ZMax;

	/** StringLine is the size from bottom of the whole string image to the hotspot in pixels.
	 *	for instance if the hotspot is bottomLeft the imaginary line of the string "bpc"
	 *	is under the b, under the loop of the p but over the leg of the p. So StringLine
	 *	is a positive value in this case. It may be a negative value for the string "^" for example.
	 */
	float StringLine;

	/// Optional: each render*() method can draw a subset of letters. Default is 0/FFFFFFFF
	uint32	SelectStart;
	uint32	SelectSize;

	CLetterColors LetterColors;

	/**
	 * Hotspot positions (origin for the string placement)
	 * You should take care that for vertical hotspot, an imaginary line is defined under
	 * letters with no leg (like m,b,c etc..) between the leg of p and the loop of the p.
	 */
	enum THotSpot
	{
		BottomLeft=0,
		MiddleLeft,
		TopLeft,
		MiddleBottom,
		MiddleMiddle,
		MiddleTop,
		BottomRight,
		MiddleRight,
		TopRight,

		HotSpotCount
	};

	/**
	 * Default constructor
	 */
	CComputedString (bool bSetupVB=true)
	{
		StringWidth = 0;
		StringHeight = 0;
		if (bSetupVB)
		{
			Vertices.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
			Vertices.setPreferredMemory (CVertexBuffer::RAMVolatile, true);
			Vertices.setName("CComputedString");
		}
		SelectStart= 0;
		SelectSize= std::numeric_limits<uint32>::max();
	}

	/**
	 *	Get the string's origin
	 * \param hotspot the origin of the string
	 */
	CVector getHotSpotVector (THotSpot hotspot);

	/**
	 * Render the unicode string in a driver.
	 * \param driver the driver where to render the primitives
	 * \param x abscissa
	 * \param y ordinate
	 * \param hotspot position of string origine
	 * \param scaleX abscissa scale
	 * \param scaleY ordinate scale
	 * \param rotateY rotation angle (axe perpendicular to screen)
	 * \param useScreenAR43 if false then string is displayed with a pixel Ratio 1:1 (independent of window resolution).
	 *	if true, the string is scaled according to window width and height, to support 4:3 aspect ratio even on weird
	 *	screen resolution such as 640*240 (ie the char still look square, but the pixel ratio is 2:1)
	 * \param roundToNearestPixel if true, snap the final string position to the nearest pixel. if set to true, and if
	 *	useScreenAR43= false, you are sure that texels of the fonts fit exactly on centers of pixels (no apparent bi-linear).
	 */
	void render2D (IDriver& driver,
					float x, float z,
					THotSpot hotspot = BottomLeft,
					float scaleX = 1, float scaleZ = 1,
					float rotateY = 0,
					bool  useScreenAR43= false,
					bool  roundToNearestPixel= true
					);

	/** Special for interface. same as render2D but clip the quads to xmin,ymin/xmax,ymax.
	 *	NB: behavior is same as render2D with: Hotspot = bottomLeft, scaleX=1, scaleZ=1, rotateY=0,
	 *	useScreenAR43= false, roundToNearestPixel= false
	 *	Additionnaly, this method doesn't render directly to the driver but add primitives to a CRenderStringBuffer
	 *	Use the method CRenderStringBuffer::flush() to flush it all.
	 */
	void render2DClip (IDriver& driver, CRenderStringBuffer &rdrBuffer,
					float x, float z,
					float xmin=0, float ymin=0, float xmax=1, float ymax=1
					);

	/** Special for interface. same as render2DClip but unproject the vertices using a frustum and a scale matrix
	 *	Use the method CRenderStringBuffer::flush() to flush it all.
	 */
	void render2DUnProjected (IDriver& driver, CRenderStringBuffer &rdrBuffer, class NL3D::CFrustum &frustum,
					const NLMISC::CMatrix &scaleMatrix,
					float x, float z, float depth, float xmin=0, float ymin=0, float xmax=1, float ymax=1);

	/**
	 * Render the unicode string in a driver, in 3D with a user matrix.
	 *	NB: size of the string is first scaled by 1/windowHeight.
	 * \param driver the driver where to render the primitives
	 * \param matrix transformation matrix
	 * \param hotspot position of string origine
	 */
	void render3D (IDriver& driver, const CMatrix &matrix, THotSpot hotspot = MiddleMiddle);

};



} // NL3D


#endif // NL_COMPUTED_STRING_H

/* End of computed_string.h */

