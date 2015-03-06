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

#ifndef NL_SCENE_H
#define NL_SCENE_H


#include "nel/3d/camera.h"
#include "nel/3d/shape.h"
#include "nel/3d/animated_lightmap.h"
#include "nel/3d/play_list_manager.h"
#include "nel/3d/quad_grid_clip_manager.h"
#include "nel/3d/particle_system_manager.h"
#include "nel/3d/animation_set.h"
// The traversals
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/load_balancing_trav.h"
#include "nel/3d/light_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/flare_model.h"

#include "nel/3d/viewport.h"
#include "nel/3d/u_scene.h"

#include "nel/misc/rgba.h"
#include "nel/misc/smart_ptr.h"

// Debug : Sept 01 2006
#include "nel/3d/skeleton_spawn_script.h"

#include <map>
#include <list>

/// This namespace contains all 3D class
namespace NL3D
{

using	NLMISC::CRefPtr;
using	NLMISC::CSmartPtr;

class	CTransform;
class	CTransformShape;
class	IDriver;
class	CShapeBank;
class	CCoarseMeshManager;
class	CInstanceGroup;
class	CLodCharacterManager;
class	CAsyncTextureManager;
class	CSkeletonModel;
class	CRootModel;
class	CVisualCollisionManager;
class   CTextureCube;
class	CWaterEnvMap;

// ***************************************************************************
/**
 * A CScene, which own a list of Render Traversals, and a render() method.
 *
 * \b USER \b RULES:
 * - Before creating any CScene, call the cool method CScene::registerBasics(), to register basic models.
 * - Create a CScene (NB: may be static \c CScene \c scene;).
 * - call first initDefaultRoot() to create / register automatically the roots:
 * - set your driver for this scene with setDriver().
 * - create any other model with createModel() (such as a camera).
 * - Specify a Camera (SmartPtr-ed !!)
 * - render().
 *
 * CScene own those Traversals and those Root and kill them at ~CScene().
 *
 * Coordinate System:  right hand cordinates with: X to the right, Y to the far, Z to the top.
 *
 *           Z ^
 *             |      > Y
 *             |    /
 *             |  /
 *             |/
 *              -------> X
 *
 * \b Shape \b System:\n
 * The scene has an instance Management:
 *		- IShape  design the object that is instancied (a mesh as example).
 *		- ITransformShape is the instance, which smart_point to a IShape.
 *		- user can add shape manually in the scene CShapeBank with CShapeBank::add (), or remove them with CShapeBank::release ().
 *		- user create instance of a shape with CScene::createInstance(string shapeName);
 *			This create/load auto the shape if needed (searching in CPath, shapename must be a valid file name),
 *			and then create the instance, with help of IShape::createInstance().
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CScene
{
public:

	/// \name Basic registration.
	//@{
	/// Register Basic models
	static void		registerBasics();

	/**
	 * Register a model, indicating from which he derive.
	 * \param idModel the Unique Id of the registered model
	 * \param idModelBase the Unique Id of the base calss of the registered model
	 * \param creator the function which create the registered model.
	 */
	static	void	registerModel(const NLMISC::CClassId &idModel, const NLMISC::CClassId &idModelBase, CTransform* (*creator)());

	//@}


	/// \name Construction / destruction.
	//@{
	/// Constructor.
	CScene(bool bSmallScene);
	/// Destructor. release().
	~CScene();
	/// Create/setRoot the defaults models roots: a CTransform and a CLightGroup.
	void			initDefaultRoots();
	/// init QuadGridClipManager
	void			initQuadGridClipManager ();
	/// Set the driver to render Traversal.
	void			setDriver(IDriver *drv);
	/// Get the driver of render Traversal.
	IDriver			*getDriver() const;


	/** Release all relative to the scene (Models, traversals...)... Destroy the Basic traversals too.
	 *	The Lod Character Manager is reset() ed, but not deleted (at dtor only).
	 */
	void			release();
	//@}


