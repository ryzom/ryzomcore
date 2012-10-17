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

#ifndef NL_U_SCENE_H
#define NL_U_SCENE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/quat.h"
#include "animation_time.h"
#include <map>


namespace NLMISC
{
class CRGBA;
class CVector;
class CPolygon2D;
class CMatrix;
};

namespace NL3D
{


class UCamera;
class UInstance;
class ULandscape;
class UCloudScape;
class UInstanceGroup;
class USkeleton;
class UTransform;
class UVisualCollisionManager;
class UAnimationSet;
class UPlayListManager;
class UPointLight;
class UWaterEnvMap;


// ****************************************************************************
// useful callback for createInstanceGroupAndAddToSceneAsync
class IAsyncLoadCallback
{
public:
	virtual ~IAsyncLoadCallback() {}
	/// Because the async loader set the pointer of the user after the whole loading (ig+shapes)
	/// what is passed through this method is the value that will be affected
	virtual void InstanceGroupCreated(UInstanceGroup *newVal) = 0;
};

// callback to know when a water surface is added
class IWaterSurfaceAddedCallback
{
public:
	virtual ~IWaterSurfaceAddedCallback() {}
	// A water surface with the given height has been added.
	virtual void waterSurfaceAdded(const NLMISC::CPolygon2D &shape, const NLMISC::CMatrix &worldMatrix, bool splashEnabled, bool usesSceneWaterenvmap) = 0;
	virtual void waterSurfaceRemoved(bool usesSceneWaterenvmap) = 0;
};

// callback to setup stencil buffer before and after landscape render
class ILandscapePolyDrawingCallback
{
public:
	virtual ~ILandscapePolyDrawingCallback() {}
	virtual void beginPolyDrawing() = 0;
	virtual void endPolyDrawing() = 0;
};


// ****************************************************************************
/**
 * Game Interface for Scene.
 * A UScene create instances, camera lights etc...  Those objects are not sharable through different scenes. You
 * cannot use a camera create with Scene1, for viewing Scene2.
 *
 * The scene is created with a default camera that you can retrieve with getCam().
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UScene
{
protected:

	/// \name Object
	// @{
	UScene() {}
	virtual	~UScene() {}
	// @}

public:
	enum TRenderPart
	{
		RenderNothing     = 0,
		RenderOpaque	  = 0x01,
		RenderTransparent = 0x02,
		RenderFlare		  = 0x04,
		RenderAll = RenderOpaque | RenderTransparent | RenderFlare
	};


	/// \name Render
	//@{
	/** Render the scene from the CurrentCamera view (setCam()), and the current Viewport. nlError if no current camera.
	 * NB: no Driver clear buffers (color or ZBuffer) are done.... \n
	 * NB: The UDriver Matrix/Viewport context for 2D/3D interface is restored after this render.
	 * NB: nlerror if the current camera has been deleted.
	 * NB: the UDriver Light setup (see UDriver::setLight() / UDriver::setAmbientColor()) is modified.
	 *	At the exit of render(), all UDriver lights are disabled.
	 */
	virtual	void			render(bool updateWaitingInstances = true, bool restoreMatrixContextAfterRender = true)=0;

	/** Begin Part Rendering
	 *	During beginPartRender()/endPartRender(), you can ask other scene to render their part, but you should
	 *	avoid to create models or modify the scene (not fully tested)
	 *  WARNING: assert-crash if between a beginPartRender()/endPartRender()
	 */
	virtual	void			beginPartRender() =0;

	/** Render a part (see render() for what it does)
	 *	beginPartRender() must have been called
	 * \param renderPart a combination of UScene::TRenderPart flags, allow to choose which part of the scene must be rendered
	 *	WARNING: always must begin rendering with at least UScene::RenderOpaque, else shadows won't work
	 *	WARNING: assert-crash if a part in 'rp' has already been rendered since the last beginPartRender()
	 */
	virtual void			renderPart(UScene::TRenderPart rp) =0;

	/** End Part Rendering (commit model creation and deletion that were asked during rendering)
	 */
	virtual void			endPartRender(bool updateWaitingInstances = true, bool restoreMatrixContextAfterRender = true) =0;


	/** Update waiting instances and igs that are loaded asynchronously
	  * NB: this is called by render()
	  * \param systemTimeEllapsed : the time between 2 calls to updateWaitingInstances, in seconds
	  */
	virtual void			updateWaitingInstances(double systemTimeEllapsed) = 0;


