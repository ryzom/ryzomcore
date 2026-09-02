# workspace_sync

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspace_sync.py`

## Rôle

Synchronise en direct (copie de fichiers) tout `.shape`/`.anim`/`.skel`/`.dds` trouvé
**n'importe où** dans un workspace Forgery (excluant les chemins exclus,
`Settings.exclusion_rules`) vers un dossier externe choisi par l'utilisateur (ex : le
dossier de données du jeu), **flat** (nom de fichier seul, pas de sous-dossier), pour que
les modifications faites dans Patina soient immédiatement visibles côté jeu sans étape
manuelle. Fonctionnalité livrée dans le commit `d530bcb8c` (Patina 1.0.0).

**Reworké 2026-09-02** (chantier "Workspace watcher: extension-based triggers anywhere +
duplicate-name guard", `project-todos/ryzom-core/forgery-object-editor.md`) : déclenché
sur un fichier n'importe où dans le workspace désormais, plus seulement les 4 anciens
sous-dossiers fixes (`anims/`, `shapes/`, `skels/`, `dds/`) ; le miroir est **flat**
(`<sync_folder>/forgery/<workspace>/<nom de fichier>`, plus de chemin relatif préservé)
au lieu de refléter la structure du workspace — décision utilisateur : ça matche le
format `.bnp` (`pack_workspace_bnp()` flatten déjà les mêmes 4 extensions avant
empaquetage), et une collision de nom surgirait de toute façon au moment du pack, donc
le miroir ne doit pas prétendre l'éviter en gardant des sous-dossiers qu'il n'aura plus
une fois empaqueté.

## API principale

- `SYNCED_EXTENSIONS = {".shape", ".anim", ".skel", ".dds"}` — extensions
 synchronisées, n'importe où dans le workspace. Volontairement **pas** `masks/`,
 `exports/`, `imports/` (masques Panoply = entrées, pas assets livrés ; `exports/` =
 zone de sortie du dialogue d'export ; `imports/` = zone de staging dont la sortie
 atterrit déjà dans `shapes/`, lui-même synchronisé) — **ni `.tga`/`.png`** depuis le
 chantier `patina-tex-dds-autoexport` (2026-08-27) : c'est `.dds` (le miroir généré de
 `tex/`, voir `docs/tex_dds_sync.md`, lui-même flat dans `build/dds/`) qui est ce dont le
 client a réellement besoin.
- `WorkspaceSyncWatcher(on_name_conflict=None)` :
  - `set_workspace_dir` — mémorise le workspace actif et reconstruit l'index anti-doublon
 sur un thread de fond (catch-up de détection uniquement, ne copie rien par lui-même).
  - `set_sync_folder` — mémorise/efface le dossier de sync configuré.
  - `sync_now` — reconstruit l'index anti-doublon puis copie tous les fichiers
 actuellement connus comme sûrs (non conflictuels), pour rattraper ce qui préexistait
 avant le démarrage du watch.
  - `refresh_fully_synced` — simple test d'existence (pas de comparaison
 de contenu/mtime), sur l'ensemble actuellement sûr, pour afficher ou non le bouton
 "Sync now" côté UI.
  - `handle_settled` — copie effective via `shutil.copy2`, vers
 `<sync_folder>/forgery/<nom du workspace>/<nom de fichier>` — le préfixe
 `forgery/<workspace>/` isole plusieurs workspaces dans le même dossier de sync sans
 collision entre eux.
- `pack_workspace_bnp(workspace_dir, bnp_path)` — fonction module-level indépendante
 (pas une méthode de `WorkspaceSyncWatcher`), packe tout fichier `SYNCED_EXTENSIONS`
 trouvé n'importe où dans le workspace (exclusions respectées) dans un `.bnp` flat.

## Garde-fou anti-doublon

Le flattening signifie que deux fichiers différents (n'importe où dans le workspace)
partageant le même nom complet (nom + extension, insensible à la casse) collisionneraient
silencieusement dans le miroir plat. Comparé par **nom complet**, pas juste le stem :
contrairement à `import_watcher`/`tex_dds_sync`, rien n'est converti ici — le fichier de
sortie garde exactement son nom d'entrée, donc seul un nom réellement identique
collisionne (`chest.shape` et `chest.dds` ne collisionnent pas). Un `DuplicateNameGuard`
(voir `duplicate_name_guard.py`) suit l'ensemble des sources connues par cette clé ; une
collision route vers le hook `on_name_conflict(path_a, path_b)` — popup de résolution
côté `apps/object_editor_mixins/shape_io.py` (`_draw_name_conflict_popup`, partagée avec
`import_watcher`/`tex_dds_sync`). Vérifié indépendamment du fait qu'un dossier de sync
soit configuré ou non (le problème sous-jacent, deux fichiers homonymes dans le
workspace, est réel dans les deux cas).

## Utilisation

- `settings.py` : `workspace_sync_folders: Dict[str, str]` (un dossier de sync par
 nom de workspace) et `last_workspace_sync_folder` (valeur par défaut suggérée pour un
 nouveau workspace).
- `apps/object_editor.py` : UI dans Settings > Tools (appelle `set_workspace_dir` et
 `sync_now`), enregistre `handle_settled` sur le `workspace_watch.WorkspaceWatcher`
 partagé via `register_extension(SYNCED_EXTENSIONS, ...)`, et route `on_name_conflict`
 vers le popup partagé.
- `export_dialog.py` : appelle `pack_workspace_bnp()` pour l'option "Full workspace" du
 menu d'export.

## Points notables / pièges

- **Copie unidirectionnelle et "copy-only"** : un fichier supprimé du
 workspace **reste tel quel** côté dossier de sync — aucune suppression miroir n'est
 faite. À garder en tête si un fichier de sync semble "fantôme".
- Pas de comparaison de contenu ou de date de modification pour `refresh_fully_synced`
 — c'est un simple test d'existence, donc ça peut afficher "tout synchronisé" alors
 qu'une version plus récente existe côté workspace sans avoir encore déclenché le
 watch.
- **Ne possède pas son propre `Observer`.** La surveillance filesystem elle-même (un
 seul `Observer` partagé avec `import_watcher`/`tex_dds_sync`) vit dans
 `workspace_watch.WorkspaceWatcher` — voir `docs/workspace_watch.md` pour le pourquoi
 de cette consolidation.
