# dds_export

**Fichier :** `nel/tools/forgery/ryzom_forgery/dds_export.py` (~280 lignes, créé le
2026-08-27, pas encore committé au moment de l'écriture de ce doc)

## Rôle

Écrit des fichiers `.dds` chargeables par le client Ryzom, à partir d'un bitmap RGBA —
équivalent Python (Panda3D + numpy) de `nel/tools/3d/tga_2_dds/tga2dds.cpp` +
`nel/tools/3d/s3tc_compressor_lib/s3tc_compressor.cpp`. Contrairement à l'original (qui
utilise libsquish), ce module ne vise pas une compression DXT bit-exacte — seul le
conteneur DDS et le format de bloc DXT doivent être corrects, puisque c'est tout ce que
le lecteur DDS du client (`CBitmap::readDDS`, `nel/src/misc/bitmap.cpp`) vérifie.

## API principale

- `build_dds(rgba, algo, build_mipmaps=False, reduce=0)` — assemble un fichier `.dds`
 complet (bytes) depuis un array HxWx4 uint8. `algo` ∈ `{DXT1, DXT1A, DXT3, DXT5}`.
- `_build_dds_header` — layout binaire exact de `DDS_HEADER`/`DDS_PIXELFORMAT`,
 vérifié champ par champ contre `s3tc_compressor.h` et contre les offsets lus par
 `CBitmap::readDDS` (`bitmap.cpp`). Validé byte-for-byte contre un vrai DDS de
 production (`tr_mo_h07_albino.dds`).
- `_build_mip_chain` — port de `CBitmap::buildMipMaps` (`bitmap.cpp`) : filtre
 box 2x2 avec arrondi `+2`, uniquement pour dimensions puissance de 2.
- `_reduce_size` — port de `dividSize` (`tga2dds.cpp`) : filtre box 2x2 **sans**
 arrondi (`>>2` strict), appliqué `reduce` fois (option `-r`, 0-8).
- `_compress_level` — compresse un niveau via `Texture.compress_ram_image` de
 Panda3D (block-conformant DXT1/DXT3/DXT5, vérifié sur la machine réelle).
- `load_rgba(path, grayscale_as_luminance=False)` — charge un TGA/PNG en RGBA top-down.
 Deux pièges corrigés ici :
  - `Texture.load(PNMImage)` flippe verticalement les lignes (RAM image bottom-up façon
 OpenGL vs PNMImage/DDS top-down) — `load_rgba` annule ce flip juste après le
 chargement.
  - Interprétation du niveau de gris pour une source mono-canal : par défaut (comme
 `CBitmap::_LoadGrayscaleAsAlpha=true`), RGB forcé à blanc + alpha = le gris (masque)
 ; avec `grayscale_as_luminance=True` (option `-g`), RGB = gris répété + alpha=255
 (luminance visible).
- `pick_default_algo(rgba)` — même heuristique par défaut que `tga2dds.cpp` : DXT5 si un
 pixel non opaque existe, DXT1 sinon.

Ce module est une **bibliothèque pure**, sans CLI (même convention que
`shape_export.py`/`shape_import.py`) — la CLI vit dans `apps/dds_export.py`.

## Utilisation

- CLI : `apps/dds_export.py` (voir `docs/apps/dds_export.md`) — `input`, `-o/--output`,
 `-a/--algo {1,1a,3,5}`, `-m/--mipmap`, `-g/--grayscale`, `-r/--reduce`.
- Pas encore intégré dans Patina (`apps/object_editor.py` ne l'importe pas au moment de
 l'écriture de ce doc) — prérequis du chantier `patina-tex-dds-autoexport`
 (`/repos/project-todos/ryzom-core/patina-tex-dds-autoexport.md`).
- Utilisé pour corriger manuellement 3 fichiers `objects/occ_stuff/anlor/
 halloween_mo_statue_0{1,2,3}_spec.dds` de `ryzom-data` qui étaient en réalité des PNG
 renommés en `.dds` (voir convention `_spec` ci-dessous).

## Points notables / pièges

- **Convention `_spec` (specular) observée en production**, cross-checkée contre
 plusieurs vrais `*_spec.dds` (`objects/ge_partylamps_spec.dds`,
 `fauna_maps/tr_mo_kamiguard_spec.dds`, `fauna_maps/fo_canitree_spec.dds`) : source
 mono-canal, `algo=DXT1`, mipmaps complets, **chargée avec `-g`** (luminance) — sinon
 l'intensité finit dans un canal alpha que le shader spéculaire ne lit pas.
- DXT1A partage le même header que DXT1 (`dwRGBBitCount` reste à 0) — incomplétude
 délibérément conservée pour coller au comportement de l'original
 (`s3tc_compressor.cpp`, "TODO: add special headers flags for DXTC1a").
- Toujours utiliser `build_mipmaps=True` pour des textures de modèles 3D (vus à distance
 variable) — un fichier `.dds` régénéré sans `-m` par erreur donnera un résultat
 visuellement dégradé par rapport à l'original Nevrax (constaté sur
 `fauna_maps/tr_mo_h07_albino.dds`, qui n'avait pas de mipmap suite à un oubli de `-m`
 lors de sa régénération).
