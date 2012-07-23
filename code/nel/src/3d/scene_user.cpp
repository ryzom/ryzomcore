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

#include "nel/3d/u_point_light.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_skeleton.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/coarse_mesh_manager.h"
#include "nel/3d/point_light_model.h"
#include "nel/3d/lod_character_manager.h"
#include "nel/3d/lod_character_shape.h"
#include "nel/3d/lod_character_shape_bank.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/async_texture_manager.h"
#include "nel/3d/water_env_map_user.h"


using namespace NLMISC;

namespace NL3D
{

H_AUTO_DECL( NL3D_UI_Scene )
H_AUTO_DECL( NL3D_Misc_Scene_CreateDel_Element )
H_AUTO_DECL( NL3D_CreateOrLoad_Instance )
H_AUTO_DECL( NL3D_CreateOrLoad_Skeleton )
H_AUTO_DECL( NL3D_Load_CLodOrCoarseMesh )
H_AUTO_DECL( NL3D_Load_AsyncIG )

#define	NL3D_HAUTO_UI_SCENE						H_AUTO_USE( NL3D_UI_Scene )
#define	NL3D_HAUTO_ELT_SCENE					H_AUTO_USE( NL3D_Misc_Scene_CreateDel_Element )
#define	NL3D_HAUTO_CREATE_INSTANCE				H_AUTO_USE( NL3D_CreateOrLoad_Instance )
#define	NL3D_HAUTO_CREATE_SKELETON				H_AUTO_USE( NL3D_CreateOrLoad_Skeleton )
#define	NL3D_HAUTO_LOAD_LOD						H_AUTO_USE( NL3D_Load_CLodOrCoarseMesh )
#define	NL3D_HAUTO_ASYNC_IG						H_AUTO_USE( NL3D_Load_AsyncIG )

// Render/Animate.
H_AUTO_DECL( NL3D_Render_Scene )
H_AUTO_DECL( NL3D_Render_Animate_Scene )
H_AUTO_DECL( NL3D_Render_Scene_Begin )
H_AUTO_DECL( NL3D_Render_Scene_Part )
H_AUTO_DECL( NL3D_Render_Scene_End )

#define	NL3D_HAUTO_RENDER_SCENE					H_AUTO_USE( NL3D_Render_Scene )
#define	NL3D_HAUTO_RENDER_SCENE_ANIMATE			H_AUTO_USE( NL3D_Render_Animate_Scene )
#define	NL3D_HAUTO_RENDER_SCENE_BEGIN			H_AUTO_USE( NL3D_Render_Scene_Begin )
#define	NL3D_HAUTO_RENDER_SCENE_PART			H_AUTO_USE( NL3D_Render_Scene_Part )
#define	NL3D_HAUTO_RENDER_SCENE_END				H_AUTO_USE( NL3D_Render_Scene_End )




// ***************************************************************************
void			CSceneUser::setAutomaticAnimationSet(UAnimationSet *as)
{
	NL3D_HAUTO_UI_SCENE;

	nlassert(as);
	as->build();
	CAnimationSetUser *asu = NLMISC::safe_cast<CAnimationSetUser *>(as);
	_Scene.setAutomaticAnimationSet(asu->_AnimationSet);
}

// ***************************************************************************
UPlayListManager			*CSceneUser::createPlayListManager()
{
	NL3D_HAUTO_ELT_SCENE;

	return _PlayListManagers.insert(new CPlayListManagerUser());
}
// ***************************************************************************
void			CSceneUser::deletePlayListManager(UPlayListManager	*playListManager)
{
	NL3D_HAUTO_ELT_SCENE;

	_PlayListManagers.erase((CPlayListManagerUser*)playListManager, "deletePlayListManager(): Bad PlayListManager ptr");
}

// ***************************************************************************

void			CSceneUser::setPolygonBalancingMode(CSceneUser::TPolygonBalancingMode polBalMode)
{
	NL3D_HAUTO_UI_SCENE;

	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	_Scene.setPolygonBalancingMode((CScene::TPolygonBalancingMode)(uint)(polBalMode));
}

// ***************************************************************************

CSceneUser::TPolygonBalancingMode	CSceneUser::getPolygonBalancingMode() const
{
	NL3D_HAUTO_UI_SCENE;

	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	return (CSceneUser::TPolygonBalancingMode)(uint)_Scene.getPolygonBalancingMode();
}


// ***************************************************************************
float			CSceneUser::getNbFaceAsked () const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getNbFaceAsked ();
}

