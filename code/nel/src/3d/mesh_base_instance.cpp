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

#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/mesh_base.h"
#include "nel/3d/scene.h"
#include "nel/3d/animation.h"
#include "nel/misc/debug.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/async_texture_manager.h"

#include <limits>

using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
CMeshBaseInstance::CMeshBaseInstance()
{
	IAnimatable::resize(AnimValueLast);
	_AsyncTextureToLoadRefCount= 0;
	_AsyncTextureMode= false;
	_AsyncTextureReady= true;
	_AsyncTextureDirty= false;
	_AsyncTextureDistance= 0;
	_VPWindTreeFixed = false;

	// I am a CMeshBaseInstance!!
	CTransform::setIsMeshBaseInstance(true);
}

// ***************************************************************************
CMeshBaseInstance::~CMeshBaseInstance()
{
	// If AsyncTextureMode, must disable. This ensure that async loading stop, and that no ref still exist
	// in the AsyncTextureManager
	if(_AsyncTextureMode)
		enableAsyncTextureMode(false);
}


// ***************************************************************************
void		CMeshBaseInstance::registerBasic()
{
	CScene::registerModel(MeshBaseInstanceId, TransformShapeId, CMeshBaseInstance::creator);
}


// ***************************************************************************
void		CMeshBaseInstance::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	uint32 i;
	CTransformShape::registerToChannelMixer(chanMixer, prefix);

	// Add any materials.
	for (i = 0; i < _AnimatedMaterials.size(); i++)
	{
		// append material  matname.*
		_AnimatedMaterials[i].registerToChannelMixer(chanMixer, prefix + _AnimatedMaterials[i].getMaterialName() + ".");
	}

	// Add any morph
	for (i = 0; i < _AnimatedMorphFactor.size(); i++)
	{
		_AnimatedMorphFactor[i].registerToChannelMixer(chanMixer, prefix + _AnimatedMorphFactor[i].getName());
	}
}


// ***************************************************************************
ITrack*		CMeshBaseInstance::getDefaultTrack (uint valueId)
{
	// Pointer on the CMeshBase
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;

	// Switch the value
	switch (valueId)
	{
	case CTransform::PosValue:
		return pMesh->getDefaultPos();
	case CTransform::RotEulerValue:
		return pMesh->getDefaultRotEuler();
	case CTransform::RotQuatValue:
		return pMesh->getDefaultRotQuat();
	case CTransform::ScaleValue:
		return pMesh->getDefaultScale();
	case CTransform::PivotValue:
		return pMesh->getDefaultPivot();
	default:
		// Problem, new values ?
		nlstop;
	};
	return NULL;
}


// ***************************************************************************
uint32 CMeshBaseInstance::getNbLightMap()
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	return (uint32)pMesh->_LightInfos.size();
}

// ***************************************************************************
void CMeshBaseInstance::getLightMapName( uint32 nLightMapNb, std::string &LightMapName )
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	if( nLightMapNb >= pMesh->_LightInfos.size() )
		return;
	LightMapName = pMesh->_LightInfos[nLightMapNb].AnimatedLight;
}

// ***************************************************************************
uint32 CMeshBaseInstance::getNbBlendShape()
{
	return (uint32)_AnimatedMorphFactor.size();
}

// ***************************************************************************
void CMeshBaseInstance::getBlendShapeName (uint32 nBlendShapeNb, std::string &BlendShapeName )
{
	if (nBlendShapeNb >= _AnimatedMorphFactor.size())
		return;
	BlendShapeName = _AnimatedMorphFactor[nBlendShapeNb].getName();
}

// ***************************************************************************
void CMeshBaseInstance::setBlendShapeFactor (const std::string &BlendShapeName, float rFactor)
{
	for (uint32 i = 0; i < _AnimatedMorphFactor.size(); ++i)
		if (BlendShapeName == _AnimatedMorphFactor[i].getName())
		{
			_AnimatedMorphFactor[i].setFactor (rFactor);
		}
}

