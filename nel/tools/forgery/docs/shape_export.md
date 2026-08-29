# shape_export

**Fichier :** `nel/tools/forgery/ryzom_forgery/shape_export.py` (~416 lignes)

## Rôle

Ce module convertit un `.shape` NeL déjà chargé/parsé (via `pynel.ryzom_shape`, non détaillé ici) vers des formats d'échange 3D grand public : `.obj`+`.mtl`, `.stl`, `.dae` (COLLADA), `.gltf`/`.glb`. C'est le moteur du bouton "Export" de Patina (`apps/object_editor.py`) et du CLI `apps/shape_exporter.py`. Il travaille exclusivement sur la géométrie déjà résolue en render passes via `shape_geometry.iter_render_passes` — il ne connaît pas la structure interne d'un Mesh/MeshMRM/MeshMultiLod, seulement des triplets `(vertex_buffer, material_id, indices)`.

Choix de design documenté en tête de fichier (`shape_export.py`) : `.obj`/`.stl` sont écrits à la main (formats texte simples, pas besoin de dépendance) ; `.dae` passe par `pycollada` et `.gltf`/`.glb` par `pygltflib` car ce sont des formats trop complexes pour un writer maison fiable.

## API principale

- `ExportFormat` (dataclass, `shape_export.py`) — décrit un format exportable : extension, label, `supports_materials`, et la fonction `export` à appeler.
- `EXPORT_FORMATS` (`shape_export.py`) — liste des 5 formats supportés : `obj`, `dae`, `stl`, `gltf`, `glb`. Seul `stl` a `supports_materials=False`.
- `export_shape(shape_value, name, export_format, output_dir, texture_mode, texture_finder)` (`shape_export.py`) — point d'entrée public. Résout le nom de fichier de sortie à partir du stem de `name`, extrait `shape_value.materials`, et délègue à `export_format.export(...)`. Lève `ValueError` si le shape n'a pas de géométrie exportable.
- `_resolve_material_texture(...)` (`shape_export.py`) — décide du nom de texture à référencer dans le matériau exporté ; si `texture_mode == TEXTURE_MODE_COPY_PNG`, décode la texture source via `load_panda_texture` et écrit une copie `.png` à côté de l'export (mise en cache dans `texture_cache` pour éviter une double écriture/décodage par export).
- `_export_obj(...)` (`shape_export.py`) — écrit `.obj`+`.mtl` à la main. Point notable : les indices `v`/`vn`/`vt` d'OBJ sont globaux au fichier ; les vertex buffers partagés entre plusieurs passes (cas fréquent d'un MatrixBlock/finest LOD) ne sont écrits qu'une seule fois via le dict `buffer_offsets` keyé par `id(vertex_buffer)`.
- `_triangle_normal(a, b, c)` (`shape_export.py`) — normale de facette par produit vectoriel, normalisée (retourne `(0,0,0)` si dégénérée).
- `_export_stl(...)` (`shape_export.py`) — fusionne toutes les passes en un seul "soup" de triangles ; ignore matériaux/UV (STL n'en a pas) ; recalcule une normale par facette à partir du winding plutôt que de réutiliser les normales de sommet du vertex buffer.
- `_export_dae(...)` (`shape_export.py`) — construit le document via `pycollada` (`Collada`, `geometry`, `material`, `scene`, `source`). Une géométrie COLLADA distincte par render pass.
- `_build_gltf_document(...)` (`shape_export.py`) — construction partagée par `.gltf` et `.glb` : `pygltflib.GLTF2.save` choisit JSON vs binaire selon l'extension du chemin de sortie, donc le document est identique dans les deux cas. Empaquette manuellement les buffers binaires (`struct.pack`) avec alignement 4 octets par `bufferView` (`shape_export.py`).
- `_export_gltf_or_glb(...)` (`shape_export.py`) — appelle `_build_gltf_document` puis `doc.save` ; pour `.gltf` (JSON), ajoute le fichier compagnon `.bin` à la liste des fichiers écrits (comportement confirmé empiriquement, `shape_export.py`).

## Utilisation

- `apps/object_editor.py` importe `EXPORT_FORMATS` ; `export_dialog.py` importe `export_shape` — c'est `ExportDialog._run_export` (voir `export_dialog.md`) qui appelle réellement `export_shape` une fois le dossier de sortie et le mode texture connus.
- `apps/shape_exporter.py` importe `EXPORT_FORMATS` pour le CLI équivalent (conversion `.shape` → format d'échange en ligne de commande).
- Dépend de `shape_geometry.py` pour `iter_render_passes`, `load_panda_texture`, `rgba_to_color`, `texture_to_pnm_image` (voir `shape_geometry.md`).
- Dépend de `ryzom_forgery.settings.TEXTURE_MODE_COPY_PNG` pour le mode de gestion des textures.

## Points notables / pièges

- STL n'a aucun support matériau/texture (`shape_export.py`, `164-166`) : la texture_mode est ignorée pour ce format bien que le paramètre soit passé (signature uniforme entre tous les `export_format.export`).
- Toute fonction `_export_*` lève `ValueError("No renderable geometry to export")` si aucune passe de rendu n'a produit de triangle (`shape_export.py`, `190-191`, `277-278`, `371-372`) — cas d'un shape vide ou d'un type de shape non pris en charge par `iter_render_passes`.
- Le fait que l'export `.gltf` produise un `.bin` compagnon n'est pas déduit de la doc pygltflib mais "confirmé en live" (commentaire `shape_export.py`) — comportement observé empiriquement plutôt que garanti par contrat d'API.
- `_resolve_material_texture` retourne `None` silencieusement (pas d'erreur) si la texture ne peut être chargée (`shape_export.py`) ; le matériau exporté n'aura simplement pas de `map_Kd`/texture associée.
- Pas de TODO explicite dans le fichier.
