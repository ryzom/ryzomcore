# workspace_watch

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspace_watch.py` (créé le 2026-08-27,
reworké le 2026-09-02)

## Rôle

Watcher filesystem générique pour un workspace : **un seul** `watchdog.observers.Observer`
(un seul thread) surveillant récursivement toute la racine du workspace, avec **un seul**
handler de debounce partagé. Chaque fichier modifié est routé vers les callbacks
enregistrées **par extension** (n'importe quel dossier) ou pour **un chemin exact**
(fichier de config racine, ex. `panoply.cfg`).

Remplace ce qui était auparavant un `Observer` dédié par fonctionnalité
(`import_watcher.py`, `workspace_sync.py`, chacun dupliquant le même mécanisme de
debounce par fichier). Consolidation demandée par l'utilisateur (2026-08-27) : avoir
plusieurs `Observer` n'apportait aucun bénéfice de fluidité/concurrence réel ici (un
`Observer` en attente ne coûte quasi rien, et le travail de fond tournait déjà sur des
threads dédiés indépendamment du nombre d'`Observer` ; le rendu Panda3D était déjà
découplé de ces watchers via une file d'attente cross-thread côté app) — c'était de la
duplication organique, pas un choix de perf délibéré. Un seul code partagé est aussi plus
facile à instrumenter/durcir (logs, isolation des erreurs par callback) que plusieurs
quasi identiques.

**Reworké 2026-09-02** (chantier "Workspace watcher: extension-based triggers anywhere +
duplicate-name guard", `project-todos/ryzom-core/forgery-object-editor.md`) : le dispatch
routait auparavant par **sous-dossier de premier niveau** (`"imports"`, `"tex"`,
`SYNCED_SUBDIRS`), incohérent avec le rework de l'affichage par catégorie virtuelle
(`virtual_categories.py`, les fichiers sont parcourus par extension, peu importe leur
vrai sous-dossier). Le dispatch route maintenant par extension, et ignore totalement tout
chemin exclu (`Settings.exclusion_rules`, jamais vérifié auparavant ici).

## API principale

- `_DebouncedHandler` — handler `watchdog` unique : timer par fichier
 (`_WATCH_DEBOUNCE_SECONDS = 0.5`), redémarré à chaque événement. Un événement `moved`
 planifie le settle pour **les deux** chemins (source ET destination), puisqu'un
 renommage affecte deux fichiers logiques.
- `WorkspaceWatcher` :
  - `register_extension(extensions, callback)` — enregistre une callback pour tout
 fichier réglé dont l'extension (en minuscule) est dans `extensions`, **n'importe quel
 dossier**. Peut être appelé avant ou après `set_workspace_dir`.
  - `register_exact(relative_path, callback)` — enregistre une callback pour un chemin
 relatif au workspace précis (ex. `"panoply.cfg"`) — pour un fichier de config, pas une
 catégorie d'asset.
  - `set_workspace_dir(workspace_dir)` — arrête l'ancien watch, démarre un nouvel
 `Observer` récursif sur la racine du workspace (`None` pour arrêter).
  - `_dispatch(path)` — au settle, vérifie d'abord `virtual_categories.is_path_excluded()`
 (relit `Settings.exclusion_rules` à chaque appel) — un chemin exclu (dossier ou motif
 fichier) n'atteint **aucune** callback, quel que soit le mode d'enregistrement. Sinon,
 appelle chaque callback dont l'extension correspond, plus chaque callback enregistrée
 pour ce chemin exact. **Chaque callback est protégée individuellement**
 (`try/except` autour de l'appel) : une callback qui lève une exception est loggée mais
 n'empêche pas les autres callbacks ni ne tue le watcher.

## Utilisation

Instancié une fois dans `apps/object_editor.py` (`self.workspace_watch`), avec les
enregistrements suivants :
- `import_watcher.IMPORT_EXTENSIONS` (`.obj/.dae/.fbx/.gltf/.glb`) →
 `ImportWatcher.handle_settled` (conversion en `.shape`, voir `docs/import_watcher.md`)
- `tex_dds_sync.TEX_EXTENSIONS` (`.tga/.png`) → `TexDdsSyncWatcher.handle_settled`
 (conversion en `.dds`, voir `docs/tex_dds_sync.md`)
- `"panoply.cfg"` (`register_exact`) → `_on_panoply_cfg_settled`
- `workspace_sync.SYNCED_EXTENSIONS` (`.shape/.anim/.skel/.dds`) →
 `WorkspaceSyncWatcher.handle_settled` (miroir externe, voir `docs/workspace_sync.md`)

`_on_active_workspace_changed` (`object_editor.py`) appelle
`workspace_watch.set_workspace_dir` en plus du `set_workspace_dir` de chacun des
trois composants (qui, eux, ne gèrent plus leur propre `Observer` — juste leur état
propre, leur index anti-doublon, et pour `workspace_sync`/`tex_dds_sync`, leur logique de
rattrapage).

## Points notables / pièges

- Le watch est récursif sur **toute** la racine du workspace (y compris `masks/`,
 `exports/`, tout sous-dossier) — seule l'exclusion (`Settings.exclusion_rules`) empêche
 un chemin de déclencher quoi que ce soit ; un fichier d'une extension non enregistrée
 (et pas au chemin exact `panoply.cfg`) est simplement ignoré au dispatch.
- `register_extension`/`register_exact` sont deux registres indépendants — un même
 chemin peut en théorie matcher les deux (pas de cas réel actuellement, `panoply.cfg`
 n'a pas d'extension surveillée par ailleurs).
- Trois des quatre callbacks (`ImportWatcher`, `TexDdsSyncWatcher`, `WorkspaceSyncWatcher`)
 flattent leur sortie (nom de fichier seul, sans sous-dossier) — voir
 `duplicate_name_guard.py` pour le garde-fou anti-collision associé, propre à chacun de
 ces trois consommateurs (pas dans `workspace_watch.py` lui-même).
