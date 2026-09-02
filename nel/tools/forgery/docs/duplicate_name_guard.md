# duplicate_name_guard

**Fichier :** `nel/tools/forgery/ryzom_forgery/duplicate_name_guard.py` (créé le
2026-09-02)

## Rôle

Suivi partagé de collision de nom pour les 3 groupes déclenchés par extension du
workspace watcher (`import_watcher.py` -> `shapes/`, `tex_dds_sync.py` -> `build/dds/`,
`workspace_sync.py` -> le miroir externe) -- voir le chantier "Workspace watcher:
extension-based triggers anywhere + duplicate-name guard" dans
`project-todos/ryzom-core/forgery-object-editor.md`. Chaque groupe flatten ses fichiers
matchés vers une sortie sans sous-dossier (nom seul) ; deux fichiers source différents
partageant ce nom (insensible à la casse) collisionneraient sinon silencieusement. Une
instance de `DuplicateNameGuard` par groupe, chacune avec sa propre clé de comparaison
(voir plus bas) et sa propre notion de "fichiers actuellement connus" -- jamais comparées
entre groupes.

## API principale

- `DuplicateNameGuard(key_of, on_conflict)` :
  - `key_of(path)` — fonction fournie par l'appelant, calcule la clé de comparaison.
 **Stem** (`path.stem.lower()`) pour `import_watcher`/`tex_dds_sync` (l'extension de
 sortie diffère de celle de la source -- `chest.obj` et `chest.fbx` produiraient
 tous les deux `shapes/chest.shape`). **Nom complet** (`path.name.lower()`) pour
 `workspace_sync` (rien n'est converti, la sortie garde le nom d'entrée tel quel --
 `chest.shape` et `chest.dds` ne collisionnent pas).
  - `on_conflict(path_a, path_b)` — appelée au plus une fois par paire non résolue,
 dès que deux fichiers différents actuellement connus partagent une clé.
  - `scan(paths)` — reconstruction complète (démarrage/reconcile) ; renvoie les chemins
 sûrs (sans collision) à traiter. Seuls les 2 premiers fichiers trouvés par clé en
 conflit sont signalés -- un 3e homonyme reste non traité et non signalé jusqu'à
 résolution de la première paire.
  - `update(path)` — vérification incrémentale pour un settle créé/renommé ; renvoie
 `True` si sûr à traiter.
  - `remove(path)` — à appeler quand `path` a disparu (suppression, ou côté source d'un
 renommage) ; renvoie le survivant d'un conflit dont `path` faisait partie, s'il existe
 encore -- redevenu sûr, et jamais traité tant que le conflit tenait, donc à la charge de
 l'appelant.
  - `known_paths()` — tous les chemins actuellement suivis sans conflit (pour
 `workspace_sync.py`'s `sync_now()`/`refresh_fully_synced()`, réutilisant l'index déjà
 construit plutôt que rescanner le disque).

## Utilisation

Une instance par watcher, construite dans son propre `__init__` avec le `key_of`
approprié, exposée aux callers de `apps/object_editor.py` via un hook `on_name_conflict`
optionnel passé au constructeur. `object_editor.py` route les 3 hooks vers
`_on_name_conflict(group, path_a, path_b)` (`shape_io.py`), qui met en file d'attente
(`_pending_name_conflicts`, jamais écrasée -- un seul scan de démarrage peut révéler
plusieurs conflits indépendants) et affiche un popup partagé
(`_draw_name_conflict_popup()`) : aperçu des 2 fichiers (nom éditable + tooltip chemin
complet + bouton "révéler dans le gestionnaire de fichiers"), 3 actions -- "Ne garder que
le 1"/"Ne garder que le 2" (supprime l'autre), "Renommer" (actif seulement si les 2 champs
diffèrent, renomme sur disque le(s) fichier(s) dont le champ a changé).

## Points notables / pièges

- **Pas de scan périodique automatique** : la détection ne tourne qu'au démarrage
 (`set_workspace_dir`, thread de fond) et à chaque settle créé/renommé du watcher partagé
 -- suffisant puisque le garde-fou tourne systématiquement *avant* toute action
 (import/conversion/copie), jamais après (voir discussion 2026-09-02 avec Nuno : pas de
 fenêtre possible où une collision atteint la sortie sans passer par la popup).
- `remove()` ne renvoie le survivant que s'il existe encore sur disque au moment de
 l'appel -- un cas rare mais possible (les deux fichiers supprimés coup sur coup) ne
 redéclenche rien, ce qui est correct (plus rien à traiter).
- Une seule popup à la fois côté UI (`shape_io.py`) même si plusieurs `DuplicateNameGuard`
 signalent des conflits en parallèle -- mise en file, traitée dans l'ordre d'arrivée.
