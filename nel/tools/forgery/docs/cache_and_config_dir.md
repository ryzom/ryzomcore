# cache_dir.py et config_dir.py — dossiers standards par utilisateur

**Fichiers :** `nel/tools/forgery/ryzom_forgery/cache_dir.py` (21 lignes) et
`nel/tools/forgery/ryzom_forgery/config_dir.py` (20 lignes)

## Rôle

Ces deux modules jumeaux exposent chacun une fonction unique qui renvoie
l'emplacement standard, propre à l'OS, d'un dossier par-utilisateur pour
Ryzom Forgery — sans dépendance externe (pas de lib type `appdirs`/
`platformdirs`), juste les variables d'environnement et chemins connus pour
chaque plateforme. La distinction entre les deux est sémantique, pas
technique : `config_dir` est pour des préférences que l'utilisateur a
choisies explicitement et qui doivent survivre indéfiniment (réglages
d'export, favoris de l'explorateur, workspaces...) ; `cache_dir` est pour
des données qu'il est bon marché de régénérer et que l'OS/l'utilisateur
peut purger sans rien casser (résultats de scan `.skel`/`.anim` par
exemple) — juste un peu plus lent à la prochaine exécution.

## API principale

- `cache_dir -> Path` (`cache_dir.py`) : renvoie
 `<base>/ryzom_forgery`, où `<base>` est `%LOCALAPPDATA%` (ou
 `~/AppData/Local` en repli) sous Windows, `~/Library/Caches` sous macOS,
 et `$XDG_CACHE_HOME` (ou `~/.cache` en repli) sous les autres OS
 (Linux/BSD...).
- `config_dir -> Path` (`config_dir.py`) : même schéma mais avec
 `%APPDATA%`/`~/AppData/Roaming` sous Windows, `~/Library/Application
 Support` sous macOS, et `$XDG_CONFIG_HOME`/`~/.config` ailleurs.

Aucune des deux fonctions ne crée le dossier — elles renvoient seulement le
`Path` calculé ; c'est à l'appelant de faire `mkdir(parents=True, exist_ok=True)`
si besoin (voir `settings.py` pour un exemple).

## Utilisation

- `settings.py` (`from .config_dir import config_dir`) : le fichier
 unique de settings TOML de Forgery vit dans
 `config_dir / "settings.toml"` (`settings.py, 163`). Voir
 `docs/settings.md`.
- `search_paths.py` (`from .cache_dir import cache_dir`) : y stocke le
 cache de scan des chemins de recherche
 (`_SCAN_CACHE_FILE_NAME`, `search_paths.py, 203`) et le cache de la
 table `.bnp` (`_BNP_TABLE_CACHE_FILE_NAME`, `search_paths.py, 222`).
- `search_paths_dialog.py, 231` réutilise `search_paths.cache_dir`
 (ré-exporté via le module `search_paths`, pas réimporté directement
 depuis `cache_dir.py`) pour son propre cache d'index externe
 (`_EXTERNAL_INDEX_CACHE_FILE_NAME`).

## Points notables / pièges

- Le docstring de `cache_dir.py` explicite la distinction de design
 avec `config_dir` — c'est la seule vraie source de vérité sur *pourquoi*
 deux modules séparés existent plutôt qu'un seul dossier partagé.
- Ces dossiers ne dépendent jamais du répertoire du projet/checkout —
 `config_dir.py` le précise explicitement ("not the project
 directory, so preferences survive across checkouts/installs") : les
 préférences survivent à une réinstallation ou un nouveau clone du dépôt.
- Sur Linux/BSD, les deux fonctions respectent la spec XDG
 (`XDG_CACHE_HOME`/`XDG_CONFIG_HOME`) si ces variables sont positionnées,
 avec repli sur `~/.cache`/`~/.config` sinon — comportement standard, pas
 de logique Forgery-spécifique au-delà du nom de sous-dossier
 `ryzom_forgery`.
- Notez que `ForgeryApp` (`app.py`) utilise un troisième emplacement,
 distinct de ces deux-là, pour la géométrie de fenêtre :
 `~/.ryzom_forgery/<titre>.json` (`app.py`, `_WINDOW_GEOMETRY_DIR`) —
 un chemin fixe `~/.ryzom_forgery`, pas dérivé de `config_dir` ni
 `cache_dir`. Ce n'est pas un bug documenté dans le code lu, juste une
 incohérence à noter : la géométrie de fenêtre n'est ni dans le dossier de
 config standard ni dans le cache standard.