// ***************************************************************************
void			CSceneUser::setGroupLoadMaxPolygon(const std::string &group, uint nFaces)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setGroupLoadMaxPolygon(group, nFaces);
}
// ***************************************************************************
uint			CSceneUser::getGroupLoadMaxPolygon(const std::string &group)
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getGroupLoadMaxPolygon(group);
}
// ***************************************************************************
float			CSceneUser::getGroupNbFaceAsked (const std::string &group) const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getGroupNbFaceAsked (group);
}


// ***************************************************************************

void CSceneUser::setCoarseMeshManagerTexture (const char *sPath)
{
	NL3D_HAUTO_LOAD_LOD;

	// Get the manager
	CCoarseMeshManager *manager=_Scene.getCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setTextureFile (sPath);
	}
}

// ***************************************************************************
void				CSceneUser::setCoarseMeshLightingUpdate(uint8 period)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setCoarseMeshLightingUpdate(period);
}

// ***************************************************************************
uint8				CSceneUser::getCoarseMeshLightingUpdate() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getCoarseMeshLightingUpdate();
}

// ***************************************************************************
void				CSceneUser::enableLightingSystem(bool enable)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.enableLightingSystem(enable);
}

// ***************************************************************************
void				CSceneUser::setAmbientGlobal(NLMISC::CRGBA ambient)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setAmbientGlobal(ambient);
}
void				CSceneUser::setSunAmbient(NLMISC::CRGBA ambient)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunAmbient(ambient);
}
void				CSceneUser::setSunDiffuse(NLMISC::CRGBA diffuse)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunDiffuse(diffuse);
}
void				CSceneUser::setSunSpecular(NLMISC::CRGBA specular)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunSpecular(specular);
}
void				CSceneUser::setSunDirection(const NLMISC::CVector &direction)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunDirection(direction);
}
void				CSceneUser::setLightGroupColor(uint lightmapGroup, NLMISC::CRGBA color)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setLightGroupColor(lightmapGroup, color);
}


// ***************************************************************************
NLMISC::CRGBA		CSceneUser::getAmbientGlobal() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getAmbientGlobal();
}
NLMISC::CRGBA		CSceneUser::getSunAmbient() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunAmbient();
}
NLMISC::CRGBA		CSceneUser::getSunDiffuse() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunDiffuse();
}
NLMISC::CRGBA		CSceneUser::getSunSpecular() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunSpecular();
}
NLMISC::CVector		CSceneUser::getSunDirection() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunDirection();
}

// ***************************************************************************

void				CSceneUser::setMaxLightContribution(uint nlights)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setMaxLightContribution(nlights);
}

// ***************************************************************************

uint				CSceneUser::getMaxLightContribution() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getMaxLightContribution();
}

// ***************************************************************************

void				CSceneUser::setLightTransitionThreshold(float lightTransitionThreshold)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setLightTransitionThreshold(lightTransitionThreshold);
}

// ***************************************************************************

float				CSceneUser::getLightTransitionThreshold() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getLightTransitionThreshold();
}

// ***************************************************************************

