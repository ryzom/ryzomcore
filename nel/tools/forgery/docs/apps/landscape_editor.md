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
partagé avec Patina (`object_editor.py`) -- configurable dans son onglet
Settings (Paths). `landscape_editor.py` n'a pas encore son propre onglet
Settings (scaffolding minimal, étape 1) ; message d'erreur explicite dans le
panel si ce chemin n'est pas configuré ou invalide. L'Explorer démarre
directement dans ce dossier si configuré (sinon un `search_path` classique,
sinon le home).

## Étape 3 -- rendu low-poly + vue 2D

Sélectionner un `.zone`/`.zonew`/`.zonel` dans l'Explorer (y compris à
l'intérieur d'un `.bnp`, ex. `nexus_zones.bnp`) parse la zone via
`pynel.ryzom_zone.parse_zone()` et construit un maillage via
`ryzom_forgery/zone_geometry.py` (`build_zone_low_poly_geom()`) : un quad par
patch, ses 4 coins bruts uniquement (`patch.vertices`, **sans** évaluation
Bézier -- vient à l'étape 4), dans l'ordre `[V0, V3, V2, V1]` (pas l'ordre
brut sur disque) pour suivre le vrai contour du patch d'après
`CBezierPatch::eval()` (`bezier_patch.cpp:98-133`).

Coloration par élévation (dégradé vert foncé -> jaune-vert clair, interpolé
par sommet) plutôt que par `CTileColor` -- anticipe l'étape 5 ("Rendu par
élévation") pour un rendu plus lisible dès l'étape 3, décision Nuno
2026-09-07.

**Limite connue** (attendue, corrigée à l'étape 4) : sur un relief très
courbé (dôme, colline), le quad plat coupe à travers la vraie surface
bombée -- visible comme des creux/"trous" en diamant à l'écran. Pas une
perte de géométrie réelle, juste la conséquence de n'utiliser que les 4
coins sans les tangentes/points intérieurs Bézier.

**Explorer** : filtre par défaut `"*"` (pas `"*.land"`) -- un `.bnp` qui ne
contient aucun fichier correspondant au filtre actif est entièrement caché
par `explorer.py` (`_draw_dir_contents()`), et les vrais `*_zones.bnp` d'une
installation Ryzom Live ne contiennent jamais de `.land` (trouvé le
2026-09-07 : plus aucun `.bnp` n'apparaissait avec `"*.land"` par défaut).

Bouton "Top view (2D)" dans le panel : sert de "vue 2D projetée" en
réutilisant `OrbitCamera.snap_to_axis("+z")` sur le même maillage, pas de
rendu 2D séparé.
