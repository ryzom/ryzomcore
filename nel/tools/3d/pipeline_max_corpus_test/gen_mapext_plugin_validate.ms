-- ============================================================================
-- gen_mapext_plugin_validate.ms -- validate the Map Extender drop-in plugin
-- (mapext198m3.dlm rebuilt from nel/tools/3d/plugin_max/map_extender) against
-- the corpus (pipeline_max_design.md S10z-seize; max_geometry_formats Part P).
--
-- RUN IN 3ds Max 2010 WITH THE DROP-IN INSTALLED (copy
-- build_vs2008_wine/bin/mapext198m3.dlm into the Max plugins directory).
-- READ-ONLY: loads every Map Extender corpus file, never saves.
--
-- For every exporting node the shape corpus tagged MAPEXT, this dumps:
--   (a) ATPOS  -- the map channel evaluated at the Map Extender's own stack
--       position (every modifier ABOVE it temporarily disabled, then restored)
--       = the plugin's own output; the offline checker compares it against the
--       stored LocalModData cache (they must be IDENTICAL: the plugin replays
--       the cache verbatim).
--   (b) FULL   -- the full-stack evaluation (what collapse/export sees);
--       informational where Edit Mesh instances above rewrite topology (the
--       primes_racines skies).
-- Offline checker: mapext_plugin_check.py <outdir>/manifest.txt
-- ============================================================================

global GFX_ROOT = "R:\\graphics\\"             -- EDIT ME: asset root
global OUT_DIR = "C:\\mapext_probe\\"          -- EDIT ME: output dir
makeDir OUT_DIR all:true
global gMan = createFile (OUT_DIR + "manifest.txt")

-- The drop-in plugin's ClassDesc names the class "Map Extender" -> MAXScript class value
-- Map_Extender. classof-string compare per the repo maxscript convention (there is NO
-- getclassid built-in in MAXScript -- the first revision of this script called one and every
-- node failed with "Call needs function or class, got: undefined"). A missing-plugin stand-in
-- reports Missing_OSM here, so this also cleanly distinguishes PLUGINSTATE.
fn isMapExt m = (
    (((classof m) as string) == "Map_Extender")
)

fn dumpChannel tm tag = (
    for ch = 1 to 2 do (
        if (meshop.getMapSupport tm ch) then (
            local nv = meshop.getNumMapVerts tm ch
            format "% CHAN % VERTS % FACES %\n" tag ch nv tm.numfaces to:gMan
            for i = 1 to nv do (
                local v = meshop.getMapVert tm ch i
                format "% MV % % % %\n" tag i (formattedPrint v.x format:".9g") (formattedPrint v.y format:".9g") (formattedPrint v.z format:".9g") to:gMan
            )
            for i = 1 to tm.numfaces do (
                local f = meshop.getMapFace tm ch i
                format "% MF % % % %\n" tag i (f.x as integer) (f.y as integer) (f.z as integer) to:gMan
            )
        )
    )
)

fn probeNode nodeName = (
    local n = getNodeByName nodeName
    if n == undefined then (
        format "NODE % MISSING\n" nodeName to:gMan
        return false
    )
    format "NODE %\n" nodeName to:gMan
    -- locate the Map Extender on the stack; report whether it resolved to a real class
    local mi = 0
    for i = 1 to n.modifiers.count do (
        if (isMapExt n.modifiers[i]) then mi = i
    )
    if mi == 0 then (
        local cls = ""
        for i = 1 to n.modifiers.count do cls += (((classof n.modifiers[i]) as string) + " ")
        format "PLUGINSTATE missing-or-standin stack: %\n" cls to:gMan
        return false
    )
    format "PLUGINSTATE ok index % of % class %\n" mi n.modifiers.count ((classof n.modifiers[mi]) as string) to:gMan
    -- (a) at-position: disable everything ABOVE the Map Extender (indices < mi are above)
    local saved = #()
    for i = 1 to (mi - 1) do (
        append saved n.modifiers[i].enabled
        n.modifiers[i].enabled = false
    )
    local tm = snapshotAsMesh n
    dumpChannel tm "ATPOS"
    free tm
    for i = 1 to (mi - 1) do n.modifiers[i].enabled = saved[i]
    -- (b) full stack
    tm = snapshotAsMesh n
    dumpChannel tm "FULL"
    free tm
    true
)

