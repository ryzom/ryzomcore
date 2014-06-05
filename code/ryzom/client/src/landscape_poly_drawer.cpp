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

#include "stdpch.h"
#include "landscape_poly_drawer.h"

// 3D Interfaces
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_landscape.h"

// 3d
#include "nel/3d/zone.h"
#include "nel/3d/driver_user.h"

// client
#include "decal.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;


// EXTERN
extern UDriver					*Driver;
extern UScene					*Scene;
extern UMaterial				GenericMat;
extern ULandscape	*Landscape;


//-----------------------------------------------------------------------------------------------------------
//---------------------------------------- CInitStencil -----------------------------------------------------
//-----------------------------------------------------------------------------------------------------------

void CInitStencil::init()
{
	// set ILandscapePolyDrawingCallback attribute of current scene
	Scene->setLandscapePolyDrawingCallback(this);
}

//-----------------------------------------------------------------------------------------------------------

void CInitStencil::beginPolyDrawing()
{
	// The eighth bit will be written with a 1 during next render (landscape)
	Driver->stencilOp(UDriver::keep, UDriver::keep, UDriver::replace);
}

//-----------------------------------------------------------------------------------------------------------

void CInitStencil::endPolyDrawing()
{
	// The eighth bit will be written with a 0 during next render (veget,...)
	Driver->stencilOp(UDriver::keep, UDriver::keep, UDriver::zero);
}


//-----------------------------------------------------------------------------------------------------------
//---------------------------------- CLandscapePolyDrawer ---------------------------------------------------
//-----------------------------------------------------------------------------------------------------------

