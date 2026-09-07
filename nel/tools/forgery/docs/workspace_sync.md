# workspace_sync

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspace_sync.py` (~155 lignes)

## Rôle

Synchronise en direct (copie de fichiers) certains sous-dossiers d'un workspace Forgery
vers un dossier externe choisi par l'utilisateur (ex : le dossier de données du jeu),
pour que les modifications faites dans Patina soient immédiatement visibles côté jeu
sans étape manuelle. Fonctionnalité livrée dans le commit `d530bcb8c` (Patina 1.0.0).

## API principale

- `SYNCED_SUBDIRS = ("anims", "shapes", "skels", "dds")` — sous-dossiers
 synchronisés. Volontairement **pas** `masks/`, `exports/`, `imports/` (masques Panoply
 = entrées, pas assets livrés ; `exports/` = zone de sortie du dialogue d'export ;
 `imports/` = zone de staging dont la sortie atterrit déjà dans `shapes/`, lui-même
 synchronisé) — **ni `tex/`** depuis le chantier `patina-tex-dds-autoexport`
 (2026-08-27) : c'est `dds/` (le miroir `.dds` généré de `tex/`, voir
 `docs/tex_dds_sync.md`) qui est ce dont le client a réellement besoin.
- `set_workspace_dir` — mémorise le workspace actif et crée les
 `SYNCED_SUBDIRS` manquants.
- `_iter_workspace_files` — parcourt tous les fichiers courants des
 `SYNCED_SUBDIRS`.
- `sync_now` — lance un thread qui copie tous les fichiers courants,
 pour rattraper ce qui préexistait avant le démarrage du watch.
- `refresh_fully_synced` — simple test d'existence (pas de comparaison
 de contenu/mtime) pour afficher ou non le bouton "Sync now" côté UI.
- `handle_settled` (ex-`_handle_settled`) — copie effective via
 `shutil.copy2`, vers `<sync_folder>/forgery/<nom du workspace>/<chemin relatif>` — le
 préfixe `forgery/<workspace>/` isole plusieurs workspaces dans le même dossier de sync
 sans collision.

## Utilisation

- `settings.py` : `workspace_sync_folders: Dict[str, str]` (un dossier de sync par
 nom de workspace) et `last_workspace_sync_folder` (valeur par défaut suggérée pour un
 nouveau workspace).
- `apps/object_editor.py` : UI dans Settings > Tools (appelle `set_workspace_dir` et
 `sync_now`), et enregistre `handle_settled` sur le `workspace_watch.WorkspaceWatcher`
 partagé pour chaque sous-dossier de `SYNCED_SUBDIRS`.

## Points notables / pièges

- **Copie unidirectionnelle et "copy-only"** : un fichier supprimé du
 workspace **reste tel quel** côté dossier de sync — aucune suppression miroir n'est
 faite. À garder en tête si un fichier de sync semble "fantôme".
- Pas de comparaison de contenu ou de date de modification pour `refresh_fully_synced`
 — c'est un simple test d'existence, donc ça peut afficher "tout synchronisé" alors
 qu'une version plus récente existe côté workspace sans avoir encore déclenché le
 watch.
- **Depuis le 2026-08-27 : ne possède plus son propre `Observer`.** `set_workspace_dir`
 ne fait plus que mémoriser l'état et créer les dossiers ; la surveillance filesystem
 elle-même (un seul `Observer` partagé avec `import_watcher`/`tex_dds_sync`) vit dans
 `workspace_watch.WorkspaceWatcher` — voir `docs/workspace_watch.md` pour le pourquoi
 de cette consolidation.
