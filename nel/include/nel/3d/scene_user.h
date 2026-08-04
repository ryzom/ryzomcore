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
		CWaitingIG(UInstanceGroup **callerPtr, const NLMISC::CVector &pos, const NLMISC::CQuat &rot, uint selectedTexture, IAsyncLoadCallback *pCB)	: IGToLoad(nullptr)
		    , CallerPtr(callerPtr), Pos(pos), Rot(rot), SelectedTexture(selectedTexture), Callback(pCB)
		{}
	};
	//
	typedef std::list<CWaitingIG> TWaitingIGList;
	TWaitingIGList		  _WaitingIGs;
	UWaterEnvMap		  *_WaterEnvMap;

	// U-level texture wrappers for water reflection info
	std::vector<CTextureUser *>		_WaterReflectionTextures;
	// Fill a UWaterReflectionInfo from an internal reflection state
	void fillWaterReflectionInfo(const CWaterReflectionManager::CActiveReflection &refl, UWaterReflectionInfo &info);

public:

	/// \name Object
	// @{
	CSceneUser(CDriverUser *drv, bool bSmallScene);
	virtual	~CSceneUser() NL_OVERRIDE;
	// @}

public:

	/// \name Render
	//@{
	// render methods
	virtual	void			render(bool updateWaitingInstances = true, bool restoreMatrixContextAfterRender = true) NL_OVERRIDE;
	virtual	void			beginPartRender() NL_OVERRIDE;
	virtual	void			renderPart(TRenderPart rp, bool doHrcPass = true, bool doTrav = true, bool keepTrav = false) NL_OVERRIDE;
	virtual	void			endPartRender(bool updateWaitingInstances = true, bool restoreMatrixContextAfterRender = true, bool keepTrav = true) NL_OVERRIDE;

	// update async loading whithout a call to render
	virtual void			updateWaitingInstances(double ellapsedTime) NL_OVERRIDE;
	virtual	void			animate(TGlobalAnimationTime time) NL_OVERRIDE;
	//@}


	/// \name Camera/Viewport.
	//@{
	virtual	void			setCam(UCamera cam) NL_OVERRIDE;
	virtual	UCamera			getCam() NL_OVERRIDE;
	virtual	void			setViewport(const class CViewport& viewport) NL_OVERRIDE;
	virtual	CViewport		getViewport() NL_OVERRIDE;
	virtual	UInstanceGroup	*findCameraClusterSystemFromRay(UInstanceGroup *startClusterSystem,
		const NLMISC::CVector &startPos, NLMISC::CVector &endPos) NL_OVERRIDE;
	//@}


	/// \name Component Mgt.
	//@{

	virtual	UCamera			createCamera() NL_OVERRIDE;
	virtual	void			deleteCamera(UCamera &cam) NL_OVERRIDE;

	virtual	UInstance		createInstance(const std::string &shapeName) NL_OVERRIDE;
	virtual	void			createInstanceAsync(const std::string &shapeName, UInstance *ppInstance, const NLMISC::CVector &position, uint selectedTexture) NL_OVERRIDE;
	virtual	void			deleteInstance(UInstance &inst) NL_OVERRIDE;

	virtual	void createInstanceGroupAndAddToSceneAsync (const std::string &instanceGroup, UInstanceGroup **pIG, const NLMISC::CVector &pos,
														const NLMISC::CQuat &rot, uint selectedTexture, IAsyncLoadCallback *pCB = nullptr) NL_OVERRIDE;

	virtual	void stopCreatingAndAddingIG(UInstanceGroup **pIG) NL_OVERRIDE;
	virtual void deleteInstanceGroup(UInstanceGroup *pIG) NL_OVERRIDE;

	/// should be called at each render
	void	updateWaitingIG();


	virtual UTransform		createTransform() NL_OVERRIDE;
	virtual	void			deleteTransform(UTransform &tr) NL_OVERRIDE;

	virtual	USkeleton		createSkeleton(const std::string &shapeName) NL_OVERRIDE;
	virtual	void			deleteSkeleton(USkeleton &skel) NL_OVERRIDE;

	virtual	ULandscape		*createLandscape() NL_OVERRIDE;
	virtual	void			deleteLandscape(ULandscape *land) NL_OVERRIDE;

	virtual	UCloudScape		*createCloudScape() NL_OVERRIDE;
	virtual	void			deleteCloudScape(UCloudScape *cs) NL_OVERRIDE;
