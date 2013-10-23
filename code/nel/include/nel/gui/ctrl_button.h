// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef RZ_CTRL_BUTTON_H
#define RZ_CTRL_BUTTON_H

#include "nel/gui/ctrl_base_button.h"
#include "nel/gui/view_renderer.h"

namespace NLGUI
{
	class CEventDescriptor;

	/**
	 * <Class description>
	 * \author Nicolas Brigand
	 * \author Nevrax France
	 * \date 2002
	 */
	class CCtrlButton : public CCtrlBaseButton
	{
	public:
        DECLARE_UI_CLASS( CCtrlButton )

		/// Constructor
		CCtrlButton(const TCtorParam &param) : CCtrlBaseButton(param)
		{
			_Scale = false;
			_Align = 0;
		}

		void setAlignFromString( const std::string &s );

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		// Init part
		virtual bool parse (xmlNodePtr cur,CInterfaceGroup * parentGroup);

		virtual void updateCoords();

		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }

		virtual bool getMouseOverShape(std::string &/* texName */, uint8 &/* rot */, NLMISC::CRGBA &/* col */);

		// Display part
		virtual void draw();

		void setTexture (const std::string&name);
		void setTexturePushed (const std::string&name);
		void setTextureOver (const std::string&name);

		void fitTexture();

		std::string getTexture () const;
		std::string getTexturePushed () const;
		std::string getTextureOver() const;

		bool isTextureValid() const { return _TextureIdNormal != -1; }

		// test if the texture must scale
		bool  getScale() const { return _Scale; }
		void  setScale(bool scale) { _Scale = scale; }


		/// \from CInterfaceElement
		sint32	getMaxUsedW() const;
		sint32	getMinUsedW() const;

		REFLECT_EXPORT_START(CCtrlButton, CCtrlBaseButton)
			REFLECT_STRING("texture", getTexture, setTexture);
			REFLECT_STRING("texture_pushed", getTexturePushed, setTexturePushed);
			REFLECT_STRING("texture_over", getTextureOver, setTextureOver);
			REFLECT_BOOL("scale", getScale, setScale);
		REFLECT_EXPORT_END

	protected:

		CViewRenderer::CTextureId	_TextureIdNormal;
		CViewRenderer::CTextureId	_TextureIdPushed;
		CViewRenderer::CTextureId	_TextureIdOver;

	private:

		bool	_Scale;
		sint32	_Align;		/// 1st bit - Left/Right (0/1) 2nd bit - Bottom/Top (0/1)
	};

}

#endif // RZ_CTRL_BUTTON_H

/* End of ctrl_button.h */
