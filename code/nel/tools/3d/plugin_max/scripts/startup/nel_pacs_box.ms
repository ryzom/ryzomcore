-- NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
-- Copyright (C) 2010  Winch Gate Property Limited
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as
-- published by the Free Software Foundation, either version 3 of the
-- License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.
--
-- You should have received a copy of the GNU Affero General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.


plugin Geometry nel_pacs_box
name:"PACS Box"
category:"Nel Tools"
classID:#(0x7f374277, 0x5d3971df)
version:2.0
extends:Box
(
	parameters pblock rollout:param
	(
		Reaction type:#integer animatable:false ui:ui_reaction default:1
		EnterTrigger type:#boolean animatable:false ui:ui_enter_trigger default:false
		ExitTrigger type:#boolean animatable:false ui:ui_exit_trigger default:false
		OverlapTrigger type:#boolean animatable:false ui:ui_overlap_trigger default:false
		CollisionMask type:#integer animatable:false ui:ui_collision_mask default:0
		OcclusionMask type:#integer animatable:false ui:ui_occlusion_mask default:1
		Obstacle type:#boolean animatable:false ui:ui_obstacle default:true
		Absorbtion type:#float animatable:false ui:ui_absorbtion default:1
		UserData0 type:#integer animatable:false ui:ui_user_data_0 default:0
		UserData1 type:#integer animatable:false ui:ui_user_data_1 default:0
		UserData2 type:#integer animatable:false ui:ui_user_data_2 default:0
		UserData3 type:#integer animatable:false ui:ui_user_data_3 default:0
	)

	rollout param "PACS Parameters"
	(
		checkbox		ui_obstacle			"Obstacle"				type:#boolean default:true
		checkbox		ui_enter_trigger	"Enter trigger"			type:#boolean default:false
		checkbox		ui_exit_trigger		"Exit trigger"			type:#boolean default:false
		checkbox		ui_overlap_trigger	"Overlap trigger"		type:#boolean default:false
		radiobuttons	ui_reaction			"Collision reaction"	labels:#("Do nothing", "Slide", "Reflexion", "Stop") default:1
		spinner			ui_collision_mask	"Collision mask"		range:[0,65535,0] type:#integer scale:1
		spinner			ui_occlusion_mask	"Occlusion mask"		range:[0,65535,1] type:#integer scale:1
		spinner			ui_absorbtion		"Absorbtion ratio"		range:[0,1,1] type:#float scale:0.01
		spinner			ui_user_data_0		"User data 0"			range:[0,65535,0] type:#integer scale:1
		spinner			ui_user_data_1		"User data 1"			range:[0,65535,0] type:#integer scale:1
		spinner			ui_user_data_2		"User data 2"			range:[0,65535,0] type:#integer scale:1
		spinner			ui_user_data_3		"User data 3"			range:[0,65535,0] type:#integer scale:1
	)
)
