// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "ai_mgr_pet.h"
#include "time_interface.h"
#include "ai_grp_pet.h"
#include "ai_player.h"
#include "ai_mgr_npc.h"
#include "ai_bot_npc.h"
#include "server_share/animal_hunger.h"
#include "ai_profile_pet.h"

using namespace NLMISC;
using namespace NLNET;
using namespace RYAI_MAP_CRUNCH;
using namespace	AITYPES;

/****************************************************************************/
/* Methods definitions                                                      */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CMgrPet                                                                  //
//////////////////////////////////////////////////////////////////////////////

void CMgrPet::serviceEvent(CServiceEvent const& info)
{
	if (info.getServiceName()=="EGS" && info.getEventType()==CServiceEvent::SERVICE_DOWN)
	{
		despawnMgr(); //	despawn all groups.
		groups().setChildSize(0); //	remove all groups.
		_EntityIdToIndex.clear();
	}
}

void CMgrPet::update()
{
	// call the update methods for all of the groups in the manager
	uint32 const timeOffset = CTimeInterface::gameCycle();
	
	FOREACH(it, CCont<CGroup>, groups())
	{
		CGrpPet* const grpPet = NLMISC::safe_cast<CGrpPet*>(*it);
		
		if (grpPet->isSpawned() && ((grpPet->getChildIndex()+timeOffset)&3)==1)
			grpPet->getSpawnObj()->update();
		
		if (grpPet->bots().isEmpty())
			grpPet->getPetManager().removePetGroup(grpPet->getPetOwner());
	}
}

void CMgrPet::createPetGroup(CEntityId const& petOwnerId)
{
	nlassert(_EntityIdToIndex.find(petOwnerId)==_EntityIdToIndex.end());
	
	CSmartPtr<CGrpPet> const petGroup = new CGrpPet(this, petOwnerId);
	groups().addChild(petGroup);
	_EntityIdToIndex.insert(std::make_pair(petOwnerId,petGroup->getChildIndex()));
}

CGrpPet* CMgrPet::getPetGroup(CEntityId const& petOwnerId)
{
	CGrpPet* grp = NULL;
	TEntityIdMapUint32::iterator const it = _EntityIdToIndex.find(petOwnerId);
	
	if (it!=_EntityIdToIndex.end())
	{		
		grp = NLMISC::type_cast<CGrpPet*>(groups()[it->second]);
		if (!grp)
			_EntityIdToIndex.erase(it);
	}
	return grp;
}

void CMgrPet::removePetGroup(NLMISC::CEntityId const& petOwnerId)
{
	TEntityIdMapUint32::iterator const it = _EntityIdToIndex.find(petOwnerId);
	if (it==_EntityIdToIndex.end())
		return;
	
	groups().removeChildByIndex(it->second);
	_EntityIdToIndex.erase(it);
}

//////////////////////////////////////////////////////////////////////////////
// CBotPet                                                                  //
//////////////////////////////////////////////////////////////////////////////

void CBotPet::changeOwner(NLMISC::CEntityId const& newOwner)
{
	if (newOwner==CEntityId::Unknown)
		return;
	
	CMgrPet* const petMgr = getAIInstance()->getPetMgr();
	
	CGrpPet* grp = petMgr->getPetGroup(newOwner);
	
	if (!grp)
	{
		petMgr->createPetGroup(newOwner);
		grp = petMgr->getPetGroup(newOwner);
		if (!grp)
			return;
	}
	if (!grp->isSpawned())
	{
		grp->spawn();
		if (!grp->isSpawned())
			return;
	}
	
	//	To Update. It s Ugly and the correct way to do that may be:
	//	a method call on the new player bot container giving the child and the new player as params.
	//	it reflects that to have a full autonomous architecture, with should have a container / owner relationship.
	//	This allow us more easy code to write but leads in more dependency or type casting as container should be template
	//	of owner or owner should derive of a base type (right way).
	{
		CGrpPet* const lastOwner = static_cast<CGrpPet*>(getOwner());
		uint32 const lastIndex = getChildIndex();
		setOwner(grp);
		if (isSpawned())
			getSpawnObj()->setSpawnGroup(grp->getSpawnObj());
		grp->bots().addChild(this);
		lastOwner->bots().removeChildByIndex(lastIndex);
				
		//	REmove Group if no more used ..
		if (lastOwner->bots().size()==0)
			lastOwner->getPetManager().removePetGroup(lastOwner->getPetOwner());
	}
}

//////////////////////////////////////////////////////////////////////////////
// CPetOwner                                                                //
//////////////////////////////////////////////////////////////////////////////

