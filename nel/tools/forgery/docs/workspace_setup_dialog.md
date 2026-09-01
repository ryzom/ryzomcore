# Dialogue de configuration des projets/workspaces

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspace_setup_dialog.py`

## Rôle

UI ImGui pour configurer et utiliser le système de "projet"/"workspace"
(modèle de données dans `workspaces.py`, voir `docs/workspaces.md`) :
la popup **obligatoire** de première utilisation ou de migration, les deux
combos Projet/Workspace affichés dans le panneau principal, les popups de
création de projet/workspace, et le contenu de l'onglet Settings pour
changer le dossier racine plus tard.

## API principale

- `_icon_button(icon, tooltip)` / `_truncate_path_to_width(path, max_width)`
 (workspace_setup_dialog.py) — utilitaires locaux inchangés (bouton icône
 avec tooltip ; troncature de chemin par recherche binaire, même approche
 que `search_paths_dialog.py`'s propre copie).
- `class WorkspaceSetupDialog` (workspace_setup_dialog.py) :
  - `__init__` — charge les settings, prépare l'état des deux dialogues de
 sélection de dossier (`_folder_dialog` pour le dossier racine,
 `_import_folder_dialog` pour `<Import Folder>`), les champs en attente
 des popups de setup/création (`_setup_project_name`/`_setup_workspace_name`
 pré-remplis `"Ryzom"`/`"WIP"`, `_setup_dpi_scale`, `_new_project_name`,
 `_new_workspace_name`, non écrits dans les settings avant validation).
 Expose `on_active_workspace_changed`, `on_dpi_preview_changed` (voir
 `docs/app.md`'s `set_live_ui_scale_preview`), et `on_setup_finished`
 (l'app hôte y branche `ForgeryApp.relaunch` dans les trois cas).
  - `is_configured` — vrai si `workspaces_root` est configuré et existe.
  - `_needs_migration` — vrai pour une config d'avant les projets :
 `workspaces_root`/`active_workspace` déjà définis, mais `active_project`
 encore `None` (le marqueur, voir `docs/settings.md`). Vérifié **avant**
 `_needs_setup` dans `draw`, sinon le formulaire complet (faux) s'afficherait
 à la place du formulaire de migration (juste le nom du projet).
  - `_needs_setup` — vrai tant que dossier racine, projet actif et
 workspace actif ne sont pas tous les trois définis ; condition
 d'ouverture de la popup obligatoire pour un tout premier lancement.
  - `active_project_name`/`active_workspace_name`/`project_names`/
 `workspace_names`/`active_workspace_dir` — accesseurs de commodité.
 `workspace_names()` combine les workspaces internes (`list_workspaces`)
 et externes (`list_external_workspaces`) du projet **actif**, `[]` si
 aucun projet actif.
  - `is_active_workspace_external` — vrai si le workspace actif est
 enregistré dans le manifeste externe de son projet plutôt qu'un vrai
 sous-dossier -- utilisé par `apps/object_editor.py`'s
 `_on_active_workspace_changed` et par `set_active_workspace` ci-dessous
 pour ne **jamais** restructurer/backfiller un dossier externe.
  - `set_active_project(name)` — change le projet actif, sauvegarde,
 **réinitialise `active_workspace` à `None`** (un nom de l'ancien projet
 n'a aucun sens dans le nouveau -- l'auto-guérison de `draw` en réélit un
 dès la frame suivante s'il y en a), notifie `on_active_workspace_changed(None)`.
  - `set_active_workspace(name)` — change le workspace actif, sauvegarde,
 complète la structure de dossiers (`ensure_structure`, idempotent) **sauf
 si `is_active_workspace_external()`**, puis notifie
 `on_active_workspace_changed`.
  - `_save` — recharge les settings à neuf et n'écrase que ses propres
 champs (`workspaces_root`, `active_project`, `active_workspace`), pour ne
 pas perdre des changements concurrents — même motif que
 `export_dialog.py`.
  - `draw` — à appeler une fois par frame depuis l'app hôte : poll les
 deux dialogues de dossier, auto-guérison à deux niveaux (projet actif
 manquant → premier projet trouvé ; workspace actif manquant → premier
 workspace du projet actif), ouvre la popup obligatoire (setup ou
 migration selon le cas) au premier frame où c'est nécessaire, **appelle
 aussi `imgui.open_popup()`** pour les popups "New project"/"New workspace"
 si `_draw_project_row`/`_draw_workspace_row` en ont fait la demande (voir
 Pièges pour pourquoi c'est fait ici et pas là-bas), puis dessine les
 trois popups.
  - `draw_active_workspace_row(width=160)` — deux lignes empilées :
 `_draw_project_row` (combo Projet + `<new>`) puis `_draw_workspace_row`
 (combo Workspace + `<New>` + `<Import Folder>`, désactivé tant qu'aucun
 projet n'est actif, + icône ouvrir dans le gestionnaire de fichiers).
 Ni l'un ni l'autre combo n'offre plus d'option `(none)` -- auto-guérison
 vers le premier disponible à la place.
  - `_register_external_workspace(folder)` — appelée après le sélecteur de
 dossier natif d'`<Import Folder>` : enregistre `folder` dans le manifeste
 du projet actif (`workspaces.add_external_workspace`), nommé d'après le
 nom du dossier lui-même, dédupliqué par un suffixe numérique en cas de
 collision plutôt que de bloquer sur une saisie de nom séparée.
  - `_draw_new_project_popup` / `_draw_new_workspace_popup` — saisie du
 nom, validation (non vide, pas de doublon), création
 (`workspaces.create_project`/`create_workspace`) puis activation. Le
 nouveau workspace hérite en plus du dernier
 `settings.last_workspace_sync_folder` utilisé, comme avant ce chantier.
  - `_draw_prompt_popup` — popup **obligatoire**, sans bouton "Later"/skip,
 sous l'une de deux formes choisies par `_needs_migration()` :
    - `_draw_migration_form` : juste le nom du projet (dossier et
   workspace déjà connus) ; bouton "Migrate" → `_finish_migration`
   (`workspaces.migrate_legacy_workspaces`, préserve `active_workspace`
   tel quel puisque seul son emplacement physique change).
    - `_draw_full_setup_form` : échelle DPI (aperçu texte en direct, voir
   `docs/app.md`) + dossier racine + nom de projet + nom de premier
   workspace (tous deux pré-remplis `"Ryzom"`/`"WIP"`) ; bouton "Finish" →
   `_finish_setup` (crée projet + workspace, sauvegarde `dpi_scale`).
  - `_after_setup_finished` — code partagé en fin de `_finish_setup`/
 `_finish_migration` : ferme le popup, appelle `on_setup_finished` (le
 chemin normal, `relaunch()`, ne revient jamais à la frame suivante pour
 que la fermeture explicite compte, mais protège un hôte qui n'aurait pas
 câblé le callback).
  - `_poll_folder_dialog` / `_poll_import_folder_dialog` — récupèrent
 chacun leur sélecteur de dossier non bloquant une fois prêt ; le premier
 ne ferme plus aucun popup lui-même (le dossier n'est qu'une des
 informations requises par la popup de setup).
  - `draw_settings_content` — bloc "Workspaces folder: <chemin tronqué>
 [icône dossier]" intégré dans l'onglet Settings de l'app hôte, inchangé.

## Utilisation

- `app.py` importe `WorkspaceSetupDialog`, instancie
 `self.workspace_setup_dialog`, et appelle `.draw` (app.py) -- **une seule
 fois** par frame, depuis `draw_ui`, en dehors de toute fenêtre nommée.
 `apps/object_editor.py` appelait aussi `.draw()` une seconde fois depuis
 `draw_panel` (imbriqué dans la fenêtre du panneau) -- doublon retiré
 2026-09-01, latent depuis longtemps (deux `begin_popup_modal`/mêmes IDs
 de widgets soumis deux fois par frame).
- `apps/object_editor.py` instancie **une seconde** `WorkspaceSetupDialog`
 séparée, et y câble `on_active_workspace_changed`, `on_dpi_preview_changed`,
 `on_setup_finished` sur elle-même (pas sur celle de `app.py`) — à noter,
 deux instances distinctes chargent chacune leurs propres settings ; leur
 cohérence dépend entièrement du pattern reload-fresh de `_save`. Un oubli
 de câblage sur l'une des deux instances romprait silencieusement l'aperçu
 DPI en direct ou le redémarrage final pour cette app-là seulement.
- `popup_utils.center_next_popup()` est appelé avant chaque
 `begin_popup_modal` de ce fichier, pour ouvrir les trois popups centrées
 sur le viewport plutôt qu'à une position ImGui par défaut.
- Dépend de `ryzom_forgery.settings` (chargement/sauvegarde) et de
 `ryzom_forgery.workspaces` (modèle de données complet -- voir
 `docs/workspaces.md` pour la liste des fonctions).

## Points notables / pièges

- **`OpenPopup()`/`BeginPopupModal()` résolvent l'ID réel du popup à partir
 du contexte/fenêtre ImGui courant au moment de l'appel** -- pas juste de
 la chaîne passée en argument. `_draw_project_row`/`_draw_workspace_row`
 s'exécutent imbriquées dans la fenêtre "Explorer" (voir
 `object_editor.py`'s `Explorer.extra_header`), alors que
 `_draw_new_project_popup`/`_draw_new_workspace_popup`'s `begin_popup_modal`
 sont appelés depuis `draw`, au niveau racine (aucune fenêtre parente).
 Appeler `imgui.open_popup()` directement depuis les lignes de combo
 calculerait donc un ID différent de celui attendu par `begin_popup_modal`
 -- le popup ne s'ouvrirait alors jamais (bug réel confirmé en test avant
 ce fix, 2026-09-01, sur le popup "New workspace" : `is_popup_open()`
 retournait `True` juste après l'appel côté Explorer, mais restait `False`
 vu depuis le point d'appel de `begin_popup_modal`). D'où le motif
 "positionner un drapeau ici, ouvrir réellement dans `draw`" répété pour
 les deux popups de création.
- Le popup de première utilisation/migration est bloquant : pas de bouton
 "Later" pour le reporter, `_needs_migration()`/`_needs_setup()` restent
 vrais (et donc le popup se rouvre) tant que les informations requises ne
 sont pas toutes réunies. Le dossier racine reste éditable depuis Settings
 à tout moment une fois passé ce premier setup.
- Le message du popup complet affirme explicitement : "shapes are only
 ever saved into a workspace, never elsewhere" — contrainte de conception
 du système de workspace globalement, pas spécifique à ce fichier.
- `draw`, `_draw_project_row` et `_draw_workspace_row` répètent chacun une
 variante de la même logique d'auto-guérison (activer le premier trouvé
 quand rien n'est actif) plutôt qu'une fonction commune -- à garder en
 tête si on retouche l'un des trois.
- Aucun TODO explicite trouvé dans le fichier.
