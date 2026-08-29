# panoply_config.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/panoply_config.py`

## Rôle

Fournit la table de correspondance `(axe, color_id[, race]) -> paramètres
HSL réels`, en lisant les constantes de production consolidées dans
`panoply.cfg` (fichier bundlé avec Forgery, voir son en-tête pour la
provenance et le schéma exact : fusion, avec préfixe `<race>_` pour les axes
`hair`/`eyes`, des vrais `panoply_common.cfg`/`panoply_<race>.cfg` de
`ryzom-data`, snapshot 2026-08-29). Ce module ne fait aucun I/O au-delà de la
lecture d'un unique fichier `.cfg` (`panoply_config.py`) résolu à chaque
appel via `_resolve_cfg_path()` : le `panoply.cfg` à la racine du workspace
Forgery actif s'il existe, sinon le fichier bundlé — de sorte que ni ce
module ni ses appelants ne dépendent des chemins de recherche de
l'utilisateur couvrant l'arborescence `leveldesign/` de `ryzom-data`. Les
paramètres qu'il renvoie alimentent directement les arguments
hue/lightness/saturation/luminosity/contrast de
`panoply_colorize.convert_bitmap`/`panoply_maker.convert_bitmap_exact`.

Patina (`object_editor.py`) est un outil grand public : il ne demande jamais
à l'utilisateur de choisir/fabriquer un `panoply_*.cfg` lui-même. Un
`panoply.cfg` de workspace n'existe que si l'utilisateur l'y a copié
lui-même (bouton engrenage à côté de "Panoply:",
`_copy_panoply_cfg_to_workspace()`) — voir
`/repos/project-todos/ryzom-core/panoply-runtime-tint.md` "Unified
panoply.cfg + Patina integration" pour l'historique de cette décision.

## API principale

- `RACE_PREFIX_TO_TABLE` — préfixe de 2 lettres en tête d'un nom de texture de base (`tr`, `fy`, `ma`, `zo`, `ge`) → nom de préfixe de clé `.cfg` correspondant (`tryker`, `fyros`, `matis`, `zorai`, `generique`), qui reprend le nom des fichiers `panoply_<race>.cfg` sources.
- `_COMMON_AXES = ("skin", "user")` / `_RACE_AXES = ("hair", "eyes")` — distingue les axes communs à toutes les races (clés `skin_*`/`user_*`, sans préfixe) des axes définis par race (clés `<race>_hair_*`/`<race>_eyes_*`) — correspond à `panoply.AXES`.
- `ColorParams` (dataclass frozen) — `id, hue, lightness, saturation, luminosity, contrast` — une entrée de couleur prête à être passée à `convert_bitmap`/`convert_bitmap_exact`.
- `set_workspace_dir(path)` — à appeler quand le workspace actif change (même motif que `search_paths_dialog.set_workspace_dir`) ; `object_editor.py` l'appelle depuis `_on_active_workspace_changed`. Ne recharge rien immédiatement, juste change ce que `_resolve_cfg_path()` renverra au prochain appel.
- `workspace_cfg_path(workspace_dir)` / `bundled_cfg_path()` — chemins respectifs du `panoply.cfg` de workspace (existant ou non) et du fichier bundlé — utilisés par le bouton "copier vers le workspace".
- `_resolve_cfg_path()` — workspace `panoply.cfg` s'il existe, sinon le bundlé.
- `_load_doc()` — charge (et met en cache, invalidé sur changement de chemin résolu ou de mtime) un `pynel.config_file.Document` pour le `.cfg` résolu.
- `_prefix_for(axis, race)` / `_entries_for(axis, race)` — résout le préfixe de clé pertinent et construit la liste de `ColorParams` correspondante ; renvoie `None` si l'axe ne s'applique pas à cette race ou si `race` est `None` pour un axe par-race.
- `get_color_params(axis, color_id, race=None)` — cherche l'entrée `color_id` dans les entrées résolues. Renvoie `None` si l'axe ne s'applique pas du tout à cette race (ex. zorai n'a pas d'axe `eyes`, generique n'a ni `hair` ni `eyes`) ou si le `color_id` n'existe pas.
- `available_color_ids(axis, race=None)` — liste tous les `color_id` définis pour un axe (optionnellement restreint à la table hair/eyes d'une race) — utile pour construire un sélecteur UI, et c'est ce que `panoply_bake.axes_for_source()` utilise pour savoir quels axes tenter pour une race donnée.

## Utilisation

- **Live-preview** (`object_editor.py`, `_ensure_live_panoply_texture`) : `panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower())` déduit la race, puis `panoply_config.get_color_params(axis, panoply.color_id_for(axis, dims[axis]), race)` récupère les paramètres HSL réels pour chaque axe sélectionné.
- **Bake réel** (`panoply_bake.axes_for_source()`, utilisé par `apps/panoply_maker.py` en mode autonome et par `object_editor._bake_panoply_real()`) : construit la liste complète des `ColorMaskAxis` candidats pour une race via `available_color_ids()`/`get_color_params()`, ensuite filtrée par les masques réellement trouvés sur disque.

Dans le pipeline global, ce module est le pont entre `panoply.py`
(vocabulaire des axes/valeurs) et `panoply_colorize.py`/`panoply_maker.py`
(calcul pixel par pixel, approché ou exact) : il traduit une sélection
abstraite (axe + valeur numérique/code race) en vrais paramètres de teinte
issus des données de production, sans jamais toucher aux pixels lui-même.

## Points notables / pièges

- Le fichier bundlé `panoply.cfg` est une **copie figée** des configs de production, pas une lecture live de `ryzom-data` : si la palette réelle change côté données du jeu, il doit être régénéré à la main à partir des vrais `panoply_common.cfg`/`panoply_<race>.cfg` — aucune automatisation de resynchronisation n'existe dans ce module.
- Un `panoply.cfg` de workspace, une fois présent, **écrase entièrement** le bundlé (pas de fusion partielle) — voir `_resolve_cfg_path()`.
- `get_color_params` renvoie `None` dans deux cas distincts (axe non applicable à la race vs. `color_id` introuvable) sans les différencier dans la valeur de retour — un appelant qui veut distinguer les deux cas doit passer par `available_color_ids` en amont.
- `race` est ignoré pour les axes `skin`/`user` mais **requis** (sinon `None` implicite via l'argument par défaut) pour `hair`/`eyes` — passer `race=None` pour un axe par-race renvoie systématiquement `None`, pas une erreur.
- Le module ne valide pas le format du `.cfg` au-delà de ce que `pynel.config_file` impose — un `.cfg` malformé (tableaux de longueurs différentes, clé manquante) provoquerait une `KeyError`/`ValueError`/`ConfigError` lors de l'accès, pas une erreur de chargement anticipée.
- Le cache (`_cache_path`/`_cache_mtime`/`_cache_doc`) ne se recharge que si le chemin résolu ou le mtime du fichier change — éditer le `.cfg` de workspace pendant que Patina tourne prend effet au prochain appel (pas de watch dédié, contrairement à `workspace_watch.py`).
