# panoply_bake.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/panoply_bake.py`

Status : **cross-validé contre le vrai `panoply_maker.exe`** (2026-08-29, voir
`docs/panoply_maker.md`) -- `bake_flat()` (le chemin exercé par la
cross-validation) produit des `.tga` quasi byte-exacts contre le binaire réel.
`bake_and_write()` (workflow `build/`) reste **écrit, pas encore validé** de
la même manière (pas d'équivalent `build/` côté binaire natif à comparer).

## Rôle

Glue réutilisable entre `panoply_maker.py` (math pure : `resample`,
`colorize_exact`, `generate_color_combinations`) et
`dds_export.py`/`pynel.hls_bank_texture_info` (I/O : compression DXT5,
écriture TGA/PNG, sérialisation `.hlsinfo`) — le pipeline complet
`BuildColoredVersionForOneBitmap` du vrai `panoply_maker.cpp`, sans logique
de résolution de fichiers propre (contrairement à `search_paths.py`) :
`apps/panoply_maker.py` (dossier à plat) et `object_editor.py` (index
`search_paths_dialog`) résolvent sources/masques très différemment, donc ce
module ne prend que des pixels déjà chargés + un callback `mask_loader`
fourni par l'appelant — même séparation que `panoply_texture.py` (décodage
Panda3D) vs `panoply_colorize.py` (math pure).

## API principale

- `load_mask_luminance(path)` — charge un masque niveaux de gris (TGA/PNG)
  en `HxW` uint8, via `dds_export.load_rgba(..., grayscale_as_luminance=True)`.
- `axes_for_source(stem, race=None)` — construit tous les `ColorMaskAxis`
  candidats (`skin`, `user`, `hair`, `eyes`, dans cet ordre) que
  `panoply_config.py` définit pour cette race — pas encore filtré par ce qui
  existe réellement sur disque pour cette source précise (voir
  `build_active_masks`). Mode **autonome** uniquement — le mode `.cfg`
  explicite utilise `panoply_maker.build_masks_from_config()` à la place
  (les vrais `.cfg` de production ne connaissent qu'une race à la fois, pas
  besoin de ce paramètre `race`).
- `build_active_masks(axes, mask_loader)` — ne garde que les axes dont
  `mask_loader(mask_ext)` renvoie un masque réel (`None` sinon) — port de
  l'étape 3 de `BuildColoredVersionForOneBitmap` ("seules les extensions de
  masque avec un fichier trouvé deviennent actives").
- `bake_source(base_rgba_u8, active_masks, low_def_shift=3, default_separator="_")`
  — construit le `SrcBitmap` bas-def du `.hlsinfo` (`resample` +
  `dds_export.build_dds(..., DXT5, build_mipmaps=True)`), ses `Masks`
  (`resample`é à la même taille bas-def), et lance
  `generate_color_combinations()` pour les `Instances` + les images pleine
  résolution. Renvoie `(HLSBankTextureInfo, [(name_suffix, result_rgba_u8), ...])`
  — `instances[i].name` est laissé vide, à remplir par l'appelant (il connaît
  seul le nom de fichier final : stem + suffixe + `output_format`). Aucun
  I/O disque ici.
- `_write_variants(stem, info, combos, output_dir, output_format)` — écrit
  chaque image de `combos` dans `output_dir` (`dds_export.save_rgba`),
  remplace `info.instances[i].name` en place. Partagé par les deux fonctions
  bout-en-bout ci-dessous.
- `bake_flat(stem, base_rgba_u8, axes, mask_loader, output_dir, hls_info_dir, low_def_shift=3, default_separator="_", output_format="tga")`
  — bout en bout, comportement **identique au vrai `panoply_maker.exe`** :
  `build_active_masks` + `bake_source` + `_write_variants` + un `.hlsinfo`
  brut dans `hls_info_dir`, rien d'autre (pas de `characters.hlsbank`/
  `panoply_files.txt` — le vrai outil ne les touche pas non plus, c'est
  `hls_bank_maker` qui s'en charge séparément). Mode utilisé par
  `apps/panoply_maker.py` en mode `.cfg` explicite (cross-validation
  byte-exacte contre le vrai binaire).