UPointLight			CSceneUser::createPointLight()
{
	NL3D_HAUTO_ELT_SCENE;

	CTransform	*model= _Scene.createModel(PointLightModelId);
	if (model)
	{
		CPointLightModel *pointLightModel= safe_cast<CPointLightModel*> (model);
		// If not found, return NULL.
		if(pointLightModel==NULL)
			return NULL;

		// The component is auto added/deleted to _Scene in ctor/dtor.
		return UPointLight (pointLightModel);
	}
	else
		return UPointLight ();
}

// ***************************************************************************
void			CSceneUser::deletePointLight(UPointLight &light)
{
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Scene.deleteModel (light.getObjectPtr());
	light.detach ();
}


// ***************************************************************************
void			CSceneUser::setGlobalWindPower(float gwp)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setGlobalWindPower(gwp);
}
// ***************************************************************************
float			CSceneUser::getGlobalWindPower() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getGlobalWindPower();
}
// ***************************************************************************
void			CSceneUser::setGlobalWindDirection(const CVector &gwd)
{
	_Scene.setGlobalWindDirection(gwd);
}
// ***************************************************************************
const CVector	&CSceneUser::getGlobalWindDirection() const
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getGlobalWindDirection();
}

// ***************************************************************************
void CSceneUser::updateWaitingIG()
{
	for(TWaitingIGList::iterator it = _WaitingIGs.begin(); it != _WaitingIGs.end();)
	{
		bool	erased= false;
		if (it->IGToLoad != NULL) // ig loaded ?
		{
			if (it->IGToLoad != (UInstanceGroup *) -1)
			{
				switch (it->IGToLoad->getAddToSceneState())
				{
					case UInstanceGroup::StateNotAdded:
						// start loading
						if (it->Callback != NULL)
							it->Callback->InstanceGroupCreated(it->IGToLoad);
						it->IGToLoad->addToSceneAsync(*this, _DriverUser, it->SelectedTexture);
					break;
					case UInstanceGroup::StateAdded:
						it->IGToLoad->setPos(it->Pos);
						it->IGToLoad->setRotQuat(it->Rot);
						this->setToGlobalInstanceGroup(it->IGToLoad);
						*it->CallerPtr = it->IGToLoad;
						// remove from list
						it = _WaitingIGs.erase(it);
						erased= true;
					break;
					case UInstanceGroup::StateError:
						delete it->IGToLoad;
						it->IGToLoad = (UInstanceGroup *) -1;
						*it->CallerPtr = it->IGToLoad;
						// remove from list
						it = _WaitingIGs.erase(it);
						erased= true;
					break;
					default:
					break;
				}
			}
			else
			{
				// loading failed
				*it->CallerPtr = it->IGToLoad;
				it = _WaitingIGs.erase(it);
				erased= true;
			}
		}
		// next IG.
		if(!erased)
			it++;
	}
}


// ***************************************************************************
void				CSceneUser::resetCLodManager()
{
	NL3D_HAUTO_UI_SCENE;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	_Scene.getLodCharacterManager()->reset();
}

// ***************************************************************************
uint32				CSceneUser::loadCLodShapeBank(const std::string &fileName)
{
	NL3D_HAUTO_LOAD_LOD;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	// Open the file
	CIFile	file(CPath::lookup(fileName));

	// create the shape bank
	uint32	bankId= _Scene.getLodCharacterManager()->createShapeBank();

	// get the bank
	CLodCharacterShapeBank	*bank= _Scene.getLodCharacterManager()->getShapeBank(bankId);
	nlassert(bank);

	// read the bank.
	file.serial(*bank);

	// recompile the shape Map.
	_Scene.getLodCharacterManager()->compile();

	return bankId;
}

// ***************************************************************************
void				CSceneUser::deleteCLodShapeBank(uint32 bankId)
{
	NL3D_HAUTO_LOAD_LOD;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	// delete the bank
	_Scene.getLodCharacterManager()->deleteShapeBank(bankId);

	// recompile the shape Map.
	_Scene.getLodCharacterManager()->compile();
}

