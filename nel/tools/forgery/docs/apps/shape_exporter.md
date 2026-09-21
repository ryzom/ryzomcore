# shape_exporter

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/shape_exporter.py` (78 lignes)

## Rôle

App 100% CLI (pas de GUI) : convertit un fichier `.shape` Ryzom vers un autre
format de mesh (`.obj`, `.dae`, `.stl`, `.gltf`, `.glb`). Le format de sortie
est déterminé par l'extension du fichier `OUTPUT` passé sur la ligne de
commande (`shape_exporter.py,23-29`).

## API principale

- `_find_format(output_path: Path)` (`shape_exporter.py`) — cherche
 dans `EXPORT_FORMATS` (importé de `shape_export.py`) le format dont
 `extension` correspond au suffixe de `output_path` ; lève `SystemExit`
 avec la liste des formats supportés sinon.
- `main(argv=None)` (`shape_exporter.py`) — point d'entrée : parse les
 arguments, résout le format d'export, construit éventuellement un
 `texture_finder`, parse le `.shape` source, appelle
 `export_format.export(...)` et affiche les chemins écrits.

Arguments CLI (déclarés `shape_exporter.py`) :
- `input` (positionnel) : fichier `.shape` source.
- `output` (positionnel) : fichier destination, son extension sélectionne le
 format d'export.
- `--data-root PATH` : racine d'un arbre de données Ryzom, utilisée pour
 résoudre les textures (nécessaire pour `--texture-mode copy_png`).
- `--texture-mode {copy_png,reference_only}` : optionnel ; si omis, vaut
 `copy_png` quand `--data-root` est fourni, sinon `reference_only`
 (`shape_exporter.py`).

## Utilisation

Importe :
- `ryzom_forgery.search_paths` — pour `build_texture_index` /
 `find_texture`, afin de construire un `texture_finder` local à partir de
 `--data-root` (`shape_exporter.py,52-58`). C'est le même scan
 récursif « .bnp-aware » utilisé par la liste « Paths » de la GUI, mais
 ici en une passe non mise en cache, car le process est court-vécu
 (commentaire `shape_exporter.py`).
- `ryzom_forgery.settings` — pour `SearchPathDir`,
 `TEXTURE_MODE_COPY_PNG`, `TEXTURE_MODE_REFERENCE_ONLY`.
- `ryzom_forgery.shape_export.EXPORT_FORMATS` — la logique d'export réelle
 et la liste des formats supportés (documentée dans `docs/shape_export.md`).
- `pynel.ryzom_shape.parse_shape` / `ShapeParseError` — pour lire et parser
 le fichier `.shape` d'entrée en bytes (`shape_exporter.py`).

Lancement : script autonome avec `if __name__ == "__main__": main`
(`shape_exporter.py`), sans dépendance à `app.py` (`ForgeryApp`) — ce
n'est pas une app graphique Panda3D/ImGui.

Un seul fichier `.shape` traité par invocation. Les matériaux du shape
(`shape_file.value.materials`, récupérés via `getattr` pour tolérer leur
absence, `shape_exporter.py`) sont transmis à `export_format.export`,
qui peut copier les textures décodées en `.png` à côté du fichier exporté
si `texture_mode == TEXTURE_MODE_COPY_PNG`.

## Points notables / pièges

- Contrairement à Patina (`object_editor.py`), aucune interface graphique :
 pas d'aperçu 3D, pas d'interaction, juste une conversion batch en ligne de
 commande.
- Les noms de textures dans un `.shape` ne référencent qu'un nom de fichier,
 jamais un chemin complet ; sans `--data-root`, l'export ne peut donc que
 laisser une référence vers ce nom (mode `reference_only`) — impossible de
 copier le `.png` réel (commentaire du docstring, `shape_exporter.py`).
- `--texture-mode copy_png` sans `--data-root` est une erreur explicite
 (`SystemExit`, `shape_exporter.py`).
- Les erreurs de lecture/parsing (`OSError`, `ShapeParseError`) et d'export
 (`ValueError`) sont converties en `SystemExit` avec message, pas de
 traceback Python brut pour l'utilisateur final (`shape_exporter.py,68-71`).
- Le dossier parent de `output` est créé automatiquement si besoin
 (`shape_exporter.py`).
