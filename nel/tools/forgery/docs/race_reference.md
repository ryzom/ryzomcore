# race_reference.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/race_reference.py`

## Rôle

Fournit, par clé race/genre (ex: `fy_hof`, `tr_hof`), les données de
référence nécessaires à `hairstyle_conform.py` pour identifier/interpoler une
couture de coiffure : `face_index` (voir `shape_geometry.build_face_vertex_index()`,
utilisé pour trouver la vraie boucle de bordure soudée au visage quand cette
race/genre est la *source* d'un conform) et `seam_ring` (voir
`shape_geometry.seam_ring_by_angle()`, l'anneau canonique à interpoler quand
cette race/genre est la *cible*).

Même motif "bundlé par défaut + override de workspace" que `panoply_config.py`
pour le fichier `.cfg` lui-même (`race_reference.cfg`, format NeL
`CConfigFile` classique — `<race_key>_face = "nom.shape";` /
`<race_key>_reference_hairstyle = "nom.shape";`). Contrairement à
`panoply.cfg` (données de couleur génériques), les deux fichiers `.shape`
nommés dans la config ne sont **pas** bundlés (données de jeu réelles) —
résolus par nom via les search paths de l'appelant
(`ryzom_forgery.search_paths`, même lookup .bnp-aware que pour toute
référence de texture ailleurs dans la suite).

## API principale

- `RaceReference` (dataclass) — `face_index, seam_ring`.
- `set_workspace_dir(path)` — même motif que `panoply_config.set_workspace_dir()` : un `race_reference.cfg` à la racine du workspace actif, s'il existe, écrase entièrement le bundlé.
- `workspace_cfg_path(workspace_dir)` / `bundled_cfg_path()` — chemins respectifs, pour un futur bouton "copier vers le workspace" (pas encore câblé dans Patina — voir "Points notables").
- `_resolve_cfg_path()` / `_load_doc()` — même motif de cache `(chemin, mtime)` que `panoply_config.py`.
- `get_reference(race_key, entries_by_lower_name)` — résout `<race_key>_face`/`<race_key>_reference_hairstyle` dans le `.cfg`, les cherche dans `entries_by_lower_name` (un `search_paths.build_texture_index()` ou l'équivalent live de `SearchPathsDialog`), et construit/retourne le `RaceReference`. Lève `FileNotFoundError` si l'un des deux noms est introuvable dans les search paths.

## Utilisation

- `hairstyle_conform.py` (CLI) : construit `entries_by_lower_name` via
  `search_paths.build_texture_index()` à partir de `--search-path`, appelle
  `get_reference()` une fois pour la race source, une fois pour la race
  cible.
- Cache : la clé combine le `(path, mtime)` du `.cfg` résolu **et**
  `FoundEntry.cache_stat()` (mtime, taille) de chacun des deux fichiers
  `.shape` résolus (l'archive `.bnp` elle-même si l'entrée en vient, voir
  `FoundEntry.cache_stat()`) — donc un rechargement du fichier `.shape` de
  référence (import dans le workspace, ré-export) invalide automatiquement
  le cache, pas seulement une édition du `.cfg`.

## Points notables / pièges

- Le cache est **entièrement en mémoire, par processus** (`_reference_cache`,
  dict global) — rien n'est persisté sur disque, contrairement au motif
  `creatures_ref_cache.json` de `creature_ref.py`. Pour un usage CLI
  court-vécu (un process par invocation de `hairstyle_conform.py`), l'intérêt
  du cache est surtout de partager le coût entre plusieurs coiffures traitées
  dans un futur usage batch/Patina, pas entre invocations séparées.
- Pas encore de bouton "copier `race_reference.cfg` vers le workspace" dans
  Patina — `workspace_cfg_path()`/`bundled_cfg_path()` existent pour ça
  (même API que `panoply_config.py`) mais rien ne les appelle encore ; à
  faire lors de la phase 2 (intégration Patina, voir
  `docs/apps/hairstyle_conform.md`).
- `get_reference()` ne valide pas que le `seam_ring` résultant est non vide
  avant de le mettre en cache -- un appelant qui a besoin de cette garantie
  (ex: la coiffure de référence n'a en fait aucune boucle soudée à son
  propre visage) doit vérifier `reference.seam_ring` lui-même après l'appel
  (voir `hairstyle_conform.py main()`).
