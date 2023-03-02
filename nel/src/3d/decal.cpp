/** \file decal.cpp
 * 
 *
 * $Id$
 */

/* Copyright, 2007 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "std3d.h"
#include "nel/3d/decal.h"

#include "nel/3d/shadow_map.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/texture_user.h"

#include "nel/3d/dru.h"

#include <iostream>

using namespace std;
using namespace NL3D;
using namespace NLMISC;






// ***************************************************************************
CDecalContext::CDecalContext() : Clipping(true), DestTris(NULL){}


// ***************************************************************************
CDecal::CDecal() : 
		_VBInitialized(false),
		_Touched(true),
		_UV1(CUV(0,0)),
		_UV2(CUV(1,1))
{
	////////////
	const float hg = 5.f;

	_Vertices.push_back(	CVector( hg,  hg, 0.f) );
	_Vertices.push_back(	CVector(-hg,  hg, 0.f) );
	_Vertices.push_back(	CVector(-hg, -hg, 0.f) );
	_Vertices.push_back(	CVector( hg, -hg, 0.f) );
	_Vertices.push_back(	CVector( hg, 0.f,  hg) );
	_Vertices.push_back(	CVector(-hg, 0.f,  hg) );
	_Vertices.push_back(	CVector(-hg, 0.f, -hg) );
	_Vertices.push_back(	CVector( hg, 0.f, -hg) );
	////////////

	setOpacity(true);
	setTransparency(false);
	setIsRenderable(true);
	setIsLightable(true);

	/////////////////////
	// vertex buffer
	////////////////////	
/*	_Vb.setPreferredMemory( CVertexBuffer::AGPPreferred, false );
	_Vb.setVertexFormat( CVertexBuffer::PositionFlag );
	_Vb.setNumVertices( 4 );

	CVertexBufferReadWrite vbAccess;
	_Vb.lock( vbAccess );
		vbAccess.setVertexCoord( 0, CVector( 0.f, 0.f,  0.f) );
		vbAccess.setVertexCoord( 1, CVector( 10.f, 0.f,  0.f) );
		vbAccess.setVertexCoord( 2, CVector( 10.f, 0.f,  10.f) );
		vbAccess.setVertexCoord( 3, CVector( 0.f, 0.f,  10.f) );
	
*/

	///////////////////////
	// material
	//////////////////////
	
	///set texture environment
	_Mat.setShader(CMaterial::TShader::Normal);///fixed pipeline

	_Mat.texConstantColor(0, CRGBA::White);
	
	_Mat.texEnvOpRGB(0, CMaterial::Replace);
	_Mat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);///out=tex
	_Mat.texEnvArg1RGB(0, CMaterial::Constant, CMaterial::SrcColor);

	_Mat.texEnvOpAlpha(0, CMaterial::Replace);
	_Mat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);///out=tex
	_Mat.texEnvArg1Alpha(0, CMaterial::Constant, CMaterial::SrcAlpha);


	_Mat.setBlend(true);
	_Mat.setSrcBlend(CMaterial::srcalpha);
	_Mat.setDstBlend(CMaterial::invsrcalpha);
	_Mat.setZWrite(false);
	_Mat.setDoubleSided(true);
	
	CTextureFile *tex = new CTextureFile("tex_decal.tga");
	tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapLinear);
	_Mat.enableUserTexMat(0, true);
	_Mat.enableUserTexMat(1, true);
	
	
	_Mat.setTexture(0, tex);
	_Mat.setUserTexMat(0, CMatrix::Identity);

	_Mat.setTexCoordGen(0, true);
	_Mat.setTexCoordGenMode(0, CMaterial::TexCoordGenObjectSpace);

	_Mat.setZBias(-0.06f);

	_Mat.getTexture(0)->setWrapS(ITexture::Clamp);
	_Mat.getTexture(0)->setWrapT(ITexture::Clamp);


}

// ***************************************************************************
void CDecal::initModel()
{
	_LastCamPos = getOwnerScene()->getCam()->getMatrix().getPos();


	_ShadowMap = new CShadowMap( &(getOwnerScene()->getRenderTrav().getShadowMapManager()) );
	
}

// ***************************************************************************
CDecal::~CDecal()
{
	delete _ShadowMap;
}

