# panoply_colorize.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/panoply_colorize.py` (~180 lignes)

## Rôle

Portage pur NumPy de l'algorithme de recoloration réellement utilisé par
`panoply_maker` pour précuire les variantes panoply — traduction de
`nel/tools/3d/panoply_maker/color_modifier.cpp`
(`CColorModifier::evalBitmapStats`/`convertBitmap`) et des conversions HLS
de `nel/src/misc/rgba.cpp` (`CRGBA::convertToHLS`/`buildFromHLS`,
`CBGRA::blendFromui`) — voir `panoply_colorize.py`. Aucun I/O, aucune
dépendance Panda3D : le module opère sur de simples tableaux NumPy
`uint8`, ce qui le rend testable de façon isolée (`panoply_colorize.py`).
Il constitue le cœur de calcul de la coloration **live** (à la volée, en
mémoire) de textures panoply dans Forgery, par opposition à la coloration
offline précuite par `panoply_maker` en C++ (voir
`.todo/forgery-object-editor.md`, chantier "génération live des textures
Panoply", Phase A Step 1).

## API principale

- `_BLEND_DIVISOR = 256.0` — `panoply_colorize.py` : poids de mélange pour un pixel de masque à couverture totale ; port du diviseur `>>8` de `CBGRA::blendFromui` (un masque à 255, pas 256, est le plafond pratique de "pleinement appliqué").
- `rgb_to_hls(rgb)` — `panoply_colorize.py` : port vectorisé de `CRGBA::convertToHLS`. Prend un array flottant `[...,3]` dans `[0,1]`, renvoie `(h, l, s, achromatic)`.
- `_hls_value(h, v1, v2)` — `panoply_colorize.py` : port du helper local `HLSValue` de `rgba.cpp` ; note explicite que c'est un simple wrap conditionnel, pas un vrai modulo, fidèle à l'original.
- `hls_to_rgb(h, l, s)` — `panoply_colorize.py` : port de `CRGBA::buildFromHLS`, renvoie un array flottant `[...,3]` dans `[0,1]`.
- `_to_uint8(float_pixels_0_255)` — `panoply_colorize.py` : cast en `uint8` par troncature (pas arrondi), reproduisant la sémantique C++ sur des valeurs déjà clampées.
- `_brightness_contrast(intensity_u8, luminosity, contrast, mean_grey)` — `panoply_colorize.py` : port de `CalcBrightnessContrast`.
- `eval_bitmap_stats(rgb_u8, mask_u8)` — `panoply_colorize.py` : port de `CColorModifier::evalBitmapStats`. Calcule les moyennes pondérées par le masque de teinte (H, moyenne circulaire), saturation (S), luminosité (L) et niveau de gris, sur toute l'image.
- `convert_bitmap(current_rgb_u8, mask_u8, hue, lightness, saturation, luminosity, contrast)` — `panoply_colorize.py` : port de `CColorModifier::convertBitmap`. Applique un axe de coloration : calcule le delta de teinte cible vs. teinte actuelle du masque, décale H/L/S, applique luminosité/contraste, puis mélange le résultat avec l'image d'entrée selon la force du masque (`coef`). L'alpha n'est pas inclus dans le calcul (préservé séparément par l'appelant).
- `colorize(base_rgba_u8, axis_masks)` — `panoply_colorize.py` : applique `convert_bitmap` une fois par `(mask, params)` dans `axis_masks`, en chaînant la sortie RGB de chaque axe comme entrée du suivant (comme `panoply_maker.cpp` qui applique successivement chaque masque sur un même `resultBitmap` en place). Préserve le canal alpha de `base_rgba_u8` tout du long.

## Utilisation

Consommé uniquement par `ryzom_forgery/apps/object_editor.py` (import
`panoply_colorize.py`), dans `_ensure_live_panoply_texture`
(`object_editor.py` et suivants) : pour chaque axe présent dans la
sélection panoply courante, l'éditeur récupère un masque déjà décodé
(`panoply_texture.ref_to_rgba_array`) et les paramètres HSL correspondants
(`panoply_config.get_color_params`), assemble la liste `axis_masks`, puis
appelle `panoply_colorize.colorize(base_rgba, axis_masks)`
(`object_editor.py`). Le résultat est ensuite reconverti en texture
Panda3D via `panoply_texture.rgba_array_to_texture` et mis en cache par
`panoply_live.LiveColorizeCache`.

Dans le pipeline global : `panoply.py` détermine quels axes/valeurs
s'appliquent à une texture donnée, `panoply_config.py` traduit ces
valeurs en paramètres HSL réels, et c'est `panoply_colorize.py` qui fait
le calcul pixel par pixel proprement dit — le seul module des six qui
réimplémente le cœur mathématique du C++ d'origine.

## Points notables / pièges

- **Divergence assumée documentée dans le docstring du fichier** (`panoply_colorize.py`) : le module ne vise pas une parité bit-exacte avec le C++ (accepté avec l'utilisateur, aucune divergence perceptible attendue sur des couleurs de jeu réelles). Une différence de comportement délibérée : `eval_bitmap_stats` calcule la moyenne de teinte via une moyenne circulaire vectorielle standard (sin/cos) au lieu de l'astuce "running-unwrap" dépendante de l'ordre de `color_modifier.cpp` — la forme vectorisée exige une formule indépendante de l'ordre, et c'est la méthode manuel/textbook pour moyenner des grandeurs circulaires, pas une approximation de l'original.
- `grey_level` est explicitement tronqué en entier (`float(int(grey_level))`, `panoply_colorize.py`) pour reproduire la sémantique `uint8` de `greyLevel` côté C++.
- Le fichier note explicitement (`panoply_colorize.py`) que le branchement de ce module dans la résolution de textures d'`object_editor` était, au moment de son écriture, une étape ultérieure — c'est désormais fait, voir `_ensure_live_panoply_texture`.
- Aucune gestion de cache/fraîcheur ici — c'est délibérément séparé dans `panoply_live.py` ; ce module est un pur calcul stateless.
