# Installation des données pipeline (téléchargement)

**Fichiers :** `nel/tools/forgery/ryzom_forgery/pipeline_data_installer.py`,
`continent_ecosystem.py`, `pipeline_data_install_dialog.py`

## Rôle

Les données du pipeline `build_gamedata` (trop volumineuses pour être
versionnées dans `ryzom-data`) sont publiées par Nuno sous forme d'archives
`.zip` téléchargeables. Ce système propose de les installer automatiquement
sous `<ryzom-data>/pipeline/` quand Atyscape en a besoin, plutôt que
d'obliger à les installer/localiser à la main.

## Catégories

Trois catégories, chacune avec sa base URL et son sous-dossier cible sous
`<ryzom-data>/pipeline/` (`pipeline_data_installer.CATEGORIES`) :

| Catégorie | Base URL | `<name>` = | Cible |
|---|---|---|---|
| `landscape` | `https://download.ryzom.com/tools/landscape/<name>.zip` | écosystème (`desert`/`jungle`/`lacustre`/`primes_racines`) | `pipeline/landscape/<name>/` |
| `pipeline_ecosystems` | `https://download.ryzom.com/tools/pipeline/<name>.zip` | écosystème (mêmes noms, contenu différent) | `pipeline/export/ecosystems/<name>/` |
| `pipeline_continents` | `https://download.ryzom.com/tools/pipeline/<name>.zip` | continent (ex. `nexus`) | `pipeline/export/continents/<name>/` |

Chaque archive contient déjà le dossier `<name>/` à sa racine, avec un
contenu déjà réduit à ce dont Atyscape a besoin — c'est Nuno qui fabrique
ces archives (`.zip`, pas `.7z`, pour rester lisible par le module stdlib
`zipfile` sans dépendance externe), donc `pipeline_data_installer.py` ne
filtre/trie jamais leur contenu.

`landscape` n'est pas une catégorie secondaire : `landscape/<écosystème>/
zones/` (bricks `.zone` bruts) et `zoneligos/` sont le seul point de départ
pour composer un continent depuis son `.land` tant qu'aucun `.zone` par
continent n'a été généré via `land_export` (pas encore câblé côté Forgery,
futur chantier `land_composition`).

## `pipeline_data_installer.py`

- `is_installed(category, name) -> bool` — le dossier `<cible>/<name>/`
  existe et n'est pas vide.
- `download_and_install(category, name, progress: dict)` — télécharge
  `<base_url><name>.zip` en streaming (`urllib.request`, stdlib), met à
  jour `progress["phase"]` (`"downloading"`/`"extracting"`) et, pendant le
  téléchargement, `progress["downloaded_bytes"]`/`progress["total_bytes"]`
  (`total_bytes` reste `None` si le serveur n'envoie pas de
  `Content-Length` — barre de progression indéterminée), puis extrait avec
  `zipfile` (stdlib, lit aussi bien du DEFLATE que du LZMA — un `.zip`
  écrit en LZMA, ex. via `7z a -tzip -mm=LZMA`, se lit sans changement de
  code) dans le dossier cible, puis supprime le fichier temporaire. Lève
  `PipelineDataInstallError` (échec réseau, 404, zip invalide, `ryzom-data`
  non configuré via `pynel.repository_paths`) — ne capture jamais
  l'exception dans `progress` elle-même, c'est la responsabilité de
  l'appelant (voir `pipeline_data_install_dialog.py`).

`ryzom-data` est résolu en interne via `pynel.repository_paths.get
("ryzom-data")` (jamais passé en paramètre par l'appelant, contrairement à
`region_loader.py`).

## `continent_ecosystem.py`

`get_ecosystem_for_continent(continent_name) -> Optional[str]` — pas de
table figée (de nouveaux continents seront ajoutés). Deux sources, dans
l'ordre :

1. **Source principale** : `<ryzom-data>/leveldesign/workspace/continents/
   <nom>/directories.py`'s `EcosystemName` (régex sur le `.py`, sans
   l'exécuter). Confirmée présente pour les 24 continents actifs, **y
   compris les `r2_*`** (Ring) — piège trouvé 2026-09-09 : le champ
   `Ecosystem` d'un `.continent` peut être absent même quand l'écosystème
   existe bien (`r2_desert` → `"desert"` via `directories.py`, alors que
   son `.continent` n'a aucun champ `Ecosystem`). `directories.py` utilise
   aussi la bonne convention de nom (`primes_racines`), alors que certains
   `.continent` disent `primes_roots.ecosystem` — une autre raison de
   préférer cette source.
2. **Repli** : `<ryzom-data>/leveldesign/world/continents/<nom>.continent`
   via `pynel.ryzom_continent.load_continent`, pour les continents absents
   de `workspace/` (seul cas connu : `testroom`).

Résultat mis en cache mémoire pour la session (pas de persistance disque).
`None` si aucune des deux sources ne connaît `continent_name`.

## `pipeline_data_install_dialog.py` — `PipelineDataInstallDialog`

- `open(items: list[(category, name)])` — ouvre la popup "Éléments
  manquants : ... Télécharger et installer ?" pour 1 à 3 éléments, sauf si
  déjà refusé pour cet ensemble exact dans la session (`_declined_sets`,
  mémoire uniquement, oublié au relancement).
- `draw()` — à appeler une fois par frame. Sur "Oui", lance
  `download_and_install` séquentiellement pour chaque élément dans un
  thread daemon (même schéma `progress` dict + `imgui.progress_bar()` que
  `landscape_editor.py`'s `_load_continent`/`_apply_render_mode`). Largeur
  de wrap du texte fixée explicitement (`push_text_wrap_pos`) — cette
  popup n'a pas d'autre contenu large pour ancrer `always_auto_resize`,
  sans quoi elle s'écrase sur la largeur (bug réel trouvé 2026-09-09).

**Piège ImGui important** : ne jamais appeler `dialog.open(...)` depuis
l'intérieur d'un `imgui.begin_combo()`/`end_combo()` (ou toute autre popup)
— `imgui.open_popup()` pour une popup différente est silencieusement perdu
à la fermeture de la popup englobante. `landscape_editor.py` différait donc
l'appel via `self._pipeline_data_install_pending`, consommé au
`draw_panel()` suivant, hors de tout combo (bug réel trouvé 2026-09-09).

## Intégration continents — `landscape_editor.py`

Au chargement d'un continent via le combo (`_load_continent`), calcule la
liste des éléments manquants et la stocke dans
`self._pipeline_data_install_pending` (jamais ouverte directement ici, voir
le piège ImGui ci-dessus) :

- `("pipeline_continents", continent)` si absent ;
- `("pipeline_ecosystems", écosystème)` et `("landscape", écosystème)` si
  `continent_ecosystem.get_ecosystem_for_continent(continent)` renvoie un
  écosystème et qu'ils sont absents.

`draw_panel()` consomme `self._pipeline_data_install_pending` en tout début
de frame (avant la barre d'onglets, donc hors de tout combo) pour appeler
`self._pipeline_data_install_dialog.open(...)`, puis appelle
`self._pipeline_data_install_dialog.draw()` à chaque frame.

Le chargement normal du continent se poursuit dans tous les cas (Oui/Non/
téléchargement en cours) — la donnée pipeline n'est jamais bloquante,
`live_data_path` reste la source par défaut. Une fois l'installation
terminée, la source de zones par continent (`region_loader.py`, voir
`docs/zone_tools.md`) redétecte `pipeline/export/continents/<continent>/`
normalement au prochain scan, sans code de bascule supplémentaire.
