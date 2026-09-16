# ai_wmap_export

**Fichier :** `nel/tools/forgery/ryzom_forgery/ai_wmap_export.py` (créé le 2026-09-16)

## Rôle

Régénère l'`ai_wmap` d'un continent (les cartes de marchabilité `<continent>_0/1/2.tga`
utilisées par le pathfinding IA) en pilotant le binaire natif `ai_build_wmap` — voir
`project-todos/forgery/ai_wmap_flora_collision.md` pour l'investigation complète.

Contrairement au reste du pipeline (`land_build.py`, via `pynel.ryzom_land_tools`/
`ryzom_pacs_tools`), ce module **n'est pas wrappé par `pynel`** : `ai_wmap` n'est pas (encore)
un stage de `land_build.py`, donc ses appels natifs restent locaux ici — décision Nuno,
2026-09-16.

## API principale

- `generate_ai_wmap(ryzom_data_path, continent)` — point d'entrée unique. Régénère
  `pipeline/export/continents/<continent>/ai_wmap/` en repartant de zéro (contenu
  précédent supprimé), et retourne un `AiWmapResult` (`wmap_paths`, `pacscrunch_log`).
  Lève `AiWmapExportError` si `zone_lighted_ig_land/` ou `rbank_output/` n'existent pas
  encore (le continent doit avoir déjà été buildé, cf. `land_build.py` Stages 7-8), ou si
  un des outils natifs sort en erreur.
- `_resolve_binary()` — résout `ai_build_wmap`(`.exe`) dans le dossier d'outils des
  Settings (`Settings > Ryzom Paths`), même convention que `land_build.py`'s
  `_resolve_binary()`/`zone_tools.py`'s `_resolve_zone_welder_binary()`.
- `AiWmapExportError` — exception dédiée du module.

## Utilisation

- CLI : `apps/ai_wmap_export.py` (voir `docs/apps/ai_wmap_export.md`).
- Pas encore intégré dans `land_build.py`/l'UI de `landscape_editor.py` — `ai_wmap` reste
  un outil de validation/diagnostic, pas un stage automatique du bouton "Build".

## Points notables / pièges

- **Bug natif contourné** : `ai_build_wmap`'s `loadPacsPrims()` (`continent_container.cpp:322`)
  lit sa liste de `.ig` depuis `sheet.LandscapeIG`, un manifeste texte
  (`<continent>_ig.txt`, un nom de fichier par ligne) qui doit vivre **dans le même
  dossier que les `.ig` finaux** (`zone_lighted_ig_land/`) — jamais généré par aucun
  autre stage du pipeline. Sans lui (et sans ce dossier dans `Paths=`), `pacsCrunch`
  charge silencieusement 0 IG / 0 bloc de collision, sans erreur. `generate_ai_wmap()`
  génère ce manifeste lui-même à chaque appel, avant de le supprimer en fin d'exécution.
- Le dossier `pacs_prim` (collision des primitives/ressources racines) dépend de
  l'écosystème du continent (`pipeline/export/ecosystems/<ecosystem>/pacs_prim`), résolu
  via `continent_ecosystem.get_ecosystem_for_continent()` — jamais un chemin fixe (une
  version précédente, jetable, de ce script avait `primes_racines` en dur, valable
  seulement pour `bagne`).
- `ai_build_wmap.cfg` est écrit temporairement dans `rbank_output/` (répertoire de travail
  imposé par l'outil natif, même convention que le `run_dir` de `ryzom_pacs_tools`) puis
  supprimé après usage, comme les deux manifestes.
- Validé 2026-09-16 sur `bagne` réel (via agentcom, le sandbox ne pouvant pas exécuter
  `ai_build_wmap`) : `Loaded 53 IGs` / `Added 2646 primitive blocs`, écart pixel résiduel
  0.024-0.033% contre `bagne_ref/ai_wmap` (bruit de précision flottante inter-plateforme,
  accepté — cf. root cause #3 du chantier).
