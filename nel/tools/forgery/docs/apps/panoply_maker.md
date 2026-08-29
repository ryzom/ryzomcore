# panoply_maker (CLI)

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/panoply_maker.py`

Status : **cross-validé contre le vrai `panoply_maker.exe`** (2026-08-29,
mode `.cfg` explicite, voir `docs/panoply_maker.md`) -- tourne sans erreur,
`.tga` quasi byte-exacts contre le binaire réel (diff pixel max 1, moyenne
0.00). Environ 8x plus lent que le binaire natif sur le même jeu de test
(24 variantes : ~2,4s réel vs ~19s Python) -- attendu (numpy/Panda3D
interprété vs C++ natif), non bloquant vu l'usage prévu (bake ponctuel,
pas un chemin critique). Le mode autonome (`--input`/`--output`) reste non
testé en conditions réelles (nécessite `ryzom-data` configuré).

## Rôle

App 100% CLI (pas de GUI), comme `shape_exporter.py` : bake offline des
variantes de couleur Panoply pour tous les fichiers sources d'un dossier,
port de `nel/tools/3d/panoply_maker/panoply_maker.cpp`. Toute la logique de
bake (résolution des masques actifs, boucle combinatoire, écriture) vit dans
`ryzom_forgery/panoply_bake.py` (voir `docs/panoply_bake.md`) — ce fichier
n'est que le driver ligne de commande, en **deux modes**, qui n'écrivent pas
la même chose.

## Modes

- **Autonome** (`--input`/`--output`, pas de `.cfg` passé) : les couleurs
  viennent de `panoply_config.py` (workspace `panoply.cfg` s'il existe,
  sinon le bundlé — voir `docs/panoply_config.md`), jamais d'un fichier
  choisi par l'utilisateur. Écrit les textures baked dans `--output`, et le
  `.hlsinfo`/`panoply_files.txt`/`characters.hlsbank` mis à jour dans
  `--build` (par défaut `<workspace>/build` si `--workspace` est donné) via
  `panoply_bake.bake_and_write()` — voir `docs/panoply_bake.md`. **Nécessite
  `ryzom-data` configuré** (`pynel.repository_paths`, voir
  `docs/repository_paths.md` de pynel) : refuse de tourner sinon
  (`SystemExit`), pour les mêmes raisons que le bouton de bake de Patina
  (voir `docs/apps/object_editor.md`). C'est le mode que
  `object_editor._bake_panoply_real()` (Patina) reproduit directement (mêmes
  appels `panoply_bake`, sa propre résolution de fichiers via
  `search_paths_dialog` au lieu d'un dossier à plat).
- **`.cfg` explicite** (argument positionnel, un seul, comme l'argv du vrai
  outil) : tout (chemins, couleurs, `mask_extensions`, `output_format`, ...)
  vient de ce fichier via `pynel.config_file` +
  `panoply_maker.build_masks_from_config()` existant, dans la forme de
  production réelle (une race par fichier, clés non préfixées). Écrit un
  `.hlsinfo` brut par source (`panoply_bake.bake_flat()`), **rien d'autre**
  (pas de `build/`, pas de `characters.hlsbank`/`panoply_files.txt`, pas de
  dépendance à `ryzom-data` configuré) — reproduit exactement le
  comportement du vrai `panoply_maker.exe`, seule façon de faire une
  comparaison byte-exacte contre lui. Voir
  `/repos/project-todos/ryzom-core/panoply-runtime-tint.md`.

## API principale

- `_iter_source_files(input_path, bitmap_extensions)` — fichiers directement
  sous `input_path` (non récursif — le layout de production sépare sources
  et masques dans des dossiers distincts, `additionnal_paths`), filtrés par
  extension, en sautant ceux dont le nom se termine par `_skin`/`_user`/
  `_hair`/`_eyes` (au cas où un masque traînerait dans le même dossier
  qu'une source).
- `_find_mask(input_dirs, stem, mask_ext, ext)` — cherche
  `{stem}_{mask_ext}{ext}` dans chaque dossier de `input_dirs`, dans l'ordre
  (`input_path` d'abord, puis `additionnal_paths` en mode `.cfg`).
- `_run_autonomous(args)` — vérifie `repository_paths.is_valid("ryzom-data")`
  (sinon `SystemExit`), résout `hlsbank_source`/`panoply_files_source` sous
  `ryzom-data/final_bnps/characters_maps_hr/`, résout `--build` (depuis
  `--workspace` si absent), puis boucle chaque source : charge l'image
  (`dds_export.load_rgba`), construit les axes (`panoply_bake.axes_for_source`,
  race déduite du préfixe du nom de fichier), et appelle
  `panoply_bake.bake_and_write()`.
- `_run_from_cfg(cfg_path)` — lit le `.cfg` (`pynel.config_file.Document`),
  construit les axes via `panoply_maker.build_masks_from_config()`, boucle
  chaque source et appelle `panoply_bake.bake_flat()`.
- `main(argv=None)` — parse les arguments, choisit le mode selon la présence
  de l'argument positionnel `cfg`.

## Utilisation

```
panoply_maker.py --input DIR --output DIR [--build DIR] [--workspace DIR] [options]
panoply_maker.py CFG.cfg
```

Options du mode autonome uniquement (un `.cfg` explicite les fournit
lui-même) : `--build` (défaut `<workspace>/build`), `--workspace` (dossier
de workspace Forgery à vérifier pour un `panoply.cfg` de override, et à
défaulter `--build` depuis), `--low-def-shift` (def. 3),
`--default-separator` (def. `"_"`), `--output-format` (def. `"tga"`),
`--bitmap-extensions` (def. `tga png`).

## Points notables / pièges

- Comme `shape_exporter.py` : aucune interface graphique, script autonome
  (`if __name__ == "__main__": main()`), sans dépendance à `app.py`.
- Le mode autonome n'accepte pas `additionnal_paths` (un seul dossier de
  recherche de masques, `--input`) — non observé comme nécessaire sur les 10
  vrais `.hlsinfo` déjà validés (voir `docs/panoply_maker.md`).
- `mustDivideBy2`/dossier `d4/` (convention legacy de la moitié de résolution)
  volontairement pas géré — jamais observé dans les vrais `.hlsinfo`.
- Pas de cache (`cache_path`) — toujours un rebuild complet, le mode le plus
  simple documenté dans `panoply.md` de pynel.
- Le mode `.cfg` explicite n'exige pas `ryzom-data` configuré (il n'y touche
  jamais) — seul le mode autonome le vérifie.
