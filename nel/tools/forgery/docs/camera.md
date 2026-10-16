# Contrôles caméra et manipulation d'objet

**Fichier :** `nel/tools/forgery/ryzom_forgery/camera.py` (~561 lignes)

## Rôle

Ce module fournit les deux contrôleurs de souris de la vue 3D de Patina
(`object_editor.py`) : `OrbitCamera` (déplacer la caméra) et
`ObjectManipulator` (déplacer l'objet inspecté). Les deux se partagent le
même jeu de boutons souris (gauche/milieu/droit) mais agissent sur des
cibles différentes selon que Ctrl est maintenu ou non — c'est
`object_targeted` qui tranche, une fonction utilisée aussi bien par les
deux contrôleurs que par `navcube.py` (couleur du gizmo). Le module gère
aussi les animations de "snap" (vue alignée sur un axe) et de rotation par
pas de 90° utilisées par les boutons du navcube.

## API principale

- `AXIS_VIEWS` (camera.py) — table (heading, pitch) pour chacune des 6
 vues axées (+x/-x/+y/-y/+z/-z), dérivée de la formule de
 `_update_camera_pos`.
- `object_targeted(app)` (camera.py) — vrai si un drag doit agir sur
 l'objet plutôt que la caméra : `app.target_mode` (None/"camera"/"object")
 prime sur l'état de Ctrl si non-None, sinon on regarde Ctrl. Partagée par
 `OrbitCamera._update`, `ObjectManipulator._update` et le gizmo.
- `AXIS_LABELS` (camera.py) — libellés de face façon Ryzom
 Studio/3ds Max (RIGHT/LEFT/BACK/FRONT/TOP/BOTTOM), pas utilisés
 directement dans ce fichier (probablement pour un usage ailleurs / futur).
- `class OrbitCamera` (camera.py) — caméra orbitale style Blender :
  - `__init__` (camera.py) : crée les bindings molette
 (`wheel_up`/`wheel_down`, versions shift-préfixées) et ajoute une tâche
 `orbit-camera-update` à `taskMgr`.
  - `_zoom`/`_zoom_drag` (camera.py, 106) : zoom discret (molette) et
 continu (drag droit), même falloff exponentiel (`zoom_speed`).
  - `_start_anim`/`_start_axis_anim`/`_advance_anim` (camera.py, 136,
 168) : deux stratégies d'animation — un lerp indépendant
 heading/pitch/up ("retarget", utilisé par `snap_to_axis`) contre une
 rotation géométriquement exacte autour d'un axe fixe ("axis", utilisée
 par `step_to_face`/`roll_step`). Voir "Points notables" ci-dessous.
  - `_update` (camera.py) : tâche par tick — avance l'animation en
 cours, lit les 3 boutons souris (avec prise en compte de
 `app.forced_drag_mode`), ignore l'input si l'UI ImGui capture la souris
 ou si `object_targeted` dit que c'est à l'objet de réagir.
  - `_orbit`/`_pan` (camera.py, 260) : rotation libre et panoramique à
 l'échelle écran exacte (basée sur le FOV et la distance courante).
  - `frame(target, distance)` (camera.py) : recentre la caméra sur un
 nouvel objet chargé, garde heading/pitch actuels, met à jour le
 "default" restauré par `reset`.
  - `reset` (camera.py) : revient au dernier framing (`frame`) mais
 toujours vue de face à plat (`AXIS_VIEWS["-y"]`), indépendamment de
 l'orientation au moment du chargement.
  - `snap_to_axis(axis)` (camera.py) : anime vers une des 6 vues axées.
  - `step_to_face(direction)` (camera.py) : pas de 90° dans une
 direction écran ("up"/"down"/"left"/"right"), utilisé par les flèches
 du navcube — voir logique délicate documentée dans le docstring
 (camera.py) sur le choix de l'axe de rotation.
  - `roll_step(direction, degrees_step=90.0)` (camera.py) : roule la
 vue autour de son propre axe avant (les 2 boutons coin du navcube).
  - `_update_camera_pos` (camera.py) : place et oriente
 `app.camera` à partir de heading/pitch/distance/target/up_hint —
 fonction centrale rejouée par tout ce qui précède.
