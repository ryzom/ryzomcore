# rebuild_shadow_skin (CLI)

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/rebuild_shadow_skin.py`

## Rôle

App 100% CLI (pas de GUI) : reconstruit le `CShadowSkin` (mesh proxy
d'ombre porté, voir `docs/shape_format.md` §4 de pynel) d'un `.shape` à
partir d'un de ses propres LODs, portage de
`nel/tools/3d/build_shadow_skin/main.cpp`'s `addShadowMesh()`. Seuls les
`CMeshMRMSkinned` sont supportés (seul type avec un writer `CShadowSkin`
côté pynel).

## API principale

- `main(argv=None)` -- point d'entrée CLI, `argparse`.
- Arguments :
  - `source` : fichier `.shape` dont on reconstruit le `CShadowSkin`
    (positionnel).
  - `output` : chemin de sortie (peut être identique à `source`).
  - `--lod N` : index de LOD explicite (0 = le plus grossier) ; par défaut,
    inféré depuis le `CShadowSkin` déjà présent (comparaison de son nombre
    de triangles à celui de chaque LOD) ou, en son absence, une règle par
    défaut indépendante du type de contenu (LOD le plus grossier avec
    >=~1000 triangles, sinon index 3 -- voir le docstring de
    `shape_geometry.default_shadow_skin_lod_index()`).
- Délègue l'algorithme réel à `ryzom_forgery.shape_geometry.rebuild_shadow_skin()`/
  `infer_shadow_skin_lod_index()`.
- Rejette (`SystemExit`) tout type de shape autre que `CMeshMRMSkinned`, et
  tout `--lod` hors des bornes réelles du shape.

## Utilisation

```
rebuild_shadow_skin.py SOURCE.shape OUTPUT.shape [--lod N]
```

## Points notables / pièges

- Suit le même moule que les autres apps CLI du projet
  (`apps/shape_exporter.py`, etc.) : `main(argv=None)` testable sans
  sous-processus, erreurs utilisateur via `SystemExit`.
- `SOURCE` et `OUTPUT` peuvent être le même fichier -- le résultat est
  entièrement recalculé en mémoire avant écriture.
