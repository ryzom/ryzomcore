# ai_wmap_export (app CLI)

**Fichier :** `nel/tools/forgery/ryzom_forgery/apps/ai_wmap_export.py` (créé le 2026-09-16)

## Rôle

App CLI pure (pas de GUI, même famille que `apps/dds_export.py`/`apps/shape_exporter.py`/
`apps/shape_importer.py`) : régénère l'`ai_wmap` d'un continent. Wrapper fin autour de la
bibliothèque `ryzom_forgery/ai_wmap_export.py` (voir `docs/ai_wmap_export.md` pour tout le
détail -- manifeste `.ig`, résolution d'écosystème, config, enchaînement des outils natifs).

## API principale

- `main(argv=None)` -- point d'entrée CLI, `argparse`.
- Arguments :
  - `continent` : nom du continent (positionnel, ex. `bagne`).
- Résout `ryzom-data` via `pynel.repository_paths.get("ryzom-data")` (Settings >
  Repository Paths) -- jamais un chemin en argument, cohérent avec le reste de Forgery où
  ce chemin est configuré une fois pour toutes.
- `if __name__ == "__main__": main`.

## Utilisation

```
python -m ryzom_forgery.apps.ai_wmap_export bagne
```

Importe `generate_ai_wmap`/`AiWmapExportError` depuis `ryzom_forgery.ai_wmap_export` --
toute la logique réelle vit dans ce module lib, cette app ne fait que l'assemblage
argparse + gestion des erreurs CLI (`SystemExit` sur échec).

## Points notables / pièges

- Suit exactement le même moule que `apps/dds_export.py` : `main(argv=None)` testable
  sans sous-processus, erreurs utilisateur via `raise SystemExit(...)`.
- **Convention générale pour tout futur script autonome de Forgery** (pas de GUI) :
  toujours cette même coupe en deux fichiers, jamais un seul script monolithique --
  1. la vraie logique (I/O, subprocess, calculs) va dans un module au niveau racine de
     `ryzom_forgery/` (bibliothèque pure, importable par n'importe quel autre app ou
     module, y compris une future GUI qui voudrait exposer le même bouton) ;
  2. `apps/<meme_nom>.py` ne fait qu'assembler `argparse` et appeler cette bibliothèque --
     jamais de logique métier, jamais de chemin de fichier en dur (tout se résout via
     `settings.py`/`repository_paths`, jamais un `/home/ulukyn/...` codé en dur comme
     dans un script agentcom jetable).
  - Un script destiné à durer (pas un jet-and-throw de debug) ne doit jamais rester dans
    `~/.cache/agentcom/` : ce dossier n'est qu'un pont d'exécution vers la machine réelle
    (le sandbox ne peut pas lancer les binaires natifs), ni partagé avec l'équipe ni
    garanti de survivre (convention Linux d'un `~/.cache`) -- tout code destiné à durer
    ou à être partagé va dans le repo Forgery, comme n'importe quel autre app.
- Pas (encore) enregistré comme `console_scripts` dans `pyproject.toml` -- comme les
  autres apps CLI du projet, s'utilise via `python -m ryzom_forgery.apps.ai_wmap_export`.
