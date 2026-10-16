# workspaces

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspaces.py` (~95 lignes)

## Rôle

Gère le concept de "workspace" : un unique dossier racine (`settings.workspaces_root`),
partagé par tous les outils Forgery, contenant un sous-dossier par workspace éditable.
Un workspace est l'espace de travail dans lequel Patina (et les autres apps Forgery)
lisent/écrivent leurs textures, masques Panoply, shapes, animations, squelettes, exports
et imports.

## API principale

- `SUBDIRS = ("tex", "masks", "shapes", "anims", "skels", "exports", "imports")`
 (`workspaces.py`) — la structure de dossiers standard d'un workspace :
  - `tex/` : textures sources du workspace (éditables, synchronisées en direct — voir
 `workspace_sync.md`).
  - `masks/` : masques Panoply (recolorisation) — entrées, jamais synchronisées.
  - `shapes/` : fichiers `.shape` du workspace, cible de l'auto-export (voir
 `import_watcher.md`).
  - `anims/`, `skels/` : animations et squelettes.
  - `exports/` : zone de sortie du dialogue d'export (`export_dialog.py`).
  - `imports/` : zone surveillée par `import_watcher.py` pour l'auto-import de meshes.
- `list_workspaces(root)` — liste les sous-dossiers de `root`.
- `workspace_path(root, name)` — chemin d'un workspace donné.
- `create_workspace(root, name)` — crée un workspace et sa structure.
- `ensure_structure(path)` — crée les `SUBDIRS` manquants dans un workspace
 existant.
- `is_inside(path, workspace)` — vérifie qu'un chemin est bien à l'intérieur
 du workspace, en résolvant les chemins (robuste aux symlinks/`..`).
- `active_workspace_path(settings)` — résout `settings.active_workspace` par
 rapport à `settings.workspaces_root`.
- `open_in_system_file_manager` / `reveal_in_system_file_manager` —
 ouverture native (explorer/Finder/xdg-open), avec sélection du fichier sur
 Windows/macOS uniquement.

## Utilisation

- `settings.py` : champs `workspaces_root` et `active_workspace`.
- `apps/object_editor.py` (Patina) : orchestre la création/sélection de workspace et
 toute la logique liée aux `SUBDIRS`.
- `workspace_setup_dialog.py` : UI de création/sélection de workspace.
- `workspace_sync.py`, `import_watcher.py` : opèrent sur des sous-dossiers spécifiques
 du workspace (`tex/`+`anims/`+`shapes/`+`skels/` pour le sync, `imports/`+`shapes/`
 pour l'auto-import).

## Points notables / pièges

- `workspaces.py` référence un chemin `.todo/forgery-object-editor.md` qui est
 **obsolète/stale** : le vrai fichier de suivi de ce chantier est désormais externe au
 repo, à `/repos/project-todos/ryzom-core/forgery-object-editor.md` (voir la convention
 de ce dépôt décrite dans `CLAUDE.md`).
- `is_inside` résout les chemins avant comparaison : important pour éviter qu'un
 chemin malicieux ou un symlink fasse croire qu'un fichier est dans le workspace alors
 qu'il ne l'est pas.
