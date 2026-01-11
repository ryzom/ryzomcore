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

#include "std3d.h"


#include "nel/3d/heat_haze.h"
#include "nel/3d/scene.h"
#include "nel/3d/deform_2d.h"
#include "nel/misc/vector_2f.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


	struct CSinWave : public CDeform2d::IPerturbUV
	{
		virtual ~CSinWave() {}
		float Phase;
		virtual void perturbUV(float x, float y, float &du, float &dv) const
		{
			du = 0.01f * sinf(25.f * y + Phase);
			dv = 0.05f * cosf(19.3f * x + Phase);
		}
	} _SinWave;




	void CHeatHaze::performHeatHaze(uint width, uint height, CScene &s, IDriver *drv)
	{
		NLMISC::CMatrix m  = s.getCam()->getMatrix();
		NLMISC::CMatrix im = m.inverted();

		// compute the shape of the horizon

		// first we compute the direction of the world up vector in the viewer basis.
		NLMISC::CVector up = im * NLMISC::CVector::K;
		// project onto the I and K vectors
		float upNorm = (up.x * up.x + up.z * up.z);

		const float threshold = 10E-4f;

		if (upNorm < threshold) return; // the viewer is looking above or below  himlself

		// Compute the right vector. This is done by intersecting the horizon plane with a near plane.
		// to do this, we transform the horizon plane into the view basis.This may be optimized, but is not critical.
		//
		NLMISC::CPlane h;
		h.make(NLMISC::CVector::K, NLMISC::CVector::Null);

		h = h * m; // note : this multiply by the transposition of m

		// intersect with near plane : we got y = 0, which gives us, as a right vector :
		// if c is not 0, we got : x = 1 and z = (-a - d) / c as a working solution.
		// Else we got x = (- d - c )/ a and z = 1


		NLMISC::CVector right;

		if (fabsf(h.c) > threshold)
		{
			right.set(1, 0, (h.a + h.d) / - h.c );
		}
		else
		{
			right.set( (h.d + h.c) / h.a, 0,  1);
		}

		right.normalize();

		// now, find a point on screen that lay on the horizon line
		static std::vector<NLMISC::CVector2f> poly(4);



		const sint xCenter = width >> 1;
		const sint yCenter = height >> 1;

		const float horizonHeight = (float) (height >> 2);
		const float horizonWidth  = (float) (width  >> 2);

		NLMISC::CVector tmp ;

		tmp = horizonWidth * right + horizonHeight * up;
		poly[0] = NLMISC::CVector2f(xCenter + tmp.x, yCenter + tmp.z) ;
		tmp = horizonWidth   * right - horizonHeight  * up;
		poly[1] = NLMISC::CVector2f(xCenter + tmp.x, yCenter + tmp.z) ;
		tmp = - horizonWidth   * right - horizonHeight  * up;
		poly[2] = NLMISC::CVector2f(xCenter + tmp.x, yCenter + tmp.z);
		tmp = - horizonWidth   * right + horizonHeight  * up;
		poly[3] = NLMISC::CVector2f(xCenter + tmp.x, yCenter + tmp.z);


		CDeform2d::doDeform(poly, drv, &_SinWave);




	}

} // NL3D
