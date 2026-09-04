# Dialogue de configuration des workspaces

**Fichier :** `nel/tools/forgery/ryzom_forgery/workspace_setup_dialog.py` (~256 lignes)

## Rôle

UI ImGui pour configurer et utiliser le système de "workspace" (modèle de
données dans `workspaces.py`, non documenté ici) : le popup de première
utilisation qui invite à choisir le dossier racine `settings.workspaces_root`,
le sélecteur de workspace actif affiché dans le panneau principal, le popup
de création d'un nouveau workspace, et le contenu de l'onglet Settings pour
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
 polling non bloquant comme `export_dialog.py`), les drapeaux du popup
 d'invite (`_prompt_dismissed`/`_prompt_offered`) et du popup de création
 de workspace. Expose `on_active_workspace_changed`, un callback que
 l'app hôte branche pour resynchroniser la résolution des chemins de
 recherche (`SearchPathsDialog.set_workspace_dir`).
  - `is_configured` (workspace_setup_dialog.py) — vrai si
 `workspaces_root` est configuré et existe (délègue à
 `workspaces.is_root_configured`).
  - `active_workspace_name`/`workspace_names`/`active_workspace_dir`
 (workspace_setup_dialog.py, 77, 82) — accesseurs de commodité sur le
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
 depuis l'app hôte : poll le dialogue de dossier, ouvre le popup
 d'invite au premier frame si non configuré et pas déjà proposé/rejeté,
 dessine les deux popups.
  - `draw_active_workspace_row(width=160)` (workspace_setup_dialog.py)
 — ligne "Current workspace: [combo] [icône ouvrir dans le gestionnaire
 de fichiers]" dessinée dans le panneau de l'app hôte, au-dessus de la
 barre Save/Export/Quit. Le combo propose `(none)`, les workspaces
 existants, et `<new>` (ouvre le popup de création) ; désactivé (avec
 tooltip explicatif) tant que la racine n'est pas configurée.
  - `_draw_new_workspace_popup` (workspace_setup_dialog.py) — saisie
 du nom, validation (non vide, pas de doublon), création via
 `workspaces.create_workspace`, puis application d'un défaut pratique :
 le nouveau workspace hérite du dernier
 `settings.last_workspace_sync_folder` utilisé (workspace_setup_dialog.py),
 simple point de départ éditable ensuite dans Settings > Tools.
  - `_draw_prompt_popup` (workspace_setup_dialog.py) — popup de
 première utilisation, avec "Choose folder..." (ouvre le sélecteur) et
 "Later" (`_prompt_dismissed = True`, ne reproposera plus pour cette
 session).
  - `_poll_folder_dialog` (workspace_setup_dialog.py) — récupère le
 résultat du sélecteur de dossier non bloquant une fois prêt, sauvegarde
 et ferme le popup courant.
  - `draw_settings_content` (workspace_setup_dialog.py) — bloc
 "Workspaces folder: <chemin tronqué> [icône dossier]" intégré dans
 l'onglet Settings de l'app hôte, même emplacement que
 `export_dialog.py`'s `draw_settings_content`.

## Utilisation

- `app.py` importe `WorkspaceSetupDialog`, instancie
 `self.workspace_setup_dialog = WorkspaceSetupDialog` (app.py), et
 appelle `.draw` (app.py).
- `apps/object_editor.py` instancie **une seconde**
 `WorkspaceSetupDialog` séparée — à noter, deux instances distinctes
 chargent chacune leurs propres settings ; leur cohérence dépend
 entièrement du pattern reload-fresh de `_save`.
- Dépend de `ryzom_forgery.settings` (chargement/sauvegarde) et de
 `ryzom_forgery.workspaces` (modèle de données : `active_workspace_path`,
 `create_workspace`, `ensure_structure`, `is_root_configured`,
 `list_workspaces`, `open_in_system_file_manager`, `workspace_path`).

## Points notables / pièges

- Le popup de création de workspace ne peut pas être ouvert directement
 depuis l'intérieur du combo : `begin_popup_modal` ne peut pas s'imbriquer
 dans le popup propre de `begin_combo`, donc l'ouverture réelle
 (`imgui.open_popup`) est différée d'une frame via le flag
 `_new_workspace_pending`, positionné dans le combo et consommé juste après
 `end_combo` (workspace_setup_dialog.py, 162-166).
- "Cancelling" le popup de première utilisation (`Later`) ne désactive la
 proposition que pour la session en cours (`_prompt_dismissed`, en mémoire
 seulement, jamais persisté) — le dossier reste configurable depuis
 Settings à tout moment (workspace_setup_dialog.py).
- Le message du popup d'invite affirme explicitement : "shapes are only ever
 saved into a workspace, never elsewhere" (workspace_setup_dialog.py)
 — contrainte de conception du système de workspace globalement, pas
 spécifique à ce fichier.
- Aucun TODO explicite trouvé dans le fichier.
