# Panoply — système de recoloration de textures Ryzom

## Résumé

`panoply_maker` (`nel/tools/3d/panoply_maker/`) est l'outil **offline** qui
génère, au build des données, toutes les variantes de couleur possibles
d'une texture (peau, armures/objets, yeux, cheveux). En haute qualité
(personnage/objet en gros plan), le client ne recolore rien au runtime : il
sélectionne le bon fichier pré-généré selon les "color slots" du
personnage/objet.

Il existe cependant un vrai système de recoloration **runtime** dans le
moteur, mais réservé aux personnages en LOD (loin de la caméra) — voir
[section dédiée plus bas](#recoloration-runtime-existante--hls_bank_maker--lod).

Ce choix est justifié par le commentaire d'origine dans `panoply_maker.cpp` :
> Not all hardware allow it to manage that at runtime (lack for palettized
> textures or pixel shaders...)

— une contrainte hardware d'époque qui n'a plus lieu d'être aujourd'hui.

## Principe de génération (`panoply_maker`)

Pour une texture de base, ex. `armor.tga` :

1. On fournit un ou plusieurs **masques en niveaux de gris**, nommés
   `armor_<maskExt>.tga` (ex. `armor_user.tga`). Le niveau de gris joue le
   rôle d'un alpha : il indique la force de l'effet de recoloration à
   chaque pixel.
2. Un fichier de config décrit, pour chaque `maskExt`, une **liste de
   modificateurs de couleur** (`CColorModifier` — voir `color_modifier.h`) :
   - `Hue`, `Saturation`, `Lightness` — teinte cible
   - `Luminosity`, `Contrast` — appliqués après
   - `ColID` — identifiant court utilisé dans le nom de fichier de sortie
3. `panoply_maker` génère **une texture de sortie par combinaison** de tous
   les masques présents sur cette texture (boucle combinatoire dans
   `BuildColoredVersionForOneBitmap`), chaque teinte étant appliquée par un
   vrai ajustement HSL du pixel (`CColorModifier::convertBitmap`), pas un
   mélange alpha.

`nel/tools/3d/panoply_maker/panoply.cfg` n'est qu'un **exemple/config
d'outil** (masks `skin`/`user`/`hair`, 6 variantes chacun dans l'exemple) —
ce n'est pas la configuration réellement utilisée en production. Le fichier
de production (`panoply_files.txt`, lu par `CColorSlotManager::init()` côté
client) n'est pas présent dans ce dépôt : il est livré avec les données
client.

## Les "color slots" réellement utilisés côté client

Ce qui est réellement figé en dur côté client (`initColorSlotManager()`,
`ryzom/client/src/color_slot_manager.cpp:63-85`) :

| Slot   | Nb variantes | Contenu réel                                             |
|--------|--------------|-----------------------------------------------------------|
| `skin` | 4 (FY/MA/TR/ZO) | couleur de peau du personnage, liée à sa **race** (Fyros/Matis/Tryker/Zoraï) |
| `user` | 8 (U1-U8)    | couleur de l'objet/armure choisie par le joueur           |
| `eyes` | 8 (E1-E8)    | couleur des yeux                                           |
| `hair` | 6 (H1-H6)    | couleur des cheveux                                        |

`CCharacterCL::applyColorSlot()` (`character_cl.cpp:539-561`) applique le
slot 0 ("Skin") avec `skin()` (race du perso), et le slot 1 ("User Color")
avec `userColor` — c'est bien ce slot **user**, pas le slot skin, qui sert
à la coloration des armures/objets.

Il existe aussi un mécanisme générique de config (`addSlotsFromConfigFile()`,
`color_slot_manager.cpp:320-369`) qui peut lire un `.cfg` avec des clés
`mask_extensions`/`<mask>_color_id` similaires à `panoply_maker.cfg` — mais
en production ce sont les tableaux figés ci-dessus qui sont utilisés.

### Ce que ça implique pour une armure

Une armure qui a un masque `skin` (pour que les zones de peau visibles à
travers l'armure suivent la race du perso) **et** un masque `user` (pour la
couleur choisie par le joueur) donne au maximum :

```
4 (skin) × 8 (user) = 32 textures pré-générées pour cet item
```

Un item n'a **jamais plusieurs couleurs `user` indépendantes en même temps**
— une seule couleur `user` (0 à 7) s'applique à tout l'item.

## `UserColor` — la donnée persistée

- `ryzom/common/src/game_share/inventories.h:281` : `UserColor` est une
  valeur de l'enum `TItemPropId` (identifiant de propriété d'objet en BDD),
  pas un type dédié avec une plage de bits explicite.
- Côté client, `dbctrl_sheet.cpp:913` : `_UserColor` pointe sur le nœud BDD
  `"...:USER_COLOR"`. `getItemColor()`/`setItemColor()`
  (`dbctrl_sheet.h:579-581`) lisent/écrivent cette valeur.