	/// \name Render
	//@{
	/** Render the scene, from the CurrentCamera view.
	 *  This also update waiting instance that are loaded asynchronously (by calling updateWaitingInstances)
	 * NB: no Driver clear buffers (color or ZBuffer) are done....
	 * This call t->traverse() function to registered render traversal following their order given.
	 * NB: assert-crash if you are between a beginPartRender() and a endPartRender()
	 * \param doHrcPass set it to false to indicate that the CHrcTrav have not to be traversed. Useful to optimize if
	 * you know that NONE of your models have moved (a good example is a shoot of the scene from different cameras).
	 */
	void			render(bool	doHrcPass = true);

	/** Begin Part Rendering
	 *	During beginPartRender()/endPartRender(), you can ask other scene to render their part, but you should
	 *	avoid to create models or modify the scene (not fully tested)
	 *  WARNING: assert-crash if beetween a beginPartRender()/endPartRender()
	 */
	void			beginPartRender();
	/** Render a part (see render() for what it does)
	 *	beginPartRender() must have been called
	 * \param renderPart a combination of UScene::TRenderPart flags, allow to choose which part of the scene must be rendered
	 *	WARNING: always must begin rendering with at least UScene::RenderOpaque, else shadows won't work
	 *	WARNING: assert-crash if a part in 'rp' has already been rendered since the last beginPartRender()
	 */
	void			renderPart(UScene::TRenderPart rp, bool doHrcPass = true, bool doTrav = true, bool keepTrav = false);
	/** End Part Rendering (commit model creation and deletion that were asked during rendering)
	 */
	void			endPartRender(bool keepTrav = false);

	//@}


	/** Update instances that are loaded asynchronously
	  * \param systemTimeEllapsed : the time between 2 calls to updateWaitingInstances, in seconds
	  */
	void			updateWaitingInstances(double systemTimeEllapsed);


	/// \name Camera/Viewport.
	//@{
	/// Set/Get the current camera/Viewport.
	void			setCam(CCamera *cam) {CurrentCamera= cam;}
	CCamera			*getCam() {return CurrentCamera;}
	void			setViewport(const class CViewport& viewport)
	{
		_Viewport=viewport;
	}
	CViewport		getViewport()
	{
		return _Viewport;
	}

	/** Special For Camera Third person. Traverse the ClusterSystem with a Ray
	 *	(clip through portals, cluster system hierarchy...), to find where could lies
	 *	the camera at End point.
	 *	\param endPos may be modified to ensure that it lies in a cluster of the found cluster system
	 *	\return the found cluster System
	 */
	CInstanceGroup *findCameraClusterSystemFromRay(CInstanceGroup *startClusterSystem,
		const NLMISC::CVector &startPos, NLMISC::CVector &endPos);
	//@}


	/// \name Model mgt.
	//@{
	/**
	 * Create a model according to his type id.
	 * Model must has been previously registered via registerModel().
	 *
	 * Then, This function attach this model to the Root (for HRC and for Clip)
	 *
	 * Model are deleted with the deleteModel() or with release() which delete all models
	 * NB: Since CScene own the model, model MUST NOT be used with SmartPtrs (but CRefPtr always work...).
	 * \param idModel the Unique Id of the Model
	 * \return a valid model of the required type. NULL, if not found.
	 * \see deleteModel()
	 */
	CTransform		*createModel(const NLMISC::CClassId &idModel);

	/** Delete a model via his pointer.
	 * The model is automatically unlinked from all other model
	 *
	 * Once a model is deleted, all pointer to him should have been deleted.
	 */
	void			deleteModel(CTransform *model);

	//@}

	/// \name Instance Mgt.
	//@{
	/// Set the shape bank
	void			setShapeBank(CShapeBank*pShapeBank);

	/// Get the shape bank
	CShapeBank   *getShapeBank(void) { return _ShapeBank ; }

