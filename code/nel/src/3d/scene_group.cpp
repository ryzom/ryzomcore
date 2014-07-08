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

#include "nel/3d/scene_group.h"
#include "nel/misc/stream.h"
#include "nel/misc/matrix.h"
#include "nel/3d/scene.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/shape_bank.h"
#include "nel/3d/u_instance_group.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/text_context.h"
#include "nel/3d/water_model.h"
#include "nel/3d/water_shape.h"
#include "nel/misc/polygon.h"
#include "nel/misc/path.h"


using namespace NLMISC;
using namespace std;

namespace NL3D
{

// ---------------------------------------------------------------------------
// CInstance
// ---------------------------------------------------------------------------

// ***************************************************************************
CInstanceGroup::CInstance::CInstance ()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	DontAddToScene = false;
	AvoidStaticLightPreCompute= false;
	StaticLightEnabled= false;
	DontCastShadow= false;
	LocalAmbientId= 0xFF;
	DontCastShadowForInterior= false;
	DontCastShadowForExterior= false;
	Visible= true;
}

// ***************************************************************************
void CInstanceGroup::CInstance::serial (NLMISC::IStream& f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 7:
		- Visible
	Version 6:
		- DontCastShadowForExterior
	Version 5:
		- DontCastShadowForInterior
	Version 4:
		- LocalAmbientId.
	Version 3:
		- StaticLight.
	Version 2:
		- gameDev data.
	Version 1:
		- Clusters
	*/
	// Serial a version number
	sint version=f.serialVersion (7);


	// Visible
	if (version >= 7)
		f.serial(Visible);

	// DontCastShadowForExterior
	if (version >= 6)
		f.serial(DontCastShadowForExterior);
	else
		DontCastShadowForExterior= false;

	// DontCastShadowForInterior
	if (version >= 5)
		f.serial(DontCastShadowForInterior);
	else
		DontCastShadowForInterior= false;

	// Serial the LocalAmbientId.
	if (version >= 4)
	{
		f.serial(LocalAmbientId);
	}
	else if(f.isReading())
	{
		LocalAmbientId= 0xFF;
	}

	// Serial the StaticLight
	if (version >= 3)
	{
		f.serial (AvoidStaticLightPreCompute);
		f.serial (DontCastShadow);
		f.serial (StaticLightEnabled);
		f.serial (SunContribution);
		nlassert(CInstanceGroup::NumStaticLightPerInstance==2);
		f.serial (Light[0]);
		f.serial (Light[1]);
	}
	else if(f.isReading())
	{
		AvoidStaticLightPreCompute= false;
		StaticLightEnabled= false;
		DontCastShadow= false;
	}

	// Serial the gamedev data
	if (version >= 2)
	{
		f.serial (InstanceName);
		f.serial (DontAddToScene);
	}

	// Serial the clusters
	if (version >= 1)
		f.serialCont (Clusters);

	// Serial the name
	f.serial (Name);

	// Serial the position vector
	f.serial (Pos);

	// Serial the rotation vector
	f.serial (Rot);

	// Serial the scale vector
	f.serial (Scale);

	// Serial the parent location in the vector (-1 if no parent)
	f.serial (nParent);
}

// ---------------------------------------------------------------------------
// CInstanceGroup
// ---------------------------------------------------------------------------

// ***************************************************************************

uint CInstanceGroup::getNumInstance () const
{
	return (uint)_InstancesInfos.size();
}

// ***************************************************************************

const string& CInstanceGroup::getShapeName (uint instanceNb) const
{
	// Return the name of the n-th instance
	return _InstancesInfos[instanceNb].Name;
}

// ***************************************************************************

const string& CInstanceGroup::getInstanceName (uint instanceNb) const
{
	// Return the name of the n-th instance
	return _InstancesInfos[instanceNb].InstanceName;
}

// ***************************************************************************

const CVector& CInstanceGroup::getInstancePos (uint instanceNb) const
{
	// Return the position vector of the n-th instance
	return _InstancesInfos[instanceNb].Pos;
}

// ***************************************************************************

const CQuat& CInstanceGroup::getInstanceRot (uint instanceNb) const
{
	// Return the rotation vector of the n-th instance
	return _InstancesInfos[instanceNb].Rot;
}

// ***************************************************************************

const CVector& CInstanceGroup::getInstanceScale (uint instanceNb) const
{
	// Return the scale vector of the n-th instance
	return _InstancesInfos[instanceNb].Scale;
}

// ***************************************************************************

void CInstanceGroup::getInstanceMatrix(uint instanceNb,NLMISC::CMatrix &dest) const
{
	dest.identity();
	dest.translate(getInstancePos(instanceNb));
	dest.rotate(getInstanceRot(instanceNb));
	dest.scale(getInstanceScale(instanceNb));
}



// ***************************************************************************

sint32 CInstanceGroup::getInstanceParent (uint instanceNb) const
{
	// Return the scale vector of the n-th instance
	return _InstancesInfos[instanceNb].nParent;
}


// ***************************************************************************
const CInstanceGroup::CInstance		&CInstanceGroup::getInstance(uint instanceNb) const
{
	return _InstancesInfos[instanceNb];
}

// ***************************************************************************
CInstanceGroup::CInstance		&CInstanceGroup::getInstance(uint instanceNb)
{
	return _InstancesInfos[instanceNb];
}

// ***************************************************************************
CTransformShape				*CInstanceGroup::getTransformShape(uint instanceNb) const
{
	if(instanceNb>_Instances.size())
		return NULL;
	return _Instances[instanceNb];
}

// ***************************************************************************
CInstanceGroup::CInstanceGroup()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_IGSurfaceLight.setOwner(this);
	_GlobalPos = CVector(0,0,0);
	_Root = NULL;
	_ClusterSystemForInstances = NULL;
	_ParentClusterSystem = NULL;
	_RealTimeSunContribution= true;
	_AddToSceneState = StateNotAdded;
	_TransformName = NULL;
	_AddRemoveInstance = NULL;
	_IGAddBeginCallback = NULL;
	_UserIg= NULL;
}