// ***************************************************************************
sint32				CSceneUser::getCLodShapeIdByName(const std::string &name) const
{
	NL3D_HAUTO_UI_SCENE;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	return _Scene.getLodCharacterManager()->getShapeIdByName(name);
}

// ***************************************************************************
sint32				CSceneUser::getCLodAnimIdByName(uint32 shapeId, const std::string &name) const
{
	NL3D_HAUTO_UI_SCENE;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	const CLodCharacterShape	*shape= _Scene.getLodCharacterManager()->getShape(shapeId);
	if(shape)
		return shape->getAnimIdByName(name);
	else
		return -1;
}


// ***************************************************************************
void			CSceneUser::render(bool updateWaitingInstancesFlag /*= true*/, bool restoreMatrixContextAfterRender /*= true*/)
{

	// render the scene.
	{
		NL3D_HAUTO_RENDER_SCENE

		if(_Scene.getCam() == NULL)
			nlerror("render(): try to render with no camera linked (may have been deleted)");
		_Scene.render(true);
	}

	if (updateWaitingInstancesFlag) updateWaitingInstances();

	// Must restore the matrix context, so 2D/3D interface not disturbed.
	if (restoreMatrixContextAfterRender) _DriverUser->restoreMatrixContext();
}

// ***************************************************************************
void			CSceneUser::beginPartRender()
{

	// render the scene.
	{
		NL3D_HAUTO_RENDER_SCENE_BEGIN
		_Scene.beginPartRender();
	}
}

// ***************************************************************************
void			CSceneUser::renderPart(TRenderPart rp)
{

	// render the scene.
	{
		NL3D_HAUTO_RENDER_SCENE_PART

		if(_Scene.getCam() == NULL)
			nlerror("render(): try to render with no camera linked (may have been deleted)");
		_Scene.renderPart(rp, true);
	}
}

// ***************************************************************************
void			CSceneUser::endPartRender(bool updateWaitingInstancesFlag, bool restoreMatrixContextAfterRender)
{

	// render the scene.
	{
		NL3D_HAUTO_RENDER_SCENE_END
		_Scene.endPartRender();
	}

	if (updateWaitingInstancesFlag) updateWaitingInstances();

	// Must restore the matrix context, so 2D/3D interface not disturbed.
	if (restoreMatrixContextAfterRender) _DriverUser->restoreMatrixContext();
}


// ***************************************************************************
/*virtual*/ void CSceneUser::updateWaitingInstances(double ellapsedTime)
{
	_Scene.updateWaitingInstances(ellapsedTime);
	updateWaitingInstances();
}


// ***************************************************************************
void CSceneUser::updateWaitingInstances()
{
	// Update waiting instances
	{
		NL3D_HAUTO_ASYNC_IG

		// Done after the _Scene.render because in this method the instance are checked for creation
		std::map<UInstance*,CTransformShape*>::iterator it = _WaitingInstances.begin();
		while( it != _WaitingInstances.end() )
		{
			if( it->second != NULL )
			{
				it->first->attach (it->second);
				std::map<UInstance*,CTransformShape*>::iterator delIt = it;
				++it;
				_WaitingInstances.erase(delIt);
			}
			else
			{
				++it;
			}
		}
	}

	// update waiting instances groups;
	{
		NL3D_HAUTO_ASYNC_IG

		updateWaitingIG();
	}
}


void			CSceneUser::animate(TGlobalAnimationTime time)
{
	NL3D_HAUTO_RENDER_SCENE_ANIMATE;

	_Scene.animate(time);
}

// ***************************************************************************

void			CSceneUser::setCam(UCamera cam)
{
	NL3D_HAUTO_UI_SCENE;

	if(cam.empty())
		nlerror("setCam(): cannot set a NULL camera");
	CCamera *camera = cam.getObjectPtr();
	if( camera->getOwnerScene() != &_Scene)
		nlerror("setCam(): try to set a current camera not created from this scene");

	_Scene.setCam(camera);
}

// ***************************************************************************