/*

	virtual	UInstanceGroup	*createInstanceGroup (const std::string &instanceGroup);
	virtual	void			deleteInstanceGroup (UInstanceGroup	*group);
*/

	virtual void setToGlobalInstanceGroup(UInstanceGroup *pIG) NL_OVERRIDE;

	virtual	UPointLight		createPointLight() NL_OVERRIDE;
	virtual	void			deletePointLight(UPointLight &light) NL_OVERRIDE;

	//@}

	/// \name Animation gestion.
	// @{
	/** Set the automatic animation set used by the scene.
	  */
	virtual void				setAutomaticAnimationSet(UAnimationSet *as) NL_OVERRIDE;
	/// Create a new PlayListManager.
	virtual	UPlayListManager	*createPlayListManager() NL_OVERRIDE ;
	/// Delete a PlayListManager.
	virtual	void				deletePlayListManager(UPlayListManager *playListManager) NL_OVERRIDE ;
	// @}

	/// \name Visual Collision manager.
	//@{
	virtual	UVisualCollisionManager		*createVisualCollisionManager() NL_OVERRIDE;
	virtual	void						deleteVisualCollisionManager(UVisualCollisionManager *mgr) NL_OVERRIDE;
	//@}


	/// \name LoadBalancing mgt.
	//@{
	virtual	void					setPolygonBalancingMode(TPolygonBalancingMode polBalMode) NL_OVERRIDE;
	virtual	TPolygonBalancingMode	getPolygonBalancingMode() const NL_OVERRIDE;

	virtual float				getNbFaceAsked () const NL_OVERRIDE;

	virtual	void				setGroupLoadMaxPolygon(const std::string &group, uint nFaces) NL_OVERRIDE;
	virtual	uint				getGroupLoadMaxPolygon(const std::string &group) NL_OVERRIDE;
	virtual float				getGroupNbFaceAsked (const std::string &group) const NL_OVERRIDE;

	virtual void				setMaxSkeletonsInNotCLodForm(uint m) NL_OVERRIDE;
	virtual uint				getMaxSkeletonsInNotCLodForm() const NL_OVERRIDE;

	//@}

	/// \name Coarse meshes mgt.
	//@{
	virtual void				setCoarseMeshManagerTexture (const char *sPath) NL_OVERRIDE;
	virtual void				setCoarseMeshLightingUpdate(uint8 period) NL_OVERRIDE;
	virtual uint8				getCoarseMeshLightingUpdate() const NL_OVERRIDE;
	//@}

	/// \name transparent Layers mgt
	//@{
	virtual void  setLayersRenderingOrder(bool directOrder = true) NL_OVERRIDE { _Scene.setLayersRenderingOrder(directOrder); }
	virtual bool  getLayersRenderingOrder() const NL_OVERRIDE { return _Scene.getLayersRenderingOrder(); }
	//@}

	/// \name Global light setup.
	//@{

	virtual	void				enableLightingSystem(bool enable) NL_OVERRIDE;

	virtual	void				setAmbientGlobal(NLMISC::CRGBA ambient) NL_OVERRIDE;
	virtual	void				setSunAmbient(NLMISC::CRGBA ambient) NL_OVERRIDE;
	virtual	void				setSunDiffuse(NLMISC::CRGBA diffuse) NL_OVERRIDE;
	virtual	void				setSunSpecular(NLMISC::CRGBA specular) NL_OVERRIDE;
	virtual	void				setSunDirection(const NLMISC::CVector &direction) NL_OVERRIDE;
	virtual void				setLightGroupColor(uint lightmapGroup, NLMISC::CRGBA color) NL_OVERRIDE;

	virtual	NLMISC::CRGBA		getAmbientGlobal() const NL_OVERRIDE;
	virtual	NLMISC::CRGBA		getSunAmbient() const NL_OVERRIDE;
	virtual	NLMISC::CRGBA		getSunDiffuse() const NL_OVERRIDE;
	virtual	NLMISC::CRGBA		getSunSpecular() const NL_OVERRIDE;
	virtual	NLMISC::CVector		getSunDirection() const NL_OVERRIDE;

	virtual	void				setMaxLightContribution(uint nlights) NL_OVERRIDE;
	virtual	uint				getMaxLightContribution() const NL_OVERRIDE;

	virtual	void				setLightTransitionThreshold(float lightTransitionThreshold) NL_OVERRIDE;
	virtual	float				getLightTransitionThreshold() const NL_OVERRIDE;

	//@}


	/// \name Weather mgt
	//@{

	virtual void				setGlobalWindPower(float gwp) NL_OVERRIDE;
	virtual float				getGlobalWindPower() const NL_OVERRIDE;
	virtual void				setGlobalWindDirection(const NLMISC::CVector &gwd) NL_OVERRIDE;
	virtual const NLMISC::CVector		&getGlobalWindDirection() const NL_OVERRIDE;

	//@}


	/// \name CLod / Character Lod mgt
	//@{
	virtual void				resetCLodManager() NL_OVERRIDE;
	virtual uint32				loadCLodShapeBank(const std::string &fileName) NL_OVERRIDE;
	virtual void				deleteCLodShapeBank(uint32 bankId) NL_OVERRIDE;
	virtual sint32				getCLodShapeIdByName(const std::string &name) const NL_OVERRIDE;
	virtual sint32				getCLodAnimIdByName(uint32 shapeId, const std::string &name) const NL_OVERRIDE;
	//@}

	/// \name Profiling and Render Filtering
	// @{
	virtual	void				enableElementRender(TRenderFilter elt, bool state) NL_OVERRIDE;
	virtual void				profileNextRender() NL_OVERRIDE;
	virtual void				getProfileResults(CBenchResults &results) NL_OVERRIDE;
	virtual void				profileQuadGridClipManager() NL_OVERRIDE;
	// @}

	/// \name Flare contexts
	// @{
		virtual uint	getNumFlareContexts() const NL_OVERRIDE;
		virtual void	setFlareContext(uint context) NL_OVERRIDE;
		virtual uint    getFlareContext() const NL_OVERRIDE;
	// @}

	/// \name ShadowMapping Options
	// @{
	virtual	uint			getShadowMapTextureSize() const NL_OVERRIDE;
	virtual	void			setShadowMapTextureSize(uint size) NL_OVERRIDE;
	virtual	uint			getShadowMapBlurSize() const NL_OVERRIDE;
	virtual	void			setShadowMapBlurSize(uint bs) NL_OVERRIDE;
	virtual	void			enableShadowPolySmooth(bool enable) NL_OVERRIDE;
	virtual	bool			getEnableShadowPolySmooth() const NL_OVERRIDE;
	virtual	void			enableGPUSkinning(bool enable) NL_OVERRIDE;
	virtual	bool			isGPUSkinningEnabled() const NL_OVERRIDE;
	virtual	void			setShadowMapDistFadeStart(float dist) NL_OVERRIDE;
	virtual	float			getShadowMapDistFadeStart() const NL_OVERRIDE;
	virtual	void			setShadowMapDistFadeEnd(float dist) NL_OVERRIDE;
	virtual	float			getShadowMapDistFadeEnd() const NL_OVERRIDE;
	virtual	void			setShadowMapMaxCasterInScreen(uint num) NL_OVERRIDE;
	virtual	uint			getShadowMapMaxCasterInScreen() const NL_OVERRIDE;
	virtual	void			setShadowMapMaxCasterAround(uint num) NL_OVERRIDE;
	virtual	uint			getShadowMapMaxCasterAround() const NL_OVERRIDE;
	virtual	void			setVisualCollisionManagerForShadow(UVisualCollisionManager *vcm) NL_OVERRIDE;
	// @}

	void						setWaterCallback(IWaterSurfaceAddedCallback *wcb) NL_OVERRIDE;
	IWaterSurfaceAddedCallback *getWaterCallback() const NL_OVERRIDE;

	void			setLandscapePolyDrawingCallback(ILandscapePolyDrawingCallback *lpd) NL_OVERRIDE;
	ILandscapePolyDrawingCallback *getLandscapePolyDrawingCallback() const NL_OVERRIDE;
	//
	virtual void setupTransparencySorting(uint8 maxPriority = 0, uint NbDistanceEntries = 1024) NL_OVERRIDE;

	/// \name Water envmaps
	// @{
	virtual void		  setWaterEnvMap(UWaterEnvMap *waterEnvMap) NL_OVERRIDE;
	virtual UWaterEnvMap *getWaterEnvMap() const NL_OVERRIDE { return _WaterEnvMap; }
	virtual void		  updateWaterEnvMaps(TGlobalAnimationTime time) NL_OVERRIDE;
	virtual void		  setForceWaterEnvMap(bool force) NL_OVERRIDE { _Scene.setForceWaterEnvMap(force); }
	virtual bool		  getForceWaterEnvMap() const NL_OVERRIDE { return _Scene.getForceWaterEnvMap(); }
	// @}

	/// \name Realtime planar water reflections
	// @{
	virtual void		  setMaxRealtimeWaterReflections(sint maxCount) NL_OVERRIDE { _Scene.getWaterReflectionManager().setMaxReflections(maxCount); }
	virtual sint		  getMaxRealtimeWaterReflections() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getMaxReflections(); }
	virtual void		  setForceRealtimeWaterReflections(bool force) NL_OVERRIDE { _Scene.getWaterReflectionManager().setForceReflections(force); }
	virtual bool		  getForceRealtimeWaterReflections() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getForceReflections(); }
	virtual uint		  beginWaterReflectionPasses() NL_OVERRIDE { return _Scene.getWaterReflectionManager().beginPasses(); }
	virtual void		  setWaterReflectionView(uint view) NL_OVERRIDE { _Scene.getWaterReflectionManager().setCurrentView(view); }
	virtual uint		  getWaterReflectionView() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getCurrentView(); }
	virtual void		  beginWaterReflectionPass(uint pass, UWaterReflectionInfo &info) NL_OVERRIDE;
	virtual void		  endWaterReflectionPass(uint pass) NL_OVERRIDE { _Scene.getWaterReflectionManager().endPass(pass); }
	virtual void		  endWaterReflectionPasses() NL_OVERRIDE { _Scene.getWaterReflectionManager().endPasses(); }
	virtual bool		  isRenderingWaterReflection() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().isRenderingReflection(); }
	virtual uint		  getNumActiveWaterReflections() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getNumActiveReflections(); }
	virtual bool		  getActiveWaterReflectionInfo(uint index, UWaterReflectionInfo &info) NL_OVERRIDE;
	virtual void		  setWaterReflectionHalfRes(bool halfRes) NL_OVERRIDE { _Scene.getWaterReflectionManager().setHalfRes(halfRes); }
	virtual bool		  getWaterReflectionHalfRes() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getHalfRes(); }
	virtual void		  setWaterReflectionPow2(bool pow2) NL_OVERRIDE { _Scene.getWaterReflectionManager().setPow2(pow2); }
	virtual bool		  getWaterReflectionPow2() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getPow2(); }
	virtual void		  setWaterReflectionFixedSize(bool fixedSize) NL_OVERRIDE { _Scene.getWaterReflectionManager().setFixedSize(fixedSize); }
	virtual bool		  getWaterReflectionFixedSize() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getFixedSize(); }
	virtual void		  setWaterReflectionMaxTextures(sint maxTextures) NL_OVERRIDE { _Scene.getWaterReflectionManager().setMaxTextures(maxTextures); }
	virtual sint		  getWaterReflectionMaxTextures() const NL_OVERRIDE { return _Scene.getWaterReflectionManager().getMaxTextures(); }
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
