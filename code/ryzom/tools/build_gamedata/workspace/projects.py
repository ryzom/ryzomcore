#!/usr/bin/python
# 
# \file projects.py
# \brief Projects configuration
# \date 2010-05-24-09-19-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Projects configuration.
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2010  Winch Gate Property Limited
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
# 
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 


ProductName = "ryzom"


ProjectsToProcess = [ ]

# Common asset export and build projects
ProjectsToProcess += [ "common/interface" ]
ProjectsToProcess += [ "common/objects" ]
ProjectsToProcess += [ "common/sfx" ]
ProjectsToProcess += [ "common/fauna" ]
ProjectsToProcess += [ "common/construction" ]
ProjectsToProcess += [ "common/outgame" ]
ProjectsToProcess += [ "common/sky" ]
ProjectsToProcess += [ "common/characters" ]
ProjectsToProcess += [ "common/characters_maps_hr" ]

# Common client data and leveldesign projects
ProjectsToProcess += [ "common/fonts" ]
ProjectsToProcess += [ "common/gamedev" ]
ProjectsToProcess += [ "common/leveldesign" ]
ProjectsToProcess += [ "common/data_common" ]
ProjectsToProcess += [ "common/exedll" ]
ProjectsToProcess += [ "common/cfg" ]

# Ecosystem projects
ProjectsToProcess += [ "ecosystems/desert" ]
ProjectsToProcess += [ "ecosystems/jungle" ]
ProjectsToProcess += [ "ecosystems/primes_racines" ]
ProjectsToProcess += [ "ecosystems/lacustre" ]

# Continent projects
ProjectsToProcess += [ "continents/fyros" ] # Note: dummy for shape and map export
ProjectsToProcess += [ "continents/matis" ] # Note: dummy for shape and map export
ProjectsToProcess += [ "continents/zorai" ] # Note: dummy for shape and map export
ProjectsToProcess += [ "continents/tryker" ] # Note: dummy for shape and map export
ProjectsToProcess += [ "continents/newbieland" ] # Note: must be after other continents due to dependencies on fy/ma/zo/tr
ProjectsToProcess += [ "continents/indoors" ] # Note: must be after other continents due to dependencies on fy/ma/zo/tr


InstallShardDataDirectories = [ ]

InstallShardDataCollisionsDirectories = [ ]
InstallShardDataCollisionsDirectories += [ "newbieland_ai" ]
InstallShardDataCollisionsDirectories += [ "newbieland_ig" ]
InstallShardDataCollisionsDirectories += [ "newbieland_pacs" ]
InstallShardDataCollisionsDirectories += [ "indoors_ai" ]
InstallShardDataCollisionsDirectories += [ "indoors_ig" ]
InstallShardDataCollisionsDirectories += [ "indoors_pacs" ]


InstallClientData = [ ]

ICMainCfg = { }
ICMainCfg["Name"] = "main_cfg"
ICMainCfg["UnpackTo"] = "cfg" # ->  = "./cfg/"
ICMainCfg["IsOptional"] = 0
ICMainCfg["IsIncremental"] = 0
ICMainCfg["Packages"] = [ [ "cfg", [ ] ] ]
InstallClientData += [ ICMainCfg ]

ICMainExedll = { }
ICMainExedll["Name"] = "main_exedll"
ICMainExedll["UnpackTo"] = "" # -> "./", to not unpack set to None
ICMainExedll["IsOptional"] = 0
ICMainExedll["IsIncremental"] = 0
ICMainExedll["Packages"] = [ [ "exedll", [ ] ] ]
InstallClientData += [ ICMainExedll ]

ICMainFonts = { }
ICMainFonts["Name"] = "main_fonts"
ICMainFonts["UnpackTo"] = "data/fonts"
ICMainFonts["IsOptional"] = 0
ICMainFonts["IsIncremental"] = 0
ICMainFonts["Packages"] = [ [ "fonts", [ ] ] ]
InstallClientData += [ ICMainFonts ]

ICMainPacked = { }
ICMainPacked["Name"] = "main_packed"
ICMainPacked["UnpackTo"] = "data"
ICMainPacked["IsOptional"] = 0
ICMainPacked["IsIncremental"] = 0
ICMainPacked["Packages"] = [ [ "packedsheets", [ ] ] ]
InstallClientData += [ ICMainPacked ]

