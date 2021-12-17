// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_SCENE_USER_H
#define NL_SCENE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/landscape_user.h"
#include "nel/3d/cloud_scape_user.h"
#include "nel/3d/instance_group_user.h"
#include "nel/3d/visual_collision_manager_user.h"
#include "nel/3d/play_list_manager_user.h"


namespace NL3D {


class UWaterEnvMap;

/**
 * TODO Class description
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSceneUser : public UScene
{
private:
	/// update async loading during a call to render
	virtual void			updateWaitingInstances();
protected:
	/// The driver which owns this scene.
	CDriverUser			*_DriverUser;
	/// The Scene.
	CScene				_Scene;


	// Components List.
	typedef	CPtrSet<CLandscapeUser>		TLandscapeSet;
	typedef	CPtrSet<CCloudScapeUser>	TCloudScapeSet;
	typedef	CPtrSet<CInstanceGroupUser>	TInstanceGroupSet;
	typedef	CPtrSet<CVisualCollisionManagerUser>	TVisualCollisionManagerSet;
	typedef	CPtrSet<CPlayListManagerUser>	TPlayListManagerSet;
	TLandscapeSet				_Landscapes;
	TCloudScapeSet				_CloudScapes;
	TInstanceGroupSet			_InstanceGroups;
	TVisualCollisionManagerSet	_VisualCollisionManagers;
	TPlayListManagerSet			_PlayListManagers;

	std::map<UInstance*,CTransformShape*> _WaitingInstances;

	// IG that wait to be created and added to that scene
	struct CWaitingIG
	{
		UInstanceGroup	*IGToLoad;
		UInstanceGroup	**CallerPtr;
		NLMISC::CVector Pos;
		NLMISC::CQuat	Rot;
		uint			SelectedTexture;
		IAsyncLoadCallback *Callback;
		CWaitingIG(UInstanceGroup **callerPtr, const NLMISC::CVector &pos, const NLMISC::CQuat &rot, uint selectedTexture, IAsyncLoadCallback *pCB)	: IGToLoad(NULL), CallerPtr(callerPtr), Pos(pos), Rot(rot), SelectedTexture(selectedTexture), Callback(pCB)
		{}
	};
	//
	typedef std::list<CWaitingIG> TWaitingIGList;
	TWaitingIGList		  _WaitingIGs;
	UWaterEnvMap		  *_WaterEnvMap;

public:

	/// \name Object
	// @{
	CSceneUser(CDriverUser *drv, bool bSmallScene);
	virtual	~CSceneUser();
	// @}

public:

	/// \name Render
	//@{
	// render methods
	virtual	void			render(bool updateWaitingInstances = true, bool restoreMatrixContextAfterRender = true);
	virtual	void			beginPartRender();
	virtual	void			renderPart(TRenderPart rp, bool doHrcPass = true, bool doTrav = true, bool keepTrav = false);
	virtual	void			endPartRender(bool updateWaitingInstances = true, bool restoreMatrixContextAfterRender = true, bool keepTrav = true);

	// update async loading whithout a call to render
	virtual void			updateWaitingInstances(double ellapsedTime);
	virtual	void			animate(TGlobalAnimationTime time);
	//@}


	/// \name Camera/Viewport.
	//@{
	virtual	void			setCam(UCamera cam);
	virtual	UCamera			getCam();
	virtual	void			setViewport(const class CViewport& viewport);
	virtual	CViewport		getViewport();
	virtual	UInstanceGroup	*findCameraClusterSystemFromRay(UInstanceGroup *startClusterSystem,
		const NLMISC::CVector &startPos, NLMISC::CVector &endPos);
	//@}


	/// \name Component Mgt.
	//@{

	virtual	UCamera			createCamera();
	virtual	void			deleteCamera(UCamera &cam);

	virtual	UInstance		createInstance(const std::string &shapeName);
	virtual	void			createInstanceAsync(const std::string &shapeName, UInstance *ppInstance, const NLMISC::CVector &position, uint selectedTexture);
	virtual	void			deleteInstance(UInstance &inst);

	virtual	void createInstanceGroupAndAddToSceneAsync (const std::string &instanceGroup, UInstanceGroup **pIG, const NLMISC::CVector &pos,
														const NLMISC::CQuat &rot, uint selectedTexture, IAsyncLoadCallback *pCB = NULL);

	virtual	void stopCreatingAndAddingIG(UInstanceGroup **pIG);
	virtual void deleteInstanceGroup(UInstanceGroup *pIG);

	/// should be called at each render
	void	updateWaitingIG();


	virtual UTransform		createTransform();
	virtual	void			deleteTransform(UTransform &tr);

	virtual	USkeleton		createSkeleton(const std::string &shapeName);
	virtual	void			deleteSkeleton(USkeleton &skel);

	virtual	ULandscape		*createLandscape();
	virtual	void			deleteLandscape(ULandscape *land);

	virtual	UCloudScape		*createCloudScape();
	virtual	void			deleteCloudScape(UCloudScape *cs);
/*

	virtual	UInstanceGroup	*createInstanceGroup (const std::string &instanceGroup);
	virtual	void			deleteInstanceGroup (UInstanceGroup	*group);
*/

