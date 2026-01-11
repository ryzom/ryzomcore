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

#ifndef NL_SCENE_GROUP_H
#define NL_SCENE_GROUP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/portal.h"
#include "nel/3d/cluster.h"
#include "nel/3d/transform.h"
#include "nel/3d/point_light_named.h"
#include "nel/3d/point_light_named_array.h"
#include "nel/3d/ig_surface_light.h"
#include "nel/3d/clip_trav.h"

#include <vector>

namespace NLMISC
{
class CRGBA;
class IStream;
struct EStream;
class CMatrix;
}

namespace NL3D {

class CScene;
class CTransformShape;
class IDriver;
class ITransformName;
struct IAddRemoveInstance;
struct IIGAddBegin;

/**
  * A CInstanceGroup is a group of mesh instance and so composed by
  *  - A reference to a mesh (refered by the name)
  *  - The transformations to get it to the world
  *  - The parent
  *
  * This class can initialize a scene and be serialized.
  *
  * \author Matthieu Besson
  * \author Nevrax France
  * \date 2001
  */
class CInstanceGroup : public NLMISC::CRefCount
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

public:

	/// Should Never be changed
	enum	{NumStaticLightPerInstance= 2};
	enum TState { StateNotAdded = 0, StateAdding, StateAdded, StateError };

public:

	/**
	 * Instance part
	 */

	/// An element of the group.
	class CInstance
	{
	public:
		/// Name of the Shape Instance
		std::string Name;

		/// The parent instance (-1 if root)
		sint32 nParent;

		/// The clusters the instance belongs to
		std::vector<uint32> Clusters;

		/// Transformations
		NLMISC::CQuat Rot;
		NLMISC::CVector Pos;
		NLMISC::CVector Scale;

		// Name of the instance
		std::string InstanceName;

		/// If true, then the shape is even not loaded/displayed (used for shadow casting?)
		bool	DontAddToScene;

		/// If false, the shape is load but hid by default (NB: can be used for camera collision)
		bool	Visible;

		/// Precomputed Lighting.
		// If true (false by default), then the instance don't cast shadow (used by ig_lighter).
		bool	DontCastShadow;
		// If true (false by default), then the instance's lighting will not be precomputed.
		bool	AvoidStaticLightPreCompute;
		// This part is precomputed.
		bool	StaticLightEnabled;		// If false, then the instance 's lighting is not pre-computed.
		uint8 	SunContribution;		// Contribution of the Sun.
		// Ids of the lights. FF == disabled. if Lights[i]==FF, then all lights >=i are considered disabled.
		uint8	Light[NumStaticLightPerInstance];
		/** Id of the ambiant Light to take for this instance. Ambient light are stored too in ig->getPointLigths()
		 *	If 0xFF => take Ambient of the sun.
		 */
		uint8	LocalAmbientId;
		/** if true (false by default), the instance don't cast shadow, but ONLY FOR ig_lighter (ig_lighter_lib)
		 *	(zone_lighter and zone_ig_lighter ignore it).
		 *	This is a special trick for the "Matis Serre" where the exterior mesh cast shadow in the interior, but
		 *	is not visible in the interior in realTime because of cluster clipping.... omg :(
		 */
		bool	DontCastShadowForInterior;
		/** This is the opposite of DontCastShadowForInterior. zone_lighter or zone_ig_lighter test it and if true,
		 *	assume this instance dont cast shadow
		 */
		bool	DontCastShadowForExterior;


		/// Constructor
		CInstance ();

		/// Serial the instance
		void serial (NLMISC::IStream& f);
	};

	/// A vector of instance.
	typedef std::vector<CInstance> TInstanceArray;

	/// Remove all of these methods. For the moment DO NOT USE THEM !!!

	/// Get number of instance in this group
	uint					getNumInstance () const;

	/// Get the name of the mesh referenced
	const std::string&		getShapeName (uint instanceNb) const;

	/// Get the instance name
	const std::string&		getInstanceName (uint instanceNb) const;

	/// Get an instance position
	const NLMISC::CVector&	getInstancePos (uint instanceNb) const;

	/// Get an instance rotation
	const NLMISC::CQuat&	getInstanceRot (uint instanceNb) const;

	// Get an instance scale
	const NLMISC::CVector&	getInstanceScale (uint instanceNb) const;

	// Get instance matrix (no pivot added)
	void					getInstanceMatrix(uint instanceNb, NLMISC::CMatrix &dest) const;

	// Get the instance father (-1 if this is a root)
	sint32					getInstanceParent (uint instanceNb) const;

