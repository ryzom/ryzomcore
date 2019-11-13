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

#ifndef NL_TEXT_CONTEXT_USER_H
#define NL_TEXT_CONTEXT_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/text_context.h"
#include "nel/3d/driver_user.h"


namespace NL3D
{


/**
 * UTextContext implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTextContextUser : public UTextContext
{
private:
	CTextContext	_TextContext;
	CDriverUser		*_DriverUser;
	IDriver			*_Driver;
	CComputedString _CacheString; // Performance Optimisation

public:

	/// Constructor
	CTextContextUser(const std::string fontFileName, const std::string fontExFileName, CDriverUser *drv, CFontManager *fmg)
	{
		nlassert(drv);
		_DriverUser= drv;

		// The enum of CComputedString and UTextContext MUST be the same!!!
		nlassert((uint)UTextContext::HotSpotCount== (uint)CComputedString::HotSpotCount);

		_Driver= drv->getDriver();
		_TextContext.init(_Driver, fmg);
		_TextContext.setFontGenerator(fontFileName, fontExFileName);
	}
	virtual ~CTextContextUser()
	{
	}


	/// \name Text look.
	// @{
	void setColor(NLMISC::CRGBA color);
	void setFontSize(uint32 fontSize);
	uint32 getFontSize() const;
	void setEmbolden(bool b);
	bool getEmbolden() const;
	void setOblique(bool b);
	bool getOblique() const;
	void setHotSpot(THotSpot hotSpot);
	THotSpot getHotSpot() const;
	void setScaleX(float scaleX);
	void setScaleY(float scaleY);
	float getScaleX() const;
	float getScaleY() const;
	void setShaded(bool b);
	bool getShaded() const;
	void setShadeOutline(bool b);
	bool getShadeOutline() const;
	void setShadeExtent(float x, float y);
	void setShadeColor (NLMISC::CRGBA sc);
	NLMISC::CRGBA getShadeColor () const;
	void setKeep800x600Ratio(bool keep);
	bool getKeep800x600Ratio() const;
	// @}


	/// \name Rendering.
	/** All rendering are done in current UDriver matrix context. So verify your 2D/3D modes.
	 *
	 */
	// @{
	uint32 textPush(const char *format, ...)  ;
	uint32 textPush(const ucstring &str)  ;
	void setStringColor(uint32 i, CRGBA newCol);
	void setStringSelection(uint32 i, uint32 selectStart, uint32 selectSize);
	void resetStringSelection(uint32 i);
	void erase(uint32 i)  ;
	virtual	CStringInfo		getStringInfo (uint32 i);
	virtual	CStringInfo		getStringInfo (const ucstring &ucstr);

	void clear()  ;
	void printAt(float x, float y, uint32 i) ;
	void printClipAt(URenderStringBuffer &renderBuffer, float x, float y, uint32 i, float xmin, float ymin, float xmax, float ymax) ;
	void printClipAtUnProjected(URenderStringBuffer &renderBuffer, class NL3D::CFrustum &frustum, const NLMISC::CMatrix &scaleMatrix, float x, float y, float depth, uint32 i, float xmin, float ymin, float xmax, float ymax);
	void printClipAtOld (float x, float y, uint32 i, float xmin, float ymin, float xmax, float ymax);

	void printAt(float x, float y, const ucstring &ucstr) ;
	void printfAt(float x, float y, const char * format, ...) ;

	void render3D(const CMatrix &mat, const ucstring &ucstr) ;
	void render3D(const CMatrix &mat, const char *format, ...) ;

	float getLastXBound() const ;
	// @}

	void			dumpCacheTexture (const char *filename);

	virtual URenderStringBuffer		*createRenderBuffer();
	virtual void					deleteRenderBuffer(URenderStringBuffer *buffer);
	virtual void					flushRenderBuffer(URenderStringBuffer *buffer);
	virtual void					flushRenderBufferUnProjected(URenderStringBuffer *buffer, bool zwrite);

	CTextContext	&getTextContext() {return _TextContext;}

	/// \letters colors in single line mode.
	/** In single line mode you can assign several color to letters
	 *
	 */
	// @{
	virtual void setLetterColors(ULetterColors * letterColors, uint index);
	virtual bool isSameLetterColors(ULetterColors * letterColors, uint index);
	virtual ULetterColors * createLetterColors();
	// @}
};


} // NL3D


#endif // NL_TEXT_CONTEXT_USER_H

/* End of text_context_user.h */