UCamera			CSceneUser::getCam()
{
	NL3D_HAUTO_UI_SCENE;

	return UCamera (_Scene.getCam());
}

// ***************************************************************************

void			CSceneUser::setViewport(const class CViewport& viewport)
{
	NL3D_HAUTO_UI_SCENE;

	_Scene.setViewport(viewport);
}
CViewport		CSceneUser::getViewport()
{
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getViewport();
}

UInstanceGroup	*CSceneUser::findCameraClusterSystemFromRay(UInstanceGroup *startClusterSystem,
			const NLMISC::CVector &startPos, NLMISC::CVector &endPos)
{
	NL3D_HAUTO_UI_SCENE;

	CInstanceGroupUser	*uig= dynamic_cast<CInstanceGroupUser*>(startClusterSystem);
	CInstanceGroup		*pIg= NULL;
	if(uig)
		pIg= &uig->_InstanceGroup;

	CInstanceGroup *resultIg= _Scene.findCameraClusterSystemFromRay(pIg, startPos, endPos);
	if(resultIg)
		return resultIg->getUserInterface();
	else
		return NULL;
}

// ***************************************************************************

UCamera			CSceneUser::createCamera()
{
	NL3D_HAUTO_ELT_SCENE;

	CTransform	*model= _Scene.createModel(CameraId);
	if (model)
	{
		CCamera *object = NLMISC::safe_cast<CCamera*>(model);
		object->setFrustum(UCamera::DefLx, UCamera::DefLy, UCamera::DefLzNear, UCamera::DefLzFar);
		return UCamera (object);
	}
	else
		return UCamera ();
}

// ***************************************************************************

void			CSceneUser::deleteCamera(UCamera &cam)
{
	NL3D_HAUTO_ELT_SCENE;

	CCamera		*object = cam.getObjectPtr();

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Scene.deleteModel (object);
	cam.detach ();
}

// ***************************************************************************

UInstance		CSceneUser::createInstance(const std::string &shapeName)
{
	NL3D_HAUTO_CREATE_INSTANCE;

	CTransformShape	*model= _Scene.createInstance(shapeName);
	return UInstance (model);
}

// ***************************************************************************

void CSceneUser::createInstanceAsync(const std::string &shapeName, UInstance *ppInstance, const NLMISC::CVector &position, uint selectedTexture)
{
	NL3D_HAUTO_CREATE_INSTANCE;

	_WaitingInstances[ppInstance] = NULL;
	_Scene.createInstanceAsync(shapeName,&_WaitingInstances[ppInstance], position, selectedTexture);
//		CTransform	*model= _Scene.createInstance(shapeName);
	// If not found, return NULL.
//		if(model==NULL)
//			return NULL;

//		if( dynamic_cast<CMeshInstance*>(model)==NULL )
//			nlerror("UScene::createInstance(): shape is not a mesh");

	// The component is auto added/deleted to _Scene in ctor/dtor.
//		return dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, model)) );
}

// ***************************************************************************

void			CSceneUser::deleteInstance(UInstance &inst)
{
	NL3D_HAUTO_ELT_SCENE;

	_Scene.deleteInstance (inst.getObjectPtr());
	inst.detach ();
}

// ***************************************************************************

void CSceneUser::createInstanceGroupAndAddToSceneAsync (const std::string &instanceGroup, UInstanceGroup **pIG, const NLMISC::CVector &pos, const NLMISC::CQuat &rot,
														uint selectedTexture, IAsyncLoadCallback *pCB)
{
	NL3D_HAUTO_ASYNC_IG;

	_WaitingIGs.push_front(CWaitingIG(pIG, pos, rot, selectedTexture, pCB));
	UInstanceGroup::createInstanceGroupAsync(instanceGroup, &(_WaitingIGs.begin()->IGToLoad));
	// this list updat will be performed at each render, see updateWaitingIG
}

