# Search Paths (moteur de scan + UI de configuration)

**Fichiers :** `nel/tools/forgery/ryzom_forgery/search_paths.py` (~224
lignes) et `nel/tools/forgery/ryzom_forgery/search_paths_dialog.py` (~809
lignes)

## Rôle

Ce système est, selon le propre commentaire du code, "the *only* place
Forgery resolves a named file from" (`search_paths.py`) : c'est le
mécanisme générique par lequel Patina retrouve une texture référencée par un
matériau, un `.skel` compatible avec un shape, un `.anim` compatible avec un
squelette, ou le fichier `panoply_files.txt` de Ryzom — à partir d'une liste
de dossiers configurés par l'utilisateur (récursifs ou non, conscients des
archives `.bnp`).

Le système se divise en deux couches nettes :

- **`search_paths.py`** est la logique de scan pure et sans état : parcourir
 des dossiers (+ archives `.bnp` qu'ils contiennent), produire des
 `FoundEntry`, et deux caches disque JSON (table des `.bnp`, résultats de
 parsing `.skel`/`.anim`) qui rendent un rescan quasi instantané si rien n'a
 changé.
- **`search_paths_dialog.py`** est `SearchPathsDialog` : la classe qui décide
 *quand* rescanner (au démarrage, sur bouton Reload, sur changement de
 workspace, sur événement filesystem), pilote le scan en tranches de temps
 sur le thread principal (pas de thread séparé), et dessine la section
 "Paths" des Settings de Patina.

L'ordre des dossiers configurés compte : le premier dossier qui contient un
fichier donné gagne (`search_paths_dialog.py`). Le dossier du
workspace actif est toujours injecté en priorité la plus haute, avant les
dossiers configurés par l'utilisateur, sans jamais être persisté dans
`settings.search_paths` (`search_paths_dialog.py,337-358`).

## API principale — `search_paths.py`

- `FoundEntry` (`search_paths.py`, `@dataclass(slots=True)`) : un
 fichier trouvé, sur disque (`fs_path`) ou dans un `.bnp` (`bnp_path`).
 `read_bytes` lit le contenu ; `cache_key` donne une identité stable
 pour le cache de scan ; `cache_stat` donne `(mtime, size)` de ce qui
 possède réellement les octets (l'archive elle-même pour une entrée `.bnp`,
 car repacker une archive invalide toutes ses entrées d'un coup — pas de
 signal moins cher disponible par entrée).
- `TEXTURE_FALLBACK_EXTENSIONS = (".tga", ".png", ".dds")`
 (`search_paths.py`) : une référence de texture dans un `.shape` ne porte
 aucune extension garantie ; ces extensions sont essayées dans cet ordre
 après un échec du nom exact.
- `_iter_bnp_entries` / `_iter_dir_entries` (`search_paths.py`) :
 parcours bas niveau. `_iter_dir_entries` est **itératif** (pile explicite,
 pas de générateur récursif via `yield from` niveau par niveau) pour éviter
 le coût cumulatif de `yield from` à chaque profondeur ; utilise
 `os.scandir` plutôt que `Path.iterdir` pour réutiliser le type de
 fichier déjà renvoyé par `readdir` et éviter un `stat` par enfant.
- `iter_all_entries(dirs, bnp_table_cache=None, exclude=None)` (`search_paths.py`) :
 génère tous les `FoundEntry` de tous les dossiers configurés, dans l'ordre
 de priorité. `exclude` (forgery-workspace-projects chantier, 2026-09) est
 un simple callable optionnel `FoundEntry -> bool` (jamais un type
 workspace-spécifique importé ici, pour garder ce module générique) --
 seul `_reload_workspace_only` (ci-dessous) en passe un réellement, jamais
 les scans de dossiers externes.
- `build_texture_index(dirs)` / `find_texture(entries_by_lower_name, name)`
 (`search_paths.py`) : version simple, non mise en cache, pour des
 appelants ponctuels (ex. le CLI `shape_exporter.py`, un process par
 export) ; `find_texture` implémente la règle de correspondance partagée
 (nom exact insensible à la casse, puis extensions de repli).
- `load_scan_cache`/`save_scan_cache` et
 `load_bnp_table_cache`/`save_bnp_table_cache`
 (`search_paths.py`) : caches JSON persistés dans `cache_dir`
 (voir `cache_dir.py`) — un cache corrompu ou absent est traité comme un
 point de départ vide, jamais une erreur.

## API principale — `search_paths_dialog.py`

- `SearchPathsDialog` (`search_paths_dialog.py-...`) : classe centrale.
 État séparé en deux moitiés — `_external_result`/`_workspace_result`
 (chacune un `_ScanResult`, `search_paths_dialog.py`) — fusionnées
 dans les dicts publics via `_merge_and_publish`
 (`search_paths_dialog.py`), le workspace l'emportant sur les
 dossiers externes en cas de collision de nom.
- `draw` (`search_paths_dialog.py`) : à appeler une fois par frame
 ImGui — fait avancer les deux scans en cours (externe et workspace) d'une
 tranche de temps bornée (`_SCAN_FRAME_TIME_BUDGET = 0.002`s,
 `search_paths_dialog.py`).
- `reload` (`search_paths_dialog.py`) : (re)scanne uniquement les
 dossiers externes configurés — no-op si un scan externe est déjà en cours.
- `_reload_workspace_only` (`search_paths_dialog.py`) : (re)scanne
 uniquement le dossier du workspace actif ; contrairement à `reload`, un
 appel pendant qu'un scan est déjà en cours **remplace** le générateur en
 cours plutôt que d'être ignoré. Construit `exclude` via
 `_make_workspace_exclude(workspace_root, exclusion_rules)` (charge
 `Settings.exclusion_rules` à chaque appel) et le passe à
 `_scan_dirs_incremental` -- c'est le seul endroit qui le fait, `reload()`
 (externe) n'a pas de notion d'exclusion.
- `_make_workspace_exclude(workspace_root, exclusion_rules)`
 (`search_paths_dialog.py`, fonction module, 2026-09) : construit le
 prédicat ci-dessus -- calcule le dossier relatif de chaque `FoundEntry`
 (l'archive `.bnp` elle-même si l'entrée vit dedans, pas le nom interne)
 et délègue à `virtual_categories.is_path_excluded()` (voir
 `docs/virtual_categories.md`) pour la décision dossier/motif fichier.
- `_scan_dirs_incremental(dirs, bnp_table_cache, cache, exclude=None)`
 (`search_paths_dialog.py`) : le cœur du scan, un **générateur** qui
 `yield` après chaque entrée traitée — permet à
 `_advance_external_scan`/`_advance_workspace_scan` de le piloter en
 tranches sur le thread principal. Construit l'index de textures, la liste
 de `.skel`/`.anim` compatibles (avec cache de parsing par `(mtime, size)`,
 y compris un cache des **échecs** de parsing pour ne jamais retenter un
 fichier connu comme non parsable), et détecte/parse `panoply_files.txt`.
 `exclude` est simplement transmis tel quel à `iter_all_entries`.
- `ensure_scanned` (`search_paths_dialog.py`) : à appeler une seule
 fois au premier besoin réel (voir `object_editor.py`) — charge un cache
 disque de l'index externe s'il existe (affichage immédiat, potentiellement
 légèrement périmé) puis lance quand même un vrai `reload` en tâche de
 fond.
- `set_workspace_dir(path)` (`search_paths_dialog.py`) : change le
 dossier du workspace actif, relance un scan workspace-only, et
 (re)démarre un watcher filesystem (`_start_workspace_watch`,
 `search_paths_dialog.py`, via `watchdog`, debounce de 0.5s dans
 `_DebouncedReloadHandler`, `search_paths_dialog.py`).
- `compatible_for(bones_name)` / `compatible_animations_for(skeleton)`
 (`search_paths_dialog.py,704-713`) : test de compatibilité
 squelette/animation par sur-ensemble/sous-ensemble de noms d'os — même
 règle que `CSkeletonModel::remapSkinBones` côté moteur.
- `skeleton_for(name)` / `animation_for(name)`
 (`search_paths_dialog.py,715-725`) : parsing complet à la demande
 d'un seul `.skel`/`.anim` — seuls les noms d'os sont gardés en mémoire pour
 tous les fichiers scannés, pas les données complètes.
- `find_texture(name)` (`search_paths_dialog.py`) : résout un nom de
 texture contre l'index scanné.
- `panoply_variants_for(base_texture_name)` (`search_paths_dialog.py`) :
 variantes panoply pour une texture de base donnée.
- `_load_ryzom_data_panoply_variants` (`search_paths_dialog.py`, 2026-08-29) : lit
 `panoply_files.txt` directement depuis `<ryzom-data>/final_bnps/characters_maps_hr/`
 (via `pynel.repository_paths`, cache par mtime), **prioritaire** dans
 `_merge_and_publish` sur celui trouvé dans un `characters_maps_hr.bnp` shippé le long
 des search paths génériques — le fichier de `ryzom-data` est la copie de travail,
 éditée à la main au fur et à mesure des ajouts, alors que celui du `.bnp` shippé ne
 reflète que le dernier vrai build `hls_bank_maker` et peut être en retard (cas vécu :
 les masques de `ryw_mark1_hof_caster01_pantabottes` invisibles dans Patina alors que
 les `.dds` existent déjà sous `ryzom-data/final_bnps/characters_maps_hr/mark_1/`, le
 `panoply_files.txt` du `.bnp` shippé n'ayant jamais été régénéré pour eux). `{}` si
 `ryzom-data` n'est pas configuré ou que le fichier n'existe pas — dans ce cas,
 `_merge_and_publish` retombe sur le comportement précédent (workspace puis search
 paths génériques).
- `draw_settings_content` (`search_paths_dialog.py`) : dessine la
 liste de dossiers (chemin tronqué avec tooltip, bascule récursif/non,
 boutons monter/descendre/supprimer, ajouter, recharger) — intégrée dans
 l'onglet Settings de Patina.
- `_truncate_path_to_width` (`search_paths_dialog.py`) : tronque un
 chemin par recherche binaire sur `imgui.calc_text_size` pour tenir dans une
 largeur pixel donnée, en gardant la fin du chemin (partie la plus
 significative) et un préfixe `...`.

## Utilisation

- `ryzom_forgery/apps/object_editor.py` instancie
 `self.search_paths_dialog = SearchPathsDialog`, câble
 `on_workspace_changed = self.explorer.refresh`, appelle
 `ensure_scanned` au premier affichage d'un shape, et
 `set_workspace_dir` au changement de workspace actif.
 Ensuite très largement utilisé pour résoudre les squelettes/animations
 compatibles et pour trouver les textures référencées par les matériaux
 (`find_texture`, utilisé à de nombreux points), ainsi que pour dessiner
 la section Paths des Settings et faire avancer le scan chaque frame
 (`draw`).
- `ryzom_forgery/shape_geometry.py` importe `FoundEntry` et
 `TEXTURE_FALLBACK_EXTENSIONS` de `search_paths.py` directement (résolution
 de texture pour l'aperçu 3D, indépendamment du dialog UI).
- `ryzom_forgery/apps/shape_exporter.py` importe le module
 `search_paths` complet — un outil CLI, un process par export, qui utilise
 probablement `build_texture_index`/`find_texture` en mode "one-shot"
 plutôt que `SearchPathsDialog` (pas de boucle ImGui à piloter en tâche de
 fond dans un CLI).

## Points notables / pièges

- **Pas de thread de fond pour le scan** : un scan réel construit de l'ordre
 de 10^5 objets Python — travail CPU pur qui, sur un vrai thread séparé,
 entrait en contention de GIL avec le thread de rendu et faisait chuter le
 framerate sous 30fps. La solution retenue est un générateur avancé par
 petites tranches (2ms/frame, `_SCAN_FRAME_TIME_BUDGET`) sur le thread
 principal — le scan met plus longtemps en temps réel mais ne bloque jamais
 l'UI (`search_paths_dialog.py`).
- **Sauvegarde des caches sur un thread séparé après coup** :
 `_save_external_scan_caches` (appelée depuis `_advance_external_scan`)
 tourne bien sur un thread dédié, car sérialiser ~10^5 entrées en JSON est
 un burst CPU d'environ 1 seconde — différent du scan lui-même (travail
 soutenu) donc jugé acceptable en thread de fond
 (`search_paths_dialog.py`).
- **Regroupement debouncé des événements filesystem** : une opération en
 masse (extraction de `.bnp`, `git checkout`...) déclenche des dizaines
 d'événements — sans debounce, chacun relancerait un scan complet
 (`search_paths_dialog.py`). Seuls les événements de type
 `created/deleted/modified/moved` comptent ; les accès en lecture pure
 (`opened`/`closed_no_write`) sont ignorés, sinon charger un `.shape` dans
 le workspace watché déclencherait lui-même un rescan
 (`search_paths_dialog.py`).
- **Cache des échecs de parsing** : un `.anim`/`.skel` que `pynel` ne sait
 pas parser (le commentaire cite explicitement `CTrackKeyFramerTCBQuat`/
 `BezierVector`/`BezierFloat`/`ConstBool` comme classes de piste non
 supportées) était re-parsé en entier à *chaque* scan avant l'ajout de ce
 cache d'échec — noté comme "a pynel gap, not a scanning bug; worth a
 separate look at some point" (`search_paths_dialog.py`).
- **Deux flags de scan indépendants** (`_scanning_external`,
 `_scanning_workspace`) plutôt qu'un seul partagé : au démarrage, le scan
 workspace-only et le `reload` complet doivent pouvoir avancer en
 parallèle (entrelacés frame par frame) sans qu'un verrou partagé fasse
 échouer silencieusement l'un des deux (`search_paths_dialog.py`).
- **`has_scanned_data`** distingue un démarrage "cache hit" (données déjà
 publiées) d'un démarrage réellement à froid, utilisé par le popup de
 restauration de scan de `object_editor.py` pour décider s'il faut attendre
 le scan de fond (`search_paths_dialog.py`).
- **Watcher filesystem uniquement sur le workspace**, jamais sur les
 dossiers de recherche externes — ceux-là restent couverts par le seul
 bouton Reload manuel, comme avant l'ajout du watcher
 (`search_paths_dialog.py`).
- **`_iter_bnp_entries`** est noté comme un gain mesuré comme "comparativement
 petit" par rapport au coût total d'un scan (la construction des ~10^5
 `FoundEntry` domine), mais gardé car des arbres de données réels contiennent
 des centaines d'archives (`search_paths.py`).
