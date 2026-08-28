# panoply_config.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/panoply_config.py` (~92 lignes)

## Rôle

Fournit la table de correspondance `(axe, color_id[, race]) -> paramètres
HSL réels`, en lisant les constantes de production figées dans
`panoply_colors.toml` (fichier bundlé avec Forgery, voir son en-tête pour
la provenance : snapshot du 2026-08-19 des vrais
`panoply_common.cfg`/`panoply_<race>.cfg` de `ryzom-data`,
`panoply_colors.toml`). Ce module ne fait aucun I/O au-delà de la
lecture de ce fichier unique bundlé (`panoply_config.py`), afin que ni
lui ni ses appelants ne dépendent des chemins de recherche de l'utilisateur
couvrant l'arborescence `leveldesign/` de `ryzom-data`. Les paramètres
qu'il renvoie alimentent directement les arguments
hue/lightness/saturation/luminosity/contrast de
`panoply_colorize.convert_bitmap`.

## API principale

- `RACE_PREFIX_TO_TABLE` — `panoply_config.py` : préfixe de 2 lettres en tête d'un nom de texture de base (`tr`, `fy`, `ma`, `zo`, `ge`) → nom de table TOML correspondante (`tryker`, `fyros`, `matis`, `zorai`, `generique`), qui reprend le nom des fichiers `panoply_<race>.cfg` sources.
- `_COMMON_AXES = ("skin", "user")` / `_RACE_AXES = ("hair", "eyes")` — `panoply_config.py` : distingue les axes communs à toutes les races (table racine `panoply_common.cfg`) des axes définis par race (table imbriquée `panoply_<race>.cfg`) — correspond à `panoply.AXES`.
- `ColorParams` (dataclass frozen) — `panoply_config.py` : `id, hue, lightness, saturation, luminosity, contrast` — une entrée de couleur prête à être passée à `convert_bitmap`.
- `_load_doc` / `_DOC` — `panoply_config.py` : parse `panoply_colors.toml` via `tomlkit` une seule fois au chargement du module (variable de module `_DOC`).
- `_table_for(axis, race)` — `panoply_config.py` : résout la sous-table TOML pertinente selon que l'axe est commun ou par-race ; renvoie `None` si l'axe ne s'applique pas à cette race ou si `race` est `None` pour un axe par-race.
- `get_color_params(axis, color_id, race=None)` — `panoply_config.py` : cherche l'entrée `color_id` dans la table résolue et construit un `ColorParams`. Renvoie `None` si l'axe ne s'applique pas du tout à cette race (ex. zorai n'a pas d'axe `eyes`, generique n'a ni `hair` ni `eyes`) ou si le `color_id` n'existe pas dans la table.
- `available_color_ids(axis, race=None)` — `panoply_config.py` : liste tous les `color_id` définis pour un axe (optionnellement restreint à la table hair/eyes d'une race) — utile pour construire un sélecteur UI.

## Utilisation

Consommé uniquement par `object_editor.py` (import `panoply_config.py`)
dans `_ensure_live_panoply_texture` (`object_editor.py+`) :

- `panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower)` (`object_editor.py`) déduit la race à partir des 2 premières lettres du nom de texture.
- `panoply_config.get_color_params(axis, panoply.color_id_for(axis, dims[axis]), race)` (`object_editor.py`) récupère les paramètres HSL réels pour chaque axe sélectionné, en s'appuyant sur `panoply.color_id_for` pour construire le `color_id` attendu.

Dans le pipeline global, ce module est le pont entre `panoply.py`
(vocabulaire des axes/valeurs) et `panoply_colorize.py` (calcul pixel par
pixel) : il traduit une sélection abstraite (axe + valeur numérique/code
race) en vrais paramètres de teinte issus des données de production, sans
jamais toucher aux pixels lui-même.

## Points notables / pièges

- Le fichier `panoply_colors.toml` est une **copie figée** des configs de production, pas une lecture live de `ryzom-data` : si la palette réelle change côté données du jeu, ce fichier doit être régénéré à la main (`panoply_colors.toml`) — aucune automatisation de resynchronisation n'existe dans ce module.
- `get_color_params` renvoie `None` dans deux cas distincts (axe non applicable à la race vs. `color_id` introuvable) sans les différencier dans la valeur de retour — un appelant qui veut distinguer les deux cas doit passer par `available_color_ids` en amont.
- `race` est ignoré pour les axes `skin`/`user` mais **requis** (sinon `None` implicite via l'argument par défaut) pour `hair`/`eyes` — passer `race=None` pour un axe par-race renvoie systématiquement `None` (`panoply_config.py`), pas une erreur.
- Le module ne valide pas le format de `panoply_colors.toml` au-delà de ce que `tomlkit` impose — une entrée malformée provoquerait une `KeyError`/`ValueError` lors de l'accès (`entry["id"]`, `float(entry["hue"])`), pas une erreur de chargement anticipée.
