# Ryzom Forgery documentation index

## Apps (outils autonomes)

- `apps/object_editor.md` — Patina, l'éditeur/visualiseur de fichiers `.shape` principal.
- `apps/landscape_editor.md` — Atyscape, éditeur de composition de paysage 3D et pipeline de build complet.
- `apps/hairstyle_conform.md` — CLI de conformation d'une coiffure `.shape` entre races/genres.
- `apps/shape_exporter.md` — CLI de conversion d'un `.shape` vers un format d'échange 3D.
- `apps/shape_importer.md` — CLI de conversion d'un format d'échange 3D vers `.shape`.
- `apps/panoply_maker.md` — CLI de bake offline des variantes de couleur Panoply.
- `apps/ai_wmap_export.md` — CLI de régénération des cartes de marchabilité IA d'un continent.
- `apps/dds_export.md` — CLI de conversion TGA/PNG vers `.dds`.
- `apps/rebuild_shadow_skin.md` — CLI de reconstruction du `CShadowSkin` d'un `.shape` depuis un de ses LODs.

## App framework et infrastructure partagée

- `app.md` — `ForgeryApp`, la classe de base Panda3D+ImGui commune à toutes les apps.
- `cache_and_config_dir.md` — dossiers standards par utilisateur pour le cache et la configuration.
- `settings.md` — préférences persistantes unifiées dans un seul fichier TOML.
- `sysinfo.md` — barre de statut système (FPS, message de statut, version).
- `splash.md` — écran de démarrage affiché au lancement d'une app.
- `commands.md` — registre de commandes du menu contextuel de l'explorateur.
- `duplicate_name_guard.md` — détection de collision de nom entre les watchers du workspace.
- `error_log.md` — relais des erreurs affichées en UI vers stderr, pour qu'elles atteignent aussi le terminal.
- `crash_log.md` — journalisation des exceptions non catchées dans un fichier `crash.log`.

## Explorateur et recherche de fichiers

- `explorer.md` — navigateur de fichiers/archives `.bnp` utilisé par les apps.
- `search_paths.md` — moteur de résolution de fichiers par chemins de recherche configurés, et son UI.
- `virtual_categories.md` — classement virtuel des fichiers d'un workspace par catégorie.

## Vue 3D

- `camera.md` — contrôleurs de caméra orbitale et de manipulation d'objet.
- `navcube.md` — gizmo de navigation 3D reflétant l'orientation de la caméra.

## Workspaces et synchronisation

- `workspaces.md` — modèle de données projet/workspace.
- `workspace_setup_dialog.md` — UI de configuration et de sélection projet/workspace.
- `workspace_watch.md` — watcher filesystem générique partagé par les mécanismes de synchronisation.
- `workspace_sync.md` — synchronisation en direct des fichiers d'un workspace vers un dossier externe.
- `tex_dds_sync.md` — synchronisation automatique des sources TGA/PNG vers `.dds`.
- `import_watcher.md` — import/mise à jour automatique de meshes externes vers `.shape`.
- `pipeline_data_installer.md` — téléchargement et installation des données du pipeline `build_gamedata`.

## Import/export de shapes

- `shape_export.md` — conversion d'un `.shape` déjà chargé vers des formats d'échange 3D.
- `shape_import.md` — construction d'un `.shape` à partir d'un format d'échange 3D.
- `shape_geometry.md` — extraction géométrique/matérielle partagée entre l'éditeur et les exporteurs.
- `export_dialog.md` — UI ImGui du flux d'export `.shape`.
- `import_dialog.md` — UI ImGui du flux d'import vers `.shape`.
- `race_reference.md` — données de référence par race/genre pour la conformation de coiffure.

## Matériaux et Panoply (recoloration)

- `material_options.md` — explications, orientées joueur, de chaque réglage de matériau.
- `material_docs_loader.md` — extraction des textes de tooltip depuis `material_options.md`.
- `properties.md` — inspecteur générique en lecture seule pour n'importe quelle dataclass parsée.
- `dds_export.md` — écriture de fichiers `.dds` à partir d'un bitmap RGBA.
- `panoply_parsing.md` — reconnaissance des suffixes de couleur Panoply dans un nom de fichier.
- `panoply_config.md` — table de correspondance axe/couleur vers paramètres HSL réels.
- `panoply_colorize.md` — algorithme de recoloration Panoply, portage NumPy pur.
- `panoply_texture.md` — pont entre les tableaux NumPy de la recoloration et les `Texture` Panda3D.
- `panoply_live.md` — fraîcheur et mémoïsation de la recoloration Panoply en direct.
- `panoply_bake.md` — glue de bake offline des variantes Panoply, entre calcul pur et écriture disque.
- `panoply_maker.md` — portage Python de l'outil offline `panoply_maker.exe`.

## Pipeline paysage

- `ai_wmap_export.md` — régénération des cartes de marchabilité IA d'un continent (bibliothèque).

## Relevés de données (live_data)

- `anim_type_survey.md` — relevé des types `.anim` réellement rencontrés dans `live_data`.
- `shape_type_survey.md` — relevé des types `.shape` réellement rencontrés dans `live_data`.
