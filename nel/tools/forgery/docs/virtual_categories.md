# virtual_categories

**Fichier :** `nel/tools/forgery/ryzom_forgery/virtual_categories.py`

## Rôle

Classe les fichiers d'un workspace en catégories virtuelles fixes
(forgery-workspace-projects chantier, 2026-09) : `shapes`, `textures`,
`3d files`, `masks`, `anims`, `skels`, `others` -- uniquement par nom
(extension, ou suffixe `_<axis>` pour un masque Panoply), **peu importe
le vrai sous-dossier où le fichier vit sur disque**. Rien n'est déplacé :
c'est un regroupement d'affichage/recherche, consommé par
`explorer.py`'s mode de navigation virtuel et par la résolution de
cible save/export (`find_existing_file`).

`dds` n'a délibérément pas sa propre catégorie : c'est un format
dérivé/final (`build/dds/`, lui-même couvert par l'exclusion par défaut
du dossier `build`), pas une source à parcourir comme les autres -- un
`.dds` isolé qui ne serait pas sous un dossier exclu atterrit simplement
dans `others`.

## API principale

- `CATEGORIES` / `CATEGORY_*` (`virtual_categories.py`) — les 7 catégories,
 dans l'ordre où `scan_workspace()` construit ses buckets (ordre stable,
 réutilisé tel quel par `explorer.py`'s affichage).
- `categorize(name)` (`virtual_categories.py`) — la catégorie d'un nom de
 fichier seul (pas de chemin) : extension pour la plupart, suffixe
 `_skin`/`_user`/`_hair`/`_eyes` (mêmes 4 axes que `panoply.py`'s `AXES` /
 `panoply_bake.py`'s `_CANDIDATE_AXES`) pour reconnaître un masque parmi
 les fichiers image. Ignore les règles d'exclusion -- voir
 `scan_workspace()`.
- `is_path_excluded(relative_dir, filename, exclusion_rules)`
 (`virtual_categories.py`) — vrai si ce fichier doit être exclu de la
 **recherche/indexation** (dossier OU motif fichier, voir
 `settings.md`'s `ExclusionRule`) ; utilisé par
 `search_paths_dialog.py`'s scan du workspace actif, pas par
 `scan_workspace()` (qui traite les deux cas différemment, voir plus bas).
- `scan_workspace(workspace_root, exclusion_rules)` (`virtual_categories.py`)
 — scan récursif complet, retourne `{catégorie: [Path, ...]}`. Une règle
 d'exclusion **dossier** élague tout le sous-arbre (jamais même visité,
 via `os.walk`'s `dirnames[:]` modifié en place) ; une règle **fichier**
 laisse le fichier visible mais le force dans `CATEGORY_OTHERS` quelle que
 soit sa catégorie naturelle (voir `ExclusionRule`'s propre docstring dans
 `settings.py` -- ce comportement diffère volontairement de
 `is_path_excluded()`, qui traite les deux pareil pour la recherche).
- `find_existing_file(workspace_root, filename, exclusion_rules)`
 (`virtual_categories.py`) — cherche un fichier nommé exactement
 `filename` n'importe où dans le workspace (dossiers exclus non
 parcourus) ; `None` si aucun, le plus récemment modifié en cas de vrais
 doublons. Utilisé par `apps/object_editor.py`'s
 `_workspace_shape_save_path()` pour cibler l'écrasement où que vive
 déjà le fichier, plutôt que toujours `shapes/<nom>`.
- `_iter_included_files(workspace_root, exclusion_rules)`
 (`virtual_categories.py`) — le générateur `os.walk`-based partagé par
 `scan_workspace`/`find_existing_file`, seul endroit qui élague
 effectivement les dossiers exclus. N'élague que les dossiers -- une
 exclusion **fichier** laisse le fichier dans la sortie (voir
 `scan_workspace()` ci-dessus, qui applique cette distinction lui-même).
- `iter_included_files(workspace_root, exclusion_rules)` (public, ajouté
 2026-09-02) — même parcours, mais applique aussi les exclusions **fichier**
 (`_is_file_excluded`), donc ne renvoie que des fichiers réellement inclus,
 dossier et fichier confondus. Pour un appelant qui doit **complètement**
 ignorer l'exclu, pas juste le recatégoriser (`scan_workspace()`) ou le
 comparer à la recherche (`is_path_excluded()`) -- utilisé par le workspace
 watcher (`workspace_watch.py`, `import_watcher.py`, `tex_dds_sync.py`,
 `workspace_sync.py`) : un fichier exclu ne doit jamais déclencher
 d'auto-import/conversion/sync, point final.

## Utilisation

- `explorer.py` : ne l'importe **pas** directement -- reçoit un dict déjà
 calculé via `virtual_categories_source` (voir `explorer.md`).
- `apps/object_editor.py` : `_scan_active_workspace_virtual_categories()`
 (appelle `scan_workspace()`, throttlé à 1 scan/seconde, voir
 `_VIRTUAL_CATEGORIES_RESCAN_INTERVAL`) et `_workspace_shape_save_path()`
 (appelle `find_existing_file()`).
- `search_paths_dialog.py` : `_make_workspace_exclude()` appelle
 `is_path_excluded()` pour construire le prédicat passé à
 `search_paths.iter_all_entries(..., exclude=...)`, uniquement pour le
 scan du workspace actif (jamais pour les search paths externes, qui
 n'ont pas de notion d'exclusion).
- `settings.py` : `ExclusionRule`/`EXCLUSION_KIND_FOLDER`/`EXCLUSION_KIND_FILE`
 importés ici pour interpréter `Settings.exclusion_rules`.

## Points notables / pièges

- **`scan_workspace()` et `is_path_excluded()` traitent une exclusion
 fichier différemment** — le premier montre quand même le fichier (dans
 `others`, pour l'affichage Wexplorer) ; le second dit "exclu, point" (pour
 l'indexation/recherche). Les deux partagent la même logique de règle
 dossier (`_is_folder_excluded`), qui elle est identique dans les deux cas.
 Facile à confondre si on retouche l'un des deux sans l'autre.
- **Pas de cache interne** : ce module ne mémorise rien -- c'est aux
 appelants (voir `apps/object_editor.py`'s throttling) d'éviter de relancer
 un scan récursif complet à chaque frame ImGui.
- **`_iter_included_files` élague via `os.walk`'s `dirnames[:]`, pas après
 coup** : un dossier exclu n'est jamais ouvert/listé, pas juste filtré de
 la sortie -- important pour un dossier volumineux (ex. `build/` avec
 beaucoup de `.dds` générés) ou même juste inaccessible en lecture.
