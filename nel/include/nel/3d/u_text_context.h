// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_U_TEXT_CONTEXT_H
#define NL_U_TEXT_CONTEXT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/matrix.h"
#include "nel/misc/utf_string_view.h"

namespace NL3D {

class	UDriver;
class	UMaterial;
class	CFrustum;

// ***************************************************************************
/** Abstract Class to render string into a temporary buffer
 */
class	URenderStringBuffer
{
public:
	URenderStringBuffer() {}
	virtual ~URenderStringBuffer() {}
};


class ULetterColors
{
public:

	ULetterColors() {}
	virtual ~ULetterColors() {}

	virtual bool isSameLetterColors(ULetterColors * letterColors) = 0;
	virtual void pushLetterColor(uint index, const NLMISC::CRGBA & color) = 0;
};


// ***************************************************************************
/**
 * A game interface to render string.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UTextContext
{
protected:
	/// \name Object
	// @{
	UTextContext() {}
	virtual ~UTextContext() {}
	// @}


public:

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


	/// The render size of a string.
	struct	CStringInfo
	{
		/// The width of the string, in pixels (eg: 30)
		float StringWidth;
		/// The height of the string, in pixels (eg: 10)
		float StringHeight;
		/** StringLine is the size from bottom of the whole string image to the hotspot in pixels.
		 *	for instance if the hotspot is bottomLeft the imaginary line of the string "bpc"
		 *	is under the b, under the loop of the p but over the leg of the p. So StringLine
		 *	is a positive value in this case. It may be a negative value for the string "^" for example.
		 */
		float StringLine;

		/// The length in characters
		size_t StringLength;

		CStringInfo() { StringWidth = StringHeight = StringLine = 0; StringLength = 0; }
		CStringInfo(float w, float h, float l, size_t len) { StringWidth = w; StringHeight = h; StringLine = l; StringLength = len; }

		/**
		 *	Get the string's origin
		 * \param hotspot the origin of the string
		 */
		NLMISC::CVector getHotSpotVector(THotSpot hotspot);

		/// convert size in pixels to size in 0-1 relative coordinates, according to Driver current size
		void		convertTo01Size(UDriver *drv);
		/// convert back to pixels size, according to Driver current size
		void		convertToPixelSize(UDriver *drv);
	};


