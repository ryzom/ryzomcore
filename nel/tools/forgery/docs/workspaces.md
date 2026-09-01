# workspaces

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspaces.py`

## Rôle

Gère les concepts de "projet" et "workspace" (forgery-workspace-projects
chantier, 2026-09) : `settings.workspaces_root` est un unique dossier
racine, partagé par tous les outils Forgery, contenant un sous-dossier par
**projet**, lui-même contenant un sous-dossier par **workspace** éditable.
Un workspace reste l'espace de travail dans lequel Patina (et les autres
apps Forgery) lisent/écrivent leurs textures, masques Panoply, shapes,
animations, squelettes, exports et imports -- les projets ne font que
regrouper plusieurs workspaces liés entre eux.

Un workspace peut aussi être **externe** : un dossier vivant n'importe où
ailleurs sur le disque, simplement référencé par le manifeste
`external_workspaces.toml` de son projet (jamais copié/déplacé). Il se
comporte ensuite exactement comme un workspace interne (mêmes catégories
virtuelles -- voir `virtual_categories.md` --, mêmes règles d'exclusion,
même résolution save/export) ; seule sa localisation physique diffère.

## API principale

- `SUBDIRS = ("tex", "masks", "shapes", "anims", "skels", "imports", "exports", "build")`
 (`workspaces.py`) — la structure de dossiers standard d'un workspace **interne** :
  - `tex/` : textures sources du workspace (éditables, synchronisées en direct — voir
 `workspace_sync.md`).
  - `masks/` : masques Panoply (recolorisation) — fichiers sources, jamais exclus de rien.
  - `shapes/` : fichiers `.shape` du workspace (emplacement canonique de repli si aucun
 fichier existant n'a été trouvé ailleurs — voir `virtual_categories.md`'s
 `find_existing_file`).
  - `anims/`, `skels/` : animations et squelettes.
  - `imports/` : zone surveillée par `import_watcher.py` pour l'auto-import de meshes.
  - `exports/` : zone de sortie du dialogue d'export (`export_dialog.py`) — **exclue par
 défaut** (voir Settings.exclusion_rules dans `settings.md`) pour éviter une boucle
 import → shape → export.
  - `build/` (et son sous-dossier `dds/`, `_BUILD_SUBDIRS`) : sortie dérivée/finale (le
 rendu Panoply `.dds`) — **exclue par défaut** elle aussi ; `dds` n'a donc plus son propre
 sous-dossier de premier niveau comme avant ce chantier.
- `list_projects(root)` / `project_path(root, project)` / `create_project(root, project)`
 — un projet lui-même n'a pas de `SUBDIRS` propre (ça, c'est le rôle de chaque workspace
 à l'intérieur).
- `list_workspaces(root, project)` — les workspaces **internes** (vrais sous-dossiers)
 d'un projet uniquement ; les externes (voir ci-dessous) sont une liste séparée
 délibérément, pour ne pas coupler les deux notions.
- `workspace_path(root, project, name)` — chemin d'un workspace interne donné.
- `create_workspace(root, project, name)` — crée un workspace interne et sa structure.
- `ensure_structure(path)` — crée les `SUBDIRS` (et `build/dds`) manquants dans un
 workspace existant — **jamais appelé pour un workspace externe** (voir
 `apps/object_editor.py`'s `_on_active_workspace_changed`/`workspace_setup_dialog.py`'s
 `set_active_workspace`, tous deux gardés par
 `WorkspaceSetupDialog.is_active_workspace_external()`).
- `list_external_workspaces(project_dir)` / `add_external_workspace(project_dir, name, path)`
 / `external_workspace_path(project_dir, name)` — lisent/écrivent le manifeste
 `external_workspaces.toml` du projet (dans le dossier du projet lui-même, pas dans le
 `settings.toml` global — voyage avec le projet s'il est partagé/déplacé).
- `migrate_legacy_workspaces(root, project_name)` — pour une config d'avant ce chantier
 (workspaces directement sous `root`, sans projet) : crée `project_name` et **déplace**
 (pas copie) tous les dossiers trouvés directement sous `root` dedans, en conservant leur
 nom. Appelée une fois par `workspace_setup_dialog.py`'s flux de migration (voir
 `workspace_setup_dialog.md`).
- `is_inside(path, workspace)` — vérifie qu'un chemin est bien à l'intérieur
 du workspace, en résolvant les chemins (robuste aux symlinks/`..`).
- `active_workspace_path(settings)` — résout `settings.active_project` +
 `settings.active_workspace` par rapport à `settings.workspaces_root` ; vérifie d'abord le
 manifeste externe du projet actif (un nom qui y correspond l'emporte), sinon résout en
 interne. `None` si l'un des trois n'est pas défini, ou si le dossier résolu n'existe plus.
- `open_in_system_file_manager` / `reveal_in_system_file_manager` —
 ouverture native (explorer/Finder/xdg-open), avec sélection du fichier sur
 Windows/macOS uniquement.

## Utilisation

- `settings.py` : champs `workspaces_root`, `active_project`, `active_workspace`,
 `exclusion_rules`.
- `virtual_categories.py` : consomme `Settings.exclusion_rules` pour son scan récursif
 (voir `virtual_categories.md`) — ne connaît rien du modèle projet/workspace lui-même,
 juste des chemins et des règles d'exclusion.
- `apps/object_editor.py` (Patina) : orchestre la création/sélection de projet/workspace
 (via `workspace_setup_dialog.py`) et toute la logique liée aux `SUBDIRS`.
- `workspace_setup_dialog.py` : UI de création/sélection de projet/workspace (deux combos),
 popup obligatoire de premier lancement et flux de migration.
- `workspace_sync.py`, `import_watcher.py` : opèrent sur des sous-dossiers spécifiques
 du workspace actif (`tex/`+`anims/`+`shapes/`+`skels/` pour le sync, `imports/`+`shapes/`
 pour l'auto-import) — inchangés par ce chantier, toujours de vrais chemins physiques,
 pas les catégories virtuelles de l'Explorer.

## Points notables / pièges

- `is_inside` résout les chemins avant comparaison : important pour éviter qu'un
 chemin malicieux ou un symlink fasse croire qu'un fichier est dans le workspace alors
 qu'il ne l'est pas.
- `list_workspaces`/`list_external_workspaces` sont volontairement deux listes séparées,
 jamais fusionnées ici -- c'est aux appelants (le combo "Workspace" de
 `workspace_setup_dialog.py`, `WorkspaceSetupDialog.workspace_names()`) de les combiner
 pour l'affichage. `active_workspace_path` suppose les deux ensembles disjoints (un nom
 n'est jamais à la fois interne et externe dans un même projet) -- garanti par
 construction (`create_workspace`/`add_external_workspace` ne sont jamais appelés pour le
 même nom), pas vérifié activement.
- `migrate_legacy_workspaces` **saute** (au lieu d'écraser) un dossier legacy si un
 dossier de même nom existe déjà à la destination -- un vrai conflit à ce stade est
 traité comme quelque chose d'inattendu à laisser l'utilisateur trancher à la main,
 plutôt que de risquer de fusionner/écraser de vraies données.
