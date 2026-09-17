# landscape_editor (Atyscape)

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/landscape_editor.py`

Nommé "Atyscape" -- même convention que `object_editor.py`/"Patina".

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
  Patina pour configurer un réglage dont lui seul a besoin.

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
l'étape 5 ("Rendu par élévation") pour un rendu plus lisible ; tourne maintenant sur la vraie surface évaluée, pas seulement
les 4 coins.

**Explorer** : filtre par défaut `"*"` (pas `"*.land"`) -- un `.bnp` qui ne
contient aucun fichier correspondant au filtre actif est entièrement caché
par `explorer.py` (`_draw_dir_contents()`), et les vrais `*_zones.bnp` d'une
installation Ryzom Live ne contiennent jamais de `.land` (avec un filtre
`"*.land"` par défaut, plus aucun `.bnp` n'apparaissait).

Bouton "Top view (2D)" dans le panel : sert de "vue 2D projetée" en
réutilisant `OrbitCamera.snap_to_axis("+z")` sur le même maillage, pas de
rendu 2D séparé.

## Étape 6 -- chargement d'un continent entier via cache disque

Voir `project-todos/forgery/landscape_editor__zone_disk_cache.md`. Le
chargement par région autour de la caméra (étape 6
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
même chose manuellement (les deux existent). Une
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

Mesuré sur 53 zones (100% cache hit) : le cache disque a bien
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

Trois ajustements faits en testant un continent entier réel :

**Zoom caméra** : `OrbitCamera.max_distance` (2000.0 par défaut, partagé
avec Patina) ne permettait pas de reculer assez pour voir un continent
entier -- multiplié par 3 puis par 6 (`self.orbit_camera.max_distance *=
6.0`, 12000.0, 3x ayant à son tour été jugé encore insuffisant)
dans `LandscapeEditorApp.__init__`, spécifique à Atyscape (ne
touche pas `camera.py` ni Patina). Ça débloque aussi l'auto-cadrage après
chargement d'un continent (`frame()` clampait déjà à `max_distance`).

**Near/far du lens** : `ForgeryApp.__init__` (`app.py`) fixe un near/far de
`(0.02, 20000.0)`, un ratio 1 000 000:1 pensé pour l'inspection rapprochée
de détails de shape par Patina -- Atyscape ne s'approche jamais autant
(zones de 160 unités, orbite démarrant à 200). Ce ratio coûtait de la
précision de depth buffer : la géométrie plate à Z=0 (le quadrillage de
zone) subissait du z-fighting contre le terrain traversant Z=0, avec un
rendu différent selon la distance caméra (un palier de zoom suffisait à
changer le résultat). Réglé en resserrant à
`self.camLens.set_near_far(1.0, 20000.0)` (ratio 20 000:1).

**Dégradé d'élévation ancré à Z=0** (`zone_geometry._elevation_colors_uint8()`) :
la première version (un dégradé marron->vert normalisé sur le min/max Z de
l'ensemble chargé) était lissée/washed-out par quelques zones très
profondes (sous-marines/grottes) qui tiraient le minimum très bas -- le
terrain normal se retrouvait alors tout au même ton près du "haut" de la
plage. Remplacé par un dégradé en deux segments ancré sur Z=0 (le niveau de
l'eau) : rouge foncé au point le plus profond -> marron à
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
migré depuis une checkbox du panel, même pattern que
`object_editor.py`/Patina : `_icon_button()` réutilisé tel quel depuis
`object_editor_mixins/ui_helpers.py`, et le même mécanisme
`_viewport_toggle_size` de mesure sur la frame précédente pour positionner
la barre exactement, `large_icon_font` inclus). Toujours affiché par-dessus
le terrain quel que soit son relief : `set_depth_test(False)`/
`set_depth_write(False)` + `set_bin("fixed", 100)`, plat à Z=0 -- une grille
de repère, pas un vrai maillage à suivre le terrain.

**Tentative abandonnée -- plan d'eau approximatif** : une surface bleue
semi-transparente à Z=0 (un quad par zone) a été essayée comme
raccourci visuel en attendant la vraie détection d'eau via `.ig`/
`CWaterShape` (étape 9). Abandonnée : elle lisait comme "eau" tout relief
simplement sous le niveau de la mer, pas seulement les vrais lacs, couvrant
en bleu la majorité d'un continent réel. Retirée entièrement
(`build_water_plane_geom()`/`_WATER_PLANE_COLOR` dans `zone_geometry.py`,
état/toggle dans `landscape_editor.py`) ; seule la détection réelle par
`.ig` reste au programme.

## Modes de rendu [POLY][WELD][LIGHT] -- retiré (`landscape_editor__zone_render_modes.md`, historique)

**Retiré entièrement** (`landscape_editor__render_modes_removal.md`)
-- voir "Suppression des modes de rendu + nettoyage de l'interface" tout en bas de ce document
pour le comportement actuel (résolution automatique et unique
`.zonel > .zonew > .zone`, plus aucun bouton). Section ci-dessous gardée
telle quelle pour mémoire historique -- `self.render_mode`/`_RENDER_MODES`/
`_resolve_zone_for_mode()`/`_draw_render_mode_bar()` n'existent plus.

Barre de 3 boutons (`_draw_render_mode_bar()`) contrôlant `self.render_mode`
(`"POLY"/"WELD"/"LIGHT"`, défaut `"POLY"`) -- re-résolvent, pour chaque zone
du continent chargé (`self._loaded_refs`/`self._loaded_extensions`, name ->
ZoneRef / name -> {ext: ZoneRef}), quelle extension réelle afficher via
`_resolve_zone_for_mode()` :

**Un 4ᵉ mode `[2D]` a existé puis a été retiré** (`landscape_editor__2d_3d_toggle.md`)
-- il résolvait exactement la même géométrie que `[POLY]`, seule la caméra changeait. Voir
plus bas "Bascule caméra 2D/3D" : ce comportement vit maintenant entièrement
côté caméra, indépendant de `self.render_mode`.

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
  qu'à une zone grisée. Violet/rose n'apparaît
  jamais dans le dégradé d'élévation réel (rouge/marron/vert), donc se
  distingue sans ambiguïté à la fois du terrain et du fond.

Le rendu Explorer (`on_selection_changed()`, sélection d'un fichier unique)
n'est **jamais** passé par cette logique de mode -- charge toujours
exactement le fichier cliqué. Ce chemin n'est de toute façon jamais utilisé
en pratique : le workflow réel charge toujours un continent
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
introduit `Settings.ryzom_data_path`, doublon corrigé depuis).

**Piège** : le nom affiché dans le combo continent
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

**Chemins `ryzom-data` -- lequel est le bon** : les vrais
`.land`/briques de leveldesign vivent sous `leveldesign/landscape/`
(actuel, maintenu -- ex. `leveldesign/landscape/desert/fyros.land`, daté
2025). `graphics/landscape/ligo/` contient de **vieilles données obsolètes**
(un même nom de fichier peut y être un format binaire legacy daté de 2020,
voire absent) -- confirmé par comparaison octet-à-octet sur les 27 `.land`
existants (19 identiques, 7 différents dont au moins un format binaire
périmé, 1 manquant). Ne jamais lire `graphics/landscape/ligo/` pour du
leveldesign actif.

### Installation automatique des données pipeline (`landscape_editor__zone_render_modes__pipeline_data_installer.md`)

Au chargement d'un continent en mode édition, `_load_continent()` vérifie si
les données `build_gamedata` nécessaires sont déjà installées sous
`<ryzom-data>/pipeline/` (`pipeline_data_installer.is_installed(category,
name)`) : le continent lui-même (`pipeline_continents`), l'export de son
écosystème (`pipeline_ecosystems`) et les bricks `.zone` brutes de son
écosystème (`landscape`, jamais optionnelle -- seul point de départ pour
composer un continent depuis son `.land` tant qu'aucun `.zone` par continent
n'a été généré via `land_export`). Si l'une manque, une proposition de
téléchargement/extraction (archives `.zip`,
`PipelineDataInstallDialog`) est ouverte au prochain `draw_panel()` -- jamais
depuis l'intérieur du popup du combo continent lui-même (piège ImGui : un
`imgui.open_popup()` pour un popup différent pendant qu'un autre est encore
ouvert est silencieusement perdu).

### Réglage `ryzom_tools_path` et génération de `.zonew` manquants (`[WELD]`)

`Settings.ryzom_tools_path` (onglet Settings, section "Ryzom Paths") pointe
vers un **dossier** contenant les exécutables natifs du pipeline (`zone_welder`
pour l'instant, d'autres à venir sans réglage supplémentaire) -- sélecteur de
dossier, persistant, partagé entre Patina et Atyscape.

`ryzom_forgery/zone_tools.py` (`run_zone_welder(ref, live_data_path,
persist_to=None)`) résout `zone_welder`/`zone_welder.exe` dans ce dossier,
rassemble la zone cible et ses voisines de grille déjà weldées
(`.zonew`/`.zonel`, une zone voisine encore `.zone` brute est simplement
exclue de l'appel plutôt que weldée en cascade) dans un dossier temporaire,
appelle le binaire et relit le `.zonew` produit. `ZoneToolError` dédiée si
`ryzom_tools_path` n'est pas configuré, si le binaire est absent, ou si
l'appel échoue/ne produit rien.

Le bouton "Generate N missing .zonew" (`_draw_render_mode_bar()`, visible
uniquement en `[WELD]` avec des zones manquantes) lance
`_generate_missing_zonew()` en tâche de fond, une zone à la fois. Chaque
`.zonew` produit est écrit directement sur disque via `persist_to`
(`zone_tools.missing_zonew_dest()`) sous
`<ryzom-data>/pipeline/export/continents/<continent>/zone_weld/<nom>.zonew`
(même racine `ryzom-data` déjà utilisée en lecture, jamais un second
réglage) -- persistant, redétecté sans regénération après relance puisque
cette source n'est jamais mise en cache (voir plus haut).

**Affichage live** : plutôt que d'attendre la fin du lot,
chaque zone weldée passe du dégradé violet/rose au dégradé d'élévation dès
que son propre `.zonew` est écrit (`progress["ready"]`, vidée à chaque frame
par `draw_panel()`), sans toucher aux zones encore en attente ; le compteur
"N manquantes" décroît au fur et à mesure (`_recompute_missing_for_mode()`).

**Cellules `.land` sans aucun `.zone`** : le repli `.land`+
brique (voir plus bas, historiquement réservé à `[POLY]`, `[2D]` n'étant
alors qu'une bascule caméra sur le même mode -- voir "Bascule caméra 2D/3D"
plus bas) s'applique aussi à `[WELD]`, pour que ces cellules deviennent visibles (dégradé violet/
rose) et comptées dans "N manquantes" au lieu d'être simplement invisibles.
`zone_welder` n'a rien à souder pour elles (aucun `.zone` réel n'existe) :
produire ce `.zone` est le rôle de `land_export`, tracé séparément comme
`landscape_editor__land_composition.md` étape 4, elle-même bloquée par
`project-todos/pynel/land_pipeline.md` étape 1. Le bouton "Generate" ne tente
donc jamais de les souder -- il rapporte une erreur explicite par cellule
("no .zone exported yet -- finish land_composition (land_export) first")
sans jamais bloquer la génération des zones qui, elles, peuvent réellement
être weldées.

## Mode global Visualisation / Édition (`landscape_editor__land_preview.md`)

`_detect_app_mode()` détermine le mode par défaut, uniquement la toute
première fois (`pynel.repository_paths.is_valid("ryzom-data")` -- `"edition"`
si `ryzom-data` est configuré et pointe vers un dossier existant,
`"visualisation"` sinon), indépendamment du contenu réel de ce `ryzom-data`
(un `.land` manquant pour tel ou tel continent est géré par le filtrage du
combo, pas par ce switch global).

**Bascule manuelle "Release"/"Dev"** (`landscape_editor__land_preview.md`
étape 4) : `_resolve_app_mode()` lit `Settings.
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
que le stem d'un `.land` réel correspond toujours au `PacsRBank`,
ex. `fyros.land`/`matis.land`/`nexus.land`, jamais à un nom de dossier) : un
continent listé dans `ryzom.world` mais sans `.land` correspondant
n'apparaît pas du tout dans le combo en édition.

