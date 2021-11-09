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


#ifndef RZ_VIEW_POLYGON_H
#define RZ_VIEW_POLYGON_H

#include "nel/gui/view_base.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/polygon.h"

namespace NLGUI
{

	/** Display of an arbitrary polygon in the ui.
	  * polygon is clipped & batched
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 1/2006
	  */
	class CViewPolygon : public CViewBase
	{
	public:
        DECLARE_UI_CLASS( CViewPolygon )

        CViewPolygon( const TCtorParam &param );
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }
		virtual void updateCoords();
		virtual void draw();
		void	setVertices(const std::vector<NLMISC::CVector> &vertices);
		// color
		void			setColorRGBA(NLMISC::CRGBA col) { _Color = col; }
		NLMISC::CRGBA	getColorRGBA() const { return _Color; }
		// from CViewBase
		virtual sint32 getAlpha() const { return (sint32) _Color.A; }
		virtual void setAlpha (sint32 a);
	private:
		NLMISC::CPolygon _Poly;
		bool _Touched;
		NLMISC::CRGBA				   _Color;
		std::vector<NLMISC::CTriangle> _Tris;
		std::vector<NLMISC::CTriangle> _RealTris; // clipped tris in screen coordinates
	};

}

#endif

