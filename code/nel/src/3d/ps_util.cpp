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


#include "nel/misc/aabbox.h"
#include "nel/misc/matrix.h"

#include "nel/3d/ps_util.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/nelu.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/font_manager.h"
#include "nel/3d/computed_string.h"
#include "nel/3d/dru.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_sound.h"
#include "nel/3d/ps_light.h"


#include "nel/3d/particle_system_shape.h"



namespace NL3D {


using NLMISC::CVector;


//#ifdef NL_DEBUG
	bool CPSUtil::_CosTableInitialized = false;
	bool CPSUtil::_PerlinNoiseTableInitialized = false;
//#endif

float CPSUtil::_CosTable[256];
float CPSUtil::_SinTable[256];
float CPSUtil::_PerlinNoiseTab[1024];




//==========================================================================
void CPSUtil::initPerlinNoiseTable(void)
{
	NL_PS_FUNC(CPSUtil_initPerlinNoiseTable)
	for (uint32 k = 0; k < 1024; ++k)
	{
		_PerlinNoiseTab[k] = (rand() % 30000) / 30000.f;
	}
	//#ifdef NL_DEBUG
		_PerlinNoiseTableInitialized = true;
	//#endif
}

//==========================================================================
void CPSUtil::initFastCosNSinTable(void)
{
	NL_PS_FUNC(CPSUtil_initFastCosNSinTable)
	for (uint32 k = 0; k < 256; k++)
	{
		const float angle =  k / 256.0f * 2.0f * float(NLMISC::Pi);
		_CosTable[k] = (float) cos( angle );
		_SinTable[k] = (float) sin( angle );
	}
	//#ifdef NL_DEBUG
		_CosTableInitialized = true;
	//#endif
}

//==========================================================================
void CPSUtil::registerSerialParticleSystem(void)
{
	NL_PS_FUNC(CPSUtil_registerSerialParticleSystem)
		NLMISC_REGISTER_CLASS(CPSLocated);
		NLMISC_REGISTER_CLASS(CParticleSystemShape);
		NLMISC_REGISTER_CLASS(CPSSound);
		NLMISC_REGISTER_CLASS(CPSLight);


		registerParticles();
		registerForces();
		registerEmitters();
		registerZones();
		registerAttribs();


		// while we are here, we perform some important inits
		initFastCosNSinTable(); // init fast cosine lookup table
		initPerlinNoiseTable(); // init perlin noise table
}

//==========================================================================
void CPSUtil::displayBBox(NL3D::IDriver *driver, const NLMISC::CAABBox &box, NLMISC::CRGBA col /* = NLMISC::CRGBA::White */)
{
	NL_PS_FUNC(CPSUtil_displayBBox)
	CVector max = box.getMax()
			,min = box.getMin();
	CVertexBuffer vb;
	vb.setVertexFormat(CVertexBuffer::PositionFlag);
	vb.setNumVertices(8);
	vb.setPreferredMemory(CVertexBuffer::AGPVolatile, false);

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord(0, min);
		vba.setVertexCoord(1, CVector(max.x, min.y, min.z));
		vba.setVertexCoord(2, CVector(min.x, max.y, min.z));
		vba.setVertexCoord(3, CVector(max.x, max.y, min.z));
		vba.setVertexCoord(4, CVector(min.x, min.y, max.z));
		vba.setVertexCoord(5, CVector(max.x, min.y, max.z));
		vba.setVertexCoord(6, CVector(min.x, max.y, max.z));
		vba.setVertexCoord(7, max);
	}


	CMaterial material;

	material.setColor(col);
	material.setLighting(false);
	material.setBlendFunc(CMaterial::one, CMaterial::one);
	material.setZWrite(false);
	material.setBlend(true);



	CIndexBuffer pb;
	pb.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	pb.setNumIndexes(2*12);
	pb.setPreferredMemory(CIndexBuffer::RAMVolatile, false);
	{
		CIndexBufferReadWrite ibaWrite;
		pb.lock (ibaWrite);
		ibaWrite.setLine(0, 0, 1);
		ibaWrite.setLine(2, 1, 5);
		ibaWrite.setLine(4, 5, 4);
		ibaWrite.setLine(6, 4, 0);
		ibaWrite.setLine(8, 0, 2);
		ibaWrite.setLine(10, 1, 3);
		ibaWrite.setLine(12, 4, 6);
		ibaWrite.setLine(14, 5, 7);
		ibaWrite.setLine(16, 6, 7);
		ibaWrite.setLine(18, 7, 3);
		ibaWrite.setLine(20, 3, 2);
		ibaWrite.setLine(22, 2, 6);
	}



