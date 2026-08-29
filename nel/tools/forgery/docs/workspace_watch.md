# workspace_watch

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspace_watch.py` (~145 lignes, créé le
2026-08-27)

## Rôle

Watcher filesystem générique pour un workspace : **un seul** `watchdog.observers.Observer`
(un seul thread) surveillant récursivement toute la racine du workspace, avec **un seul**
handler de debounce partagé. Chaque fichier modifié est routé vers les callbacks
enregistrées pour le sous-dossier de premier niveau dont il dépend (`tex`, `imports`,
`shapes`, etc.).

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

## API principale

- `_DebouncedHandler` — handler `watchdog` unique : timer par fichier
 (`_WATCH_DEBOUNCE_SECONDS = 0.5`), redémarré à chaque événement. Un événement `moved`
 planifie le settle pour **les deux** chemins (source ET destination), puisqu'un
 renommage affecte deux fichiers logiques.
- `WorkspaceWatcher` :
  - `register(subdir, callback)` — enregistre une callback pour un sous-dossier de
 premier niveau du workspace (ex. `"tex"`, `"imports"`). Peut être appelé avant ou
 après `set_workspace_dir`.
  - `set_workspace_dir(workspace_dir)` — arrête l'ancien watch, démarre un nouvel
 `Observer` récursif sur la racine du workspace (`None` pour arrêter).
  - `_dispatch(path)` — au settle, résout le sous-dossier de premier niveau
 du chemin modifié et appelle chaque callback enregistrée pour ce sous-dossier.
 **Chaque callback est protégée individuellement** (`try/except` autour de l'appel) :
 une callback qui lève une exception est loggée mais n'empêche pas les autres
 callbacks ni ne tue le watcher.

## Utilisation

Instancié une fois dans `apps/object_editor.py` (`self.workspace_watch`), avec les
enregistrements suivants :
- `"imports"` → `ImportWatcher.handle_settled` (conversion en `.shape`, voir
 `docs/import_watcher.md`)
- `"tex"` → `TexDdsSyncWatcher.handle_settled` (conversion en `.dds`, voir
 `docs/tex_dds_sync.md`)
- `"anims"`, `"shapes"`, `"skels"`, `"dds"` (= `workspace_sync.SYNCED_SUBDIRS`) →
 `WorkspaceSyncWatcher.handle_settled` (miroir externe, voir `docs/workspace_sync.md`)

`_on_active_workspace_changed` (`object_editor.py`) appelle
`workspace_watch.set_workspace_dir` en plus du `set_workspace_dir` de chacun des
trois composants (qui, eux, ne gèrent plus leur propre `Observer` — juste leur état
propre et, pour `workspace_sync`/`tex_dds_sync`, leur logique de rattrapage).

## Points notables / pièges

- Le watch est récursif sur **toute** la racine du workspace (y compris `masks/`,
 `exports/`, sous-dossiers sans callback enregistrée) — les événements qui tombent
 dans un sous-dossier sans callback sont simplement ignorés au dispatch. Léger surcoût
 de timers de debounce inutiles pour ces dossiers, jugé négligeable face à la
 simplicité du design à un seul `Observer`.
- Le dispatch se fait uniquement sur le **premier niveau** de chemin relatif
 (`relative.parts[0]`) — un fichier dans `tex/foo/bar.png` est routé vers les callbacks
 de `"tex"`, peu importe la profondeur.
