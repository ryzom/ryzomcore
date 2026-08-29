# Patina (object_editor.py)

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/object_editor.py` (~3861 lignes)

## Rôle

Patina est l'application principale de Ryzom Forgery : un éditeur/visualiseur de fichiers `.shape` NeL/Ryzom, construit sur Panda3D (rendu 3D) + ImGui (interface). `APP_INFO` (object_editor.py) la déclare avec l'id `"object_editor"`, le nom `"Patina"`, le sous-titre `"Object Editor"`.

Elle permet de :
- Parcourir l'arborescence de fichiers via l'Explorer hérité de `ForgeryApp` (filtre par défaut `*.shape`, object_editor.py).
- Charger et visualiser un `.shape` en 3D — le rendu ne couvre que `CMesh`, `CMeshMRM` (LOD le plus fin) et `CMeshMultiLod` (slot 0), voir la docstring de module lignes 1-7 ; les autres types de shapes (squelette, eau, flare, particules...) n'affichent que leurs propriétés (onglet "All Properties"), sans rendu 3D.
- Inspecter/éditer les matériaux : textures simples, Multi Bitmap (variantes qualité/écosystème/saison), transparence (Blend/Alpha Test/Opacité), couleur diffuse, overrides de couleur visuels.
- Intégrer le pipeline Panoply (recoloration peau/couleur utilisateur/cheveux/yeux) en lecture seule sur le rendu.
- Synchroniser les textures modifiées sur disque en direct (détection de fraîcheur, recalcul Panoply à la volée).
- Importer des mesh externes (.obj/.dae/.fbx) via `ImportDialog`, avec un flux d'auto-export watché (`ImportWatcher`) et un flux de résolution de conflit si le shape cible est déjà ouvert.
- Prévisualiser une animation squelettique (Skinning preview) et le vent (WindTreeParams) sur les shapes qui en disposent.
- Gérer un système de "workspace" actif (voir `docs/workspaces.md`), avec synchronisation externe (`docs/workspace_sync.md`).

Point d'entrée : `main(argv=None)` (object_editor.py) instancie `ObjectEditorApp` et appelle `.run` (hérité de `ForgeryApp`) ; le bloc `if __name__ == "__main__"` (object_editor.py) permet de lancer le fichier directement.

## Architecture générale

### Classe principale

`class ObjectEditorApp(ForgeryApp)` (object_editor.py) est la seule classe applicative du fichier. Son `__init__` (object_editor.py) :

1. Détermine la racine initiale de l'Explorer depuis les search paths configurés, ou `Path.home` en fallback (object_editor.py).
2. Construit l'arbre de nœuds Panda3D : `_object_pivot` (pivot manipulé par Ctrl+drag) → `model_root` (racine de la géométrie du shape courant, détruite/recréée à chaque chargement/remplacement) — object_editor.py.
3. Initialise l'état des helpers de viewport (grille, axes monde/pivot, transparence) — object_editor.py.
4. Initialise tout l'état "shape courant" à `None`/vide : `shape_file`, `shape_error`, caches de textures, matériaux, sélection Panoply, etc. (object_editor.py).
5. Instancie les composants externes assemblés ici : `OrbitCamera`, `ObjectManipulator`, `NavigationCube` (object_editor.py), puis `ExportDialog`, `ImportDialog`, `SearchPathsDialog`, `WorkspaceSetupDialog`, `ImportWatcher`, `WorkspaceSyncWatcher` (object_editor.py).
6. Enregistre les tâches Panda3D (`taskMgr.add`) pour le vent, la fraîcheur des textures, le temps de preview d'animation et le re-skinning (object_editor.py, 735, 743).
7. Enregistre les commandes contextuelles de l'Explorer pour `.shape`/`.skel`/`.anim` (object_editor.py).
8. Termine par `_restore_session_state` (object_editor.py) qui recharge le dernier dossier/shape de la session précédente — appelé en dernier, une fois tout le reste de l'état initialisé.

### Boucle de mise à jour (par frame)

`draw_panel` (object_editor.py) est le point d'entrée ImGui appelé chaque frame par `ForgeryApp`. Il :
- Dessine les contrôles flottants du viewport (nav cube, panneau de transform, toggles, vent, skinning preview, toggles de shapes de référence) — object_editor.py.
- Dessine/poll les dialogues (`export_dialog`, `import_dialog`, `search_paths_dialog`, `workspace_setup_dialog`) et les file pickers en attente (squelette, animation, éditeur d'image, dossier de sync) — object_editor.py.
- Dessine les popups modales spécifiques à Patina (match de matériaux au remplacement, scan en cours après restauration de session, conflit d'import) et vide la file de statuts en attente de l'`ImportWatcher` — object_editor.py.
- Affiche l'onglet actif (Textures / Materials / All Properties / Settings) — object_editor.py — et la barre du bas (Save/Export/Quit) — object_editor.py.

Trois tâches Panda3D tournent en continu indépendamment du dessin ImGui :
- `_update_wind` (object_editor.py) : anime les vertices d'un shape avec `WindTreeParams`, port de `meshvp_wind_tree.cpp`.
- `_update_skin_preview_time` (object_editor.py) et `_update_skin_preview` (object_editor.py) : avancent le temps d'animation et re-skinnent en direct un `CMeshMRMSkinned`.
- `_update_texture_freshness` (object_editor.py) : revérifie une fois par seconde si des textures (Panoply ou non) ont changé sur disque dans le workspace actif, et force un réaffichage des matériaux si besoin.

## Fonctionnalités principales

### Ouverture / affichage d'un shape

- `_load_shape(item)` (object_editor.py) : parse un `.shape` depuis un `ExplorerItem` (fichier ou entrée de `.bnp`), remplit `_shape_source_path`/`_shape_source_bnp_path`/`_shape_source_name`/`_texture_search_dirs`, puis appelle `_display_shape`.
- `_display_shape(shape_file)` (object_editor.py) : affecte `self.shape_file`, applique la rotation par défaut du shape (`CMeshBase::DefaultRotQuat`) au pivot (object_editor.py), appelle `_rebuild_geometry` puis `_auto_select_multi_bitmap_slot`.
- `_reset_shape_state` (object_editor.py) : réinitialise tout l'état d'édition avant d'afficher un nouveau shape (matériaux, expansions UI, caches de textures...).
- `_rebuild_geometry` (object_editor.py) : cœur du rendu — reconstruit `model_root`, itère `iter_render_passes` (shape_geometry.py, documenté ailleurs) pour construire un `GeomNode` par passe, applique les matériaux (`_apply_material`), construit l'état vent/skin si applicable, recadre la caméra sur la bbox, reconstruit les helpers de viewport et les shapes de référence.
- Commandes contextuelles Explorer : `.shape` → "Load in viewer" (`_on_load_command`, object_editor.py), `.skel`/`.anim` → chargement en tant que squelette/animation de preview de skinning (object_editor.py). Depuis `on_selection_changed` (object_editor.py), une simple sélection ne charge plus rien automatiquement — seul le clic droit "Load..." le fait.

### Rendu 3D et matériaux

- `_apply_material_common` / `_apply_material_texture` / `_apply_material` (object_editor.py) : appliquent respectivement l'état commun (two-sided, depth-write selon les flags CMaterial), la texture+blend+alpha-test, puis l'ensemble complet (y compris l'override de couleur manuel). Constantes de flags `CMaterial` reprises de `material.h` : `_IDRV_MAT_ZWRITE`, `_IDRV_MAT_BLEND`, `_IDRV_MAT_DOUBLE_SIDED`, `_IDRV_MAT_ALPHA_TEST` (object_editor.py).
- Note de correction documentée dans le code : BLEND et ALPHA_TEST sont deux états indépendants côté moteur (`driver_opengl_material.cpp`), c'était auparavant un `if`/`elif` ici qui perdait silencieusement l'alpha test dès que le blend était actif également — corrigé (object_editor.py).
- `_reapply_material` / `_reapply_all_materials` (object_editor.py) : réappliquent un ou tous les matériaux sur les `NodePath` déjà construits, sans reconstruire toute la géométrie — utilisé après édition ou changement Panoply.
- Override de couleur manuel (visualisation, jamais sauvegardé) : `_draw_material_color_button`/`_set_material_override_color` (object_editor.py) — désactive texture/material/light pour montrer une couleur plate par matériau.
- Édition réelle de la couleur diffuse (sauvegardée) pour un matériau sans texture : `_draw_texture_color_button`/`_set_material_diffuse_color` (object_editor.py).
- Vent (`_update_wind`, `_build_wind_state`, `_WindState`) : object_editor.py, 1588-1698 — décodage du canal `PrimaryColor` en poids/branches de phase, exactement comme `wind_tree_vp.glsl`.
- Re-skinning de `CMeshMRMSkinned` (`_SkinState`, `_build_skin_state`, `_update_skin_preview`) : object_editor.py, 1028-1069 — vectorisé numpy, réutilise le squelette/l'animation choisis dans le panneau "Skinning preview".
- Panneau flottant "Skinning preview" (`_draw_bone_preview_controls`, object_editor.py) : choix squelette/animation (scannés via `search_paths_dialog`, mis en évidence en vert si compatibles), lecture/pause, slider de temps.
- Shapes de référence pour échelle (cube 1×1×1, plus petit/plus grand personnage) : constantes `_REFERENCE_EXAMPLES_DIR`/`_REFERENCE_SHAPES` (object_editor.py), logique dans `_get_reference_shape`/`_toggle_reference_shape`/`_rebuild_reference_shapes` (object_editor.py) et barre de toggles `_draw_reference_shapes_toggles` (object_editor.py).
- Helpers de viewport (grille au sol, axes monde/pivot, transparence objet) : `_rebuild_viewport_helpers` (object_editor.py), toggles associés (object_editor.py), dessinés par `_draw_viewport_toggles` (object_editor.py).
- Panneau Position/Rotation/Scale (`_draw_transform_panel` et méthodes associées, object_editor.py) : verrouillage par axe et par "pivot" (édite `model_root` local au lieu de `_object_pivot`), reset par ligne ou global (bouton Home du nav cube via `reset_object_transform`, object_editor.py). Champs X/Y/Z en `imgui.drag_float` (`_TRANSFORM_DRAG_PARAMS`, object_editor.py) — glisser pour ajuster, double-clic/Ctrl+clic pour taper une valeur exacte.

### Édition des matériaux — Textures / Materials tabs

- `_draw_textures_tab` (object_editor.py) : section Panoply globale, puis liste des matériaux "simple bitmap" (`_draw_simple_material_row`, object_editor.py), puis l'éditeur Multi Bitmap.
- `_draw_materials_tab` (object_editor.py) : une ligne par matériau avec bouton couleur/texture, badge de type (Multi Bitmap / Simple bitmap / Color), case Double-sided, boutons de conversion, section dépliable "Transparency" (`_draw_material_transparency_section`, object_editor.py) couvrant Blend (avec presets Alpha/Additive), Alpha Test (seuil), Opacité (alpha diffuse — masqué si ni Blend ni Alpha Test actif, car sans effet visible sinon).
- Section dépliable "Texture filtering" (`_draw_material_texture_filtering_section`, object_editor.py) : Wrap S/T, Mag/Min Filter et la case "grayscale = alpha mask" du slot 0 (`Texture.wrap_s`/`wrap_t`/`mag_filter`/`min_filter`/`load_grayscale_as_alpha`, voir `pynel`). Toute édition vide `self._texture_cache` avant de réappliquer le matériau (`load_panda_texture()` ne relit ces réglages qu'au premier décodage d'un nom donné, pas sur un cache hit). Un import frais (sans vraie valeur de `wrap_s`/`wrap_t`) retombe sur l'heuristique existante `_uvs_need_repeat()` jusqu'à ce que l'utilisateur choisisse explicitement une valeur.
- Section dépliable "Texture offset/tiling/rotation" (`_draw_material_texture_transform_section`, object_editor.py) : édite `Material.tex_user_mat[0]` via `decompose_uv_matrix`/`compose_uv_matrix` (shape_geometry.py), qui traduisent la matrice générique NeL en (offset U/V, échelle U/V, rotation) en supposant l'absence de cisaillement — vrai pour tout usage réel connu (rollout UV Offset/Tiling/Angle de 3dsMax). Reflété dans le viewport 3D via `NodePath.set_tex_transform()` (`_apply_material_texture`, object_editor.py) et `shape_geometry.py`'s `uv_matrix_to_panda_mat4()` — Offset U/V, l'inversion de l'axe V et le sens de la Rotation sont confirmés corrects par test contre le vrai client (2026-08-28). Scale U/V et Rotation sont mutuellement exclusifs dans l'UI (grisés tant que l'autre n'est pas à sa valeur neutre) : leur combinaison produit un artefact de répétition GPU (voir `docs/material_options.md`'s "Matrice de texture exportée"), et n'est utilisée par aucun shape réel du jeu (0/2600 shapes scannés dans `ryzom_live/data`).
- Conversions de type de matériau : `_convert_to_multi_bitmap` (texture simple → `CTextureMultiFile` à un seul slot rempli), `_convert_multi_bitmap_to_simple` (seul le slot 0/Low Quality est rempli), `_convert_multi_bitmap_to_color` (aucun slot rempli) — object_editor.py.
- Indices de bulles d'aide contextuelles tirées de `material_docs.py`/`docs/material_options.md`, affichées dans la barre de statut au survol (`_doc_hint_if_hovered`, object_editor.py ; mécanisme similaire pour le Multi Bitmap, object_editor.py).

### Multi Bitmap

- `_MULTI_BITMAP_SLOT_LABELS` (object_editor.py) : mapping index de slot → (qualité, écosystème, saison), les trois conventions Georges/moteur connues (elles ne sont pas mutuellement exclusives, toutes sont affichées).
- `_multi_bitmap_entries` (object_editor.py) : liste chaque matériau dont le slot 0 est un `CTextureMultiFile` — seul le slot 0 est actuellement rendu (`_apply_material` ne regarde que `textures[0]`).
- `_auto_select_multi_bitmap_slot` (object_editor.py) : si l'index sélectionné stocké dans le shape ne résout à rien pour aucun matériau (cas fréquent, ex. `fo_carnitree.shape`), bascule automatiquement sur le premier slot rempli — le vrai client choisit dynamiquement selon qualité/écosystème/saison, il n'y a donc pas de "bon" fallback unique.
- `_select_multi_bitmap_slot(entries, index)` (object_editor.py) : bascule tous les matériaux Multi Bitmap du shape sur le même index en une fois (un choix d'apparence global, pas par matériau).
- `_draw_multi_bitmap_editor` (object_editor.py) : une ligne par slot (bouton Select, chevron d'expansion, libellé, aperçu du slot 0/représentatif) ; en mode déplié, une ligne par matériau avec combo de texture, bouton parcourir, bouton copier vers le workspace.

### Intégration Panoply

- `_draw_global_panoply_section` (object_editor.py) : sélection globale par axe (skin/user/hair/eyes), affichée en haut des onglets Textures et Materials. Le rendu de la sélection est un pur override — ne modifie jamais les données du shape (`texture.file_name` reste inchangé). Règle particulière : le premier clic sur une sélection vide pré-remplit aussi les autres axes disponibles avec leur première valeur, car aucun fichier réel n'existe pour un seul axe isolé (ex. `..._FY.tga` n'existe pas, seul `..._FY_U1.tga` existe). Affiche à côté de "Panoply:" (2026-08-29) : un bouton engrenage (`_copy_panoply_cfg_to_workspace`, voir plus bas) tant qu'aucun `panoply.cfg` n'existe dans le workspace actif, remplacé par un bouton crayon une fois qu'il existe (lance l'éditeur texte configuré dans Settings > Tools, `settings.text_editor_path`, désactivé tant qu'aucun n'est configuré) ; puis un bouton feu (`_bake_panoply_real_all`) qui bake **toutes** les textures du shape ayant au moins un masque résolu — pendant que le bouton feu par-texture (voir `_draw_panoply_masks_for` ci-dessous) reste disponible pour bake une seule texture.
- `_resolve_panoply_texture_name` / `_panoply_dims_for` / `_resolve_panoply_refs` (object_editor.py) : résolvent le nom de texture réellement chargé compte tenu de la sélection Panoply courante, sans jamais toucher au disque.
- `_ensure_live_panoply_texture` (object_editor.py) : si le fichier "baked" attendu est manquant/périmé (`panoply_live.is_baked_stale`), recolore la texture de base en mémoire (`panoply_colorize`, paramètres `panoply_config`) et l'insère directement dans `_texture_cache` sous le nom résolu — tout appelant de `load_panda_texture` la récupère alors de façon transparente. **Approximation rapide** (voir `docs/panoply_maker.md`) — pour un bake réel, voir `_bake_panoply_real` ci-dessous.
- Masques Panoply : `_draw_panoply_masks_for` (object_editor.py) affiche les vignettes des masques déjà présents pour un texture donné, et `_create_panoply_mask` (object_editor.py) permet de créer un masque noir (poids nul) vide directement dans `masks/` du workspace actif pour un axe manquant. Affiche aussi le bouton "Bake real Panoply variants" (icône feu) quand au moins un masque est résolu.
- `_bake_panoply_real` (object_editor.py) : bake **réel** d'une texture, port exact `panoply_maker.cpp` (`panoply_bake.py`, voir `docs/panoply_bake.md`) — distinct de l'approximation live ci-dessus. Résout la race depuis le nom de la texture (`panoply_config.RACE_PREFIX_TO_TABLE`), construit les axes candidats (`panoply_bake.axes_for_source`), résout chaque masque via `search_paths_dialog.find_texture` (uniquement des fichiers réels sur disque, `FoundEntry.fs_path` — un fichier vivant seulement dans un `.bnp` ne peut pas être baked depuis ici), puis écrit chaque variante dans `tex/` et le `.hlsinfo`/`panoply_files.txt`/`characters.hlsbank` mis à jour dans `build/` du workspace actif via `panoply_bake.bake_and_write` (sources de départ : `characters.hlsbank`/`panoply_files.txt` réels sous `<ryzom-data>/final_bnps/characters_maps_hr/`, jamais écrasés — voir `docs/panoply_bake.md`). Ne fait plus aucune vérification `ryzom-data`/workspace lui-même (2026-08-29) — tourne sur le thread de fond de `_start_panoply_bake` (ci-dessous), où ni imgui ni `request_settings_attention` ne sont utilisables ; ces checks vivent désormais dans `_start_panoply_bake`, sur le thread principal, avant même le lancement du thread. Accepte un `on_variant` optionnel, transmis tel quel à `panoply_bake.bake_and_write`.
- `_bake_panoply_real_all` (object_editor.py, 2026-08-29) : construit la liste des textures du shape ayant au moins un masque résolu (`_panoply_texture_has_mask`, même vérification que `_draw_panoply_masks_for`) et appelle `_start_panoply_bake` — bouton feu à côté du gear/crayon dans `_draw_global_panoply_section`, complète le bouton feu par-texture (qui appelle aussi `_start_panoply_bake`, avec une liste à un seul élément).
- `_start_panoply_bake(texture_names)` / `_run_panoply_bake` / `_draw_bake_progress_popup` (object_editor.py, 2026-08-29) : le bake tourne sur un thread `threading.Thread` dédié (`daemon=True`) — un bake est lent (~1s/variante, vu lors de la cross-validation) et bloquerait toute l'UI sur le thread principal sinon. `_start_panoply_bake` fait les vérifications workspace actif/`ryzom-data` (imgui-safe, thread principal uniquement) puis initialise `self._bake_progress` (dict : `texture_index`/`texture_total`/`texture_name`/`variant_suffix`/`variant_index`/`variant_total`/`done`/`error`) et lance le thread ; refuse de démarrer un second bake tant qu'un autre tourne (`done` pas encore `True`). `_run_panoply_bake` (sur le thread) boucle `_bake_panoply_real` sur chaque texture, avec un callback `on_variant` qui met à jour `self._bake_progress` en place (écritures de champs simples, sûres sous le GIL, pas de verrou) — ne touche jamais à imgui. `_draw_bake_progress_popup` (thread principal, appelé chaque frame depuis `draw_ui`) lit `self._bake_progress` en lecture seule : popup modale (`_BAKE_PROGRESS_POPUP_ID`) avec texture/variante courantes et une barre de progression (`imgui.progress_bar`, fraction = position dans les textures + fraction de variante de la texture courante) ; bouton OK une fois `done` (ou en cas d'erreur) qui referme la popup et remet `self._bake_progress` à `None`.

### Settings-attention (mécanisme générique, 2026-08-29)

`ForgeryApp` (`app.py`, base partagée à toutes les apps Forgery) expose
`request_settings_attention(section, field_key, duration=3.0)` : au lieu de désactiver un
bouton avec un tooltip ou d'ouvrir une popup modale quand une action est bloquée par un
réglage manquant, saute vers l'onglet Settings, déplie la bonne section et fait clignoter
le champ concerné (bordure orange pulsée). Consommé par trois helpers, câblés dans le tab
Settings d'`object_editor.py` : `_consume_settings_tab_flags()` (force l'onglet
sélectionné), `_consume_settings_section_open(section)` (force le `collapsing_header`
déplié, appelé juste avant), `_begin_attention_flash(field_key)`/`_end_attention_flash()`
(entoure le champ ciblé). Utilisé par : bouton "Edit" texture (`image_editor_path`,
`_draw_texture_edit_button`), bouton "Edit" `panoply.cfg` (`text_editor_path`,
`_draw_global_panoply_section`), bake réel (`ryzom-data`, `_bake_panoply_real`).
- `_draw_repository_paths_settings` / `_poll_repository_paths_dialog` (object_editor.py) : Settings > Paths, un sélecteur de dossier par dépôt (`pynel.repository_paths.REPOSITORIES` -- ryzom-core/ryzom-data/ryzom-private-data/ryzom-docker), même motif que `workspace_setup_dialog`/`_draw_workspace_sync_settings`. Persisté par pynel (`repository_paths.json`), partagé avec tout autre outil basé sur pynel — voir `docs/repository_paths.md` de pynel.
- `_copy_panoply_cfg_to_workspace` (object_editor.py) : copie le `panoply.cfg` bundlé (`panoply_config.bundled_cfg_path()`) tel quel vers la racine du workspace actif (`panoply_config.workspace_cfg_path()`) — point de départ éditable ; une fois là, il prime systématiquement sur le bundlé (`panoply_config._resolve_cfg_path()`), pour la preview live comme pour le bake réel. Patina ne demande jamais explicitement à l'utilisateur de choisir/fabriquer un `.cfg` (voir `docs/panoply_config.md`).
- `_on_panoply_cfg_settled` (object_editor.py, ajouté 2026-08-29) : enregistré sur `self.workspace_watch` pour `"panoply.cfg"` (un fichier à la racine du workspace a un seul segment dans son chemin relatif, que `WorkspaceWatcher._dispatch()` traite comme sa propre clé "subdir", même mécanisme que les enregistrements `"tex"`/`"imports"` -- pas besoin d'un sous-dossier dédié). Tourne sur le thread background du watcher : se contente de positionner `self._panoply_cfg_changed = True`. Sans ça, éditer le `panoply.cfg` du workspace pendant que Patina tourne n'aurait **aucun effet visible** : `panoply_config.py` recharge bien son `.cfg` en interne (cache invalidé par mtime), mais ni `_texture_cache` ni `panoply_live.LiveColorizeCache` (dont la clé ne dépend pas des paramètres de couleur, seulement de `base_name`/axes/mtimes des sources) ne savent que quelque chose a changé -- confirmé en lisant `_panoply_freshness_signature`/`LiveColorizeCache.make_key`, aucun des deux ne touche au `.cfg`.
- `_update_texture_freshness` (object_editor.py) consomme ce flag au tick suivant (thread principal, ~1x/seconde) : évince tout `_texture_cache` marqué Panoply (`_panoply_texture_sources`), vide `_panoply_texture_signatures` et appelle `self._live_panoply_cache.clear()` (nouvelle méthode, `panoply_live.py`), puis force le réaffichage complet comme toute autre détection de changement dans cette même fonction.
- `_update_texture_freshness` (object_editor.py) revérifie chaque seconde les combinaisons Panoply en jeu (`_panoply_texture_sources`/`_panoply_texture_signatures`) et les textures simples du workspace, évince du cache et force un réaffichage complet si quelque chose a changé sur disque — c'est la seule mécanique de "reload" de Patina, il n'y a pas de bouton "Reload" manuel (voir `.todo/forgery-object-editor.md`, Phase A Step 5, citée dans le commentaire). Ne concerne que la preview live, pas le bake réel (déclenché à la main via le bouton feu).

### Import de mesh et remplacement de géométrie

- Bouton toolbar Explorer "Import mesh" (`_draw_import_toolbar_button`, object_editor.py) ouvre `ImportDialog`.
- `_on_import_new_shape(mesh, source_path)` (object_editor.py) : construit un nouveau shape `Mesh` à partir d'un mesh importé — seul `CMesh` peut être construit ex nihilo (voir `shape_import.py`).
- `_on_import_replace(mesh, source_path)` (object_editor.py) : remplace la géométrie d'un shape `Mesh` existant. Si le nombre de matériaux importés correspond exactement à l'existant, les indices sont supposés déjà alignés (`_replace_geometry(mesh, index_map=None)`). Sinon, ouvre `_draw_replace_match_popup` (object_editor.py) pour faire correspondre manuellement chaque matériau importé à un matériau existant ou l'ajouter comme nouveau.
- `_replace_geometry(mesh, index_map)` (object_editor.py) : ne touche qu'à `shape_file.value.geom` (vertex buffer, matrix blocks, bbox), laisse les matériaux (et leurs éditions) intacts.
- Watcher d'auto-export (`ImportWatcher`, `docs/import_watcher.md`) branché ici via des callbacks cross-thread :
  - `_is_shape_open_at` (object_editor.py) — lecture pure, appelée depuis le thread de fond.
  - `_on_open_shape_conflict`/`_on_import_status` (object_editor.py) — se contentent de mettre en file `_pending_import_conflict`/`_pending_import_status`, car le vrai travail (imgui/disque) doit se faire sur le thread principal.
  - `_draw_import_conflict_popup` (object_editor.py) — dessinée chaque frame depuis `draw_panel` : si le shape actuellement ouvert dans le viewport est aussi la cible d'un import auto-exporté et qu'il a des changements non sauvegardés en mémoire (`_has_unsaved_changes_at`, object_editor.py, comparaison sérialisation vs. disque), propose 4 choix (Save puis importer / Importer sans sauver / Sauver une copie / Annuler), chacun coloré selon sa "sécurité" vis-à-vis des éditions en cours (object_editor.py).
  - `_reload_shape_value_from_disk` (object_editor.py) : recharge la géométrie/matériaux depuis le fichier réécrit sans réinitialiser l'état d'édition ni re-seeder la rotation du pivot — même approche "perturbation minimale" que `_replace_geometry`.

### Sauvegarde / export

- `_workspace_shape_save_path` (object_editor.py) : la destination de Save est *toujours* `<workspace actif>/shapes/<nom>`, quelle que soit l'origine du shape (y compris depuis un `.bnp`, qui ne permettait pas de sauver avant).
- `_write_shape`/`_on_save_clicked`/`_draw_save_confirmation_popup` (object_editor.py) : confirmation d'écrasement demandée une seule fois par session (`_save_overwrite_confirmed`). Une sauvegarde redéfinit aussi la baseline de rotation pour Ctrl+Reset (object_editor.py).
- Seuls les types listés dans `_WRITABLE_SHAPE_TYPES = {"Mesh", "MeshMRM", "MeshMRMSkinned", "MeshMultiLod"}` (object_editor.py) affichent le bouton Save/Export — correspond à ce que `pynel.ryzom_shape.save_shape` sait réellement réécrire.
- Export (`_draw_bottom_bar`, object_editor.py) : menu de formats (`EXPORT_FORMATS`, shape_export.py) puis délégation à `self.export_dialog.export(...)` sur l'état en mémoire du shape (éditions incluses), pas une relecture depuis le disque/bnp.

### Copie de textures vers le workspace / édition externe

- `_copy_texture_to_workspace`/`_is_texture_in_workspace`/`_texture_copy_destination` (object_editor.py) : copie opt-in (jamais automatique) d'une texture résolue vers `tex/` (ou `masks/`) du workspace actif.
- `_draw_texture_copy_button` (object_editor.py) : bouton "Copier" tant que la texture n'est pas dans le workspace, devient un bouton "Edit" une fois copiée (`_draw_texture_edit_button`, object_editor.py) qui lance l'éditeur d'image externe configuré (`app_settings.image_editor_path`, réglé depuis Settings → Tools).
- Couleur verte (`_TEXTURE_IN_WORKSPACE_COLOR`) appliquée aux champs texte de référence de texture et bordure de vignette (`_draw_preview_workspace_border`) pour indiquer visuellement qu'un fichier est déjà dans le workspace.

### Workspace actif, watchers, Explorer

- `_on_active_workspace_changed(workspace_dir)` (object_editor.py) : point central qui propage un changement de workspace actif à `search_paths_dialog`, `import_watcher`, `workspace_sync`, `tex_dds_sync`, `workspace_watch`, et reconstruit `explorer.pinned_folders` (une entrée épinglée par sous-dossier de workspace, dont `dds/` depuis le chantier `patina-tex-dds-autoexport`).
- Le watch de `search_paths_dialog` couvre déjà tout le workspace ; `explorer.pinned_folders` (Wexplorer) réutilise ce même watch via `on_workspace_changed = self.explorer.refresh` (object_editor.py) plutôt que d'ouvrir un `Observer` de plus.
- **Depuis le 2026-08-27** : `import_watcher`, `workspace_sync` et `tex_dds_sync` (nouveau, conversion `tex/` → `dds/`, voir `docs/tex_dds_sync.md`) ne possèdent plus chacun leur propre `Observer` dédié -- ils partagent un unique `self.workspace_watch` (`ryzom_forgery.workspace_watch.WorkspaceWatcher`, voir `docs/workspace_watch.md`), sur lequel chacun enregistre sa callback `handle_settled` pour le(s) sous-dossier(s) qui le concernent (`"imports"`, `"tex"`, et `workspace_sync.SYNCED_SUBDIRS` = `anims`/`shapes`/`skels`/`dds`). Changement demandé par l'utilisateur : les `Observer` séparés n'apportaient aucun bénéfice réel (voir `docs/workspace_watch.md`), c'était de la duplication organique.
- Réglages Settings → Paths/Tools : `_draw_workspace_sync_settings` (object_editor.py, bouton "Sync now" si pas totalement synchronisé), `_draw_image_editor_settings` (object_editor.py), `_draw_text_editor_settings` (object_editor.py, 2026-08-29, même motif — utilisé par le bouton crayon du `panoply.cfg`, voir "Intégration Panoply"), `_draw_ui_font_settings` (object_editor.py, nécessite un redémarrage — `self.relaunch`).

### Session (restauration au démarrage)

- `_save_session_state`/`_restore_session_state` (object_editor.py) : persistent/restaurent le dossier Explorer courant, le `.bnp` ouvert, et le dernier shape chargé (via `settings.py`, best-effort, erreurs juste loggées).
- `_draw_restore_scan_popup` (object_editor.py) : si un shape est restauré avant la fin du scan initial des search paths (démarrage à froid, sans cache), affiche un popup "Scanning assets..." puis vide le cache de textures et réapplique tous les matériaux une fois le scan terminé — évite que les toutes premières résolutions de texture (avant que l'index existe) empoisonnent le cache avec des "introuvable" permanents.

## Utilisation (assemblage des autres modules)

Imports significatifs et où ils sont utilisés dans ce fichier :

| Module | Utilisé pour |
|---|---|
| `ryzom_forgery.app.ForgeryApp` | Classe de base : Explorer, panel, sysinfo, boucle Panda3D/ImGui (héritée par `ObjectEditorApp`). |
| `ryzom_forgery.camera.ObjectManipulator, OrbitCamera` | Manipulation caméra (orbit) et objet (Ctrl+drag) — object_editor.py. |
| `ryzom_forgery.explorer.ExplorerItem` | Reconstruction d'éléments Explorer pour la restauration de session (object_editor.py). |
| `ryzom_forgery.export_dialog.ExportDialog` | Dialogue et logique d'export — instancié object_editor.py, déclenché depuis `_draw_bottom_bar`. |
| `ryzom_forgery.import_dialog.ImportDialog` | Dialogue d'import de mesh — instancié avec callbacks `_on_import_new_shape`/`_on_import_replace` (object_editor.py). |
| `ryzom_forgery.import_watcher.ImportWatcher` | Auto-export `imports/` -> `.shape` ; `handle_settled` enregistrée sur `workspace_watch` pour `"imports"`. |
| `ryzom_forgery.tex_dds_sync.TexDdsSyncWatcher` | Auto-export `tex/` -> `.dds` (avec mipmaps) ; `handle_settled` enregistrée sur `workspace_watch` pour `"tex"`. |
| `ryzom_forgery.workspace_watch.WorkspaceWatcher` | `Observer` unique partagé, dispatch par sous-dossier vers `import_watcher`/`tex_dds_sync`/`workspace_sync`. |
| `ryzom_forgery.material_docs.load_material_docs` | Bulles d'aide contextuelles Multi Bitmap / matériaux (object_editor.py). |
| `ryzom_forgery.navcube.NavigationCube` | Gizmo de navigation/cible (objet vs caméra) — object_editor.py. |
| `ryzom_forgery.panoply`, `panoply_colorize`, `panoply_config`, `panoply_live`, `panoply_texture`, `panoply_bake` | Pipeline Panoply complet, preview live et bake réel (voir section dédiée ci-dessus). |
| `ryzom_forgery.properties.draw_properties` | Onglet "All Properties" (object_editor.py). |
| `ryzom_forgery.search_paths_dialog.SearchPathsDialog` | Résolution de textures/squelettes/animations/panoply à travers les chemins de recherche configurés. |
| `ryzom_forgery.settings` (`app_settings`) | Lecture/écriture des préférences persistantes (dossiers de session, police, éditeur d'image, dossiers de sync). |
| `ryzom_forgery.shape_export.EXPORT_FORMATS` | Liste des formats proposés au bouton Export. |
| `ryzom_forgery.shape_geometry` (`finest_skinned_lod`, `iter_render_passes`, `load_panda_texture`, `resolve_texture_ref`, `rgba_to_color`, `shape_bbox`, `shape_geom`, `solid_color_texture`) | Cœur de la conversion géométrie/texture NeL → Panda3D. |
| `ryzom_forgery.shape_import.texture_search_dirs_for` | Dossiers de recherche de texture additionnels pour un mesh importé. |
| `ryzom_forgery.workspace_setup_dialog.WorkspaceSetupDialog, _truncate_path_to_width` | Gestion du workspace actif ; helper de troncature de chemin réutilisé pour l'affichage des chemins longs (éditeur d'image, dossier de sync). |
| `ryzom_forgery.workspace_sync.WorkspaceSyncWatcher, SYNCED_SUBDIRS` | Miroir `anims`/`shapes`/`skels`/`dds` vers un dossier externe. |
| `ryzom_forgery.workspaces` (`SUBDIRS`, `ensure_structure`, `reveal_in_system_file_manager`) | Structure de dossiers du workspace (dont `dds/` depuis 2026-08-27) ; révélation d'un fichier dans le gestionnaire système (vignettes de texture). |
| `pynel.ryzom_animation` | Parsing/évaluation d'animations `.anim` pour la preview de skinning. |
| `pynel.ryzom_shape` | Parsing/écriture des `.shape` (`parse_shape`, `save_shape`), types de données (`Rgba`, `ShapeFile`, `Texture`, `WindTreeParams`, `SkeletonShape`, `MeshMRMSkinned`). |
| `pynel.ryzom_skin.bone_skin_matrices_for_mesh` | Calcul des matrices de skinning par frame pour la preview. |
| `pynel.repository_paths` | Résolution des 4 dépôts Ryzom (ryzom-core/ryzom-data/ryzom-private-data/ryzom-docker), voir "Intégration Panoply" ci-dessus et Settings > Paths. |

## Points notables / pièges

- **Pas de suivi "dirty" au sens classique.** Il n'y a aucun flag "shape modifié" maintenu au fil des éditions. La seule détection d'un état "non sauvegardé" est `_has_unsaved_changes_at` (object_editor.py), qui *sérialise en mémoire* le shape courant et le compare octet-à-octet à ce qui est sur disque — coûteux en principe (un parse+sérialisation complet), mais n'est appelé que lors d'un conflit d'auto-import réel, jamais en continu. Cela signifie qu'aucun indicateur visuel "modifications non sauvegardées" n'existe ailleurs dans l'UI (pas d'astérisque sur le titre, pas d'avertissement à la fermeture ou en changeant de shape) — perdre des éditions en chargeant un autre shape sans sauver est silencieusement possible.
- **Seul le slot 0 des textures est rendu.** `_multi_bitmap_entries` et `_apply_material_texture` ne considèrent que `material.textures[0]` — un matériau multi-texture (au-delà du slot 0) n'a aucun support de rendu ni d'édition ici (object_editor.py, 2591).
- **Le panneau Multi Bitmap n'affiche/n'édite que jusqu'à `max(len(_MULTI_BITMAP_SLOT_LABELS), ...)` slots** — au-delà de 8 slots labellisés, un slot supplémentaire réel resterait affiché mais avec un libellé numérique brut (`_multi_bitmap_slot_label`, object_editor.py).
- **La sélection Panoply est un pur override de rendu, jamais persistée dans le shape ni dans les settings** — elle est réinitialisée à chaque `_reset_shape_state` (object_editor.py) ; changer de shape perd donc le choix de teinte/couleur.
- **Pas de bouton "Reload" manuel pour les textures** — remplacé entièrement par la détection automatique de fraîcheur (`_update_texture_freshness`, ~1×/s), une décision de design explicitement documentée en commentaire renvoyant à `.todo/forgery-object-editor.md` (Phase A Step 5).
- **Deux caches de recentrage de rotation (`_object_pivot_base_quat`)** : recalculé au chargement du shape (`_display_shape`) *et* à chaque sauvegarde (`_write_shape`) — un Ctrl+drag après Save ne revient donc plus à l'orientation d'origine du fichier via Ctrl+Reset, mais à l'orientation au moment du dernier Save.
- **`_replace_geometry` ne re-seed jamais le pivot** contrairement à `_display_shape` — un choix délibéré documenté en commentaire (object_editor.py) pour préserver la manipulation manuelle de l'utilisateur lors d'un remplacement de géométrie (import "Replace" ou auto-export watché).
- **Comportement `if`/`elif` corrigé pour Blend vs Alpha Test** — signalé en commentaire comme un bug historique (les deux flags sont indépendants côté moteur), désormais deux `if` distincts (object_editor.py). Bon exemple de dette technique déjà résolue mais dont la trace reste dans les commentaires.
- **`_draw_replace_match_popup` ne peut pas être ouverte depuis l'intérieur du popup `ImportDialog` lui-même** — `imgui.open_popup` imbriqué échoue silencieusement dans ImGui ; contournement documenté : l'ouverture est différée d'une frame, faite depuis le niveau racine de `draw_panel` (object_editor.py).
- **Deux flux de rechargement distincts pour un import mesh** : "nouveau shape" (`_on_import_new_shape`, ne fonctionne que pour créer un `Mesh` neuf) et "remplacement" (`_on_import_replace`, uniquement si le shape courant est déjà de type `Mesh` — un `MeshMRM`/`MeshMultiLod` refuse le remplacement de géométrie avec un message d'erreur, object_editor.py).
- **`_texture_cache`/`_panoply_texture_sources`/`_panoply_texture_signatures` sont keyés uniquement par nom de texture**, pas par shape — commentaire explicite (object_editor.py) sur le risque qu'un même nom résolve différemment selon les `_texture_search_dirs` de deux shapes distincts ; c'est pourquoi ces caches sont entièrement vidés à chaque `_reset_shape_state`.
- **Impression console de debug systématique** à chaque reconstruction de géométrie (`print(f"[object_editor] pass {pass_count}: ...")`, object_editor.py) — bruit non conditionnel, présent en usage normal, pas seulement en debug.
- **Le vent et le skinning sont mutuellement exclusifs par construction du moteur** : un commentaire précise que `CMeshMRMSkinnedGeom` n'a pas du tout de champ `vertex_program`, donc aucune animation de vent n'est possible sur les personnages/créatures skinnés dans NeL (object_editor.py) — ce n'est pas une limitation de Patina mais du format lui-même.
