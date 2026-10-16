/**
 * \file export_ids.h
 * \brief The NeL export contract identifiers, defined once for every headless exporter and the
 * glTF writer: the NEL3D_APPDATA_* script AppData sub-ids (plugin_max/nel_mesh_lib/
 * export_appdata.h and the per-process maxscripts) and the special-object scene class ids the
 * node selection gates dispatch on. Previously each tool main carried its own subset — one copy
 * even held a wrong (unused) value — and the shape/glTF routes had to keep their copies in sync
 * by hand.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_MAX_EXPORT_COMMON_EXPORT_IDS_H
#define PIPELINE_MAX_EXPORT_COMMON_EXPORT_IDS_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>

// ---------------------------------------------------------------------------------------------
// NeL export script AppData sub-ids (keyed MAXSCRIPT_UTILITY_CLASS_ID / superclass 4128 — see
// appdata_util.h). Values follow the reference export_appdata.h / maxscripts; the maxscripts'
// DONOTEXPORT spelling is the one used here.

// Node/shape export flags
#define NEL3D_APPDATA_LOD_NAME_COUNT 1423062537
#define NEL3D_APPDATA_LOD_NAME 1423062538
#define NEL3D_APPDATA_LOD_BLEND_IN 1423062548
#define NEL3D_APPDATA_LOD_BLEND_OUT 1423062549
#define NEL3D_APPDATA_LOD_COARSE_MESH 1423062550
#define NEL3D_APPDATA_LOD_DYNAMIC_MESH 1423062551
#define NEL3D_APPDATA_LOD_DIST_MAX 1423062552
#define NEL3D_APPDATA_LOD_BLEND_LENGTH 1423062553
#define NEL3D_APPDATA_LOD_MRM 1423062554
#define NEL3D_APPDATA_LOD_SKIN_REDUCTION 1423062555
#define NEL3D_APPDATA_LOD_NB_LOD 1423062556
#define NEL3D_APPDATA_LOD_DIVISOR 1423062557
#define NEL3D_APPDATA_LOD_DISTANCE_FINEST 1423062558
#define NEL3D_APPDATA_LOD_DISTANCE_MIDDLE 1423062559
#define NEL3D_APPDATA_LOD_DISTANCE_COARSEST 1423062560
#define NEL3D_APPDATA_ACCEL 1423062561
#define NEL3D_APPDATA_ACCEL_DEFAULT 32
#define NEL3D_APPDATA_INSTANCE_NAME 1423062562
#define NEL3D_APPDATA_DONT_ADD_TO_SCENE 1423062563
#define NEL3D_APPDATA_IGNAME 1423062564
#define NEL3D_APPDATA_DONOTEXPORT 1423062565
#define NEL3D_APPDATA_EXPORT_NOTE_TRACK 1423062566
#define NEL3D_APPDATA_LUMELSIZEMUL 1423062567
#define NEL3D_APPDATA_SOFTSHADOW_RADIUS 1423062568
#define NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT 1.4f
#define NEL3D_APPDATA_SOFTSHADOW_CONELENGTH 1423062569
#define NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT 15.0f

// Vegetable process
#define NEL3D_APPDATA_VEGETABLE 1423062580
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND 1423062581
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED 1423062582
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED 1423062583
#define NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED 1423062584
#define NEL3D_APPDATA_BEND_CENTER 1423062585
#define NEL3D_APPDATA_BEND_FACTOR 1423062586
#define NEL3D_APPDATA_BEND_FACTOR_DEFAULT 1.0f
#define NEL3D_APPDATA_VEGETABLE_FORCE_BEST_SIDED_LIGHTING 1423062616

// Materials / lighting
#define NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS 1423062587
#define NEL3D_APPDATA_EXPORT_REALTIME_LIGHT 1423062588
#define NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION 1423062589
#define NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT 1423062590
#define NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT 1423062591
#define NEL3D_APPDATA_VERTEXPROGRAM_ID 1423062592
#define NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_INTERIOR 1423062636
#define NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_EXTERIOR 1423062637
#define NEL3D_APPDATA_EXPORT_LMC_ENABLED 1423062638
#define NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START 1423062639
#define NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START (1423062639 + 16)
#define NEL3D_APPDATA_REALTIME_AMBIENT_ADD_SUN 1423062672

// Collision / PACS / camera
#define NEL3D_APPDATA_COLLISION 1423062613
#define NEL3D_APPDATA_COLLISION_EXTERIOR 1423062614
#define NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION 1423062671

// Skeleton / character processes
#define NEL3D_APPDATA_EXPORT_SWT 1423062611
#define NEL3D_APPDATA_EXPORT_SWT_WEIGHT 1423062612
#define NEL3D_APPDATA_BONE_LOD_DISTANCE 1423062615
#define NEL3D_APPDATA_AUTOMATIC_ANIMATION 1423062617
#define NEL3D_APPDATA_CHARACTER_LOD 1423062618

// Remanence (sword-trail FX)
#define NEL3D_APPDATA_USE_REMANENCE 1423062631
#define NEL3D_APPDATA_REMANENCE_SLICE_NUMBER 1423062632
#define NEL3D_APPDATA_REMANENCE_SAMPLING_PERIOD 1423062633
#define NEL3D_APPDATA_REMANENCE_SHIFTING_TEXTURE 1423062634
#define NEL3D_APPDATA_REMANENCE_ROLLUP_RATIO 1423062635

// Interface meshes (border normal welding)
#define NEL3D_APPDATA_INTERFACE_FILE 1423062700
#define NEL3D_APPDATA_INTERFACE_THRESHOLD 1423062701
#define NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS 1423062702

// Animation process
#define NEL3D_APPDATA_EXPORT_NODE_ANIMATION 1423062800
#define NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME 1423062801
#define NEL3D_APPDATA_EXPORT_SSS_TRACK 1423062802

// Ig process extras (cluster/audio ids live in their own sub-id ranges)
#define NEL3D_APPDATA_INSTANCE_SHAPE 1970
#define NEL3D_APPDATA_OCC_MODEL 84682540
#define NEL3D_APPDATA_OPEN_OCC_MODEL 84682541
#define NEL3D_APPDATA_SOUND_GROUP 84682542
#define NEL3D_APPDATA_ENV_FX 84682543

// Lightmap plugin ids (nel_export lightmap rollup)
#define NEL3D_APPDATA_LM_ANIMATED_LIGHT 41654685
#define NEL3D_APPDATA_LM_ANIMATED 41654686
#define NEL3D_APPDATA_LM_LIGHT_GROUP 41654687

// Ligoscape ids (zone process)
#define NEL3D_APPDATA_ZONE_ROTATE 1266703978
#define NEL3D_APPDATA_ZONE_SYMMETRY 1266703979
#define NEL3D_APPDATA_LIGO_PASSABLE 1304892483
#define NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX 1342141818

// Zone painter (ui M16) — neighbor hints written on board-session save.
// Value is a single null-terminated script string (MAXSCRIPT_UTILITY_CLASS_ID / superclass
// 4128, same shape as every other NEL3D_APPDATA_* entry Max setAppData would produce):
//   v1|dx,dy:basename|dx,dy:basename|...
// dx,dy are integer cell offsets relative to the eligible zone's footprint origin;
// basename is the neighbor .max file basename without extension. Variable-length (no fixed
// 8-slot ring). Unknown future versions: readers ignore the entry. Chosen after a full-repo
// NEL3D_APPDATA id clash scan — 1423062900 was free (gap after EXPORT_SSS_TRACK 1423062802).
#define NEL3D_APPDATA_PAINTER_NEIGHBOR_HINTS 1423062900

// ---------------------------------------------------------------------------------------------
// Special-object scene class identities the selection gates dispatch on. Part-A-only ids (the
// part-B varies per object instance) as plain macros; full ids as CClassId constants.

#define CLASSID_PARTA_NEL_PS 0x58ce2893
#define CLASSID_PARTA_NEL_FLARE 0x4e913532
#define CLASSID_PARTA_NEL_WAVE_MAKER 0x77e24828
#define CLASSID_PARTA_XREF 0x92aab38c

namespace PMAX_EXPORT_IDS {

const NLMISC::CClassId CLASSID_PACS_BOX(0x7f374277, 0x5d3971df);
const NLMISC::CClassId CLASSID_PACS_CYL(0x62a56810, 0x4b3d601c);

// The custom UVW-mapping plugin ("Map Extender", mapext198m3.dlm — design §9 / §10z-seize)
const NLMISC::CClassId CLASSID_MAP_EXTENDER(0x2ec82081, 0x045a6271);

} /* namespace PMAX_EXPORT_IDS */

#endif /* PIPELINE_MAX_EXPORT_COMMON_EXPORT_IDS_H */

/* end of file */