// ***************************************************************************
CInstanceGroup::~CInstanceGroup()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

}

// ***************************************************************************
void CInstanceGroup::build (const CVector &vGlobalPos, const TInstanceArray& array,
							const std::vector<CCluster>& Clusters,
							const std::vector<CPortal>& Portals,
							const std::vector<CPointLightNamed> &pointLightList,
							const CIGSurfaceLight::TRetrieverGridMap *retrieverGridMap,
							float igSurfaceLightCellSize)
{
	_GlobalPos = vGlobalPos;
	// Copy the array
	_InstancesInfos = array;

	_Portals = Portals;
	_ClusterInfos = Clusters;

	// Link portals and clusters
	uint32 i, j, k;
	for (i = 0; i < _Portals.size(); ++i)
	{
		for (j = 0; j < _ClusterInfos.size(); ++j)
		{
			bool bPortalInCluster = true;
			for (k = 0; k < _Portals[i]._Poly.size(); ++k)
				if (!_ClusterInfos[j].isIn (_Portals[i]._Poly[k]) )
				{
					bPortalInCluster = false;
					break;
				}
			if (bPortalInCluster)
			{
				_Portals[i].setCluster(&_ClusterInfos[j]);
				_ClusterInfos[j].link (&_Portals[i]);
			}
		}
	}

	// Create Meta Cluster if needed
	/*
	CCluster clusterTemp;
	bool mustAdd = false;
	for (i = 0; i < _Portals.size(); ++i)
	if (_Portals[i].getNbCluster() == 1)
	{
		mustAdd = true;
		break;
	}
	if (mustAdd)
	{
		CCluster clusterTemp;
		_ClusterInfos.push_back(clusterTemp);
		CCluster *pMetaCluster = &_ClusterInfos[_ClusterInfos.size()-1];
		pMetaCluster->setMetaCluster();
		for (i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getNbCluster() == 1)
		{
			_Portals[i].setCluster(pMetaCluster);
			pMetaCluster->link(&_Portals[i]);
		}
	}*/


	// Build the list of light. NB: sort by LightGroupName the array.
	std::vector<uint>	plRemap;
	buildPointLightList(pointLightList, plRemap);

	// Build IgSurfaceLight
	// clear
	_IGSurfaceLight.clear();
	if(retrieverGridMap)
	{
		//build
		_IGSurfaceLight.build(*retrieverGridMap, igSurfaceLightCellSize, plRemap);
	}
}


// ***************************************************************************
void CInstanceGroup::build (const CVector &vGlobalPos, const TInstanceArray& array,
							const std::vector<CCluster>& Clusters,
							const std::vector<CPortal>& Portals)
{
	// empty pointLightList
	std::vector<CPointLightNamed> pointLightList;

	build(vGlobalPos, array, Clusters, Portals, pointLightList);
}


// ***************************************************************************
void CInstanceGroup::retrieve (CVector &vGlobalPos, TInstanceArray& array,
				std::vector<CCluster>& Clusters,
				std::vector<CPortal>& Portals,
				std::vector<CPointLightNamed> &pointLightList) const
{
	// Just copy infos. NB: light information order have change but is still valid
	vGlobalPos= _GlobalPos;
	array= _InstancesInfos;

	Portals= _Portals;
	Clusters= _ClusterInfos;
	// Must reset links to all portals and clusters.
	uint	i;
	for(i=0; i<Portals.size(); i++)
		Portals[i].resetClusterLinks();
	for(i=0; i<Clusters.size(); i++)
		Clusters[i].resetPortalLinks();


	pointLightList= getPointLightList();
}


// ***************************************************************************

void CInstanceGroup::serial (NLMISC::IStream& f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// Serial a header
	f.serialCheck (NELID("TPRG"));

	/*
	Version 5:
		_ _RealTimeSunContribution
	Version 4:
		_ IGSurfaceLight
	Version 3:
		- PointLights
	*/
	// Serial a version number
	sint version=f.serialVersion (5);


	// _RealTimeSunContribution
	if (version >= 5)
	{
		f.serial(_RealTimeSunContribution);
	}
	else if(f.isReading())
	{
		_RealTimeSunContribution= true;
	}


	// Serial the IGSurfaceLight
	if (version >= 4)
	{
		f.serial(_IGSurfaceLight);
	}
	else if(f.isReading())
	{
		_IGSurfaceLight.clear();
	}


	// Serial the PointLights info
	if (version >= 3)
	{
		f.serial(_PointLightArray);
	}
	else if(f.isReading())
	{
		_PointLightArray.clear();
	}


	if (version >= 2)
		f.serial(_GlobalPos);

	if (version >= 1)
	{
		f.serialCont (_ClusterInfos);
		f.serialCont (_Portals);
		// Links
		if (f.isReading())
		{
			uint32 i, j;
			for (i = 0; i < _ClusterInfos.size(); ++i)
			{
				uint32 nNbPortals;
				f.serial (nNbPortals);
				_ClusterInfos[i]._Portals.resize (nNbPortals);
				// Recreate clusters to portals links
				for (j = 0; j < nNbPortals; ++j)
				{
					sint32 nPortalNb;
					f.serial (nPortalNb);
					_ClusterInfos[i]._Portals[j] = &_Portals[nPortalNb];
					_Portals[nPortalNb].setCluster (&_ClusterInfos[i]);
				}
			}
		}
		else // We are writing to the stream
		{
			uint32 i, j;
			for (i = 0; i < _ClusterInfos.size(); ++i)
			{
				uint32 nNbPortals = (uint32)_ClusterInfos[i]._Portals.size();
				f.serial (nNbPortals);
				for (j = 0; j < nNbPortals; ++j)
				{
					sint32 nPortalNb = (sint32)(_ClusterInfos[i]._Portals[j] - &_Portals[0]);
					f.serial (nPortalNb);
				}
			}
		}
	}

	// Serial the array
	f.serialCont (_InstancesInfos);
}

