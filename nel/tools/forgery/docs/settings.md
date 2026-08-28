# settings.py — préférences persistantes unifiées

**Fichier :** `nel/tools/forgery/ryzom_forgery/settings.py` (165 lignes)

## Rôle

Ce module centralise toutes les préférences utilisateur de Ryzom Forgery
dans un unique fichier TOML lisible/éditable à la main :
`config_dir / "settings.toml"` (voir `docs/cache_and_config_dir.md`). Le
docstring en tête (`settings.py`) justifie ce choix : un seul fichier
facile à lire/éditer à la main est jugé plus important que d'avoir chaque
concern indépendamment chargeable dans son propre fichier JSON. Il regroupe
des domaines très différents : le flux d'export, les favoris de
l'explorateur, les chemins de recherche génériques, et les workspaces.
`workspaces_root` y est explicitement décrit comme le seul vrai concept de
"racine de données" — un dossier unique partagé par toutes les apps
Forgery, contenant un sous-dossier par workspace — distinct de
`search_paths`, qui reste une liste en lecture seule et ordonnée par
priorité de dossiers externes pour résoudre des assets (textures de
shapes, compatibilité `.skel`/`.anim`, panoplies).

## API principale

- `ExportSettings` (`settings.py`) : `@dataclass` — `output_folder`
 (`None` = même dossier que le `.shape` source, jusqu'à ce que
 l'utilisateur en choisisse un explicitement), `remember_output_folder`,
 `texture_mode` (`TEXTURE_MODE_COPY_PNG` ou `TEXTURE_MODE_REFERENCE_ONLY`,
 constantes `settings.py`), `remember_texture_mode`.
- `SearchPathDir` (`settings.py`) : `@dataclass` — `path: str`,
 `recursive: bool = False`.
- `Settings` (`settings.py`) : `@dataclass` racine, tous les champs
 documentés en commentaire dans le code :
  - `explorer_favorites: List[str]`
  - `export: ExportSettings`
  - `search_paths: List[SearchPathDir]`
  - `workspaces_root: Optional[str]`, `active_workspace: Optional[str]`
  - `workspace_sync_folders: Dict[str, str]` (dossier miroir externe par
 workspace, voir `workspace_sync.py`) et
 `last_workspace_sync_folder: Optional[str]` (dernière valeur définie,
 utilisée uniquement pour pré-remplir un nouveau workspace, jamais relue
 pour un workspace existant — `settings.py`)
  - État de restauration de session : `last_folder`, `last_bnp`,
 `last_shape_path`, `last_shape_bnp`, `last_shape_name`
 (`settings.py`) — `last_shape_bnp`/`last_bnp` ne sont renseignés
 que quand le dossier/shape parcouru vit dans une archive `.bnp`.
  - `image_editor_path: Optional[str]` : chemin vers un éditeur d'image
 externe (GIMP/Krita/Photoshop), utilisé par le bouton "Edit" de l'onglet
 Textures ; le bouton reste désactivé tant que ce n'est pas défini.
  - `ui_font_name: str = "Roboto Bold"`, `ui_font_size: float = 14.0` : clé
 dans `app.py`'s `_AVAILABLE_FONTS` et sa taille ; appliqué une seule
 fois au démarrage (l'atlas de police est construit avant la première
 frame), un changement ne prend effet qu'au redémarrage — pas de
 reconstruction d'atlas à chaud.
- `load -> Settings` (`settings.py`) : parse
 `config_dir/"settings.toml"` avec `tomlkit`. Renvoie un `Settings`
 par défaut (valeurs vierges) si le fichier est absent ou invalide
 (capture `OSError`/`tomlkit.exceptions.TOMLKitError`, `settings.py`).
 Remplit chaque champ un par un depuis le TOML, avec des replis explicites
 (`or None` / valeur par défaut) plutôt qu'un simple `data.get(...)`. Les
 champs de `export` sont copiés un par un si présents dans la table
 `export` du TOML (`settings.py`), et `search_paths` est reconstruit
 uniquement à partir des entrées qui sont des dicts contenant une clé
 `"path"` (`settings.py`, entrées malformées silencieusement
 ignorées).
- `save(settings: Settings) -> None` (`settings.py`) : reconstruit un
 document `tomlkit` de zéro (pas d'édition en place du fichier existant),
 avec un commentaire d'en-tête ("safe to edit by hand"). N'écrit chaque
 champ optionnel que s'il n'est pas `None` (`settings.py`), sauf
 `explorer_favorites`, `workspace_sync_folders`, `ui_font_name`,
 `ui_font_size`, `export` et `search_paths` qui sont toujours écrits.
 Crée le dossier parent (`mkdir(parents=True, exist_ok=True)`) avant
 d'écrire le fichier.

## Utilisation

Ce module est importé comme `from . import settings as app_settings` dans
`app.py`, `workspace_setup_dialog.py`, `export_dialog.py`,
`search_paths_dialog.py`, `explorer.py`, et `apps/object_editor.py`. Le
motif d'usage systématique dans tout le projet est un cycle lire-modifier-
écrire : `fresh = app_settings.load; fresh.<champ> = ...; app_settings.save(fresh)`
(ex. `explorer.py`, `export_dialog.py`,
`workspace_setup_dialog.py`, `apps/object_editor.py`
et plusieurs autres occurrences) — aucun état `Settings` n'est gardé
longtemps en mémoire partagée entre les modules, chacun recharge à chaque
fois qu'il a besoin d'une valeur à jour.

- `app.py` (`_load_ui_font`) lit `ui_font_name`/`ui_font_size`.
- `explorer.py` lit `explorer_favorites` à la construction de
 l'Explorer.
- `search_paths_dialog.py` lit `search_paths`.
- `export_dialog.py` lit `export` (`ExportSettings`).
- `workspace_setup_dialog.py` charge `Settings` entier à la construction
 du dialogue.
- `apps/object_editor.py` est le plus gros consommateur : lit
 `search_paths`, `workspace_sync_folders` (783, 3351, 3366,
 3373), l'état de session (1369), `image_editor_path` (3216), et écrit
 différents champs à plusieurs endroits (1358, 3303, 3339, 3397, 3405).

## Points notables / pièges

- `load` ne fait aucune validation de type stricte hors les quelques
 vérifications explicites (ex. `search_paths` filtre les entrées non-dict
 ou sans `"path"`, `settings.py`) — un TOML corrompu dans une
 section non prévue ne lève pas d'erreur, il est simplement ignoré.
- Toute la persistance repose sur `tomlkit` plutôt que le module `toml`/
 `tomllib` standard — probablement pour préserver le style d'écriture
 humaine (commentaires, formatage) plutôt qu'un simple besoin de
 parsing ; le fichier passe explicitement par `tomlkit.document`/
 `tomlkit.table`/`tomlkit.comment` en écriture (`settings.py`).
- Le cycle lire-modifier-écrire pratiqué partout dans le projet (`fresh =
 load; ...; save(fresh)`) signifie qu'il n'y a pas de verrouillage de
 fichier ni de gestion de concurrence : deux écritures rapprochées à
 peu d'intervalle (ex. deux widgets modifiant deux champs différents dans
 la même frame) pourraient en théorie s'écraser l'une l'autre si les deux
 ont chargé leur `fresh` avant que l'autre n'ait sauvegardé. Rien dans le
 code lu ne protège explicitement contre ce cas.
- `ui_font_size`/`ui_font_name` sont les deux seuls champs de préférence
 qui nécessitent un redémarrage complet pour prendre effet (documenté en
 commentaire, `settings.py`) — cohérent avec l'existence de
 `ForgeryApp.relaunch` dans `app.py`.
