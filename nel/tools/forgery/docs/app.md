# app.py — la classe de base des apps Forgery

**Fichier :** `nel/tools/forgery/ryzom_forgery/app.py` (369 lignes)

## Rôle

`app.py` définit `ForgeryApp`, la classe de base (héritant de Panda3D
`ShowBase`) dont dérive chaque outil de Ryzom Forgery — notamment
`ObjectEditorApp` (Patina, `apps/object_editor.py`). Elle met en place
tout ce qui est commun à ces outils : la fenêtre Panda3D, l'overlay Dear
ImGui (via `p3dimgui`), le thème visuel, les polices (texte + icônes Font
Awesome), l'éclairage de scène par défaut, et le layout d'écran standard —
une barre système en bas, un explorateur de fichiers `.bnp` à gauche, un
panneau spécifique à l'outil à droite, et le centre libre pour le viewport
3D. Les sous-classes n'ont normalement qu'à surcharger `draw_panel` (et
éventuellement `panel_title`, `on_selection_changed`, `_on_exit`)
pour ajouter leur logique métier.

## API principale

- `_slugify(title)` (`app.py`) : normalise un titre de fenêtre en nom de
 fichier (utilisé pour la persistance de géométrie de fenêtre par app).
- `ForgeryApp.__init__(self, explorer_root, title=..., explorer_width=300, panel_width=320, sysinfo_height=28, explorer_default_filter=DEFAULT_FILTER)`
 (`app.py`) : construit toute la fenêtre/l'UI. Dans l'ordre : charge la
 géométrie de fenêtre sauvegardée (`_load_window_geometry`), affiche le
 splash (`Splash`, voir `docs/splash.md`) *avant* `ShowBase.__init__`,
 configure la fenêtre (titre, icône, taille/position), désactive la
 caméra trackball par défaut (`disableMouse`), élargit le near/far plane
 de la caméra (`app.py`, `0.02`–`20000.0`), initialise `p3dimgui`,
 applique le thème (coins arrondis, fonds opaques, texte blanc pur),
 charge les polices, crée le watermark du panneau, ajoute un éclairage
 ambiant + directionnel, instancie `CommandRegistry`, `SysInfoBar`,
 `WorkspaceSetupDialog` et `Explorer`, puis abonne `draw_ui` à l'événement
 Panda3D `imgui-new-frame`.
- `windowEvent(self, win)` (`app.py`) : surcharge Panda3D ; sauvegarde
 la géométrie de la fenêtre à chaque événement fenêtre concernant la
 fenêtre principale.
- `_on_exit(self)` (`app.py`) : hook no-op à surcharger par les
 sous-classes, appelé une seule fois juste avant la sortie réelle du
 process (branché sur `self.exitFunc`, `app.py`).
- `relaunch(self)` (`app.py`) : relance le process avec la même ligne
 de commande via `os.execv` (remplace le process en place, même PID).
 Utilisé quand un changement de police/taille de police ne peut prendre
 effet qu'au démarrage. Nommée `relaunch` et non `restart` délibérément —
 voir Pièges ci-dessous.
- `_load_window_geometry(self)` / `_save_window_geometry(self)`
 (`app.py`, `app.py`) : persistent la position/taille de fenêtre
 dans `~/.ryzom_forgery/<titre_slugifié>.json`, par app (clé = titre de
 fenêtre slugifié).
- `_dpi_scale()` (`app.py`, fonction module) : lit la variable d'env
 `RYZOM_FORGERY_DPI_SCALE` (`1.0` par défaut si absente/invalide) — posée
 par le launcher Ryztart (`ryzom_forgery_launcher.launcher.call_LaunchApp()`,
 dépôt `ryztart`) via `webview.screens[0].scale` (Qt/GTK/Cocoa sous le
 capot, seule source fiable qu'on ait : le fenêtrage propre de Panda3D n'a
 aucune détection DPI sous Linux). `1.0` (aucun scaling) si l'app est
 lancée directement (`dev.sh`, sans passer par Ryztart).
- `_load_ui_font(self)` (`app.py`) : charge la police choisie par
 l'utilisateur (`Settings.ui_font_name`/`ui_font_size`, voir
 `docs/settings.md`), multipliée par `_dpi_scale()`, comme police par
 défaut d'ImGui ; retombe sur `_DEFAULT_FONT_NAME` ("Roboto Bold") si le
 fichier stocké n'existe plus.
- `_load_icon_font(self)` (`app.py`) : fusionne les glyphes Font
 Awesome 4 dans la police par défaut (pour utiliser `ICON_FA_*` dans un
 `imgui.text`/`button` normal), et construit en plus `self.large_icon_font`
 (1.5× `_ICON_FONT_SIZE * _dpi_scale()`), une police icône autonome pour
 les gros boutons icône-seule (ex. barres de bascule du viewport dans
 `object_editor.py`).