	/// Get the shape bank (const version)
	const CShapeBank   *getShapeBank(void) const { return _ShapeBank ; }

	/** Create a model, instance of the shape "shapename". If not present, try to load "shapename" via the CPath.
	 * If fails, return NULL.
	 */
	CTransformShape	*createInstance(const std::string &shapeName);

	/** Create an instance, if the shape is not present, load the shape asynchronously. The instance is really
	 * created when we process it in the rendering.
	 */
	void			createInstanceAsync(const std::string &shapeName, CTransformShape **pInstance, const NLMISC::CVector &position, uint selectedTexture);

	/** Delete an instance via his pointer. An instance is an entity which reference a shape.
	 */
	void			deleteInstance(CTransformShape*model);

	//@}

	CInstanceGroup *getGlobalInstanceGroup() { return _GlobalInstanceGroup;	}

	/// \name Light Mgt.
	//@{
	void	animate (TGlobalAnimationTime atTime);
	/// Add an IG for auto PointLight Factor animation. called by CInstanceGroup::addToScene()
	void	addInstanceGroupForLightAnimation(CInstanceGroup *ig);
	/// Remove an IG for auto PointLight Factor animation. called by CInstanceGroup::removeFromScene()
	void	removeInstanceGroupForLightAnimation(CInstanceGroup *ig);
	// TODO:
	// TurnOff/TurnOn
	// Animate
	//CLight* addStaticLight(CLight*); // Generate Light Map
	//void addDynamicLight(CLight*); // ??? Vertex lighting
	//@}



	/// get the current time of the scene, in second. It start from 0 (at the firt call of animate)
	TGlobalAnimationTime getCurrentTime(void) const { return _CurrentTime ; }

	/// get the ellapsed time (in second) between the last 2 calls of animate.
	TAnimationTime		getEllapsedTime(void) const { return _EllapsedTime ; }

	/** System time is a time that always run (independent from the animation time that run only on animate())
	 *	It is updated at beginning of render()
	 */
	double				getCurrentSystemTime() const {return _GlobalSystemTime;}

	/// get the ellapsed time (in second) between the last 2 calls of render().
	double				getEllapsedSystemTime() const { return _DeltaSystemTimeBetweenRender;}

	/// get the number of time render has been called
	uint64				getNumRender() const { return _NumRender; }

	/// true if currently rendering
	bool				isRendering() const {return _IsRendering;}

	/// \name LoadBalancing mgt.
	//@{

	/** Get the last face count asked from the instances before reduction.
	 *	It gets the sum of All groups.
	 */
	float					getNbFaceAsked () const;


	/** Set the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	void					setGroupLoadMaxPolygon(const std::string &group, uint nFaces);
	/** Get the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	uint					getGroupLoadMaxPolygon(const std::string &group);
	/** Get the last face count asked from the instances before reduction. only for the given group
	 *	return 0 if the Group does not exist.
	 */
	float					getGroupNbFaceAsked (const std::string &group) const;


	/** The mode of polygon balancing.
	 * PolygonBalancingOff => Models will be rendered with the number of faces they want to render.
	 * PolygonBalancingOn  => Models will be rendered with the number of faces the LoadBalancing want.
	 * PolygonBalancingClamp => Same as PolygonBalancingOn, but factor <= 1, ie models won't be rendered
	 *	with more face they want to render.
	 */
	enum			TPolygonBalancingMode {PolygonBalancingOff=0, PolygonBalancingOn, PolygonBalancingClamp, CountPolygonBalancing};

	/// Set the PolygonBalancingMode
	void					setPolygonBalancingMode(TPolygonBalancingMode polBalMode);
	/// Get the PolygonBalancingMode
	TPolygonBalancingMode	getPolygonBalancingMode() const;


	/** LoadBalancing for CLod and Skeletons.
	 *	Setup the max number of skeletons displayed in std way (ie not CLod). Default is 20.
	 */
	void					setMaxSkeletonsInNotCLodForm(uint m) {_MaxSkeletonsInNotCLodForm= m;}
	uint					getMaxSkeletonsInNotCLodForm() const {return _MaxSkeletonsInNotCLodForm;}

