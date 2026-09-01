# Explorer (arborescence de fichiers)

**Fichier :** `nel/tools/forgery/ryzom_forgery/explorer.py` (~665 lignes)

## Rôle

`Explorer` est le panneau de gauche standard des apps Forgery (Patina en
premier lieu) : un navigateur de fichiers qui parcourt une arborescence sur
disque, sait "entrer" dans les archives `.bnp`/`.bnpe` comme si c'étaient des
dossiers (`explorer.py,332-341,483-532`), propose une recherche par nom et
un filtre par extension (glob, avec des préréglages `*.shape`/`*.skel`/
`*.anim`/`*`, `explorer.py,447-452`), une sélection multiple (ctrl+clic,
`explorer.py,643-650`), des favoris persistés (`explorer.py,
343-376`) et un menu contextuel construit dynamiquement à partir d'un
`CommandRegistry` (`explorer.py`). Il fournit aussi un mini
navigateur permanent ("Wexplorer") qui reste accessible quel que soit
l'endroit courant de l'arborescence principale, sous deux formes possibles :
les dossiers épinglés (`pinned_folders`, réels) et/ou les catégories
virtuelles (`virtual_categories_source`, voir plus bas —
forgery-workspace-projects chantier, 2026-09) que l'app hôte peut fournir
en plus ou à la place.

## API principale

- `ExplorerItem` (`explorer.py`) : une entrée sélectionnable — soit un
 vrai chemin disque, soit un fichier virtuel dans un `.bnp` (`bnp_path` posé
 alors, `name` étant le nom de l'entrée dans l'archive et non un chemin
 réel). `read_bytes` (`explorer.py`) lit via `BnpReader` ou
 directement le fichier selon le cas.
- `Explorer.__init__` (`explorer.py`) : construit l'état — racine
 courante, filtre/recherche, favoris, caches de listing (`_dir_cache`,
 `_bnp_cache`, `_bnp_visible_cache`), état des vignettes de textures,
 callbacks (`on_selection_changed`, `extra_header`, `extra_toolbar`),
 `pinned_folders`, et `virtual_categories_source` (voir plus bas).
- `draw` (`explorer.py`) : point d'entrée appelé chaque frame ImGui
 — dessine dans l'ordre l'en-tête optionnel de l'app hôte, le Wexplorer, la
 barre d'outils (Refresh + extra), recherche/filtre, barre de chemin,
 favoris, puis la liste principale (dossier ou contenu de `.bnp`).
- `refresh` (`explorer.py`) : vide les caches de listing pour
 forcer une relecture disque au prochain `draw`.
- `_draw_path_bar` / `_compute_path_suggestions` /
 `_draw_path_suggestions_popup` (`explorer.py`) : barre de saisie
 de chemin avec autocomplétion de sous-dossiers, navigable au clavier
 (flèches haut/bas), sélection par clic ou Entrée.
- `_draw_pinned_folders` / `_draw_tree_children` (`explorer.py`) :
 le "Wexplorer" — dossiers épinglés par l'app hôte (`pinned_folders`),
 affichés en arbre extensible sur place, non affecté par le filtre/la
 recherche de la vue principale, avec vignettes de texture.
- `_draw_pinned_virtual_categories` (`explorer.py`) : même emplacement/
 principe que `_draw_pinned_folders`, mais pour `virtual_categories_source`
 (`callable() -> {catégorie: [Path, ...]}`, optionnel) au lieu de vrais
 sous-dossiers -- chaque catégorie est une liste plate (pas de récursion :
 c'est justement le but, l'emplacement réel d'un fichier sur disque
 n'importe plus), réutilise `_draw_leaf` telle quelle. Volontairement
 découplé de `virtual_categories.py` (aucun import ici) -- c'est l'app hôte
 qui possède le scan/le cache/les règles d'exclusion (voir
 `apps/object_editor.py`'s `_scan_active_workspace_virtual_categories`,
 `virtual_categories.md`), `explorer.py` se contente de dessiner le dict
 déjà calculé, dans l'ordre où ses clés arrivent.
