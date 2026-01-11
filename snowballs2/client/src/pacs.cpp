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

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <vector>

#include <nel/pacs/u_retriever_bank.h>
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_move_primitive.h>

#include <nel/3d/u_scene.h>
#include <nel/3d/u_instance_group.h>
#include <nel/3d/u_visual_collision_manager.h>

#include <nel/3d/u_instance.h>

#include <nel/misc/vectord.h>
#include <nel/misc/config_file.h>

#include "snowballs_client.h"
#include "landscape.h"
#include "pacs.h"
#include "entities.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

namespace SBCLIENT {

extern ULandscape *Landscape;

/*******************************************************************
 *                             GLOBALS                             *
 *******************************************************************/

// The retriever bank used in the world
URetrieverBank *RetrieverBank = NULL;
// The global retriever used for pacs
UGlobalRetriever *GlobalRetriever = NULL;
// The move container used for dynamic collisions
UMoveContainer *MoveContainer = NULL;

// The collision manager for ground snapping
UVisualCollisionManager *VisualCollisionManager = NULL;

/*******************************************************************
 *                            COLLISIONS                           *
 *******************************************************************/

// The collision primitive for the instances in the landscape
static vector<UMovePrimitive *> _InstancesMovePrimitives;

//
// Functions
//

void initPACS()
{
	// check stuff we need for this to init correctly
	nlassert(Landscape);
	nlassert(ConfigFile);
	nlassert(Scene);

	// check stuff we can't have yet
	nlassert(!RetrieverBank);
	nlassert(!GlobalRetriever);
	nlassert(!MoveContainer);
	nlassert(!VisualCollisionManager);

	// init the global retriever
	RetrieverBank = URetrieverBank::createRetrieverBank(ConfigFile->getVar("RetrieverBankName").asString().c_str());
	nlassert(RetrieverBank);

	// create the retriever bank
	GlobalRetriever = UGlobalRetriever::createGlobalRetriever(ConfigFile->getVar("GlobalRetrieverName").asString().c_str(), RetrieverBank);
	nlassert(GlobalRetriever);

	// create the move primitive
	MoveContainer = UMoveContainer::createMoveContainer(GlobalRetriever, 100, 100, 6.0);
	nlassert(MoveContainer);

	// create a visual collision manager
	// this should not be in pacs, but this is too close to pacs to be put elsewhere
	// -- -- put it elsewhere anyways, the other code in this page can be made re-usable 
	//       to share between the client and the collision service.
	VisualCollisionManager = Scene->createVisualCollisionManager();
	nlassert(VisualCollisionManager);
	VisualCollisionManager->setLandscape(Landscape);

	// -- -- move this to snowballs specific game task
	// create a move primitive for each instance in the instance group
	for (uint j = 0; j < InstanceGroups.size(); ++j)
	{
		for (uint i = 0; i < InstanceGroups[j]->getNumInstance(); ++i)
		{
			UMovePrimitive *primitive = MoveContainer->addCollisionablePrimitive(0, 1);
			primitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
			primitive->setReactionType(UMovePrimitive::DoNothing);
			primitive->setTriggerType(UMovePrimitive::NotATrigger);
			primitive->setCollisionMask(OtherCollisionBit+SelfCollisionBit+SnowballCollisionBit);
			primitive->setOcclusionMask(StaticCollisionBit);
			primitive->setObstacle(true);

			// setup the radius of each mesh in the instance group
			string	name = InstanceGroups[j]->getShapeName(i);
			float rad;

			// -- -- improve this
			     if (strlwr(name) == "pi_po_igloo_a")		rad = 4.5f;
			else if (strlwr(name) == "pi_po_snowman_a")		rad = 1.0f;
			else if (strlwr(name) == "pi_po_pinetree_a")	rad = 2.0f;
			else if (strlwr(name) == "pi_po_tree_a")		rad = 2.0f;
			else if (strlwr(name) == "pi_po_pingoo_stat_a")	rad = 1.0f;
			else if (strlwr(name) == "pi_po_gnu_stat_a")	rad = 1.0f;
			else
			{
				rad = 2.0f;
				nlwarning ("Instance name '%s' doesn't have a good radius for collision", name.c_str());
			}

			primitive->setRadius(rad);
			primitive->setHeight(6.0f);

			primitive->insertInWorldImage(0);
			CVector	pos = InstanceGroups[j]->getInstancePos(i);
			primitive->setGlobalPosition(CVectorD(pos.x, pos.y, pos.z-1.5f), 0);
			_InstancesMovePrimitives.push_back(primitive);
		}
	}
}

void releasePACS()
{
	// delete all move primitives
	if (!MoveContainer) nlwarning("_InstancesMovePrimitives: !MoveContainer");
	else
	{
		vector<UMovePrimitive *>::iterator it(_InstancesMovePrimitives.begin()), end(_InstancesMovePrimitives.end());
		for (; it != end; ++it) MoveContainer->removePrimitive(*it);
		_InstancesMovePrimitives.clear();
	}

	// delete all allocated objects
	if (!GlobalRetriever) nlwarning("GlobalRetriever: !GlobalRetriever");
	else { UGlobalRetriever::deleteGlobalRetriever(GlobalRetriever); GlobalRetriever = NULL; }
	if (!RetrieverBank) nlwarning("RetrieverBank: !RetrieverBank");
	else { URetrieverBank::deleteRetrieverBank(RetrieverBank); RetrieverBank = NULL; }
	if (!MoveContainer) nlwarning("MoveContainer: !MoveContainer");
	else { UMoveContainer::deleteMoveContainer(MoveContainer); MoveContainer = NULL; }

	// delete the visual collision manager
	if (!Scene) nlwarning("VisualCollisionManager: !Scene");
	else if (!VisualCollisionManager) nlwarning("VisualCollisionManager: !VisualCollisionManager");
	else { Scene->deleteVisualCollisionManager(VisualCollisionManager); VisualCollisionManager = NULL; }
}

} /* namespace SBCLIENT */

/* end of file */