	driver->activeVertexBuffer(vb);
	driver->activeIndexBuffer(pb);
	driver->renderLines(material, 0, pb.getNumIndexes()/2);
}


//==========================================================================
void CPSUtil::displayArrow(IDriver *driver, const CVector &start, const CVector &v, float size, CRGBA col1, CRGBA col2)
{

	NL_PS_FUNC(CPSUtil_displayArrow)

	const float coneSize = size * 0.1f;

	static CIndexBuffer vTab;
	static const TIndexType vTabIndexes[] =
	{ 1, 2, 4,
	  4, 2, 3,
	  1, 2, 0,
	  2, 3, 0,
	  3, 4, 0,
	  4, 1, 0 };
	vTab.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);

	if (vTab.getNumIndexes()==0)
	{
		vTab.setNumIndexes (sizeofarray(vTabIndexes));
		CIndexBufferReadWrite iba;
		vTab.lock(iba);
		memcpy (iba.getPtr(), vTabIndexes, sizeof(vTabIndexes));
	}


	CVector end = start + size * v;
	CDRU::drawLine(start, end, col1, *driver);
	CMatrix m;
	buildSchmidtBasis(v, m);


	CVertexBuffer vb;
	vb.setVertexFormat(CVertexBuffer::PositionFlag);
	vb.setNumVertices(5);
	vb.setPreferredMemory(CVertexBuffer::AGPVolatile, false);


	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord(0, end + m * CVector(0, 0, 3.0f * coneSize) );
		vba.setVertexCoord(1, end + m * CVector(-coneSize, -coneSize, 0) );
		vba.setVertexCoord(2, end + m * CVector(coneSize, -coneSize, 0) );
		vba.setVertexCoord(3, end + m * CVector(coneSize, coneSize, 0) );
		vba.setVertexCoord(4, end + m * CVector(-coneSize, coneSize, 0) );
	}


	CMaterial material;

	material.setColor(col2);
	material.setLighting(false);
	material.setBlendFunc(CMaterial::one, CMaterial::one);
	material.setZWrite(false);
	material.setBlend(true);
	material.setDoubleSided(true);



	driver->activeVertexBuffer(vb);
	driver->activeIndexBuffer(vTab);
	driver->renderTriangles(material, 0, 6);


}

//==========================================================================
void CPSUtil::displayBasis(IDriver *driver, const CMatrix &modelMat, const NLMISC::CMatrix &m, float size, CFontGenerator &fg, CFontManager &fm)
{
	NL_PS_FUNC(CPSUtil_displayBasis)

	CMaterial material;

	driver->setupModelMatrix(modelMat);




	displayArrow(driver, m.getPos(), m.getI(), size, CRGBA(127, 127, 127), CRGBA(0, 0, 80));

	displayArrow(driver, m.getPos(), m.getJ(), size, CRGBA(127, 127, 127), CRGBA(0, 0, 80));

	displayArrow(driver, m.getPos(), m.getK(), size, CRGBA(127, 127, 127), CRGBA(200, 0, 80));


	// draw the letters

	CPSUtil::print(driver, std::string("x"), fg, fm, modelMat * m * CVector(1.4f * size, 0, 0), 15.0f * size);

	CPSUtil::print(driver, std::string("y"), fg, fm, modelMat * m * CVector(0, 1.4f  * size, 0), 15.0f * size);

	CPSUtil::print(driver, std::string("z"), fg, fm, modelMat * m * CVector(0, 0, 1.4f  * size), 15.0f * size);

};


//==========================================================================
void CPSUtil::print(IDriver *driver, const std::string &text, CFontGenerator &fg, CFontManager &fm, const CVector &pos, float size, NLMISC::CRGBA col /*= NLMISC::CRGBA::White*/)
{
	NL_PS_FUNC(CPSUtil_print)
	nlassert((&fg) && (&fm));
	CComputedString cptedString;
	fm.computeString ( text,
						&fg,
						col,
						16,
						driver,
						cptedString);


	CMatrix mat = driver->getViewMatrix();
	mat.setPos(CVector::Null);
	mat.scale(CVector(size, size, size));
	mat.transpose();
	mat.setPos(pos);
	cptedString.render3D(*driver, mat);
}


