# tex_dds_sync

**Fichier :** `nel/tools/forgery/ryzom_forgery/tex_dds_sync.py` (~140 lignes, créé le
2026-08-27)

## Rôle

Maintient le sous-dossier `dds/` d'un workspace automatiquement synchronisé avec `tex/` :
chaque source TGA/PNG dans `tex/` obtient un `.dds` correspondant dans `dds/`, régénéré à
chaque création/modification, et supprimé à chaque suppression de la source — chantier
`patina-tex-dds-autoexport`
(`/repos/project-todos/ryzom-core/patina-tex-dds-autoexport.md`).

Les textures specular sont hors scope pour l'instant : Patina ne gère pas encore les
rôles/slots de texture, donc `tex/` ne contient en pratique jamais de fichier
identifiable comme specular. Tout fichier supporté de `tex/` est converti uniformément,
y compris les variantes Multi Bitmap (saison/qualité) — c'est un miroir par fichier, pas
conscient de quel shape/matériau utilise réellement une texture donnée.

L'export DDS construit toujours les mipmaps (`build_mipmaps=True`) — ce sont toujours
des textures de modèles 3D (voir `docs/dds_export.md` sur pourquoi un asset régénéré
sans mipmap est visiblement dégradé).

Ne possède pas son propre watch filesystem — `handle_settled` est enregistrée sur un
`workspace_watch.WorkspaceWatcher` partagé pour le sous-dossier `"tex"` (voir
`docs/workspace_watch.md` pour le pourquoi de cette consolidation).

## API principale

- `_SUPPORTED_EXTENSIONS = {".tga", ".png"}` — même périmètre que
 `tga2dds.cpp`/`dds_export.py`, pas les extensions plus larges d'`explorer.py`.
- `TexDdsSyncWatcher(on_status=None)` :
  - `set_workspace_dir(workspace_dir)` — mémorise le workspace actif, crée `tex/`/`dds/`
 si absents, et lance `reconcile` sur un thread de fond.
  - `reconcile` — rattrapage complet : régénère tout `tex/` dont le `.dds` est manquant
 ou plus ancien (comparaison de mtime), supprime tout `.dds` orphelin dont la source
 `tex/` a disparu.
  - `handle_settled(tex_path)` — callback enregistrée sur le `WorkspaceWatcher` partagé :
 si le fichier existe encore, régénère son `.dds` ; sinon, supprime le `.dds`
 correspondant.
  - `_convert(tex_path, dds_path)` / `_remove(dds_path)` — implémentation, avec
 `try/except` large autour de la conversion (`load_rgba`/`build_dds`) pour qu'une
 texture invalide ne tue jamais le watcher.

## Utilisation

Instancié dans `apps/object_editor.py` (`self.tex_dds_sync`), enregistré sur
`self.workspace_watch` pour `"tex"`. `set_workspace_dir` appelé depuis
`_on_active_workspace_changed` (`object_editor.py`), aux côtés de
`workspace_watch.set_workspace_dir`.

## Points notables / pièges

- Décisions utilisateur (2026-08-27) : pas d'exclusion par nom de fichier pour le
 specular pour l'instant (à revoir si/quand Patina gère les slots de texture) ;
 rattrapage complet systématique à l'ouverture d'un workspace (comme
 `workspace_sync.sync_now`, mais génératif plutôt que copie).
- La comparaison mtime dans `reconcile` est simple (pas de hash de contenu) — un
 fichier `tex/` touché sans changement réel de contenu déclenchera quand même une
 regénération.
