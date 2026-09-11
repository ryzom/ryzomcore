# landscape_editor (Atyscape)

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/landscape_editor.py`

Nommé "Atyscape" par Nuno (2026-09-07) -- même convention que `object_editor.py`/"Patina".

Status : scaffolding en cours, voir `project-todos/forgery/landscape_editor.md`
pour l'état d'avancement complet (rendu terrain progressif, édition `.ig`,
composition `.land`, overlay PACS).

## Rôle

Nouvel outil Forgery de type "Landscape Editor" : remplace/complète l'outil
de leveldesign actuel (2D, tuiles en bitmaps, éditeur Ligo) par un vrai
rendu 3D du terrain -- chargement/rendu des zones `.zonel`, chargement/
édition des `.ig`, et à terme un overlay des collisions PACS.

## Étape 1 -- scaffolding

App `ForgeryApp` minimale (même base que `object_editor.py`/Patina) : fenêtre,
explorateur filtré sur `*.land`, `OrbitCamera` réutilisée telle quelle
(nécessite que l'app définisse `self.forced_drag_mode`/`self.target_mode`,
tous deux `None` ici -- pas d'`ObjectManipulator`/objet ciblé dans cet
éditeur, contrairement à Patina). Découverte automatique par ryztart via
`APP_INFO` (pas de fichier "menu" séparé à éditer).

## Sélecteur de continent + bornes de région

Voir `project-todos/forgery/landscape_editor__continent-selector.md`. Logique
dans `ryzom_forgery/continent_selector.py` :

- `load_continent_locations(live_data_path)` : lit `world.packed_sheets`
  (son unique entrée réelle, `ryzom.world`), retourne les `ContLoc` triés par
  `selection_name` -- alimente le dropdown du panel (libellé =
  `selection_name`, valeur = `continent_name`, ex. `"bagne"` -- **sans**
  extension, confirmé sur les vraies données ; `.continent` doit être ajouté
  soi-même avant de chercher dans `sheet_id.bin`).
- `resolve_continent_bounds(live_data_path, continent_name)` : résout
  l'entrée `continent.packed_sheets` correspondante (clé `f"{continent_name}.continent"`
  retrouvée via `sheet_id.bin` -- **pas un fichier plat** dans une vraie
  installation, uniquement empaqueté dans `leveldesign.bnp`, `BnpReader` en
  repli si le fichier plat n'existe pas), puis reproduit
  `CContinent::getCorners()` : `ContinentParameters.zone_min`/`zone_max` sont
  des **noms de zone**, pas des coordonnées --
  `pynel.ryzom_packed_sheets.zone_name_to_world_pos()` les décode en position
  de coin origine, chaque axe est remis dans l'ordre min/max (l'ordre
  lettre/chiffre du nom de zone n'est pas garanti croissant), et `+160` est
  appliqué au coin max (un nom de zone désigne son origine, pas son
  étendue).

Le résultat est exposé comme `LandscapeEditorApp.continent_bounds`
(`(min_x, min_y, max_x, max_y)`), recalculé uniquement quand la sélection
change (pas à chaque frame) -- à consommer par le "chargement par région"
(`project-todos/forgery/landscape_editor.md` étape 6, pas encore
implémenté). Affiché dans le panel pour validation visuelle immédiate en
attendant.

**Chemin des données** : `world.packed_sheets`/`continent.packed_sheets`/
`sheet_id.bin` sont lus depuis `settings.live_data_path`, un réglage global
partagé avec Patina (`object_editor.py`) -- configurable dans l'onglet
Settings d'Atyscape (voir "Onglets" ci-dessous) comme dans celui de Patina.
Message d'erreur explicite dans le panel si ce chemin n'est pas configuré ou
invalide. L'Explorer démarre directement dans ce dossier si configuré
(sinon un `search_path` classique, sinon le home).

## Onglets (Landscape / Settings)

`draw_panel()` (project-todos/forgery/
landscape_editor__zone_render_modes__ryzom_paths_ui.md) utilise une barre
d'onglets (`imgui.begin_tab_bar`, mêmes helpers `_begin_tab_item_with_icon`/
`_push_tab_color`/`_pop_tab_color` que `object_editor.py`, déplacés dans
`object_editor_mixins/ui_helpers.py` pour être réutilisables par toute app
Forgery) :

- **Landscape** (`_draw_landscape_tab()`) : le contenu historique du panel --
  sélecteur de continent, chargement/statut de zone, bouton "Top view (2D)".
- **Settings** : `self.ryzom_paths_section.draw(self)` -- voir
  `ryzom_forgery/ryzom_paths_section.py`. Regroupe `live_data_path`, les
  chemins de dépôts (`pynel.repository_paths`) et `settings.ryzom_tools_path`
  sous un header "Ryzom Paths" unique, réutilisé tel quel par l'onglet
  Settings de Patina : ce sont des réglages génériques de toute la suite
  Forgery, pas propres à une seule app, donc éditables depuis n'importe
  laquelle -- Patina (graphistes) et Atyscape (level designers) ont des
  publics disjoints, un utilisateur d'Atyscape ne doit jamais avoir à ouvrir
  Patina pour configurer un réglage dont lui seul a besoin (décision Nuno
  2026-09-08).

`_draw_viewport_toggles()` (quadrillage de zone) reste appelé hors de la
barre d'onglets -- fenêtre flottante indépendante, jamais masquée par un
changement d'onglet.

## Étapes 3-4 -- rendu terrain (low-poly puis tessellation Bézier)

Sélectionner un `.zone`/`.zonew`/`.zonel` dans l'Explorer (y compris à
l'intérieur d'un `.bnp`, ex. `nexus_zones.bnp`) parse la zone via
`pynel.ryzom_zone.parse_zone()` et construit un maillage via
`ryzom_forgery/zone_geometry.py`.

**Étape 4 (actuelle)** : `build_zone_tessellated_geom()` -- chaque patch est
tessellé sur une grille `(order_s+1)×(order_t+1)` (repli 1×1 si
`order_s`/`order_t` == 0, patch version < 2), évaluée via la vraie surface
Bézier (`_eval_bezier_patch()`). La grille de contrôle 4×4 canonique
(`_bezier_control_grid()`) est reconstruite depuis l'indexation propre à NeL
(`Vertices`/`Tangents`/`Interiors`) en inspectant quel terme de
`CBezierPatch::eval()` (`bezier_patch.cpp:98-133`) contribue à quel coin
`(s,t)` -- une fois arrangée en grille standard, une évaluation
tensor-product classique (base de Bernstein cubique) reproduit `eval()`
exactement, sans lister les 16 termes à plat. Vérifié : les 4 coins évalués
correspondent exactement aux sommets bruts (`patch.vertices`).

**Étape 3 (historique, remplacée)** : `build_zone_low_poly_geom()` (retirée
du module) ne traçait qu'un quad par patch avec ses 4 coins bruts, sans
Bézier -- sur un relief très courbé (dôme, colline), le quad plat coupait à
travers la vraie surface bombée, visible comme des creux/"trous" en diamant
à l'écran. Corrigé par la tessellation de l'étape 4.

Coloration par élévation (dégradé vert foncé -> jaune-vert clair, interpolé
par sommet, `_elevation_color()`) plutôt que par `CTileColor` -- anticipe
l'étape 5 ("Rendu par élévation") pour un rendu plus lisible, décision Nuno
2026-09-07 ; tourne maintenant sur la vraie surface évaluée, pas seulement
les 4 coins.

**Explorer** : filtre par défaut `"*"` (pas `"*.land"`) -- un `.bnp` qui ne
contient aucun fichier correspondant au filtre actif est entièrement caché
par `explorer.py` (`_draw_dir_contents()`), et les vrais `*_zones.bnp` d'une
installation Ryzom Live ne contiennent jamais de `.land` (trouvé le
2026-09-07 : plus aucun `.bnp` n'apparaissait avec `"*.land"` par défaut).

Bouton "Top view (2D)" dans le panel : sert de "vue 2D projetée" en
réutilisant `OrbitCamera.snap_to_axis("+z")` sur le même maillage, pas de
rendu 2D séparé.

## Étape 6 -- chargement d'un continent entier via cache disque

Voir `project-todos/forgery/landscape_editor__zone_disk_cache.md`. Décision
révisée le 2026-09-08 : le chargement par région autour de la caméra (étape 6
originale) est abandonné -- des mesures (194 zones, rayon 400) ont montré que
le listing (0.656 s, une seule fois) et la lecture des `.bnp` (0.276 s) sont
négligeables, mais que le parsing (`parse_zone`, 11.221 s) et la tessellation
(`build_zone_tessellated_geom`, 14.506 s) dominent totalement (~132 ms/zone
au total, boucle Python pure dans `_eval_bezier_patch()`) et sont linéaires
au nombre de zones, quelle que soit la stratégie de streaming. Autant payer
ce coût une seule fois par zone et le mettre en cache disque plutôt que de le
répéter à chaque déplacement de caméra -- d'où le chargement d'un continent
entier dès sa sélection dans le dropdown.

**Ce qui est mis en cache (`ryzom_forgery/zone_cache.py`)** : uniquement les
positions tessellées par patch (`n_s`/`n_t` + un tuple plat de floats
x,y,z,...) et la bounding box de la zone (`bb_center`/`bb_half_size`) --
jamais la couleur, les indices de triangle, ni de futures normales/UV. Ces
derniers sont tous dérivables à coût quasi nul depuis les seules positions
(couleur : juste le Z du sommet + min/max Z de la zone ; indices : pure
arithmétique sur `n_s`/`n_t` ; une future normale : différence de positions
voisines ; un futur UV de texturage : `i/n_s`, `j/n_t`). En ne bakant que la
seule partie réellement coûteuse à produire, aucune évolution du rendu
(lighting étape 10, texturage étape 12) n'invalidera jamais ce cache.

- `zone_cache.write_zone_cache(zone_name, source_path, data)` /
  `read_zone_cache(zone_name, source_path)` : un fichier pickle par zone sous
  `config_dir() / "zone_cache"`, écriture atomique (fichier temporaire +
  rename). L'enveloppe pickle stocke une version de format + le mtime/taille
  du fichier source (`.zone*` libre, ou le `.bnp`/`.bnpe` entier si la zone y
  est empaquetée -- impossible de dater une entrée interne à une archive) ;
  toute divergence (version ou source modifiée) invalide silencieusement le
  cache, à charge de l'appelant de le régénérer.
- `zone_geometry.compute_zone_patch_positions(zone)` : la partie coûteuse
  isolée (évaluation Bézier), partagée par `zone_to_cache_data(zone)` (utilisé
  par le chemin direct -- `build_zone_tessellated_geom()` en est un simple
  wrapper -- et par `on_selection_changed()`, sélection Explorer d'une zone
  unique) et par `region_loader.load_zone_cache_data(ref)` (chemin cache
  d'abord). `zone_geometry.build_zone_geom_from_cache(cache_data)` reconstruit
  le `GeomNode` (couleur + indices recalculés) depuis un `ZoneCacheData`,
  qu'il vienne du disque ou d'une zone fraîchement parsée -- une seule et
  même fonction de rendu dans tous les cas.
- `region_loader.load_zone_cache_data(ref)` : lit le cache disque si présent
  et frais ; sinon parse la zone réelle (`load_zone_ref()`), calcule son
  `ZoneCacheData` et l'écrit sur disque (best-effort -- une erreur d'écriture,
  disque plein/permissions, n'empêche jamais d'afficher la zone qui vient
  d'être calculée). Pure donnée, aucun appel Panda3D -- sûr à appeler depuis
  le thread d'arrière-plan de `_run_load_continent()`, la construction du
  `GeomNode` reste réservée au thread principal (`_set_loaded_zones()`).

**Déclenchement** : `_select_continent()` lance automatiquement
`_load_continent()` dès qu'un continent est choisi dans le dropdown ; le
bouton "Build cache for this continent" du panel déclenche exactement la
même chose manuellement (les deux existent, décision Nuno 2026-09-08). Une
barre de progression (`imgui.progress_bar`, "X/Y zones") s'affiche pendant le
chargement (thread d'arrière-plan, `_run_load_continent()`), qui traite
potentiellement plusieurs centaines de zones pour un continent entier.

`self.zones` (dans `landscape_editor.py`) contient toujours des
`ZoneCacheData` (jamais un `Zone` pynel brut), converties via
`zone_to_cache_data()` dès qu'une zone est parsée -- `_set_loaded_zones()` et
`_frame_on_loaded_zones()` n'ont donc qu'une seule forme de donnée à gérer,
que le chargement vienne de l'Explorer (une zone) ou d'un continent entier
(potentiellement des centaines).

## Écriture en bloc du `GeomNode` (`build_zone_geom_from_cache`)

Mesuré le 2026-09-08 (53 zones, 100% cache hit) : le cache disque a bien
rendu le listing+chargement quasi gratuit (0.195 s total), mais la
construction du `GeomNode` (couleur + indices + attach) a pris 3.365 s
(~63.5 ms/zone) -- à peine mieux que les 74.8 ms/zone d'origine (bnp+parse+
Bézier+construction confondus). Le vrai goulot n'était donc pas le calcul
(couleur/indices sont de l'arithmétique triviale) mais le nombre d'appels
Python un par un à `GeomVertexWriter.add_data3()`/`add_data4()` et
`GeomTriangles.add_vertices()` -- chaque appel traverse le binding Python/C++
de Panda3D, des milliers de fois par zone.

`build_zone_geom_from_cache()` construit désormais un buffer `numpy`
structuré (`[('vertex','<f4',3),('color','u1',4)]`, 16 octets/sommet -- le
layout réel de `GeomVertexFormat.get_v3c4()`, vérifié par `assert` au
runtime plutôt que supposé en dur) et l'écrit d'un seul coup via
`vertex_array.modify_handle().set_data(buf.tobytes())`. La couleur par
élévation (`_elevation_colors_uint8()`) est calculée vectorisée (numpy) sur
tous les sommets d'un patch à la fois, avec **troncature** (pas arrondi) du
`composante * 255.0` -- vérifié bit-à-bit identique à ce que produisait
`add_data4()` avant ce changement. Les indices de triangles sont construits
en un tableau `numpy` plat (`uint16`, ou `uint32` au-delà de 65535 sommets
dans une même zone) via `np.meshgrid` et écrits d'un coup via
`triangles.modify_vertices().modify_handle().set_data(idx.tobytes())`.
Gain mesuré sur données synthétiques (5555 sommets/zone, 53 zones) : 17.4 ->
3.2 ms/zone, ~5.4x.

`build_zone_tessellated_geom()` (chemin direct depuis une zone fraîchement
parsée) délègue toujours à `build_zone_geom_from_cache()`, donc bénéficie du
même gain sans code séparé à maintenir.

## Étape 7 -- caméra, dégradé d'élévation ancré à Z=0, quadrillage de zone

Trois ajustements demandés par Nuno en testant un continent entier réel :

**Zoom caméra** : `OrbitCamera.max_distance` (2000.0 par défaut, partagé
avec Patina) ne permettait pas de reculer assez pour voir un continent
entier -- multiplié par 3 puis par 6 (`self.orbit_camera.max_distance *=
6.0`, 12000.0, 3x ayant à son tour été jugé encore insuffisant le
2026-09-08) dans `LandscapeEditorApp.__init__`, spécifique à Atyscape (ne
touche pas `camera.py` ni Patina). Ça débloque aussi l'auto-cadrage après
chargement d'un continent (`frame()` clampait déjà à `max_distance`).

**Near/far du lens** : `ForgeryApp.__init__` (`app.py`) fixe un near/far de
`(0.02, 20000.0)`, un ratio 1 000 000:1 pensé pour l'inspection rapprochée
de détails de shape par Patina -- Atyscape ne s'approche jamais autant
(zones de 160 unités, orbite démarrant à 200). Ce ratio coûtait de la
précision de depth buffer : la géométrie plate à Z=0 (le quadrillage de
zone) subissait du z-fighting contre le terrain traversant Z=0, avec un
rendu différent selon la distance caméra (trouvé par Nuno 2026-09-08, un
palier de zoom suffisait à changer le résultat). Réglé en resserrant à
`self.camLens.set_near_far(1.0, 20000.0)` (ratio 20 000:1), confirmé par
Nuno.

**Dégradé d'élévation ancré à Z=0** (`zone_geometry._elevation_colors_uint8()`) :
la première version (un dégradé marron->vert normalisé sur le min/max Z de
l'ensemble chargé) était lissée/washed-out par quelques zones très
profondes (sous-marines/grottes) qui tiraient le minimum très bas -- le
terrain normal se retrouvait alors tout au même ton près du "haut" de la
plage. Remplacé par un dégradé en deux segments ancré sur Z=0 (le niveau de
l'eau, d'après Nuno) : rouge foncé au point le plus profond -> marron à
Z=0 -> vert au point le plus haut, chaque moitié interpolée indépendamment
(`_DEEP_COLOR`/`_SEA_LEVEL_COLOR`/`_PEAK_COLOR`). Insensible à la profondeur
du point le plus bas : le terrain proche de la surface reste toujours dans
la même plage de marron/vert clairs.

**Quadrillage de zone** (`zone_geometry.build_zone_grid_geom()`) : overlay
`LineSegs` des limites de cellules `ZONE_CELL_SIZE` (160 unités, déplacé de
`region_loader.py` vers `zone_geometry.py` pour éviter un import circulaire
-- `region_loader.py` l'importe maintenant depuis `zone_geometry`), bornes
calées au multiple de 160 le plus proche (floor/ceil) pour que chaque zone
affichée ait son contour complet. Rebâti dans `_set_loaded_zones()` à chaque
changement de zones chargées ; toggle icône dans la barre flottante
bas-gauche de la vue 3D (`_draw_viewport_toggles()`, visible par défaut --
migré depuis une checkbox du panel le 2026-09-08, même pattern que
`object_editor.py`/Patina : `_icon_button()` réutilisé tel quel depuis
`object_editor_mixins/ui_helpers.py`, et le même mécanisme
`_viewport_toggle_size` de mesure sur la frame précédente pour positionner
la barre exactement, `large_icon_font` inclus). Toujours affiché par-dessus
le terrain quel que soit son relief : `set_depth_test(False)`/
`set_depth_write(False)` + `set_bin("fixed", 100)`, plat à Z=0 -- une grille
de repère, pas un vrai maillage à suivre le terrain.

**Tentative abandonnée -- plan d'eau approximatif** : une surface bleue
semi-transparente à Z=0 (un quad par zone) a été essayée le 2026-09-08 comme
raccourci visuel en attendant la vraie détection d'eau via `.ig`/
`CWaterShape` (étape 9). Abandonnée : elle lisait comme "eau" tout relief
simplement sous le niveau de la mer, pas seulement les vrais lacs, couvrant
en bleu la majorité d'un continent réel. Retirée entièrement
(`build_water_plane_geom()`/`_WATER_PLANE_COLOR` dans `zone_geometry.py`,
état/toggle dans `landscape_editor.py`) ; seule la détection réelle par
`.ig` reste au programme.

## Modes de rendu [2D][POLY][WELD][LIGHT] (`landscape_editor__zone_render_modes.md`)

Barre de 4 boutons (`_draw_render_mode_bar()`) contrôlant `self.render_mode`
(`"2D"/"POLY"/"WELD"/"LIGHT"`, défaut `"POLY"`). `[2D]` ne fait que la bascule
caméra existante (`snap_to_axis("+z")`) ; `[POLY]`/`[WELD]`/`[LIGHT]`
re-résolvent, pour chaque zone du continent chargé (`self._loaded_refs`/
`self._loaded_extensions`, name -> ZoneRef / name -> {ext: ZoneRef}), quelle
extension réelle afficher via `_resolve_zone_for_mode()` :

- `[WELD]` accepte `.zonew` OU `.zonel` comme "réellement weldé" (un `.zonel`
  livré implique que le weld a eu lieu, même si le `.zonew` intermédiaire n'a
  pas été conservé -- un vrai `live_data` shippé ne garde souvent que l'étape
  finale). Repli sur `.zone` sinon.
- `[LIGHT]` n'accepte que `.zonel`. Repli sur `.zonew` puis `.zone`.
- Une zone affichée via repli (pas la bonne extension pour le mode actif) est
  rendue avec un dégradé violet -> rose (`_FALLBACK_LOW`/`_FALLBACK_HIGH`,
  `build_zone_geom_from_cache(..., fallback=True)`) plutôt qu'en dégradé
  d'élévation. Une première version en dégradé de gris (quasi-noir -> gris
  clair) s'est révélée indiscernable du fond gris du viewport -- une zone en
  repli à basse élévation ressemblait à un vrai trou dans le terrain plutôt
  qu'à une zone grisée (trouvé le 2026-09-09, Nuno). Violet/rose n'apparaît
  jamais dans le dégradé d'élévation réel (rouge/marron/vert), donc se
  distingue sans ambiguïté à la fois du terrain et du fond.

Le rendu Explorer (`on_selection_changed()`, sélection d'un fichier unique)
n'est **jamais** passé par cette logique de mode -- charge toujours
exactement le fichier cliqué. Ce chemin n'est de toute façon jamais utilisé
en pratique (Nuno 2026-09-09) : le workflow réel charge toujours un continent
entier via le combo.

### Source de zones par continent -- `live_data_path` vs export pipeline réel

`region_loader.find_zones_in_region()`/`get_zone_index()`/
`get_zone_extensions_index()` prennent un `continent_name` + `ryzom_data_path`
optionnels. Si `<ryzom_data_path>/pipeline/export/continents/<continent_name>/`
existe (un vrai export du pipeline `build_gamedata`, avec ses sous-dossiers
`zone`/`zone_weld`/`zone_lighted` parmi bien d'autres -- seuls ces trois sont
scannés), ces zones **remplacent entièrement** `live_data_path` pour ce
continent (jamais fusionné) -- `live_data_path` n'a alors même plus besoin
d'être configuré. Cette source n'est **jamais mise en cache** (contrairement
à `live_data_path`) : elle sert à tester le pipeline avec de vraies données à
plusieurs étapes (contrairement à `live_data_path` qui ne livre que l'étape
finale), y compris en supprimant manuellement un fichier pendant une session
pour vérifier un cas de repli -- un cache figé casserait ce test.

`ryzom_data_path` est résolu via `pynel.repository_paths.get("ryzom-data")`
(le sélecteur déjà existant dans Settings > Ryzom Paths, `RepositoryPathsDialog`)
-- **jamais** un réglage dédié en plus (une première version avait
introduit `Settings.ryzom_data_path`, doublon corrigé le 2026-09-09).

**Piège découvert le 2026-09-09** : le nom affiché dans le combo continent
(`ContLoc.selection_name`, ex. `"nexus"`) peut être **différent** du nom
interne (`ContLoc.continent_name`, ex. `"lecarrefour"`) utilisé pour résoudre
les bornes (`continent_selector.resolve_continent_bounds`). `continent_name`
ne nomme que les fichiers projet sous
`ryzom-data/leveldesign/world/<continent_name>/` (`.continent`, `.region`,
`continent.cfg`) -- un vrai dossier d'export pipeline sous
`ryzom-data/pipeline/export/continents/` est nommé d'après `selection_name`,
pas `continent_name`. `landscape_editor.py` garde donc les deux séparément :
`self.selected_continent_name` (résolution des bornes) et
`self._selected_continent_pipeline_name` (recherche de l'export pipeline,
issu de `cont_loc.selection_name`).

**Chemins `ryzom-data` -- lequel est le bon** (2026-09-09) : les vrais
`.land`/briques de leveldesign vivent sous `leveldesign/landscape/`
(actuel, maintenu -- ex. `leveldesign/landscape/desert/fyros.land`, daté
2025). `graphics/landscape/ligo/` contient de **vieilles données obsolètes**
(un même nom de fichier peut y être un format binaire legacy daté de 2020,
voire absent) -- confirmé par comparaison octet-à-octet sur les 27 `.land`
existants (19 identiques, 7 différents dont au moins un format binaire
périmé, 1 manquant). Ne jamais lire `graphics/landscape/ligo/` pour du
leveldesign actif.

## Mode global Visualisation / Édition (`landscape_editor__land_preview.md`)

`_detect_app_mode()` détermine le mode par défaut, uniquement la toute
première fois (`pynel.repository_paths.is_valid("ryzom-data")` -- `"edition"`
si `ryzom-data` est configuré et pointe vers un dossier existant,
`"visualisation"` sinon), indépendamment du contenu réel de ce `ryzom-data`
(un `.land` manquant pour tel ou tel continent est géré par le filtrage du
combo, pas par ce switch global).

**Bascule manuelle "Release"/"Dev"** (`landscape_editor__land_preview.md`
étape 4, 2026-09-10) : `_resolve_app_mode()` lit `Settings.
landscape_editor_mode` (persistant, `None` tant que l'utilisateur n'a jamais
touché au bouton) -- si `None`, `_detect_app_mode()` sert de valeur par
défaut ; si `"edition"` mais `ryzom-data` n'est plus valide, repli forcé sur
`"visualisation"` **sans modifier** le choix sauvegardé (reprend son effet
dès que `ryzom-data` est reconfiguré). Bouton "Switch to Release"/"Switch to
Dev" à côté du badge (`_set_app_mode()`, sauvegarde immédiate), désactivé
(grisé, tooltip) tant que `ryzom-data` n'est pas configuré/valide -- Dev
n'a aucun sens sans lui. `_MODE_BADGE_LABEL` affiche "Release"/"Dev" dans
l'UI ; les identifiants internes (`_MODE_VISUALISATION`/`_MODE_EDITION`,
valeurs stockées dans `Settings.landscape_editor_mode`) restent
`"visualisation"`/`"edition"`, seul le libellé UI a changé.

**Liste des continents par mode** : `_ensure_continent_locations_loaded()`
délègue à `_load_visualisation_continent_locations()` (inchangé -- `world.
packed_sheets` via `live_data_path`) ou `_load_edition_continent_locations()`
selon `self._app_mode`, invalidé (`self._cont_locs = None`) dès que le mode
change en cours de session. En édition, la liste vient directement de
`<ryzom-data>/leveldesign/world/ryzom.world` (`pynel.ryzom_world.load_world()`,
jamais `live_data_path`/`world.packed_sheets`) via la nouvelle fonction
`continent_selector.load_continent_locations_from_world_file()` -- utilise
`WorldContinentEntry.struct_name` comme identifiant de continent (pas
`.continent_name`, documenté par `pynel` comme non fiable, ex. l'entrée
`"matis"` a `continent_name="lesfalaises"`), puis filtrée par
`land_loader.find_land_files()` (nouveau module, scan récursif de
`<ryzom-data>/leveldesign/landscape/`, indexé par stem de fichier -- confirmé
2026-09-10 que le stem d'un `.land` réel correspond toujours au `PacsRBank`,
ex. `fyros.land`/`matis.land`/`nexus.land`, jamais à un nom de dossier) : un
continent listé dans `ryzom.world` mais sans `.land` correspondant
n'apparaît pas du tout dans le combo en édition.

**Fallback `.land`+brique pour `[POLY]`/`[2D]`** (`land_geometry.py`, nouveau
module) : en édition, quand `self.render_mode` est `"2D"` ou `"POLY"`,
`_apply_render_mode()` résout `(land_path, brick_zones_dir)` pour le continent
sélectionné (`land_loader.find_land_files()` + `continent_ecosystem.
get_ecosystem_for_continent()` pour situer `<ryzom-data>/pipeline/landscape/
<eco>/zones/`), et `_run_load_refs()` (thread d'arrière-plan) charge le
`.land` (`pynel.ryzom_land.load_land()`), calcule les cellules déjà couvertes
par un vrai `.zone` exporté (`bb_center` des zones déjà chargées, `floor()`
et non `round()` -- une valeur exactement `.5` romprait sur la parité avec
`round()`), et pour chaque cellule utilisée du `.land` (`zone_name !=
STRING_UNUSED`) qui n'a AUCUN `.zone` exporté correspondant, charge la
brique référencée (`<eco>/zones/<zone_name>.zone`) et calcule sa géométrie
positionnée via `land_geometry.build_land_piece_cache_data()`, ajoutée à
`self.zones` sous la clé `"land:<origin_x>:<origin_y>"` et marquée dans le
même ensemble `gray` que `[WELD]`/`[LIGHT]` -- rendue avec le dégradé violet
-> rose de repli habituel (`build_zone_geom_from_cache(..., fallback=True)`),
jamais le dégradé d'élévation normal (Nuno 2026-09-10).

**Pièces multi-cellules** (Nuno 2026-09-10 : "certains .zone ne font pas
160x160 mais peuvent être plus grandes 320x160 voire 320x320") : une brique
peut être une "large piece" ligo référencée par plusieurs cellules du
`.land` à la fois, chacune stockant son propre `ZoneUnit.pos_x`/`pos_y` --
un nom trompeur ("position in a large piece", `zone_region.h`), pas une
position de grille, mais la sous-position de cette cellule dans la pièce.
Un premier essai traitait chaque cellule référençant la brique comme un
placement 160x160 indépendant, dupliquant/chevauchant la pièce sur chaque
cellule qu'elle occupe réellement (trouvé 2026-09-10, Nuno, sur `49_CL`/
`49_CM` de `nexus`). Corrigé : `land_geometry.piece_origin()` reproduit
`CExport::treatPattern()`'s `deltaX`/`deltaY` (export.cpp:479-498) pour
retrouver l'origine de grille propre à la pièce depuis UNE cellule
référençante + sa taille en cellules -- taille dérivée directement de la
bounding box réelle de la brique déjà chargée (`land_geometry.
brick_size_in_cells()`, Nuno 2026-09-10 : "quand tu charges un .zone tu dois
bien avoir les dimensions"), jamais d'une base "ZoneBank" ligo (hors scope,
non parsée par pynel). Toutes les cellules d'une même pièce résolvent vers
la même origine -- `_run_load_refs()` déduplique dessus (`rendered_pieces`,
clé `(zone_name, origin_x, origin_y, rot, flip)`) pour ne rendre la pièce
qu'une seule fois.

`build_land_piece_cache_data()` positionne la pièce (une cellule simple
n'étant que le cas `size_x == size_y == 1`) en tournant/retournant **autour
du centre de sa propre bounding box réelle** (`x`/`y` dans `[0, width]`/
`[0, height]`, pas un `ZONE_CELL_SIZE` fixe -- mirror d'abord si `flip` :
`x = width - x`, puis rotation par `rot * 90°` via `rot=1: (height-y, x)` /
`rot=2: (width-x, height-y)` / `rot=3: (y, width-x)`), puis translation de
`(origin_x, origin_y) * ZONE_CELL_SIZE`. Une première version tournait/
retournait autour du coin local (0,0) en lisant littéralement la
construction `CMatrix` de `CExport::transformZone()` -- position
complètement fausse en pratique (trouvé et corrigé 2026-09-10, Nuno) ; une
deuxième version centrait bien la rotation mais sur un `ZONE_CELL_SIZE` fixe
plutôt que la vraie taille de la pièce -- correcte pour une brique simple,
fausse dès qu'une pièce multi-cellules était tournée. Vérifié empiriquement
(pas juste relu) contre les vraies zones exportées de `bagne`
(`ryzom-data/pipeline/export/continents/bagne/`, présentes en local),
pièces multi-cellules tournées incluses (`solprimer-mz_monticulea`/
`solprimer-mz_coulea`) : 45 des 53 cellules utilisées de `bagne.land`
tombent à moins de 2 unités du centre réel (bruit flottant/mesh), les 5
restantes à moins de 28 unités (mesh légèrement irrégulier, pas un bug
systématique identifié) -- les anciennes erreurs de 100 à 200+ unités et le
chevauchement des pièces multi-cellules ont disparu.

Appliqué directement aux positions déjà évaluées de la surface Bézier
(`zone_geometry.compute_zone_patch_positions()`, réutilisé tel quel) plutôt
qu'aux points de contrôle bruts : une surface de Bézier est une combinaison
affine de ses points de contrôle (base de Bernstein de somme 1), donc une
transformation affine commute avec l'évaluation.

## Nom de zone + position sous le curseur (`landscape_editor__cursor_zone_status.md`)

`_update_cursor_status()` (appelé en tête de `draw_panel()`) affiche, dans
la barre de statut partagée (`SysInfoBar`, à droite des FPS, séparé par le
même mécanisme que le statut Explorer), le nom de la zone 160x160 sous le
curseur et sa position monde entre parenthèses -- ex. `27_AG (2541, -4848)`.
`mouse_picking.mouse_ground_position()` (nouveau module, zéro dépendance
app) unprojette la souris via `camLens.extrude()` et intersecte le rayon
avec le plan `Z=0` (pas un vrai raycast contre le terrain tessellé -- juste
`(x, y)`, pas d'altitude réelle, hors scope). `pynel.ryzom_packed_sheets.
world_pos_to_zone_name()` (nouveau, inverse de `zone_name_to_world_pos()`,
porté de `CExport::getZoneNameFromXY()`) convertit la position en nom de
zone, `None` hors de la grille valide `[0, 255]`. `SysInfoBar.cursor_info`
est un champ dédié, séparé de `SysInfoBar.status` (déjà utilisé par
l'affichage de sélection Explorer partagé, `app.py`) pour ne pas entrer en
conflit avec lui.

**Nom de la brique `.land`** (Nuno 2026-09-10) : en mode édition, la ligne
affiche en plus le nom que le `.land` assigne lui-même à cette cellule --
toujours ce nom-là, quel que soit le fichier réellement utilisé pour le
rendu (`.zone`/`.zonew`/`.zonel` exporté, ou brique de fallback) : ex.
`27_AG (2541, -4848) -- solprimer-mz_coulea`. Deux essais précédents
montraient plutôt le fichier réellement chargé (`ZoneRef.source_path`, ou la
brique de fallback) -- rejetés par Nuno, le premier car redondant avec le
nom de zone déjà affiché juste avant (`"49_CK -- 49_CK"`, les fichiers
exportés étant nommés d'après leur position), le second parce que ce n'est
pas l'info voulue : le nom de brique du `.land`, pas le fichier de sortie du
pipeline. `_ensure_land_cell_names_loaded()` lit le `.land` du continent
sélectionné une fois par changement de continent (`self._land_cell_names`,
`(pos_x, pos_y) -> ZoneUnit.zone_name`, cellules `< UNUSED >` exclues),
vidé hors mode édition ou sans continent sélectionné.

Étape suivante du chantier land_preview : extraction en mixins dédiés par
mode une fois cette logique édition bien distincte.