	virtual void setToGlobalInstanceGroup(UInstanceGroup *pIG);

	virtual	UPointLight		createPointLight();
	virtual	void			deletePointLight(UPointLight &light);

	//@}

	/// \name Animation gestion.
	// @{
	/** Set the automatic animation set used by the scene.
	  */
	virtual void				setAutomaticAnimationSet(UAnimationSet *as);
	/// Create a new PlayListManager.
	virtual	UPlayListManager	*createPlayListManager() ;
	/// Delete a PlayListManager.
	virtual	void				deletePlayListManager(UPlayListManager *playListManager) ;
	// @}

	/// \name Visual Collision manager.
	//@{
	virtual	UVisualCollisionManager		*createVisualCollisionManager();
	virtual	void						deleteVisualCollisionManager(UVisualCollisionManager *mgr);
	//@}


	/// \name LoadBalancing mgt.
	//@{
	virtual	void					setPolygonBalancingMode(TPolygonBalancingMode polBalMode);
	virtual	TPolygonBalancingMode	getPolygonBalancingMode() const;

	virtual float				getNbFaceAsked () const;

	virtual	void				setGroupLoadMaxPolygon(const std::string &group, uint nFaces);
	virtual	uint				getGroupLoadMaxPolygon(const std::string &group);
	virtual float				getGroupNbFaceAsked (const std::string &group) const;

	virtual void				setMaxSkeletonsInNotCLodForm(uint m);
	virtual uint				getMaxSkeletonsInNotCLodForm() const;

	//@}

	/// \name Coarse meshes mgt.
	//@{
	virtual void				setCoarseMeshManagerTexture (const char *sPath);
	virtual void				setCoarseMeshLightingUpdate(uint8 period);
	virtual uint8				getCoarseMeshLightingUpdate() const;
	//@}

	/// \name transparent Layers mgt
	//@{
	virtual void  setLayersRenderingOrder(bool directOrder = true) { _Scene.setLayersRenderingOrder(directOrder); }
	virtual bool  getLayersRenderingOrder() const { return _Scene.getLayersRenderingOrder(); }
	//@}

	/// \name Global light setup.
	//@{

	virtual	void				enableLightingSystem(bool enable);

	virtual	void				setAmbientGlobal(NLMISC::CRGBA ambient);
	virtual	void				setSunAmbient(NLMISC::CRGBA ambient);
	virtual	void				setSunDiffuse(NLMISC::CRGBA diffuse);
	virtual	void				setSunSpecular(NLMISC::CRGBA specular);
	virtual	void				setSunDirection(const NLMISC::CVector &direction);
	virtual void				setLightGroupColor(uint lightmapGroup, NLMISC::CRGBA color);