- `draw_panel(self)` / `panel_title(self)` (`app.py`, `app.py`) :
 points d'extension no-op/generic à surcharger par les sous-classes.
- `_draw_panel_watermark(self)` (`app.py`) : dessine le filigrane de
 marque (splash réduit, alpha 0.30) centré en bas du panneau droit, avant
 le contenu de `draw_panel`.
- `on_selection_changed(self, items)` / `_on_explorer_selection_changed(self, items)`
 (`app.py`, `app.py`) : réagit au changement de sélection dans
 l'Explorer ; met à jour le statut de la sysinfo bar puis délègue au hook
 surchargeable `on_selection_changed`.
- `draw_ui(self)` (`app.py`) : callback appelé à chaque frame ImGui.
 Ferme le splash au tout premier appel, dessine le dialogue de setup de
 workspace, puis positionne/dessine les trois fenêtres pinnées (sysinfo,
 Explorer, panel) selon `display_size` et les largeurs courantes.

## Utilisation

- `ObjectEditorApp` (`apps/object_editor.py`) hérite de `ForgeryApp` et
 l'appelle explicitement dans son `__init__` (`apps/object_editor.py`).
 C'est l'unique app "produit" connue qui l'utilise dans ce dossier.
- `examples/smoke_test.py` montre un usage minimal (`SmokeTestApp`)
 utilisé pour valider que le socle marche, avec un enregistrement de
 commande via `self.commands.register_global(...)`.
- `_AVAILABLE_FONTS` est importé directement par `object_editor.py`
 (`from ryzom_forgery.app import _AVAILABLE_FONTS, ForgeryApp`) pour
 peupler le picker de police dans ses réglages.

## Points notables / pièges

- Le splash (`self._splash = Splash(...)`, `app.py`) est créé **avant**
 `ShowBase.__init__(self)` — il doit couvrir tout l'écart entre la
 construction Python et la première frame réellement affichée, pas
 seulement le temps du `__init__`. Il n'est fermé qu'au tout premier appel
 de `draw_ui` (`app.py`), donc à la première frame ImGui
 effective.
- Le commentaire de `app.py` explique pourquoi le near/far plane
 par défaut de Panda3D (near=1.0) est changé à `(0.02, 20000.0)` : sans
 ça, un petit maillage (ex. une coiffure) peut se retrouver entièrement
 derrière le near plane et ne jamais s'afficher.
- Le thème ImGui applique des couleurs fixes en dur (`app.py`) :
 fonds `window_bg`/`popup_bg` rendus totalement opaques (le thème sombre
 par défaut d'ImGui laisse une translucidité ~0.94), et `modal_window_dim_bg`
 totalement désactivé (alpha 0) plutôt que juste atténué — le commentaire
 précise que dans ce pipeline de rendu, le dimming de la modal se mélangeait
 aussi avec le contenu de la modal elle-même (bug confirmé empiriquement,
 `app.py`), donc le compromis assumé est : plus de dimming du fond
 derrière une modal, en échange de couleurs de modal fidèles.
- `relaunch` (`app.py`) est nommée ainsi et non `restart` très
 délibérément : `ShowBase` (classe parente) définit déjà sa propre méthode
 `restart`, appelée depuis `ShowBase.__init__` pour démarrer la tâche
 IGLOOP. Un override nommé `restart` ici serait appelé en boucle, en plein
 `__init__`, via `os.execv` — une boucle infinie de ré-exec au même PID
 déguisée en "l'app redémarre toute seule sans raison" (bug déjà rencontré
 et documenté dans le commentaire, `app.py`).
- Notez à `app.py` que le corps de `relaunch` contient deux appels
 identiques à `os.execv(...)` à la suite — le second est mort code puisque
 `execv` ne retourne jamais en cas de succès (il remplace le process). Le
 fichier ne commente pas explicitement ce doublon ; possible reliquat/faute
 de frappe, à vérifier si on retouche ce fichier.
- `_ICON_FONT_PATH`/`_FONTS_DIR` pointent dans les assets internes
 d'`imgui_bundle` (`app.py`) — aucune police n'est bundlée séparément
 dans le wheel Forgery, tout vient de la dépendance `imgui_bundle` elle-même.
- Le layout des trois fenêtres pinnées utilise `imgui.Cond_.first_use_ever`
 pour la taille initiale (`app.py`), mais la contrainte
 `set_next_window_size_constraints` (`app.py`, `360-361`) est elle
 appliquée à chaque frame — l'explorateur et le panneau sont donc
 redimensionnables en largeur (150 à 900px, `_SIDE_PANEL_MIN_WIDTH`/`_MAX_WIDTH`)
 mais leur hauteur reste verrouillée à `body_height`.
- Le titre du panneau utilise l'astuce ImGui `f"{self.panel_title}###panel"`
 (`app.py`) pour garder une identité ImGui stable (`###panel`) malgré
 un titre affiché qui change dynamiquement.