// ***************************************************************************
void CInstanceGroup::createRoot (CScene& scene)
{
	_Root = (CTransform*)scene.createModel (TransformId);
	_Root->setDontUnfreezeChildren (true);
	setPos (CVector(0,0,0));
}

// ***************************************************************************
void CInstanceGroup::setTransformNameCallback (ITransformName *pTN)
{
	_TransformName = pTN;
}


// ***************************************************************************
void CInstanceGroup::setAddRemoveInstanceCallback(IAddRemoveInstance *callback)
{
	_AddRemoveInstance = callback;
}

// ***************************************************************************
void CInstanceGroup::setIGAddBeginCallback(IIGAddBegin *callback)
{
	_IGAddBeginCallback = callback;
}

// ***************************************************************************
bool CInstanceGroup::addToScene (CScene& scene, IDriver *driver, uint selectedTexture)
{
	// Init the scene lights
	_PointLightArray.initAnimatedLightIndex (scene);

	uint32 i, j;

	// Test if portals are linked to their 2 clusters
	for (i = 0; i < _Portals.size(); ++i)
	for (j = 0; j < 2; ++j)
	{
		if (_Portals[i]._Clusters[j] == NULL)
		{
			nlwarning("Portal %d (name:%s) is not linked to 2 clusters. Instance Group Not Added To Scene.", i, _Portals[i].getName().c_str());
		}
	}

	_Instances.resize (_InstancesInfos.size(), NULL);

	if (_IGAddBeginCallback)
		_IGAddBeginCallback->startAddingIG((uint)_InstancesInfos.size());

	// Creation and positionning of the new instance

	vector<CInstance>::iterator it = _InstancesInfos.begin();

	// Water surface may have a callback when they are created, and this callback need their position
	// Their position isn't set right now however, so must call that callback later
	IWaterSurfaceAddedCallback *oldCallback = scene.getWaterCallback();
	scene.setWaterCallback(NULL);
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		// Get the shape name
		string shapeName;
		getShapeName (i, shapeName);
		if (!shapeName.empty ())
		{
			if (!_InstancesInfos[i].DontAddToScene)
			{
				// TMP FIX : some pacs_prim files where exported ...
				if (nlstricmp(NLMISC::CFile::getExtension(shapeName), "pacs_prim") == 0)
				{
					nlwarning("Can't read %s (not a shape)", shapeName.c_str());
					_Instances[i] = NULL;
				}
				else
				{
					_Instances[i] = scene.createInstance (shapeName);
				}
				if( _Instances[i] == NULL )
				{
					nlwarning("Not found '%s' file", shapeName.c_str());
				}
			}
		}
	}
	scene.setWaterCallback(oldCallback);
	return addToSceneWhenAllShapesLoaded (scene, driver, selectedTexture);
}

// ***************************************************************************

void CInstanceGroup::getShapeName (uint instanceIndex, std::string &shapeName) const
{
	const CInstance &rInstanceInfo = _InstancesInfos[instanceIndex];
	shapeName = rInstanceInfo.Name;

	// If there is a callback added to this instance group then transform
	// the name of the shape to load.
	if (_TransformName != NULL && !rInstanceInfo.InstanceName.empty())
	{
		shapeName = _TransformName->transformName (instanceIndex, rInstanceInfo.InstanceName, rInstanceInfo.Name);
	}

	toLower(shapeName);
	if (!shapeName.empty() && shapeName.find('.') == std::string::npos)
		shapeName += ".shape";
}