// ***************************************************************************
void CMeshBaseInstance::traverseHrc()
{
	CMeshBase			*mb = NLMISC::safe_cast<CMeshBase *>((IShape *) Shape);

	// if the base instance uses automatic animations, we must also setup the date of the channel mixer controlling this object
	if (mb->getAutoAnim())
	{
		// Unfreeze HRC for those models
		CTransform *node = this;
		while (node)
		{
			node->unfreezeHRC();
			node = node->hrcGetParent();
		}

		// setup the channel mixer date
		CChannelMixer *chanMix = getChannelMixer();
		if (chanMix)
		{
			const CAnimation *anim = chanMix->getSlotAnimation(0);
			/** We perform wrapping ourselves.
			  * We avoid using a playlist, to not create one more obj.
			  */
			if (anim)
			{
				// Animation offset are setuped before clipping, they will be used for detail too.
				float animLength = anim->getEndTime() - anim->getBeginTime();
				if (animLength > 0)
				{
					float currTime = (TAnimationTime) getOwnerScene()->getCurrentTime();
					float startTime = (uint) (currTime / animLength) * animLength;
					// Set the channel mixer date using the global date of the scene
					chanMix->setSlotTime(0, anim->getBeginTime() + currTime - startTime);
				}
				else
				{
					chanMix->setSlotTime(0, anim->getBeginTime());
				}

				/** Eval non detail animation
				  */
				chanMix->eval(false);
			}
		}
	}

	CTransformShape::traverseHrc();
}

// ***************************************************************************
void CMeshBaseInstance::traverseAnimDetail()
{
	CMeshBase			*mb = NLMISC::safe_cast<CMeshBase *>((IShape *) Shape);

	CTransformShape::traverseAnimDetail();


	// update animated materials.
	// test if animated materials must be updated.
	if(IAnimatable::isTouched(CMeshBaseInstance::OwnerBit))
	{
		// must test / update all AnimatedMaterials.
		for(uint i=0;i<_AnimatedMaterials.size();i++)
		{
			// This test and update the pointed material.
			_AnimatedMaterials[i].update();
		}

		IAnimatable::clearFlag(CMeshBaseInstance::OwnerBit);
	}

	// Lightmap automatic animation

	// Animated lightmap must have the same size than shape info lightmap.
	const uint count0 = (uint)_AnimatedLightmap.size();
	const uint count1 = (uint)mb->_LightInfos.size ();
	nlassert (count0 == count1);
	if (count0 == count1)
	{
		for ( uint i = 0; i < count0; ++i )
		{
			CMeshBase::CLightMapInfoList &groupInfo = mb->_LightInfos[i];
			std::list<CMeshBase::CLightMapInfoList::CMatStage>::iterator ite = groupInfo.StageList.begin ();
			while (ite != groupInfo.StageList.end ())
			{
				sint animatedLightmap = _AnimatedLightmap[i];
				if (animatedLightmap != -1)
				{
					CRGBA factor = getOwnerScene ()->getAnimatedLightFactor (animatedLightmap, groupInfo.LightGroup);
					Materials[ite->MatId].setLightMapFactor ( ite->StageId, factor );
				}
				else
				{
					CRGBA factor = getOwnerScene ()->getLightmapGroupColor (groupInfo.LightGroup);
					Materials[ite->MatId].setLightMapFactor ( ite->StageId, factor );
				}
				ite++;
			}
		}
	}
}


// ***************************************************************************
void CMeshBaseInstance::selectTextureSet(uint id)
{
	nlassert(Shape);
	CMeshBase *mb = NLMISC::safe_cast<CMeshBase *>((IShape *) Shape);
	const uint numMat = mb->getNbMaterial();
	nlassert(numMat == Materials.size());
	// see which material are selectable
	for(uint k = 0; k < numMat; ++k)
	{
		CMaterial &mat = mb->getMaterial(k);
		for(uint l = 0; l < IDRV_MAT_MAXTEXTURES; ++l)
		{
			if (mat.getTexture(uint8(l)) && mat.getTexture(uint8(l))->isSelectable())
			{
				// use a smartPtr so the textFile will be released if just used to set the name for AsyncTextures.
				CSmartPtr<ITexture>		texNSV= mat.getTexture(uint8(l))->buildNonSelectableVersion(id);

				// std case: just replace the texture.
				if(!_AsyncTextureMode)
				{
					Materials[k].setTexture(uint8(l), texNSV);
				}
				// Async case
				else
				{
					// If texture file, must copy the texture name
					if(AsyncTextures[k].IsTextureFile[l])
					{
						CTextureFile	*textFile= safe_cast<CTextureFile*>((ITexture*)texNSV);
						AsyncTextures[k].TextureNames[l]= textFile->getFileName();
					}
					// else replace the texture.
					else
						Materials[k].setTexture(uint8(l), texNSV);
				}
			}
		}
	}

	// Flag the instance as AsyncTextureDirty if in this mode
	if(_AsyncTextureMode)
	{
		setAsyncTextureDirty(true);
	}
}


