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

#ifndef NL_INSTANCE_GROUP_USER_H
#define NL_INSTANCE_GROUP_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_instance_group.h"
#include "nel/3d/scene_group.h"


namespace NLMISC
{
	class CVector;
	class CQuat;
}

namespace NL3D
{

class UScene;
class UInstanceGroup;
class CInstanceUser;

/**
 * Implementation of the user interface managing instance groups.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CInstanceGroupUser : public UInstanceGroup
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

public:
	CInstanceGroupUser ();
	virtual ~CInstanceGroupUser ();
	// Init with a scene.
	//bool load (const std::string &instanceGroup);

	// Init without a scene
	bool init (const std::string &instanceGroup, bool async= false);

private:
	// From UInstanceGroup
	void setTransformNameCallback (ITransformName *pTN);
	void setAddRemoveInstanceCallback(IAddRemoveInstance *callback);
	void setIGAddBeginCallback(IIGAddBegin *callback);


	void addToScene (class UScene& scene, UDriver *driver, uint selectedTexture);
	void addToScene (class CScene& scene, IDriver *driver, uint selectedTexture);

	void addToSceneAsync (class UScene& scene, UDriver *driver, uint selectedTexture);
	TState getAddToSceneState ();
	void stopAddToSceneAsync ();

	virtual UInstance		getInstance (uint instanceNb) const;
	virtual void			setDistMax(uint instance, float dist);
	virtual float			getDistMax(uint instance) const;
	virtual void		    setCoarseMeshDist(uint instance, float dist);
	virtual float           getCoarseMeshDist(uint instance) const;


	void removeFromScene (class UScene& scene);
	uint getNumInstance () const;
	const std::string& getShapeName (uint instanceNb) const;
	const std::string& getInstanceName (uint instanceNb) const;
	virtual void				getInstanceMatrix(uint instanceNb, NLMISC::CMatrix &dest) const;
	const NLMISC::CVector& getInstancePos (uint instanceNb) const;
	const NLMISC::CQuat& getInstanceRot (uint instanceNb) const;
	const NLMISC::CVector& getInstanceScale (uint instanceNb) const;
	UInstance getByName (const std::string& name) const;
	sint	  getIndexByName(const std::string &name) const;


	void setBlendShapeFactor (const std::string &bsName, float rFactor);

	void createRoot (UScene &scene);
	void setClusterSystemForInstances (UInstanceGroup *pClusterSystem);
	bool linkToParentCluster(UInstanceGroup *father);
	UInstanceGroup *getParentCluster() const;
	void getDynamicPortals (std::vector<std::string> &names);
	void setDynamicPortal (std::string& name, bool opened);
	bool getDynamicPortal (std::string& name);


	void setPos (const NLMISC::CVector &pos);
	void setRotQuat (const NLMISC::CQuat &q);

	bool getStaticLightSetup(NLMISC::CRGBA sunAmbient, uint retrieverIdentifier, sint surfaceId, const NLMISC::CVector &localPos,
		std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient);

	NLMISC::CVector getPos ();
	NLMISC::CQuat	getRotQuat ();

	// The real instance group
	CInstanceGroup	_InstanceGroup;
	// For access through getInstance() and getByName()
	std::map<std::string,CTransformShape*>	_InstanceMap;
	// Async stuff
	TState _AddToSceneState;
	UScene *_AddToSceneTempScene;
	UDriver *_AddToSceneTempDriver;

	virtual void			freezeHRC();
	virtual void			unfreezeHRC();

	virtual void			displayDebugClusters(UDriver *drv, UTextContext *txtCtx);

	virtual bool			dontCastShadowForInterior(uint instance) const;
	virtual bool			dontCastShadowForExterior(uint instance) const;

	friend class CTransformUser;
	friend class CSceneUser;

	void		removeInstancesUser();

public:
	// Debug purpose only.
	CInstanceGroup	&getInternalIG()
	{
		return _InstanceGroup;
	}
};


} // NL3D


#endif // NL_INSTANCE_GROUP_USER_H

/* End of instance_group_user.h */