- `_draw_dir_contents` (`explorer.py`) : vue principale, plate,
 clic pour naviguer (pas d'expansion en place).
- `_bnp_entries` / `_bnp_visible_entries` (`explorer.py`) :
 listing (avec cache) et filtrage (avec cache séparé) du contenu d'un
 `.bnp`.
- `_draw_bnp_contents` (`explorer.py`) : liste le contenu d'un
 `.bnp` ouvert, avec `imgui.ListClipper` pour ne dessiner que les lignes
 visibles (archives à plusieurs milliers d'entrées).
- `_get_thumbnail_ref` / `_decode_thumbnail_worker`
 (`explorer.py`) : pipeline de vignettes de texture — décodage en
 arrière-plan (thread dédié par texture, jusqu'à
 `_MAX_CONCURRENT_THUMBNAIL_DECODES` en parallèle), upload GPU fait sur le
 thread principal une fois l'image prête.
- `_draw_leaf` (`explorer.py`) : dessine une ligne fichier (icône
 ou vignette, sélection, simple clic = première commande du menu
 contextuel (ex. charger un `.shape`, depuis 2026-09-01 — auparavant
 double-clic), clic droit = menu contextuel complet).
- `_select` / `_item_key` (`explorer.py`) : gestion de la
 sélection multiple, clé stable qui distingue un fichier réel d'une entrée
 `.bnp` (`f"{bnp_path}!{name}"`).

## Utilisation

- `ryzom_forgery/app.py` instancie `Explorer(self, Path(explorer_root),
 self.commands, default_filter=explorer_default_filter)` et branche
 `on_selection_changed`.
- `ryzom_forgery/apps/object_editor.py` importe `ExplorerItem` directement
 (pour construire/inspecter des items de sélection, ex. lors d'imports ou de
 drag&drop de fichiers).
- `ryzom_forgery/search_paths.py` importe seulement la constante
 `BNP_EXTENSIONS` de ce module (pas la classe `Explorer` elle-même) pour
 reconnaître les archives `.bnp` pendant un scan de dossiers.

## Points notables / pièges

- **Cycle d'imports assumé** : `shape_geometry.py -> search_paths.py ->
 explorer.py` (pour `BNP_EXTENSIONS`) existe déjà ; c'est pourquoi
 `_decode_thumbnail_worker` importe `load_panda_texture` localement plutôt
 qu'en tête de module (`explorer.py`), pour éviter de transformer ce
 cycle en import impossible.
- **Icônes FA4, pas FA5** : les glyphes `.skel`/`.anim` visés initialement
 (`ICON_FA_BONE`, `ICON_FA_WALKING`) n'existent pas dans la police FA 4.7
 réellement chargée par l'app — ils restent invisibles sans même un
 rectangle de repli. Le code utilise donc `ICON_FA_MALE`/`ICON_FA_FILM`
 (`explorer.py`). Toute nouvelle icône doit être vérifiée dans la
 police FA4 réellement embarquée avant utilisation.
- **Vignettes jamais invalidées** : `_thumbnail_tex_refs` n'est jamais vidé
 (contrairement à `_dir_cache`/`_bnp_cache` que `refresh` nettoie) — posé
 comme acceptable car un dossier `tex/` de workspace ne grossit pas vers les
 milliers d'entrées que peut atteindre un arbre de données Ryzom complet
 (`explorer.py`).
- **Décodage limité à 4 threads concurrents** et image réduite à 32×32 avant
 upload GPU (`_THUMBNAIL_DECODE_SIZE`, `_MAX_CONCURRENT_THUMBNAIL_DECODES`,
 `explorer.py`) — motivé par des textures sources jusqu'à 4096×4096
 ayant pris plus d'une seconde à décoder/uploader en pleine résolution pour
 une icône de 16px.
- **Le popup d'autocomplétion de chemin n'est pas une fenêtre overlay** : une
 vraie fenêtre flottante ImGui a été essayée et cassait la détection de clic
 (le focus quittait l'input au clic, ce qui empêchait le clic d'atteindre le
 `Selectable` dessous) — remplacée par un `begin_child` dans le flux normal
 du layout (`explorer.py`).
- **`.bnp` toujours plat** : une archive `.bnp` n'a pas de notion de
 sous-dossier ; `_draw_bnp_contents` s'appuie sur cette garantie pour
 utiliser `ListClipper` sans hiérarchie (`explorer.py`).
- **Pas de vignette pour un fichier à l'intérieur d'un `.bnp`** : `item.path`
 pointe alors vers l'archive, pas vers un chemin disque de l'entrée elle-même,
 donc `_draw_leaf` ne tente jamais de vignette dans ce cas
 (`explorer.py`).
- **Filtre + recherche n'affectent pas le Wexplorer** : ni
 `_draw_tree_children` ni `_draw_pinned_virtual_categories` ne tiennent
 compte de `self.search`/`self.extension_filter`, volontairement — c'est
 un panneau "tout parcourir", distinct de la vue principale filtrable
 (`explorer.py`).
- **`virtual_categories_source` est rappelé à chaque frame** : `draw()`
 l'invoque sans mémoriser le résultat lui-même -- si l'app hôte n'a pas sa
 propre mise en cache (voir `apps/object_editor.py`'s
 `_VIRTUAL_CATEGORIES_RESCAN_INTERVAL`), un scan récursif complet du
 workspace tournerait 60 fois/seconde, exactement le genre de coût que
 `_dir_cache` existe pour éviter côté vue principale.