CPetOwner::~CPetOwner()
{
	if (_petGroup.isNull())
		return;
	
	NLMISC::safe_cast<CMgrPet*>(_petGroup->getOwner())->groups().removeChildByIndex(_petGroup->getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// CPetSpawnMsgImp                                                          //
//////////////////////////////////////////////////////////////////////////////

void CPetSpawnMsgImp::callback(std::string const& name, NLNET::TServiceId id)
{
	CAIInstance* aiInstance = NULL;
	CPetSpawnConfirmationMsg confirmMsg;
	
	confirmMsg.CharacterMirrorRow = CharacterMirrorRow;
	confirmMsg.PetIdx = PetIdx;
	
	switch(SpawnMode)
	{
	case NEAR_PLAYER:
	case NEAR_POINT:
		{
			CAIInstance* const aiInstance = CAIS::instance().getAIInstance(AIInstanceId);	//	gets the AIInstance.
			CEntityId const petOwnerId = CMirrors::getEntityId(CharacterMirrorRow);
			
			if (petOwnerId.isUnknownId() || !aiInstance)
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::CHARATER_UNKNOWN;
				confirmMsg.send("EGS");
#ifdef NL_DEBUG
			nlwarning("Unknow player");
#endif
				return;
			}
			
			CGrpPet* petGrp = aiInstance->getPetMgr()->getPetGroup(petOwnerId);
			if (!petGrp)
			{
				aiInstance->getPetMgr()->createPetGroup(petOwnerId);
				petGrp = aiInstance->getPetMgr()->getPetGroup(petOwnerId);
			}
			
			if (!petGrp)
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::INTERNAL_ERROR;
				confirmMsg.send("EGS");
#ifdef NL_DEBUG
				nlerror("Group must exist, perhaps Pet manager is not initialised or the player id is wrong.");
#else
				nlwarning("Group must exist, perhaps Pet manager is not initialised or the player id is wrong.");
#endif
				return;
			}
			
			CBotPet* botPet = NULL;
			if (PetIdx<petGrp->bots().size())
				botPet = static_cast<CBotPet*>(petGrp->bots()[PetIdx]);
			
			if (botPet && botPet->isSpawned())
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::PET_ALREADY_SPAWNED;
				confirmMsg.send("EGS");
				return;
			}
			
			if (!petGrp->isSpawned())
				petGrp->spawn();
			botPet = safe_cast<CBotPet*>(petGrp->bots().addChild(new CBotPet(petGrp),PetIdx));
			if (!botPet) // creation failed.
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::INTERNAL_ERROR;	//	the creation failed coz of an internal error.
				confirmMsg.send("EGS");
#ifdef NL_DEBUG
				nlerror("Pet cannot be created, perhaps memory problems.");
#else
				nlwarning("Pet cannot be created, perhaps memory problems.");
#endif
				return;
			}
			
			//	calc a valid spawn position.
			{
				CAIPos position;
				CAIEntityPhysical const* const phys = CAIS::instance().getEntityPhysical(CMirrors::DataSet->getDataSetRow(petOwnerId));
				
				//	TSpawnMode
				switch (SpawnMode)
				{
				case NEAR_PLAYER:
					{
						if (phys)
						{
							position = phys->pos();
							CAngle angle = phys->theta	();
							double dist = 2 + (double)(botPet->getChildIndex()&3);
							position.setX(position.x() - CAICoord(dist*angle.asVector2d().x));
							position.setY(position.y() - CAICoord(dist*angle.asVector2d().y));
							break;
						}
						else
						{
							nlwarning("Trying to spawn a pet near a player %s that is not in this aiinstance. try to use Coordinates instead (NEAR_POINT).", petOwnerId.toString().c_str());
						}
					}
				case NEAR_POINT:
					position = CAIPos(Coordinate_X*0.001f, Coordinate_Y*0.001f, (sint32)(Coordinate_H*0.001f), Heading);
					break;
				}
				
				CWorldPosition spawnPos;
				if (!CWorldContainer::calcNearestWPosFromPosAnRadius(position.h(), spawnPos, position, 16, 300, CWorldContainer::CPosValidatorDefault()))
				{
					// try with an auto vertical pos
					if (!CWorldContainer::calcNearestWPosFromPosAnRadius(vp_auto, spawnPos, position, 16, 300, CWorldContainer::CPosValidatorDefault()))
					{
						confirmMsg.SpawnError = CPetSpawnConfirmationMsg::INTERNAL_ERROR;
						confirmMsg.send("EGS");
#ifdef NL_DEBUG
						nlstopex(("Pet cannot be created, positionnal problem at %s", position.toString().c_str()) );
#else
						nlwarning("Pet cannot be created, positional problem at %s", position.toString().c_str() );
#endif
						return;
					}
				}
				position = CAIPos(spawnPos.toAIVector(), spawnPos.getMetricHeight(), position.theta());
				botPet->setSpawnPos(position);
			}
			
			AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(PetSheetId);
			
			if (!sheet)
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::INTERNAL_ERROR;
				confirmMsg.send("EGS");
				return;
			}
			
			botPet->setSheet(sheet);

			if (!CustomName.empty())
			{
				botPet->setCustomName(CustomName);
			}
			
			if (!botPet->spawn())
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::INTERNAL_ERROR;
				confirmMsg.send("EGS");