// ***************************************************************************
// Private method
bool CInstanceGroup::addToSceneWhenAllShapesLoaded (CScene& scene, IDriver *driver, uint selectedTexture)
{
	uint32 i, j;

	vector<CInstance>::iterator it = _InstancesInfos.begin();
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CInstance &rInstanceInfo = *it;

		if (!rInstanceInfo.DontAddToScene)
		{
			if (_Instances[i])
			{
				_Instances[i]->setPos (rInstanceInfo.Pos);
				_Instances[i]->setRotQuat (rInstanceInfo.Rot);
				_Instances[i]->setScale (rInstanceInfo.Scale);
				_Instances[i]->setPivot (CVector::Null);
				if(rInstanceInfo.Visible)
					_Instances[i]->show();
				else
					_Instances[i]->hide();

				if (scene.getWaterCallback())
				{
					CWaterModel *wm = dynamic_cast<CWaterModel *>(_Instances[i]);
					if (wm)
					{
						const CWaterShape *ws = safe_cast<const CWaterShape *>((const IShape *) wm->Shape);
						scene.getWaterCallback()->waterSurfaceAdded(ws->getShape(), wm->getMatrix(), ws->isSplashEnabled(), ws->getUseSceneWaterEnvMap(0) || ws->getUseSceneWaterEnvMap(1));
					}
				}
				// Static Light Setup
				if( rInstanceInfo.StaticLightEnabled )
				{
					// Count lights.
					uint numPointLights;
					for(numPointLights= 0; numPointLights<CInstanceGroup::NumStaticLightPerInstance; numPointLights++)
					{
						if(rInstanceInfo.Light[numPointLights]==0xFF)
							break;
					}
					// Max allowed.
					numPointLights= min(numPointLights, (uint)NL3D_MAX_LIGHT_CONTRIBUTION);

					// Get pl ptrs.
					CPointLight		*pls[CInstanceGroup::NumStaticLightPerInstance];
					for(uint j=0; j<numPointLights;j++)
					{
						uint	plId= rInstanceInfo.Light[j];
						pls[j]= (CPointLight*)(&_PointLightArray.getPointLights()[plId]);
					}

					// get frozenAmbientlight.
					CPointLight *frozenAmbientlight;
					if(rInstanceInfo.LocalAmbientId == 0xFF)
						// must take the sun one.
						frozenAmbientlight= NULL;
					else
						// ok, take the local ambient one.
						frozenAmbientlight= (CPointLight*)(&_PointLightArray.getPointLights()[rInstanceInfo.LocalAmbientId]);

					// Setup the instance.
					_Instances[i]->freezeStaticLightSetup(pls, numPointLights, rInstanceInfo.SunContribution, frozenAmbientlight);
				}

				// Driver not NULL ?
				if (driver)
				{
					// Flush shape's texture with this driver
					_Instances[i]->Shape->flushTextures (*driver, selectedTexture);
				}
			}
		}
		else
		{
			_Instances[i] = NULL;
		}
	}

	// Setup the hierarchy
	// We just have to set the traversal HRC (Hierarchy)
	if (_Root == NULL)
	{
		createRoot (scene);
	}
	it = _InstancesInfos.begin();
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	if (!_InstancesInfos[i].DontAddToScene && _Instances[i] != NULL)
	{
		CInstance &rInstanceInfo = *it;
		if( rInstanceInfo.nParent != -1 ) // Is the instance get a parent
			_Instances[rInstanceInfo.nParent]->hrcLinkSon( _Instances[i] );
		else
			_Root->hrcLinkSon( _Instances[i] );
	}
	// Attach the root of the instance group to the root of the hierarchy traversal
	scene.getRoot()->hrcLinkSon( _Root );

	// Cluster / Portals
	// -----------------

	CClipTrav *pClipTrav = &scene.getClipTrav();
	_ClipTrav = pClipTrav;

	// Create the MOT links (create the physical clusters)
	_ClusterInstances.resize (_ClusterInfos.size());
	for (i = 0; i < _ClusterInstances.size(); ++i)
	{
		_ClusterInstances[i] = (CCluster*)scene.createModel (ClusterId);
		_ClusterInstances[i]->Group = this;
		_ClusterInstances[i]->_Portals = _ClusterInfos[i]._Portals;
		_ClusterInstances[i]->_LocalVolume = _ClusterInfos[i]._LocalVolume;
		_ClusterInstances[i]->_LocalBBox = _ClusterInfos[i]._LocalBBox;
		_ClusterInstances[i]->_Volume = _ClusterInfos[i]._Volume;
		_ClusterInstances[i]->_BBox = _ClusterInfos[i]._BBox;
		_ClusterInstances[i]->FatherVisible = _ClusterInfos[i].FatherVisible;
		_ClusterInstances[i]->VisibleFromFather = _ClusterInfos[i].VisibleFromFather;
		_ClusterInstances[i]->FatherAudible = _ClusterInfos[i].FatherAudible;
		_ClusterInstances[i]->AudibleFromFather = _ClusterInfos[i].AudibleFromFather;
		_ClusterInstances[i]->Name = _ClusterInfos[i].Name;
		_ClusterInstances[i]->setSoundGroup(_ClusterInfos[i].getSoundGroup());
		_ClusterInstances[i]->setEnvironmentFx(_ClusterInfos[i].getEnvironmentFx());
		pClipTrav->registerCluster (_ClusterInstances[i]);
		_ClusterInstances[i]->clipUnlinkFromAll();
	}

	// Relink portals with newly created clusters
	for (i = 0; i < _Portals.size(); ++i)
	for (j = 0; j < 2; ++j)
	{
		if (_Portals[i]._Clusters[j])
		{
			sint32 nClusterNb;
			nClusterNb = (sint32)(_Portals[i]._Clusters[j] - &_ClusterInfos[0]);
			_Portals[i]._Clusters[j] = _ClusterInstances[nClusterNb];
		}
	}

	// Link shapes to clusters
	for (i = 0; i < _Instances.size(); ++i)
	if (_Instances[i] != NULL && !_InstancesInfos[i].DontAddToScene)
	{
		if (_InstancesInfos[i].Clusters.size() > 0)
		{
			_Instances[i]->clipUnlinkFromAll();
			for (j = 0; j < _InstancesInfos[i].Clusters.size(); ++j)
				_ClusterInstances[_InstancesInfos[i].Clusters[j]]->clipAddChild( _Instances[i] );
			// For the first time we have to set all the instances to NOT move (and not be rebinded)
			_Instances[i]->freeze();
			_Instances[i]->setClusterSystem (this);
		}
		else
		{
			// These instances are not attached to a cluster at this level so we cannot freeze them
			// Moreover we must set their clustersystem they will be tested against
			_Instances[i]->setClusterSystem (_ClusterSystemForInstances);
		}
	}
	_Root->freeze();

	// HRC OBS like
	for (i = 0; i < _ClusterInstances.size(); ++i)
	{
		_ClusterInstances[i]->setWorldMatrix (_Root->getMatrix());

		for (j = 0; j < _ClusterInstances[i]->getNbPortals(); ++j)
		{
			CPortal *pPortal = _ClusterInstances[i]->getPortal(j);
			pPortal->setWorldMatrix (_Root->getMatrix());
		}

		// Re affect the cluster to the accelerator if not the root
		if (!_ClusterInstances[i]->isRoot())
		{
			_ClipTrav->unregisterCluster(_ClusterInstances[i]);
			_ClipTrav->registerCluster (_ClusterInstances[i]);
		}
	}


	// Link the instance group to the parent
	linkToParent (scene.getGlobalInstanceGroup());

	// Attach the clusters to the root of the instance group
	for (i = 0; i < _ClusterInstances.size(); ++i)
		_Root->hrcLinkSon( _ClusterInstances[i] );


	// Default: freezeHRC all instances.
	freezeHRC();


	// Register the instanceGroup for light animation
	// -----------------
	// If some PointLight to animate
	if(_PointLightArray.getPointLights().size() > 0)
		scene.addInstanceGroupForLightAnimation(this);

	_AddToSceneState = StateAdded;

	if (_AddRemoveInstance)
		_AddRemoveInstance->instanceGroupAdded();
	return true;
}