- `class ObjectManipulator` (camera.py) — pendant Ctrl-maintenu
 d'`OrbitCamera`, agit sur un `NodePath` (pivot de l'objet) :
  - `__init__` (camera.py) : ajoute la tâche `object-manipulator-update`.
  - `_target_node(prop)` (camera.py) : retourne `app.model_root` si le
 verrou "pivot" de la propriété (position/rotation/scale) est actif dans
 le panneau, sinon `self.pivot` — doit rester synchronisé avec la copie
 équivalente dans `ObjectEditorApp._transform_node`.
  - `_rotate`/`_move`/`_scale` (camera.py, 514, 545) : rotation type
 trackball relative à la caméra (composée en espace monde, pas local),
 déplacement à l'échelle écran identique à `_pan`, mise à l'échelle
 exponentielle identique à `_zoom_drag` — chacune respecte les verrous
 par axe (`app.transform_locks`).

## Utilisation

- `apps/object_editor.py` importe `ObjectManipulator, OrbitCamera` et
 les instancie en `apps/object_editor.py`
 (`self.orbit_camera = OrbitCamera(self, distance=10.0)`,
 `self.object_manipulator = ObjectManipulator(self, self._object_pivot,
 self.orbit_camera)`).
- `navcube.py` importe `object_targeted` pour synchroniser la couleur du
 cube (interne/externe) avec la cible active du drag, et pour son bouton
 Home (camera.py + navcube.py).
- `apps/object_editor.py` lit/écrit `self.forced_drag_mode` et
 `self.target_mode` (initialisés à `None`, apps/object_editor.py,643),
 consommés par `_update` des deux contrôleurs et par le gizmo.

## Points notables / pièges

- Deux mécaniques d'animation distinctes coexistent volontairement : le lerp
 indépendant heading/pitch/up (`_start_anim`) est correct pour un
 "retarget" vers une vue sans rapport (pas de vrai chemin "unique" entre
 deux vues arbitraires), mais serait géométriquement faux pour un pas fixe
 de rotation — d'où `_start_axis_anim`, qui interpole l'ANGLE d'une
 rotation réelle et retombe toujours exactement sur l'arc de cercle
 attendu (camera.py).
- `step_to_face("up"/"down")` relit l'axe "droite" *actuellement rendu à
 l'écran* (`self.app.camera.getQuat.getRight`) plutôt que de dériver
 un axe depuis heading/pitch, à cause d'une singularité de coordonnées aux
 pôles (+z/-z) où heading devient un artefact `atan2(0,0)` — voir le
 docstring détaillé (camera.py).
- `up_hint` n'est jamais remis à Z de force par `step_to_face` : il est
 tourné avec la même rotation que la vue, pour qu'un pas "up"/"down"
 laissant une autre face "en haut" garde cette face en haut lors d'un pas
 ultérieur "left"/"right" — comportement voulu, comme un vrai cube
 physique (camera.py).
- `step_to_face` et `roll_step` sont des no-op tant qu'une animation est
 en cours (camera.py, 365-366) : c'est délibéré pour donner un pas
 net de 90° par `anim_duration` même en maintenant le bouton, plutôt que de
 subir le timer fixe du "button repeat" d'ImGui qui désynchroniserait
 vitesse et durée réelle de l'animation.
- `_advance_anim` en mode "axis" utilise une vitesse angulaire constante
 (pas de smoothstep) explicitement pour éviter un "stutter" à chaque
 frontière de 90° lorsque plusieurs pas s'enchaînent (camera.py).
 Le mode "retarget", lui, utilise un smoothstep car il ne s'enchaîne
 jamais (camera.py).
- `ObjectManipulator._rotate` compose `quat * delta` (et non
 `delta * quat`) précisément parce que Panda3D compose `A * B` comme "B en
 premier, dans le repère local de A" — l'ordre inverse ferait tourner
 l'objet autour de ses propres axes déjà inclinés plutôt qu'autour des
 axes caméra fixes (camera.py, commentaire explicite sur ce piège).
- Aucun TODO explicite trouvé dans le fichier.
