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


#ifndef RZ_CTRL_QUAD_H
#define RZ_CTRL_QUAD_H

#include "nel/gui/ctrl_base.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/geom_ext.h"

namespace NLGUI
{

	/** Display of an arbitrary textured quad in the UI. The applied texture is filtered.
	  * Unlike CViewBitmap, the texture is always scaled here, and this ui element coordinates
	  * are driven by the quad vertices coordinates (see setQuad).
	  *
	  * Derives from CCtrlBase for tooltipping support
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 12/2005
	  */
	class CCtrlQuad : public CCtrlBase
	{
	public:
        DECLARE_UI_CLASS( CCtrlQuad )

		enum TWrapMode { Repeat = 0, Clamp, CustomUVs, WrapModeCount };


        CCtrlQuad( const TCtorParam &param );

		// from CInterfaceElement
		bool parse(xmlNodePtr cur,CInterfaceGroup *parentGroup);
		virtual void updateCoords();
		virtual void draw();
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }

		// from CViewBase
		virtual sint32 getAlpha() const { return (sint32) _Color.A; }
		virtual void setAlpha (sint32 a);

		// texture
		void setTexture(const std::string &texName);
		std::string getTexture () const;

		// color
		void			setColorRGBA(NLMISC::CRGBA col) { _Color = col; }
		NLMISC::CRGBA	getColorRGBA() const { return _Color; }

		/** Set a new quad relative to parent pos
		  * x,y, w, h & hotspot are updated to fit the bounding rect of the quad
		  */
		void setQuad(const NLMISC::CQuad &quad);
		void setQuad(const NLMISC::CVector &start, const NLMISC::CVector &end, float thickness);
		/** Fit the given texture size (no hotspot for now, always centered)
		  * NB : current texture is not modified.
		  */
		void setQuad(const std::string &texName, const NLMISC::CVector &pos, float angle = 0.f, float offCenter = 0.f);
		void setQuad(const NLMISC::CVector &pos, float radius, float angle = 0.f);
		const NLMISC::CQuad &getQuad() const { return _Quad; }

		void setAdditif(bool additif);
		bool getAdditif() const { return _Additif; }

		void setFiltered(bool filtered);
		bool getFiltered() const { return _Filtered; }

		void setPattern(float umin, float umax, TWrapMode wrapMode);

		/** Set uvs for each corners -> this will change the wrap mode to CustomUVs
		  * Use setPattern(0.f, 0.f, CCtrlQuad::Repeat) to return to previous behavior
		  */
		void setCustomUVs(const NLMISC::CUV uvs[4]);


		// from CCtrlBase, no op by default
		virtual bool		handleEvent (const NLGUI::CEventDescriptor &event);

		// see if this control contains the given point (in parent coords)
		bool contains(const NLMISC::CVector2f &pos) const;

		// no capturable by default (just tooltip capability wanted)
		virtual	bool		isCapturable() const { return false; }


	private:
		NLMISC::CRGBA				_Color;
		NLMISC::CQuad				_Quad;
		NLMISC::CQuadUV				_RealQuad; // absolute coords
		float						_ClampedUCorrection;
		CViewRenderer::CTextureId	_TextureId;	/// Accelerator
		bool						_Additif;
		bool						_Filtered;
		float						_UMin;
		float						_UMax;
		TWrapMode					_WrapMode;
		NLMISC::CUV					_CustomUVs[4];
	};

}

#endif
