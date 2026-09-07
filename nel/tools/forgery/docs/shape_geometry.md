# shape_geometry

**Fichier :** `nel/tools/forgery/ryzom_forgery/shape_geometry.py` (~359 lignes)

## Rôle

Module d'extraction géométrique/matérielle partagé entre l'éditeur 3D live (`apps/object_editor.py`) et les exporteurs (`shape_export.py`), pour éviter que les deux ne maintiennent chacun leur propre logique de "comment parcourir les render passes d'un shape parsé". Il fournit une interface unifiée (`iter_render_passes`) qui masque les différences entre `CMesh`, `CMeshMRM`, `CMeshMultiLod` et `CMeshMRMSkinned` (types `pynel.ryzom_shape`), ainsi que la résolution/décodage de textures (`load_panda_texture`) en objets `panda3d.core.Texture`.

## API principale

### Parcours des passes de rendu
- `_passes_from_mesh_geom(geom)` (`shape_geometry.py`) — cas `CMesh` simple : itère tous les `matrix_blocks` × leurs `rdr_passes`.
- `_resolve_lod_geomorphs(vertex_buffer, lod)` (`shape_geometry.py`) — résout les "wedges" placeholder de géomorphing d'un LOD MRM vers leur valeur statique "end" (pas d'animation de transition LOD dans cet outil) ; sans ça, ces sommets seraient rendus collés à l'origine.
- `finest_skinned_lod(geom)` (`shape_geometry.py`) — retourne `(lod, packed_vertices)` pour le LOD le plus fin d'un `MeshMRMSkinnedGeom` (`geom.lods[-1]`), avec géomorphs résolus. `None` si pas de LOD. Exposé publiquement (pas seulement interne) car un re-skin live par frame en a besoin.
- `_passes_from_mrm_skinned_geom_rigid(geom)` (`shape_geometry.py`) — fallback "sans squelette" : sommets bind-pose bruts, non skinnés, imitant le comportement du vrai moteur Ryzom quand un `CMeshMRMSkinned` n'a pas de squelette attaché.
- `_passes_from_mrm_skinned_geom(geom, skeleton, bone_world_matrices)` (`shape_geometry.py`) — skinning réel via `pynel.ryzom_skin.bone_skin_matrices_for_mesh`/`skin_vertex`, à une pose donnée.
- `_passes_from_mrm_geom(geom)` (`shape_geometry.py`) — cas `CMeshMRM` : utilise `geom.lods[-1]` (le LOD le plus détaillé, convention documentée en commentaire — LOD 0 = le moins détaillé côté NeL).
- `iter_render_passes(shape_value, skeleton=None, bone_world_matrices=None)` (`shape_geometry.py`) — **fonction pivot** : dispatch selon `isinstance(shape_value, ...)` (`Mesh`, `MeshMRM`, `MeshMultiLod` slot 0, `MeshMRMSkinned`), yield `(vertex_buffer, material_id, indices)`. Sans squelette/pose fournis pour un shape skinné, retombe automatiquement sur le rendu rigide bind-pose.
- `shape_geom(shape_value)` (`shape_geometry.py`) — même dispatch que `iter_render_passes` mais retourne l'objet geom brut (pour du code qui a besoin de données niveau geom, ex. `vertex_program`/`WindTreeParams`).
- `shape_bbox(shape_value)` (`shape_geometry.py`) — bbox du shape, même dispatch.