	//@}

	/// \name Coarse meshes managers.
	//@{
	CCoarseMeshManager		*getCoarseMeshManager ()
	{
		return _CoarseMeshManager;
	}

	/**
	 *	Each coarseMesh lighting will be updated every "period" frame. clamped to 1,255
	 */
	void					setCoarseMeshLightingUpdate(uint8 period);

	/// see setCoarseMeshLightingUpdate()
	uint8					getCoarseMeshLightingUpdate() const {return _CoarseMeshLightingUpdate;}

	/// Get the LodCharacterManager. NULL if user did not set it. (done in DriverUser)
	CLodCharacterManager	*getLodCharacterManager () const	{return _LodCharacterManager;}
	/// set the LodCharacterManager
	void					setLodCharacterManager(CLodCharacterManager	*m) {_LodCharacterManager= m;}

	//@}


	/// \name transparent Layer mgt
	//@{
		/** Set the order or rendering for transparent objects.
		  * In real case, with direct order, we have:
		  * - Underwater is rendered.
		  * - Water is rendered.
		  * - Objects above water are rendered.
		  */
		void  setLayersRenderingOrder(bool directOrder = true);
		bool  getLayersRenderingOrder() const;
	//@}


	/// \name Sun Light mgt
	//@{

	/** Enable Scene Lighting system. For backward compatibility, false by default.
	 *	If false, all objects will take last driver 's light setup
	 */
	void			enableLightingSystem(bool enable);

	/// see enableLightingSystem
	bool			isLightingSystemEnabled() const {return _LightingSystemEnabled;}

	/// set the global Ambient used for the scene. Default to (50, 50, 50).
	void			setAmbientGlobal(NLMISC::CRGBA ambient);
	/// set the Ambient of the Sun used for the scene.
	void			setSunAmbient(NLMISC::CRGBA ambient);
	/// set the Diffuse of the Sun used for the scene.
	void			setSunDiffuse(NLMISC::CRGBA diffuse);
	/// set the Specular of the Sun used for the scene.
	void			setSunSpecular(NLMISC::CRGBA specular);
	/// set the Direction of the Sun used for the scene.
	void			setSunDirection(const NLMISC::CVector &direction);
	/// set the color of a light group.
	void			setLightGroupColor(uint lightmapGroup, NLMISC::CRGBA color);


	/// get the global Ambient used for the scene. Default to (50, 50, 50).
	NLMISC::CRGBA	getAmbientGlobal() const;
	/// get the Ambient of the Sun used for the scene.
	NLMISC::CRGBA	getSunAmbient() const;
	/// get the Diffuse of the Sun used for the scene.
	NLMISC::CRGBA	getSunDiffuse() const;
	/// get the Specular of the Sun used for the scene.
	NLMISC::CRGBA	getSunSpecular() const;
	/// get the Direction of the Sun used for the scene.
	NLMISC::CVector	getSunDirection() const;
	/// get the number of light group.
	uint			getNumLightGroup () const
	{
		return (uint)_LightGroupColor.size ();
	}
	/// get the color of a lightmap group.
	NLMISC::CRGBA	getLightmapGroupColor(uint lightGroup) const
	{
		if (lightGroup<_LightGroupColor.size())
			return _LightGroupColor[lightGroup];
		else
			return CRGBA::White;
	}
	/// get an animated light factor.
	NLMISC::CRGBA	getAnimatedLightFactor (sint animatedLightmap, uint lightGroup) const
	{
		if (((uint)animatedLightmap)<_AnimatedLightPtr.size ())
		{
			return _AnimatedLightPtr[animatedLightmap]->getFactor (lightGroup);
		}
		else
		{
			return getLightmapGroupColor (lightGroup);
		}
	}
	/// get an animated lightmap index by name. Return -1 if not found.
	sint			getAnimatedLightNameToIndex (const std::string &name) const;