	/**
	 *	Set the animation time for lightmap and Particle System animation.
	 */
	virtual	void			animate (TGlobalAnimationTime time) =0;

	//@}


	/// \name Camera/Viewport.
	//@{
	/// Set the current camera for this scene. nlError if NULL, or if the camera was not created by this UScene.
	virtual	void			setCam(UCamera cam)=0;
	/// Get the current camera.
	virtual	UCamera			getCam()=0;
	/// Set the current Viewport.
	virtual	void			setViewport(const class CViewport& viewport)=0;
	/// Get the current Viewport.
	virtual	CViewport		getViewport()=0;

	/** Special For Camera Third person. Traverse the ClusterSystem with a Ray
	 *	(clip through portals, cluster system hierarchy...), to find where could lies
	 *	the camera at End point.
	 *	\param endPos may be modified to ensure that it lies in a cluster of the found cluster system
	 *	\return the found cluster System
	 */
	virtual	UInstanceGroup	*findCameraClusterSystemFromRay(UInstanceGroup *startClusterSystem,
		const NLMISC::CVector &startPos, NLMISC::CVector &endPos) =0;
	//@}


	/// \name Component Mgt.
	/** A Uscene is a factory of Instance/Landscape/Lights etc...
	 *
	 */
	//@{

	/// Create a camera. Useful for this scene only.
	virtual	UCamera			createCamera()=0;
	/// Delete a camera.
	virtual	void			deleteCamera(UCamera &cam)=0;

	/** Create an instance of the shape "shapename". If not present, try to load "shapename" via CPath.
	 * If fails (file not found), return NULL.
	 * nlerror if the file is not a mesh file.
	 */
	virtual	UInstance		createInstance(const std::string &shapeName)=0;
	/** Same as createInstance but the instance is loaded asynchronously.
	 * You must poll to know if the instance if created by calling render()
	 */
	virtual	void			createInstanceAsync(const std::string &shapeName,UInstance *ppInstance, const NLMISC::CVector &position, uint selectedTexture)=0;
	/** Delete an instance via his pointer.
	 */
	virtual	void			deleteInstance(UInstance &inst)=0;

	/** create an instance group asynchronously, and add it to a scene asynchronously once it has been
	  * loaded. The instance groups that must be added to the scene are checked at each render.
	  * The pIG pointer is filled once the instance group has been loaded and add to the scene
	  */
	virtual	void createInstanceGroupAndAddToSceneAsync (const std::string &instanceGroup,
													    UInstanceGroup **pIG,
														const NLMISC::CVector &pos,
														const NLMISC::CQuat &rot,
														uint selectedTexture,
														IAsyncLoadCallback *pCB = NULL
													   ) = 0;

	/**	If we are adding the ig : stop loading and adding it to the scene
	  * if the ig is already added to the scene : remove it from scene
	  */
	virtual	void stopCreatingAndAddingIG(UInstanceGroup **pIG) = 0;

	/** Delete an instance group
	  */
	virtual void deleteInstanceGroup(UInstanceGroup *pIG) = 0;

	/** Create a dummy object
	 */
	virtual	UTransform		createTransform()=0;
	/** Delete a dummy object
	 */
	virtual	void			deleteTransform(UTransform &tr)=0;

	/** Create a skeleton instance of the skeleton shape "shapename". If not present, try to load "shapename" via CPath.
	 * If fails (file not found), return NULL.
	 * nlerror if the file is not a skeleton shape file.
	 */
	virtual	USkeleton		createSkeleton(const std::string &shapeName)=0;
	/** Delete a skeleton instance via his pointer.
	 */
	virtual	void			deleteSkeleton(USkeleton &skel)=0;

	/// Create a landscape. There could be many Landscape per Scene (Driver? :). But never tested :).
	virtual	ULandscape		*createLandscape()=0;
	/// delete a landscape.
	virtual	void			deleteLandscape(ULandscape *land)=0;

	/// Create a landscape. There could be many Landscape per Scene (Driver? :). But never tested :).
	virtual	UCloudScape		*createCloudScape()=0;
	/// delete a landscape.
	virtual	void			deleteCloudScape(UCloudScape *cs)=0;