// ***************************************************************************
bool CInstanceGroup::addToSceneAsync (CScene& scene, IDriver *driver, uint selectedTexture)
{
	// Init the scene lights
	_PointLightArray.initAnimatedLightIndex (scene);

	uint32 i;

	_AddToSceneState = StateAdding;
	_AddToSceneTempScene = &scene;
	_AddToSceneTempDriver = driver;
	_AddToSceneTempSelectTexture = selectedTexture;

	_Instances.resize (_InstancesInfos.size(), NULL);

	if (_IGAddBeginCallback)
		_IGAddBeginCallback->startAddingIG((uint)_InstancesInfos.size());

	// Creation and positionning of the new instance

	vector<CInstance>::iterator it = _InstancesInfos.begin();
	set<string> allShapesToLoad;
	_AddToSceneSignal = false;
	bool loadAsyncStarted = false;
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CInstance &rInstanceInfo = *it;
		if (!rInstanceInfo.DontAddToScene)
		{
			string shapeName = rInstanceInfo.Name;
			if (_TransformName != NULL && !rInstanceInfo.InstanceName.empty())
			{
				shapeName = _TransformName->transformName (i, rInstanceInfo.InstanceName, rInstanceInfo.Name);
			}

			toLower(shapeName);

			if (!shapeName.empty() && shapeName.find('.') == std::string::npos)
				shapeName += ".shape";


			if (allShapesToLoad.find(shapeName) == allShapesToLoad.end())
			{
				allShapesToLoad.insert (shapeName);
				if (scene.getShapeBank()->getPresentState(shapeName) != CShapeBank::Present)
				{
					// Load it from file asynchronously
					scene.getShapeBank()->loadAsync (shapeName, scene.getDriver(), rInstanceInfo.Pos, &_AddToSceneSignal, selectedTexture);
					loadAsyncStarted = true;
				}
			}
		}
	}
	if (!loadAsyncStarted)
		_AddToSceneSignal = true;
	else
		_AddToSceneSignal = false;
	//CAsyncFileManager::getInstance().signal (&_AddToSceneSignal);

	return true;
}

// ***************************************************************************
void CInstanceGroup::stopAddToSceneAsync ()
{
	if (_AddToSceneState != StateAdding)
		return;
	vector<CInstance>::iterator it = _InstancesInfos.begin();
	CAsyncFileManager::getInstance().cancelSignal (&_AddToSceneSignal);
	for (uint32 i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CInstance &rInstanceInfo = *it;
		if (!rInstanceInfo.DontAddToScene)
		{
			string shapeName;


			bool getShapeName = true;

			if (_TransformName != NULL && !rInstanceInfo.InstanceName.empty())
			{
				shapeName = _TransformName->transformName (i, rInstanceInfo.InstanceName, rInstanceInfo.Name);
				if (shapeName != rInstanceInfo.Name)
					getShapeName = false;
			}


			if (getShapeName)
			{
				if (rInstanceInfo.Name.find('.') == std::string::npos)
					shapeName = rInstanceInfo.Name + ".shape";
				else	// extension has already been added
					shapeName  = rInstanceInfo.Name;
			}

			toLower(shapeName);
			_AddToSceneTempScene->getShapeBank()->cancelLoadAsync (shapeName);
		}
	}
	_AddToSceneState = StateNotAdded;
}

// ***************************************************************************
CInstanceGroup::TState CInstanceGroup::getAddToSceneState ()
{
	// If we are adding but we have finished loading shapes (all shapes are here)
	if (_AddToSceneState == StateAdding)
	{
		if (_AddToSceneSignal)
		{
			addToScene (*_AddToSceneTempScene, _AddToSceneTempDriver, _AddToSceneTempSelectTexture);
		}
	}
	return _AddToSceneState;
}

// ***************************************************************************
// Search in the hierarchy of ig the most low level (child) ig that contains the clusters that
// are flagged to be visible from father or which father is visible
bool CInstanceGroup::linkToParent (CInstanceGroup *pFather)
{
	uint32 i, j;
	bool ret;
/*
	for (i = 0; i < pFather->_ClusterInstances.size(); ++i)
	{
		for(j = 0; j < pFather->_ClusterInstances[i]->Children.size(); ++j)
		{
			if (linkToParent(pFather->_ClusterInstances[i]->Children[j]->Group))
				return true;
		}
	}
*/
	ret = false;
	if (this != pFather)
	{
		for (j = 0; j < this->_ClusterInstances.size(); ++j)
		{
			if ((this->_ClusterInstances[j]->FatherVisible) ||
				(this->_ClusterInstances[j]->VisibleFromFather))
			{
				for (i = 0; i < pFather->_ClusterInstances .size(); ++i)
				{
					// If my cluster j is in the cluster i of the father
					if (pFather->_ClusterInstances[i]->isIn(this->_ClusterInstances[j]->getBBox()))
					{
						if (this->_ClusterInstances[j]->Father != pFather->_ClusterInstances[i]) // and not already son of the father cluster ?
						{
							// unlink from parent
							this->_ClusterInstances[j]->unlinkFromParent();

							// relink to the new father found
							pFather->_ClusterInstances[i]->Children.push_back(this->_ClusterInstances[j]);
							this->_ClusterInstances[j]->Father = pFather->_ClusterInstances[i];
						}
						ret = true;
					}
				}
			}
		}
	}

	// store new parent
	if(ret)
		_ParentClusterSystem= pFather;

	return ret;
}

