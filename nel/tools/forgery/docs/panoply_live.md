# panoply_live.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/panoply_live.py` (~57 lignes)

## Rôle

Gère la logique de **fraîcheur** (faut-il recalculer une texture panoply
en live ou la variante précuite sur disque suffit-elle ?) et la
**mémoïsation en mémoire** des résultats de coloration live, pour éviter
de recalculer une texture identique à chaque frame — Phase A Step 3 du
chantier "génération live des textures Panoply" (`panoply_live.py`,
voir `.todo/forgery-object-editor.md`). Ce module ne fait aucun I/O
disque de lui-même (`panoply_live.py`) : les appelants lui fournissent
des références déjà résolues, "duck-typées" comme `search_paths.FoundEntry`
(tout objet exposant `.cache_stat -> (mtime, size)`, le même duck type
que le cache de scan de `search_paths.py`).

## API principale

- `is_baked_stale(baked_ref, base_ref, mask_refs)` — `panoply_live.py` : `True` si la variante précuite doit être remplacée par un recalcul live. Cas : `baked_ref is None` (rien résolu sur disque), ou mtime de la texture de base plus récent que celui du fichier précuit, ou mtime d'un des masques (`mask_refs`, un par axe) plus récent. Chaque comparaison lit `cache_stat` à la volée — aucune donnée n'est mise en cache par cette fonction elle-même.
- `LiveColorizeCache` — `panoply_live.py` : classe de mémoïsation.
  - `make_key(base_name, dims, base_ref, mask_refs)` (méthode statique, `panoply_live.py`) : construit une clé composée de `(base_name, axis_items triés, mtime de la base, mtimes triés de chaque masque)`. Clé sur les mtimes des sources plutôt que sur juste `base_name`+axes : un base texture ou un masque édité fait naturellement rater le cache au lieu de servir un résultat périmé, sans invalidation explicite nécessaire.
  - `get(key)` / `set(key, image)` (`panoply_live.py`) : accès simple dict, aucune éviction par entrée.
  - `clear()` (`panoply_live.py`, ajouté 2026-08-29) : vide tout le cache d'un coup — nécessaire quand quelque chose d'**extérieur** à la clé change ce qu'une combinaison doit rendre, typiquement l'édition du `panoply.cfg` du workspace (les paramètres de couleur eux-mêmes ne font pas partie de `make_key()`) — voir `object_editor._on_panoply_cfg_settled()`.

## Utilisation

Consommé uniquement par `object_editor.py` (import `panoply_live.py`).
Deux usages distincts :

- `LiveColorizeCache` est instancié une fois par éditeur (`self._live_panoply_cache = panoply_live.LiveColorizeCache`, `object_editor.py`), utilisé dans `_ensure_live_panoply_texture` (`object_editor.py`) pour éviter de recolorer une combinaison base/masques/axes déjà vue.
- `is_baked_stale` est appelé au même endroit (`object_editor.py`) pour décider s'il faut même tenter un recalcul live, ou si le fichier précuit trouvé sur disque (par ex. livré avec les données du jeu) est encore valide tel quel.

Dans le pipeline global, `panoply_live.py` est la couche d'orchestration
qui entoure `panoply_colorize.py` (le calcul) et `panoply_texture.py` (la
conversion Panda3D) : elle décide *si* et *avec quel résultat mémoïsé* ces
deux modules doivent être invoqués, sans jamais lire ou écrire de pixels
elle-même.

## Points notables / pièges

- Aucune éviction **par entrée** : le commentaire du docstring (`panoply_live.py`) justifie ce choix par le fait que le nombre de combinaisons distinctes réellement vues en session reste petit en pratique (quelques choix d'axes sur les textures d'un shape) — c'est un choix de design assumé, pas un oubli. `clear()` (tout ou rien) existe pour l'invalidation externe (voir ci-dessus), pas pour une éviction fine.
- `make_key()` ne dépend PAS des paramètres de couleur (hue/lightness/...), seulement de `base_name`/axes/mtimes des sources — un `panoply.cfg` édité est donc invisible à ce cache par sa seule clé ; c'est `object_editor.py` qui doit appeler `clear()` explicitement (via le `WorkspaceWatcher` sur `panoply.cfg`, voir `docs/apps/object_editor.md`).
- La détection de péremption ne compare que des mtimes, jamais un hash de contenu — un fichier réécrit avec un contenu identique mais un mtime plus récent est traité comme périmé (recalcul inutile mais pas incorrect).
- `is_baked_stale(None, ...)` renvoie toujours `True` (`panoply_live.py`) : l'absence de variante précuite sur disque force systématiquement le chemin live, quel que soit l'état des autres références.
- Ce module ne connaît rien du format des images ni des paramètres de couleur — son typage est délibérément générique (duck typing sur `cache_stat`), le découplant de `search_paths.py` et de `panoply_texture.py`.
