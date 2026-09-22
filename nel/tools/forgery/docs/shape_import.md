# shape_import

**Fichier :** `nel/tools/forgery/ryzom_forgery/shape_import.py` (~617 lignes)

## Rôle

Ce module fait l'opération inverse de `shape_export.py` : il lit un fichier `.obj`, `.dae` ou `.fbx` et construit un `pynel.ryzom_shape.Mesh` (un `CMesh`) prêt à être sauvegardé en `.shape` via `save_shape`. C'est le moteur du bouton "Import" de Patina (`apps/object_editor.py`, via `import_dialog.py`) et du CLI `apps/shape_importer.py`, ainsi que de la surveillance automatique `import_watcher.py`.

Limitation structurelle documentée en tête de fichier (`shape_import.py`) : seul `CMesh` peut être construit "from scratch" par pynel — `CMeshMRM` (avec LOD progressifs) nécessite `CMRMBuilder`, une classe C++ sans binding Python, donc hors de portée ici. Un shape importé n'a donc jamais de LOD.

Deuxième choix documenté : `.obj`/`.mtl` sont parsés à la main (formats texte simples) ; `.dae` et `.fbx` passent par `assimp-py`, une bibliothèque tierce déjà nécessaire pour `.fbx`.

## API principale

### Parsing .obj / .mtl
- `ObjFace`, `ObjMesh` (dataclasses, `shape_import.py`) — structures intermédiaires du parsing OBJ.
- `_resolve_obj_index(raw, count)` (`shape_import.py`) — résout un indice OBJ (1-based, ou négatif = relatif à la fin de la liste courante) en indice 0-based.
- `parse_obj(path)` (`shape_import.py`) — parseur OBJ minimal (`v`, `vn`, `vt`, `usemtl`, `mtllib`, `f`). Triangule les n-gones en éventail (`shape_import.py`). Lève `ShapeImportError` si aucun sommet ou aucune face.
- `MtlMaterial` (dataclass, `shape_import.py`), `parse_mtl(path)` (`shape_import.py`) — parseur MTL minimal (`Kd`, `Ka`, `Ks`, `Ns`, `d`/`Tr`, `map_Kd`).

### Construction du Mesh pynel
- `_texture_base_name(texture_name, base_dir)` (`shape_import.py`) — résout une référence de texture relative au dossier du fichier importé ; si le fichier résolu existe réellement sur disque *au moment de l'import*, garde le chemin absolu complet (permet un rendu immédiat sans que l'utilisateur ait à relier la texture) ; sinon retombe sur le simple nom de fichier.
- `_build_material(texture_name, double_sided, base_dir)` (`shape_import.py`) — construit un matériau NeL "vierge" avec des valeurs fixes calquées sur le défaut du plugin exporteur 3ds Max (`_NEL_DEFAULT_GRAY`, etc., voir Points notables). Seule la texture diffuse et le flag `double_sided` proviennent réellement du fichier source.
- `_assemble_mesh(positions, normals, texcoords, materials, rdr_passes)` (`shape_import.py`) — assemblage final commun à tous les importeurs : un `Mesh` mono-matrix-block, non skinné, avec bbox recalculée à partir des positions.
- `build_mesh(obj_mesh, mtl_materials, base_dir)` (`shape_import.py`) — convertit un `ObjMesh` en `Mesh` pynel : dédoublonnage des sommets combinés `(pos, uv, normal)` (`combined_vertex_id`), regroupement des faces par matériau en `RdrPass`.
- `import_obj(path)` (`shape_import.py`) — point d'entrée .obj : parse le `.obj` puis le(s) `.mtl` référencé(s) par `mtllib`, appelle `build_mesh`.

### Import via assimp (.dae / .fbx)
- `_YUP_TO_ZUP_MATRIX` (`shape_import.py`) — matrice de conversion du Y-up canonique d'Assimp vers le Z-up de Ryzom (rotation pure, déterminant +1).
- `_mat_mul_mat`, `_mat_mul_point`, `_mat_mul_dir`, `_normalize` (`shape_import.py`) — petites primitives matricielles maison (pas de dépendance type numpy pour ça). `_mat_mul_dir` utilise la sous-matrice 3x3 sans inverse-transpose, correct seulement pour rotation/scale uniforme (limite assumée, `shape_import.py`).
- `_iter_mesh_instances(node, parent_transform)` (`shape_import.py`) — parcourt récursivement le graphe de scène Assimp, produit `(mesh_index, world_transform)` pour chaque instance de mesh (un mesh peut être référencé par plusieurs nœuds).
- `_mesh_positions/_mesh_normals/_mesh_texcoords(mesh)` (`shape_import.py`) — extraction des canaux bruts d'un mesh Assimp (gèrent l'absence de canal, qui revient à `None` chez assimp-py plutôt qu'une liste vide).
- `_build_material_from_assimp_material(material, base_dir)` (`shape_import.py`) — comme `_build_material` mais depuis un dict de propriétés assimp-py ; ne lit que la texture diffuse et `TWOSIDED`.
- `_import_via_assimp(path)` (`shape_import.py`) — cœur de l'import .dae/.fbx : lance `assimp_py.import_file` avec les flags `Process_Triangulate | Process_JoinIdenticalVertices | Process_GenNormals | Process_GlobalScale`, bake les transforms de nœud dans les sommets, applique la conversion Y-up→Z-up, renormalise les normales générées. Lève `ShapeImportError` si aucun mesh/sommet.
- `import_dae(path)` / `import_fbx(path)` (`shape_import.py`) — wrappers triviaux autour de `_import_via_assimp`.
- `IMPORTERS` (`shape_import.py`) — dict `{"obj": import_obj, "dae": import_dae, "fbx": import_fbx}`, source unique partagée par `import_dialog.py`, `apps/shape_importer.py`, `import_watcher.py`.
- `find_importer(path)` (`shape_import.py`) — retourne l'importeur pour l'extension de `path`, ou `None`.
- `texture_search_dirs_for(path)` (`shape_import.py`) — dossiers de recherche de textures additionnels pour un fichier importé : son propre dossier, plus `<nom>.fbm` si `.fbx` (convention 3ds Max/FBX SDK).