	/** Assign the Instance Group to the root cluster
	  *
	  */
	virtual	void setToGlobalInstanceGroup(UInstanceGroup *pIGU) = 0;
	//virtual	UInstanceGroup	*createInstanceGroup(const std::string &instanceGroup) =0;
	/// Delete an instance group from this scene.
	//virtual	void			deleteInstanceGroup(UInstanceGroup	*group) =0;


	/// Create a dynamic PointLight. Useful for this scene only.
	virtual	UPointLight		createPointLight()=0;
	/// Delete a dynamic PointLight.
	virtual	void			deletePointLight(UPointLight &light)=0;


	//@}

	/// \name Animation Mgt.
	// @{
	/// Create a new PlayListManager.
	virtual	UPlayListManager	*createPlayListManager() =0;
	/// Delete a PlayListManager.
	virtual	void				deletePlayListManager(UPlayListManager *playListManager) =0;
	/** Set the automatic animation set used by the scene.
	  */
	virtual void				setAutomaticAnimationSet(UAnimationSet *as) = 0;
	// @}

	/// \name Visual Collision manager.
	//@{
	virtual	UVisualCollisionManager		*createVisualCollisionManager() =0;
	virtual	void						deleteVisualCollisionManager(UVisualCollisionManager *mgr) =0;
	//@}


	/// \name LoadBalancing Mgt.
	//@{

	/** The mode of polygon balancing. NB: this apply to All LoadBalancingGroups, but the "Default" group
	 *	which is always considered as PolygonBalancingOff
	 * PolygonBalancingOff => Models will be rendered with the number of faces they want to render.
	 * PolygonBalancingOn  => Models will be rendered with the number of faces the LoadBalancing want.
	 * PolygonBalancingClamp => Same as PolygonBalancingOn, but factor <= 1, ie models won't be rendered
	 *	with more face they want to render.
	 */
	enum			TPolygonBalancingMode {PolygonBalancingOff=0, PolygonBalancingOn, PolygonBalancingClamp, CountPolygonBalancing };

	/// Set the PolygonBalancingMode
	virtual	void				setPolygonBalancingMode(TPolygonBalancingMode polBalMode) =0;
	/// Get the PolygonBalancingMode
	virtual	TPolygonBalancingMode	getPolygonBalancingMode() const =0;


	/** Get the last face count asked from the instances before reduction.
	 *	It gets the sum of All groups.
	 */
	virtual float				getNbFaceAsked () const =0;


	/** Set the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	virtual	void				setGroupLoadMaxPolygon(const std::string &group, uint nFaces) =0;
	/** Get the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	virtual	uint				getGroupLoadMaxPolygon(const std::string &group) =0;
	/** Get the last face count asked from the instances before reduction. only for the given group
	 *	return 0 if the Group does not exist.
	 */
	virtual float				getGroupNbFaceAsked (const std::string &group) const =0;

	/** LoadBalancing for CLod and Skeletons.
	 *	Setup the max number of skeletons displayed in std way (ie not CLod). Default is 20.
	 */
	virtual void				setMaxSkeletonsInNotCLodForm(uint m) =0;
	/// see setMaxSkeletonsInNotCLodForm()
	virtual uint				getMaxSkeletonsInNotCLodForm() const =0;

	//@}

	/// \name Coarse meshes Mgt.
	//@{

	/** Set the coarse meshes's common texture.
	*/
	virtual void				setCoarseMeshManagerTexture (const char *sPath) =0;

	/**
	 *	Each coarseMesh lighting will be updated every "period" frame. clamped to 1,255
	 */
	virtual void				setCoarseMeshLightingUpdate(uint8 period) =0;

	/// see setCoarseMeshLightingUpdate()
	virtual uint8				getCoarseMeshLightingUpdate() const  =0;

	//@}


	/// \name Global light setup.
	//@{

	/** Enable Scene Lighting system. For backward compatibility, false by default.
	 *	If false, all objects will take last driver 's light setup
	 */
	virtual	void				enableLightingSystem(bool enable) =0;