- `bake_and_write(stem, base_rgba_u8, axes, mask_loader, output_dir, build_dir, hlsbank_source=None, panoply_files_source=None, low_def_shift=3, default_separator="_", output_format="tga")`
  — bout en bout, workflow "next patch" (2026-08-29, Nuno) : `build_active_masks`
  + `bake_source` + `_write_variants` dans `output_dir`, puis écrit
  `build_dir/{stem}.hlsinfo`, `build_dir/panoply_files.txt` et
  `build_dir/characters.hlsbank` **sans jamais toucher `ryzom-data`** —
  `hlsbank_source`/`panoply_files_source` (les vrais fichiers de
  `ryzom-data/final_bnps/characters_maps_hr/`) ne servent que de point de
  départ en lecture. Voir `merge_panoply_files_txt()`/`load_or_empty_hlsbank()`
  ci-dessous pour la logique d'accumulation entre bakes successifs dans un
  même `build_dir`. Partagé par `apps/panoply_maker.py` (mode autonome, en
  boucle sur tout un dossier) et par le bouton "Bake real Panoply variants"
  de Patina (`object_editor._bake_panoply_real`, une source à la fois) —
  écrit une seule fois ici pour que les deux ne divergent jamais. Renvoie la
  liste des chemins de textures écrits (pas les 3 fichiers de `build_dir`,
  toujours à des noms fixes).
- `merge_panoply_files_txt(source_path, new_names)` — contenu mis à jour
  d'un `panoply_files.txt` : lignes de `source_path` **inchangées, dans
  l'ordre** (ou aucune si `source_path` est `None`/n'existe pas), plus les
  noms de `new_names` absents, ajoutés à la fin — ne trie/réordonne jamais
  les lignes existantes, pour garder un diff minimal contre le vrai fichier
  de production.
- `load_or_empty_hlsbank(path)` — `hls_texture_bank.load_hlsbank(path)` si
  `path` existe, sinon un `HLSTextureBank()` vide (valide, testé : sérialise
  à 13 octets, round-trip propre) — jamais une erreur si le fichier source
  n'existe pas encore.
- `_pick_existing(*candidates)` — le premier chemin de `candidates` qui
  existe réellement, `None` sinon. `bake_and_write()` l'utilise pour
  préférer `build_dir`'s propre `characters.hlsbank`/`panoply_files.txt`
  (accumulé par un bake précédent dans ce même `build_dir`) à la vraie
  source `ryzom-data` -- **seul le tout premier bake d'une session
  redémarre depuis la vraie source**, les suivants s'accumulent dans
  `build_dir`.

## Utilisation

Voir `docs/apps/panoply_maker.md` (CLI) et `docs/apps/object_editor.md`
section "Intégration Panoply" (`_bake_panoply_real`) pour les deux
appelants réels. Les deux résolvent `ryzom-data` via
`pynel.repository_paths` (voir `docs/repository_paths.md` de pynel) --
`panoply_bake.py` lui-même ne connaît rien de ce mécanisme, il prend juste
des chemins optionnels tout faits.

## Points notables / pièges

- Aucune logique de recherche de fichiers ici (voir "Rôle" ci-dessus) —
  toujours des tableaux numpy déjà chargés en entrée.
- `bake_source()` ne consomme le générateur `generate_color_combinations()`
  qu'une seule fois (construit `combos`/`instances` en parallèle dans la même
  boucle) — le reconsommer deux fois recalculerait tout le recoloriage pour
  rien.
- `divided_by_2` de `HLSBankTextureInfo` toujours `False` — la convention
  legacy `mustDivideBy2`/dossier `d4/` n'est pas portée (voir
  `docs/panoply_maker.md`/`docs/apps/panoply_maker.md`).
- `hls_texture_bank.append_texture_info()` n'a **aucune détection de
  doublon** : re-baker deux fois le même item dans le même `build_dir`
  ajoute deux `ColorTexture`/jeux d'instances au lieu de remplacer le
  premier -- pas de garde-fou ajouté ici pour l'instant (voir le module
  source, comportement hérité tel quel).
