# Dialogue de configuration des workspaces

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspace_setup_dialog.py`

## Rôle

UI ImGui pour configurer et utiliser le système de "workspace" (modèle de
données dans `workspaces.py`, non documenté ici) : la popup **obligatoire**
de première utilisation (dossier racine `settings.workspaces_root` + nom du
premier workspace -- aucun moyen de la fermer sans la compléter), le
sélecteur de workspace actif affiché dans le panneau principal (toujours un
workspace actif dès qu'il en existe un, pas d'état "(none)"), le popup de
création d'un nouveau workspace, et le contenu de l'onglet Settings pour
changer le dossier racine plus tard (workspace_setup_dialog.py).

## API principale

- `_icon_button(icon, tooltip)` (workspace_setup_dialog.py) — bouton
 icône avec tooltip au survol, utilitaire local.
- `_truncate_path_to_width(path, max_width)` (workspace_setup_dialog.py)
 — tronque un chemin par recherche binaire de la plus longue "queue" qui
 tient dans `max_width` pixels, préfixée de `...` ; même approche que la
 copie dans `search_paths_dialog.py` (mentionné explicitement en
 commentaire, workspace_setup_dialog.py).
- `class WorkspaceSetupDialog` (workspace_setup_dialog.py) :
  - `__init__` (workspace_setup_dialog.py) : charge les settings, prépare
 l'état du dialogue de sélection de dossier (`portable_file_dialogs`,
 polling non bloquant comme `export_dialog.py`), le champ en attente de la
 popup de setup (`_setup_workspace_name`/`_setup_error`, non écrits dans
 les settings avant "Finish"), le drapeau `_prompt_offered`, et l'état du
 popup de création de workspace. Expose `on_active_workspace_changed`, un
 callback que l'app hôte branche pour resynchroniser la résolution des
 chemins de recherche (`SearchPathsDialog.set_workspace_dir`).
  - `is_configured` (workspace_setup_dialog.py) — vrai si
 `workspaces_root` est configuré et existe (délègue à
 `workspaces.is_root_configured`). Ne dit rien à lui seul d'un workspace
 actif -- voir `_needs_setup` ci-dessous pour la condition complète.
  - `_needs_setup` (workspace_setup_dialog.py) — vrai tant que
 `workspaces_root` **et** un `active_workspace` ne sont pas tous les deux
 définis ; condition d'ouverture de la popup obligatoire (`draw`), plus
 stricte que `is_configured` puisque n'avoir aucun workspace actif n'est
 plus un état stable valide.
  - `active_workspace_name`/`workspace_names`/`active_workspace_dir`
 (workspace_setup_dialog.py) — accesseurs de commodité sur le
 modèle `workspaces.py`.
  - `set_active_workspace(name)` (workspace_setup_dialog.py) — change le
 workspace actif, sauvegarde, complète la structure de dossiers
 (`ensure_structure`, idempotent — utile si de nouveaux `SUBDIRS` ont été
 ajoutés depuis la création du workspace), puis notifie
 `on_active_workspace_changed`.
  - `_save` (workspace_setup_dialog.py) — recharge les settings à
 neuf et n'écrase que ses propres champs (`workspaces_root`,
 `active_workspace`), pour ne pas perdre des changements concurrents
 faits par un autre composant — même motif que `export_dialog.py`.
  - `draw` (workspace_setup_dialog.py) — à appeler une fois par frame
 depuis l'app hôte : poll le dialogue de dossier, auto-active le premier
 workspace trouvé si la racine est configurée mais qu'aucun n'est actif
 (auto-guérison, ex. `settings.toml` édité à la main), ouvre le popup de
 setup obligatoire au premier frame où `_needs_setup()` est vrai et pas
 déjà proposé, **appelle aussi `imgui.open_popup()` pour le popup de
 création de workspace** si `draw_active_workspace_row` en a fait la
 demande (voir Pièges ci-dessous pour pourquoi c'est fait ici et pas
 là-bas), puis dessine les deux popups.
  - `draw_active_workspace_row(width=160)` (workspace_setup_dialog.py)
 — ligne "Current workspace: [combo] [icône ouvrir dans le gestionnaire
 de fichiers]" dessinée dans le panneau de l'app hôte, au-dessus de la
 barre Save/Export/Quit. Le combo propose les workspaces existants et
 `<new>` (positionne `_new_workspace_pending`, consommé par `draw`) — plus
 d'option `(none)` : si le workspace actif enregistré n'existe plus sur
 disque, le premier workspace disponible est auto-activé à la place (même
 logique d'auto-guérison que `draw`) ; désactivé (avec tooltip explicatif)
 tant que la racine n'est pas configurée.
  - `_draw_new_workspace_popup` (workspace_setup_dialog.py) — saisie
 du nom, validation (non vide, pas de doublon), création via
 `workspaces.create_workspace`, puis application d'un défaut pratique :
 le nouveau workspace hérite du dernier
 `settings.last_workspace_sync_folder` utilisé (workspace_setup_dialog.py),
 simple point de départ éditable ensuite dans Settings > Tools.
  - `_draw_prompt_popup` (workspace_setup_dialog.py) — popup
 **obligatoire** de première utilisation, sans bouton "Later"/skip : le
 choix du dossier racine ("Choose folder...") et le nom du premier
 workspace (`input_text`) ; le bouton "Finish" ne s'active que quand
 racine + nom sont tous deux renseignés. Reste ouverte tant que
 `_needs_setup()` est vrai.
  - `_finish_setup` (workspace_setup_dialog.py) — validation du nom
 (pas de doublon), crée le workspace (`workspaces.create_workspace`),
 puis `set_active_workspace(name)` (sauvegarde `workspaces_root` +
 `active_workspace` en un seul `_save`).
  - `_poll_folder_dialog` (workspace_setup_dialog.py) — récupère le
 résultat du sélecteur de dossier non bloquant une fois prêt et
 sauvegarde ; ne ferme plus aucun popup lui-même (le dossier n'est qu'une
 des deux informations requises par la popup de setup, le nom du
 workspace doit encore être saisi avant "Finish").
  - `draw_settings_content` (workspace_setup_dialog.py) — bloc
 "Workspaces folder: <chemin tronqué> [icône dossier]" intégré dans
 l'onglet Settings de l'app hôte, même emplacement que
 `export_dialog.py`'s `draw_settings_content`.

## Utilisation

- `app.py` importe `WorkspaceSetupDialog`, instancie
 `self.workspace_setup_dialog = WorkspaceSetupDialog` (app.py), et
 appelle `.draw` (app.py) -- **une seule fois** par frame, depuis
 `draw_ui`, en dehors de toute fenêtre nommée. `apps/object_editor.py`
 appelait aussi `.draw()` une seconde fois depuis `draw_panel` (imbriqué
 dans la fenêtre du panneau) -- doublon retiré 2026-09-01, sans rapport
 direct avec le bug de `<new>` documenté ci-dessous mais latent depuis
 longtemps (deux `begin_popup_modal`/mêmes IDs de widgets soumis deux
 fois par frame).
- `apps/object_editor.py` instancie **une seconde** `WorkspaceSetupDialog`
 séparée, et y câble `on_active_workspace_changed` sur elle-même (pas sur
 celle de `app.py`) — à noter, deux instances distinctes chargent chacune
 leurs propres settings ; leur cohérence dépend entièrement du pattern
 reload-fresh de `_save`.
- `popup_utils.center_next_popup()` est appelé avant chaque
 `begin_popup_modal` de ce fichier, pour ouvrir les deux popups centrées
 sur le viewport plutôt qu'à une position ImGui par défaut.
- Dépend de `ryzom_forgery.settings` (chargement/sauvegarde) et de
 `ryzom_forgery.workspaces` (modèle de données : `active_workspace_path`,
 `create_workspace`, `ensure_structure`, `is_root_configured`,
 `list_workspaces`, `open_in_system_file_manager`, `workspace_path`).

## Points notables / pièges

- **`OpenPopup()`/`BeginPopupModal()` résolvent l'ID réel du popup à partir
 du contexte/fenêtre ImGui courant au moment de l'appel** -- pas juste de
 la chaîne passée en argument. `draw_active_workspace_row` s'exécute
 imbriquée dans la fenêtre "Explorer" (voir `object_editor.py`'s
 `Explorer.extra_header`), alors que `_draw_new_workspace_popup`'s
 `begin_popup_modal` est appelé depuis `draw`, au niveau racine (aucune
 fenêtre parente). Appeler `imgui.open_popup(_NEW_WORKSPACE_POPUP_ID)`
 directement dans `draw_active_workspace_row` calculait donc un ID
 différent de celui attendu par `begin_popup_modal` -- le popup ne
 s'ouvrait alors jamais (confirmé en test réel : `is_popup_open()`
 retournait `True` juste après l'appel, côté Explorer, mais restait
 `False` vu depuis `_draw_new_workspace_popup`). Fix (2026-09-01) :
 `draw_active_workspace_row` ne fait plus que positionner
 `_new_workspace_pending` ; l'appel réel à `open_popup()` a été déplacé
 dans `draw`, au même niveau que `begin_popup_modal` lui-même.
- Le popup de première utilisation est bloquant : il n'y a pas de bouton
 "Later" pour le reporter, `_needs_setup()` reste vrai (et donc le popup
 se rouvre) tant que racine + workspace actif ne sont pas tous les deux
 définis. Le dossier racine reste éditable depuis Settings à tout moment
 une fois passé ce premier setup (workspace_setup_dialog.py).
- Le message du popup affirme explicitement : "shapes are only ever
 saved into a workspace, never elsewhere" (workspace_setup_dialog.py)
 — contrainte de conception du système de workspace globalement, pas
 spécifique à ce fichier.
- `draw_active_workspace_row` et `draw` partagent la même logique
 d'auto-guérison (activer le premier workspace trouvé quand aucun n'est
 actif) en deux endroits légèrement différents (racine configurée sans
 workspace actif pour l'un, workspace actif enregistré mais absent du
 disque pour l'autre) plutôt qu'une fonction commune — à garder en tête
 si on retouche l'un des deux.
- Aucun TODO explicite trouvé dans le fichier.
