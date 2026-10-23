# scene.py -- modèle de scène Patina

Données pures (pas d'UI, pas de Panda3D) d'une scène multi-shapes, sérialisée en JSON dans `<workspace>/scenes/*.scene` (dossier créé à la demande, workspaces externes compris). Chantier : `project-todos/forgery/patina-multi-shapes-scenes.md`.

- `Scene` : `entries`, `edited_id` (entrée en cours d'édition), `solo_id` (seule entrée affichée), `creature` (PNJ unique partagé), `ui` (dict opaque : caméra, panneaux, lumières, vent...).
- `SceneEntry` : `id`, `source`, `visible`, `transparent`, `placement` (`auto`/`origin`/`pivot`), `bind_slot` / `bind_attach_point`. `editable` est faux pour les 3 shapes de référence (jamais éditables, jamais solo).
- Sources JSON : `reference` (label), `workspace` (chemin relatif), `file` (absolu), `bnp` (bnp absolu + nom).
- `from_dict`/`load_scene` retirent silencieusement les entrées dont la source n'existe plus (ou mal formées) et remettent `edited_id`/`solo_id` à vide si elles pointaient dessus. `bnp_has(bnp, nom)` optionnel pour vérifier l'entrée dans le bnp (sinon seul le `.bnp` est vérifié).
- `save_scene(scene, workspace_dir, "default.scene")` ; `DEFAULT_SCENE_NAME = "default.scene"`.
