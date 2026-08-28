# dds_export (app CLI)

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/dds_export.py` (~46 lignes)

## Rôle

App CLI pure (pas de GUI, pas de dépendance à `app.py`/`ForgeryApp`, même famille que
`apps/shape_exporter.py`/`apps/shape_importer.py`) : convertit un fichier TGA/PNG en
`.dds` chargeable par le client Ryzom. Wrapper fin autour de la bibliothèque
`ryzom_forgery/dds_export.py` (voir `docs/dds_export.md` pour tout le détail
algorithmique — conteneur DDS, mipmaps, compression DXT via Panda3D).

## API principale

- `main(argv=None)` — point d'entrée CLI, `argparse`.
- Arguments :
  - `input` : fichier TGA ou PNG source (positionnel).
  - `-o/--output` : chemin `.dds` de sortie (défaut : même nom, extension `.dds`).
  - `-a/--algo {1,1a,3,5}` : algo DXT (défaut : `pick_default_algo` — DXT1 si pas
 d'alpha, DXT5 sinon, même heuristique que `tga2dds.cpp`).
  - `-m/--mipmap` : construit la chaîne de mipmaps.
  - `-g/--grayscale` : charge une source mono-canal en luminance visible plutôt qu'en
 masque alpha.
  - `-r/--reduce N` (0-8) : réduit la taille de l'image avant compression.
- `if __name__ == "__main__": main`.

## Utilisation

Importe `DXT1`/`DXT1A`/`DXT3`/`DXT5`, `build_dds`, `load_rgba`, `pick_default_algo`
depuis `ryzom_forgery.dds_export` — toute la logique réelle vit dans ce module
lib, cette app ne fait que l'assemblage argparse + gestion des erreurs CLI
(`SystemExit` sur échec de lecture).

## Points notables / pièges

- Suit exactement le même moule que `apps/shape_exporter.py`/`apps/shape_importer.py` :
 fonction `main(argv=None)` testable sans sous-processus, erreurs utilisateur via
 `raise SystemExit(...)` plutôt que des exceptions qui remonteraient une trace Python.
- N'est pas (encore) enregistré comme `console_scripts` dans
 `nel/tools/forgery/pyproject.toml` — comme les deux autres apps CLI du projet,
 s'utilise via `./dev.sh ryzom_forgery/apps/dds_export.py ...` ou
 `python -m ryzom_forgery.apps.dds_export`.
- Pas encore appelé par Patina — c'est `dds_export.build_dds`/`load_rgba` (les
 fonctions de bibliothèque, pas cette app CLI) qui seront réutilisées par le futur
 watcher d'auto-export `tex/` → `dds/` (chantier `patina-tex-dds-autoexport`).