// ***************************************************************************
void CMeshBaseInstance::initAnimatedLightIndex (const CScene &scene)
{
	/* Scan lightmaps used by the shape, and for each, bind the transform shape to an
	 * animated lightmap index from the scene. This index will be used at runtime to
	 * get quickly a lightmap factor. This index is not set in the CShape because
	 * the CShape can be used with several CScene.
	 */

	// For each lightmap in the shape
	CMeshBase *pMB = static_cast<CMeshBase*> (static_cast<IShape*> (Shape));
	const uint count = (uint)pMB->_LightInfos.size ();
	uint i;

	// Resize the index array
	_AnimatedLightmap.resize (count);

	for (i=0; i<count; i++)
	{
		// The light info
		CMeshBase::CLightMapInfoList &lightInfo = pMB->_LightInfos[i];

		// Get the lightmap info
		_AnimatedLightmap[i] = scene.getAnimatedLightNameToIndex (lightInfo.AnimatedLight);
	}

	// Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail (count != 0);
}


// ***************************************************************************
uint CMeshBaseInstance::getNumMaterial () const
{
	return (uint)Materials.size ();
}


// ***************************************************************************
const CMaterial	*CMeshBaseInstance::getMaterial (uint materialId) const
{
	return &(Materials[materialId]);
}


// ***************************************************************************
CMaterial	*CMeshBaseInstance::getMaterial (uint materialId)
{
	return &(Materials[materialId]);
}


// ***************************************************************************
bool		CMeshBaseInstance::fastIntersect(const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D)
{
	if(!Shape || !supportFastIntersect())
		return false;

	// Use the system geometry to test the intersection
	CMeshBase *pMB = static_cast<CMeshBase*> (static_cast<IShape*> (Shape));
	return pMB->getSystemGeometry().fastIntersect(getWorldMatrix(), p0, dir, dist2D, distZ, computeDist2D);
}


// ***************************************************************************
// ***************************************************************************
// Async texture loading
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshBaseInstance::enableAsyncTextureMode(bool enable)
{
	// if same, no-op.
	if(_AsyncTextureMode==enable)
		return;
	_AsyncTextureMode= enable;

	// if comes to async texture mode, must prepare AsyncTexturing
	if(_AsyncTextureMode)
	{
		_AsyncTextureReady= true;

		// For all TextureFiles in material
		for(uint i=0;i<Materials.size();i++)
		{
			for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
			{
				// test if really a CTextureFile
				CTextureFile	*text= dynamic_cast<CTextureFile*>(Materials[i].getTexture(uint8(stage)));
				if(text)
				{
					// Must setup the AsyncTextures
					AsyncTextures[i].IsTextureFile[stage]= true;
					AsyncTextures[i].TextureNames[stage]= text->getFileName();
					AsyncTextures[i].TextIds[stage]= std::numeric_limits<uint>::max();
					// Now, must copy the textureFile, to Avoid writing in CMeshBase TextureFile descriptor !!!
					CTextureFile *tf = new CTextureFile(*text);
					// setup a dummy texture => Instance won't block rendering because texture not yet ready
					tf->setFileName("blank.tga");
					Materials[i].setTexture(uint8(stage), tf);
				}
				else
				{
					AsyncTextures[i].IsTextureFile[stage]= false;
				}
			}
		}

		// For convenience, flag the instance as Dirty.
		setAsyncTextureDirty(true);
	}
	// else, AsyncTextureMode disabled
	else
	{
		// first, must stop and release all textures in the async manager.
		releaseCurrentAsyncTextures();
		nlassert(_AsyncTextureToLoadRefCount==0);
		// clear the array => ensure good work if enableAsyncTextureMode(true) is made later
		contReset(_CurrentAsyncTextures);

		// For all TextureFiles in material, copy User setup from AsyncTextures, to real fileName
		for(uint i=0;i<Materials.size();i++)
		{
			for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
			{
				// if an async texture file
				if(AsyncTextures[i].IsTextureFile[stage])
				{
					// copy the texture name into the texture file.
					CTextureFile	*text= safe_cast<CTextureFile*>(Materials[i].getTexture(uint8(stage)));
					text->setFileName(AsyncTextures[i].TextureNames[stage]);
					// clear string space
					AsyncTextures[i].TextureNames[stage].clear();
				}
			}
		}
	}
}