**Fallback `.land`+brique pour `[POLY]`/`[WELD]`** (`land_geometry.py`, nouveau
module ; étendu à `[WELD]`, voir plus haut "Cellules `.land`
sans aucun `.zone`") : en édition, quand `self.render_mode` est `"POLY"` ou
`"WELD"`, `_apply_render_mode()` résout `(land_path, brick_zones_dir)` pour le continent
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
jamais le dégradé d'élévation normal.

**Pièces multi-cellules** (certaines briques `.zone` ne font pas
160x160 mais peuvent être plus grandes, 320x160 voire 320x320) : une brique
peut être une "large piece" ligo référencée par plusieurs cellules du
`.land` à la fois, chacune stockant son propre `ZoneUnit.pos_x`/`pos_y` --
un nom trompeur ("position in a large piece", `zone_region.h`), pas une
position de grille, mais la sous-position de cette cellule dans la pièce.
Un premier essai traitait chaque cellule référençant la brique comme un
placement 160x160 indépendant, dupliquant/chevauchant la pièce sur chaque
cellule qu'elle occupe réellement (trouvé sur `49_CL`/
`49_CM` de `nexus`). Corrigé : `land_geometry.piece_origin()` reproduit
`CExport::treatPattern()`'s `deltaX`/`deltaY` (export.cpp:479-498) pour
retrouver l'origine de grille propre à la pièce depuis UNE cellule
référençante + sa taille en cellules -- taille dérivée directement de la
bounding box réelle de la brique déjà chargée (`land_geometry.
brick_size_in_cells()`), jamais d'une base "ZoneBank" ligo (hors scope,
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
complètement fausse en pratique (trouvé et corrigé) ; une
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

