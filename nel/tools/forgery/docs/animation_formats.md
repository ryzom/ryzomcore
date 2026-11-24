# Formats d'animation importables / exportables

Le format `.anim` lui-même (parsing, types de tracks, évaluation) est documenté dans `nel/tools/pynel/docs/animation.md`. Cette page ne couvre que les conversions.

## Import (source → `.anim` Ryzom)

- Formats : `.fbx`, `.dae`, `.gltf`, `.glb` (via `assimp_py`). `.obj` n'a pas d'animation : résultat vide.
- `shape_import.extract_animation(path, skeleton)` : un `Animation` par clip trouvé dans `scene.animations` (tous, sans filtre de nom), échantillonné à 30 Hz en marchant la hiérarchie d'os depuis `_find_armature_root()` (comme `extract_skeleton()`). Rotation en nlerp (lerp + renormalisation), pas un slerp complet.
- `ImportWatcher._maybe_export_animations()` (appelée après le traitement du `.skel`) écrit un `.anim` par clip, nommé d'après le clip (pas la source), dans `<workspace>/anims/` si absent du workspace, et **toujours réécrit** à chaque réimport. Voir `import_watcher.md`.
- Squelette : lu depuis l'armature du fichier source (voir `import_watcher.md`, squelette partagé).

## Export (`.anim` Ryzom → source)

- Formats avec skin + animation : `.dae`, `.fbx`, `.gltf`, `.glb`, via `shape_export.export_mesh_with_skin(..., animation=...)` (un seul writer, `assimp_py.export_file()`). `.obj` et `.stl` : géométrie seule.
- `_assimp_animation_from_pynel()` : un canal par os ayant au moins un track `.pos`/`.rotquat`/`.scale`, rééchantillonné à 30 Hz quel que soit le format interne du `.anim` (Sampled ou KeyFramer). Durée = `animation_duration()`.
- Pas d'animation exportée sur un mesh rigide (condition `not rigid` dans le code).
- Limite : la compensation `UnheritScale` n'est pas représentée ; un vrai `.skel` 3dsMax avec des os `unherit_scale=True` donnerait une bind pose subtilement fausse (voir `shape_export.md`).
- Pas encore branché à l'UI Patina d'après `shape_export.md` ; `export_dialog.export()`/`quick_export()` acceptent toutefois un paramètre `animation`.

## Lecture dans Patina

Le `.anim` natif est joué par la Skinning preview (`object_editor.md`) et par le Mode/animation des PNJ (`creature_ref.resolve_animation()`).

## Réserve

Le writer `.anim` (`save_animation`, `build_animation`) est référencé par le code Forgery mais absent du pynel de ce checkout : voir `pynel/docs/animation.md`.
