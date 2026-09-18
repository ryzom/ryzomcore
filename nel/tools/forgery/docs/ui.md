# ryzom_forgery/ui/ — helpers ImGui génériques et widgets réutilisables

**Dossier :** `nel/tools/forgery/ryzom_forgery/ui/`

## Rôle

Regroupe les helpers ImGui génériques partagés par Patina, Atyscape et tout
dialogue commun aux deux apps — un seul endroit, importable depuis n'importe
quel module, sans dépendance vers un dossier spécifique à une app.

## Modules

- `widgets.py` : `_icon_button`, `_set_panel_pos`/`_capture_panel_pos`
 (position de panneau flottant persistée), `_push_tab_color`/`_pop_tab_color`/
 `_begin_tab_item_with_icon` (coloration des onglets), `_colored_button`, et
 les constantes de couleur de bouton partagées (`_SAVE_BUTTON_COLOR`,
 `_CONFIRM_YES_COLOR`, etc.).
- `popups.py` : `center_next_popup(always=False)` — centre la prochaine
 `imgui.begin_popup_modal()` sur le viewport principal.
- `colors.py` : `pastel_color_for(icon)` (couleur pastel déterministe par
 glyphe, hashée sur le nom de l'icône), `darken(color, factor=0.5)`,
 `ERROR_COLOR` (constante partagée, pas encore utilisée partout — voir
 Historique).
- `layout.py` : deux primitives de centrage ciblées (pas un moteur de layout
 générique) :
 - `center_x(content_width)` : centre horizontalement un groupe d'éléments
   de largeur connue dans la fenêtre courante (remplace un `imgui.same_line()`).
 - `center_v(item_height, container_height=None)` : centre verticalement un
   élément de hauteur connue dans un conteneur (défaut
   `imgui.get_frame_height()`) — décalage relatif au curseur courant, ne
   dépend jamais du `WindowPadding` de la fenêtre (voir Historique).
- `panel.py` : `dock_rect(available, side, size)` (fonction pure : calcule le
 rectangle occupé + le rectangle restant) et `Panel(side, size,
 resizable=False, min_size=None, max_size=None)` — un widget auto-suffisant
 pour une fenêtre fixée sur un bord d'écran (`left`/`right`/`top`/`bottom`),
 étirée sur l'axe parallèle au bord. `Panel.draw(available, title, flags=0)`
 est un context manager : gère lui-même position, taille, contraintes de
 redimensionnement, les flags ImGui (`no_move`/`no_collapse`/`no_resize`
 sauf si `resizable`/`no_title_bar` si `title` commence par `##`) et la
 paire begin/end — l'appelant ne dessine que son contenu à l'intérieur du
 `with`. Un panneau `resizable=True` capture sa nouvelle taille tout seul à
 la fin de chaque `draw()`. Encore utilisé par `app.py` aujourd'hui pour
 Explorer/panel droit uniquement (sysinfo migré, voir `box_panel.py`
 ci-dessous) -- migré élément par élément par `project-todos/forgery/
 box_ui_migration.md`.
- `box.py` : `Box(dock="top", h=STRETCH, v=STRETCH, margin=0, padding=0,
 align="start")` — rectangle imbriqué, récursif (hbox/vbox), voir
 `project-todos/forgery/box_ui_migration.md`. `STRETCH` (sentinelle) et
 `Percent(value)` (pourcentage de l'extension du parent) sont les 3 types de
 valeur possibles pour `h`/`v`. `add(box)` ajoute un enfant ; `layout(allocated)`
 calcule `external_rect`/`internal_rect` (margin/padding) puis place ses
 enfants un par un selon leur `dock` (`top`/`bottom`/`left`/`right`/`center`),
 en partageant l'espace à parts égales entre enfants étirés sur le même axe
 de flux. `align` (`"start"`/`"center"`/`"end"`, défaut `"start"`) positionne
 un enfant `top`/`bottom`/`left`/`right` de taille fixe/Percent sur son AXE
 SECONDAIRE (ex. centrer verticalement un `LabelWidget` docké `"left"` dans
 une rangée plus haute que lui) -- sans effet sur un enfant `dock="center"`
 (déjà centré sur les deux axes). Délibérément sans aucune dépendance
 `imgui_bundle`, pour rester testable en pur Python (aucun affichage
 nécessaire).
- `box_panel.py` : le pendant ImGui de `box.py`, séparé pour la même raison
 (`box.py` reste importable sans `imgui_bundle`). `Panel(Box)` : une `Box`
 qui en plus ouvre une vraie fenêtre ImGui pour elle-même (position/taille/
 contraintes de redimensionnement/flags/begin-end, plus `extra_flags` pour
 des `imgui.WindowFlags_` additionnels comme `no_scrollbar`), typiquement
 ajoutée comme enfant direct d'une `Box` racine de la taille de l'écran.
 `Widget(Box)` : une `Box` qui dessine du vrai contenu ImGui dans son
 `internal_rect` au lieu de placer des enfants — `draw_content(w, h)` à
 surcharger, reçoit la taille RÉSOLUE (jamais `self.h`/`self.v` bruts, qui
 peuvent être `STRETCH` ou `Percent`). `ButtonWidget`, `LabelWidget` (avec
 `color`/`disabled`), `TextInputWidget`, `ProgressBarWidget` : implémentations
 concrètes de `Widget`. Utilisé RÉELLEMENT par `app.py` pour sysinfo depuis
 `box_ui_migration__sysinfo.md` -- migrer le reste de l'app (Explorer, panel
 droit, dialogues, popups...) continue élément par élément.

## Utilisation

- `app.py` (`ForgeryApp`, partagée par Patina et Atyscape) : Explorer et le
 panel droit restent des `Panel` (`ui/panel.py`) ; sysinfo est maintenant un
 `box_panel.Panel`, enfant d'une `Box` racine de la taille de l'écran
 (`self._ui_root`) -- chaînés dans `draw_ui()` (voir `docs/app.md`).
- `sysinfo.py` : `SysInfoBar.build_content_box()` reconstruit un vrai arbre
 `Box`/`LabelWidget`/`ProgressBarWidget` à chaque frame (le contenu change
 frame à frame) au lieu de dessiner directement -- `align="center"`
 remplace `center_x()`/`center_v()` (`ui/layout.py`, toujours utilisées
 ailleurs tant que leur propre app n'est pas migrée).

## Historique / pièges

- `ERROR_COLOR` (`colors.py`) existe mais n'est pas encore utilisée à la
 place des 9 sites qui recopient `(1.0, 0.4, 0.4, 1.0)` en dur ailleurs
 dans le code — migration prévue comme chantier de suivi séparé, pas encore
 fait.
- `_icon_button` a encore 7 copies locales indépendantes ailleurs dans le
 code (non migrées vers `ui/widgets.py`) — même chantier de suivi.
- Une taille de `Panel` qui s'ajuste automatiquement à son propre contenu
 (`imgui.WindowFlags_.always_auto_resize`) a été tentée pour `sysinfo` puis
 abandonnée : plantage reproductible de Panda3D (`AssertionError:
 !_center.is_nan()`, `boundingSphere.I`, isolé par bisection), mécanisme non
 compris. Tout `Panel` a donc une taille fixe (éventuellement
 redimensionnable à la souris via `resizable=True`), jamais déterminée par
 son contenu.
- Le calcul de position horizontale d'un élément aligné à droite doit
 soustraire `imgui.get_style().window_padding.x`, pas seulement la largeur
 de l'élément -- oublier ce terme fait déborder l'élément de la zone de
 contenu réelle de la fenêtre (rencontré sur le libellé de version de
 `sysinfo.py`, provoquait l'apparition d'un scrollbar horizontal).
- Dans `box.py`, le partage à parts égales entre enfants `STRETCH` du même
 côté doit être calculé une seule fois contre `internal_rect` (le budget
 total d'origine), jamais contre le rectangle `remaining` qui rétrécit à
 chaque enfant placé -- sinon un partage 50/50 devient 50/25 au 2e enfant.
- Dans `box_panel.py`, `Panel.layout()` doit utiliser la vraie zone de
 contenu (`imgui.get_cursor_screen_pos()`/`get_content_region_avail()`
 après `imgui.begin()`), pas le rectangle extérieur de la fenêtre demandé —
 une barre de titre visible mange de la hauteur en haut, sinon la première
 rangée d'enfants empiète dessous.
- Toujours dans `box_panel.py`, un conteneur (`Box`/sous-classe) qui dessine
 son propre remplissage doit le faire AVANT d'appeler `super().layout()`
 (qui dessine ses enfants, y compris le contenu réel d'un `Widget`) — dans
 l'autre ordre, le remplissage opaque du conteneur recouvre le contenu déjà
 dessiné de ses enfants, peu importe l'ordre d'ajout dans l'arbre.
- Le `padding` d'un `Panel` (hérité de `Box`) et le `WindowPadding` d'ImGui
 ne doivent jamais coexister (double marge) — `Panel.layout()` force
 `imgui.StyleVar_.window_padding` à `(0, 0)`, le `padding` de la `Box`
 devient l'unique source.
- **Toute migration d'une app/fenêtre existante vers ce système doit
 décomposer son contenu réel en `Box`/`Widget`, jamais l'envelopper derrière
 un seul callback opaque qui appelle l'ancienne méthode `draw()` telle
 quelle.** Un tel wrapper (ex. un hypothétique `CallbackWidget`) ne migre
 que le *fenêtrage* (position/taille de la fenêtre) -- tout ce qui est
 dessiné à l'intérieur reste du imgui direct avec centrage manuel
 (`ui/layout.py`'s `center_x`/`center_v`), donc aucun gain : le but même de
 `Box`/`Widget` est d'éliminer ce calcul à la main (un enfant docké
 `dock="center"` est centré automatiquement sur les deux axes par
 `Box._place_child()`). Un wrapper de ce genre n'est acceptable que comme
 scaffolding strictement temporaire pour poser le fenêtrage d'un premier
 jet, jamais comme livrable final d'une migration (Nuno, 2026-09-19 : "TOUT
 est a faire en Box, absolument TOUT" -- décision déjà prise une session
 plus tôt en construisant `box.py`/`box_panel.py` mais jamais notée ici,
 perdue avec le script de test jetable de l'époque -- voir aussi
 `project-todos/forgery/box_ui_migration.md`).
