# Barre de statut système (sysinfo)

**Fichier :** `nel/tools/forgery/ryzom_forgery/sysinfo.py`

## Rôle

Fournit la barre de statut du bas, commune à toutes les apps Ryzom Forgery :
FPS/temps de frame du moteur, une ligne de curseur (`cursor_info`), une ligne
de statut libre que n'importe quelle partie de l'app peut mettre à jour, la
tâche de fond active (voir plus bas), et le numéro de version affiché à
droite.

## API principale

- `_version_label()` (sysinfo.py) — construit la chaîne
 `"Ryzom Forgery v<version> -- Ulukyn, Claude@anthropic"` en lisant la
 version installée via `importlib.metadata.version("ryzom_forgery")`,
 avec repli sur `"dev"` si le paquet n'est pas installé.
- `class SysInfoBar` (sysinfo.py) :
  - `set_status(text, color=None)` / `set_cursor_info(text)` — mettent à
 jour les lignes de statut/curseur libres.
  - `report_task(task_id, label, fraction)` / `clear_task(task_id)` — file
 d'attente des tâches de fond (`{task_id: (label, fraction)}`, ordre
 d'insertion = ordre de file). N'importe quel site de l'app enregistre sa
 propre tâche en cours ; seule la plus ancienne encore active est affichée,
 centrée. `fraction` à `None` = libellé seul, pas de barre (indéterminé).
 Voir `project-todos/forgery/unified_background_task_status_bar.md`.
  - `build_content_box()` — reconstruit un arbre `ui/box.py`'s `Box` à
 chaque frame (le contenu change frame à frame -- FPS, curseur/statut
 optionnels, tâche active -- voir `project-todos/forgery/
 box_ui_migration__sysinfo.md`) : un `LabelWidget` FPS+ms (`dock="left"`),
 puis curseur/statut si non vides (chacun précédé d'un espaceur `Box`),
 puis, si une tâche est active, un groupe `dock="center"` (libellé +
 `ProgressBarWidget` si `fraction` n'est pas `None`), puis le libellé de
 version (`dock="right"`, grisé). Chaque `LabelWidget`/`ProgressBarWidget`
 utilise `align="center"` pour se centrer verticalement dans la hauteur de
 la rangée -- ne dessine rien elle-même, c'est `layout()` sur l'arbre
 retourné (fait par l'appelant) qui déclenche le dessin réel.

## Utilisation

- `app.py` importe `SysInfoBar`, instancie `self.sysinfo = SysInfoBar()`.
 Son contenu est dessiné via un `ui/box_panel.py`'s `Panel(dock="bottom",
 v=sysinfo_height)`, enfant de la `Box` racine écran de `ForgeryApp`
 (`self._ui_root`) -- `draw_ui()` réassigne `self._sysinfo_box_panel.children`
 au résultat de `build_content_box()` avant chaque `layout()` (voir
 `docs/ui.md`/`docs/app.md`).
- `app.py` appelle `self.sysinfo.set_status(...)` pour afficher le chemin du
 fichier survolé/sélectionné dans l'explorateur, ou le nombre d'éléments
 sélectionnés.
- N'importe quel site déclenchant une tâche de fond (bake Panoply, clone de
 repo, chargement de région...) appelle `report_task()`/`clear_task()`
 depuis son propre poll/`draw()` sur le thread principal -- jamais depuis un
 thread de fond directement.
- `navcube.py` lit `self.app.sysinfo_height` pour positionner son propre
 panneau juste au-dessus de la barre de statut — dépendance indirecte, pas
 un import de `sysinfo.py` lui-même.

## Points notables / pièges

- `framerate` peut valoir 0 au tout premier frame ; `frame_ms` est alors mis
 à `0.0` plutôt que de diviser par zéro.
- Le libellé de version est figé au lancement (calculé une seule fois dans
 `__init__`) — un changement de version en cours de run ne serait pas
 reflété sans redémarrage.
- La fenêtre `sysinfo` est de taille FIXE (`sysinfo_height=36`), jamais
 ajustée automatiquement à son contenu — voir `docs/ui.md`'s Historique
 pour pourquoi (`always_auto_resize` faisait planter Panda3D).
- Le segment de tâche (`dock="center"`) se centre dans l'espace restant une
 fois FPS/curseur/statut/version placés, pas contre la largeur totale de la
 fenêtre -- ne peut donc jamais chevaucher les autres segments (différence
 assumée par rapport à l'ancien `center_x()`, voir
 `project-todos/forgery/box_ui_migration__sysinfo.md`).