//==========================================================================
void CPSUtil::buildSchmidtBasis(const CVector &k_, NLMISC::CMatrix &result)
{
	NL_PS_FUNC(CPSUtil_buildSchmidtBasis)
	const float epsilon = 10E-4f;
	CVector k = k_;
	k.normalize();
	CVector i;
	if ((1.0f - fabsf(k * CVector::I)) > epsilon)
	{
		i = k ^ CVector::I;
	}
	else
	if ((1.0f - fabs(k * CVector::J)) > epsilon)
	{
		i = k ^ CVector::J;
	}
	else
	{
		i = k ^ CVector::K;
	}

	i = i - (k * i) * k;
	i.normalize();
	result.setRot(i, k ^ i, k, true);
}


//==========================================================================
void CPSUtil::displaySphere(IDriver &driver, float radius, const CVector &center, uint nbSubdiv, CRGBA color)
{
	NL_PS_FUNC(CPSUtil_displaySphere)
	uint x, y, k;
	CVector p, p1, p2;

	static const CVector lK[] = { CVector::I, -CVector::I
								,CVector::J, -CVector::J
								,CVector::K, -CVector::K };

/*	static const CVector lI = { CVector::J, -CVector::J
								,CVector::K, -CVector::K
								,CVector::I, -CVector::I };*/



	for (k = 0; k < 6; ++k)
	{
		const CVector &I = lK[(k + 2) % 6];
		const CVector &K = lK[k];
		const CVector J = K ^ I;

		for (x = 0; x < nbSubdiv; ++x)
		{
			for (y = 0; y < nbSubdiv; ++y)
			{
				p = ((2.f * x / float(nbSubdiv) ) - 1.f) * I + ((2.f * y / float(nbSubdiv) ) - 1.f) * J + K;
				p1 = p + 2.f / float(nbSubdiv) * I;
				p2 = p + 2.f / float(nbSubdiv) * J;

				p.normalize();
				p1.normalize();
				p2.normalize();

				p = center + radius * p;
				p1 = center + radius * p1;
				p2 = center + radius * p2;

				CDRU::drawLine(p, p1, color, driver);
				CDRU::drawLine(p, p2, color, driver);
			}
		}
	}
}


//==========================================================================
void CPSUtil::displayDisc(IDriver &driver, float radius, const CVector &center, const CMatrix &mat, uint nbSubdiv, CRGBA color)
{
	NL_PS_FUNC(CPSUtil_displayDisc)
	// not optimized, but for edition only
	float thetaDelta = (float) NLMISC::Pi * 2.f / nbSubdiv;
	float theta = 0.f;
	const CVector &I = mat.getI();
	const CVector &J = mat.getJ();
	for (uint k = 0; k < nbSubdiv; ++k)
	{

		CDRU::drawLine(center + radius * ((float) cos(theta) * I + (float) sin(theta) * J)
					   , center + radius * ((float) cos(theta + thetaDelta) * I + (float) sin(theta + thetaDelta) * J)
					   , color, driver);
		theta += thetaDelta;
	}

}

//==========================================================================
void CPSUtil::displayCylinder(IDriver &driver, const CVector &center, const CMatrix &mat, const CVector &dim, uint nbSubdiv, CRGBA color)
{
	NL_PS_FUNC(CPSUtil_displayCylinder)
	// not optimized, but for edition only
	float thetaDelta = (float) NLMISC::Pi * 2.f / nbSubdiv;
	float theta = 0.f;
	const CVector &I = mat.getI();
	const CVector &J = mat.getJ();
	const CVector &K = mat.getK();

	for (uint k = 0; k < nbSubdiv; ++k)
	{

		CDRU::drawLine(center + dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , center + dim.z * K + dim.x * cosf(theta + thetaDelta) * I + dim.y * sinf(theta + thetaDelta) * J
					   , color, driver);

		CDRU::drawLine(center - dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , center - dim.z * K + dim.x * cosf(theta + thetaDelta) * I + dim.y * sinf(theta + thetaDelta) * J
					   , color, driver);

		CDRU::drawLine(center + dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , center - dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , color, driver);




		theta += thetaDelta;
	}
}

//==========================================================================
void CPSUtil::display3DQuad(IDriver &driver, const CVector &c1, const CVector &c2
								,const CVector &c3,  const CVector &c4, CRGBA color)
{
	NL_PS_FUNC(CPSUtil_display3DQuad)
	CDRU::drawLine(c1, c2, color, driver);
	CDRU::drawLine(c2, c3, color, driver);
	CDRU::drawLine(c3, c4, color, driver);
	CDRU::drawLine(c4, c1, color, driver);
}

} // NL3D
