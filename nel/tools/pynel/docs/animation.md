# Animations `.anim` (pynel.ryzom_animation)

Module : `pynel/ryzom_animation.py`. Format natif NeL `CAnimation` (magic disque `NEL_ANIM`), reverse-engineeré depuis `nel/src/3d/animation.cpp` et les `serial()` des tracks.

## Lecture

- `load_animation(path | fichier binaire)` / `parse_animation(bytes)` → `Animation(name, id_by_name, tracks, min_end_time, sss_shapes)`.
- `id_by_name` : nom de canal (`"<os>.pos"`, `"<os>.rotquat"`, `"<os>.scale"`) → index dans `tracks`. Un track peut être `None` (« null track »).
- CLI : `python -m pynel.ryzom_animation fichier.anim` affiche les tracks (`_dump`).

## Types de tracks (`Track.class_name`)

| Classe NeL | Contenu |
|---|---|
| `CTrackDefaultVector` / `CTrackDefaultQuat` | valeur constante (`value`) |
| `CTrackSampledVector` / `CTrackSampledQuat` | clés échantillonnées à pas fixe (`keys`, `time_blocks`), quaternions dépackés (`CQuatPack`) |
| `CTrackKeyFramerLinearVector` / `CTrackKeyFramerLinearQuat` | clés irrégulières (`keyframes` = paires temps/valeur) |

Toute autre sous-classe d'`ITrack` lève `AnimationParseError` (pas de données silencieusement fausses).

Répartition mesurée sur le live (`forgery/docs/anim_type_survey.md`, 4568 `.anim`) : 4357 Sampled/Default, 153 KeyFramer, 45 mixtes, 11 non parsés (classe de track non supportée), 2 vides. [MEASURED]

## Évaluation

- `evaluate_track(track, time)` : valeur d'un canal à `time` (secondes) ; slerp pour les quaternions.
- `animation_duration(anim)` : plus grand `end_time` des tracks (les constantes ne comptent pas).
- `evaluate_bone_world_matrix(skeleton, bone_name, anim, time)` / `evaluate_all_bone_world_matrices(skeleton, anim, time)` : matrices monde des os, utilisées par la Skinning preview de Patina.
- `_bone_local_matrix()` et `_unherit_scale_comp()` : pose locale d'un os, compensation `UnheritScale` de `CBone::compute()`.

## Écriture

- Le module est **lecture seule** d'après son docstring d'en-tête. Forgery importe pourtant `save_animation`/`AnimationWriteError` (`import_watcher.py:35`) et `build_animation` (`shape_import.py:1371`). Un grep de ces trois noms dans `nel/tools/pynel` de ce checkout ne les trouve pas : [MEASURED] absents de cette branche. À vérifier (chantier `project-todos/pynel/anim_write.md` cité dans le code).
- Le docstring d'en-tête est aussi périmé sur un point : il dit que `CTrackKeyframer` n'est pas supporté, alors que `Track` et `_parse_keyframer_linear` le gèrent.

## Conversions vers/depuis fbx, dae, gltf

Voir `nel/tools/forgery/docs/animation_formats.md`.