	// Get a const ref on the instance
	const CInstance			&getInstance(uint instanceNb) const;

	// Get a mutable ref on the instance
	CInstance				&getInstance(uint instanceNb);

	// Get the ig global pos
	const NLMISC::CVector &getGlobalPos() const { return _GlobalPos; }

	/** Get the instance added to the scene. NULL if instanceNb too big, if addToScene not called,
	 *	or if instance is DontAddToScene
	 */
	CTransformShape			*getTransformShape(uint instanceNb) const;

	/**
	 * Construct, serialize and link to scene
	 */

	CInstanceGroup();
	~CInstanceGroup();

	/** Build the group
	 *	Build with an empty list of light
	 */
	void build (const CVector &vGlobalPos, const TInstanceArray& array,
				const std::vector<CCluster>& Clusters,
				const std::vector<CPortal>& Portals);

	/** Build the group
	 *	Build also the list of light. NB: sort by LightGroupName the array.
	 *	Give also a ptr on a retrieverGridMap to build surfaces (if not NULL).
	 */
	void build (const CVector &vGlobalPos, const TInstanceArray& array,
				const std::vector<CCluster>& Clusters,
				const std::vector<CPortal>& Portals,
				const std::vector<CPointLightNamed> &pointLightList,
				const CIGSurfaceLight::TRetrieverGridMap *retrieverGridMap= NULL,
				float igSurfaceLightCellSize= 0);

	/** Retreive group information. NB: data may have changed, eg: order of lights.
	 */
	void retrieve (CVector &vGlobalPos, TInstanceArray& array,
				std::vector<CCluster>& Clusters,
				std::vector<CPortal>& Portals,
				std::vector<CPointLightNamed> &pointLightList) const;

	/// Serial the group
	void serial (NLMISC::IStream& f);

	/// Add all the instances to the scene
	void createRoot (CScene& scene);

	/// Setup the callback in charge of changing name at the addToScene moment
	void setTransformNameCallback (ITransformName *pTN);

	/// Set a callback to know when an instance has been added / removed
	void setAddRemoveInstanceCallback(IAddRemoveInstance *callback);

	/// Set a callback to know when an instance group is being created, and how many instances it contains
	void setIGAddBeginCallback(IIGAddBegin *callback);



	/**
	  * Add all the instances to the scene. By default, freezeHRC() those instances and the root.
	  *
	  * \param scene is the scene in which you want to add the instance group.
	  * \param driver is a pointer to a driver. If this pointer is not NULL, the textures used by
	  * the shapes will be preloaded in this driver. If the pointer is NULL (default), textures
	  * will ve loaded when the shape will be used.
	  */
	bool addToScene (CScene& scene, IDriver *driver=NULL, uint selectedTexture=0);
	bool addToSceneAsync (CScene& scene, IDriver *driver=NULL, uint selectedTexture=0);
	void stopAddToSceneAsync ();
	TState getAddToSceneState ();

	/**
	  * Get the instance name to create as written in the instance group.
	  */
	void getShapeName (uint instanceIndex, std::string &shapeName) const;

	/// User Interface related: yes it is ugly....
	void					setUserInterface(class UInstanceGroup *uig) {_UserIg= uig;}
	class UInstanceGroup	*getUserInterface() const {return _UserIg;}

	/** For Debug Display of clusters. The view matrix and frustum should have been setuped
	 *	NB: the ModelMatrix is modified by this method
	 */
	void					displayDebugClusters(IDriver *drv, class CTextContext *txtCtx);

private:
	bool addToSceneWhenAllShapesLoaded (CScene& scene, IDriver *driver, uint selectedTexture);

public:
	/// Remove all the instances from the scene
	bool removeFromScene (CScene& scene);


	/**
	 * LightMap part
	 */

	/// Get all lights (lightmaps) from an instance group
	void getLights (std::set<std::string> &LightNames);



	/**
	 * BlendShape part
	 */

	/// Get all the blendshapes from an instance group
	void getBlendShapes (std::set<std::string> &BlendShapeNames);

	/// Set the blendshape factor for the whole instance group (-100 -> 100)
	void setBlendShapeFactor (const std::string &BlendShapeName, float rFactor);


	/**
	 * Cluster/Portal system part
	 */

	/// To construct the cluster system by hand
	void addCluster (CCluster *pCluster);

	/// Set the cluster system to test for instances that are not in a cluster of this IG
	void setClusterSystemForInstances (CInstanceGroup *pIG);

	/// Get all dynamic portals of an instance group
	void getDynamicPortals (std::vector<std::string> &names);