#ifdef NL_DEBUG
				nlassert("Pet cannot be created, perhaps memory or positional problems.");
#else
				nlwarning("Pet cannot be created, perhaps memory or positional problems.");
#endif
				return;
			}
			
			if (!botPet->getSpawn())
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::INTERNAL_ERROR; // the creation failed coz of an internal error.
				confirmMsg.send("EGS");
#ifdef NL_DEBUG
				nlerror("Pet cannot be created, perhaps memory problems.");
#else
				nlwarning("Pet cannot be created, perhaps memory problems.");
#endif
				return;
			}

			botPet->getSpawn()->setAIProfile(new CAIPetProfileStand(botPet->getSpawn()));
			
			confirmMsg.PetMirrorRow = botPet->getSpawn()->dataSetRow();
			
			if (!confirmMsg.PetMirrorRow.isValid()) // creation failed.
			{
				confirmMsg.SpawnError = CPetSpawnConfirmationMsg::INTERNAL_ERROR;
				confirmMsg.send("EGS");
#ifdef NL_DEBUG
				nlerror("Pet cannot be created, perhaps memory problems.");
#else
				nlwarning("Pet cannot be created, perhaps memory problems.");
#endif
				return;
			}
			
			confirmMsg.SpawnError = CPetSpawnConfirmationMsg::NO_ERROR_SPAWN;
			confirmMsg.send("EGS");
		}
		break;
	default:
		//	not implemented ..
		confirmMsg.SpawnError = CPetSpawnConfirmationMsg::NOT_IMPLEMENTED;
		confirmMsg.send("EGS");
#ifdef NL_DEBUG
		nlerror("Unimplemented Message");
#else
		nlwarning("Unimplemented Message");
#endif
		return;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CPetSetOwnerImp                                                          //
//////////////////////////////////////////////////////////////////////////////

void CPetSetOwnerImp::callback(std::string const& name, NLNET::TServiceId id)
{
	CEntityId const& petOwner = CMirrors::getEntityId(OwnerMirrorRow);
	
	CSpawnBotPet* const petSpawnPtr = NLMISC::type_cast<CSpawnBotPet*>(CAIS::instance().getEntityPhysical(PetMirrorRow));
	if (!petSpawnPtr)
		return;
	
	petSpawnPtr->getPersistent().changeOwner(petOwner);
}

//////////////////////////////////////////////////////////////////////////////
// CPetCommandMsgImp                                                        //
//////////////////////////////////////////////////////////////////////////////

void CPetCommandMsgImp::callback(std::string const& name, NLNET::TServiceId id)
{
	CAIEntityPhysical* petEntPtr = CAIS::instance().getEntityPhysical(PetMirrorRow);
	if (!petEntPtr)
		return;
	
	CSpawnBotPet* petSpawnPtr = NLMISC::type_cast<CSpawnBotPet*>(petEntPtr);
	if (!petSpawnPtr)
		return;

	CAIPos const position(Coordinate_X*0.001f, Coordinate_Y*0.001f, Coordinate_H, 0);
	
	switch (Command)
	{
	case STAND:
		petSpawnPtr->setAIProfile(new CAIPetProfileStand(petSpawnPtr));
		break;
		
	case FOLLOW:
		petSpawnPtr->setAIProfile(new CAIPetProfileFollowPlayer(petSpawnPtr, CharacterMirrorRow));
		break;
		
	case GOTO_POINT:
		{
			CAIPetProfileGotoPoint* const profile = new CAIPetProfileGotoPoint(petSpawnPtr, position, petSpawnPtr->getAStarFlag());
			
			if (profile->isValid())
				petSpawnPtr->setAIProfile(profile);
			else
				CPetCommandConfirmationMsg(CPetCommandConfirmationMsg::POSITION_COLLISION_NOT_VALID, *this).send("EGS");
		}
		break;
		
	case GOTO_POINT_DESPAWN:
		{
			CAIPetProfileGotoPoint* const profile = new CAIPetProfileGotoPoint(petSpawnPtr, position, petSpawnPtr->getAStarFlag(), true);
			
			if (profile->isValid())
				petSpawnPtr->setAIProfile(profile);
			else
				CPetCommandConfirmationMsg(CPetCommandConfirmationMsg::POSITION_COLLISION_NOT_VALID,*this).send("EGS");
		}
		break;
		
	case LIBERATE:	//	not really implemented for now ..
		petSpawnPtr->getPersistent().setDespawn();
		break;
		
	case DESPAWN:
		petSpawnPtr->getPersistent().setDespawn();
		break;
	default:
		break;
	}
};