- `updateArmourColor(sint8 col)` (`dbctrl_sheet.cpp:4820-4847`) n'applique
  la couleur que si **`col >= 0 && col <= 7`** (`dbctrl_sheet.cpp:4825`) —
  confirmant la plage **0 à 7 (8 valeurs)**, cohérente avec le tableau
  `_ArmourColor[8]` (`dbctrl_sheet.h:827`, dimensionné via
  `RM_COLOR::NumColors`).

## Cache de build (`.hlsinfo`)

Un fichier `.hlsinfo` par texture source est écrit dans `hls_info_path`
(voir `hls_bank_texture_info.h`). Il contient :

- une version basse résolution compressée de la texture source
- les valeurs Hue/Sat/Lum réellement appliquées pour chaque instance générée

Il sert aussi de cache de build : `CheckIfNeedRebuildColoredVersionForOneBitmap`
compare les dates de modification de la source et des masques contre le
cache pour éviter de régénérer une texture si rien n'a changé.

## Config de l'outil `panoply_maker.cfg`

Clés principales lues par `panoply_maker.cpp` (à ne pas confondre avec le
fichier de production `panoply_files.txt`, absent du dépôt) :

- `input_path`, `output_path`, `cache_path`, `hls_info_path`
- `output_format` (`tga` ou `png`, défaut `tga`)
- `bitmap_extensions` — extensions de fichiers à traiter
- `default_separator` — séparateur dans les noms de fichiers générés (défaut `_`)
- `low_def_shift` — facteur de réduction de la version basse-def stockée dans `.hlsinfo` (défaut 3, soit 512×512 → 64×64)
- `optimize_textures` — si activé, avertit si une texture RGBA pourrait être optimisée en RGB (alpha uniforme à 255)
- `mask_extensions` + pour chaque extension `<ext>_luminosities`, `<ext>_constrasts`, `<ext>_hues`, `<ext>_lightness`, `<ext>_saturations`, `<ext>_color_id` — définissent la liste des `CColorModifier` pour ce masque

## Recoloration runtime existante : `hls_bank_maker` + LOD

`nel/tools/3d/hls_bank_maker/hls_bank_maker.cpp` prend tous les `.hlsinfo`
générés par `panoply_maker` (chacun contient une version basse-def
compressée DXTC5 de la texture source + les deltas Hue/Lum/Sat de chaque
variante, compressés en `CHLSColorDelta`: `DHue` 0-255, `DLum`/`DSat`
-127..+127) et les concatène en un seul fichier compilé `.hlsbank`
(`CHLSTextureBank`, `nel/include/nel/3d/hls_texture_bank.h`).

Ce fichier est chargé au runtime, mais uniquement pour les **personnages en
LOD** (faible détail, loin de la caméra) :

- `ryzom/client/src/init_main_loop.cpp:914` : `Driver->loadHLSBank("characters.hlsbank")`,
  juste après l'init du LOD Character Manager.
- `nel/src/3d/async_texture_manager.cpp:196-204,570-577`
  (`CAsyncTextureManager`) : quand une texture demandée est trouvée dans
  `HLSManager` (`CHLSTextureManager::findTexture`), au lieu de charger un
  fichier précuit, elle est **recolorée en mémoire** via
  `HLSManager.buildTexture()` — qui appelle
  `CHLSColorTexture::colorizeDXTCBlockRGB()`
  (`nel/include/nel/3d/hls_color_texture.h:132`), une manipulation directe
  des blocs DXTC5 compressés (pas un shader GPU, pas de décompression
  complète).
- `setupMaxHLSColoringPerFrame()` limite le budget de recoloration à 20 Ko
  par frame par défaut, pour lisser le coût CPU.

Ce compromis a du sens : les personnages lointains n'ont pas besoin de la
texture haute qualité précuite par `panoply_maker` — le moteur recolore à
la volée une version basse-def (`low_def_shift`, typiquement 512×512 →
64×64) directement en CPU, économisant beaucoup d'espace disque pour tout
ce qui est loin. En gros plan, c'est `panoply_maker` (précuit, haute
qualité) qui reste utilisé — la manipulation directe de blocs DXTC n'a pas
la finesse voulue de près.

## Ne pas confondre avec `_usercolor` (tga2dds) — jamais utilisé en production

`nel/tools/3d/tga_2_dds/tga2dds.cpp` contient un mécanisme **distinct et
non utilisé en production** : un fichier annexe `<nom>_usercolor.tga`
fusionné pixel par pixel à la conversion DDS pour précalculer un canal
alpha de teinte (préservant la luminance du pixel source). Cette approche
suppose un blend fait au runtime dans le shader — jamais implémentée côté
client, contrairement à `panoply_maker`/`color_slot_manager` qui, eux, sont
le système effectivement utilisé en jeu.

## Réimplémentation moderne

`nel/tools/forgery/ryzom_forgery/panoply*.py` — pipeline Python équivalent
pour l'outil Forgery.