void CSceneUser::stopCreatingAndAddingIG(UInstanceGroup **pIG)
{
	NL3D_HAUTO_ASYNC_IG;

	for(TWaitingIGList::iterator it = _WaitingIGs.begin(); it != _WaitingIGs.end(); ++it)
	{
		CWaitingIG &rWIG = *it;
		if (rWIG.CallerPtr == pIG)
		{
			if (rWIG.IGToLoad == NULL)
			{
				UInstanceGroup::stopCreateInstanceGroupAsync(pIG);
			}
			// Ig must be initialized
			else if (rWIG.IGToLoad != (UInstanceGroup*)-1)
			{
				switch(rWIG.IGToLoad->getAddToSceneState())
				{
					case UInstanceGroup::StateAdding:
						rWIG.IGToLoad->stopAddToSceneAsync();
					break;
					case UInstanceGroup::StateAdded:
						rWIG.IGToLoad->removeFromScene(*this);
						delete rWIG.IGToLoad;
					break;
					case UInstanceGroup::StateNotAdded:
						delete rWIG.IGToLoad;
					break;
					default:
					break;
				}
			}
			_WaitingIGs.erase(it);
			return;
		}
	}
}

void CSceneUser::deleteInstanceGroup(UInstanceGroup *pIG)
{
	delete pIG;
}

// ***************************************************************************

UTransform CSceneUser::createTransform()
{
	NL3D_HAUTO_ELT_SCENE;

	CTransform	*model= _Scene.createModel(TransformId);
	return UTransform (model);
}

// ***************************************************************************

void			CSceneUser::deleteTransform(UTransform &tr)
{
	NL3D_HAUTO_ELT_SCENE;

	_Scene.deleteModel (tr.getObjectPtr());
	tr.detach ();
}

// ***************************************************************************

USkeleton		CSceneUser::createSkeleton(const std::string &shapeName)
{
	NL3D_HAUTO_CREATE_SKELETON;

	CTransformShape *ts = _Scene.createInstance(shapeName);
	if (ts)
	{
		CSkeletonModel	*model= safe_cast<CSkeletonModel*> (ts);
		return USkeleton (model);
	}
	else
	{
		return USkeleton ();
	}
}

// ***************************************************************************

void			CSceneUser::deleteSkeleton(USkeleton &skel)
{
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Scene.deleteInstance (skel.getObjectPtr());
	skel.detach ();
}

// ***************************************************************************

ULandscape		*CSceneUser::createLandscape()
{
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return _Landscapes.insert(new CLandscapeUser(&_Scene));
}
void			CSceneUser::deleteLandscape(ULandscape *land)
{
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Landscapes.erase((CLandscapeUser*) land);
}

UCloudScape *CSceneUser::createCloudScape()
{
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return _CloudScapes.insert(new CCloudScapeUser(&_Scene));
}
void CSceneUser::deleteCloudScape(UCloudScape *cs)
{

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_CloudScapes.erase((CCloudScapeUser*) cs);
}
/*

UInstanceGroup	*CSceneUser::createInstanceGroup (const std::string &instanceGroup)
{
	// Create the instance group
	CInstanceGroupUser *user=new CInstanceGroupUser;

	// Init the class
	if (!user->load (instanceGroup))
	{
		// Prb, erase it
		delete user;

		// Return error code
		return NULL;
	}

	// Insert the pointer in the pointer list
	_InstanceGroups.insert (user);

	// return the good value
	return user;
}

void			CSceneUser::deleteInstanceGroup (UInstanceGroup	*group)
{
	// The component is auto added/deleted to _Scene in ctor/dtor.
	_InstanceGroups.erase (dynamic_cast<CInstanceGroupUser*>(group));
}
*/

void CSceneUser::setToGlobalInstanceGroup(UInstanceGroup *pIG)
{
	NL3D_HAUTO_UI_SCENE;

	CInstanceGroupUser *pIGU = (CInstanceGroupUser*)pIG;
	pIGU->_InstanceGroup.setClusterSystemForInstances(_Scene.getGlobalInstanceGroup());
}