	/** setup the max number of point light that can influence a model. NB: clamped by NL3D_MAX_LIGHT_CONTRIBUTION
	 *	Default is 3.
	 *	NB: the sun contribution is not taken into account
	 */
	void		setMaxLightContribution(uint nlights);
	/** \see setMaxLightContribution()
	  */
	uint		getMaxLightContribution() const;

	/** Advanced. When a model is influenced by more light than allowed, or when it reach the limits
	 *	of the light (attenuationEnd), the light can be darkened according to some threshold.
	 *	The resultLightColor begin to fade when distModelToLight== attEnd- threshold*(attEnd-attBegin).
	 *	when distModelToLight== 0, resultLightColor==Black.
	 *	By default, this value is 0.1f. Setting higher values will smooth transition but will
	 *	generally darken the global effects of lights.
	 *	NB: clamp(value, 0, 1);
	 */
	void		setLightTransitionThreshold(float lightTransitionThreshold);
	/** \see getLightTransitionThreshold()
	  */
	float		getLightTransitionThreshold() const;


	//@}


	/// \name Weather mgt
	//@{

	/// Set the current windPower for all the scene. 0-1.
	void			setGlobalWindPower(float gwp);
	/// get the current windPower
	float			getGlobalWindPower() const {return _GlobalWindPower;}

	/// Set the current windDirection for all the scene. dir.z set to 0 and vector normalized.
	void			setGlobalWindDirection(const CVector &gwd);
	/// get the current windDirection
	const CVector	&getGlobalWindDirection() const {return _GlobalWindDirection;}

	//@}


	/// \name Trav accessor. Use it with caution. (used for mesh rendering)
	//@{
	CHrcTrav			&getHrcTrav() {return HrcTrav;}
	CClipTrav			&getClipTrav() {return ClipTrav;}
	CLightTrav			&getLightTrav() {return LightTrav;}
	CAnimDetailTrav		&getAnimDetailTrav() {return AnimDetailTrav;}
	CLoadBalancingTrav	&getLoadBalancingTrav() {return LoadBalancingTrav;}
	CRenderTrav			&getRenderTrav() {return RenderTrav;}

	/// Get the Root For Hrc and Basics Clip of the scene
	CTransform			*getRoot() const {return Root;}
	/// Get the RootCluster For Clip of the scene
	CCluster			*getRootCluster() const {return RootCluster;}

	//@}


	/// Get a ref. to the particle system manager. You shouldn't call this (has methods for private processing)
	CParticleSystemManager &getParticleSystemManager();


	/// set the automatic animation set used by this scene. It is stored as a smart pointer
	void				setAutomaticAnimationSet(CAnimationSet *as);

	/// Get a reference to the set of automatic animations
	CAnimationSet		*getAutomaticAnimationSet() const { return _AutomaticAnimationSet; }


	/// Get the async texture manager. NULL if was not setuped
	CAsyncTextureManager		*getAsyncTextureManager() const {return _AsyncTextureManager;}
	/// Set the async texture manager
	void						setAsyncTextureManager(CAsyncTextureManager *mgr) {_AsyncTextureManager= mgr;}

	/// \name Render Filtering
	// @{
	void				enableElementRender(UScene::TRenderFilter elt, bool state);
	uint32				getFilterRenderFlags() const {return _FilterRenderFlags;}
	// @}

	/// \name Private
	// @{
	/// The scene owns a list of skeleton models. Added/Removed by CSkeletonModel intModel()/dtor
	typedef std::list<CSkeletonModel*>		TSkeletonModelList;
	typedef TSkeletonModelList::iterator	ItSkeletonModelList;
	ItSkeletonModelList			appendSkeletonModelToList(CSkeletonModel *skel);
	void						eraseSkeletonModelToList(ItSkeletonModelList	it);
	ItSkeletonModelList			getSkeletonModelListBegin() {return _SkeletonModelList.begin();}
	ItSkeletonModelList			getSkeletonModelListEnd() {return _SkeletonModelList.end();}