	/// set the global Ambient used for the scene. Default to (50, 50, 50).
	virtual	void				setAmbientGlobal(NLMISC::CRGBA ambient) =0;
	/// set the Ambient of the Sun used for the scene.
	virtual	void				setSunAmbient(NLMISC::CRGBA ambient) =0;
	/// set the Diffuse of the Sun used for the scene.
	virtual	void				setSunDiffuse(NLMISC::CRGBA diffuse) =0;
	/// set the Specular of the Sun used for the scene.
	virtual	void				setSunSpecular(NLMISC::CRGBA specular) =0;
	/// set the Direction of the Sun used for the scene.
	virtual	void				setSunDirection(const NLMISC::CVector &direction) =0;
	/// set the color of a light group.
	virtual void				setLightGroupColor(uint lightmapGroup, NLMISC::CRGBA color) =0;


	/// get the global Ambient used for the scene. Default to (50, 50, 50).
	virtual	NLMISC::CRGBA		getAmbientGlobal() const =0;
	/// get the Ambient of the Sun used for the scene.
	virtual	NLMISC::CRGBA		getSunAmbient() const =0;
	/// get the Diffuse of the Sun used for the scene.
	virtual	NLMISC::CRGBA		getSunDiffuse() const =0;
	/// get the Specular of the Sun used for the scene.
	virtual	NLMISC::CRGBA		getSunSpecular() const =0;
	/// get the Direction of the Sun used for the scene.
	virtual	NLMISC::CVector		getSunDirection() const =0;

	/** setup the max number of point light that can influence a model. NB: clamped by NL3D_MAX_LIGHT_CONTRIBUTION
	 *	Default is 3.
	 *	NB: the sun contribution is not taken into account
	 */
	virtual	void				setMaxLightContribution(uint nlights) =0;
	/** \see setMaxLightContribution()
	  */
	virtual	uint				getMaxLightContribution() const =0;

	/** Advanced. When a model is influenced by more light than allowed, or when it reach the limits
	 *	of the light (attenuationEnd), the light can be darkened according to some threshold.
	 *	The resultLightColor begin to fade when distModelToLight== attEnd- threshold*(attEnd-attBegin).
	 *	when distModelToLight== 0, resultLightColor==Black.
	 *	By default, this value is 0.1f. Setting higher values will smooth transition but will
	 *	generally darken the global effects of lights.
	 *	NB: clamp(value, 0, 1);
	 */
	virtual	void				setLightTransitionThreshold(float lightTransitionThreshold) =0;
	/** \see getLightTransitionThreshold()
	  */
	virtual	float				getLightTransitionThreshold() const =0;


	//@}


	/// \name transparent Layers Mgt
	//@{
		/** Set the order or rendering of layers containing transparent objects.
		  * In real case, with direct order, we have:
		  * - Underwater is rendered.
		  * - Water is rendered.
		  * - Objects above water are rendered.
		  */
		virtual void  setLayersRenderingOrder(bool directOrder = true) = 0;
		virtual bool  getLayersRenderingOrder() const = 0;
	//@}


	/// \name Weather Mgt
	//@{

	/// Set the current windPower for all the scene. 0-1.
	virtual void				setGlobalWindPower(float gwp) =0;
	/// get the current windPower
	virtual float				getGlobalWindPower() const =0;

	/// Set the current windDirection for all the scene. dir.z set to 0 and vector normalized.
	virtual void				setGlobalWindDirection(const NLMISC::CVector &gwd) =0;
	/// get the current windDirection
	virtual const NLMISC::CVector		&getGlobalWindDirection() const =0;

	//@}


	/// \name CLod / Character Lod Mgt
	/** NB: for an historic reason, CLod interface is in UScene, BUT THERE IS ONLY ONE LOD MANAGER PER UDriver!
	 */
	//@{

	/// reset the manager.
	virtual void				resetCLodManager() =0;

	/** Load a Shape Bank. The ShapeMap is rebuilt. Hence slow call.
	 *	NB: a vector of ShapeBank is maintained internally, hence, not so many shapeBank should be
	 *	created at same Time.
	 *	throw exception if failed to load the file
	 *	\param fileName is a .clodbank file, to be loaded. CPath::lookup is used.
	 *	\return	id of the shape Bank.
	 */
	virtual uint32				loadCLodShapeBank(const std::string &fileName) =0;

	/// delete a Shape Bank. No-op if bad id.
	virtual void				deleteCLodShapeBank(uint32 bankId) =0;

	/** Get a shapeId by its name. -1 if not found.
	 */
	virtual sint32				getCLodShapeIdByName(const std::string &name) const =0;

