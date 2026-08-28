# splash.py et _splash_process.py — écran de démarrage

**Fichiers :** `nel/tools/forgery/ryzom_forgery/splash.py` (45 lignes) et
`nel/tools/forgery/ryzom_forgery/_splash_process.py` (65 lignes)

## Rôle

Ces deux fichiers implémentent l'écran de démarrage (splash screen) affiché
pendant qu'une app Forgery se lance. `splash.py` expose la classe `Splash`,
utilisée côté process principal (`app.py`) ; `_splash_process.py` est un
petit script Tk autonome, lancé comme **sous-process séparé** par `Splash`,
qui affiche réellement l'image et se contente d'attendre d'être tué. Il n'y
a aucune communication inter-process (IPC) : le timing (durée minimale
d'affichage) est entièrement géré côté process parent par `Splash.close` ;
le sous-process ne sait faire que deux choses, s'afficher et se terminer.

## API principale

### `splash.py`

- `Splash.__init__(self, image_path, min_duration=1.2, subsample=1, center_on=None)`
 (`splash.py`) : mémorise l'heure de démarrage (`time.monotonic`) et
 lance immédiatement le sous-process
 `python -m ryzom_forgery._splash_process IMAGE_PATH SUBSAMPLE [TARGET_X TARGET_Y TARGET_WIDTH TARGET_HEIGHT]`
 via `subprocess.Popen`. `center_on`, si fourni, est un tuple
 `(x, y, width, height)` de la fenêtre cible sur laquelle centrer le splash
 (l'écran/moniteur visé), transmis tel quel en arguments positionnels.
- `Splash.close(self)` (`splash.py`) : bloque jusqu'à ce que
 `min_duration` secondes se soient écoulées depuis la construction, puis
 termine (`terminate` + `wait`) le sous-process splash. `min_duration`
 est un **plancher** sur le temps réel écoulé, pas un délai ajouté en plus
 — si le lancement de l'app a déjà pris plus longtemps que `min_duration`,
 `close` ne dort pas du tout (`splash.py`).

### `_splash_process.py`

- `main(argv=None)` (`_splash_process.py`) : point d'entrée du
 sous-process. Parse `image_path`, `subsample`, et optionnellement
 `center_on` (4 entiers) depuis `argv`. Crée une fenêtre Tk sans bordure
 (`overrideredirect(True)`) et toujours au premier plan
 (`attributes("-topmost", True)`), charge l'image via `tk.PhotoImage`
 (avec sous-échantillonnage optionnel via `subsample`), calcule sa
 position (centrée sur `center_on` si fourni, sinon centrée sur l'écran
 entier via `winfo_screenwidth`/`winfo_screenheight`), l'affiche dans
 un `tk.Label`, programme une auto-destruction de sécurité après 5000 ms
 (`root.after(5000, root.destroy)`, `_splash_process.py`), puis lance
 `root.mainloop`.

## Utilisation

- `ForgeryApp.__init__` (`app.py`) est l'unique appelant connu de
 `Splash` dans le projet : `self._splash = Splash(_SPLASH_PATH, center_on=center_on) if _SPLASH_PATH.exists else None`,
 créé **avant** `ShowBase.__init__(self)` pour couvrir tout l'intervalle
 entre la construction Python et la première frame réellement rendue.
 `ForgeryApp.draw_ui` (`app.py`) appelle `self._splash.close`
 au tout premier appel puis met `self._splash` à `None`.
- `_splash_process.py` n'est jamais importé directement : il est invoqué
 uniquement comme sous-process via `python -m ryzom_forgery._splash_process`
 (`splash.py`), jamais appelé en tant que module Python normal.

## Points notables / pièges

- Le choix d'un **process séparé** plutôt qu'un thread est délibéré et
 documenté dans les deux fichiers (`splash.py`,
 `_splash_process.py`) : Tk/Tcl n'est pas thread-safe, et sur macOS
 un toolkit GUI est en plus restreint au thread principal du process. Un
 ancien design faisait tourner Tk sur un thread dans le process Forgery
 principal, ce qui plantait (`Tcl_AsyncDelete: async handler deleted by
 the wrong thread`) une fois que l'app s'est mise à lancer d'autres
 threads de fond (ex. le watcher de système de fichiers du workspace).
 Un process séparé élimine le problème par construction : aucun thread du
 process principal ne peut jamais interagir avec l'interpréteur Tcl du
 splash.
- Le timer d'auto-destruction de 5000 ms dans `_splash_process.py`
 est un filet de sécurité, pas le mécanisme normal de fermeture : si le
 process parent meurt anormalement (crash, `kill -9`) sans jamais appeler
 `Splash.close`, ce timer borne les dégâts à une seule fenêtre fantôme
 au lieu d'une accumulation indéfinie. Le commentaire précise que 5s
 couvre largement un démarrage normal (`min_duration` par défaut n'est
 que 1.2s).
- `winfo_screenwidth`/`winfo_screenheight` (utilisés seulement en
 fallback, sans `center_on`) renvoient la taille du bureau virtuel entier
 sur un setup X11 multi-écran — voir le commentaire équivalent dans
 `app.py` qui explique pourquoi `ForgeryApp` préfère calculer et
 passer `center_on` explicitement plutôt que de laisser
 `_splash_process.py` centrer sur l'écran entier (ce qui centrerait à
 cheval sur plusieurs moniteurs).
- Aucun protocole de communication retour (le sous-process ne signale
 jamais "je suis prêt/affiché" au parent) : le parent lance le splash puis
 poursuit immédiatement son propre travail d'initialisation, sans
 synchronisation autre que le fait que `Popen` a réussi à démarrer le
 process.
