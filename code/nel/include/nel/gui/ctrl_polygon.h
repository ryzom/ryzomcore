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


#ifndef RZ_CTRL_POLYGON_H
#define RZ_CTRL_POLYGON_H

#include "nel/gui/ctrl_base.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/polygon.h"

namespace NLMISC
{
	class CVector2f;
}

namespace NLGUI
{

	/** Display of an arbitrary polygon in the ui.
	  * polygons are clipped & batched.
	  *
	  * Derives from CCtrlBase in order to provide button / tooltip capability
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 1/2006
	  */
	class CCtrlPolygon : public CCtrlBase
	{
	public:
        DECLARE_UI_CLASS( CCtrlPolygon )
        CCtrlPolygon( const TCtorParam &param );
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }
		virtual void updateCoords();
		virtual void draw();
		/** Change the vertices. This is costly because concav / complex polys are split in a list of triangles
		  */
		void	setVertices(const std::vector<NLMISC::CVector> &vertices);
		const std::vector<NLMISC::CVector> &getVertices() const { return _Poly.Vertices; }
		// test if current position in inside the current (transformed) poly (in window space)
		bool	contains(const NLMISC::CVector2f &pos) const;
		// color
		void			setColorRGBA(NLMISC::CRGBA col) { _Color = col; }
		NLMISC::CRGBA	getColorRGBA() const { return _Color; }
		// from CViewBase
		virtual sint32  getAlpha() const { return (sint32) _Color.A; }
		virtual void	setAlpha(sint32 a);
		/** Change the matrix for this poly. Changing the matrix is usually cheaper than changing
		  * The vertices because complex poly do not have to be split again
		  */
		//void setMatrix(const NLMISC::CMatrix &mat);
		//const NLMISC::CMatrix &getMatrix() const { return _Matrix; }
		// test if last call to 'setVertices' was for a valid poly (e.g one that doesn't overlapp itself)
		bool isValid() const { return _Valid; }
		virtual bool		handleEvent (const NLGUI::CEventDescriptor &event);

		// no capturable by default (just tooltip capability wanted)
		virtual	bool		isCapturable() const { return false; }
	private:
		NLMISC::CPolygon _Poly;
		NLMISC::CPolygon2D _XFormPoly;
		//NLMISC::CMatrix  _Matrix;
		bool			 _Valid;
		bool			 _Touched;
		NLMISC::CRGBA				   _Color;
		std::vector<NLMISC::CTriangle> _Tris;
		std::vector<NLMISC::CTriangle> _RealTris; // clipped tris in screen coordinates
	private:
		void updateBoudingRect();
	protected:
		// TMP TMP : have to solve matrix imprecision for display in map -> do the full computation for now ...
		virtual void computeScaledVertex(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src);
	public:
		void touch();
	};

}


#endif