	virtual	NLMISC::CRGBA		getAmbientGlobal() const;
	virtual	NLMISC::CRGBA		getSunAmbient() const;
	virtual	NLMISC::CRGBA		getSunDiffuse() const;
	virtual	NLMISC::CRGBA		getSunSpecular() const;
	virtual	NLMISC::CVector		getSunDirection() const;

	virtual	void				setMaxLightContribution(uint nlights);
	virtual	uint				getMaxLightContribution() const;

	virtual	void				setLightTransitionThreshold(float lightTransitionThreshold);
	virtual	float				getLightTransitionThreshold() const;

	//@}


	/// \name Weather mgt
	//@{

	virtual void				setGlobalWindPower(float gwp);
	virtual float				getGlobalWindPower() const;
	virtual void				setGlobalWindDirection(const NLMISC::CVector &gwd);
	virtual const NLMISC::CVector		&getGlobalWindDirection() const;

	//@}


	/// \name CLod / Character Lod mgt
	//@{
	virtual void				resetCLodManager();
	virtual uint32				loadCLodShapeBank(const std::string &fileName);
	virtual void				deleteCLodShapeBank(uint32 bankId);
	virtual sint32				getCLodShapeIdByName(const std::string &name) const;
	virtual sint32				getCLodAnimIdByName(uint32 shapeId, const std::string &name) const;
	//@}

	/// \name Profiling and Render Filtering
	// @{
	virtual	void				enableElementRender(TRenderFilter elt, bool state);
	virtual void				profileNextRender();
	virtual void				getProfileResults(CBenchResults &results);
	virtual void				profileQuadGridClipManager();
	// @}

	/// \name Flare contexts
	// @{
		virtual uint	getNumFlareContexts() const;
		virtual void	setFlareContext(uint context);
		virtual uint    getFlareContext() const;
	// @}

	/// \name ShadowMapping Options
	// @{
	virtual	uint			getShadowMapTextureSize() const;
	virtual	void			setShadowMapTextureSize(uint size);
	virtual	uint			getShadowMapBlurSize() const;
	virtual	void			setShadowMapBlurSize(uint bs);
	virtual	void			enableShadowPolySmooth(bool enable);
	virtual	bool			getEnableShadowPolySmooth() const;
	virtual	void			setShadowMapDistFadeStart(float dist);
	virtual	float			getShadowMapDistFadeStart() const;
	virtual	void			setShadowMapDistFadeEnd(float dist);
	virtual	float			getShadowMapDistFadeEnd() const;
	virtual	void			setShadowMapMaxCasterInScreen(uint num);
	virtual	uint			getShadowMapMaxCasterInScreen() const;
	virtual	void			setShadowMapMaxCasterAround(uint num);
	virtual	uint			getShadowMapMaxCasterAround() const;
	virtual	void			setVisualCollisionManagerForShadow(UVisualCollisionManager *vcm);
	// @}

	void						setWaterCallback(IWaterSurfaceAddedCallback *wcb);
	IWaterSurfaceAddedCallback *getWaterCallback() const;

	void			setLandscapePolyDrawingCallback(ILandscapePolyDrawingCallback *lpd);
	ILandscapePolyDrawingCallback *getLandscapePolyDrawingCallback() const;
	//
	virtual void setupTransparencySorting(uint8 maxPriority = 0, uint NbDistanceEntries = 1024);

	/// \name Water envmaps
	// @{
	virtual void		  setWaterEnvMap(UWaterEnvMap *waterEnvMap);
	virtual UWaterEnvMap *getWaterEnvMap() const { return _WaterEnvMap; }
	virtual void		  updateWaterEnvMaps(TGlobalAnimationTime time);
	// @}

public:
	/// \name Accessor for CSceneUser.
	// @{
	CScene		&getScene()
	{
		return _Scene;
	}
	// @}

};


} // NL3D


#endif // NL_SCENE_USER_H

/* End of scene_user.h */
