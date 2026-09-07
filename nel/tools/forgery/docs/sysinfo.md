# Barre de statut système (sysinfo)

**Fichier :** `nel/tools/forgery/ryzom_forgery/sysinfo.py` (~48 lignes)

## Rôle

Fournit la barre de statut du bas, commune à toutes les apps Ryzom Forgery :
FPS/temps de frame du moteur, une ligne de statut libre que n'importe quelle
partie de l'app peut mettre à jour, et le numéro de version affiché à droite
(sysinfo.py).

## API principale

- `_version_label` (sysinfo.py) — construit la chaîne
 `"Ryzom Forgery v<version> -- Ulukyn, Claude@anthropic"` en lisant la
 version installée via `importlib.metadata.version("ryzom_forgery")`,
 avec repli sur `"dev"` si le paquet n'est pas installé
 (`PackageNotFoundError`).
- `class SysInfoBar` (sysinfo.py) :
  - `__init__` (sysinfo.py) : initialise `status`/`status_color` à vide,
 calcule `_version_label` une seule fois (la version ne peut pas changer
 en cours de session, sysinfo.py).
  - `set_status(text, color=None)` (sysinfo.py) — met à jour la ligne de
 statut libre, avec une couleur optionnelle `(r, g, b, a)`.
  - `draw` (sysinfo.py) — dessine, sur une seule ligne ImGui : FPS +
 ms/frame (calculé depuis `imgui.get_io.framerate`), puis le texte de
 statut (coloré ou non), puis le libellé de version aligné à droite (
 calculé via `imgui.calc_text_size` pour positionner le `same_line`).

## Utilisation

- `app.py` importe `SysInfoBar` et instancie `self.sysinfo = SysInfoBar`
 (app.py), dessinée dans une fenêtre ImGui dédiée "##sysinfo"
 (app.py) dimensionnée par `sysinfo_height` (attribut de
 `ForgeryApp`, configurable à la construction, app.py).
- `app.py,326,328` appelle `self.sysinfo.set_status(...)` pour afficher
 le chemin du fichier survolé/sélectionné dans l'explorateur, ou le nombre
 d'éléments sélectionnés.
- `navcube.py` lit `self.app.sysinfo_height` (navcube.py) pour
 positionner son propre panneau juste au-dessus de la barre de statut —
 dépendance indirecte, pas un import de `sysinfo.py` lui-même.

## Points notables / pièges

- Très petit module, aucune logique complexe. `framerate` peut valoir 0 au
 tout premier frame ; `frame_ms` est alors mis à `0.0` plutôt que de
 diviser par zéro (sysinfo.py).
- Le libellé de version est figé au lancement (calculé une seule fois dans
 `__init__`) — un changement de version en cours de run (rare, mais
 possible en dev avec une réinstallation à chaud) ne serait pas reflété
 sans redémarrage.
- Aucun TODO explicite trouvé dans le fichier.