CLandscapePolyDrawer::CLandscapePolyDrawer()
{
	// set ILandscapePolyDrawingCallback attribute of current scene to modify stencil operation
	// from NEL classes with callbacks.
	_InitStencil = new CInitStencil();
	_InitStencil->init();

	// shadow rectangle
	_Shadow.V0 = CVector(0.0, 0.0, 0.0);
	_Shadow.V1 = CVector(1.0, 0.0, 0.0);
	_Shadow.V2 = CVector(1.0, 1.0, 0.0);
	_Shadow.V3 = CVector(0.0, 1.0, 0.0);
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::initLandscapePolyDrawingCallback()
{
	// set ILandscapePolyDrawingCallback attribute of current scene to modify stencil operation
	// from NEL classes with callbacks.
	nlassert(_InitStencil);
	_InitStencil->init();
}

//-----------------------------------------------------------------------------------------------------------

CLandscapePolyDrawer::~CLandscapePolyDrawer()
{
	delete _InitStencil;
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::addPoly(const NLMISC::CPolygon2D &poly,
								   const NLMISC::CRGBA & color, const NLMISC::CAABBox & bBox)
{
	nlassert(!poly.Vertices.empty());
	_Polygons.push_back(poly);
	_PolyColor.push_back(color);
	_BBoxes.push_back(bBox);

	buildShadowVolume((uint)_Polygons.size());
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::beginRenderLandscapePolyPart()
{
	// activation of stencil test
	Driver->enableStencilTest(true);

	Driver->stencilFunc(UDriver::always, 128, 0xff);

	// the eighth bit will be written with a 0 during next render to mark stencil buffer
	// parts which will support Shadow Volume algorithm (landscape).
	// When stencil operation will be modify with "replace" operation, the eighth bit will be written
	// with a 1 (vegeation, shadow...)
	Driver->stencilOp(UDriver::keep, UDriver::keep, UDriver::zero);
}

//-----------------------------------------------------------------------------------------------------------

inline void createFace(CVector * face, const vector<CVector> & vertices, uint v1,
					   uint v2, uint v3, uint v4)
{
	face[0] = vertices[v1];
	face[1] = vertices[v2];
	face[2] = vertices[v3];
	face[3] = vertices[v4];
}

void CLandscapePolyDrawer::setScissor(uint polyId)
{
	// vector of bounding box vertices
	const CAABBox & bBox = _BBoxes[polyId];
	const CVector& center = bBox.getCenter();
	const CVector& halfSize = bBox.getHalfSize();

	vector<CVector> vertices(8);
	vertices[0] = CVector(center.x-halfSize.x, center.y+halfSize.y, center.z+halfSize.z);
	vertices[1] = CVector(center.x+halfSize.x, center.y+halfSize.y, center.z+halfSize.z);
	vertices[2] = CVector(center.x+halfSize.x, center.y-halfSize.y, center.z+halfSize.z);
	vertices[3] = CVector(center.x-halfSize.x, center.y-halfSize.y, center.z+halfSize.z);
	vertices[4] = CVector(center.x-halfSize.x, center.y+halfSize.y, center.z-halfSize.z);
	vertices[5] = CVector(center.x+halfSize.x, center.y+halfSize.y, center.z-halfSize.z);
	vertices[6] = CVector(center.x+halfSize.x, center.y-halfSize.y, center.z-halfSize.z);
	vertices[7] = CVector(center.x-halfSize.x, center.y-halfSize.y, center.z-halfSize.z);

	// transform each point to obtain vertices coordinates in camera location
	CMatrix	transformMatrix = Scene->getCam().getMatrix();
	transformMatrix.invert();
	for(uint i=0; i<8; i++)
	{
		CVector & point = vertices[i];
		point = transformMatrix.mulPoint(point);
	}

	// clip bounding box faces
	CVector faces[6][4];
	createFace(faces[0], vertices, 3, 2, 1, 0);
	createFace(faces[1], vertices, 7, 6, 5, 4);
	createFace(faces[2], vertices, 0, 1, 5, 4);
	createFace(faces[3], vertices, 7, 6, 2, 3);
	createFace(faces[4], vertices, 2, 6, 5, 1);
	createFace(faces[5], vertices, 7, 3, 0, 4);

	// "near" plane of current frustum
	CPlane nearPlane;
	nearPlane.make(CVector(0, 1, 0), CVector(0, Driver->getFrustum().Near, 0));
	CVector out[10];

	_FiniteFrustum = Driver->getFrustum();
	CVector2f rectMax(0,0), rectMin(0,0);

	// for each face, we research its intersection points with "near" plane (if exist)
	for(uint f=0; f<6; f++)
	{
		uint outNb = nearPlane.clipPolygonFront(faces[f], out, 4);

		// if intersection points exist, we project them in windows coordinates to recover
		// scissor rectangle
		if(outNb!=0)
		{
			for(uint v=0; v<outNb; v++)
			{
				CVector & point = out[v];
				//project
				point = _FiniteFrustum.project(point);
				//clamp
				NLMISC::clamp(point.x, 0, 1);
				NLMISC::clamp(point.y, 0, 1);

				// search for bounding rectangle of scissor test
				if(f==0 && v==0)
				{
					rectMax = rectMin = point;
				}
				else
				{
					rectMax.x = std::max(rectMax.x, point.x);
					rectMax.y = std::max(rectMax.y, point.y);
					rectMin.x = std::min(rectMin.x, point.x);
					rectMin.y = std::min(rectMin.y, point.y);
				}
			}
		}
	}

	CScissor scissor(rectMin.x, rectMin.y, rectMax.x - rectMin.x, rectMax.y - rectMin.y);
	Driver->setScissor(scissor);
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::renderLandscapePolyPart()
{
	uint i;

	UCamera cam = Scene->getCam();
	Driver->setMatrixMode3D(cam);

	CViewport oldViewPort = Driver->getViewport();

	Driver->setViewport(Scene->getViewport());

	// get initial values of depth range
	float depthRangeMin;
	Driver->getDepthRange(depthRangeMin, _MaxDepthRange);

	// get original (finite) frustum
	_FiniteFrustum = Driver->getFrustum();

	// we can write in stencil buffer only the last 7 bits.
	// The eighth bit mustn't be written because it indicates if a part of stencil buffer
	// can support shadow volume algorithm.
	Driver->stencilMask(0x7f);

	for(i=1; i<=_Polygons.size(); i++)
	{
		// calculate and set scissor rectangle to optimize algorithm.
		setScissor(i-1);

		// disable color buffer update
		Driver->setColorMask(false, false, false, false);
		Scene->enableLightingSystem(false);

		// disable Z buffer update
		GenericMat.setZWrite(false);

		// ZFAIL algorithm
		{
			// test and write only if eighth bit is a 1 (landscape).
			Driver->stencilFunc(UDriver::notequal, 0, 0x80);

			// display of shadow volumes in two passes

			GenericMat.setDoubleSided(false);

			// render the shadow volume back faces and increment the stencil buffer
			// when the depth test fails
			Driver->setCullMode(UDriver::CW);
			Driver->stencilOp(UDriver::keep, UDriver::incr, UDriver::keep);
			drawShadowVolume(i, true);

			// render the shadow volume front faces and decrement the stencil buffer
			// when the depth test fails
			Driver->setCullMode(UDriver::CCW);
			Driver->stencilOp(UDriver::keep, UDriver::decr, UDriver::keep);
			drawShadowVolume(i, false);

			GenericMat.setDoubleSided(true);
		}

		// enable color and Z Buffer updates
		Driver->setColorMask(true, true, true, true);
		GenericMat.setZWrite(true);

		// enable light
		Scene->enableLightingSystem(true);

		// render of the polygon : render of a rectangle whith polygon color.
		// It's renderer only if stencil buffer is different to 0 on seven last bits
		Driver->stencilFunc(UDriver::notequal, 0, 0x7f);

		// if stencil buffer is different to 0 (polygon), value must be
		// replace by 0 (seven last bits).
		Driver->stencilOp(UDriver::keep, UDriver::keep, UDriver::replace);
		drawPolygon(i);

		// reset neutral values
		Driver->stencilOp(UDriver::keep, UDriver::keep, UDriver::keep);
		Driver->setScissor(CScissor(0, 0, 1, 1));
	}

	// render decals on landscape only
	Driver->stencilFunc(UDriver::equal, 0x80, 0x80);

	// call to render the decals just before the projected polygons on landscape
	CDecalRenderList::getInstance().renderAllDecals();

	// disable stencil test
	Driver->enableStencilTest(false);
	Driver->stencilMask(0xff);
	Driver->stencilFunc(UDriver::always, 0, 0xff);

	// reset viewport
	Driver->setViewport(oldViewPort);
}

//-----------------------------------------------------------------------------------------------------------

void
CLandscapePolyDrawer::infiniteFrustum()
{
	// setup new values of depth range
	Driver->setDepthRange(0.0, 1.0);

	// znear and zfar initial values
	float znear, zfar;
	znear = _FiniteFrustum.Near;
	zfar = _FiniteFrustum.Far;

	// initial projection matrix
	CMatrix frustumMatrix = Driver->getFrustumMatrix();

	// epsilon depends from OpenGL or Direct3D use
	double factor = Driver->getClipSpaceZMin() == -1.f ? 2 : 1;
	double epsilon = factor*(1 - _MaxDepthRange*(zfar/(zfar - znear)));

	// update of coefficients to obtain "robust" infinite frustum,
	// and join depth range used in main scen display
	// OpenGL
	if(factor == 2)
	{
		frustumMatrix.setCoefficient((float)(epsilon-1), 2, 2);
		frustumMatrix.setCoefficient((float)(znear*(epsilon-2)), 2, 3);
	}
	// Direct3D
	else
	{
		frustumMatrix.setCoefficient((float)(1-epsilon), 2, 2);
		frustumMatrix.setCoefficient((float)(znear*(epsilon-1)), 3, 2);
	}

	// initial projection matrix
	Driver->setFrustumMatrix(frustumMatrix);
}

//-----------------------------------------------------------------------------------------------------------

void
CLandscapePolyDrawer::finiteFrustum()
{
	// depth range
	Driver->setDepthRange(0.0, _MaxDepthRange);

	// projection matrix
	Driver->setFrustum(_FiniteFrustum);
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::buildShadowVolume(uint poly)
{
	uint i, v1, v2, v3, v4;
	CVertexBuffer vb;
	CIndexBuffer  ib;
	CVector2f barycenter(0, 0);
	const CPolygon2D & polygon = _Polygons[poly-1];
	uint verticesNb = (uint)polygon.Vertices.size();

	// barycenter polygon
	for(i=0; i<verticesNb; i++)
	{
		barycenter += polygon.Vertices[i];
	}
	barycenter = barycenter/((float)verticesNb);
	_Barycenters.push_back(barycenter);

	// vertex buffer initialization.
	// vertices coordinates aren't calculated immediately but in drawShadowVolume method,
	// because they are calculated in camera location.
	vb.setVertexFormat(CVertexBuffer::PositionFlag);
	vb.setNumVertices(2*verticesNb + 2);
	vb.setPreferredMemory(CVertexBuffer::RAMVolatile, false);
	{
		CVertexBufferReadWrite vba;
		vb.lock(vba);

		for(i=0; i<2*verticesNb + 2; i++)
		{
			vba.setVertexCoord(i, CVector(0.0, 0.0, 0.0));
		}
	}
	_PolyVB.push_back(vb);

	// index buffer initialization.
	int index = 0;
	ib.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	ib.setNumIndexes(12*verticesNb);
	ib.setPreferredMemory(CIndexBuffer::RAMVolatile, false);
	{
		CIndexBufferReadWrite iba;
		ib.lock (iba);

		// near cap
		for(v1=1; v1<=verticesNb; v1++)
		{
			v2 = v1+1;
			if(v2 == verticesNb+1)
			{
				v2 = 1;
			}

			iba.setTri(index, v1, v2, 0);
			index += 3;
		}

		// far cap
		for(v1=verticesNb+2; v1<=2*verticesNb+1; v1++)
		{
			v2 = v1+1;
			if(v2 == 2*verticesNb+2)
			{
				v2 = verticesNb+2;
			}

			iba.setTri(index, v2, v1, verticesNb+1);
			index += 3;
		}

		// surrounding faces
		for(v1=1; v1<=verticesNb; v1++)
		{
			v3 = v1+verticesNb+1;

			v2 = v1+1;
			if(v2 == verticesNb+1)
			{
				v2 = 1;
			}

			v4 = v3+1;
			if(v4 == 2*verticesNb+2)
			{
				v4 = verticesNb+2;
			}

			// first triangle
			iba.setTri(index, v2, v1, v3);
			index += 3;

			// second triangle
			iba.setTri(index, v2, v3, v4);
			index += 3;
		}
	}

	_PolyIB.push_back(ib);
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::drawShadowVolume(uint poly, bool firstPass)
{
	if(firstPass)
	{
		// render shadow volume in camera location, in order to minimize
		// polygon coordinates
		CVertexBuffer & vb = _PolyVB[poly-1];
		const CPolygon2D & polygon = _Polygons[poly-1];
		const CVector2f & barycenter = _Barycenters[poly-1];
		uint verticesNb = (uint)polygon.Vertices.size();

		uint i;
		CVector2f vertex;
		CVector * vertexVB = NULL;
		const CVector cameraPos = Scene->getCam().getPos();

		float height = 2000.0;
		{
			CVertexBufferReadWrite vba;
			vb.lock(vba);

			// top vertices
			vertexVB = vba.getVertexCoordPointer(0);
			*vertexVB = CVector(barycenter.x, barycenter.y, height) - cameraPos;

			for(i=0; i<verticesNb; i++)
			{
				vertex = polygon.Vertices[i];
				vertexVB = vba.getVertexCoordPointer(i+1);
				*vertexVB = CVector(vertex.x, vertex.y, height) - cameraPos;
			}

			// bottom vertices
			vertexVB = vba.getVertexCoordPointer(verticesNb+1);
			*vertexVB = CVector(barycenter.x, barycenter.y, -height) - cameraPos;

			for(i=0; i<verticesNb; i++)
			{
				vertex = polygon.Vertices[i];
				vertexVB = vba.getVertexCoordPointer(i+verticesNb+2);
				*vertexVB = CVector(vertex.x, vertex.y, -height) - cameraPos;
			}
		}

		// new matrix model/view to reposition in camera position
		Driver->setModelMatrix(CMatrix::Identity);

		CMatrix viewMatrix = Driver->getViewMatrix();
		_OldViewMatrix = viewMatrix;
		viewMatrix.setPos(CVector::Null);
		Driver->setViewMatrix(viewMatrix);

		// before drawing shadow volume, we setup an infinite frustum
		// and a [0, 1] depth range
		infiniteFrustum();
	}

	// render Vertex Buffer
	((CDriverUser*)Driver)->getDriver()->activeVertexBuffer(_PolyVB[poly-1]);
	((CDriverUser*)Driver)->getDriver()->activeIndexBuffer(_PolyIB[poly-1]);
	((CDriverUser*)Driver)->getDriver()->renderTriangles(
		*GenericMat.getObjectPtr(), 0, 4*(uint32)_Polygons[poly-1].Vertices.size());

	if(!firstPass)
	{
		// we recover initial (finite) projection matrix
		finiteFrustum();

		// setup old model/view matrix
		Driver->setViewMatrix(_OldViewMatrix);
	}
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::drawPolygon(uint poly)
{
	GenericMat.setZWrite(false);

	const CRGBA & currentColor = GenericMat.getColor();

	// activation of transparency
	GenericMat.setBlend(true);
	GenericMat.setBlendFunc(UMaterial::srcalpha, UMaterial::invsrcalpha);
	GenericMat.getObjectPtr()->setOpacity(128);
	GenericMat.getObjectPtr()->setColor(_PolyColor[poly-1]);

	// draw a half transparent rectangle which covers all scissor rectangle
	Driver->setMatrixMode2D11();
	Driver->drawQuad(_Shadow, GenericMat);
	UCamera cam = Scene->getCam();
	Driver->setMatrixMode3D(cam);

	// reset old material of scene
	GenericMat.setBlend(false);
	GenericMat.setColor(currentColor);

	GenericMat.setZWrite(true);
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::deletePolygons()
{
	uint i;

	_Polygons.clear();
	_Barycenters.clear();
	_PolyColor.clear();
	_BBoxes.clear();

	for(i=0; i<_PolyVB.size(); i++)
	{
		_PolyVB[i].deleteAllVertices();
	}
	_PolyVB.clear();

	for(i=0; i<_PolyIB.size(); i++)
	{
		_PolyIB[i].deleteAllIndexes();
	}
	_PolyIB.clear();
}

//-----------------------------------------------------------------------------------------------------------

void CLandscapePolyDrawer::computeBBoxFromPolygon(const NLMISC::CPolygon2D &poly2D,
												  NLMISC::CAABBox &destBBox)
{
	if (poly2D.Vertices.empty())
	{
		destBBox.setMinMax(CVector::Null, CVector::Null);
		return;
	}

	// search for min and max dimensions of polygon on x and y axes.
	CVector2f point = poly2D.Vertices[0];
	CVector2f rectMax(point), rectMin(point);
	for(uint i=1; i<poly2D.Vertices.size(); i++)
	{
		point = poly2D.Vertices[i];

		rectMax.x = std::max(rectMax.x, point.x);
		rectMax.y = std::max(rectMax.y, point.y);
		rectMin.x = std::min(rectMin.x, point.x);
		rectMin.y = std::min(rectMin.y, point.y);
	}

	// init bounding box
	float w = rectMax.x - rectMin.x, h = rectMax.y - rectMin.y;
	destBBox.setCenter(CVector(rectMin.x + w/2 , rectMin.y + h/2, 0));
	destBBox.setHalfSize(CVector(w/2 , h/2, 0));

	// search for zones list on which the polygon will be "projected".
	uint zoneDim = 160;
	sint32 xmin = ((uint)(rectMin.x/zoneDim))*zoneDim;
	sint32 xmax = ((uint)(rectMax.x/zoneDim))*zoneDim;
	sint32 ymin = (((uint)(rectMin.y/zoneDim)))*zoneDim;
	sint32 ymax = (((uint)(rectMax.y/zoneDim)))*zoneDim;

	std::list<uint16> zoneIds;
	for(sint32 x=xmin; x<=xmax; x+=zoneDim)
	{
		for(sint32 y=ymin; y<=ymax; y+=zoneDim)
		{
			float xcount = (float)(x/zoneDim);
			float ycount = (float)(-y/zoneDim) + 1;

			uint16 zoneId = (uint16) ((ycount-1)*256+xcount);
			zoneIds.push_back(zoneId);
		}
	}

	// search for min and max dimensions on z axis.
	bool firstExtend = true;
	std::list<uint16>::iterator it;
	for(it=zoneIds.begin(); it!=zoneIds.end(); it++)
	{
		if (Landscape)
		{
			const CZone* zone =	Landscape->getZone(*it);
			if(zone)
			{
				// For each zone, we traverse its patchs and check intersection between our polygon
				// bounding rectangle on (x, y) plane and the bounding sphere patch .
				// If intersection, we use the bounding box of the patch to extend the polygon boudning box
				// on z axis.
				// NB : The patch bounding box isn't used immediately because it's more expensive
				// to obtain than bounding sphere.
				sint numPatchs = zone->getNumPatchs();
				for(sint i=0; i<numPatchs; i++)
				{
					const CBSphere& bSphere = zone->getPatchBSphere(i);

					const CVector2f sphereMax = CVector2f(bSphere.Center.x + bSphere.Radius,
						bSphere.Center.y + bSphere.Radius);
					const CVector2f sphereMin = CVector2f(bSphere.Center.x - bSphere.Radius,
						bSphere.Center.y - bSphere.Radius);

					// intersection beetween patch bounding sphere and polygon bounding rectangle
					if(sphereMin.x<rectMax.x && sphereMax.x>rectMin.x && sphereMin.y<rectMax.y
						&& sphereMax.y>rectMin.y)
					{
						const CPatch* patch = zone->getPatch(i);
						const CAABBox &	patchBBox = patch->buildBBox();
						const CVector& center = patchBBox.getCenter();
						const CVector& halfSize = patchBBox.getHalfSize();

						const CVector2f patchMax = CVector2f(center.x + halfSize.x, center.y + halfSize.y);
						const CVector2f patchMin = CVector2f(center.x - halfSize.x, center.y - halfSize.y);

						// intersection beetween patch bounding box and polygon bounding rectangle
						if(patchMin.x<rectMax.x && patchMax.x>rectMin.x && patchMin.y<rectMax.y
							&& patchMax.y>rectMin.y)
						{
							if(firstExtend)
							{
								destBBox.setCenter(CVector(destBBox.getCenter().x, destBBox.getCenter().y,
									center.z));
								destBBox.setHalfSize(CVector(destBBox.getHalfSize().x, destBBox.getHalfSize().y,
									halfSize.z));
								firstExtend = false;
							}
							else
							{
								float zmin=destBBox.getCenter().z-destBBox.getHalfSize().z;
								float zmax=destBBox.getCenter().z+destBBox.getHalfSize().z;

								if(center.z-halfSize.z < zmin)
									zmin = center.z-halfSize.z;
								if(center.z+halfSize.z > zmax)
									zmax = center.z+halfSize.z;

								destBBox.setCenter(CVector(destBBox.getCenter().x, destBBox.getCenter().y,
									zmin + (zmax-zmin)/2));
								destBBox.setHalfSize(CVector(destBBox.getHalfSize().x, destBBox.getHalfSize().y,
									(zmax-zmin)/2));
							}
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------