// ***************************************************************************
void			CMeshBaseInstance::startAsyncTextureLoading(const NLMISC::CVector &position)
{
	if(!getAsyncTextureMode())
		return;

	// If the async texutre manager is not setuped in the scene, abort.
	CAsyncTextureManager	*asyncTextMgr= getOwnerScene()->getAsyncTextureManager();
	if(!asyncTextMgr)
		return;

	uint	i;


	/* for all new texture names to load, add them to the manager
		NB: done first before release because of RefCount Management (in case of same texture name).
	*/
	for(i=0;i<AsyncTextures.size();i++)
	{
		for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
		{
			if(AsyncTextures[i].IsTextureFile[stage])
			{
				uint	id;
				id= asyncTextMgr->addTextureRef(AsyncTextures[i].TextureNames[stage], this, position);
				AsyncTextures[i].TextIds[stage]= id;
			}
		}
	}

	/* For all old textures (0 for the first time...), release them.
	*/
	releaseCurrentAsyncTextures();

	// OK! bkup the setup
	_CurrentAsyncTextures= AsyncTextures;

	// texture async is not ready.
	_AsyncTextureReady= false;
}

// ***************************************************************************
void			CMeshBaseInstance::releaseCurrentAsyncTextures()
{
	// If the async texutre manager is not setuped in the scene, abort.
	CAsyncTextureManager	*asyncTextMgr= getOwnerScene()->getAsyncTextureManager();
	if(!asyncTextMgr)
		return;

	// release all texture in the manager
	for(uint i=0;i<_CurrentAsyncTextures.size();i++)
	{
		for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
		{
			if(_CurrentAsyncTextures[i].IsTextureFile[stage])
			{
				asyncTextMgr->releaseTexture(_CurrentAsyncTextures[i].TextIds[stage], this);
			}
		}
	}
}

// ***************************************************************************
bool			CMeshBaseInstance::isAsyncTextureReady()
{
	// if ok, just quit
	if(_AsyncTextureReady)
		return true;

	// test if async loading ended
	if(_AsyncTextureToLoadRefCount==0)
	{
		// must copy all fileNames into the actual Texture Files. Those are the valid ones now!
		for(uint i=0;i<_CurrentAsyncTextures.size();i++)
		{
			for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
			{
				if(_CurrentAsyncTextures[i].IsTextureFile[stage])
				{
					// copy the texture name into the texture file.
					CTextureFile	*text= safe_cast<CTextureFile*>(Materials[i].getTexture(uint8(stage)));
					// Since the texture is really uploaded in the driver, the true driver Texture Id will
					// be bound to this texture.
					text->setFileName(_CurrentAsyncTextures[i].TextureNames[stage]);
					/* Since driver setup will only occurs when object become visible, it is a good idea to release
					   Old driver info, because it may points to old driver texture data (eg: old shared textureFile).
					   thus doing so release VRAM Texture Memory
					*/
					text->releaseDriverSetup();
				}
			}
		}

		// Ok, we are now ready.
		_AsyncTextureReady= true;
		return true;
	}
	else
		return false;
}


// ***************************************************************************
sint			CMeshBaseInstance::getAsyncTextureId(uint matId, uint stage) const
{
	if(matId>=_CurrentAsyncTextures.size())
		return -1;
	if(!_CurrentAsyncTextures[matId].isTextureFile(stage))
		return -1;
	return _CurrentAsyncTextures[matId].TextIds[stage];
}



} // NL3D
