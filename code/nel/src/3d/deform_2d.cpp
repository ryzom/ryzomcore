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

#include "nel/3d/deform_2d.h"

#include "nel/misc/vector_2f.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/heat_haze.h"
#include "nel/3d/quad_effect.h"
#include "nel/3d/texture_blank.h"
#include "nel/3d/driver.h"
#include "nel/3d/dru.h"
#include "nel/3d/material.h"

namespace NL3D {



uint CDeform2d::_Width;
uint CDeform2d::_Height;
uint CDeform2d::_XGranularity;
uint CDeform2d::_YGranularity;
uint CDeform2d::_XQuad;
uint CDeform2d::_YQuad;
NLMISC::CSmartPtr<ITexture> CDeform2d::_Tex;


void		CDeform2d::setupBuffer(uint width, uint height, uint xGranularity, uint yGranularity
										, uint xQuad, uint yQuad)
{
	_Width  = width;
	_Height = height;
	_XGranularity = xGranularity;
	_YGranularity = yGranularity;
	_XQuad = xQuad;
	_YQuad = yQuad;

	uint realWidth = NLMISC::raiseToNextPowerOf2(_Width);
	uint realHeight= NLMISC::raiseToNextPowerOf2(_Height);

	_Tex = new CTextureBlank;
	_Tex->resize(realWidth, realHeight, CBitmap::RGBA);
}




/// compute the UV perturbation at one position of the screen (with screen mapped to 0..1, 0..1)
static inline void computePerturbation(const float x, const float y, float &dx, float &dy)
{
	// dummy func for now
/*	dx = 0.05f * (float) (rand() * (1.F  / RAND_MAX));
	dy = 0.05f * (float) (rand() * (1.F  / RAND_MAX));*/
	dx= 0.f;
	dy = 0.f;
}

void CDeform2d::doDeform(const TPoint2DVect &surf, IDriver *drv, IPerturbUV *uvp)
{

	nlassert(uvp);

	typedef CQuadEffect::TPoint2DVect TPoint2DVect;
	TPoint2DVect dest;

	CQuadEffect::processPoly(surf, (float) _XGranularity, (float) _YGranularity, dest);

	uint realWidth = NLMISC::raiseToNextPowerOf2(_Width);
	uint realHeight= NLMISC::raiseToNextPowerOf2(_Height);


	// draw the poly contour
	/*for (uint k = 0; k < dest.size(); ++k)
	{
		CDRU::drawLine(dest[k].x, dest[k].y, dest[(k + 1) % dest.size()].x, dest[(k + 1) % dest.size()].y, *drv, CRGBA::Red);
	}*/




	static CMaterial mat;
	mat.setDoubleSided(true);
	mat.setLighting(false);
	mat.setZFunc(CMaterial::always);
/*	mat.setColor(CRGBA::Red);
	mat.texEnvOpRGB(0, CMaterial::Add); */

	static CVertexBuffer  vb;
	vb.setName("CDeform2d");
	vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	vb.setPreferredMemory(CVertexBuffer::RAMVolatile, false);



	drv->setFrustum(0, (float) _Width, 0, (float) _Height, -1, 1, false);
	drv->setupViewMatrix(CMatrix::Identity);
	drv->setupModelMatrix(CMatrix::Identity);


	const float iDu = 1.f / _Width;
	const float iDv = 1.f / _Height;
	const float widthRatio = _Width / (float) realWidth;
	const float heightRatio = _Height / (float) realHeight;


	float u, u2, v;
	float du, dv;

	TPoint2DVect::const_iterator it;

	// get back datas from frame buffer
	for (it = dest.begin(); it != dest.end(); ++it)
	{
		// todo hulud use the new render to texture interface
		// drv->copyFrameBufferToTexture(_Tex, 0, (uint32) it->x,(uint32) it->y, (uint32) it->x, (uint32) it->y, _XGranularity, _YGranularity);
	}


	/** setup the whole vertex buffer
	  * we don't share vertices here, as we work with unaligned quads
	  */
	vb.setNumVertices((uint32)dest.size() << 2);
	mat.setTexture(0, _Tex);
	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);

		uint k = 0; // current index in the vertex buffer
		for (it = dest.begin(); it != dest.end(); ++it, k += 4)
		{

			// \todo optimize this by a direct access to the vertex buffer (if needed)
			// blit data to frame buffer and apply deformations

			vba.setVertexCoord(k, NLMISC::CVector(it->x, 0, it->y));
			vba.setVertexCoord(k + 1, NLMISC::CVector(it->x + _XGranularity, 0, it->y));
			vba.setVertexCoord(k + 2, NLMISC::CVector(it->x + _XGranularity, 0, it->y + _YGranularity));
			vba.setVertexCoord(k + 3, NLMISC::CVector(it->x , 0, it->y + _YGranularity));

			// perturbation of the uv coordinates

			u =  it->x * iDu;
			v = it->y * iDv;
			uvp->perturbUV(u, v, du, dv);
			vba.setTexCoord(k, 0, (u + du) * widthRatio, (v + dv) * heightRatio );

			u2 =  (it->x + _XGranularity) * iDu;
			uvp->perturbUV(u2, v, du, dv);
			vba.setTexCoord(k + 1, 0, (u2 + du) * widthRatio, (v + dv) * heightRatio );

			v =  (it->y + _YGranularity) * iDv;
			uvp->perturbUV(u2, v, du, dv);
			vba.setTexCoord(k + 2, 0, (u2 + du) * widthRatio, (v + dv) * heightRatio );

			uvp->perturbUV(u, v, du, dv);
			vba.setTexCoord(k + 3, 0, (u + du) * widthRatio, (v + dv) * heightRatio );
		}
	}

	drv->activeVertexBuffer(vb);
	drv->renderRawQuads(mat, 0, (uint32)dest.size());
}

} // NL3D