// ***************************************************************************
bool CInstanceGroup::removeFromScene (CScene& scene)
{
	uint32 i, j, k;

	// Remove shapes
	for (i = 0; i < _Instances.size(); ++i)
	{
		CTransformShape *pTShape = _Instances[i];
		if(pTShape)
		{
			// For security, unfreeze any StaticLightSetup setuped.
			pTShape->unfreezeStaticLightSetup();
			// delete the instance
			scene.deleteInstance (pTShape);
			_Instances[i] = NULL;
		}
	}

	// Relink portals with old clusters
	for (i = 0; i < _Portals.size(); ++i)
	for (k = 0; k < 2; ++k)
	{
		if (_Portals[i]._Clusters[k])
		{
			for (j = 0; j < _ClusterInstances.size(); ++j)
				if( _Portals[i]._Clusters[k] == _ClusterInstances[j] )
					break;

			nlassert (j!=_ClusterInstances.size());
			_Portals[i]._Clusters[k] = &_ClusterInfos[j];
		}
	}

	// Remove clusters
	CClipTrav *pClipTrav = &scene.getClipTrav();
	for (i = 0; i < _ClusterInstances.size(); ++i)
	{
		pClipTrav->unregisterCluster (_ClusterInstances[i]);
		scene.deleteModel (_ClusterInstances[i]);
	}

	scene.deleteModel (_Root);
	_Root = NULL;


	// UnRegister the instanceGroup for light animation
	// -----------------
	// If some PointLight to animate
	if(_PointLightArray.getPointLights().size() > 0)
		scene.removeInstanceGroupForLightAnimation(this);

	if (_AddRemoveInstance)
		_AddRemoveInstance->instanceGroupRemoved();
	return true;
}


// ***************************************************************************
void CInstanceGroup::getLights( set<string> &LightNames )
{
	LightNames.clear();
	for( uint32 i = 0; i < _Instances.size(); ++i )
	{
		CMeshInstance *pMI = dynamic_cast<CMeshInstance*>(_Instances[i]);
		if( pMI != NULL )
		{
			uint32 nNbLM = pMI->getNbLightMap();
			for( uint32 j = 0; j < nNbLM; ++j )
			{
				string sTmp;
				pMI->getLightMapName( j, sTmp );
				set<string>::iterator itSet =  LightNames.find(sTmp);
				if( itSet == LightNames.end() )
					LightNames.insert( sTmp );
			}
		}
	}
}

// ***************************************************************************
void CInstanceGroup::getBlendShapes( set<string> &BlendShapeNames )
{
	BlendShapeNames.clear();
	for( uint32 i = 0; i < _Instances.size(); ++i )
	{
		CMeshBaseInstance *pMBI = dynamic_cast<CMeshBaseInstance*>(_Instances[i]);
		if (pMBI != NULL)
		{
			uint32 nNbBS = pMBI->getNbBlendShape();
			for( uint32 j = 0; j < nNbBS; ++j )
			{
				string sTmp;
				pMBI->getBlendShapeName( j, sTmp );
				set<string>::iterator itSet =  BlendShapeNames.find(sTmp);
				if( itSet == BlendShapeNames.end() )
					BlendShapeNames.insert( sTmp );
			}
		}
	}
}

// ***************************************************************************
void CInstanceGroup::setBlendShapeFactor( const string &BlendShapeName, float rFactor )
{
	for( uint32 i = 0; i < _Instances.size(); ++i )
	{
		CMeshBaseInstance *pMI = dynamic_cast<CMeshBaseInstance*>(_Instances[i]);
		if( pMI != NULL )
		{
			pMI->setBlendShapeFactor( BlendShapeName, rFactor );
		}
	}
}

// ***************************************************************************
void CInstanceGroup::addCluster(CCluster *pCluster)
{
	_ClusterInstances.push_back(pCluster);
}

// ***************************************************************************
void CInstanceGroup::setClusterSystemForInstances(CInstanceGroup *pIG)
{
	_ClusterSystemForInstances = pIG;
	for (uint32 i = 0; i < _Instances.size(); ++i)
		if (_Instances[i] && _InstancesInfos[i].Clusters.size() == 0)
			_Instances[i]->setClusterSystem (_ClusterSystemForInstances);
}

// ***************************************************************************
void CInstanceGroup::getDynamicPortals (std::vector<std::string> &names)
{
	for (uint32 i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getName() != "")
			names.push_back (_Portals[i].getName());
}

// ***************************************************************************
void CInstanceGroup::setDynamicPortal (std::string& name, bool opened)
{
	for (uint32 i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getName() == name)
			_Portals[i].open (opened);
}

// ***************************************************************************
bool CInstanceGroup::getDynamicPortal (std::string& name)
{
	for (uint32 i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getName() == name)
			return _Portals[i].isOpened ();
	return false;
}

// ***************************************************************************
void CInstanceGroup::setPos (const CVector &pos)
{
	if (_Root != NULL)
		/// \todo Make this work (precision): _Root->setPos (_GlobalPos+pos);
		_Root->setPos (pos);
}

// ***************************************************************************
void CInstanceGroup::setRotQuat (const CQuat &quat)
{
	if (_Root != NULL)
		_Root->setRotQuat (quat);
}

// ***************************************************************************
CVector CInstanceGroup::getPos ()
{
	if (_Root != NULL)
		return _Root->getPos ();
	else
		return CVector(0.0f, 0.0f, 0.0f);
}

// ***************************************************************************
CQuat CInstanceGroup::getRotQuat ()
{
	if (_Root != NULL)
		return _Root->getRotQuat ();
	else
		return CQuat();
}

// ***************************************************************************
void		CInstanceGroup::linkRoot (CScene &/* scene */, CTransform *father)
{
	if(_Root)
	{
		father->hrcLinkSon( _Root );
	}
}

// ***************************************************************************
void		CInstanceGroup::freezeHRC()
{
	// For all instances.
	for (uint i=0; i < _Instances.size(); i++)
	{
		if(_Instances[i])
			_Instances[i]->freezeHRC();
	}
	// and for root.
	_Root->freezeHRC();
}


