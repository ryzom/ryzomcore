# shape_export

**Fichier :** `nel/tools/forgery/ryzom_forgery/shape_export.py`

## Rôle

Ce module convertit un `.shape` NeL déjà chargé/parsé (via `pynel.ryzom_shape`, non détaillé ici) vers des formats d'échange 3D grand public : `.obj`+`.mtl`, `.stl`, `.dae` (COLLADA), `.fbx`, `.gltf`/`.glb`. C'est le moteur du bouton "Export" de Patina (`apps/object_editor.py`) et du CLI `apps/shape_exporter.py`. Il travaille exclusivement sur la géométrie déjà résolue en render passes via `shape_geometry.iter_render_passes` — il ne connaît pas la structure interne d'un Mesh/MeshMRM/MeshMultiLod, seulement des triplets `(vertex_buffer, material_id, indices)`.

Choix de design documenté en tête de fichier (`shape_export.py`) : `.obj`/`.stl` sont écrits à la main (formats texte simples, pas besoin de dépendance). `.dae`/`.fbx`/`.gltf`/`.glb` passent tous par `assimp_py.export_file()` (notre fork, cf. `project-todos/assimp_py/bones_animations.md`) — un seul writer pour les quatre formats, capable d'embarquer skin/bones (`export_mesh_with_skin`, cf. `project-todos/forgery/mesh_skin_export.md`), ce que `pycollada`/`pygltflib` (anciens writers, retirés) ne pouvaient pas exprimer.

## API principale