	/// The scene owns a list of CTransform enabled to Cast shadows. Added/Removed by createShadowMap()/deleteShadowMap()
	typedef std::list<CTransform*>				TShadowCasterList;
	typedef TShadowCasterList::iterator			ItShadowCasterList;
	void						registerShadowCasterToList(CTransform *sc);
	void						unregisterShadowCasterToList(CTransform *sc);
	ItShadowCasterList			getShadowCasterListBegin() {return _ShadowCasterList.begin();}
	ItShadowCasterList			getShadowCasterListEnd() {return _ShadowCasterList.end();}

	/// For skeleton spawn script model creation
	void						addSSSModelRequest(const CSSSModelRequest &req);

	// @}


	/// \name Profiling
	// @{

	// Enable Profiling for the next render(). Reset All stats.
	void						profileNextRender();
	bool						isNextRenderProfile() const {return _NextRenderProfile;}

	// Result profiling
	UScene::CBenchResults		BenchRes;
	// increment with the current VBufferFormat
	void							incrementProfileTriVBFormat(std::map<uint32, uint32> &formatToTri, uint32 vbFormat, uint32 numTris)
	{
		std::map<uint32, uint32>::iterator	it= formatToTri.find(vbFormat);
		if(it==formatToTri.end())
		{
			it= formatToTri.insert(std::make_pair(vbFormat,0)).first;
		}
		it->second+= numTris;
	}

	// @}

	/** \name Flare contexts
	  * The flare objects are designed to work with a single scene, because they simulate 'retinian persistence' based on the visibility in the current scene.
	  * Several context allow to deals with a flare rendered from several points of views.
	  * There's a limited number of contexts (MaxNumFlareContexts)
	  */
	// @{
		// The max number of contexts for flares
		enum { MaxNumFlareContexts = CFlareModel::MaxNumContext };
		// Set the current context for flares. context must be < to MaxNumFlareContexts
		void	setFlareContext(uint context) { nlassert(context < MaxNumFlareContexts); _FlareContext = context; }
		// Get the current context for flares
		uint    getFlareContext() const { return _FlareContext; }
	// @}


	/// \name ShadowMapping Options
	// @{
	/// return the current base ShadowMap TextureSize. Default is 64 texels.
	uint			getShadowMapTextureSize() const {return _ShadowMapTextureSize;}
	/// must be a power of 2.
	void			setShadowMapTextureSize(uint size);
	/// get the size of the blur (<=3 means number of fakeBlur).
	uint			getShadowMapBlurSize() const {return _ShadowMapBlurSize;}
	/// set the size of the blur (<=3 means number of fakeBlur).
	void			setShadowMapBlurSize(uint bs);
	/// Enable Polygon Smoothing
	void			enableShadowPolySmooth(bool enable);
	/// get Enable Polygon Smoothing flag
	bool			getEnableShadowPolySmooth() const;

	/// ShadowMap Distance Fade Start (40 default)
	void			setShadowMapDistFadeStart(float dist);
	float			getShadowMapDistFadeStart() const {return _ShadowMapDistFadeStart;}
	/// ShadowMap Distance Fade end (50 default)
	void			setShadowMapDistFadeEnd(float dist);
	float			getShadowMapDistFadeEnd() const {return _ShadowMapDistFadeEnd;}
	/// ShadowMap max Caster In Screen (optimize CPU/GPU)
	void			setShadowMapMaxCasterInScreen(uint num);
	uint			getShadowMapMaxCasterInScreen() const {return _ShadowMapMaxCasterInScreen;}
	/// ShadowMap max Caster Around (optimize memory)
	void			setShadowMapMaxCasterAround(uint num);
	uint			getShadowMapMaxCasterAround() const {return _ShadowMapMaxCasterAround;}

