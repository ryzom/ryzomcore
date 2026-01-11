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



#ifndef CL_PACS_CLIENT_H
#define CL_PACS_CLIENT_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
// Pacs Interface.
// Client.
#include "ig_client.h"
// std
#include <string>

// Size max of an entity in PACS (Trees and creatures)
#define RYZOM_ENTITY_SIZE_MAX 16

class CIGCallback; // collision with igs of the landscapes (trees..)

///////////
// USING //
///////////
using NL3D::UInstanceGroup;
using NLPACS::UMoveContainer;
using NLPACS::UGlobalRetriever;
using NLPACS::UGlobalPosition;

////////////
// GLOBAL //
////////////
// Global Move Container.
extern UMoveContainer	*PACS;
extern CIGCallback		*IGCallbacks;
extern UGlobalRetriever	*GR;
extern const float		LRRefeshRadius;
// World Images
extern const uint8 staticWI;	//  Static World Image
extern const uint8 dynamicWI;	// Dynamic World Image
// Collision Masks.
extern const NLPACS::UMovePrimitive::TCollisionMask MaskColNone;
extern const NLPACS::UMovePrimitive::TCollisionMask MaskColPlayer;
extern const NLPACS::UMovePrimitive::TCollisionMask MaskColNpc;
extern const NLPACS::UMovePrimitive::TCollisionMask MaskColDoor;
extern const NLPACS::UMovePrimitive::TCollisionMask MaskColAll;

extern const uint16 UserDataTree;
extern const uint16 UserDataLift;
extern const uint16 UserDataDoor;
extern const uint16 UserDataEntity;

///////////////
// FUNCTIONS //
///////////////
/// Initialize PACS.
void initPACS (const char* rbank, const char* gr, NLMISC::IProgressCallback &progress);
void releasePACS ();

/** Initialize landscape IG collisions
  */
void initLandscapeIGCallbacks();
/** release landscape IG collisions.
  */
void releaseLandscapeIGCallbacks();



/// Get the cluster from a global position.
UInstanceGroup *getCluster(const UGlobalPosition &gp);

/// PACS primitive manager
void	initPrimitiveBlocks();
void	addPacsPrim(const std::string &filename);
void	deletePrimitiveBlocks();

/// The pacs primitives
typedef CHashMap<std::string, NLPACS::UPrimitiveBlock *> TPacsPrimMap;
extern TPacsPrimMap	PacsPrims;

#endif // CL_PACS_CLIENT_H

/* End of pacs_client.h */