## Utilisation

- `import_dialog.py` importe `ShapeImportError, find_importer` — c'est `ImportDialog._poll_file_dialog` qui appelle l'importeur choisi.
- `apps/object_editor.py` importe `texture_search_dirs_for` ; `object_editor.py` connecte `ImportDialog` à ses propres callbacks `_on_import_new_shape`/`_on_import_replace`.
- `apps/shape_importer.py` importe `IMPORTERS, ShapeImportError, find_importer` pour le CLI équivalent.
- `import_watcher.py` importe `ShapeImportError, find_importer` pour la surveillance automatique de fichiers (relance l'import quand le fichier source change), en réutilisant `find_importer` à plusieurs endroits (`import_watcher.py,112,150`).
- Dépend uniquement de `pynel.ryzom_shape` pour les types de données du Mesh (non détaillé ici, package tiers).

## Points notables / pièges

- Un shape importé n'a **jamais de LOD** — c'est une limitation structurelle de pynel (pas de binding pour `CMRMBuilder`), pas un oubli (`shape_import.py`).
- Les valeurs par défaut du matériau construit (`_NEL_DEFAULT_GRAY = Rgba(150,150,150,255)`, `_NEL_DEFAULT_SPECULAR`, `_NEL_DEFAULT_SHININESS = 8.0`, etc., `shape_import.py`) sont **délibérément déconnectées** des valeurs diffuse/ambient/specular/opacity du fichier source — elles reproduisent le comportement du vrai pipeline Ryzom (le plugin NeL de 3ds Max ignore aussi la matière source et impose ses propres défauts). Seule la texture diffuse et `double_sided` (nécessité géométrique réelle) sont reprises du fichier importé. Confirmé par comparaison d'un export Cinema4D→fbx→3dsMax→shape réel vs import direct (voir `logs/forgery-object-editor.md`, référencé en commentaire).
- Conversion V des UV (`shape_import.py`) : `.obj`/`.dae`/`.fbx` utilisent la convention native du format (V=0 en bas, façon OpenGL) ; les vrais fichiers `.shape` NeL stockent V=0 en haut. La conversion `1.0 - v` est appliquée une fois pour toutes à l'import (`_assemble_mesh`), pas au moment de l'affichage — sinon le fichier `.shape` sauvegardé serait faux au rechargement et dans le vrai moteur.
- Conversion d'axes Y-up→Z-up (`shape_import.py`) : Assimp normalise toujours en Y-up canonique, quel que soit l'axe déclaré par le fichier source (COLLADA `<up_axis>` ou métadonnées FBX). L'ancien import `.dae` via `pycollada` (abandonné, voir historique git) donnait l'illusion d'un round-trip correct par pure coïncidence : les exports Forgery `.dae` déclarent `Y_UP` alors que leurs données sont réellement Z-up (`shape_import.py`) — sans `_YUP_TO_ZUP_MATRIX`, un vrai fichier `.dae` correctement tagué `Z_UP` aurait été mal orienté.
- `_mat_mul_dir` (normales) utilise la sous-matrice 3x3 directement plutôt qu'une inverse-transpose : correct seulement pour rotation/scale uniforme ; les nœuds FBX à scale non-uniforme (rares) peuvent donner des normales légèrement fausses — limite assumée et documentée (`shape_import.py`).
- `Process_GlobalScale` est nécessaire pour éviter un mesh 100x trop grand sur un fichier FBX authored en centimètres (défaut Blender) — sans lui, Assimp laisse le facteur d'échelle sur le nœud racine au lieu de l'appliquer (`shape_import.py`).
- Les normales générées par Assimp (`Process_GenNormals`) ne sont pas de longueur unitaire dans les unités d'origine du fichier — `_normalize` est nécessaire après tout transform de nœud, systématiquement (`shape_import.py`).
- `.obj` seul gère un matériau par défaut nommé `_DEFAULT_MATERIAL_NAME = "__default__"` pour les faces sans `usemtl` (`shape_import.py`, `355`).