	/// The scene may have one special VisualCollisionManager which can manage some shadow receiving
	void					setVisualCollisionManagerForShadow(CVisualCollisionManager *vcm) {_VisualCollisionManagerForShadow= vcm;}
	CVisualCollisionManager *getVisualCollisionManagerForShadow() const {return _VisualCollisionManagerForShadow;}
	// @}

	void			setWaterCallback(IWaterSurfaceAddedCallback *wcb) { _WaterCallback = wcb; }
	IWaterSurfaceAddedCallback *getWaterCallback() const { return _WaterCallback; }

	void			setLandscapePolyDrawingCallback(ILandscapePolyDrawingCallback *lpd)
	{ _PolyDrawingCallback = lpd;}
	ILandscapePolyDrawingCallback *getLandscapePolyDrawingCallback() const { return _PolyDrawingCallback;}

	// Add a new occlusion query object that must be tested at the end of the rendering
	void		    insertInOcclusionQueryList(CFlareModel *fm)
	{
		fm->Next = _FirstFlare;
		_FirstFlare = fm;
	}

	// debugging aid : draw all occlusion test mesh that are used by flares
	void			renderOcclusionTestMeshs();

	// Set a water envmap to be used with water surfaces in that scene. Water envmap may be shared accross several scenes.
	void		  setWaterEnvMap(CWaterEnvMap *waterEnvMap) { _WaterEnvMap = waterEnvMap; }
	// Get currenlty used water envmap for that scene.
	CWaterEnvMap *getWaterEnvMap() const { return _WaterEnvMap; }
	/** Update water envmaps. Water textures that need to be updated includes UWaterEnvMap textures & Day/Night textures (as defined in the water material).
	  * Should be called at the beginning of the frame before anything is rendered.
	  */
	void		  updateWaterEnvMaps(TGlobalAnimationTime time);
	// Get vertex buffer used for water rendering
	CVertexBuffer &getWaterVB() { return _WaterVB; }
private:

	/// The camera / Viewport.
	CRefPtr<CCamera>	CurrentCamera;
	CViewport		_Viewport;

	// the current time
	TGlobalAnimationTime  _CurrentTime ;

	// the real time
	TGlobalAnimationTime  _RealTime;
	TGlobalAnimationTime  _InitTime;

	// true when its the first call of animate
	bool _FirstAnimateCall ;

	// the ellapsed time
	TAnimationTime  _EllapsedTime ;

	// System time is a time that always run (independent from the animation time that run only on animate())
	double	_DeltaSystemTimeBetweenRender;
	double	_GlobalSystemTime;
	uint64  _NumRender; // the number of time render has been called


	/// \name The traversals
	//@{
	CHrcTrav			HrcTrav;
	CClipTrav			ClipTrav;
	CLightTrav			LightTrav;
	CAnimDetailTrav		AnimDetailTrav;
	CLoadBalancingTrav	LoadBalancingTrav;
	CRenderTrav			RenderTrav;
	//@}

	// The root models (will be deleted by CScene).
	CTransform			*Root;
	// The Root Cluster
	CCluster			*RootCluster;
	// This model is used to clip any model which has a Skeleton ancestor
	CRootModel			*SonsOfAncestorSkeletonModelGroup;


	// The Ligths automatic movements

	std::map<std::string, uint>					_AnimatedLightNameToIndex;
	std::list<CAnimatedLightmap>				_AnimatedLight;
	std::vector<CAnimatedLightmap*>				_AnimatedLightPtr;
	CPlayListManager _LMAnimsAuto;

	/* Lightmap factor for each lightmap group.
	 * size() must be == to CAnimatedLightmap::_GroupColor.size () */
	std::vector<NLMISC::CRGBA>		_LightGroupColor;

	// List of InstanceGroup to animate PointLightFactor.
	typedef std::set<CInstanceGroup*>	TAnimatedIgSet;
	typedef TAnimatedIgSet::iterator	ItAnimatedIgSet;
	TAnimatedIgSet	_AnimatedIgSet;

