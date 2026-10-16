// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2018  Winch Gate Property Limited
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
	virtual ~CTextContextUser() NL_OVERRIDE
	{
	}


	/// \name Text look.
	// @{
	void setColor(NLMISC::CRGBA color) NL_OVERRIDE;
	void setFontSize(uint32 fontSize) NL_OVERRIDE;
	uint32 getFontSize() const NL_OVERRIDE;
	void setEmbolden(bool b) NL_OVERRIDE;
	bool getEmbolden() const NL_OVERRIDE;
	void setOblique(bool b) NL_OVERRIDE;
	bool getOblique() const NL_OVERRIDE;
	void setHotSpot(THotSpot hotSpot) NL_OVERRIDE;
	THotSpot getHotSpot() const NL_OVERRIDE;
	void setScaleX(float scaleX) NL_OVERRIDE;
	void setScaleY(float scaleY) NL_OVERRIDE;
	float getScaleX() const NL_OVERRIDE;
	float getScaleY() const NL_OVERRIDE;
	void setShaded(bool b) NL_OVERRIDE;
	bool getShaded() const NL_OVERRIDE;
	void setShadeOutline(bool b) NL_OVERRIDE;
	bool getShadeOutline() const NL_OVERRIDE;
	void setShadeExtent(float x, float y) NL_OVERRIDE;
	void setShadeColor (NLMISC::CRGBA sc) NL_OVERRIDE;
	NLMISC::CRGBA getShadeColor () const NL_OVERRIDE;
	void setKeep800x600Ratio(bool keep) NL_OVERRIDE;
	bool getKeep800x600Ratio() const NL_OVERRIDE;
	// @}


	/// \name Rendering.
	/** All rendering are done in current UDriver matrix context. So verify your 2D/3D modes.
	 *
	 */
	// @{
	uint32 textPush(const char *format, ...) NL_OVERRIDE  ;
	uint32 textPush(NLMISC::CUtfStringView sv) NL_OVERRIDE  ;
	void setStringColor(uint32 i, CRGBA newCol) NL_OVERRIDE;
	void setStringSelection(uint32 i, uint32 selectStart, uint32 selectSize) NL_OVERRIDE;
	void resetStringSelection(uint32 i) NL_OVERRIDE;
	void erase(uint32 i) NL_OVERRIDE  ;
	virtual	CStringInfo		getStringInfo (uint32 i) NL_OVERRIDE;
	virtual	CStringInfo		getStringInfo (NLMISC::CUtfStringView sv) NL_OVERRIDE;
	virtual	CStringInfo		getStringInfo (NLMISC::CUtfStringView sv, size_t len) NL_OVERRIDE;

	void clear() NL_OVERRIDE  ;
	void printAt(float x, float y, uint32 i) NL_OVERRIDE ;
	void printClipAt(URenderStringBuffer &renderBuffer, float x, float y, uint32 i, float xmin, float ymin, float xmax, float ymax) NL_OVERRIDE ;
	void printClipAtUnProjected(URenderStringBuffer &renderBuffer, class NL3D::CFrustum &frustum, const NLMISC::CMatrix &scaleMatrix, float x, float y, float depth, uint32 i, float xmin, float ymin, float xmax, float ymax) NL_OVERRIDE;
	void printClipAtOld (float x, float y, uint32 i, float xmin, float ymin, float xmax, float ymax) NL_OVERRIDE;

	void printAt(float x, float y, NLMISC::CUtfStringView sv) NL_OVERRIDE ;
	void printfAt(float x, float y, const char * format, ...) NL_OVERRIDE ;

	void render3D(const CMatrix &mat, NLMISC::CUtfStringView sv) NL_OVERRIDE ;
	void render3D(const CMatrix &mat, const char *format, ...) NL_OVERRIDE ;

	float getLastXBound() const NL_OVERRIDE ;
	// @}

	void			dumpCacheTexture (const char *filename) NL_OVERRIDE;

	virtual URenderStringBuffer		*createRenderBuffer() NL_OVERRIDE;
	virtual void					deleteRenderBuffer(URenderStringBuffer *buffer) NL_OVERRIDE;
	virtual void					flushRenderBuffer(URenderStringBuffer *buffer) NL_OVERRIDE;
	virtual void					flushRenderBufferUnProjected(URenderStringBuffer *buffer, bool zwrite) NL_OVERRIDE;

	CTextContext	&getTextContext() {return _TextContext;}

	/// \letters colors in single line mode.
	/** In single line mode you can assign several color to letters
	 *
	 */
	// @{
	virtual void setLetterColors(ULetterColors * letterColors, uint index) NL_OVERRIDE;
	virtual bool isSameLetterColors(ULetterColors * letterColors, uint index) NL_OVERRIDE;
	virtual ULetterColors * createLetterColors() NL_OVERRIDE;
	// @}
};


} // NL3D


#endif // NL_TEXT_CONTEXT_USER_H

/* End of text_context_user.h */