ICUser = { }
ICUser["Name"] = "user"
ICUser["UnpackTo"] = "user"
ICUser["IsOptional"] = 0
ICUser["IsIncremental"] = 0
ICUser["Packages"] = [ [ "user", [ ] ] ]
InstallClientData += [ ICUser ]

ICExamples = { }
ICExamples["Name"] = "examples"
ICExamples["UnpackTo"] = "examples"
ICExamples["IsOptional"] = 0
ICExamples["IsIncremental"] = 0
ICExamples["Packages"] = [ [ "examples", [ ] ] ]
InstallClientData += [ ICExamples ]

ICCommon = { }
ICCommon["Name"] = "common"
ICCommon["UnpackTo"] = None
ICCommon["IsOptional"] = 0
ICCommon["IsIncremental"] = 1
ICCommon["Packages"] = [ ]
ICCommon["Packages"] += [ [ "sound", [ ] ] ]
ICCommon["Packages"] += [ [ "sky", [ ] ] ]
ICCommon["Packages"] += [ [ "sfx", [ ] ] ]
ICCommon["Packages"] += [ [ "objects", [ ] ] ]
ICCommon["Packages"] += [ [ "construction", [ ] ] ]
ICCommon["Packages"] += [ [ "outgame", [ ] ] ]
ICCommon["Packages"] += [ [ "leveldesign", [ ] ] ]
ICCommon["Packages"] += [ [ "interfaces", [ ] ] ]
ICCommon["Packages"] += [ [ "gamedev", [ ] ] ]
ICCommon["Packages"] += [ [ "data_common", [ ] ] ]
ICCommon["Packages"] += [ [ "fauna_swt", [ ] ] ]
ICCommon["Packages"] += [ [ "fauna_skeletons", [ ] ] ]
ICCommon["Packages"] += [ [ "fauna_shapes", [ ] ] ]
ICCommon["Packages"] += [ [ "fauna_maps", [ ] ] ]
ICCommon["Packages"] += [ [ "fauna_animations", [ ] ] ]
InstallClientData += [ ICCommon ]

