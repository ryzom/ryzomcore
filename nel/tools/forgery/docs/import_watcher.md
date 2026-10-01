# import_watcher

**Fichier :** `nel/tools/forgery/ryzom_forgery/import_watcher.py` (~309 lignes)

## Rôle

Surveille `<workspace>/imports/` pour des fichiers `.obj`/`.dae`/`.fbx` créés ou
modifiés, et maintient `<workspace>/shapes/` synchronisé automatiquement : import complet
si la shape cible n'existe pas encore, mise à jour de la géométrie + texture diffuse si
elle existe déjà. C'est le mécanisme "auto-export imports/ -> shapes/", chantier terminé
(voir Points notables).

## API principale

- `_INVALID_NAME_CHARS = re.compile(r"[^A-Za-z0-9_-]")`, `sanitize_shape_name`, `target_shape_path` — dérivent le nom de fichier `.shape`
 cible (`<workspace>/shapes/<nom_sanitizé>.shape`) à partir du fichier importé.
- `export_new_shape` — cible absente : import complet headless via
 `find_importer` (même chemin que `apps/shape_importer.py`).
- `update_existing_shape` — cible présente : remplace uniquement la
 géométrie (`shape_file.value.geom = mesh.geom`) et met à jour la texture
 diffuse de chaque matériau si elle a changé (`_update_diffuse_texture`) —
 tout le reste (blend, alpha-test, 2-sided, autres stages Multi Bitmap) est préservé.
- Exceptions : `MaterialCountMismatch` (levée ) si le nombre de
 matériaux diffère entre l'existant et le nouveau mesh ; `UnsupportedShapeTypeError` si la shape existante n'est pas un `CMesh` simple.
- `_backup_and_reexport` — en cas de `MaterialCountMismatch` : renomme la
 cible en `<stem>_backup_<YYYYMMDD_HHMMSS><suffix>` puis ré-exporte le nouveau mesh sous
 le nom cible d'origine (flux 100% automatique, sans popup de conflit).
- `class ImportWatcher` — orchestre le tout, avec hooks `is_shape_open`,
 `on_open_shape_conflict`, `on_status` (docstring détaillée ) pour gérer le
 cas où la shape cible est actuellement ouverte dans le viewport de Patina.
- **Depuis le 2026-08-27 : ne possède plus son propre `Observer`.** `set_workspace_dir`
 se contente de mémoriser le workspace actif et de créer `imports/` si absent ;
 `handle_settled` (ex-`_handle_settled`, méthode désormais publique) est enregistrée
 par `apps/object_editor.py` sur un `workspace_watch.WorkspaceWatcher` partagé pour le
 sous-dossier `"imports"` — voir `docs/workspace_watch.md` pour le pourquoi de cette
 consolidation (plus de `_DebouncedImportHandler` : le filtre
 `find_importer(path) is not None` vit maintenant dans `handle_settled` lui-même).

## Utilisation

- `apps/object_editor.py` : instancie `ImportWatcher`, fournit les hooks
 `is_shape_open`/`on_open_shape_conflict` (popup si la shape cible est ouverte dans le
 viewport), remonte `on_status` dans la sysbar (`_on_import_status` /
 `_flush_pending_import_status`), et enregistre `handle_settled` sur le
 `WorkspaceWatcher` partagé pour `"imports"`.
- Utilise en interne le même moteur que `apps/shape_importer.py` (`find_importer`).

## Points notables / pièges

- **Pas de suivi d'état "dirty" dans Patina** (mentionné explicitement ) :
 si la shape cible est ouverte dans le viewport avec des modifications non
 sauvegardées, l'auto-import peut écraser ces modifications — d'où les hooks de
 conflit fournis à `ImportWatcher`.
- Le design a changé en cours de route pour le cas `MaterialCountMismatch` : un popup de
 confirmation était initialement prévu, remplacé par le flux 100% automatique
 (backup + ré-export) — décision utilisateur, voir `_backup_and_reexport`.
- Chantier "Auto-export imports/ -> shapes/" **terminé** (5/5 steps), retiré du suivi
 externe (`project-todos/ryzom-core/forgery-object-editor.md`, commit `2ab2f29` dans ce
 dépôt de suivi) — historique récupérable via
 `git show 2ab2f29~1:ryzom-core/forgery-object-editor.md` dans `project-todos`. Journal
 narratif détaillé : `logs/forgery-object-editor.md` (sections datées 2026-08-26/27).
