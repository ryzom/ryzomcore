# tex_dds_sync

**Fichier :** `nel/tools/forgery/ryzom_forgery/tex_dds_sync.py` (créé le 2026-08-27,
reworké le 2026-09-02)

## Rôle

Maintient `<workspace>/build/dds/` automatiquement synchronisé avec **toute source
TGA/PNG trouvée n'importe où dans le workspace** (excluant les chemins exclus,
`Settings.exclusion_rules`) : chaque source obtient un `build/dds/<stem>.dds` **flat**
(pas de sous-dossier, nom de base seul), régénéré à chaque création/modification, et
supprimé à chaque suppression de la source — chantier `patina-tex-dds-autoexport`
(`/repos/project-todos/ryzom-core/patina-tex-dds-autoexport.md`).

**Reworké 2026-09-02** (chantier "Workspace watcher: extension-based triggers anywhere +
duplicate-name guard", `project-todos/ryzom-core/forgery-object-editor.md`) : déclenché
sur une source n'importe où dans le workspace désormais, plus seulement `tex/`, et la
sortie est **flat** dans `build/dds/` au lieu de miroiter le sous-chemin de la source sous
un `dds/` de premier niveau. Corrige au passage un bug préexistant trouvé pendant ce
chantier : ce module lisait/écrivait un `<workspace>/dds/` racine, alors que
`workspaces.py` crée en réalité `build/dds/` (`_BUILD_SUBDIRS`, déplacé là lors du rework
des workspaces du 2026-09-01) — les deux ne désignaient jamais le même dossier.

Les textures specular sont hors scope pour l'instant : Patina ne gère pas encore les
rôles/slots de texture, donc une source n'est en pratique jamais identifiable comme
specular. Tout fichier supporté est converti uniformément, y compris les variantes Multi
Bitmap (saison/qualité) — c'est un miroir par fichier, pas conscient de quel shape/matériau
utilise réellement une texture donnée.

L'export DDS construit toujours les mipmaps (`build_mipmaps=True`) — ce sont toujours
des textures de modèles 3D (voir `docs/dds_export.md` sur pourquoi un asset régénéré
sans mipmap est visiblement dégradé).

Ne possède pas son propre watch filesystem — `handle_settled` est enregistrée sur un
`workspace_watch.WorkspaceWatcher` partagé via `register_extension(TEX_EXTENSIONS, ...)`
(voir `docs/workspace_watch.md`).

## Garde-fou anti-doublon

Le flattening signifie que deux sources différentes (n'importe où dans le workspace)
partageant le même nom de base (insensible à la casse) collisionneraient silencieusement
sur le même `build/dds/<stem>.dds`. Un `DuplicateNameGuard` (voir
`duplicate_name_guard.py`) suit l'ensemble des sources connues par stem ; une collision
route vers le hook `on_name_conflict(path_a, path_b)` au lieu de convertir l'un ou l'autre
fichier — popup de résolution côté `apps/object_editor_mixins/shape_io.py`
(`_draw_name_conflict_popup`, partagée avec `import_watcher`/`workspace_sync`). Vérifié à
la fois lors du `reconcile()` de démarrage (avant toute conversion pour une paire en
conflit) et à chaque création/renommage réglé. Quand un côté du conflit disparaît
(suppression, ou renommage), le survivant est converti automatiquement
(`DuplicateNameGuard.remove()` le renvoie).

## API principale

- `TEX_EXTENSIONS = {".tga", ".png"}` — même périmètre que
 `tga2dds.cpp`/`dds_export.py`, pas les extensions plus larges d'`explorer.py`.
- `TexDdsSyncWatcher(on_status=None, on_name_conflict=None)` :
  - `set_workspace_dir(workspace_dir)` — mémorise le workspace actif, crée `build/dds/`
 si absent, et lance `reconcile` sur un thread de fond.
  - `reconcile` — rattrapage complet : scanne tout le workspace, construit/vérifie
 l'index anti-doublon (signale toute collision avant de convertir quoi que ce soit pour
 une paire en conflit), régénère tout fichier conflict-free dont le `.dds` est manquant
 ou plus ancien (comparaison de mtime), supprime tout `.dds` orphelin dont la source a
 disparu ou est en conflit.
  - `handle_settled(tex_path)` — callback enregistrée sur le `WorkspaceWatcher` partagé :
 si le fichier existe encore et n'est pas en conflit, régénère son `.dds` ; sinon (fichier
 disparu), supprime le `.dds` correspondant et traite le survivant d'un éventuel conflit
 résolu.
  - `_convert(tex_path, dds_path)` / `_remove(dds_path)` — implémentation, avec
 `try/except` large autour de la conversion (`load_rgba`/`build_dds`) pour qu'une
 texture invalide ne tue jamais le watcher.

## Utilisation

Instancié dans `apps/object_editor.py` (`self.tex_dds_sync`), enregistré sur
`self.workspace_watch` via `register_extension(TEX_EXTENSIONS, ...)`, avec `on_name_conflict`
routé vers le popup partagé. `set_workspace_dir` appelé depuis
`_on_active_workspace_changed` (`object_editor.py`), aux côtés de
`workspace_watch.set_workspace_dir`.

## Points notables / pièges

- Décisions utilisateur (2026-08-27) : pas d'exclusion par nom de fichier pour le
 specular pour l'instant (à revoir si/quand Patina gère les slots de texture) ;
 rattrapage complet systématique à l'ouverture d'un workspace (comme
 `workspace_sync.sync_now`, mais génératif plutôt que copie).
- La comparaison mtime dans `reconcile` est simple (pas de hash de contenu) — un
 fichier touché sans changement réel de contenu déclenchera quand même une
 regénération.
- Décision utilisateur (2026-09-02) : la sortie flat dans `build/dds/` matche le format
 `.bnp` (`workspace_sync.pack_workspace_bnp` flatten déjà les mêmes 4 extensions avant
 empaquetage) — une collision de nom surgirait de toute façon au moment du pack, donc
 le miroir ne doit pas prétendre l'éviter en gardant des sous-dossiers qu'il n'aura plus
 une fois empaqueté.
