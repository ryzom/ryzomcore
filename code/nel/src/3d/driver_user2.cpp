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

// ***************************************************************************
// THIS FILE IS DIVIDED IN TWO PARTS BECAUSE IT MAKES VISUAL CRASH.
// fatal error C1076: compiler limit : internal heap limit reached; use /Zm to specify a higher limit
// ***************************************************************************

#include "nel/3d/driver_user.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/text_context_user.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/dru.h"
#include "nel/3d/scene.h"
#include "nel/3d/texture_user.h"
#include "nel/3d/u_camera.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// Component Management.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
UScene			*CDriverUser::createScene(bool bSmallScene)
{
	CSceneUser *pSU = new CSceneUser(this, bSmallScene);

	// set the shape bank
	pSU->getScene().setShapeBank( &_ShapeBank._ShapeBank );
	// set the MeshSkin Vertex Streams
	pSU->getScene().getRenderTrav().setMeshSkinManager(&_MeshSkinManager);
	pSU->getScene().getRenderTrav().setShadowMeshSkinManager(&_ShadowMeshSkinManager);
	// set the AsyncTextureManager
	pSU->getScene().setAsyncTextureManager(&_AsyncTextureManager);
	// set the lodManager
	pSU->getScene().setLodCharacterManager(&_LodCharacterManager);
	return _Scenes.insert(pSU);
}
// ***************************************************************************
void			CDriverUser::deleteScene(UScene	*scene)
{

	_Scenes.erase((CSceneUser*)scene, "deleteScene(): Bad scene ptr");
}

// ***************************************************************************

void CDriverUser::beginDefaultRenderTarget(uint32 width, uint32 height)
{
	if (_MatRenderTarget.empty())
	{
		_MatRenderTarget.attach(&_MatRenderTargetInt);
		UMaterial &umat = _MatRenderTarget;
		CMaterial &mat = _MatRenderTargetInt;
		umat.initUnlit();
		umat.setColor(CRGBA::White);
		umat.setBlend(false);
		umat.setAlphaTest(false);
		mat.setBlendFunc(CMaterial::one, CMaterial::zero);
		mat.setZWrite(false);
		mat.setZFunc(CMaterial::always);
		mat.setDoubleSided(true);
			
		_RenderTargetQuad.V0 = CVector(0.f, 0.f, 0.5f);
		_RenderTargetQuad.V1 = CVector(1.f, 0.f, 0.5f);
		_RenderTargetQuad.V2 = CVector(1.f, 1.f, 0.5f);
		_RenderTargetQuad.V3 = CVector(0.f, 1.f, 0.5f);
		_RenderTargetQuad.Uv0 = CUV(0.f, 0.f);
		_RenderTargetQuad.Uv1 = CUV(1.f, 0.f);
		_RenderTargetQuad.Uv2 = CUV(1.f, 1.f);
		_RenderTargetQuad.Uv3 = CUV(0.f, 1.f);
	}

	nlassert(!_EffectRenderTarget);
	if (width == 0 || height == 0)
		getWindowSize(width, height);
	_EffectRenderTarget = getRenderTargetManager().getRenderTarget(width, height);
	setRenderTarget(*_EffectRenderTarget);
}

// ***************************************************************************

void CDriverUser::endDefaultRenderTarget(UScene *scene)
{
	nlassert(_EffectRenderTarget);

	CTextureUser texNull;
	setRenderTarget(texNull);

	_MatRenderTarget.getObjectPtr()->setTexture(0, _EffectRenderTarget->getITexture());

	UCamera	pCam;
	if (scene)
	{
		pCam = scene->getCam();
	}
	CViewport oldVp = getViewport();
	CViewport vp = CViewport();
	setViewport(vp);
	setMatrixMode2D11();
	bool fog = fogEnabled();
	enableFog(false);
	drawQuad(_RenderTargetQuad, _MatRenderTarget);
	enableFog(fog);
	setViewport(oldVp);
	if (scene)
	{
		setMatrixMode3D(pCam);
	}

	_MatRenderTarget.getObjectPtr()->setTexture(0, NULL);

	getRenderTargetManager().recycleRenderTarget(_EffectRenderTarget);
	_EffectRenderTarget = NULL;
}