// ***************************************************************************
void CDecal::registerBasic()
{
	CScene::registerModel( DecalId, TransformId, CDecal::creator);
}

// ***************************************************************************
void CDecal::traverseRender()
{
	// NL_ALLOC_CONTEXT( RdrDecal )
	// No rendering Op. But delay the clip and render after landscape rendering
		

	///TODO christo pas beau le "Jean"
	static std::string texName = "Jean";
	getOwnerScene()->getRenderTrav().getDecalManager().addDecal(this, texName);

}

// ***************************************************************************
std::vector<CVector> &CDecal::getVertices(const bool useVertexProgram)
{
	///position of decal has changed ??
	const NLMISC::CVector &camPos = getOwnerScene()->getCam()->getMatrix().getPos();
	if ( (camPos - _LastCamPos).norm() >= 4.f )
	{
		_Touched = true;
	}
				
	if( _Touched ) ///is NOT touched, direct render
	{
		_LastCamPos = camPos;
		computeDecal(useVertexProgram);
	}
	
	return _Vertices;
}

// ***************************************************************************
void CDecal::computeDecal(const bool useVertexProgram)
{
	CScene *sc = getOwnerScene();
	
	/// Verify that coll manager exists
	CVisualCollisionManager *vcm;
	if( !(vcm=sc->getVisualCollisionManagerForShadow()) )
		nlassert("VisualCollisionManager does NOT exist, NO decal rendering");
	

	///Preparing shadow map clipping planes
	//CShadowMap sm( &(sc->getRenderTrav().getShadowMapManager()) );
	CDecalContext context;
	
	float decalSize = 1.0f;
	_ClipCorners[0] =  getWorldMatrix() * (CVector(0.f,    1.f,    0.f) * decalSize);
	_ClipCorners[1] =  getWorldMatrix() * (CVector(1.f,    1.f,    0.f) * decalSize);
	_ClipCorners[2] =  getWorldMatrix() * (CVector(1.f,    0.f,    0.f) * decalSize);
	_ClipCorners[3] =  getWorldMatrix() * (CVector(0.f,    0.f,    0.f) * decalSize);

	///Unit Cube for clipping
	context.WorldClipPlanes.resize(4);
	context.WorldBBox.setMinMax( getWorldMatrix() * (CVector(0.f,0.f,0.f) * decalSize), getWorldMatrix() * (CVector(1.f,1.f,1.f) * decalSize) );
	
	for(uint i=0; i<4; ++i)
	{
		context.WorldClipPlanes[i].make(
			_ClipCorners[i],
			_ClipCorners[(i + 1) & 3],
			_ClipCorners[i] + (_ClipCorners[(i + 1) & 3] - _ClipCorners[i]).norm() * CVector::K);///&3 = %4 = %2^(3-1)		
		
		context.WorldClipPlanes[i].invert();		
	}

			
	CMatrix refPosMatrix;
	refPosMatrix.identity();
	refPosMatrix.setPos( sc->getCam()->getMatrix().getPos() );
	
	CMatrix worldToUVMatrix = getReverseUVMatrix() * getWorldMatrix().inverted();
	//worldToUVMatrix = worldToUVMatrix * refPosMatrix;


	//CMatrix world = getWorldMatrix();
	//smp.setWorldSpaceTextMat(CMatrix::Identity);
	//_Mat.setUserTexMat(0, CMatrix::Identity);
	
	/*CShadowMapProjector smp;
	vcm->receiveShadowMap(sc->getDriver(), &sm, CVector(0,0,0), _Mat, smp);
	*/

	context.WorldMatrix = getWorldMatrix();
	
	_Vertices.clear();
	context.DestTris = &_Vertices;
	vcm->receiveDecal(context);
	
	

	sc->getDriver()->setupModelMatrix(getWorldMatrix());
		
	CDRU::drawWiredBox( (CVector::K * (-10.f)),
						CVector::I * decalSize,
						CVector::J * decalSize,
						(CVector::K * 20.f),
						CRGBA::White,
						*(sc->getDriver()) );

}

// ***************************************************************************
void CDecal::setUVCoord(const CUV uv1, const CUV uv2)
{
		_UV1 = uv1;
		_UV2 = uv2;
}