	/// Set the state of a dynamic portal (true=opened, false=closed)
	void setDynamicPortal (std::string& name, bool opened);

	/// Get the state of a dynamic portal (true=opened, false=closed)
	bool getDynamicPortal (std::string& name);


	/**
	 * Transformation part
	 */

	/// link the root of the ig to a node. No-op if not added to scene. Pass NULL to reset by default
	void linkRoot (CScene &scene, CTransform *father);

	/// Set the position of the IG
	void setPos (const CVector &pos);

	/// Set the rotation of the IG
	void setRotQuat (const CQuat &quat);

	/// Get the position of the IG
	CVector getPos ();

	/// Get the rotation of the IG
	CQuat getRotQuat ();

	/// see CTransform::freezeHRC(). Do it for all instances (not clusters), and for the root of the IG.
	void		freezeHRC();

	/// see CTransform::unfreezeHRC(). Do it for all instances (not clusters), and for the root of the IG.
	void		unfreezeHRC();


public:

	/// \name RealTime lighting part
	// @{

	/// get the list of light. NB: the array is sorted by LightGroupName.
	const std::vector<CPointLightNamed> &getPointLightList() const {return _PointLightArray.getPointLights();}

	/// Get the number of point lights
	uint								 getNumPointLights() const { return (uint)_PointLightArray.getPointLights().size(); }

	/// Get a mutable ref on a point light named
	CPointLightNamed					&getPointLightNamed(uint index)
	{
		return _PointLightArray.getPointLights()[index];
	}

	/// set the Light factor for all pointLights "lightGroupName".
	void			setPointLightFactor(const CScene &scene);

	/// See CIGSurfaceLight::getStaticLightSetup()
	bool			getStaticLightSetup(NLMISC::CRGBA sunAmbient, uint retrieverIdentifier, sint surfaceId, const CVector &localPos,
		std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
	{
		return _IGSurfaceLight.getStaticLightSetup(sunAmbient, retrieverIdentifier, surfaceId, localPos,
			pointLightList, sunContribution, localAmbient);
	}

	/// Get the SurfaceLight info, for debug purposes.
	const CIGSurfaceLight	&getIGSurfaceLight() const {return _IGSurfaceLight;}

	/// Setuped at export, tells if the ig is touched by the sun. true by default.
	void			enableRealTimeSunContribution(bool enable);
	bool			getRealTimeSunContribution() const {return _RealTimeSunContribution;}


	// @}

	/** Look through all hierarchy our clusters that must be linked to our parent
	  */
	bool linkToParent (CInstanceGroup*pFather);

	/** Get the parent ClusterSystem
	 */
	CInstanceGroup	*getParentClusterSystem() const {return _ParentClusterSystem;}

public:

	TInstanceArray					_InstancesInfos;
	std::vector<CTransformShape*>	_Instances;

	std::vector<CPortal>	_Portals;
	std::vector<CCluster>	_ClusterInfos;
	std::vector<CCluster*>	_ClusterInstances;

	CTransform		*_Root;

	CClipTrav		*_ClipTrav;
	// The cluster system used to link unclustered instances (setClusterSystemForInstances)
	CInstanceGroup	*_ClusterSystemForInstances;
	// The cluster system parent of us (linkToParent() call)
	CInstanceGroup	*_ParentClusterSystem;

	NLMISC::CVector _GlobalPos;


private:
	/// \name PointLight part
	// @{

	/// RealTimeSunContribution. Used for ig_lighter and zone_ig_lighter
	bool							_RealTimeSunContribution;

	/// Array of pointLights
	CPointLightNamedArray			_PointLightArray;

	/// Build the list of light. NB: sort by LightGroupName the array, and return index remap.
	void			buildPointLightList(const std::vector<CPointLightNamed> &pointLightList,
										std::vector<uint>	&plRemap);

	///	The object used to light dynamic models in town and buildings
	CIGSurfaceLight					_IGSurfaceLight;

	// @}

	/// \name Async loading part
	// @{
	bool		_AddToSceneSignal;
	TState		_AddToSceneState;
	CScene		*_AddToSceneTempScene;
	IDriver		*_AddToSceneTempDriver;
	uint		_AddToSceneTempSelectTexture;
	// @}

	ITransformName       *_TransformName;
	IAddRemoveInstance   *_AddRemoveInstance;
	IIGAddBegin			 *_IGAddBeginCallback;

	// Yes this is ugly, but impossible otherwise with the actual user interface system...
	UInstanceGroup		 *_UserIg;
};


} // NL3D


#endif // NL_SCENE_GROUP_H

/* End of scene_group.h */