	/** Get a AnimId of a shape by its name. -1 if not found.
	 */
	virtual sint32				getCLodAnimIdByName(uint32 shapeId, const std::string &name) const =0;

	//@}


	/** \name Flare contexts
	  * The flare objects are designed to work with a single scene, because they simulate 'retinia persistence' based on the visibility in the current scene.
	  * Several context allow to deals with a flare rendered from several points of views.
	  * There's a limited number of contexts
	  */
	// @{
		// Get the max number of contexts
		virtual uint getNumFlareContexts() const = 0;
		// Set the current context for flares. context must be < to MaxNumFlareContexts
		virtual void	setFlareContext(uint context) = 0;
		// Get the current context for flares
		virtual uint    getFlareContext() const = 0;
	// @}

	/// \name Profiling and Render Filtering
	// @{
	enum	TRenderFilter
	{
		FilterLandscape=	0x00000001,
		FilterWater=		0x00000002,
		FilterMeshNoVP=		0x00000004,
		FilterMeshVP=		0x00000008,
		FilterMeshMRMNoVP=	0x00000010,
		FilterMeshMRMVP=	0x00000020,
		FilterMeshLodNoVP=	0x00000040,
		FilterMeshLodVP=	0x00000080,
		FilterSkeleton=		0x00000100,			// For skins, and also for sticked objects.
		FilterSegRemanence= 0x00000200,
		FilterPS=			0x00000400,
		FilterFlare=		0x00000800,
		FilterCoarseMesh=	0x00001000,			// Disable render of CoarseMesh (but not management through Meshs!)

		// Combos:
		FilterAllMeshNoVP=	FilterMeshNoVP + FilterMeshMRMNoVP + FilterMeshLodNoVP,
		FilterAllMeshVP=	FilterMeshVP + FilterMeshMRMVP + FilterMeshLodVP,
		FilterAllMesh=		FilterAllMeshNoVP+FilterAllMeshVP,
		FilterFX=			FilterSegRemanence + FilterPS + FilterFlare,
	};

	/** Enable or disable some Models to be rendered (for profile). Default is ALL enabled
	 *	NB: filtering is made at clip pass. Hence, much of the render processing is skipped (animDetail, light, lod, render)
	 */
	virtual	void				enableElementRender(TRenderFilter elt, bool state) =0;

	/// For Scene Profiling
	struct	CBenchResults
	{
		std::map<uint32, uint32>							MeshMRMProfileTriVBFormat;
		std::map<uint32, uint32>							MeshProfileTriVBFormat;
		uint32												NumMeshRdrNormal;
		uint32												NumMeshRdrBlock;
		uint32												NumMeshRdrBlockWithVBHeap;
		uint32												NumMeshTriRdrNormal;
		uint32												NumMeshTriRdrBlock;
		uint32												NumMeshTriRdrBlockWithVBHeap;
		uint32												NumMeshMRMRdrNormal;
		uint32												NumMeshMRMRdrBlock;
		uint32												NumMeshMRMRdrBlockWithVBHeap;
		uint32												NumMeshMRMTriRdrNormal;
		uint32												NumMeshMRMTriRdrBlock;
		uint32												NumMeshMRMTriRdrBlockWithVBHeap;
		uint32												NumMeshVBufferStd;
		uint32												NumMeshVBufferHard;
		uint32												NumMeshMRMVBufferStd;
		uint32												NumMeshMRMVBufferHard;

		void	reset()
		{
			MeshMRMProfileTriVBFormat.clear();
			MeshProfileTriVBFormat.clear();
			NumMeshRdrNormal= 0;
			NumMeshRdrBlock= 0;
			NumMeshRdrBlockWithVBHeap= 0;
			NumMeshTriRdrNormal= 0;
			NumMeshTriRdrBlock= 0;
			NumMeshTriRdrBlockWithVBHeap= 0;
			NumMeshMRMRdrNormal= 0;
			NumMeshMRMRdrBlock= 0;
			NumMeshMRMRdrBlockWithVBHeap= 0;
			NumMeshMRMTriRdrNormal= 0;
			NumMeshMRMTriRdrBlock= 0;
			NumMeshMRMTriRdrBlockWithVBHeap= 0;
			NumMeshVBufferStd= 0;
			NumMeshVBufferHard= 0;
			NumMeshMRMVBufferStd= 0;
			NumMeshMRMVBufferHard= 0;
		}
	};