ICCharacterArmors = [ ]
ICCharacterArmors += [ "zo_hom_visage" ]
ICCharacterArmors += [ "zo_hom_underwear" ]
ICCharacterArmors += [ "zo_hom_civil01" ]
ICCharacterArmors += [ "zo_hom_cheveux" ]
ICCharacterArmors += [ "zo_hom_caster01" ]
ICCharacterArmors += [ "zo_hom_armor01" ]
ICCharacterArmors += [ "zo_hom_armor00" ]
ICCharacterArmors += [ "zo_hof_visage" ]
ICCharacterArmors += [ "zo_hof_underwear" ]
ICCharacterArmors += [ "zo_hof_civil01" ]
ICCharacterArmors += [ "zo_hof_cheveux" ]
ICCharacterArmors += [ "zo_hof_caster01" ]
ICCharacterArmors += [ "zo_hof_armor01" ]
ICCharacterArmors += [ "zo_hof_armor00" ]
ICCharacterArmors += [ "zo_casque01" ]
ICCharacterArmors += [ "tr_hom_visage" ]
ICCharacterArmors += [ "tr_hom_underwear" ]
ICCharacterArmors += [ "tr_hom_refugee" ]
ICCharacterArmors += [ "tr_hom_civil01" ]
ICCharacterArmors += [ "tr_hom_cheveux" ]
ICCharacterArmors += [ "tr_hom_caster01" ]
ICCharacterArmors += [ "tr_hom_armor01" ]
ICCharacterArmors += [ "tr_hom_armor00" ]
ICCharacterArmors += [ "tr_hof_visage" ]
ICCharacterArmors += [ "tr_hof_underwear" ]
ICCharacterArmors += [ "tr_hof_refugee" ]
ICCharacterArmors += [ "tr_hof_civil01" ]
ICCharacterArmors += [ "tr_hof_cheveux" ]
ICCharacterArmors += [ "tr_hof_caster01" ]
ICCharacterArmors += [ "tr_hof_armor01" ]
ICCharacterArmors += [ "tr_hof_armor00" ]
ICCharacterArmors += [ "tr_casque01" ]
ICCharacterArmors += [ "ma_hom_visage" ]
ICCharacterArmors += [ "ma_hom_underwear" ]
ICCharacterArmors += [ "ma_hom_civil01" ]
ICCharacterArmors += [ "ma_hom_cheveux" ]
ICCharacterArmors += [ "ma_hom_caster01" ]
ICCharacterArmors += [ "ma_hom_armor01" ]
ICCharacterArmors += [ "ma_hom_armor00" ]
ICCharacterArmors += [ "ma_hof_visage" ]
ICCharacterArmors += [ "ma_hof_underwear" ]
ICCharacterArmors += [ "ma_hof_civil01" ]
ICCharacterArmors += [ "ma_hof_cheveux" ]
ICCharacterArmors += [ "ma_hof_caster01" ]
ICCharacterArmors += [ "ma_hof_casque01" ]
ICCharacterArmors += [ "ma_hof_armor04" ]
ICCharacterArmors += [ "ma_hof_armor01" ]
ICCharacterArmors += [ "ma_hof_armor00" ]
ICCharacterArmors += [ "fy_hom_visage" ]
ICCharacterArmors += [ "fy_hom_underwear" ]
ICCharacterArmors += [ "fy_hom_ruflaket" ]
ICCharacterArmors += [ "fy_hom_civil01" ]
ICCharacterArmors += [ "fy_hom_cheveux" ]
ICCharacterArmors += [ "fy_hom_caster01" ]
ICCharacterArmors += [ "fy_hom_barman" ]
ICCharacterArmors += [ "fy_hom_armor01" ]
ICCharacterArmors += [ "fy_hom_armor00" ]
ICCharacterArmors += [ "fy_hof_visage" ]
ICCharacterArmors += [ "fy_hof_underwear" ]
ICCharacterArmors += [ "fy_hof_civil01" ]
ICCharacterArmors += [ "fy_hof_cheveux" ]
ICCharacterArmors += [ "fy_hof_caster01" ]
ICCharacterArmors += [ "fy_hof_armor01" ]
ICCharacterArmors += [ "fy_hof_armor00" ]
ICCharacterArmors += [ "ge_hof_armor02" ]
ICCharacterArmors += [ "ge_hof_armor03" ]
ICCharacterArmors += [ "ge_hof_armor04" ]
ICCharacterArmors += [ "ge_hof_caster00" ]
ICCharacterArmors += [ "ge_hom_armor02" ]
ICCharacterArmors += [ "ge_hom_armor03" ]
ICCharacterArmors += [ "ge_hom_armor04" ]
ICCharacterArmors += [ "ge_hom_caster00" ]
ICCharacter = { }
ICCharacter["Name"] = "character"
ICCharacter["UnpackTo"] = None
ICCharacter["IsOptional"] = 0
ICCharacter["IsIncremental"] = 1
ICCharacter["Packages"] = [ ]
ICCharacter["Packages"] += [ [ "characters_swt", [ ] ] ]
ICCharacter["Packages"] += [ [ "characters_skeletons", [ ] ] ]
ICCharacter["Packages"] += [ [ "characters_shapes", [ ] ] ]
ICCharacter["Packages"] += [ [ "characters_animations", [ ] ] ]
ICCharacterMapsConditions = [ ]
for armor in ICCharacterArmors:
	ICCharacterMapsConditions += [ "-ifnot" ]
	ICCharacterMapsConditions += [ armor + "*" ]
ICCharacter["Packages"] += [ [ "characters_maps_hr", [ "characters_maps_hr.bnp" ] + ICCharacterMapsConditions, "characters.hlsbank" ] ]
for armor in ICCharacterArmors:
	ICCharacter["Packages"] += [ [ "characters_maps_hr", [ "characters_maps_" + armor + "_hr.bnp", "-if", armor + "*" ], "characters.hlsbank" ] ]
InstallClientData += [ ICCharacter ]