fn probeFile relPath nodeNames = (
    format "FILE %\n" relPath to:gMan
    local ok = false
    try (
        loadMaxFile (GFX_ROOT + relPath) quiet:true
        ok = true
    ) catch (
        format "LOADFAIL %\n" relPath to:gMan
    )
    if ok then (
        for nn in nodeNames do (
            try ( probeNode nn ) catch ( format "NODEFAIL % %\n" nn (getCurrentException()) to:gMan )
        )
    )
    flush gMan
)

probeFile "stuff\\fyros\\agents\\actors\\female\\fy_hof_armor00.max" #("fy_hof_armor00_armpad", "fy_hof_armor00_bottes", "fy_hof_armor00_gilet", "fy_hof_armor00_hand", "fy_hof_armor00_pantabottes")
probeFile "stuff\\fyros\\agents\\actors\\female\\fy_hof_caster01.max" #("fy_hof_caster01_pantabottes")
probeFile "stuff\\fyros\\agents\\actors\\male\\fy_hom_armor00.max" #("fy_hom_armor00_bottes", "fy_hom_armor00_gilet", "fy_hom_armor00_pantabottes")
probeFile "stuff\\fyros\\agents\\actors\\male\\fy_hom_caster01.max" #("fy_hom_caster01_pantabottes")
probeFile "stuff\\generique\\agents\\actors\\female\\ge_hof_armor02.max" #("ge_hof_armor02_gilet", "ge_hof_armor02_pantabottes")
probeFile "stuff\\generique\\agents\\actors\\male\\ge_hom_caster00.max" #("ge_hom_caster00_pantabottes")
probeFile "stuff\\matis\\agents\\actors\\female\\ma_hof_armor01.max" #("ma_hof_armor01_armpad", "ma_hof_armor01_pantabottes")
probeFile "stuff\\matis\\agents\\actors\\male\\ma_hom_caster01.max" #("ma_hom_caster01_pantabottes")
probeFile "stuff\\matis\\agents\\actors\\visages\\ma_hof_visage.max" #("ma_hof_visage")
probeFile "stuff\\matis\\agents\\actors\\visages\\ma_hom_visage.max" #("ma_hom_visage")
probeFile "stuff\\tryker\\agents\\actors\\female\\tr_hof_armor00.max" #("tr_hof_armor00_bottes", "tr_hof_armor00_gilet")
probeFile "stuff\\tryker\\agents\\actors\\female\\tr_hof_armor01.max" #("tr_hof_armor01_armpad", "tr_hof_armor01_bottes", "tr_hof_armor01_gilet", "tr_hof_armor01_pantabottes")
probeFile "stuff\\tryker\\agents\\actors\\male\\tr_hom_caster01.max" #("tr_hom_caster01_pantabottes")
probeFile "stuff\\tryker\\agents\\actors\\male\\tr_hom_underwear.max" #("tr_hom_underwear_hand")
probeFile "stuff\\tryker\\agents\\actors\\visages\\tr_hof_visage.max" #("tr_hof_visage")
probeFile "stuff\\tryker\\agents\\actors\\visages\\tr_hom_cheveux_artistic02.max" #("tr_hom_cheveux_artistic02")
probeFile "stuff\\tryker\\agents\\actors\\visages\\tr_hom_visage.max" #("tr_hom_visage")
probeFile "stuff\\zorai\\agents\\actors\\female\\zo_hof_armor01.max" #("zo_hof_armor01_armpad")
probeFile "stuff\\zorai\\agents\\actors\\male\\zo_hom_armor00.max" #("zo_hom_armor00_armpad", "zo_hom_armor00_bottes", "zo_hom_armor00_gilet", "zo_hom_armor00_hand")
probeFile "stuff\\zorai\\agents\\actors\\male\\zo_hom_caster01_bottes_armor01.max" #("zo_hom_caster01_bottes_armor01")
probeFile "stuff\\generique\\decors\\constructions\\gen_bt_atelier.max" #("gen_bt_atelier_caisse")
probeFile "stuff\\generique\\decors\\constructions\\gen_bt_auberge.max" #("gen_bt_aub_ext")
probeFile "stuff\\generique\\decors\\constructions\\gen_bt_comptoir.max" #("gen_bt_comptoir_caisse")
probeFile "stuff\\generique\\decors\\constructions\\gen_bt_drapeau_revendication.max" #("gen_bt_drapeau_revendication")
probeFile "stuff\\generique\\decors\\constructions\\gen_bt_tourgarde.max" #("gen_bt_tour_portebas", "gen_bt_tour_porteleft", "gen_bt_tour_porteright")
probeFile "stuff\\goo\\agents\\monsters\\coureur\\go_mo_c03.max" #("go_mo_c03")
probeFile "stuff\\goo\\agents\\monsters\\crustaces\\go_mo_estrasson.max" #("go_mo_estrasson")
probeFile "stuff\\tryker\\agents\\monsters\\cheval\\c03\\tr_mo_c03_boss.max" #("tr_mo_c03_boss")
probeFile "stuff\\tryker\\agents\\monsters\\chien\\varinx\\tr_mo_varinx_boss.max" #("tr_mo_varinx_boss")
probeFile "stuff\\tryker\\agents\\monsters\\coureur\\filin\\tr_mo_filin_boss.max" #("tr_mo_filin_boss")
probeFile "stuff\\tryker\\agents\\monsters\\crustace\\estrasson\\tr_mo_estrasson.max" #("tr_mo_estrasson")
probeFile "stuff\\tryker\\agents\\monsters\\kitin\\kitimandib\\tr_mo_kitimandib_boss.max" #("tr_mo_kitimandib_boss")
probeFile "stuff\\tryker\\agents\\monsters\\ryzomien\\kakty\\tr_mo_kakty_boss.max" #("tr_mo_kakty_boss")
probeFile "stuff\\caravan\\agents\\accessories\\ca_wea_trooper_gun.max" #("ca_wea_trooper_gun")
probeFile "stuff\\generique\\agents\\accessories\\ge_acc_pick_o.max" #("ge_acc_pick_o")
probeFile "stuff\\generique\\agents\\accessories\\ge_hom_mission_reward_outpost_gauntlet.max" #("ge_hom_mission_reward_outpost_gauntlet")
probeFile "stuff\\generique\\agents\\accessories\\ge_kapo_pilon_o.max" #("ge_kapo_pilon_o")
probeFile "stuff\\generique\\agents\\accessories\\ge_kbij_marteau_o.max" #("ge_kbij_marteau_o")
probeFile "stuff\\generique\\agents\\accessories\\ge_kwj_ciseau_o.max" #("ge_kwj_ciseau_o")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_carapace_b.max" #("ge_mission_carapace_b")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_carapace_bull.max" #("ge_mission_carapace_bull")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_entrepot.max" #("object04")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_feu_off.max" #("ge_mission_feu_off")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_fortuna_wheel.max" #("ge_mission_fortuna_wheel_ring")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_jarre.max" #("ge_mission_jarre")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_karavan_acc_pick_a.max" #("ge_mission_karavan_acc_pick_a")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_maison_ruine.max" #("ge_mission_maison_ruine")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_mass_diam.max" #("ge_mission_mass_diam")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_ossements_b.max" #("ge_mission_ossements_b")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_outpost_infostele.max" #("ge_mission_outpost_infostele")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_panneau.max" #("ge_mission_panneau")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_pique_diam.max" #("ge_mission_pique_diam")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_reward_karavan_bigshield.max" #("ge_mission_reward_karavan_bigshield")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_ringate_teleport.max" #("ge_mission_ringate_teleport")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_statue_jena_karavan.max" #("ge_mission_statue_jena_karavan")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_stele.max" #("ge_mission_stele")
probeFile "stuff\\generique\\agents\\accessories\\ge_mission_stele_karavan.max" #("ge_mission_stele_karavan")
probeFile "stuff\\generique\\agents\\accessories\\ge_wea_fusarc.max" #("ge_wea_fusarc")
probeFile "stuff\\matis\\agents\\accessories\\ma_hof_acc_gauntlet.max" #("ma_hof_acc_gauntlet")
probeFile "stuff\\matis\\agents\\accessories\\ma_wea_batonspellcaster.max" #("baton magie01")
probeFile "stuff\\zorai\\agents\\accessories\\zo_hom_acc_gauntlet.max" #("zo_hom_acc_gauntlet")
probeFile "stuff\\outgame\\appart_crea\\ma_appart_outgame.max" #("ma_appart_outgame")
probeFile "sfx\\meshtoparticle\\ma_fx_birds.max" #("ma_fx_bird01")
probeFile "sfx\\meshtoparticle\\tr_fx_birds.max" #("tr_fx_bird01")
probeFile "sky_v2\\max\\de_sky_dome.max" #("de_clouds_dome_altitude", "de_clouds_dome_light")
probeFile "sky_v2\\max\\fo_sky_dome.max" #("fo_clouds_dome_altitude", "fo_clouds_dome_light", "fo_horizon_dome")
probeFile "sky_v2\\max\\ju_sky_dome.max" #("ju_clouds_dome_altitude", "ju_clouds_dome_light")
probeFile "sky_v2\\max\\la_sky_dome.max" #("la_clouds_dome_altitude", "la_clouds_dome_light")
probeFile "stuff\\primes_racines\\sky_bagne\\sky_primes_racines_bagne.max" #("sky_primes_racines_bagne")
probeFile "stuff\\fyros\\decors\\constructions\\fy_cn_mairie\\fy_cn_mairie01.max" #("fy_cn_mairie_entree-a", "fy_cn_mairie_entree-b", "fy_cn_mairie_int")
probeFile "stuff\\fyros\\decors\\constructions\\fy_cn_sheriff.max" #("fy_bt_sheriff_modif_ok_inter_assistant", "fy_bt_sheriff_modif_ok_inter_sherif")
probeFile "stuff\\fyros\\decors\\constructions\\fy_cn_smokehouse.max" #("fy_smoke_cage_elevateur")
probeFile "stuff\\fyros\\decors\\constructions\\fy_panneau.max" #("box01")
probeFile "stuff\\fyros\\decors\\constructions\\fy_cn_hall_reunion_vitrine\\fy_hall_reunion.max" #("halo_appart")
probeFile "stuff\\matis\\decors\\constructions\\appart_joueur\\ma_appart_joueur.max" #("ma_appart_armoire", "ma_appart_lit")
probeFile "stuff\\matis\\decors\\constructions\\hall_vitrine_hall_reunion\\ma_hall_vitrine_hall_reunion.max" #("ma_int_hall_reunion", "ma_plafonier_hall_vitrine", "ma_stairs_hall_reunion")
probeFile "stuff\\matis\\decors\\constructions\\ma_mairie.max" #("ma_mairie_couloir")
probeFile "stuff\\matis\\decors\\constructions\\ma_serre.max" #("ma_serre_exterieur_couloir")
probeFile "stuff\\primes_racines\\sky_route_gouffre\\sky_primes_racines_route_gouffre.max" #("sky_primes_racines_route_gouffre")
probeFile "stuff\\primes_racines\\sky_sources\\sky_primes_racines_sources.max" #("sky_primes_racines_sources")
probeFile "stuff\\primes_racines\\sky_terre\\sky_primes_racines_terre.max" #("sky_primes_racines_terre")
probeFile "stuff\\tryker\\decors\\constructions\\tr_barge.max" #("tr_barge_porte_int_left")
probeFile "stuff\\tryker\\decors\\constructions\\tr_coffre.max" #("coffre")
probeFile "landscape\\ligo\\desert\\max\\zonematerial-converted-1_mz_city.max" #("city_part08", "city_part50", "city_part63", "fy_bt_sheriff_plato_ext_fy_sheriff")
probeFile "stuff\\fyros\\decors\\vegetations\\fy_s1_baobab_c.max" #("fy_s1_baobab_c")
probeFile "landscape\\ligo\\lacustre\\max\\zonematerial-converted-203_dx.max" #("tr_waterfall_03_a")
probeFile "stuff\\tryker\\decors\\vegetations\\tr_s3_aquabole_a.max" #("tr_s3_aquabole_a")

close gMan
format "gen_mapext_plugin_validate: DONE\n"