- `ExportFormat` (dataclass, `shape_export.py`) — décrit un format exportable : extension, label, `supports_materials`, et la fonction `export` à appeler.
- `EXPORT_FORMATS` (`shape_export.py`) — liste des 6 formats supportés : `obj`, `dae`, `stl`, `fbx`, `gltf`, `glb`. Seul `stl` a `supports_materials=False`.
- `export_shape(shape_value, name, export_format, output_dir, texture_mode, texture_finder)` (`shape_export.py`) — point d'entrée public (rigide, pas de skin). Résout le nom de fichier de sortie à partir du stem de `name`, extrait `shape_value.materials`, bake `default_rot_quat` dans une copie de la géométrie, et délègue à `export_format.export(...)`. Lève `ValueError` si le shape n'a pas de géométrie exportable.
- `export_mesh_with_skin(shape_value, skeleton, output_path, texture_mode, texture_finder)` (`shape_export.py`) — point d'entrée public pour un export skin/bones (`.dae`/`.fbx`/`.gltf`/`.glb`, format déduit de l'extension de `output_path`) ; `skeleton` est un `pynel.ryzom_shape.SkeletonShape` dont les noms d'os correspondent à `shape_value.geom.bones_name`. Pas encore branché à l'UI Patina (cf. `project-todos/forgery/mesh_skel_anim_io.md`, sous-chantier 6).
- `_resolve_material_texture(...)` (`shape_export.py`) — décide du nom de texture à référencer dans le matériau exporté ; si `texture_mode == TEXTURE_MODE_COPY_PNG`, décode la texture source via `load_panda_texture` et écrit une copie `.png` à côté de l'export (mise en cache dans `texture_cache` pour éviter une double écriture/décodage par export).
- `_export_obj(...)` (`shape_export.py`) — écrit `.obj`+`.mtl` à la main. Point notable : les indices `v`/`vn`/`vt` d'OBJ sont globaux au fichier ; les vertex buffers partagés entre plusieurs passes (cas fréquent d'un MatrixBlock/finest LOD) ne sont écrits qu'une seule fois via le dict `buffer_offsets` keyé par `id(vertex_buffer)`.
- `_triangle_normal(a, b, c)` (`shape_export.py`) — normale de facette par produit vectoriel, normalisée (retourne `(0,0,0)` si dégénérée).
- `_export_stl(...)` (`shape_export.py`) — fusionne toutes les passes en un seul "soup" de triangles ; ignore matériaux/UV (STL n'en a pas) ; recalcule une normale par facette à partir du winding plutôt que de réutiliser les normales de sommet du vertex buffer.
- `_assimp_scene_from_shape(shape_value, materials, output_path, texture_mode, texture_finder, skeleton=None)` (`shape_export.py`) — construit l'`assimp_py.Scene` complète (un `assimp_py.Mesh` par render pass, sous un Node "Meshes" ; le squelette, si fourni, sous un Node "Armature" frère). Voir "Axes" ci-dessous pour la conversion Z-up/Y-up.
- `_assimp_mesh_from_geometry(...)` (`shape_export.py`) — un `assimp_py.Mesh` (rigide ou skinné selon `bones`) depuis les canaux `Position`/`Normal`/`TexCoord0` d'un pass.
- `_skin_state_for_shape(...)` / `_skin_bones_for_export(...)` (`shape_export.py`) — réutilisent les builders `_build_skin_state`/`_build_mrm_skin_state`/`_build_mesh_skin_state` de `apps/object_editor_mixins/skin_state_helpers.py` (mêmes tables que la Skinning preview live) et les transposent au format par-os attendu par `assimp_py.Bone`.
- `_assimp_skeleton_root_node(skeleton)` (`shape_export.py`) — hiérarchie de `Node` (un par os, `father_id`) sous un Node racine `"Armature"`, chaque transform locale via `pynel.ryzom_animation._bone_local_matrix()`.
- `_export_via_assimp(...)` (`shape_export.py`) — appelle `assimp_py.export_file(scene, output_path, format_id)`, `format_id` résolu depuis l'extension (`collada`/`fbx`/`gltf2`/`glb2`).

## Utilisation

- `apps/object_editor.py` importe `EXPORT_FORMATS` ; `export_dialog.py` importe `export_shape` — c'est `ExportDialog._run_export` (voir `export_dialog.md`) qui appelle réellement `export_shape` une fois le dossier de sortie et le mode texture connus.
- `apps/shape_exporter.py` importe `EXPORT_FORMATS` pour le CLI équivalent (conversion `.shape` → format d'échange en ligne de commande).
- Dépend de `shape_geometry.py` pour `iter_render_passes`, `shape_geom`, `load_panda_texture`, `rgba_to_color`, `texture_to_pnm_image` (voir `shape_geometry.md`).
- Dépend de `ryzom_forgery.settings.TEXTURE_MODE_COPY_PNG` pour le mode de gestion des textures.
- `export_mesh_with_skin` dépend de `apps/object_editor_mixins/skin_state_helpers.py` (builders de skin partagés avec la Skinning preview live) et de `pynel.ryzom_animation._bone_local_matrix`.

## Points notables / pièges

- STL n'a aucun support matériau/texture : la texture_mode est ignorée pour ce format bien que le paramètre soit passé (signature uniforme entre tous les `export_format.export`).
- Toute fonction `_export_*` lève `ValueError("No renderable geometry to export")` si aucune passe de rendu n'a produit de triangle — cas d'un shape vide ou d'un type de shape non pris en charge par `iter_render_passes`.
- `_resolve_material_texture` retourne `None` silencieusement (pas d'erreur) si la texture ne peut être chargée ; le matériau exporté n'aura simplement pas de texture associée.
- **Axes** : un export **rigide** (`export_shape`, ou `export_mesh_with_skin` sans effet skin réel) convertit `Position`/`Normal` de la convention Z-up de Ryzom vers le Y-up qu'exigent `.fbx` (`FBXExporter.cpp` fixe `UpAxis=1` en dur, quel que soit le contenu) et `.gltf` (aucune métadonnée d'axe dans le format) via `_zup_to_yup()`. Un export **skinné** (`export_mesh_with_skin` avec un `skeleton`) reste volontairement en Z-up brut, mesh et squelette non convertis : convertir le mesh sans convertir cohéremment chaque `offset_matrix` d'os et toute la hiérarchie du squelette casserait le skin (pas seulement une orientation visuellement fausse, une déformation de forme fausse). À corriger dans un chantier de suivi si la validation visuelle de Nuno (Blender, `mesh_skin_export.md` Étape 7) montre que c'est gênant en pratique.
- **Limite squelette connue** : `_assimp_skeleton_root_node()` ne représente pas la compensation `UnheritScale` de `CBone::compute()` (voir `pynel.ryzom_animation._unherit_scale_comp()`) — sans effet pour un `.skel` généré par pynel (toujours `unherit_scale=False`), mais un vrai `.skel` 3dsMax (rig Biped) avec des os `unherit_scale=True` exporterait une bind pose subtilement fausse.
- **UV** : `.obj`/`.dae`/`.fbx`/`.gltf` stockent tous leurs UV en convention "OpenGL" (V=0 en bas), l'inverse de la convention native NeL d'un `.shape` (V=0 en haut, voir `shape_import.md`) -- `_assimp_mesh_from_geometry()` et `_export_obj()` appliquent toutes les deux `v' = 1 - v` avant d'écrire. Manqué une fois sur `_export_obj()` (trouvé 2026-09-05, Nuno : "obj... mirroir uv" -- l'orientation-only fix précédent n'avait pas touché les UV).
- **Échelle FBX (`UnitScaleFactor`)** : `_assimp_scene_from_shape()`/`export_assembled_creature()` passent `metadata={"UnitScaleFactor": 100.0}` à `assimp_py.Scene(...)` pour un export `.fbx` uniquement -- sans ça, `FBXExporter.cpp::WriteGlobalSettings()` retombe sur `UnitScaleFactor=1.0` (convention FBX : "ce sont des centimètres"), alors que nos données sont en mètres natifs Ryzom ; au réimport, `Process_GlobalScale` divise alors par 100 en trop (trouvé 2026-09-05, Nuno : "il apparait tres petit et tres loin de la camera", confirmé numériquement en comparant les positions réimportées `.fbx` vs `.dae`/`.glb` d'un même shape). Nécessite `assimp_py` avec le support `Scene(metadata=...)` (`project-todos/assimp_py/scene_metadata.md`). `.dae`/`.gltf` n'ont pas cette ambiguïté d'échelle par fichier, pas concernés.
- Pas de TODO explicite dans le fichier au-delà des points ci-dessus.