	/// \name Shape/Instances.
	//@{
	CShapeBank		*_ShapeBank;

	typedef std::multimap<std::string,CTransformShape**> TWaitingInstancesMMap;
	TWaitingInstancesMMap _WaitingInstances;

	CInstanceGroup* _GlobalInstanceGroup;
	//@}

	/// \name Coarse meshes managers.
	//@{
	// The Coarse Mesh Manager. Owned by the scene
	CCoarseMeshManager			*_CoarseMeshManager;
	// CLodCharacterManager is not a model. setuped by user
	CLodCharacterManager		*_LodCharacterManager;
	//@}

	/// \name Clip features
	//@{
	CQuadGridClipManager		*_QuadGridClipManager;
	//@}


	// Lighting.
	bool						_LightingSystemEnabled;

	/// set setCoarseMeshLightingUpdate
	uint8						_CoarseMeshLightingUpdate;

	/// \name Particle systems specific
	//@{
		CParticleSystemManager	_ParticleSystemManager;
	//@}


	/// \name Weather mgt
	//@{
	float						_GlobalWindPower;
	CVector						_GlobalWindDirection;
	//@}


	// A set of automatic animation
	NLMISC::CSmartPtr<CAnimationSet>			_AutomaticAnimationSet;


	// The async texture manager, setuped by the user.
	CAsyncTextureManager		*_AsyncTextureManager;


	// List of skeletons in the scene
	TSkeletonModelList			_SkeletonModelList;
	// Max Skeleton displayed as std.
	uint						_MaxSkeletonsInNotCLodForm;

	// Render filtering
	uint32						_FilterRenderFlags;

	// profile
	bool						_NextRenderProfile;

	IWaterSurfaceAddedCallback	*_WaterCallback;

	ILandscapePolyDrawingCallback * _PolyDrawingCallback;


	CFlareModel	*_FirstFlare; // first flare to test for occlusion query

	// List of shadow casters in the scene
	TShadowCasterList			_ShadowCasterList;

// ******************
private:
	struct	CModelEntry
	{
		NLMISC::CClassId	ModelId, BaseModelId;
		CTransform* (*Creator)();
		bool	operator<(const CModelEntry& o) const {return ModelId<o.ModelId;}
		bool	operator==(const CModelEntry& o) const {return ModelId==o.ModelId;}
		bool	operator!=(const CModelEntry& o) const {return !(*this==o);}
	};


private:
	// must do this for _UpdateModelList access.
	friend	class CTransform;
	friend	class CClipTrav;

	std::set<CTransform*>				_Models;
	CTransform							*_UpdateModelList;

	static std::set<CModelEntry>	_RegModels;

	/// Update all models. All dirty models are cleaned
	void			updateModels();

	// current context for rendering of flares
	uint			_FlareContext;

	// ShadowMap Options
	uint			_ShadowMapTextureSize;
	uint			_ShadowMapBlurSize;
	float			_ShadowMapDistFadeStart;
	float			_ShadowMapDistFadeEnd;
	uint			_ShadowMapMaxCasterInScreen;
	uint			_ShadowMapMaxCasterAround;
	CVisualCollisionManager		*_VisualCollisionManagerForShadow;

	// Delayed model delete
	bool			_IsRendering;
	std::vector<CTransform*>	_ToDelete;

	UScene::TRenderPart	_RenderedPart;
	void	renderOcclusionTestMeshsWithCurrMaterial();
	CWaterEnvMap	*_WaterEnvMap;
	/// Delayed model creation For skeleton spawn script animation
	std::vector<CSSSModelRequest>		_SSSModelRequests;
	void									flushSSSModelRequests();
	// common vb for water display
	CVertexBuffer	 _WaterVB;

	bool			_RequestParticlesAnimate;

};



}


#endif // NL_SCENE_H

/* End of scene.h */