ICEsPrimesRacines = { }
ICEsPrimesRacines["Name"] = "es_primes_racines"
ICEsPrimesRacines["UnpackTo"] = None
ICEsPrimesRacines["IsOptional"] = 0
ICEsPrimesRacines["IsIncremental"] = 1
ICEsPrimesRacines["Packages"] = [ ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_vegetable_sets", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_vegetables", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_tiles", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_shapes", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_pacs_prim", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_maps", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_lightmaps", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_displaces", [ ] ] ]
ICEsPrimesRacines["Packages"] += [ [ "primes_racines_bank", [ ] ] ]
InstallClientData += [ ICEsPrimesRacines ]

ICEsDesert = { }
ICEsDesert["Name"] = "es_desert"
ICEsDesert["UnpackTo"] = None
ICEsDesert["IsOptional"] = 1
ICEsDesert["IsIncremental"] = 1
ICEsDesert["Packages"] = [ ]
ICEsDesert["Packages"] += [ [ "desert_vegetable_sets", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_vegetables", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_tiles", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_shapes", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_pacs_prim", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_maps", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_lightmaps", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_displaces", [ ] ] ]
ICEsDesert["Packages"] += [ [ "desert_bank", [ ] ] ]
InstallClientData += [ ICEsDesert ]

ICEsLacustre = { }
ICEsLacustre["Name"] = "es_lacustre"
ICEsLacustre["UnpackTo"] = None
ICEsLacustre["IsOptional"] = 1
ICEsLacustre["IsIncremental"] = 1
ICEsLacustre["Packages"] = [ ]
ICEsLacustre["Packages"] += [ [ "lacustre_vegetable_sets", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_vegetables", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_tiles", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_shapes", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_pacs_prim", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_maps", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_lightmaps", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_displaces", [ ] ] ]
ICEsLacustre["Packages"] += [ [ "lacustre_bank", [ ] ] ]
InstallClientData += [ ICEsLacustre ]

ICEsJungle = { }
ICEsJungle["Name"] = "es_jungle"
ICEsJungle["UnpackTo"] = None
ICEsJungle["IsOptional"] = 1
ICEsJungle["IsIncremental"] = 1
ICEsJungle["Packages"] = [ ]
ICEsJungle["Packages"] += [ [ "jungle_vegetable_sets", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_vegetables", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_tiles", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_shapes", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_pacs_prim", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_maps", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_lightmaps", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_displaces", [ ] ] ]
ICEsJungle["Packages"] += [ [ "jungle_bank", [ ] ] ]
InstallClientData += [ ICEsJungle ]

ICFyros = { }
ICFyros["Name"] = "fyros"
ICFyros["UnpackTo"] = None
ICFyros["IsOptional"] = 1
ICFyros["IsIncremental"] = 1
ICFyros["Packages"] = [ ]
ICFyros["Packages"] += [ [ "fyros_zones", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_shapes", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_pacs", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_maps", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_lightmaps", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_ig", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_newbie_zones", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_newbie_pacs", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_newbie_ig", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_island_zones", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_island_pacs", [ ] ] ]
ICFyros["Packages"] += [ [ "fyros_island_ig", [ ] ] ]
InstallClientData += [ ICFyros ]

ICMatis = { }
ICMatis["Name"] = "matis"
ICMatis["UnpackTo"] = None
ICMatis["IsOptional"] = 1
ICMatis["IsIncremental"] = 1
ICMatis["Packages"] = [ ]
ICMatis["Packages"] += [ [ "matis_zones", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_shapes", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_pacs", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_maps", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_lightmaps", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_ig", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_island_zones", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_island_pacs", [ ] ] ]
ICMatis["Packages"] += [ [ "matis_island_ig", [ ] ] ]
InstallClientData += [ ICMatis ]

ICZorai = { }
ICZorai["Name"] = "zorai"
ICZorai["UnpackTo"] = None
ICZorai["IsOptional"] = 1
ICZorai["IsIncremental"] = 1
ICZorai["Packages"] = [ ]
ICZorai["Packages"] += [ [ "zorai_zones", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_shapes", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_pacs", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_maps", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_lightmaps", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_ig", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_island_zones", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_island_pacs", [ ] ] ]
ICZorai["Packages"] += [ [ "zorai_island_ig", [ ] ] ]
InstallClientData += [ ICZorai ]

ICTryker = { }
ICTryker["Name"] = "tryker"
ICTryker["UnpackTo"] = None
ICTryker["IsOptional"] = 1
ICTryker["IsIncremental"] = 1
ICTryker["Packages"] = [ ]
ICTryker["Packages"] += [ [ "tryker_zones", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_shapes", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_pacs", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_maps", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_lightmaps", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_ig", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_newbie_zones", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_newbie_shapes", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_newbie_pacs", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_newbie_ig", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_island_zones", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_island_shapes", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_island_pacs", [ ] ] ]
ICTryker["Packages"] += [ [ "tryker_island_ig", [ ] ] ]
InstallClientData += [ ICTryker ]

ICSources = { }
ICSources["Name"] = "sources"
ICSources["UnpackTo"] = None
ICSources["IsOptional"] = 1
ICSources["IsIncremental"] = 1
ICSources["Packages"] = [ ]
ICSources["Packages"] += [ [ "sources_zones", [ ] ] ]
ICSources["Packages"] += [ [ "sources_shapes", [ ] ] ]
ICSources["Packages"] += [ [ "sources_pacs", [ ] ] ]
ICSources["Packages"] += [ [ "sources_maps", [ ] ] ]
ICSources["Packages"] += [ [ "sources_lightmaps", [ ] ] ]
ICSources["Packages"] += [ [ "sources_ig", [ ] ] ]
InstallClientData += [ ICSources ]

ICRouteGouffre = { }
ICRouteGouffre["Name"] = "route_gouffre"
ICRouteGouffre["UnpackTo"] = None
ICRouteGouffre["IsOptional"] = 1
ICRouteGouffre["IsIncremental"] = 1
ICRouteGouffre["Packages"] = [ ]
ICRouteGouffre["Packages"] += [ [ "route_gouffre_zones", [ ] ] ]
ICRouteGouffre["Packages"] += [ [ "route_gouffre_shapes", [ ] ] ]
ICRouteGouffre["Packages"] += [ [ "route_gouffre_pacs", [ ] ] ]
ICRouteGouffre["Packages"] += [ [ "route_gouffre_maps", [ ] ] ]
ICRouteGouffre["Packages"] += [ [ "route_gouffre_lightmaps", [ ] ] ]
ICRouteGouffre["Packages"] += [ [ "route_gouffre_ig", [ ] ] ]
InstallClientData += [ ICRouteGouffre ]

ICBagne = { }
ICBagne["Name"] = "bagne"
ICBagne["UnpackTo"] = None
ICBagne["IsOptional"] = 1
ICBagne["IsIncremental"] = 1
ICBagne["Packages"] = [ ]
ICBagne["Packages"] += [ [ "bagne_zones", [ ] ] ]
ICBagne["Packages"] += [ [ "bagne_shapes", [ ] ] ]
ICBagne["Packages"] += [ [ "bagne_pacs", [ ] ] ]
ICBagne["Packages"] += [ [ "bagne_maps", [ ] ] ]
ICBagne["Packages"] += [ [ "bagne_lightmaps", [ ] ] ]
ICBagne["Packages"] += [ [ "bagne_ig", [ ] ] ]
InstallClientData += [ ICBagne ]

ICTerre = { }
ICTerre["Name"] = "terre"
ICTerre["UnpackTo"] = None
ICTerre["IsOptional"] = 1
ICTerre["IsIncremental"] = 1
ICTerre["Packages"] = [ ]
ICTerre["Packages"] += [ [ "terre_zones", [ ] ] ]
ICTerre["Packages"] += [ [ "terre_shapes", [ ] ] ]
ICTerre["Packages"] += [ [ "terre_pacs", [ ] ] ]
ICTerre["Packages"] += [ [ "terre_maps", [ ] ] ]
ICTerre["Packages"] += [ [ "terre_lightmaps", [ ] ] ]
ICTerre["Packages"] += [ [ "terre_ig", [ ] ] ]
InstallClientData += [ ICTerre ]

ICNexus = { }
ICNexus["Name"] = "nexus"
ICNexus["UnpackTo"] = None
ICNexus["IsOptional"] = 1
ICNexus["IsIncremental"] = 1
ICNexus["Packages"] = [ ]
ICNexus["Packages"] += [ [ "nexus_zones", [ ] ] ]
ICNexus["Packages"] += [ [ "nexus_shapes", [ ] ] ]
ICNexus["Packages"] += [ [ "nexus_pacs", [ ] ] ]
ICNexus["Packages"] += [ [ "nexus_maps", [ ] ] ]
ICNexus["Packages"] += [ [ "nexus_lightmaps", [ ] ] ]
ICNexus["Packages"] += [ [ "nexus_ig", [ ] ] ]
InstallClientData += [ ICNexus ]

ICNewbieland = { }
ICNewbieland["Name"] = "newbieland"
ICNewbieland["UnpackTo"] = None
ICNewbieland["IsOptional"] = 1
ICNewbieland["IsIncremental"] = 1
ICNewbieland["Packages"] = [ ]
ICNewbieland["Packages"] += [ [ "newbieland_zones", [ ] ] ]
ICNewbieland["Packages"] += [ [ "newbieland_shapes", [ ] ] ]
ICNewbieland["Packages"] += [ [ "newbieland_pacs", [ ] ] ]
ICNewbieland["Packages"] += [ [ "newbieland_maps", [ ] ] ]
ICNewbieland["Packages"] += [ [ "newbieland_ig", [ ] ] ]
InstallClientData += [ ICNewbieland ]

ICIndoors = { }
ICIndoors["Name"] = "indoors"
ICIndoors["UnpackTo"] = None
ICIndoors["IsOptional"] = 1
ICIndoors["IsIncremental"] = 1
ICIndoors["Packages"] = [ ]
ICIndoors["Packages"] += [ [ "indoors_shapes", [ ] ] ]
ICIndoors["Packages"] += [ [ "indoors_pacs", [ ] ] ]
ICIndoors["Packages"] += [ [ "indoors_lightmaps", [ ] ] ]
ICIndoors["Packages"] += [ [ "indoors_ig", [ ] ] ]
InstallClientData += [ ICIndoors ]

ICR2 = { }
ICR2["Name"] = "r2"
ICR2["UnpackTo"] = None
ICR2["IsOptional"] = 1
ICR2["IsIncremental"] = 1
ICR2["Packages"] = [ ]
ICR2["Packages"] += [ [ "r2_misc", [ ] ] ]
ICR2["Packages"] += [ [ "r2_jungle", [ ] ] ]
ICR2["Packages"] += [ [ "r2_lakes", [ ] ] ]
ICR2["Packages"] += [ [ "r2_desert", [ ] ] ]
ICR2["Packages"] += [ [ "r2_roots", [ ] ] ]
ICR2["Packages"] += [ [ "r2_forest", [ ] ] ]
ICR2["Packages"] += [ [ "r2_lakes2", [ ] ] ]
ICR2["Packages"] += [ [ "r2_jungle2", [ ] ] ]
ICR2["Packages"] += [ [ "r2_forest2", [ ] ] ]
ICR2["Packages"] += [ [ "r2_desert2", [ ] ] ]
ICR2["Packages"] += [ [ "r2_roots2", [ ] ] ]
ICR2["Packages"] += [ [ "r2_forest_maps", [ ] ] ]
ICR2["Packages"] += [ [ "r2_desert_maps", [ ] ] ]
ICR2["Packages"] += [ [ "r2_roots_maps", [ ] ] ]
ICR2["Packages"] += [ [ "r2_lakes_maps", [ ] ] ]
ICR2["Packages"] += [ [ "r2_jungle_maps", [ ] ] ]
ICR2["Packages"] += [ [ "r2_roots_pz", [ ] ] ]
ICR2["Packages"] += [ [ "r2_lakes_pz", [ ] ] ]
ICR2["Packages"] += [ [ "r2_jungle_pz", [ ] ] ]
ICR2["Packages"] += [ [ "r2_forest_pz", [ ] ] ]
ICR2["Packages"] += [ [ "r2_desert_pz", [ ] ] ]
InstallClientData += [ ICR2 ]


# end of file