public:


	/// \name Text look.
	// @{
	/**
	 * set the font color
	 * \param color the font color
	 */
	virtual	void			setColor (NLMISC::CRGBA color) = 0;
	/**
	 * set the font size. Should be called before the first print
	 * \param fonSize the font size
	 */
	virtual	void			setFontSize (uint32 fontSize) = 0;
	/**
	 * get the font size
	 * \return the font size
	 */
	virtual	uint32			getFontSize () const = 0;
	/**
	 * set embolden (bold) state
	 * \param embolden the embbolden state
	 */
	virtual void			setEmbolden (bool b) = 0;
	/**
	 * \return the embolden state
	 */
	virtual bool			getEmbolden () const = 0;
	/**
	 * set oblique (italic) state
	 * \param oblique the oblique state
	 */
	virtual void			setOblique (bool b) = 0;
	/**
	 * \return the oblique state
	 */
	virtual bool			getOblique () const = 0;
	/**
	 * set the hot spot
	 * \param fonSize the font size
	 */
	virtual	void			setHotSpot (THotSpot hotSpot) = 0;
	/**
	 * get the hot spot
	 * \return the hot spot
	 */
	virtual	THotSpot		getHotSpot () const = 0;
	/**
	 * set the X scale
	 * \param scaleX the X scale
	 */
	virtual	void			setScaleX (float scaleX) = 0;
	/**
	 * set the Y scale
	 * \param scaleY the Y scale
	 */
	virtual	void			setScaleY (float scaleY) = 0;
	/**
	 * \return the X scale
	 */
	virtual	float			getScaleX () const = 0;
	/**
	 * \return the Y scale
	 */
	virtual	float			getScaleY () const = 0;
	/**
	 * set the shade states
	 * \param the shade state
	 */
	virtual	void			setShaded (bool b) = 0;
	/**
	 * \return the shade state
	 */
	virtual	bool			getShaded () const = 0;
	/**
	 * set the shade states
	 * \param the shade state
	 */
	virtual	void			setShadeOutline (bool b) = 0;
	/**
	 * \return the shade state
	 */
	virtual	bool			getShadeOutline () const = 0;
	/**
	 * set the shadow's size
	 * \param the shade extent
	 */
	virtual	void			setShadeExtent (float x, float y) = 0;
	/**
	 * set the shadow's color
	 * The alpha of the shade is multiplied at each draw with the alpha of the color. Default: (0,0,0,255)
	 * \param the shadow color
	 */
	virtual	void			setShadeColor (NLMISC::CRGBA sc) = 0;
	/**
	 * get the shadow's color
	 * \returb the shadow color
	 */
	virtual	NLMISC::CRGBA	getShadeColor () const = 0;
	/** set to true if you want that the font manager look at Driver window size, and resize
	 * fontSize so it keeps same size than if it was in 800x600...
	 */
	virtual	void			setKeep800x600Ratio (bool keep) = 0;
	/** return keep800x600Ratio state.
	 */
	virtual	bool			getKeep800x600Ratio () const = 0;
	// @}


	/// \name Rendering.
	/** All rendering are done in proprietary matrix context for UTextContext:
	 *		- UTextContext use the setuped viewport of UDriver Matrix context.
	 *		- UTextContext use its own Matrix2D setup (own Frustum and own ViewAMtrix/ ModelMatrix).
	 *			  Exception: render3D() use UDriver Matrix context for Frustum/ViewMatrix, but use its own modelmatrix.
	 */
	// @{
	/**
	 * compute and add a string to the stack
	 * \param a string
	 * \return the index where string has been inserted
	 */
	virtual	uint32			textPush (const char *format, ...) = 0;
	/**
	 * computes an ucstring and adds the result to the stack
	 * \param an ucstring
	 * \return the index where computed string has been inserted
	 */
	virtual	uint32			textPush (NLMISC::CUtfStringView sv) = 0;
	/**
	 * set the color of a string.
	 */
	virtual	void			setStringColor(uint32 i, NLMISC::CRGBA newCol)= 0;
	/**
	 * set the letter selection of a string. Only letters in the range given are displayed. Default is 0/0xFFFFFFFF
	 */
	virtual	void			setStringSelection(uint32 i, uint32 selectStart, uint32 selectSize)= 0;
	/**
	 * reset the letter selection of a string to 0/0xFFFFFFFF (all displayed)
	 */
	virtual	void			resetStringSelection(uint32 i)= 0;
	/**
	 * remove a string from the list
	 */
	virtual	void			erase (uint32 i) = 0;
	/**
	 * Get a string information from the list. return CStringInfo(0,0) if not found.
	 *	The returned string info is in pixel size per default.
	 */
	virtual	CStringInfo		getStringInfo (uint32 i) = 0;
	/**
	 * Get a string information from the ucstring
	 *	The returned string info is in pixel size per default.
	 */
	virtual	CStringInfo		getStringInfo (NLMISC::CUtfStringView sv) = 0;
	/**
	* Get a string information from the ucstring
	*	The returned string info is in pixel size per default.
	*/
	virtual	CStringInfo		getStringInfo (NLMISC::CUtfStringView sv, size_t len) = 0;
	/**
	 * empty the map
	 */
	virtual	void			clear () = 0;
	/**
	 * print a string of the list (2D method). x/y E [0,1]
	 * (rq : it leaves the string in the stack)
	 * z : if the hotspot is bottom z is the position of the line of the string, not the bottom of the string bounding box !
	*/
	virtual	void			printAt (float x, float y, uint32 i) = 0;
	/** Same as printAt but special version for interface: clip and insert in a temp buffer.
	  * z : if the hotspot is bottom z is the position of the line of the string, not the bottom of the string bounding box !
	*/
	virtual	void			printClipAt (URenderStringBuffer &renderBuffer, float x, float y, uint32 i, float xmin, float ymin, float xmax, float ymax) = 0;
	/** Same as printClipAt but special version for planar 3d interface: the final vertices are unproject using a frustum.
	  * depth is the positive depth to used to unproject the string
	  * worldSpaceMatrix is used to mul each vertex (when in 0..1 coordinate for x/y and in meter for z coordinate)
	*/
	virtual	void			printClipAtUnProjected (URenderStringBuffer &renderBuffer, class NL3D::CFrustum &frustum, const NLMISC::CMatrix &worldSpaceMatrix, float x, float y, float depth, uint32 i, float xmin, float ymin, float xmax, float ymax) = 0;
	// TEMP
	virtual	void			printClipAtOld (float x, float y, uint32 i, float xmin, float ymin, float xmax, float ymax) = 0;
	/**
	 * compute and print a ucstring at the location (2D method) x/y E [0,1]
	 */
	virtual	void			printAt (float x, float y, NLMISC::CUtfStringView sv) = 0;
	/**
	 * compute and print a string at the location (2D method) x/y E [0,1]
	 */
	virtual	void			printfAt (float x, float y, const char *format, ...) = 0;

	/**
	 * compute and render a ucstring at the location (3D method)
	 *	render3D() use UDriver Matrix context for Frustum/ViewMatrix, but use its own modelmatrix (mat).
	 */
	virtual	void			render3D (const NLMISC::CMatrix &mat, NLMISC::CUtfStringView sv) = 0;
	/**
	 * compute and render a string at the location (3D method)
	 *	render3D() use UDriver Matrix context for Frustum/ViewMatrix, but use its own modelmatrix (mat).
	 */
	virtual	void			render3D (const NLMISC::CMatrix &mat, const char *format, ...) = 0;

	/**
	 * Return max x coordinate of last string printed. Useful to know if a string
	 * goes out of the screen (screen limit is supposed at x==4/3, should actually
	 * depend on driver's frustum).
	 *	\return x coordinate
	 */
	virtual	float			getLastXBound () const = 0;
	// @}

	/// Used for debug
	virtual void			dumpCacheTexture (const char *filename) = 0;

	/// create a renderBuffer for printClipAt(). Must delete it with deleteRenderBuffer()
	virtual URenderStringBuffer		*createRenderBuffer() = 0;
	virtual void					deleteRenderBuffer(URenderStringBuffer *buffer) = 0;

	/// Flush the rendered string buffer. This method sets the driver matrix to a 2d11 matrix and sets ztest to always and disable z write.
	virtual void					flushRenderBuffer(URenderStringBuffer *buffer) = 0;

	/// Flush the rendered string buffer. This method doesn't change the current matrices nor the material properties.
	virtual void					flushRenderBufferUnProjected(URenderStringBuffer *buffer, bool zwrite) = 0;

	/// In single line mode you can assign several color to letters
	virtual void setLetterColors(ULetterColors * letterColors, uint index) = 0;
	virtual bool isSameLetterColors(ULetterColors * letterColors, uint index) = 0;
	virtual ULetterColors * createLetterColors() = 0;

};


} // NL3D


#endif // NL_U_TEXT_CONTEXT_H

/* End of u_text_context.h */