// ***************************************************************************
UTextContext	*CDriverUser::createTextContext(const std::string fontFileName, const std::string fontExFileName)
{

	return _TextContexts.insert(new CTextContextUser(fontFileName, fontExFileName, this, &_FontManager));
}
// ***************************************************************************
void			CDriverUser::deleteTextContext(UTextContext	*textContext)
{

	_TextContexts.erase((CTextContextUser*)textContext, "deleteTextContext: Bad TextContext");
}
// ***************************************************************************
void			CDriverUser::setFontManagerMaxMemory(uint maxMem)
{

	_FontManager.setMaxMemory(maxMem);
}
// ***************************************************************************
std::string		CDriverUser::getFontManagerCacheInformation() const
{

	return _FontManager.getCacheInformation();
}

// ***************************************************************************
UTextureFile	*CDriverUser::createTextureFile(const std::string &file)
{

	CTextureFileUser	*text= new CTextureFileUser(file);
	_Textures.insert(text);
	return text;
}
// ***************************************************************************
void			CDriverUser::deleteTextureFile(UTextureFile *textfile)
{

	_Textures.erase(dynamic_cast<CTextureFileUser*>(textfile), "deleteTextureFile: Bad textfile");
}
// ***************************************************************************
UTextureMem		*CDriverUser::createTextureMem(uint width, uint height, CBitmap::TType texType)
{

	CTextureMemUser	*pTx= new CTextureMemUser(width, height, texType);
	_Textures.insert(pTx);
	return pTx;
}
// ***************************************************************************
void			CDriverUser::deleteTextureMem(UTextureMem *pTx)
{

	_Textures.erase(dynamic_cast<CTextureMemUser*>(pTx), "deleteTextureMem: Bad pTx");
}
// ***************************************************************************
UMaterial		CDriverUser::createMaterial()
{

	return UMaterial(new CMaterial);
}
// ***************************************************************************
void			CDriverUser::deleteMaterial(UMaterial &umat)
{
	delete umat.getObjectPtr();
	umat.detach();
}

// ***************************************************************************
UAnimationSet			*CDriverUser::createAnimationSet(bool headerOptim)
{
	return _AnimationSets.insert(new CAnimationSetUser(this, headerOptim));
}

// ***************************************************************************
UAnimationSet			*CDriverUser::createAnimationSet(const std::string &animationSetFile)
{
	H_AUTO( NL3D_Load_AnimationSet )

	NLMISC::CIFile	f;
	// throw exception if not found.
	std::string	path= CPath::lookup(animationSetFile);
	f.open(path);
	return _AnimationSets.insert(new CAnimationSetUser(this, f));
}

// ***************************************************************************
void			CDriverUser::deleteAnimationSet(UAnimationSet *animationSet)
{
	_AnimationSets.erase((CAnimationSetUser*)animationSet, "deleteAnimationSet(): Bad AnimationSet ptr");
}



// ***************************************************************************
// ***************************************************************************
// Profile.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDriverUser::profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut)
{

	_Driver->profileRenderedPrimitives(pIn, pOut);
}


// ***************************************************************************
uint32			CDriverUser::profileAllocatedTextureMemory()
{

	return _Driver->profileAllocatedTextureMemory();
}


// ***************************************************************************
uint32			CDriverUser::profileSetupedMaterials() const
{

	return _Driver->profileSetupedMaterials();
}


// ***************************************************************************
uint32			CDriverUser::profileSetupedModelMatrix() const
{
	return _Driver->profileSetupedModelMatrix();
}


// ***************************************************************************
void			CDriverUser::enableUsedTextureMemorySum (bool enable)
{
	_Driver->enableUsedTextureMemorySum (enable);
}


// ***************************************************************************
uint32			CDriverUser::getUsedTextureMemory () const
{
	return _Driver->getUsedTextureMemory ();
}


// ***************************************************************************
void			CDriverUser::startProfileVBHardLock()
{
	_Driver->startProfileVBHardLock();
}

// ***************************************************************************
void			CDriverUser::endProfileVBHardLock(std::vector<std::string> &result)
{
	_Driver->endProfileVBHardLock(result);
}

// ***************************************************************************
void			CDriverUser::profileVBHardAllocation(std::vector<std::string> &result)
{
	_Driver->profileVBHardAllocation(result);
}

// ***************************************************************************
void CDriverUser::startProfileIBLock()
{
	_Driver->startProfileIBLock();
}

// ***************************************************************************
void CDriverUser::endProfileIBLock(std::vector<std::string> &result)
{
	_Driver->endProfileIBLock(result);
}

// ***************************************************************************
void CDriverUser::profileIBAllocation(std::vector<std::string> &result)
{
	_Driver->profileIBAllocation(result);
}


// ***************************************************************************
void			CDriverUser::profileTextureUsage(std::vector<std::string> &result)
{
	_Driver->profileTextureUsage(result);
}



} // NL3D
