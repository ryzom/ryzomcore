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
	virtual ~CInstanceGroupUser () NL_OVERRIDE;
	// Init with a scene.
	//bool load (const std::string &instanceGroup);

	// Init without a scene
	bool init (const std::string &instanceGroup, bool async= false);

private:
	// From UInstanceGroup
	void setTransformNameCallback (ITransformName *pTN) NL_OVERRIDE;
	void setAddRemoveInstanceCallback(IAddRemoveInstance *callback) NL_OVERRIDE;
	void setIGAddBeginCallback(IIGAddBegin *callback) NL_OVERRIDE;


	void addToScene (class UScene& scene, UDriver *driver, uint selectedTexture) NL_OVERRIDE;
	void addToScene (class CScene& scene, IDriver *driver, uint selectedTexture);

	void addToSceneAsync (class UScene& scene, UDriver *driver, uint selectedTexture) NL_OVERRIDE;
	TState getAddToSceneState () NL_OVERRIDE;
	void stopAddToSceneAsync () NL_OVERRIDE;

	virtual UInstance		getInstance (uint instanceNb) const NL_OVERRIDE;
	virtual void			setDistMax(uint instance, float dist) NL_OVERRIDE;
	virtual float			getDistMax(uint instance) const NL_OVERRIDE;
	virtual void		    setCoarseMeshDist(uint instance, float dist) NL_OVERRIDE;
	virtual float           getCoarseMeshDist(uint instance) const NL_OVERRIDE;


	void removeFromScene (class UScene& scene) NL_OVERRIDE;
	uint getNumInstance () const NL_OVERRIDE;
	const std::string& getShapeName (uint instanceNb) const NL_OVERRIDE;
	const std::string& getInstanceName (uint instanceNb) const NL_OVERRIDE;
	virtual void				getInstanceMatrix(uint instanceNb, NLMISC::CMatrix &dest) const NL_OVERRIDE;
	const NLMISC::CVector& getInstancePos (uint instanceNb) const NL_OVERRIDE;
	const NLMISC::CQuat& getInstanceRot (uint instanceNb) const NL_OVERRIDE;
	const NLMISC::CVector& getInstanceScale (uint instanceNb) const NL_OVERRIDE;
	UInstance getByName (const std::string& name) const NL_OVERRIDE;
	sint	  getIndexByName(const std::string &name) const NL_OVERRIDE;


	void setBlendShapeFactor (const std::string &bsName, float rFactor) NL_OVERRIDE;

	void createRoot (UScene &scene) NL_OVERRIDE;
	void setClusterSystemForInstances (UInstanceGroup *pClusterSystem) NL_OVERRIDE;
	bool linkToParentCluster(UInstanceGroup *father) NL_OVERRIDE;
	UInstanceGroup *getParentCluster() const NL_OVERRIDE;
	void getDynamicPortals (std::vector<std::string> &names) NL_OVERRIDE;
	void setDynamicPortal (std::string& name, bool opened) NL_OVERRIDE;
	bool getDynamicPortal (std::string& name) NL_OVERRIDE;


	void setPos (const NLMISC::CVector &pos) NL_OVERRIDE;
	void setRotQuat (const NLMISC::CQuat &q) NL_OVERRIDE;

	bool getStaticLightSetup(NLMISC::CRGBA sunAmbient, uint retrieverIdentifier, sint surfaceId, const NLMISC::CVector &localPos,
		std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient) NL_OVERRIDE;

	NLMISC::CVector getPos () NL_OVERRIDE;
	NLMISC::CQuat	getRotQuat () NL_OVERRIDE;

	// The real instance group
	CInstanceGroup	_InstanceGroup;
	// For access through getInstance() and getByName()
	std::map<std::string,CTransformShape*>	_InstanceMap;
	// Async stuff
	TState _AddToSceneState;
	UScene *_AddToSceneTempScene;
	UDriver *_AddToSceneTempDriver;

	virtual void			freezeHRC() NL_OVERRIDE;
	virtual void			unfreezeHRC() NL_OVERRIDE;

	virtual void			displayDebugClusters(UDriver *drv, UTextContext *txtCtx) NL_OVERRIDE;

	virtual bool			dontCastShadowForInterior(uint instance) const NL_OVERRIDE;
	virtual bool			dontCastShadowForExterior(uint instance) const NL_OVERRIDE;

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
