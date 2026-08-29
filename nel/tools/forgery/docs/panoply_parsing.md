# panoply.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/panoply.py` (~123 lignes)

## Rôle

Ce module contient la logique pure de nommage/parsing du système panoply
(voir `nel/tools/pynel/panoply.md` pour le contexte complet du système
d'origine) : aucun I/O, aucune dépendance UI. Il sait reconnaître, dans un
nom de fichier de texture, les suffixes de couleur ajoutés par
`panoply_maker` pour chacun des 4 axes réellement observés dans les données
de production (`panoply.py`) :

- `skin` — teinte de peau liée à la race (FY/MA/TR/ZO), partagée par tous
 les objets ;
- `user` — couleur "craft" choisie par le joueur (U1-U8), partagée aussi ;
- `hair`/`eyes` — couleurs cheveux/yeux (H1-H6/E1-E8), définies par race
 (une pièce d'armure n'a jamais ces masques).

Une texture ne porte que les axes pour lesquels un masque existait
réellement au moment du build (`panoply.py`) — l'absence d'une
variante pour un axe donné signifie simplement qu'aucun masque n'a jamais
existé pour cet axe sur cette texture, pas une donnée manquante.

Le fichier source de vérité en production est `panoply_files.txt` (livré
dans `characters_maps_hr.bnp`), une liste de noms de fichiers, une par
ligne — c'est ce texte brut que `parse_panoply_files` consomme.

## API principale

- `RACES = ("fy", "ma", "tr", "zo")`, `AXES = ("skin", "user", "hair", "eyes")` — `panoply.py` : constantes de référence pour les 4 axes et les 4 codes de race.
- `_TOKEN_PATTERNS` / `_AXIS_LETTER` — `panoply.py` : regex reconnaissant un token de nom de fichier par axe (`fy|ma|tr|zo` pour skin, `u\d+`/`h\d+`/`e\d+` pour les 3 autres) et la lettre utilisée pour reconstruire un nom (`U`, `H`, `E`).
- `_classify_token(token)` — `panoply.py` : renvoie `(axis, value)` pour un token donné (`value` = code de race en minuscule pour `skin`, entier pour les autres) ou `None` si aucun pattern ne correspond.
- `parse_panoply_files(text)` — `panoply.py` : parse le contenu brut de `panoply_files.txt` et retourne `{base_texture_stem: {axis: [valeurs triées]}}`. Pour chaque ligne dont l'extension est dans `search_paths.TEXTURE_FALLBACK_EXTENSIONS` (`.tga`/`.png`/`.dds` — corrigé 2026-08-29, codé en dur sur `.tga` seul auparavant, ce qui ignorait silencieusement **toutes** les lignes d'un vrai `panoply_files.txt` de production comme celui de `ryzom-data/final_bnps/characters_maps_hr/`, qui liste des `.dds`), dépile les tokens `_`-séparés depuis la fin tant qu'ils correspondent à un axe encore non vu pour cette ligne ; s'arrête au premier token non reconnu ou déjà vu. Une ligne sans suffixe reconnu (texture de base non panopliée, listée aussi) ne contribue à rien.
- `color_id_for(axis, value)` — `panoply.py` : construit le token `color_id` façon `panoply_common.cfg`/`panoply_<race>.cfg` (ex. `"tr"` → `"TR"`, `("user", 4)` → `"U4"`). C'est exactement ce que `object_editor.py`'s `_ensure_live_panoply_texture` doit passer à `panoply_config.get_color_params`.
- `variant_file_name(base_texture_name, **dims)` — `panoply.py` : reconstruit le nom de fichier réel d'une variante à partir du nom de base et des valeurs d'axes fournies (dans l'ordre `AXES`), en n'ajoutant que les axes effectivement passés. Ne touche jamais aux données du matériau du shape — sert uniquement à résoudre quel fichier charger pour le rendu/preview.

## Utilisation

Deux consommateurs identifiés par grep des imports :

- `ryzom_forgery/search_paths_dialog.py` : appelle `panoply.parse_panoply_files` (`search_paths_dialog.py`) dès qu'un fichier nommé `panoply_files.txt` est trouvé parmi les chemins de recherche indexés, et stocke le résultat comme `panoply_variants` du workspace/de l'external index. Expose `panoply_variants_for(base_texture_name)` (`search_paths_dialog.py`) pour interroger ce dictionnaire.
- `ryzom_forgery/apps/object_editor.py` : utilise `panoply.AXES`, `panoply.RACES`, `panoply.color_id_for` et `panoply.variant_file_name` dans tout le pipeline de résolution/coloration live des textures (`_panoply_dims_for`, `_resolve_panoply_texture_name`, `_ensure_live_panoply_texture`, `_draw_global_panoply_section`, `_draw_panoply_masks_for` — voir `object_editor.py` et suivants).

Articulation entre les 6 modules : `panoply.py` fournit le vocabulaire
(axes, parsing de noms, reconstruction de noms) que tous les autres
modules du pipeline utilisent : `panoply_config.py` en dérive les codes
`color_id` pour aller chercher les paramètres HSL réels ;
`panoply_colorize.py` applique ces paramètres pixel par pixel ;
`panoply_texture.py` fait le pont avec Panda3D ; `panoply_live.py`
orchestre la fraîcheur/le cache autour de tout ça. `object_editor.py` est
le seul point d'orchestration appelant les 5 modules ensemble.

## Points notables / pièges

- Le parsing de `parse_panoply_files` dépile les tokens **dans l'ordre où ils apparaissent** dans le nom de fichier, sans supposer un ordre fixe (skin/user/hair/eyes) même si c'est l'ordre réel de sortie de `panoply_maker` (`panoply.py`) — le code n'en dépend pas.
- Un axe déjà vu dans la boucle de dépilement arrête le parsing (`classified[0] in found` → `break`, `panoply.py`) : une même ligne ne peut donc pas avoir deux valeurs pour le même axe.
- `variant_file_name` n'exige pas que tous les axes soient fournis — un axe absent de `dims` (valeur `None`) est simplement omis (`panoply.py`), ce qui permet de résoudre correctement une texture qui ne porte pas tous les axes (ex. une texture sans masque `hair`).
- `color_id_for` traite `skin` différemment des 3 autres axes (juste `.upper` du code de race au lieu de lettre+chiffre) — reflète fidèlement le format réel des `color_id` dans les `.cfg` sources, pas une simplification du module.