// ***************************************************************************
UVisualCollisionManager		*CSceneUser::createVisualCollisionManager()
{
	NL3D_HAUTO_ELT_SCENE;

	return _VisualCollisionManagers.insert(new CVisualCollisionManagerUser);
}
void						CSceneUser::deleteVisualCollisionManager(UVisualCollisionManager *mgr)
{
	NL3D_HAUTO_ELT_SCENE;

	CVisualCollisionManagerUser	*vcmUser= dynamic_cast<CVisualCollisionManagerUser*>(mgr);

	// if it was the one used for shadow receiving in this scene, then set NULL
	if(_Scene.getVisualCollisionManagerForShadow()==&vcmUser->getVCM())
		_Scene.setVisualCollisionManagerForShadow(NULL);

	// and delete it
	_VisualCollisionManagers.erase(vcmUser);
}


// ***************************************************************************
CSceneUser::CSceneUser(CDriverUser *drv, bool bSmallScene) : _Scene(bSmallScene)
{
	nlassert(drv);
	_DriverUser= drv;

	// init default Roots.
	_Scene.initDefaultRoots();

	// Set driver.
	_Scene.setDriver(_DriverUser->getDriver());

	// Set viewport
	_Scene.setViewport (CViewport());

	// init QuadGridClipManager
	_Scene.initQuadGridClipManager ();

	// Create default camera, and active!!
	setCam(createCamera());

	_WaterEnvMap = NULL;
}

CSceneUser::~CSceneUser()
{
	_VisualCollisionManagers.clear();
	_Landscapes.clear();
	_CloudScapes.clear();
	_Scene.release();
	_Scene.setDriver(NULL);
	_Scene.setCam(NULL);
	_DriverUser= NULL;
}

// ***************************************************************************
void		CSceneUser::setMaxSkeletonsInNotCLodForm(uint m)
{
	_Scene.setMaxSkeletonsInNotCLodForm(m);
}

// ***************************************************************************
uint		CSceneUser::getMaxSkeletonsInNotCLodForm() const
{
	return _Scene.getMaxSkeletonsInNotCLodForm();
}


// ***************************************************************************
void		CSceneUser::enableElementRender(TRenderFilter elt, bool state)
{
	_Scene.enableElementRender(elt, state);
}


// ***************************************************************************
void		CSceneUser::profileNextRender()
{
	_Scene.profileNextRender();
}
// ***************************************************************************
void		CSceneUser::getProfileResults(CBenchResults &results)
{
	// Fill profile results
	results= _Scene.BenchRes;
}

// ***************************************************************************
void		CSceneUser::profileQuadGridClipManager()
{
	_Scene.getClipTrav().getQuadGridClipManager()->profile();
}

// ***************************************************************************
uint CSceneUser::getNumFlareContexts() const
{
	return CScene::MaxNumFlareContexts;
}

// ***************************************************************************
void CSceneUser::setFlareContext(uint context)
{
	if (context >= CScene::MaxNumFlareContexts)
	{
		nlwarning("Invalid flare context");
		return;
	}
	_Scene.setFlareContext(context);
}

// ***************************************************************************
uint CSceneUser::getFlareContext() const
{
	return _Scene.getFlareContext();
}


// ***************************************************************************
uint			CSceneUser::getShadowMapTextureSize() const
{
	return _Scene.getShadowMapTextureSize();
}

// ***************************************************************************
void			CSceneUser::setShadowMapTextureSize(uint size)
{
	_Scene.setShadowMapTextureSize(size);
}

// ***************************************************************************
uint			CSceneUser::getShadowMapBlurSize() const
{
	return _Scene.getShadowMapBlurSize();
}

// ***************************************************************************
void			CSceneUser::setShadowMapBlurSize(uint bs)
{
	_Scene.setShadowMapBlurSize(bs);
}

