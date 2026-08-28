# export_dialog

**Fichier :** `nel/tools/forgery/ryzom_forgery/export_dialog.py` (~215 lignes)

## Rôle

UI ImGui du flux d'export `.shape` → `.obj`/`.dae`/`.stl`/`.gltf`/`.glb` dans Patina : demande un dossier de sortie (dialogue natif via `portable_file_dialogs`) et, pour les formats qui supportent les matériaux, comment gérer les textures — chacun avec une case "se souvenir de ce choix" persistée dans le fichier de settings partagé (`ryzom_forgery.settings`). Une section de réglages toujours disponible (`draw_settings_content`) permet de revoir/changer ces choix mémorisés sans déclencher un export. C'est la couche UI au-dessus de `shape_export.export_shape`.

## API principale

- `_icon_button(icon, tooltip)` (`export_dialog.py`) — bouton icône + tooltip, copie locale du même pattern que dans `search_paths_dialog.py` (convention assumée du codebase : chaque module garde sa propre petite copie plutôt que de factoriser).
- `ExportDialog.__init__` (`export_dialog.py`) — charge la config export courante depuis `app_settings.load.export`, initialise l'état de dialogue en attente.
- `ExportDialog._save` (`export_dialog.py`) — recharge le fichier de settings à neuf et n'écrase que la section `export`, pour ne pas écraser d'autres sections modifiées entretemps par d'autres composants (Explorer favorites, search paths, data_root).
- `ExportDialog.export(shape_value, name, export_format, texture_finder, source_folder=None)` (`export_dialog.py`) — point d'entrée appelé par `object_editor.py` (bouton Export). Si un dossier de sortie est déjà mémorisé (`remember_output_folder`), saute directement à la confirmation ; sinon ouvre le sélecteur de dossier natif.
- `ExportDialog.draw` (`export_dialog.py`) — à appeler chaque frame ImGui ; poll les dialogues de fichier et dessine le popup de confirmation.
- `_poll_folder_dialog` (`export_dialog.py`) — récupère le résultat du sélecteur de dossier une fois prêt (non bloquant, `ready(0)`), annule si l'utilisateur ferme sans choisir.
- `_open_confirmation_if_needed` (`export_dialog.py`) — décide si un popup de confirmation est nécessaire (dossier venant d'être choisi sans mémorisation active, ou format à matériaux sans mode texture mémorisé) ; sinon lance l'export directement.
- `_draw_confirmation_popup` (`export_dialog.py`) — popup modal ImGui : affiche le dossier cible, propose la case "toujours utiliser ce dossier" si pertinent, et pour les formats à matériaux un choix radio Copier en .png / Référencer le nom de fichier original + case "toujours utiliser ce choix". Boutons Export/Cancel.
- `_run_export` (`export_dialog.py`) — appelle réellement `shape_export.export_shape`, capture les exceptions dans `self._status` (pas de re-raise — échec affiché en UI seulement).
- `_start_settings_folder_dialog` / `_poll_settings_folder_dialog` (`export_dialog.py`) — dialogue de dossier indépendant pour l'onglet Settings, séparé de celui d'un export en cours.
- `draw_settings_content` (`export_dialog.py`) — contenu à intégrer directement dans l'onglet "Settings" de l'appli hôte (pas une fenêtre flottante séparée) : dossier de sortie, case mémorisation, choix du mode texture par défaut. Chaque champ se sauvegarde immédiatement au changement (pas de bouton Save séparé).

## Utilisation

- `apps/object_editor.py` importe `ExportDialog` ; instancié en `object_editor.py` (`self.export_dialog = ExportDialog`).
- `object_editor.py` appelle `self.export_dialog.export(...)` depuis le bouton Export de la barre du bas, après que l'utilisateur a choisi un format via un sélecteur préalable (`object_editor.py`).
- `object_editor.py` appelle `self.export_dialog.draw` dans la boucle de frame principale.
- Dépend de `shape_export.export_shape` (voir `shape_export.md`) et de `ryzom_forgery.settings` (`TEXTURE_MODE_COPY_PNG`, `TEXTURE_MODE_REFERENCE_ONLY`, section `export` du fichier de settings : `output_folder`, `remember_output_folder`, `texture_mode`, `remember_texture_mode`).
- `workspace_setup_dialog.py,99,239` référence explicitement ce module comme modèle du pattern "dialogue non-bloquant + `_save` qui ne réécrit que sa propre section".

## Points notables / pièges

- L'échec d'un export n'est jamais levé à l'appelant : `_run_export` capture toute `Exception` et se contente de mettre à jour `self._status` (`export_dialog.py`) — l'appelant (`object_editor.py`) ne peut pas réagir programmatiquement à un échec, seulement l'utilisateur voit le message dans l'UI.
- Le mode texture n'est proposé/pertinent que si `export_format.supports_materials` est vrai (`export_dialog.py, 121, 137, 143-144`) ; pour STL (`supports_materials=False`), `pending["texture_mode"]` est forcé à `TEXTURE_MODE_REFERENCE_ONLY` par défaut même si non affiché (`export_dialog.py`) — sans conséquence puisque `_export_stl` ignore ce paramètre de toute façon.
- `_save` recharge systématiquement le fichier de settings frais avant d'écraser sa propre section — un piège classique évité ici explicitement (voir docstring `export_dialog.py`) : ne jamais réutiliser une copie de settings mise en cache depuis `__init__`.
- Pas de TODO explicite dans le fichier.