	/// Enable Profiling for the next render(). Reset All stats.
	virtual void				profileNextRender() =0;

	/// get The result of the profiling.
	virtual void				getProfileResults(CBenchResults &results) =0;

	/**	output (nlinfo) Stats for Usage of the QuadClip
	 */
	virtual void				profileQuadGridClipManager() =0;

	// @}


	/// \name ShadowMapping Options
	// @{
	/// return the current base ShadowMap TextureSize. Default is 64 texels.
	virtual	uint			getShadowMapTextureSize() const =0;
	/// must be a power of 2.
	virtual	void			setShadowMapTextureSize(uint size) =0;
	/// get the size of the blur (<=3 means number of fakeBlur).
	virtual	uint			getShadowMapBlurSize() const =0;
	/// set the size of the blur (<=3 means number of fakeBlur).
	virtual	void			setShadowMapBlurSize(uint bs) =0;
	/// Enable Polygon Smoothing
	virtual	void			enableShadowPolySmooth(bool enable) =0;
	/// get Enable Polygon Smoothing flag
	virtual	bool			getEnableShadowPolySmooth() const =0;

	/// ShadowMap Distance Fade Start (40 default)
	virtual	void			setShadowMapDistFadeStart(float dist) =0;
	virtual	float			getShadowMapDistFadeStart() const =0;
	/// ShadowMap Distance Fade end (50 default)
	virtual	void			setShadowMapDistFadeEnd(float dist) =0;
	virtual	float			getShadowMapDistFadeEnd() const =0;
	/// ShadowMap max Caster In Screen (optimize CPU/GPU)
	virtual	void			setShadowMapMaxCasterInScreen(uint num) =0;
	virtual	uint			getShadowMapMaxCasterInScreen() const =0;
	/// ShadowMap max Caster Around (optimize memory)
	virtual	void			setShadowMapMaxCasterAround(uint num) =0;
	virtual	uint			getShadowMapMaxCasterAround() const =0;

	/// The scene may have one special VisualCollisionManager which can manage some shadow receiving
	virtual	void			setVisualCollisionManagerForShadow(UVisualCollisionManager *vcm) =0;
	// @}

	// set a callback to know when a water surface is instantiated
	virtual void			setWaterCallback(IWaterSurfaceAddedCallback *wcb) = 0;
	virtual IWaterSurfaceAddedCallback *getWaterCallback() const = 0;


	virtual void			setLandscapePolyDrawingCallback(ILandscapePolyDrawingCallback *lpd) = 0;
	virtual ILandscapePolyDrawingCallback *getLandscapePolyDrawingCallback() const = 0;

	/** Setup transparency sorting
	  * \param maxPriority Defines the valid range for priority in the [0, n] interval. By default, there's no priority sorting (0 -> single priority, 255 -> 256 possible priorities)
	  *                    Objects with higher priority are displayed before any other object with lower priority,
	  *                    whatever their distance is.
	  * \param NbDistanceEntries Defines the granularity for distance sorting. A value of N with a view distance of D meters means
	  *                          that the sorting accuracy will be of D / N meters at worst (when visible objects occupy the whole distance range)
	  * NB : The memory allocated is a multiple of NumPriority * NbDistanceEntries * 2 (2 if because of water ordering)
	  */
	virtual void setupTransparencySorting(uint8 maxPriority = 0, uint NbDistanceEntries = 1024) = 0;

	/// \name Water envmaps
	// @{
	/** Set a water envmap to be used with water surfaces in that scene. Water envmap may be shared across several scene.
	  * The envmap should have been created from the same UDriver interface than the scene
	  */
	virtual void		  setWaterEnvMap(UWaterEnvMap *waterEnvMap) = 0;
	// Get currently used water envmap for that scene.
	virtual UWaterEnvMap *getWaterEnvMap() const = 0;
	/** Update water envmaps. Water textures that need to be updated includes UWaterEnvMap textures & Day/Night textures (as defined in the water material).
	  * Should be called at the beginning of the frame before anything is rendered.
	  */
	virtual void		  updateWaterEnvMaps(TGlobalAnimationTime time) = 0;
	// @}
};

} // NL3D

#endif // NL_U_SCENE_H

/* End of u_scene.h */
