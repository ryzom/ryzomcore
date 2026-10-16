# shape_importer

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/shape_importer.py` (46 lignes)

## Rôle

App 100% CLI (pas de GUI) : convertit un fichier mesh (`.obj`, `.dae`,
`.fbx`) en fichier `.shape` Ryzom. Le format d'entrée est déterminé par
l'extension du fichier `INPUT` (`shape_importer.py,19-24`). Le résultat
est toujours un `CMesh` (jamais de LOD / `CMeshMRM`), cf. docstring
`shape_importer.py` qui renvoie vers le docstring de module de
`shape_import.py` pour l'explication.

## API principale

- `_find_importer(input_path: Path)` (`shape_importer.py`) — délègue
 à `find_importer` de `shape_import.py` ; lève `SystemExit` avec la liste
 des formats supportés (`IMPORTERS`) si aucun importeur ne correspond.
- `main(argv=None)` (`shape_importer.py`) — point d'entrée : parse les
 arguments, trouve l'importeur, exécute `importer(args.input)` pour obtenir
 un `mesh`, puis sauvegarde via `save_shape` dans un `ShapeFile(type_name="Mesh", value=mesh)`.

Arguments CLI (déclarés `shape_importer.py`) :
- `input` (positionnel) : fichier mesh source (`.obj`/`.dae`/`.fbx`).
- `output` (positionnel) : fichier `.shape` destination.

Pas d'option `--data-root` ni `--texture-mode` ici (contrairement à
`shape_exporter.py`) : l'import ne gère pas la résolution de textures dans
cette app.

## Utilisation

Importe :
- `ryzom_forgery.shape_import.IMPORTERS`, `ShapeImportError`,
 `find_importer` — la logique réelle de conversion mesh → géométrie Ryzom,
 documentée dans `docs/shape_import.md`.
- `pynel.ryzom_shape.ShapeFile`, `save_shape` — pour construire et écrire
 le conteneur `.shape` final (`shape_importer.py`).

Lancement : script autonome avec `if __name__ == "__main__": main`
(`shape_importer.py`), pas de dépendance à `app.py` (`ForgeryApp`) —
comme `shape_exporter.py`, ce n'est pas une app graphique Panda3D/ImGui.

## Points notables / pièges

- Contrairement à Patina (`object_editor.py`), aucune interface graphique :
 conversion batch en ligne de commande uniquement, pas d'aperçu, pas
 d'édition interactive.
- Limitation explicite et assumée : produit uniquement un `CMesh` simple,
 sans niveaux de détail (LOD) — `CMeshMRM` ne peut pas être construit par
 cette voie (docstring `shape_importer.py`, détail dans
 `shape_import.py`).
- Les erreurs (`OSError`, `ShapeImportError`) levées par l'importeur sont
 converties en `SystemExit` avec message, pas de traceback brut
 (`shape_importer.py`).
- Le dossier parent de `output` est créé automatiquement si besoin
 (`shape_importer.py`).
- Fichier plus court et plus simple que `shape_exporter.py` : pas de gestion
 de textures, pas d'options supplémentaires, une seule responsabilité
 (choisir l'importeur, convertir, sauvegarder).
