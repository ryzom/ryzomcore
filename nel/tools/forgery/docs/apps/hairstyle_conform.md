# hairstyle_conform

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/hairstyle_conform.py` (116 lignes)

## Rôle

App 100% CLI (pas de GUI) : adapte une coiffure `.shape` d'une race/genre
pour qu'elle se raccorde sans trou au crâne d'une race/genre différente.
Ne fait *que* la couture externe — la restylisation de l'intérieur de la
coiffure (chaque style étant trop différent pour un algorithme général)
reste manuelle, dans un logiciel 3D, à partir du fichier produit.

Voir `ryzom_forgery/shape_geometry.py` pour l'algorithme
(`conform_hairstyle_boundary()` : pré-alignement rigide par centroïde de
couture, puis snap par sommet interpolé par angle sur l'anneau de couture
cible), `ryzom_forgery/race_reference.py` pour comment la vraie boucle de
couture est identifiée par race/genre (voir "Points notables" ci-dessous —
"la plus grande boucle" ne suffit pas), et `pynel/docs/shape_format.md` §4
pour le format `CMeshMRMSkinned` sous-jacent.

## API principale

- `_load_positions_indices(shape_value)` (`hairstyle_conform.py`) —
  aplatit les passes de rendu (`ryzom_forgery.shape_geometry.iter_render_passes`)
  en une seule liste de positions + indices, comme le font les scripts
  d'analyse ponctuels dans le reste du projet.
- `_require_mrm_skinned(shape_file, path)` (`hairstyle_conform.py`) — lève
  `SystemExit` si le shape n'est pas `CMeshMRMSkinned` (seul type disposant
  d'un writer géométrie dans pynel, voir `shape_format.md`).
- `main(argv=None)` (`hairstyle_conform.py`) — parse les arguments, construit
  l'index de search paths (`search_paths.build_texture_index`), résout les
  deux `RaceReference` (`race_reference.get_reference`), lit et valide
  `source`, calcule les nouvelles positions (`conform_hairstyle_boundary`),
  les recompacte dans `packed_vertices` via `PackedVertex.with_pos()`, puis
  écrit le résultat (`save_shape`).

Arguments CLI (déclarés `hairstyle_conform.py`) :
- `source` (positionnel) : coiffure `.shape` à adapter.
- `source_race_key` (positionnel) : clé `race_reference.cfg` de la race/genre
  de `source` (ex: `fy_hof`) — sert à identifier sa vraie boucle de bordure.
- `target_race_key` (positionnel) : clé `race_reference.cfg` de la race/genre
  cible (ex: `tr_hof`) — fournit l'anneau de couture canonique à interpoler.
- `output` (positionnel) : fichier `.shape` écrit, bordure conforme.
- `--search-path PATH [PATH ...]` (répétable, **requis**) : dossier(s)
  récursifs .bnp-aware pour résoudre les noms de fichiers de
  `race_reference.cfg`.
- `--workspace DIR` (optionnel) : dossier de workspace dont le
  `race_reference.cfg` propre, s'il existe, écrase le bundlé (voir
  `race_reference.set_workspace_dir`).

## Utilisation

Importe :
- `pynel.ryzom_shape` — `MeshMRMSkinned` (vérification de type),
  `ShapeParseError`/`ShapeWriteError` (erreurs converties en `SystemExit`),
  `parse_shape`/`save_shape`.
- `ryzom_forgery.race_reference` — `set_workspace_dir`, `get_reference`
  (résolution + cache des données de référence par race/genre, voir
  `docs/race_reference.md`).
- `ryzom_forgery.search_paths` — `build_texture_index` (scan .bnp-aware
  ponctuel à partir de `--search-path`, même mécanisme que
  `shape_exporter.py --data-root`).
- `ryzom_forgery.settings.SearchPathDir`.
- `ryzom_forgery.shape_geometry` — `iter_render_passes`,
  `conform_hairstyle_boundary`, `seam_loop_by_angle_indexed` (juste pour la
  validation amont "source a bien une bordure soudée").

Lancement : script autonome avec `if __name__ == "__main__": main`
(`hairstyle_conform.py`), même pattern que `shape_exporter.py` — pas de
dépendance à `app.py`/`ForgeryApp`, pas d'app graphique Panda3D/ImGui.

## Points notables / pièges

- Uniquement `CMeshMRMSkinned` supporté (les coiffures/vêtements de
  `characters_shapes.bnp` le sont toutes) — tout autre type échoue avec un
  message explicite plutôt qu'une erreur cryptique.
- **La vraie boucle de bordure n'est pas "la plus grande"** — corrigé après
  test sur une vraie coiffure volumineuse (`fy_hof_cheveux_basic02.shape` :
  9 boucles de bordure, dont deux de 30 sommets qui ne touchent pas du tout
  le visage, alors que la vraie couture ne fait que 26 sommets). C'est pour
  ça que `source_race_key`/`target_race_key` (et donc le visage de chaque
  race via `race_reference.py`) sont nécessaires : la bonne boucle est
  identifiée par coïncidence de position avec le visage
  (`shape_geometry.main_seam_loop()`), pas par taille.
- `target_race_key`'s coiffure de référence (dans `race_reference.cfg`) n'a
  besoin de couvrir tout le crâne (ex: un style rasé) que pour garantir un
  anneau de couture complet à interpoler ; `source`, lui, peut avoir une
  bordure partielle (frange, raie sur le côté) — chaque sommet de sa
  bordure a juste besoin d'un angle valable pour l'interpolation.
- Ne touche que les sommets de bordure de `source` ; l'intérieur du mesh ne
  reçoit que la translation rigide de pré-alignement, jamais de
  déformation — restylisation intentionnellement hors scope (voir
  `shape_geometry.py`'s `conform_hairstyle_boundary()` docstring).
- `CShadowSkin` et `MatrixInfluences`/`InfluencedVertices` (voir
  `shape_format.md` §4) sont recopiés tels quels par le writer pynel — pas
  régénérés après l'édition des positions ; potentiellement périmés si un
  futur usage édite des géométries plus profondément que ce cas (frontière
  connue, pas d'impact visuel constaté sur les cas testés jusqu'ici).
- Dossier parent de `output` créé automatiquement si besoin.