**Cache disque des bricks de repli** (perf : mesuré sur
`nexus`, 17 cellules de repli, 2.135s -> 0.134s, ~16x) : `build_land_piece_cache_data()`
refaisait un parse + une vraie tessellation Bézier (`compute_zone_patch_positions()`)
à chaque chargement, pour rien -- le fichier brique ne change jamais entre deux
chargements. Découpé en deux caches disque distincts, tous deux via
`zone_cache.py` (même mécanisme que le cache par zone de l'étape 6) :
- **Cache de la géométrie brute** (`"land_brick_<nom>"`, non transformée) --
  `zone_geometry.zone_to_cache_data()` du brick parsé une seule fois, réutilisé
  pour tous ses placements (rotations/positions) ; sur un hit, `load_zone()`
  ne tourne même plus (sa bounding box vient directement du cache, via la
  nouvelle `land_geometry.brick_size_in_cells_from_half_size()`).
- **Cache de la pièce transformée** (`"land_piece_<nom>_<origin_x>_<origin_y>_<rot>_<flip>"`)
  -- la nouvelle `land_geometry.transform_zone_cache_data()` (extraite de
  l'ancienne `build_land_piece_cache_data()`, qui n'est plus qu'un fin wrapper
  autour d'elle) applique la transformation position/rotation/flip à une
  `ZoneCacheData` déjà tessellée (brute ou déjà en cache), sans jamais
  ré-évaluer Bézier.

Invalidation : la clé de cache par brick inclut le fichier `.zone` source
(mtime/taille, comme d'habitude) ; en plus, un stamp unique sur le fichier
`.land` lui-même (`"__land_file__"`, utilisé par le cache de continent
complet ci-dessous) invalide tout repli d'un coup si le `.land` change de
disposition (brique/rotation réassignée à une cellule) sans que le fichier
brique référencé change.

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

**Nom de la brique `.land`** : en mode édition, la ligne
affiche en plus le nom que le `.land` assigne lui-même à cette cellule --
toujours ce nom-là, quel que soit le fichier réellement utilisé pour le
rendu (`.zone`/`.zonew`/`.zonel` exporté, ou brique de fallback) : ex.
`27_AG (2541, -4848) -- solprimer-mz_coulea`. Deux essais précédents
montraient plutôt le fichier réellement chargé (`ZoneRef.source_path`, ou la
brique de fallback) -- rejetés, le premier car redondant avec le
nom de zone déjà affiché juste avant (`"49_CK -- 49_CK"`, les fichiers
exportés étant nommés d'après leur position), le second parce que ce n'est
pas l'info voulue : le nom de brique du `.land`, pas le fichier de sortie du
pipeline. `_ensure_land_cell_names_loaded()` lit le `.land` du continent
sélectionné une fois par changement de continent (`self._land_cell_names`,
`(pos_x, pos_y) -> ZoneUnit.zone_name`, cellules `< UNUSED >` exclues),
vidé hors mode édition ou sans continent sélectionné.

### Extraction en mixins (`landscape_editor__land_preview.md` étape 5)

`landscape_editor.py` (`LandscapeEditorApp(EditModeMixin, ViewModeMixin,
ForgeryApp)`) délègue maintenant la logique propre à chaque mode à deux
mixins dédiés, même pattern que Patina (`object_editor_mixins/`) :

- **`landscape_editor_edit_mode.py`** (`EditModeMixin`) : `_load_edition_continent_locations()`,
  `_ensure_land_cell_names_loaded()`, `_resolve_land_fallback_paths()`/
  `_load_land_fallback_pieces()` (le fallback `.land`+brique, extrait de
  `_apply_render_mode()`/`_run_load_refs()`), `_generate_missing_zonew()`/
  `_run_generate_missing_zonew()` (bouton `[WELD]`, sans objet hors édition
  puisque `ryzom_data_path` y est toujours `None`).
- **`landscape_editor_view_mode.py`** (`ViewModeMixin`) : `_load_visualisation_continent_locations()`
  -- aussi mince que ça, la Visualisation n'a aucune logique propre au-delà
  de la lecture de `live_data_path`.
- **`landscape_editor_modes.py`** : constantes/helpers partagés entre le
  fichier principal et les deux mixins (`_RENDER_MODES`, `_MODE_REAL_EXTENSIONS`,
  `_resolve_zone_for_mode()`, `_MODE_VISUALISATION`/`_MODE_EDITION`,
  `_detect_app_mode()`...) -- un module séparé plutôt que de les laisser dans
  `landscape_editor.py`, pour que les deux mixins puissent les importer sans
  jamais importer `landscape_editor.py` lui-même (import circulaire), même
  raisonnement que `object_editor_mixins/ui_helpers.py`.

Le pipeline de rendu partagé (`zone_geometry.py`/`zone_cache.py`/
`zone_geom_cache.py`, `_apply_render_mode()`/`_run_load_refs()`/
`_set_loaded_zones()`) reste dans `landscape_editor.py` -- il n'est jamais
dupliqué entre les mixins, seuls les points d'extension propres à un mode
(résolution du fallback `.land`, génération `.zonew`, source de la liste de
continents) sont délégués. Non-régression des deux modes validée
sur un continent réel.

## Cache disque du `NodePath` d'une zone (`landscape_editor__region_management__zone_bam_cache.md`)

Même une fois le cache par zone tessellée (étape 6) et celui des bricks de
repli (ci-dessus) chauds, il restait un coût mesuré à 1.349s pour 151 zones
(~8.9ms/zone, nexus) : la construction des `GeomNode` Panda3D elle-même
(couleur + indices + `attach_new_node`, `_set_loaded_zones()`), entièrement
sur le thread principal. Objectif : repasser sous 1s de
temps total de rechargement, quitte à geler l'UI le temps de la
(re)construction -- le gel n'est pas un problème tant que le total est
rapide.

**`ryzom_forgery/zone_geom_cache.py`** sérialise le `NodePath` déjà construit
via le format natif Panda3D (`.bam`, `NodePath.write_bam_file()`/
`Loader.load_model(..., noCache=True)`), plutôt que de refaire tourner
`GeomVertexWriter`/numpy à chaque chargement. **Un bundle par ZONE** (pas par
continent) -- remplace un module antérieur (`continent_geom_cache.py`, un
seul bundle `.bam` par `(continent, mode)`) qui ne payait plus une fois le
chargement paresseux par région (`landscape_editor__region_management.md`)
en place : ce bundle unique était réécrit à chaque bascule de région (le jeu
de zones chargées change), donc plus jamais réutilisé d'une bascule à
l'autre. Un bundle par zone, sous `cache_dir() / "zone_geom_cache"`, reste
valide peu importe quelles AUTRES zones sont chargées à côté.

**Le manifeste** (`ZoneManifestEntry` par zone/pièce `.land` : extension
résolue + mtime/taille du fichier source, `rot`/`flip` -- une brique
retournée/pivotée EN PLACE, même fichier/mtime/taille, a sinon une signature
de cache identique à sa version non tournée -- plus `min_z`/`max_z`, voir
ci-dessous) est comparé au manifeste fraîchement recalculé (`_run_load_refs()`
le construit à la volée, `progress["manifest_zones"]`) **avant** de toucher
au `.bam` de cette zone : identique -> chargement direct du bundle, renommé
par zone (`node_path.set_name(name)`, le `GeomNode` de
`build_zone_geom_from_cache()` s'appelant toujours `"zone-tessellated"`, sans
quoi deux bundles rechargés seraient indiscernables) puis reparenté
directement sous `self._zone_root`. Différent -> reconstruction de CETTE
zone seule (jamais pire que l'existant, et jamais un miss d'une zone ne
force la reconstruction d'une autre), puis sauvegarde du nouveau bundle.

**`min_z`/`max_z` -- référence sur TOUT le continent, pas le sous-ensemble
chargé** : le dégradé d'élévation dépend de la plage globale de tout ce qui
est affiché (voir l'étape 7 plus haut), qui changerait à chaque bascule de
région si elle était recalculée sur le seul sous-ensemble coché -- invalidant
le cache de TOUTE zone à chaque coche/décoche. `region_loader.
get_continent_z_range()` calcule donc cette plage une seule fois par
chargement de continent, sur TOUTES ses zones réelles (`self.
_all_continent_refs`), via `pynel.ryzom_zone.parse_zone_header()` (lecture
d'en-tête seule, `zone_bb` uniquement -- **jamais** `parse_zone()` complet :
bug de perf réel trouvé en testant, `parse_zone()` sur un `.zonel` de 423 Ko
prend 268ms + 14,3 Mo retenus par zone, contre 0,02-0,05ms pour l'en-tête
seule, ~5000x plus rapide -- voir `project-todos/pynel/
zone_header_reader.md`). Effet de bord accepté : une petite région
isolée a le même étalonnage de couleur que si tout le continent était
chargé, au lieu d'être renormalisée sur elle-même seule.

Un cache hit se lit en quelques dizaines de ms (chargement `.bam` seul, testé
en isolation headless) contre plus d'une seconde de reconstruction --
confirmé fonctionnel sur un rechargement répété du même continent,
actif en Visualisation ET en Édition (la désactivation spéciale en Édition
qui existait pour l'ancien cache par continent n'a plus lieu d'être, voir
plus bas).

## Bascule caméra 2D/3D (`landscape_editor__2d_3d_toggle.md`)

`[2D]` a été retiré de la barre de modes de rendu (voir plus haut) et
remplacé par un bouton icône dédié (`ICON_FA_CUBE`) dans
`_draw_viewport_toggles()`, à côté de celui de la grille -- purement un
comportement caméra, indépendant de `self.render_mode`.

- **2D par défaut au lancement** : `self._top_down_locked = True` dès
  `__init__`, `OrbitCamera` construite directement avec l'orientation
  `AXIS_VIEWS["+z"]` (vue du dessus) plutôt qu'un `snap_to_axis()` animé
  depuis le défaut (0, 0) -- rien à animer au tout premier affichage.
- **`OrbitCamera.lock_rotation`** (`camera.py`, nouveau flag, `False` par
  défaut -- partagé avec Patina mais inoffensif tant qu'aucune app ne le
  touche) : quand actif, `_update()` ignore le drag de rotation (bouton
  gauche) ; pan (bouton milieu) et zoom (bouton droit/molette) continuent de
  fonctionner normalement.
- **`_toggle_top_down()`** : en passant en 2D, sauvegarde l'orientation 3D
  courante (`self._saved_3d_heading_pitch`) puis `lock_rotation = True` +
  `snap_to_axis("+z")` (animé). En repassant en 3D, `lock_rotation = False`
  puis `OrbitCamera.animate_to_orientation(heading, pitch)` (nouvelle
  méthode, animée comme `snap_to_axis()` -- qui délègue maintenant à elle --
  plutôt qu'un saut instantané : la transition 3D->2D
  animait déjà bien via `snap_to_axis()`, 2D->3D doit se sentir pareil)
  restaure cette orientation, ou `(heading=0°, pitch=45°)` par défaut si
  aucune bascule 2D n'a encore eu lieu depuis le lancement.

## Transparence 50% et wireframe sur le terrain (`landscape_editor__transparency_wireframe.md`)

Deux boutons icône indépendants et combinables dans `_draw_viewport_toggles()`,
à côté de celui du 2D/3D -- même mécanisme que Patina (`_toggle_object_transparency()`/
`_apply_object_wireframe()`, `object_editor_mixins/viewport_transform.py`),
appliqués à `self._zone_root` (donc à toutes les zones chargées, quel que
soit `self.render_mode` ou l'état 2D/3D) :

- **Transparence** (`self._zone_transparent`, `ICON_FA_CIRCLE_HALF_STROKE`) :
  `TransparencyAttrib.M_alpha` + `set_color_scale(1, 1, 1, alpha)`.
- **Wireframe à 3 états** (`self._wireframe_mode`, `"off"`/`"overlay"`/`"pure"`,
  `_apply_zone_wireframe`/`_cycle_zone_wireframe`/`_set_zone_wireframe_mode`,
  `ICON_FA_DRAW_POLYGON`, project-todos/forgery/`wireframe_cycle_states.md`
  -- remplace l'ancien booléen on/off) : `"off"` aucune
  surcharge ; `"overlay"` (ancien comportement) `set_render_mode_filled_wireframe((0, 0, 0, 1), 1)`
  -- arêtes noires par-dessus le rendu texturé/ombré normal, ne le remplace
  jamais (`object_editor__wireframe_overlay.md`) ; `"pure"` (nouveau)
  `set_render_mode_wireframe(1)` + `set_texture_off(1)` -- filaire seul, sans
  texture ni remplissage. Même bouton/icône dans les 3 états (actif dès que
  l'état n'est pas `"off"`), clic gauche cycle, clic droit ouvre un popup
  listant les 3 états pour y sauter directement (convention générale des
  boutons à cycle Forgery). `self._zone_root` n'étant jamais détruit/recréé
  (contrairement à `model_root` de Patina), l'état n'a jamais besoin d'être
  ré-appliqué après un rechargement de continent.

**Chantier abandonné** : un bouton cyclique Shading/Constant Shading/Unshaded
(portage de celui de Patina, `docs/apps/object_editor.md`) avait été ajouté
ici aussi puis retiré après test -- seul le mode Shading normal
(déjà le comportement par défaut, sans bouton) s'est révélé utile sur le
terrain (`logs/forgery.md`).

## Sélection de zone (pivot de rotation)

Clic gauche sans glisser (`_on_zone_click_down`/`_on_zone_click_up`, seuil
`_ZONE_CLICK_MAX_DRAG` en coordonnées souris normalisées -- distingue un
simple clic d'un drag d'orbite `OrbitCamera`, tous deux liés au même bouton
`mouse1`) sur une zone (`_select_zone_at_cursor()`, même calcul
position-sous-le-curseur -> `world_pos_to_zone_name()` que la barre de
statut du curseur) :

- Dessine sa bordure en orange, épaisseur double de celle de la grille
  (`build_zone_selection_border_geom()`, `zone_geometry.py`), avec le même
  traitement "toujours visible par-dessus le terrain" que `self._grid_np`
  (`set_light_off`/`set_depth_test(False)`/`set_depth_write(False)`,
  `set_bin("fixed", 101)` -- un cran au-dessus des 100 de la grille pour
  passer devant elle, la bordure sinon se voyant mal).
- Fait du centre de la zone (Z=0, même ancrage niveau-de-la-mer que le
  dégradé d'élévation) le nouveau pivot de rotation de la caméra, via
  `OrbitCamera.retarget()` (`camera.py`, nouveau -- comme `frame()` mais ne
  touche pas à la distance, aucun zoom automatique voulu à la sélection).
- Un clic hors de toute zone valide désélectionne (bordure retirée).

`zone_name_to_world_pos()` décode le bord **nord** (Y max) d'une zone, pas
son bord min (vérifié empiriquement : `zone_name_to_world_pos("62_AG").y ==
-9920.0`, et `world_pos_to_zone_name(x, -9920.0) == "62_AG"` mais
`world_pos_to_zone_name(x, -9919.0) == "61_AG"`) -- contrairement à X (bord
ouest, sans ambiguïté). Un premier essai traitait à tort `origin.y` comme le
bord min, décalant la bordure d'une rangée vers le nord -- en réalité seul
l'affichage de la bordure était décalé, le nom réellement sélectionné était
déjà correct.

**Sélection par bounding box réelle, pas par nom de zone 160×160** :
`_select_zone_at_cursor()` ne convertit plus la position
curseur en nom via `world_pos_to_zone_name()` -- `_find_loaded_zone_at(x, y)`
cherche directement dans `self.zones` laquelle des zones/pièces réellement
chargées couvre `(x, y)` par sa propre bounding box (`bb_center`/
`bb_half_size`). Une pièce `.land` multi-cellules (320×160, etc.) sélectionne
alors tout son vrai contour d'un coup, plutôt que la seule tranche 160×160
sur laquelle le clic est tombé -- une zone couvrant plusieurs cellules
sélectionne donc désormais tout son contour, pas juste la tranche cliquée.
`_select_zone()` dessine la bordure sur cette
même vraie bounding box, et ne retargete plus la caméra en vue 2D
(`self._top_down_locked`) -- la vue du dessus n'a pas de pivot d'orbite à
gérer, la caméra ne bouge donc pas en vue 2D.

## Composition `.land` en 3D + bouton Build (`landscape_editor__land_composition.md`)

Remplace l'outil de composition Ligo 2D historique par une vraie édition 3D
du `.land` : chaque brick est affichée à sa vraie position/rotation/flip
avec son vrai relief, une cellule de la grille se choisit/s'édite
directement, et un bouton "Build" déclenche le pipeline natif complet.
Validé sur une vraie composition (`bagne`).

### Rendu par étape de build (remplace le dégradé binaire réel/repli)

`zone_geometry.zone_build_stage(ext_map)` renvoie 0-3 selon l'extension réelle
la plus avancée présente sur disque pour une zone (`.zonel`=3, `.zonew`=2,
`.zone`=1, rien=0 -- un `.land` brut affiché en repli). Chaque étage a son
propre dégradé sombre->clair (`zone_geometry._STAGE_COLORS`, éditable en live
depuis l'onglet Settings d'Atyscape, section "Colors", `_draw_zone_colors_
settings()` -- persistant via `Settings.landscape_zone_stage_colors`) : seul
l'étage 3 (entièrement construit) se normalise sur la plage Z de tout
l'ensemble chargé (comme l'ancien dégradé d'élévation, pour lire un
continent fini comme un seul paysage continu) ; les étages 0-2 restent
normalisés sur leur propre plage Z (leur relief, souvent une infime tranche
du continent, paraîtrait uniforme sinon). `build_zone_geom_from_cache()`
prend désormais un paramètre `stage` (plus `fallback: bool`), et
`_rebuild_all_zone_nodes()` reconstruit tous les `GeomNode` en place quand
une couleur d'étage change (un changement de couleur n'affecte que les zones
construites APRÈS l'appel à `set_stage_colors()`).

### Renommage `[LAND]`/masquage en Visualisation -- retiré (historique)

**Retiré** avec le reste de la barre de modes de rendu (voir
plus haut) -- `self.render_mode`/`_RENDER_MODES` n'existent plus, il n'y a
plus de libellé à renommer.

L'identifiant interne `self.render_mode`/`_RENDER_MODES` reste `"POLY"` --
seul le libellé affiché change en `"LAND"`, et uniquement en mode Édition
(Dev) : Visualisation (Release) n'a jamais accès au `.land`, donc y garde
`"POLY"` -- en pratique Visualisation n'affiche même plus la barre de
sélection de mode du tout (`_draw_render_mode_bar()`) : un vrai
`*_zones.bnp` shippé ne contient que des `.zonel` (confirmé 155/155 entrées
sur `nexus_zones.bnp`), donc les 3 boutons résoudraient tous vers le même
fichier -- remplacés par un simple libellé "Render mode: LIGHT".

### Énumération pilotée par le `.land` en Édition (anti-péremption)

`EditModeMixin._build_land_driven_refs()` remplace, en Édition uniquement,
le scan disque `region_loader.find_zones_in_region()` par une énumération
qui ne considère QUE les cellules que le `.land` référence *actuellement*
(`land_geometry.used_land_cells()`) : un `.zone`/`.zonew`/`.zonel` périmé sur
le disque (position retirée/modifiée dans le `.land` depuis son export) est
purement ignoré dans les 3 modes, jamais affiché comme s'il était encore
d'actualité. La résolution position -> nom de zone réel passe par
`land_geometry.expected_zone_name(pos_x, pos_y)` (= `world_pos_to_zone_name(pos_x
* ZONE_CELL_SIZE, pos_y * ZONE_CELL_SIZE)`) -- **toujours appelée sur le
coin EXACT de la cellule**, jamais un point intérieur : confirmé
que `world_pos_to_zone_name()` décale d'une rangée pour tout point
strictement à l'intérieur d'une cellule (`world_pos_to_zone_name(880,
-9840)`, le vrai `bb_center` de `62_AF.zone`, renvoie `"61_AF"`, pas
`"62_AF"`) -- ce même bug affectait aussi le nom de zone affiché sous le
curseur (`_update_cursor_status()`), corrigé par le même appel via
`expected_zone_name()`.

Le repli `.land`+brique (`_load_land_fallback_pieces()`) résout maintenant
les cellules déjà couvertes par une vraie zone **par nom**
(`land_geometry.land_cell_for_zone_name()`, l'inverse exact d'
`expected_zone_name()`), plus par le `bb_center` géométrique de la zone
chargée -- la bounding box réelle d'une zone ne tombe pas toujours pile au
centre de sa cellule nominale, ce qui pouvait faire apparaître à la fois une
vraie zone ET une brique de repli au même endroit, avec chacune sa propre
couleur.

### Édition de la grille (choisir/placer une brique)

`land_geometry.land_cell_index()` retrouve l'index plat `.land` d'une
cellule `(pos_x, pos_y)` (`None` hors de l'étendue courante -- agrandir la
grille elle-même est hors scope). `_draw_land_composition_editor()`
(panneau au bas de l'onglet Landscape, visible seulement en Édition, mode
`[LAND]`, avec une cellule sélectionnée) affiche la brique courante, une
checklist ✅/❌ `.zone`/`.zonew`/`.zonel` pour cette cellule (lue directement
dans `self._loaded_extensions`, jamais recalculée depuis `self.render_mode`),
un sélecteur de brique parmi celles du dossier écosystème
(`_available_land_bricks()`, mis en cache par dossier), rotation (par pas de
90°) et flip, et "Clear cell". Chaque édition
(`_apply_land_cell_edit()`/`_clear_land_cell()`) : invalide les fichiers déjà
construits pour cette cellule (voir plus bas), réécrit le `.land` en entier
(`_save_land_region()`, XML léger, pas de batching), puis ne rafraîchit QUE
la cellule éditée (`_apply_lightweight_cell_update()`, jamais un
`_load_continent()` complet -- une édition ne doit modifier qu'une seule
zone).

**Invalidation incrémentale** (`_invalidate_built_zone_files()`) : une
édition supprime `.zonew`/`.zonel`/`.depend`/`.ig` de la cellule éditée ET de
ses 8 voisines (un weld/lighting voisin peut dépendre du bord qui vient de
changer -- `zone_lighter` se relance sur toutes les zones concernées, y
compris celles qui avaient déjà un `.zonel`), mais seule la cellule éditée elle-même perd
en plus son `.zone` (sa géométrie d'élévation brute correspond à l'ancienne
brique, devenue franchement fausse plutôt que simplement "en attente d'un
rebuild") -- les `.zone` voisins ne sont jamais supprimés, leur élévation ne
dépend que de la carte de hauteur globale du continent, jamais de la
composition locale.

### Bouton "Build" (`land_build.py`, `continent_pipeline_reference.py`)

`ryzom_forgery/land_build.py` (nouveau module) orchestre la chaîne complète
`land_export` -> `zone_welder` (passe 1, sans relief) -> `zone_elevation` ->
`zone_welder` (passe 2, avec relief) -> `zone_dependencies` -> `zone_lighter`
-> `zone_ig_lighter` sur TOUT le continent courant, à partir de
`ryzom-data/leveldesign/world/continent_pipeline_reference.csv` (nouveau
module `continent_pipeline_reference.py` -- seule source de vérité
pipeline Forgery, voir `project_forgery_pipeline_vs_legacy_workspace`,
jamais `leveldesign/workspace/`). Chaque étape écrit directement dans les
vrais sous-dossiers d'export (`zone`/`zone_weld`/`zone_lighted`,
`region_loader.continent_zone_dirs()`) plutôt que dans le dossier de
transit `land_export`, pour que le résultat soit immédiatement visible par
le scan disque d'Atyscape sans étape d'installation séparée.

**Reconstruction incrémentale, par étage réel indépendant** (pas juste "a un
`.zonel`" -- deux bugs trouvés sur de vraies données `bagne`) :
une zone déjà weldée (`.zonew` présent) n'est jamais re-weldée/re-élevée
juste parce qu'elle n'a pas encore de `.zonel` ; et une zone peut avoir un
`.zonel` réel sur disque sans `.zonew` à côté (données préexistantes) --
avoir besoin d'un weld/élévation et avoir besoin d'un lighting sont deux
vérifications séparées.

**Deux bugs natifs de `zone_dependencies`**, contournés dans `land_build.py`
(mêmes bugs déjà documentés dans `nel/tools/pynel/docs/zone_tools.md` §4) :
l'échange min/max de `firstZone`/`lastZone` casse un axe si on lui passe les
deux zones réelles aux coins extrêmes -- deux noms synthétiques
`<minRow>_<minCol>`/`<maxRow>_<maxCol>` (jamais ouverts comme fichiers,
juste décodés) garantissent un ordre croissant sur les deux axes à la fois ;
et l'outil écrit un `.depend` par zone (toujours en minuscules), jamais un
fichier unique nommé d'après `argv[4]`.

**Parallélisation** (`zone_lighter`) : chaque zone est
lancée dans son propre process natif via un `ThreadPoolExecutor`
(`os.cpu_count()` workers) -- le vrai coût dominant du build (~2s/zone). Le
multi-threading INTERNE de `zone_lighter` (`PropertiesConfig.cpu_num`) reste
fixé à 1 : cumuler concurrence process ET thread ferait juste concurrencer
l'ordonnanceur OS pour rien.

**`continent_pipeline_reference.csv`, transposé** (une ligne par champ, une
colonne par continent) : tous les chemins qu'il contient sont relatifs à
`ryzom_data_path`. Une poignée de champs de `LandExportConfig`
(`z_factor_1`/`z_factor_2`/`zone_light`/`export_collisions`/
`export_additionnal_igs`/`cell_size`/`threshold`) sont identiques sur les 24
`land_exporter.cfg` réels et donc codés en dur plutôt que lus du CSV.
**`PropertiesConfig.cpu_num` n'est jamais lu du CSV** malgré une colonne
`cpu_num` existante (résidu de l'extraction initiale) : ce serait une
caractéristique de la MACHINE qui build, pas du continent -- y figer un
nombre de cœurs particulier enverrait cette valeur à tous les autres
utilisateurs de Forgery (même classe d'erreur que supposer que tout le monde
a le même `workspace/`/`graphics/` personnel, voir `feedback-forgery-multi-user-no-
personal-disk`) ; `os.cpu_count()` est lu à chaque exécution, sur la machine
qui build réellement.

**Affichage live pendant le Build** : chaque zone tout juste éclairée
apparaît immédiatement dans la vue 3D (`progress["ready"]`, même convention
que "Generate missing .zonew"), avec un rechargement complet
(`_load_continent()`) une seule fois à la fin pour faire disparaître les
anciennes pièces de repli devenues obsolètes (ajouter des zones sans
retirer les existantes n'aurait sinon jamais nettoyé les anciennes). Le
cache disque `.bam` de continent entier
(`continent_geom_cache.py`, retiré depuis, voir plus haut) était désactivé en
Édition (`continent=None, mode=None` passés à `_set_loaded_zones()`) -- un
hit inattendu avait été trouvé après suppression d'une zone construite,
et une composition qui change en direct sous l'utilisateur
n'est jamais un bon candidat pour un cache À L'ÉCHELLE DU CONTINENT. Le
cache par zone qui l'a remplacé (`zone_geom_cache.py`) n'a plus ce problème
-- chaque zone/pièce s'invalide sur sa PROPRE fraîcheur (mtime/taille/
rot/flip), jamais sur celle d'une autre -- donc actif dans les deux modes.

### `cache_dir()` séparé de `config_dir()`

`config_dir.py` expose maintenant `cache_dir()` en plus de `config_dir()` --
mêmes conventions par OS, mais pointant vers le vrai dossier de cache
(`~/.cache/ryzom_forgery` sur Linux, etc.) plutôt que dans le dossier de
config, un dossier de cache dans le dossier de config n'ayant pas sa place.
`zone_cache.py`/`zone_geom_cache.py` (données
purement jetables/régénérables) migrent vers `cache_dir()` ; `config_dir()`
reste réservé aux vraies préférences utilisateur.

## Chargement paresseux par région (`landscape_editor__region_management.md`)

Charger un continent entier d'un coup (étape 6) reste rapide une fois en
cache, mais un premier chargement (ou un continent jamais visité) reste
lourd pour rien si l'utilisateur ne veut regarder qu'une petite portion. Ce chantier
introduit un chargement par RÉGION (au sens `world.lua`/`world.json` : la
hiérarchie continent -> région -> lieu, pas les "régions" de `region_loader.py`
qui désignent un rayon géographique quelconque autour de la caméra, concept
indépendant) : à la sélection d'un continent, plus aucune zone n'est chargée
en géométrie réelle automatiquement -- chaque zone/cellule du continent
s'affiche comme un simple carré violet plein (footprint 160×160,
`zone_geometry.build_zone_placeholders_geom()`), et un panneau liste les
régions du continent sous forme de cases à cocher : cocher une région charge
la géométrie réelle de ses zones (remplaçant leur carré violet) ; décocher
décharge et remet le carré violet.

**Source de la hiérarchie -- une par mode, jamais l'une à la place de
l'autre** (`ryzom_forgery/region_hierarchy.py`) :
- Visualisation : `world.lua`, embarqué dans `gamedev.bnp`
  (`live_data_path`), lu via `pynel.ryzom_bnp.BnpReader` +
  `pynel.region_export.parse_world_lua`.
- Édition : `ryzom-data/leveldesign/world/world.json` directement
  (`json.loads`) -- même structure `{nom: [visible, points, enfants]}` que
  `world.lua`, juste sérialisée en JSON plutôt qu'en Lua.
- Résolution de la clé continent : `f"continent_{pipeline_continent_name}"`
  (ex. `"nexus"` -> `"continent_nexus"`), sauf `newbieland` dont la clé est
  le nom nu -- essayer le préfixe puis, si absent, le nom nu tel quel.
- Un continent sans entrée, ou dont l'entrée n'a aucun enfant de niveau
  région (`continent_indoors`, childless), retombe sur le comportement
  d'avant ce chantier : chargement complet immédiat, pas de carrés violets
  ni de panneau.
- Les entrées `pvp_zone_*` (ex. `pvp_zone_ichor`, `pvp_zone_nexus`) sont
  filtrées : au même niveau hiérarchique qu'une vraie région dans
  `world.lua`/`world.json`, ce sont des zones PvP, pas des régions
  géographiques (confirmé sur tous les continents réels : chaque vraie
  région est systématiquement `region_*`, sans exception).

**Assignation zone -> région** (`region_hierarchy.assign_zones_to_regions()`) :
test point-dans-polygone (ray-casting) du centre du footprint 160×160 de
chaque zone contre le polygone de chaque région (déjà en coordonnées monde).
Une zone hors de tout polygone de région reste en permanence carré violet,
jamais assignable à aucune case à cocher. Une seule région sur le continent
-> cochée automatiquement par défaut ; plusieurs -> rien coché par défaut.

**Mode Édition -- le repli `.land`+brique est LUI AUSSI filtré par région**
(pas seulement les zones à export réel) : chaque cellule `.land` utilisée
(export réel ou non) est assignée à une région via le centre de sa cellule
(`self._land_cell_region_map`, même test point-dans-polygone). Sans ce
filtrage, le repli existant (`_load_land_fallback_pieces()`) chargeait une
vraie brique -- coût identique à une vraie zone -- pour TOUTE cellule
absente de `self._loaded_refs`, quelle que soit la région cochée, annulant
tout le bénéfice du chargement paresseux (bug trouvé en testant sur le
continent `bagne` sans export réel : les 53 cellules se
rechargeaient à chaque bascule). `_apply_render_mode()` calcule
`allowed_land_cells` (cellules des régions cochées) et le passe à
`_load_land_fallback_pieces(..., allowed_cells=...)` -- une cellule hors de
ce set reste carré violet au lieu de charger sa brique.
`_rebuild_region_placeholders()` construit donc les carrés violets
différemment par mode : en Visualisation, une zone absente de
`self._loaded_refs` ; en Édition, une cellule `.land` dont la région n'est
PAS cochée (le repli s'occupe déjà des cellules d'une région cochée sans
export réel -- pas de double géométrie au même endroit).

**Caméra figée sur le continent entier** : la caméra ne se
recentre/zoome plus QUE lors de la sélection du continent
(`_select_continent()`), jamais lors d'une bascule de région -- nouveau
paramètre `frame_camera=False` passé par le consommateur de chargement de
continent à `_set_loaded_zones()` (`True` par défaut, préserve le
comportement pour la sélection d'une zone unique via l'Explorer).
`OrbitCamera.frame_bounds()` (`camera.py`, nouvelle méthode) remplace
l'ancien calcul `distance = max(largeur, hauteur) * 0.75` (sous-cadrait) par
un calcul basé sur le vrai FOV de la lentille (`distance * tan(fov/2)` =
demi-étendue monde visible à cette distance, même relation que `_pan()`), en
projetant la bbox sur les axes écran RÉELS de la caméra
(`getQuat(render).getRight()/.getUp()`, jamais supposés alignés sur X/Y
monde -- `up_hint` n'est jamais réinitialisé à un axe fixe, voir
`step_to_face()`), et en tenant compte de `self.app.explorer_width`/
`.panel_width` : les panneaux Explorer/outil se posent PAR-DESSUS la vue 3D
sans rétrécir le `DisplayRegion`/lens (qui reste calé sur toute la fenêtre),
donc masquent visuellement les bords sans que le FOV le sache -- cause
réelle du rognage constaté sur `nexus`/`tryker` (le plus large des deux
panneaux fait foi, le frustum étant centré sur toute la fenêtre). Une marge
d'une case (`ZONE_CELL_SIZE`) est ajoutée de chaque côté pour laisser de
l'air.

**Chargement `.ig` manuel** (c'est le bouton de la vue 3D qui déclenche le
chargement, pas un bouton séparé du panneau) :
l'icône arbre existante dans la barre d'outils de la vue 3D
(`_toggle_ig_visibility()`) déclenche désormais le chargement en plus de la
visibilité -- l'activer (re)charge `.ig` pour `self._loaded_refs` (le jeu de
zones actuellement chargé, donc PAR RÉGION cochée, pas tout le continent),
remplaçant l'affichage précédent ; la désactiver masque seulement (pas de
déchargement). Aucun rechargement automatique si les zones changent
entre-temps -- seul un nouveau clic recharge, et il recharge TOUT
`self._loaded_refs` depuis zéro (pas de diff incrémental avec ce qui était
déjà affiché). Raison du passage en manuel : le coût CPU du parsing `.ig`/
`.shape` (Python pur, lié au GIL même en tâche de fond) faisait geler
l'appli à chaque bascule de région automatique -- inacceptable une fois
répété à chaque coche/décoche, alors qu'il n'était payé qu'une fois par
continent avant ce chantier.

**Eau en `two_sided`** : les meshes d'eau (`resolved.kind` `"water_polygon"`/
`"water_point"`, `ig_geometry.py`) sont maintenant en `set_two_sided(True)`
comme le terrain -- un plan d'eau est une unique face plate, invisible de
l'autre côté par défaut (backface culling), donc invisible dès que la
caméra passait sous son niveau ou que son winding ne faisait pas face à la
vue initiale.

**`out_ig_dir` du CSV pipeline -- bug de données trouvé en testant** :
`continent_pipeline_reference.csv` pointait `out_ig_dir` vers
`ligo_ig_land` (le brouillon LIGO brut, édition en cours, potentiellement
très incomplet -- un seul instance pour `46_BZ`/nexus contre 56 dans la
vraie donnée) au lieu de `zone_lighted_ig_land` (le vrai résultat final de
`zone_ig_lighter`, cohérent avec la convention déjà en place pour le
terrain `zone_lighted`). Corrigé dans le CSV (`ryzom-data`) pour les 25
continents -- ce n'est pas un bug de code Forgery, juste une mauvaise valeur
dans le fichier de référence.

**⚠️ Important -- tenir `world.json` synchronisé avec les `.primitive`** :
Atyscape ne charge/n'édite aucun `region_***.primitive` à ce stade (portée
explicitement hors scope de ce chantier) -- il lit uniquement `world.lua`
(Visualisation) et `world.json` (Édition), deux fichiers déjà générés en
amont par `pynel.region_export` à partir des `region_*.primitive` de
`ryzom-private-data`. **Toute future modification ou ajout d'un
`region_***.primitive` devra impérativement régénérer `world.json`**
(`python -m pynel.region_export --format json`, voir
`project-todos/pynel/region_places_export.md`) -- sans quoi ce mécanisme de
régions affichera une hiérarchie/des polygones désynchronisés de la réalité,
silencieusement (aucune erreur, juste des données obsolètes). Cette règle
s'applique dès maintenant, même si ce chantier ne touche jamais lui-même aux
`.primitive`.

## Chargement complet + texturé des `.ig`, par région (`landscape_editor__ig_full_load.md`)

Pour comment le vrai client charge chaque type de `.ig` (par convention de
nom pour le "land", par fiche Georges village/continent pour le "other")
et d'où chacun est généré, voir `pynel/nel/tools/pynel/docs/ig_client_loading.md`.

Remplace entièrement l'ancien chargement incrémental par zone visible
(géométrie `.ig` grise plate, un bouton unique "sapin" rechargeant tout
`self._loaded_refs` à chaque clic) par un chargement texturé (vraies
textures diffuses+alpha, `ig_geometry.build_textured_instance_template()`,
mêmes helpers déjà éprouvés que `shape_geometry.py`/
`object_editor_mixins/materials.py`) réparti en DEUX granularités :

- Les `.ig` propres à une zone (ex. `215_ED.ig`) suivent le découpage par
  région déjà en place pour la géométrie de zone (`_load_region_ig()` /
  `_run_load_region_ig()`) : cocher une région charge automatiquement ses
  `.ig` de zone depuis un `.bam` par `(continent, région, mode)`
  (`ig_full_geom_cache.py`, même schéma que `zone_geom_cache.py`), construit
  au premier chargement puis relu ensuite ; décocher détache sans décharger
  le cache disque.
- Tout le reste (village/eau/autres, ex. `tr_villagea.ig`/`tr_water.ig` --
  pas rattachables de façon fiable à une région précise) va dans un seul
  `.bam` continent entier, via le bouton "Load remaining .ig instances" du
  panneau de droite (`_load_ig_rest()`/`_run_load_ig_rest()`).
- Les `.ig` de ciel/canopée (`SkyIg`/`Spring|Summer|Autumn|WinterCanopyIG`,
  ex. `canope_tryker.ig`) ne sont JAMAIS chargés, dans aucun des deux cas
  (`ig_full_load.sky_ig_names()`, lu depuis le `.continent` brut en Édition,
  `continent.packed_sheets` en Visualisation).

`ig_full_load.py` énumère les refs réelles d'un continent : Visualisation
depuis `<continent>_ig.bnp`/`.bnpe` à la racine de `live_data_path` ;
Édition depuis `out_ig_dir` (par zone) + `ig_other_lighted_dir` (village/
eau/ciel/autres) de `continent_pipeline_reference.build_land_export_config()`
réunis. Classification zone/ciel/reste : zone si le nom correspond au motif
de grille (`^\d+_[A-Za-z]{2}$`), ciel si dans la liste des atomes ci-dessus,
reste sinon. Deux icônes séparées dans la barre flottante du viewport
pilotent l'affichage sans jamais déclencher elles-mêmes de chargement : sapin
(`self._ig_region_root`, `.ig` de zone par région) et maison
(`self._ig_rest_root`, bundle "reste") -- changer de continent/mode détache
tout sans résidu, affichage désactivé par défaut.

**Arborescence "IG Zones"/"IG Others" remplace l'Explorer dans le panneau de
gauche** (`landscape_editor__ig_inspector_tree.md`, même principe de dossiers
virtuels que dans Patina) : `ForgeryApp.draw_left_panel_content()` (nouveau point
d'extension, `app.py` -- toute autre app garde l'Explorer réel via
l'implémentation par défaut) est surchargé par `landscape_editor.py` pour
dessiner deux dossiers virtuels à cocher construits en introspectant le
scene graph déjà chargé (`_build_ig_tree_entries()`, sur
`container_np.get_children()`) : "IG Zones" (groupé par région) et
"IG Others" (plat), chacun listant les `.ig` réellement chargés puis, sous
chacun, ses `.shape` réels. Cascade de cases à cocher exactement comme
Patina : décocher un `.ig` décoche tous ses `.shape` (et masque le
NodePath du `.ig` entier) ; recocher un `.shape` réactive automatiquement
son `.ig` parent. Cet outil, construit pour l'inspection, s'est révélé
déterminant pour isoler visuellement le bug d'eau invisible ci-dessous (un
seul `.shape` désactivable à la fois, sans toucher au reste de l'`.ig`).

**Corrections trouvées en testant réellement sur Tryker** :

- Rendu tantôt tout noir : matériaux réels avec diffuse/spéculaire calculés
  sans configuration d'éclairage de scène propre -- fixé en forçant
  l'ambiant au maximum et en ignorant diffuse/spéculaire/émissif
  (`ig_geometry._apply_ig_material()`).
- Cache de géométrie `water_polygon` empoisonné : contrairement à
  `water_point`, un `water_polygon` n'est JAMAIS mis en cache/partagé --
  l'ancienne version partageait une clé de cache `(kind, None)` entre TOUTES
  les instances `water_polygon`, donc un seul polygone dégénéré quelque part
  rendait `None` pour tous les suivants, pour toujours.
- Eau semi-transparente (alpha 0.5, `TransparencyAttrib.M_alpha`) plutôt
  qu'opaque ; z-fighting eau/eau réglé par `set_depth_write(False)` ;
  z-fighting eau/terrain réglé par un vrai décalage Z monde
  (`_WATER_Z_LIFT`, `set_depth_offset()` s'étant révélé sans effet mesurable).
- Cache du bundle "reste" jamais relu : `_load_ig_rest()` reconstruisait à
  chaque clic au lieu de vérifier `ig_full_geom_cache.read_ig_bundle()`
  d'abord, contrairement à `_load_region_ig()` qui le faisait déjà.
- **Bug réel dans pynel** : deux instances d'eau bien réelles
  (`Water14`/`Water10`, `tr_water.ig`) restaient invisibles sur leurs zones
  malgré une résolution sans erreur -- la cause réelle était
  `pynel.ryzom_packed_sheets.world_pos_to_zone_name()` (`row = floor(-y/160)`
  au lieu de `row = -floor(y/160)`), qui décalait quasi toute position d'une
  zone et faussait les vérifications manuelles pendant le débogage. Corrigé
  contre la vraie formule C++ (`getZoneNameFromXY`/`getPosFromZoneName`).
- **Collision de nom `.shape` via les Search Paths partagées** : le vrai
  bug d'affichage restant après le fix pynel -- `water14.shape` existait
  sous DEUX chemins atteints par les Search Paths (le bon export pipeline
  Tryker ET un `.shape` générique sans rapport, même nom, ailleurs sur le
  disque de l'utilisateur), le second gagnant silencieusement selon l'ordre
  de scan. Résolu par un mécanisme générique et réutilisable plutôt qu'un
  correctif ad hoc (voir section suivante).
- `search_paths_dialog.find_texture()` renommée `find_file()`
  (`_texture_entries`/`texture_entries` renommés `_file_entries`/
  `file_entries` partout, y compris le cache disque JSON) : le nom laissait
  croire à tort que seules des textures y transitaient, alors que `.shape`/
  `.skel`/`.anim` y passent aussi.
- `find_file(name, priority_paths=None)` : `priority_paths`, si fourni, est
  essayé EN PREMIER (petit index dédié, mis en cache par tuple de chemins),
  et ne retombe sur l'index partagé habituel que s'il n'y trouve rien.
  Atyscape (`_shape_priority_paths()`) force ainsi `pipeline/export/
  continents/<continent>/` PUIS `pipeline/export/ecosystems/<ecosystem>/`
  (Édition uniquement -- Visualisation n'a pas cette collision, les `.bnp`
  de `live_data_path` étant déjà scopés par continent) : les deux racines
  sont nécessaires, les props de bâtiment partagés (ex.
  `tr_agora_village_a.shape`, `tr_villagea.ig`) vivant sous l'arbre
  écosystème et non sous l'arbre continent (trouvé après un premier fix
  incomplet qui ne couvrait que le continent, faisant disparaître tous les
  bâtiments de village). Résout la collision une fois pour toutes sans
  toucher aux Search Paths partagées ni à la config personnelle de
  l'utilisateur (aucune exclusion `graphics`/`workspace` en dur, aucun tri
  de priorité côté recherche partagée).

**Carrés violets "région non chargée" -- transparence + priorité de rendu la
plus basse** : `zone_geometry._ZONE_PLACEHOLDER_COLOR`
passe à 30% transparent (alpha 0.7) et `_rebuild_region_placeholders()`
place le NodePath dans le bin Panda3D `"background"` (priorité de rendu la
plus basse, dessiné avant tout le reste) avec `set_depth_write(False)` --
toute géométrie réelle chargée ensuite au même endroit (région cochée,
bundle `.ig`) se superpose sans jamais être masquée par un placeholder
résiduel.

## Nom + bornes de la zone sélectionnée (`landscape_editor__selected_zone_info.md`)

Le panneau de droite affiche, sous la sélection de zone existante (pivot de
rotation), le nom de la zone actuellement sélectionnée et ses bornes monde
`(min_x, min_y)`-`(max_x, max_y)` (`self._selected_zone_bounds`, calculé dans
`_select_zone()`/remis à `None` dans `_clear_zone_selection()`) -- utile pour
vérifier visuellement qu'une géométrie (eau, bâtiment) tombe bien dans la
zone attendue, sans calcul manuel.

## Suppression des modes de rendu + nettoyage de l'interface

(`landscape_editor__render_modes_removal.md`) Les 3 boutons de rendu
[POLY/LAND]/[WELD]/[LIGHT] (voir plus haut, sections marquées historiques)
ont été retirés entièrement, pour faire comme en Visualisation, avec un seul
mode. Résolution désormais
**toujours** `.zonel > .zonew > .zone` (la priorité déjà utilisée par
`region_loader.py`), identique en Visualisation et Édition, sans notion de
"fallback gris" (`_resolve_zone_for_mode()`/le paramètre `gray` de
`_set_loaded_zones()` ont disparu). La colorisation par étage de build
(`zone_build_stage()`, voir plus haut) reste inchangée -- elle était déjà
indépendante du mode. Le bouton "Generate N missing .zonew" a disparu avec
le mode `[WELD]` -- le bouton "Build" (pipeline complet) suffit à tout
générer.

### Panneau gauche : continent + régions (remplace le panneau droit historique)

`draw_left_panel_content()` (qui affichait déjà l'arbre IG Zones/Others,
voir plus haut) affiche maintenant AU-DESSUS le sélecteur de continent et la
liste des régions à cocher -- déplacés depuis l'onglet Landscape du panneau
droit. Chaque ligne (continent ou région) a deux icônes : centrer la caméra
dessus (`ICON_FA_CROSSHAIRS`, `self._region_bounds` -- bornes des polygones
de région, calculées une fois au scan du continent, jamais recalculées
depuis les zones réellement chargées) et recharger son cache
(`ICON_FA_ROTATE`, voir "Rechargement forcé du cache" plus bas). Le texte
"Bounds: X[...] Y[...]"/"Regions (check to load...)" a disparu -- les
icônes remplacent l'info textuelle. Les libellés de région affichent le nom
sans le préfixe `region_` (`region_hierarchy._regions_from_world()` filtre
aussi désormais en liste blanche `"region_"` plutôt qu'en liste noire
`"pvp_zone_"` -- corrige des entrées non-région qui apparaissaient à tort
sur certains continents).

### Panneau droit : sections repensées

`_draw_landscape_tab()` ne garde que le strict nécessaire :

- **"Selected Zone"** (repliable, désactivée si rien n'est sélectionné) :
  regroupe le nom+bornes de la zone sélectionnée et l'éditeur de composition
  `.land` (`_draw_land_composition_editor()`, plus gaté sur un mode de rendu
  -- juste "Édition + continent sélectionné"). Icône colorée dans le titre,
  nom de zone en orange, bornes en vert (même convention que Patina --
  `pastel_color_for()`, `icon_colors.py`, déjà utilisé par les icônes du
  reste de la suite). Le texte "-- cell (x, y)" de l'éditeur de composition a
  été retiré (redondant avec la barre de statut curseur). Chaque statut
  ✅/❌ `.zone`/`.zonew`/`.zonel` de la cellule a maintenant une icône
  corbeille à côté quand le fichier existe (loose, jamais dans un `.bnp`) --
  clic : supprime le fichier réel puis retombe sur le meilleur étage encore
  présent (`best_ref_for_extensions()`), jamais un simple retour à la brique
  brute si un étage intermédiaire existe encore -- décocher supprime le
  fichier, l'alternative étant un bouton delete dédié si trop compliqué.
- **"Continent status"** (toujours affichée en Édition) : 4 compteurs
  (Zones/Raw/Welded/Lighted, calculés depuis `self._loaded_extensions`) en
  vert. Le bouton [Build] ne s'affiche plus que si quelque chose manque
  (`lighted < total`) ou qu'un build est en cours -- remplace l'ancien
  bouton toujours visible en bas de panneau.
- **"Stats"** (repliable, replié par défaut) : un vrai tableau ImGui
  (bordures, lignes alternées, colonnes numériques alignées) -- région par
  région (zones/patches/instances `.ig`), plus IG Others et un total, à la
  place d'une première version en texte brut jugée peu lisible.
- Le bloc "Instances (.ig)" (bouton manuel "Load remaining .ig instances" +
  barres de progression) a disparu -- `_load_ig_rest()` reste appelé
  automatiquement au chargement d'un continent (déjà le cas depuis l'étape
  12 du chantier principal), seul le déclenchement manuel a disparu.

### Chargement incrémental par région (perf)

**Bug trouvé via `(IA_AGENT_DEBUG)` chronométré, pas deviné** (le
comportement observé était tordu : charger une seule région ne montrait
aucune progression, en charger une deuxième en montrait) : `_toggle_region()` recalculait
`self._loaded_refs` comme l'union de TOUTES les régions cochées puis
rechargeait tout depuis zéro à chaque coche (`_apply_render_mode()`) --
cocher une Nᵉ région retraitait aussi les N-1 précédentes, un coût
quadratique sur une séquence de coches. `_toggle_region()` est maintenant
incrémental : `_load_region_incremental()` ne charge que la région qu'on
vient de cocher (ses vraies zones + son propre repli `.land`, restreint à
ses cellules via `allowed_cells`) et la fusionne dans ce qui est déjà
affiché (`_rebuild_zone_node()` par zone, jamais un rechargement complet) ;
`_unload_region_incremental()` retire juste les zones de la région qu'on
décoche, sans rien recharger. Une région cochée pendant qu'une autre charge
déjà est mise en file (`self._region_incremental_queue`), pas perdue.

**Cache du `.land` parsé** (`EditModeMixin._load_land_cached()`) : le
fichier `.land` entier (jusqu'à 1000+ cellules) était reparsé
(`load_land()`) à chaque appel de `_load_land_fallback_pieces()`, donc à
chaque coche de région -- jamais mis en cache. Un cache mémoire simple
(`self._land_fallback_cache`, invalidé par mtime) élimine ce reparsing
redondant.

**Saut du chargement des positions quand le `.bam` va de toute façon hit**
(`_run_load_refs()`) : le chargement des positions tessellées
(`zone_cache.py`, ~3ms/zone -- coût de désérialisation `pickle` de milliers
de `float` Python individuels, pas de l'I/O disque) tournait
systématiquement AVANT même de savoir si le cache géométrie `.bam`
(`zone_geom_cache.py`) allait de toute façon être réutilisé -- ce qui est le
cas la quasi-totalité du temps. `ZoneGeomManifest` porte maintenant aussi
`bb_center`/`bb_half_size` (format bumpé à la version 2) : un simple
manifeste (petit fichier pickle, jamais le `.bam` lui-même) suffit à savoir
si le cache va hit, et si oui les bornes s'y trouvent déjà -- le chargement
des positions est alors sauté entièrement (`ZoneCacheData(patches=(), ...)`
comme "coquille"). `_rebuild_zone_node()` a dû apprendre à vérifier lui
aussi le cache `.bam` (il ne le faisait pas avant, d'où un premier bug :
zone invisible la 2ᵉ fois qu'une région skip-optimisée était rechargée) et,
en dernier recours si la coquille arrive jusqu'à lui sans `.bam` valide, à
recharger les vraies positions à la demande plutôt que de construire un
maillage vide.

**Boutons "recharger le cache"** (icône rotation, panneau gauche, par
région ou pour tout le continent) : invalident réellement les DEUX caches
(`force=True` propagé jusqu'à `load_zone_cache_data()`, qui saute alors sa
propre lecture de `zone_cache.py`, ET jusqu'à `_rebuild_zone_node()`, qui
saute alors sa vérification `.bam` -- même mécanisme de cache que
d'habitude, jamais un second cache à côté). Le bouton continent traite
CHAQUE région l'une après l'autre (`self._continent_cache_reload_queue`) --
une région cochée voit son affichage rafraîchi en direct, une région non
cochée voit juste son fichier de cache disque régénéré en silence.

### Boussole nord + reset caméra

Petite fenêtre flottante en bas à droite de la vue 3D avec une flèche
dessinée à la main (`imgui.get_window_draw_list()` -- ImGui n'a pas de
rotation de glyphe/texte native) pointant le vrai nord, recalculée chaque
frame depuis la transformation réelle de la caméra
(`self.camera.get_mat(self.render)`, lignes 0/2 = right/up monde, pas une
approximation à partir du seul heading -- correcte même en 3D inclinée).
Clic : réinitialise le cap à 0° via `OrbitCamera.animate_to_orientation()`
-- **pas** une simple assignation `self.orbit_camera.heading = 0.0`, qui ne
bouge rien visuellement (`OrbitCamera._update()` ne recalcule la position
réelle que sur un drag souris ou une animation en cours, jamais sur une
simple modification de champ -- bug trouvé en traçant `camera.py` après
un signalement du bouton reset restant sans effet visuel).

## Mode "Low Poly" pour le terrain (`landscape_editor__low_poly_mode.md`)

Bouton icône (`ICON_FA_GAUGE_SIMPLE`, barre flottante de la vue 3D) pour les
PC faibles -- divise `order_s`/`order_t` par 4 avant tessellation
(`zone_geometry.compute_zone_patch_positions(zone, low_poly=True)`, même
plancher `_MIN_GRID_SEGMENTS` qu'en qualité normale) : ~16x moins de faces
par patch (une grille 2D, diviser les deux axes par 4 divise le nombre de
faces par ~16). Réglage révisé de /2 à /4 après un premier essai.

**Même cache que d'habitude, jamais un second** -- `zone_cache.py` gagne un paramètre `variant` (vide en
qualité normale, `"_low"` en low poly, inséré dans le nom de fichier) et
`zone_geom_cache.py` réutilise sa propre notion de `mode` (déjà stabilisée à
`"zone"` par la suppression des modes de rendu ci-dessus) avec une seconde
valeur `"zone_low"` -- toujours le même module/dossier de cache, juste une
clé différente selon la qualité active. Persisté (`Settings.
landscape_low_poly`, même mécanisme que `landscape_editor_mode`) ; basculer
recharge tout ce qui est actuellement affiché (`_apply_render_mode()`, une
bascule de qualité touche l'ensemble de l'affichage d'un coup, contrairement
à une coche de région). Hors scope pour l'instant : le LOD des shapes/`.ig`
(bâtiments/props), un chantier séparé à traiter plus tard.
