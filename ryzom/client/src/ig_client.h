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



#ifndef CL_IG_CLIENT_H
#define CL_IG_CLIENT_H



#define INSTANCE_DOORS_FYROS_CITY			"Animated_door.ig"
#define INSTANCE_DOORS_FYROS_APPART			"appart_int_doors.ig"
#define INSTANCE_DOORS_FX					"FxDustDoor.ig"
#define INSTANCE_APPART						"apart.ig"
#define INSTANCE_CITY						"street.ig"
#define INSTANCE_TAVERNE					"taverne.ig"

#define DOOR_TAVERN							"portetavern"
#define DOOR_TAVERN_ANIM_OPEN_PART_ONE		"TavernDoor_R_open.anim"
#define DOOR_TAVERN_ANIM_OPEN_PART_TWO		"TavernDoor_L_open.anim"
#define DOOR_TAVERN_ANIM_CLOSE_PART_ONE		"TavernDoor_R_close.anim"
#define DOOR_TAVERN_ANIM_CLOSE_PART_TWO		"TavernDoor_L_close.anim"

#define DOOR_APPART							"porteappart"
#define DOOR_APPART_ANIM_OPEN_PART_ONE		"AppartDoor_R_open.anim"
#define DOOR_APPART_ANIM_OPEN_PART_TWO		"AppartDoor_L_open.anim"
#define DOOR_APPART_ANIM_CLOSE_PART_ONE		"AppartDoor_R_close.anim"
#define DOOR_APPART_ANIM_CLOSE_PART_TWO		"AppartDoor_L_close.anim"

#define DOOR_APPART_INT						"porteappartint"
#define DOOR_APPART_ANIM_INT_OPEN_PART_ONE	"AppartDoor_Int_R_open.anim"
#define DOOR_APPART_ANIM_INT_OPEN_PART_TWO	"AppartDoor_Int_L_open.anim"
#define DOOR_APPART_ANIM_INT_CLOSE_PART_ONE	"AppartDoor_Int_R_close.anim"
#define DOOR_APPART_ANIM_INT_CLOSE_PART_TWO	"AppartDoor_Int_L_close.anim"

#define HOUSE_PLANT_LITTLE_BELL				"plante_qui_bouge"
#define HOUSE_PLANT_LITTLE_BELL_MESH		"Fy_Acc_PlanteGrelot.shape"
#define HOUSE_PLANT_LITTLE_BELL_SKELETON	"Fy_Acc_PlanteGrelot.skel"
#define HOUSE_PLANT_LITTLE_BELL_ANIM1		"Fy_Acc_PlanteGrelot01a"
#define HOUSE_PLANT_LITTLE_BELL_ANIM2		"Fy_Acc_PlanteGrelot02a"
#define HOUSE_PLANT_LITTLE_BELL_ANIM3		"Fy_Acc_PlanteGrelot03a"

#define CITY_DOGA							"doga"
#define CITY_DOGA_MESH						"FY_MO_dag.shape"
#define CITY_DOGA_SKELETON					"FY_MO_dag.skel"
#define CITY_DOGA_ANIM1						"FY_MO_dag_couche"
#define CITY_DOGA_ANIM2						"FY_MO_dag_gratte"
#define CITY_DOGA_ANIM3						"FY_MO_dag_iidle"

#define TAVERN_ROTOR						"rotor"
#define TAVERN_ROTOR_ANIM					"rotor"

#define TAVERN_CONEROTOR					"rotorcone"
#define TAVERN_CONEROTOR_ANIM				"conerotor"


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
// 3d Interface.
#include "nel/3d/u_instance_group.h"
#include "nel/3d/landscapeig_manager.h"
// Std
#include <string>
#include <map>


///////////
// USING //
///////////
using NL3D::UInstanceGroup;
using std::string;
using std::map;


////////////
// GLOBAL //
////////////

// igs for the city
extern map<string, UInstanceGroup *> IGCity;

// Other igs. These are igs of villages that are currently loaded
extern map<string, UInstanceGroup *> IGLoaded;




extern	NL3D::CLandscapeIGManager		LandscapeIGManager;


///////////////
// FUNCTIONS //
///////////////
/// Initialize Instances Group
void initIG();

#endif // CL_IG_CLIENT_H

/* End of ig_client.h */
