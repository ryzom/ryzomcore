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

#include "nel/3d/shadow_map.h"
#include "nel/3d/texture_mem.h"
#include "nel/misc/common.h"
#include "nel/3d/driver.h"
#include "nel/3d/shadow_map_manager.h"


using namespace std;
using namespace NLMISC;

namespace NL3D {


// ***************************************************************************
CShadowMap::CShadowMap(CShadowMapManager *smm)
{
	nlassert(smm);
	_ShadowMapManager= smm;
	_TextSize= 0;
	LastGenerationFrame= 0;
	DistanceFade= 0;
	TemporalOutScreenFade= 0;
	TemporalInScreenFade= 0;
	_FadeAround= 0;
	_FinalFade= 1;
	// see doc why 1.
	InScreenFadeAccum= 1;
}


// ***************************************************************************
CShadowMap::~CShadowMap()
{
	resetTexture();
}

// ***************************************************************************
void			CShadowMap::initTexture(uint textSize)
{
	textSize= max(textSize, 2U);
	// if same size than setup, quit
	if(_TextSize==textSize)
		return;
	resetTexture();
	textSize= raiseToNextPowerOf2(textSize);
	_TextSize= textSize;

	// Allocate in the Manager.
	_Texture= _ShadowMapManager->allocateTexture(_TextSize);

	// Since our texture has changed, it is no more valid. reset counter.
	LastGenerationFrame= 0;
}


// ***************************************************************************
void			CShadowMap::resetTexture()
{
	// release the SmartPtr
	if(_Texture)
	{
		_ShadowMapManager->releaseTexture(_Texture);
		_Texture= NULL;
	}
	_TextSize= 0;
}


// ***************************************************************************
void			CShadowMap::buildClipInfoFromMatrix()
{
	CVector	vi= LocalProjectionMatrix.getI();
	CVector	vj= LocalProjectionMatrix.getJ();
	CVector	vk= LocalProjectionMatrix.getK();
	CVector	vp= LocalProjectionMatrix.getPos();

	// **** Compute the planes.
	LocalClipPlanes.resize(6);
	// The plane 0 is the back plane.
	LocalClipPlanes[0].make(-vj, vp);
	// Left and botom plane.
	LocalClipPlanes[1].make(-vi, vp);
	LocalClipPlanes[2].make(-vk, vp);
	// Right and top plane.
	LocalClipPlanes[3].make(vi, vp + vi);
	LocalClipPlanes[4].make(vk, vp + vk);
	// Front plane.
	LocalClipPlanes[5].make(vj, vp + vj);

	// **** Compute the AA bounding box.
	LocalBoundingBox.setHalfSize(CVector::Null);
	CVector		p0= vp;
	CVector		p1= p0 + vi + vj + vk;
	LocalBoundingBox.setCenter(p0);
	LocalBoundingBox.extend(p0 + vi);
	LocalBoundingBox.extend(p0 + vj);
	LocalBoundingBox.extend(p0 + vk);
	LocalBoundingBox.extend(p1 - vi);
	LocalBoundingBox.extend(p1 - vj);
	LocalBoundingBox.extend(p1 - vk);
	LocalBoundingBox.extend(p1);
}


// ***************************************************************************
void			CShadowMap::buildCasterCameraMatrix(const CVector &lightDir, const CMatrix &localPosMatrix, const CAABBox &bbShape, CMatrix &cameraMatrix)
{
	// compute the orthogonal LightSpace camera matrix. Remind that J look forward and K is up here.
	cameraMatrix.setArbitraryRotJ(lightDir);

	CAABBox		bbLocal;
	bbLocal= CAABBox::transformAABBox(cameraMatrix.inverted() * localPosMatrix, bbShape);

	// Enlarge for 1 pixel left and right.
	float	scaleSize= (float)getTextureSize();
	scaleSize= (scaleSize+2)/scaleSize;
	CVector		hs= bbLocal.getHalfSize();
	hs.x*= scaleSize;
	hs.z*= scaleSize;
	// TestUniform
	/*float	maxLen= max(hs.x, hs.y);
	maxLen= max(maxLen, hs.z);
	hs.x= hs.y= hs.z= maxLen;*/
	// EndTestUniform
	bbLocal.setHalfSize(hs);

	// setup the orthogonal camera Matrix so that it includes all the BBox
	cameraMatrix.translate(bbLocal.getMin());
	CVector	vi= cameraMatrix.getI() * bbLocal.getSize().x;
	CVector	vj= cameraMatrix.getJ() * bbLocal.getSize().y;
	CVector	vk= cameraMatrix.getK() * bbLocal.getSize().z;
	cameraMatrix.setRot(vi,vj,vk);
}


// ***************************************************************************
void			CShadowMap::buildProjectionInfos(const CMatrix &cameraMatrix, const CVector &backPoint, float shadowMaxDepth)
{
	// Modify the cameraMatrix to define the Aera of Shadow.
	CVector		projp= cameraMatrix.getPos();
	CVector		proji= cameraMatrix.getI();
	CVector		projj= cameraMatrix.getJ();
	CVector		projk= cameraMatrix.getK();
	// modify the J vector so that it gets the Wanted Len
	CVector	vj= projj.normed();
	projj= vj*shadowMaxDepth;
	// Must move Pos so that the IK plane include the backPoint
	projp+= (backPoint*vj-projp*vj) * vj;
	// set the matrix
	LocalProjectionMatrix.setRot(proji, projj, projk);
	LocalProjectionMatrix.setPos(projp);

	// compute The clipPlanes and bbox.
	buildClipInfoFromMatrix();
}


// ***************************************************************************
void			CShadowMap::processFades()
{
	clamp(DistanceFade, 0.f, 1.f);
	clamp(TemporalOutScreenFade, 0.f, 1.f);
	clamp(TemporalInScreenFade, 0.f, 1.f);

	_FadeAround= max(DistanceFade, TemporalOutScreenFade);
	_FinalFade= max(_FadeAround, TemporalInScreenFade);

	/* if the fadeAround is 1, then release the texture
		Don't take _FinalFade because this last may change too much cause of TemporalInScreenFade.
		While FadeAround is somewhat stable (as entities and the camera don't move too much),
		TemporalInScreenFade is dependent of camera rotation.

		=> _FinalFade allow to not draw too much shadows (CPU gain),
		while FadeAround allow in addition the capacity to not use too much texture memory
	*/
	if(getFadeAround()==1)
		resetTexture();
}


// ***************************************************************************
CShadowMapProjector::CShadowMapProjector()
{
	// Trans matrix from Nel basis (Z up) to UVW basis (V up)
	_XYZToUWVMatrix.setRot(CVector::I, CVector::K, CVector::J, true);
	// Trans Matrix so Y is now the U (for clamp map).
	_XYZToWUVMatrix.setRot(CVector::K, CVector::I, CVector::J, true);
}

// ***************************************************************************
void	CShadowMapProjector::setWorldSpaceTextMat(const CMatrix &ws)
{
	_WsTextMat= ws;
}

// ***************************************************************************
void	CShadowMapProjector::applyToMaterial(const CMatrix &receiverWorldMatrix, CMaterial &material)
{
	CMatrix		osTextMat;
	osTextMat.setMulMatrix(_WsTextMat, receiverWorldMatrix);

	/* Set the TextureMatrix for ShadowMap projection so that UVW= mat * XYZ.
		its osTextMat but must rotate so Z map to V
	*/
	material.setUserTexMat(0, _XYZToUWVMatrix * osTextMat);
	/* Set the TextureMatrix for ClampMap projection so that UVW= mat * XYZ.
		its osTextMat but must rotate so Y map to U
	*/
	material.setUserTexMat(1, _XYZToWUVMatrix * osTextMat);
}


} // NL3D
