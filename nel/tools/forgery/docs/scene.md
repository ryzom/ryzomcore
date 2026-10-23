# scene.py -- modèle de scène Patina

Données pures (pas d'UI, pas de Panda3D) d'une scène multi-shapes, sérialisée en JSON dans `<workspace>/scenes/*.scene` (dossier créé à la demande, workspaces externes compris). Chantier : `project-todos/forgery/patina-multi-shapes-scenes.md`.

- `Scene` : `entries`, `edited_id` (entrée en cours d'édition), `solo_id` (seule entrée affichée), `creature` (PNJ unique partagé), `ui` (dict opaque : caméra, panneaux, lumières, vent...).
- `SceneEntry` : `id`, `source`, `visible`, `transparent`, `placement` (`auto`/`origin`/`pivot`), `bind_slot` / `bind_attach_point`, `kept` (faux seulement pour le « temp shape », jamais sauvé : toute entrée lue dans une scène est `kept`). `editable` est faux pour les 3 shapes de référence (jamais éditables, jamais solo).
- Sources JSON : `reference` (label), `workspace` (chemin relatif), `file` (absolu), `bnp` (bnp absolu + nom).
- `from_dict`/`load_scene` retirent silencieusement les entrées dont la source n'existe plus (ou mal formées) et remettent `edited_id`/`solo_id` à vide si elles pointaient dessus. `bnp_has(bnp, nom)` optionnel pour vérifier l'entrée dans le bnp (sinon seul le `.bnp` est vérifié).
- `save_scene(scene, workspace_dir, "default.scene")` ; `DEFAULT_SCENE_NAME = "default.scene"`.
- Nom de fichier : l'extension est toujours `.scene` (l'utilisateur ne saisit que le nom). À la sortie, Patina sauve la scène dans le fichier d'où elle vient (`default.scene` si elle n'en a jamais eu), `settings.last_scene_name` retient lequel pour proposer son rechargement au lancement.
- Le côté Patina (UI, état, sauvegarde) est dans `ryzom_forgery/apps/object_editor_mixins/scene_io.py`, voir `docs/apps/object_editor.md`.
