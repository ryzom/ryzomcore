# import_watcher

**Fichier :** `nel/tools/forgery/ryzom_forgery/import_watcher.py`

## Rôle

Surveille **tout le workspace** (excluant les chemins exclus, `Settings.exclusion_rules`)
pour des fichiers `.obj`/`.dae`/`.fbx`/`.gltf`/`.glb` créés ou modifiés (`IMPORT_EXTENSIONS`,
n'importe quel dossier), et maintient `<workspace>/shapes/` synchronisé automatiquement :
import complet si la shape cible n'existe pas encore, mise à jour de la géométrie + texture
diffuse si elle existe déjà. C'est le mécanisme "auto-export -> shapes/", chantier terminé
(voir Points notables).

**Reworké 2026-09-02** (chantier "Workspace watcher: extension-based triggers anywhere +
duplicate-name guard", `project-todos/ryzom-core/forgery-object-editor.md`) : déclenché
sur une source n'importe où dans le workspace désormais, plus seulement `imports/` (ce
dossier reste créé comme emplacement de dépôt suggéré par défaut, mais n'est plus spécial
pour le watcher lui-même).

## API principale

- `IMPORT_EXTENSIONS` — `{".obj", ".dae", ".fbx", ".gltf", ".glb"}`, dérivé de
 `shape_import.IMPORTERS`.
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
 `on_open_shape_conflict`, `on_status`, `on_name_conflict` (docstrings détaillées) pour
 gérer le cas où la shape cible est actuellement ouverte dans le viewport de Patina, et
 le cas où deux sources différentes cibleraient le même `.shape`.
- **Ne possède pas son propre `Observer`.** `set_workspace_dir` mémorise le workspace
 actif, crée `imports/` (toujours créé, juste plus spécial au watcher), et lance sur un
 thread de fond un scan complet du workspace pour reconstruire l'index anti-doublon (sans
 rien réimporter — ce watcher est événementiel, pas un réconciliateur comme
 `tex_dds_sync`/`workspace_sync`, voir plus bas). `handle_settled` est enregistrée par
 `apps/object_editor.py` sur un `workspace_watch.WorkspaceWatcher` partagé via
 `register_extension(IMPORT_EXTENSIONS, ...)`.

## Garde-fou anti-doublon

Deux sources différentes (n'importe où dans le workspace) partageant le même nom de base
une fois sanitizé (`sanitize_shape_name(stem)`, insensible à la casse) cibleraient le même
`shapes/<nom>.shape` — collision silencieuse depuis que le déclenchement n'est plus limité
à `imports/`. Un `DuplicateNameGuard` (voir `duplicate_name_guard.py`) suit l'ensemble des
sources connues par cette clé ; une collision route vers le hook `on_name_conflict(path_a,
path_b)` au lieu d'exporter/mettre à jour l'un ou l'autre fichier — popup de résolution
côté `apps/object_editor_mixins/shape_io.py` (`_draw_name_conflict_popup`, partagée avec
`tex_dds_sync`/`workspace_sync`). Vérifié au démarrage (scan complet, avant tout import
pour une paire en conflit) et à chaque création/renommage réglé. Quand un côté du conflit
disparaît (suppression, ou renommage du fichier), le survivant est traité automatiquement
(`DuplicateNameGuard.remove()` le renvoie).

## Utilisation

- `apps/object_editor.py` : instancie `ImportWatcher`, fournit les hooks
 `is_shape_open`/`on_open_shape_conflict` (popup si la shape cible est ouverte dans le
 viewport), `on_name_conflict` (popup de doublon, voir ci-dessus), remonte `on_status`
 dans la sysbar (`_on_import_status` / `_flush_pending_import_status`), et enregistre
 `handle_settled` sur le `WorkspaceWatcher` partagé via `register_extension`.
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