### Couleurs et textures
- `rgba_to_color(rgba)` (`shape_geometry.py`) — `Rgba` NeL (0-255) → tuple float (0-1).
- `solid_color_texture(color)` (`shape_geometry.py`) — Texture Panda3D 1x1 pour afficher une couleur unie avec le même widget ImGui qu'une vraie texture.
- `_find_local_texture_ref(name, search_dirs)` (`shape_geometry.py`) — fallback de recherche de texture par nom dans des dossiers locaux (typiquement le dossier d'un mesh importé) et leurs sous-dossiers `tex`/`textures`/`data`, insensible à la casse, avec repli sur des extensions alternatives (`TEXTURE_FALLBACK_EXTENSIONS`, de `search_paths.py`).
- `resolve_texture_ref(name, search_dirs=None, finder=None)` (`shape_geometry.py`) — résolution en deux étapes (`search_dirs` d'abord, puis `finder`), avec gestion spéciale d'un `name` déjà un chemin absolu (bypass complet ; repli sur recherche par nom de base si le fichier n'existe plus). Exposée séparément de `load_panda_texture` pour des appelants qui veulent juste inspecter/décoder eux-mêmes une référence sans passer par le cache.
- `load_panda_texture(name, cache=None, search_dirs=None, repeat=False, finder=None)` (`shape_geometry.py`) — résout puis décode une texture en `panda3d.core.Texture`. Gère `.dds` via `Texture.read_dds` (avec capture d'`AssertionError` pour des en-têtes DDS malformés) et les autres formats via `PNMImage`. `repeat` bascule le wrap mode en `WM_repeat` (défaut Panda3D = `WM_clamp`).
- `texture_to_pnm_image(panda_texture)` (`shape_geometry.py`) — relit les pixels d'une Texture Panda3D vers un `PNMImage` (utilisé pour ré-exporter en `.png`).

## Utilisation

- `shape_export.py` importe `iter_render_passes, load_panda_texture, rgba_to_color, texture_to_pnm_image`.
- `apps/object_editor.py` importe `finest_skinned_lod, iter_render_passes, load_panda_texture, resolve_texture_ref, rgba_to_color, shape_bbox` — l'éditeur 3D live s'appuie massivement sur ce module pour construire sa géométrie de rendu (`object_editor.py, 2053`) et gérer le rendu skinné avec fallback rigide (`object_editor.py`).
- `explorer.py` importe `load_panda_texture` localement (pas au niveau module) pour générer des vignettes/aperçus.
- `panoply_texture.py` référence le duck-type `finder` de `load_panda_texture` (même interface `.name`/`.read_bytes`).
- `search_paths_dialog.py` référence ce module pour le rôle de `SearchPathsDialog.find_texture` comme `finder`.
- Dépend de `search_paths.py` (`FoundEntry`, `TEXTURE_FALLBACK_EXTENSIONS`) et de `pynel.ryzom_shape`/`pynel.ryzom_skin` (packages tiers).

## Points notables / pièges

- La convention LOD MRM est contre-intuitive : `lods[-1]` (le dernier) est le **plus détaillé**, `lods[0]` le moins détaillé — car `CMeshMRMGeom` charge le mesh progressivement du plus grossier au plus fin (`shape_geometry.py`). Documenté explicitement pour éviter une inversion.
- `iter_render_passes` pour `MeshMRMSkinned` sans `skeleton`/`bone_world_matrices` ne lève pas d'erreur : il retombe silencieusement sur le rendu rigide bind-pose (`shape_geometry.py, 170-174`) — comportement voulu pour permettre d'inspecter un shape avant d'avoir chargé un `.skel` compatible.
- Le flip V des UV (convention NeL, V=0 en haut, vs Panda3D/OpenGL) est appliqué **uniformément à toutes les données**, y compris les vrais fichiers `.shape`, au niveau des coordonnées UV construites côté `object_editor.py._build_vertex_data` — pas ici dans `load_panda_texture` — car le chemin DDS n'a pas d'équivalent pour flipper les lignes de pixels après décodage (`shape_geometry.py`). C'est cohérent uniquement parce que `shape_import.py` convertit déjà la convention UV source vers la convention NeL à l'import (voir `shape_import.md`).
- Le `repeat` wrap mode n'est **pas** appliqué par défaut à toutes les textures : l'testé montre que ça décalait visiblement les textures de meshes importés (UV qui dépassent [0,1] par bruit flottant sans intention de tiling) — seul le cas explicitement détecté par `object_editor.py._uvs_need_repeat` (ex. `ooc_summer_raceline.shape`) l'active (`shape_geometry.py`).
- Un en-tête DDS malformé peut déclencher une `AssertionError` C++ dans Panda3D au lieu d'un simple échec de `read_dds` — capturée explicitement (`shape_geometry.py`).
- Pas de TODO explicite dans le fichier.