// ***************************************************************************
void		CInstanceGroup::unfreezeHRC()
{
	// For all instances.
	for (uint i=0; i < _Instances.size(); i++)
	{
		if(_Instances[i])
			_Instances[i]->unfreezeHRC();
	}
	// and for root.
	_Root->unfreezeHRC();
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CInstanceGroup::buildPointLightList(const std::vector<CPointLightNamed> &pointLightList,
	std::vector<uint>	&plRemap)
{
	// build.
	_PointLightArray.build(pointLightList, plRemap);

	// remap Instance precalc lighted.
	for(uint i=0; i<_InstancesInfos.size(); i++)
	{
		CInstance	&inst= _InstancesInfos[i];
		// If the instance has no precomputed lighting, skip
		if(!inst.StaticLightEnabled)
			continue;

		// remap pointlights
		for(uint l=0; l<CInstanceGroup::NumStaticLightPerInstance; l++)
		{
			// If NULL light, break and continue to next instance
			if(inst.Light[l]== 0xFF)
				break;
			else
			{
				// Check good index.
				nlassert(inst.Light[l] < _PointLightArray.getPointLights().size());
				// Remap index, because of light sorting.
				inst.Light[l]= uint8(plRemap[inst.Light[l]]);
			}
		}

		// remap ambient light
		if(inst.LocalAmbientId!=0xFF)
		{
			nlassert(inst.LocalAmbientId < _PointLightArray.getPointLights().size());
			inst.LocalAmbientId= uint8(plRemap[inst.LocalAmbientId]);
		}
	}

}

// ***************************************************************************
void			CInstanceGroup::setPointLightFactor(const CScene &scene)
{
	_PointLightArray.setPointLightFactor(scene);
}


// ***************************************************************************
void			CInstanceGroup::enableRealTimeSunContribution(bool enable)
{
	_RealTimeSunContribution= enable;
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CInstanceGroup::displayDebugClusters(IDriver *drv, class CTextContext *txtCtx)
{
	uint	opacity= 50;
	CRGBA	colorCluster(255, 128, 255, uint8(opacity));
	// portals are drawn twice
	CRGBA	colorPortal(128, 255, 128, uint8(opacity/2));

	CMaterial		clusterMat;
	CMaterial		portalMat;
	CMaterial		lineMat;
	clusterMat.initUnlit();
	clusterMat.setBlend(true);
	clusterMat.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
	clusterMat.setZWrite(false);
	clusterMat.setDoubleSided(true);
	clusterMat.setColor(colorCluster);
	portalMat.initUnlit();
	portalMat.setBlend(true);
	portalMat.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
	portalMat.setZWrite(false);
	portalMat.setDoubleSided(true);
	portalMat.setColor(colorPortal);
	lineMat.initUnlit();
	lineMat.setBlend(true);
	lineMat.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
	lineMat.setZWrite(false);
	lineMat.setDoubleSided(true);
	lineMat.setColor(CRGBA(0,0,0,uint8(opacity)));


	// The geometry for each cluster
	CVertexBuffer	vb;
	// too big cluster won't be rendered
	const uint	maxVertices= 10000;
	vb.setVertexFormat(CVertexBuffer::PositionFlag);
	vb.setNumVertices(maxVertices);
	CIndexBuffer		clusterTriangles;
	CIndexBuffer		clusterLines;
	CIndexBuffer		portalTriangles;
	CIndexBuffer		portalLines;
	//
	clusterTriangles.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	clusterLines.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	portalTriangles.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	portalLines.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	//
	clusterTriangles.setNumIndexes(maxVertices*3);
	clusterLines.setNumIndexes(maxVertices*2);
	portalTriangles.setNumIndexes(maxVertices*3);
	portalLines.setNumIndexes(maxVertices*2);

	// setup identity matrix
	drv->setupModelMatrix(CMatrix::Identity);

	// For all clusters
	uint	i;
	for(i=0;i<_ClusterInstances.size();i++)
	{
		CCluster	*cluster= _ClusterInstances[i];
		if(cluster)
		{
			uint	numTotalVertices= 0;

			// **** Build a set of polys representing the volume (slow but debug!)
			static	std::vector<CPolygon>	polygons;
			polygons.clear();
			polygons.resize(cluster->_Volume.size());
			// for each plane, build the associated polygon
			uint	j;
			for(j=0;j<polygons.size();j++)
			{
				// Start with a big quad centered on bbox center
				CPlane	p= cluster->_Volume[j];
				p.normalize();
				CVector	quadCenter= p.project(cluster->_BBox.getCenter());

				// choose a basis on this plane
				CMatrix		mat;
				mat.setArbitraryRotK(p.getNormal());
				mat.setPos(quadCenter);

				// Build the initial Big quad
				CPolygon	&poly= polygons[j];
				poly.Vertices.resize(4);
				float	s= 10 * cluster->_BBox.getRadius();
				poly.Vertices[0]= mat * CVector(-s,-s,0);
				poly.Vertices[1]= mat * CVector(s,-s,0);
				poly.Vertices[2]= mat * CVector(s,s,0);
				poly.Vertices[3]= mat * CVector(-s,s,0);

				// clip this poly against all the other (ie not me) planes
				// This make this algo O(N2) but this is for debug....
				for(uint k=0;k<cluster->_Volume.size();k++)
				{
					if(j!=k)
					{
						poly.clip(&cluster->_Volume[k], 1);
					}
				}

				// count the number of vertices / triangles / lines to add
				if(poly.Vertices.size()>=3)
				{
					numTotalVertices+= (uint)poly.Vertices.size();
				}
			}

			// **** count the number of portals vertices
			for(j=0;j<cluster->_Portals.size();j++)
			{
				numTotalVertices+= (uint)cluster->_Portals[j]->_Poly.size();
			}

			// **** Draw those cluster polygons, and portals
			// too big clusters won't be rendered
			if(numTotalVertices<=maxVertices)
			{
				uint	iVert= 0;
				uint	j;

				// build the cluster geometry
				clusterTriangles.setNumIndexes(maxVertices*3);
				clusterLines.setNumIndexes(maxVertices*2);

				// Locks
				CVertexBufferReadWrite vba;
				vb.lock (vba);
				CIndexBufferReadWrite ibaCT;
				clusterTriangles.lock (ibaCT);
				CIndexBufferReadWrite ibaCL;
				clusterLines.lock (ibaCL);

				uint numTriIndexes = 0;
				uint numLineIndexes = 0;

				for(j=0;j<polygons.size();j++)
				{
					CPolygon	&poly= polygons[j];
					if(poly.Vertices.size()>=3)
					{
						uint	k;
						// add the vertices
						for(k=0;k<poly.Vertices.size();k++)
							vba.setVertexCoord(iVert+k, poly.Vertices[k]);

						// add the triangles
						for(k=0;k<poly.Vertices.size()-2;k++)
						{
							if (numTriIndexes<clusterTriangles.capacity())
							{
								ibaCT.setTri(numTriIndexes, iVert+0, iVert+k+1, iVert+k+2);
								numTriIndexes += 3;
							}
						}

						// add the lines
						for(k=0;k<poly.Vertices.size();k++)
						{
							if (numLineIndexes<clusterLines.capacity())
							{
								ibaCL.setLine(numLineIndexes, iVert+k, iVert+ ((k+1)%poly.Vertices.size()) );
								numLineIndexes += 2;
							}
						}

						iVert+= (uint)poly.Vertices.size();
					}
				}

				// Unlocks
				ibaCT.unlock ();
				ibaCL.unlock ();
				clusterTriangles.setNumIndexes(numTriIndexes);
				clusterLines.setNumIndexes(numLineIndexes);

				// build the portal geometry
				portalTriangles.setNumIndexes(maxVertices*3);
				portalLines.setNumIndexes(maxVertices*2);

				// Locks
				CIndexBufferReadWrite ibaPT;
				portalTriangles.lock (ibaPT);
				CIndexBufferReadWrite ibaPL;
				portalLines.lock (ibaPL);

				numTriIndexes = 0;
				numLineIndexes = 0;

				for(j=0;j<cluster->_Portals.size();j++)
				{
					std::vector<CVector>	&portalVerts= cluster->_Portals[j]->_Poly;
					if(portalVerts.size()>=3)
					{
						uint	k;
						// add the vertices
						for(k=0;k<portalVerts.size();k++)
							vba.setVertexCoord(iVert+k, portalVerts[k]);

						// add the triangles
						for(k=0;k<portalVerts.size()-2;k++)
						{
							if (numTriIndexes<clusterTriangles.capacity())
							{
								ibaPT.setTri(numTriIndexes, iVert+0, iVert+k+1, iVert+k+2);
								numTriIndexes += 3;
							}
						}

						// add the lines
						for(k=0;k<portalVerts.size();k++)
						{
							if (numTriIndexes<clusterTriangles.capacity())
							{
								ibaPL.setLine(numLineIndexes, iVert+k, iVert+ ((k+1)%portalVerts.size()) );
								numLineIndexes += 2;
							}
						}

						iVert+= (uint)portalVerts.size();
					}
				}

				// Unlock
				ibaPT.unlock ();
				ibaPL.unlock ();
				portalTriangles.setNumIndexes(numTriIndexes);
				portalLines.setNumIndexes(numLineIndexes);
				vba.unlock ();

				// render 2 pass with or without ZBuffer (for clearness)
				for(uint pass=0;pass<2;pass++)
				{
					if(pass==0)
					{
						clusterMat.setZFunc(CMaterial::always);
						portalMat.setZFunc(CMaterial::always);
						lineMat.setZFunc(CMaterial::always);
					}
					else
					{
						clusterMat.setZFunc(CMaterial::lessequal);
						portalMat.setZFunc(CMaterial::lessequal);
						lineMat.setZFunc(CMaterial::lessequal);
					}

					drv->activeVertexBuffer(vb);
					drv->activeIndexBuffer(clusterTriangles);
					drv->renderTriangles (clusterMat, 0, clusterTriangles.getNumIndexes()/3);
					drv->activeIndexBuffer(clusterLines);
					drv->renderLines (lineMat, 0, clusterLines.getNumIndexes()/2);
					drv->activeIndexBuffer(portalTriangles);
					drv->renderTriangles (portalMat, 0, portalTriangles.getNumIndexes()/3);
					drv->activeIndexBuffer(portalLines);
					drv->renderLines (lineMat, 0, portalLines.getNumIndexes()/2);
				}
			}

		}
	}

	// **** For all clusters, Draw the cluster name at center of the cluster
	if(txtCtx)
	{
		CComputedString computedStr;

		// bkup fontSize
		uint		bkFontSize;
		CMatrix		fontMatrix;
		bkFontSize= txtCtx->getFontSize();
		// to be readable
		txtCtx->setFontSize(24);

		// the font matrix
		fontMatrix.setRot(drv->getViewMatrix().inverted());
		fontMatrix.normalize(CMatrix::YZX);
		fontMatrix.scale(10);

		// parse all clusters
		for(i=0;i<_ClusterInstances.size();i++)
		{
			CCluster	*cluster= _ClusterInstances[i];
			if(cluster)
			{
				fontMatrix.setPos(cluster->_BBox.getCenter());
				txtCtx->computeString(cluster->Name, computedStr);
				computedStr.render3D(*drv, fontMatrix);
			}
		}

		// restore fontsize
		txtCtx->setFontSize(bkFontSize);
	}

}


} // NL3D