// ***************************************************************************
void			CSceneUser::enableShadowPolySmooth(bool enable)
{
	_Scene.enableShadowPolySmooth(enable);
}

// ***************************************************************************
bool			CSceneUser::getEnableShadowPolySmooth() const
{
	return _Scene.getEnableShadowPolySmooth();
}

// ***************************************************************************
void			CSceneUser::setShadowMapDistFadeStart(float dist)
{
	_Scene.setShadowMapDistFadeStart(dist);
}
// ***************************************************************************
float			CSceneUser::getShadowMapDistFadeStart() const
{
	return _Scene.getShadowMapDistFadeStart();
}
// ***************************************************************************
void			CSceneUser::setShadowMapDistFadeEnd(float dist)
{
	_Scene.setShadowMapDistFadeEnd(dist);
}
// ***************************************************************************
float			CSceneUser::getShadowMapDistFadeEnd() const
{
	return _Scene.getShadowMapDistFadeEnd();
}
// ***************************************************************************
void			CSceneUser::setShadowMapMaxCasterInScreen(uint num)
{
	_Scene.setShadowMapMaxCasterInScreen(num);
}
// ***************************************************************************
uint			CSceneUser::getShadowMapMaxCasterInScreen() const
{
	return _Scene.getShadowMapMaxCasterInScreen();
}
// ***************************************************************************
void			CSceneUser::setShadowMapMaxCasterAround(uint num)
{
	_Scene.setShadowMapMaxCasterAround(num);
}
// ***************************************************************************
uint			CSceneUser::getShadowMapMaxCasterAround() const
{
	return _Scene.getShadowMapMaxCasterAround();
}
// ***************************************************************************
void			CSceneUser::setVisualCollisionManagerForShadow(UVisualCollisionManager *vcm)
{
	if(vcm==NULL)
		_Scene.setVisualCollisionManagerForShadow(NULL);
	else
	{
		CVisualCollisionManagerUser	*vcmUser= static_cast<CVisualCollisionManagerUser*>(vcm);
		_Scene.setVisualCollisionManagerForShadow(&vcmUser->getVCM());
	}
}
// ***************************************************************************
void CSceneUser::setWaterCallback(IWaterSurfaceAddedCallback *wcb)
{
	_Scene.setWaterCallback(wcb);
}
// ***************************************************************************
IWaterSurfaceAddedCallback *CSceneUser::getWaterCallback() const
{
	return _Scene.getWaterCallback();
}
// ***************************************************************************
void CSceneUser::setLandscapePolyDrawingCallback(ILandscapePolyDrawingCallback *lpd)
{
	_Scene.setLandscapePolyDrawingCallback(lpd);
}
// ***************************************************************************
ILandscapePolyDrawingCallback *CSceneUser::getLandscapePolyDrawingCallback() const
{
	return _Scene.getLandscapePolyDrawingCallback();
}
// ***************************************************************************
void CSceneUser::setupTransparencySorting(uint8 maxPriority /*=0*/,uint NbDistanceEntries /*=1024*/)
{
	_Scene.getRenderTrav().setupTransparencySorting(maxPriority, NbDistanceEntries);
}

// ***************************************************************************
void CSceneUser::setWaterEnvMap(UWaterEnvMap *waterEnvMap)
{
	if (waterEnvMap)
	{
		if (((CWaterEnvMapUser *) waterEnvMap)->EnvMap.Driver != _DriverUser)
		{
			nlwarning("Water envmap can only be set in a scene that was created from the same driver");
			return;
		}
	}
	_Scene.setWaterEnvMap(&((CWaterEnvMapUser *) waterEnvMap)->EnvMap);
	_WaterEnvMap = waterEnvMap;
}


// ***************************************************************************
void CSceneUser::updateWaterEnvMaps(TGlobalAnimationTime time)
{
	_Scene.updateWaterEnvMaps(time);
}




} // NL3D
