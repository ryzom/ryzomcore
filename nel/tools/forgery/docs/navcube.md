# Gizmo de navigation 3D (navcube)

**Fichier :** `nel/tools/forgery/ryzom_forgery/navcube.py` (~627 lignes)

## Rôle

Implémente le petit cube de navigation en bas à droite de la vue 3D de
Patina : un cube cliquable rendu dans sa propre `DisplayRegion`, qui reflète
en temps réel l'orientation de la caméra principale (`OrbitCamera`), plus
la barre de boutons flottante (reset, pas de 90°, roulis, icônes de mode)
qui l'accompagne. Le gizmo est purement un miroir + 6 cibles de clic : sa
rotation ne vient jamais de lui-même, uniquement des contrôles du viewport
principal (navcube.py).

## API principale

- Constantes de géométrie/couleur : `_FACE_BASIS` (navcube.py), couleurs
 du cube externe (scène/caméra, semi-transparent, navcube.py) et
 interne (objet, opaque, plus petit, navcube.py), couleurs
 actif/inactif `_ACTIVE_COLOR`/`_INACTIVE_COLOR` (navcube.py) qui
 indiquent quel cube (scène ou objet) réagira au prochain drag.
- `object_targeted` est importé de `camera.py` (navcube.py) pour cette
 logique actif/inactif.
- Fonctions de construction géométrique : `_face_corners` (navcube.py),
 `_make_face_geom` (navcube.py), `_make_face_collision`
 (navcube.py) — génèrent quads + polygones de collision par face.
- `_draw_rotate_icon` (navcube.py) — dessine une icône flèche circulaire
 à la main (arc + triangle) car aucun glyphe Font Awesome (FA4 ou FA6) ne
 distingue rotation horaire/anti-horaire (les glyphes `rotate*` sont
 génériques, non directionnels).
- `_orbit_offset(heading, pitch, distance)` (navcube.py) — même formule
 que `OrbitCamera._update_camera_pos`, pour que la caméra du gizmo
 tourne en lock-step avec la vraie.
- `class NavigationCube` (navcube.py) :
  - `__init__` (navcube.py) : construit les 6 faces + collisions du cube
 externe, le cube interne (`_build_inner_cube`), les arêtes, le triède
 d'axes XYZ, les labels, la caméra/lens/`DisplayRegion` dédiée (sort=5,
 donc rendue avant l'UI ImGui — navcube.py), le picking par
 rayon (`CollisionTraverser`), puis démarre la tâche `navcube-update` et
 écoute `mouse1`.
  - `_build_inner_cube` (navcube.py) : cube interne prévu pour
 représenter l'orientation propre de l'objet inspecté, séparément de la
 caméra — mis à jour en copiant le quaternion de `object_pivot` chaque
 frame (navcube.py).
  - `_layout` (navcube.py) : calcule le rectangle pixel du panneau à
 partir de `imgui.get_io.display_size` (pas la taille brute de la
 fenêtre, pour rester cohérent sous mise à l'échelle DPI), ancré au bord
 du viewport 3D (pas au bord de la fenêtre).
  - `_update` (navcube.py) : tâche par tick — repositionne la
 `DisplayRegion`, oriente la caméra du gizmo sur heading/pitch/up_hint de
 l'`orbit_camera`, recopie l'orientation de l'objet dans `inner_np`,
 calcule la face survolée (`_pick_axis_at_mouse`), et retente les
 couleurs actif/inactif des deux cubes selon `object_targeted(self.app)`.
  - `_pick_axis_at_mouse`/`_on_click` (navcube.py, 426) : picking par
 rayon depuis la position souris locale au panneau ; un clic appelle
 `self.orbit_camera.snap_to_axis(axis)`.
  - `_icon_button`/`_roll_button` (navcube.py, 444) : boutons carrés
 auto-dimensionnés (glyphe FA ou icône dessinée à la main).
  - `_draw_status_icon` (navcube.py) : icône bas-droite du pavé
 directionnel, affiche/force `app.forced_drag_mode` (cycle
 pointeur→move→rotate→scale).
  - `_draw_mode_icon` (navcube.py) : icône bas-gauche, affiche/force
 `app.target_mode` (cycle Ctrl-décide→camera→objet).
  - `draw_controls` (navcube.py) : dessine la fenêtre flottante de
 boutons (roulis/flèches/Home/mode/status) au-dessus du gizmo, à appeler
 une fois par frame depuis `draw_panel`.

## Utilisation

- `apps/object_editor.py` instancie
 `self.nav_cube = NavigationCube(self, self.orbit_camera,
 self._object_pivot)`.
- `apps/object_editor.py` appelle `self.nav_cube.draw_controls` à
 chaque frame (dans la boucle de dessin du panneau).
- `apps/object_editor.py` lit `self.nav_cube._panel_px` directement
 (accès à un attribut "privé") pour positionner autre chose par rapport au
 panneau du gizmo.
- Dépend de `camera.py` (`object_targeted`) et lit plusieurs attributs de
 `app` : `sysinfo_height`, `panel_width`, `forced_drag_mode`,
 `target_mode`, `mouseWatcherNode`, `imgui`.

## Points notables / pièges

- Le cube interne (objet) existe et est mis à jour (`inner_np.set_quat(...)`
 à navcube.py) mais son commentaire de construction précise qu'il
 "représentera la rotation de l'objet inspecté une fois câblé séparément
 de la caméra" (navcube.py) — en l'état il est déjà bien câblé sur
 `object_pivot.get_quat`, donc ce commentaire semble décrire un état
 antérieur du développement plutôt qu'un TODO actif ; à vérifier si le
 câblage est jugé complet ou encore partiel par les auteurs.
- Couleurs des arêtes : `set_color_scale` est utilisé au lieu de
 `set_color` car la géométrie construite via `LineSegs` a une couleur
 par-sommet déjà "cuite" qui ignorerait un `set_color` — seul
 `set_color_scale` (un `ColorScaleAttrib` multiplié séparément) permet de
 reteinter dynamiquement (navcube.py).
- La `DisplayRegion` du gizmo est réglée en `sort=5`, explicitement en
 dessous du `sort=10` de la 2D UI d'ImGui (`makeCamera2d`), pour ne
 jamais dessiner par-dessus les fenêtres/tooltips ImGui qui chevauchent
 son rectangle (navcube.py).
- Les labels d'axes XYZ sont dessinés directement dans la scène 3D
 (billboardés sur la caméra du gizmo) plutôt qu'en overlay ImGui, car la
 projection via la lens à chaque frame "s'est avérée peu fiable"
 (navcube.py) — limitation constatée, pas un choix idéal.
- `draw_controls` interroge `imgui.is_item_active` à chaque frame plutôt
 que de s'appuyer sur le "button repeat" natif d'ImGui, pour éviter un
 décalage entre la fin réelle d'une animation de pas et le prochain tir du
 timer de répétition (navcube.py).
- `_CAM_DISTANCE = 9.5` est calculé pour que les coins du cube (à
 `sqrt(3) * _HALF` du centre) restent dans le cadre avec un FOV de 30°
 (navcube.py) — valeur dérivée géométriquement, pas arbitraire.
- Aucun TODO explicite trouvé dans le fichier, hormis la remarque ci-dessus
 sur le cube interne.
