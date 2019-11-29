// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef NL_VIEW_BITMAP_H
#define NL_VIEW_BITMAP_H

#include "nel/gui/view_base.h"
#include "nel/3d/u_texture.h"
#include "nel/gui/view_renderer.h"

namespace NLGUI
{

	/**
	 * class implementing a bitmap view
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CViewBitmap : public CViewBase
	{
	public:
		DECLARE_UI_CLASS(CViewBitmap)
		enum EType { Stretched = 0, Tiled, TypeCount };
	public:

		/// Constructor
		CViewBitmap(const TCtorParam &param) : CViewBase(param)
		{
			_Color = NLMISC::CRGBA(255,255,255,255);
			_Scale = false;
			_Rot = 0;
			_Flip = false;
			_Tile = false;
			_Align = 0;
			_Type = Stretched;
			_InheritGCAlpha = false;

			// Default parameters for createTexture
			_TxtOffsetX = 0;
			_TxtOffsetY = 0;
			_TxtWidth = -1;
			_TxtHeight = -1;

			// Support for https://.. textures
			_HtmlDownload = false;
		}

		/// Destructor
		virtual ~CViewBitmap();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		/**
		 * parse an xml node and initialize the base view mambers. Must call CViewBase::parse
		 * \param cur : pointer to the xml node to be parsed
		 * \param parentGroup : the parent group of the view
		 * \partam id : a refence to the string that will receive the view ID
		 * \return true if success
		 */
		bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }

		virtual void updateCoords ();

		/// Draw the view
		virtual void draw ();

		bool  getScale() const { return _Scale; }
		void setScale (bool s) { _Scale = s; }
		bool  getTile() const { return _Tile; }
		void setTile (bool s) { _Tile = s; }
		void setColor (const NLMISC::CRGBA &r) { _Color = r; }

		// Reflected

		virtual void setTexture(const std::string & TxName);
		virtual std::string getTexture () const;

		/** Force the bitmap to match current texture size
		  * The 'scale' flag isnot modified
		  */
		void fitTexture();

		bool isTextureValid() const { return _TextureId != -1; }

		void setColorAsString(const std::string & col);
		std::string getColorAsString() const;

		void	setColorAsInt(sint32 col);
		sint32	getColorAsInt() const;

		void			setColorRGBA(NLMISC::CRGBA col);
		NLMISC::CRGBA	getColorRGBA() const;

		virtual sint32 getAlpha() const { return _Color.A; }
		virtual void setAlpha (sint32 a) { _Color.A = (uint8)a; }

		REFLECT_EXPORT_START(CViewBitmap, CViewBase)
			REFLECT_STRING ("color", getColorAsString, setColorAsString);
			REFLECT_SINT32 ("color_as_int", getColorAsInt, setColorAsInt);
			REFLECT_RGBA ("color_rgba", getColorRGBA, setColorRGBA);
			REFLECT_SINT32 ("alpha", getAlpha, setAlpha);
			REFLECT_STRING ("texture", getTexture, setTexture);
			REFLECT_BOOL("scale", getScale, setScale);
		REFLECT_EXPORT_END

		/// \from CInterfaceElement
		sint32	getMaxUsedW() const;
		sint32	getMinUsedW() const;

		virtual void serial(NLMISC::IStream &f);

	protected:
		CViewRenderer::CTextureId	_TextureId;	/// Accelerator
		NLMISC::CRGBA	_Color;
		sint32	_Rot;
		sint32	_Align;		/// 1st bit - Left/Right (0/1) 2nd bit - Bottom/Top (0/1)
		EType	_Type;
		bool	_Scale          : 1;
		bool	_Flip           : 1;
		bool	_Tile           : 1;
		bool	_InheritGCAlpha : 1;
		bool	_HtmlDownload   : 1;

		// For single texture

		sint32	_TxtOffsetX;		// Offset X of the single texture
		sint32	_TxtOffsetY;		// Offset Y of the single texture
		sint32	_TxtWidth;			// Width of the single texture
		sint32	_TxtHeight;			// Height of the single texture

	};


}

#endif // NL_VIEW_BITMAP_H

/* End of view_bitmap.h */
