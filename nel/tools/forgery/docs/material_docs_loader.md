# material_docs.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/material_docs.py` (~47 lignes)

## Rôle

Extrait, depuis `docs/material_options.md` (voir ce fichier pour le
contenu réel des explications), le texte de tooltip associé à chaque
réglage de matériau, afin que les UI d'outils (l'éditeur de matériau
d'`object_editor.py`) puissent afficher les mêmes explications
"orientées joueur" en info-bulle ImGui, sans dupliquer ce texte dans le
code (`material_docs.py`). Ce module s'appuie sur la convention
documentée dans `material_options.md` lui-même : chaque option y est une
section `## Titre {#cle-stable}`, avec une ligne `**Résumé :**` juste
après l'en-tête contenant le texte court destiné à la tooltip
(`material_docs.py`, voir aussi `material_options.md`). Ce
module n'a aucun rapport direct avec le pipeline panoply — c'est un
loader de documentation générique pour l'éditeur de matériau.

## API principale

- `DOC_PATH` — `material_docs.py` : chemin par défaut vers `docs/material_options.md`, résolu relativement à ce module (`.parent.parent / "docs" / "material_options.md"`).
- `_HEADER_RE` — `material_docs.py` : regex capturant `title`/`key` d'un en-tête `## Titre {#cle}` ou `### Titre {#cle}`.
- `_SUMMARY_RE` — `material_docs.py` : regex capturant le contenu de la ligne `**Résumé :**` (multi-lignes jusqu'au premier paragraphe vide ou fin de texte).
- `MaterialDoc` (NamedTuple) — `material_docs.py` : `title`, `summary` (texte court, un paragraphe), `full_text` (corps complet de la section, résumé inclus).
- `load_material_docs(path=DOC_PATH)` — `material_docs.py` : lit le fichier, découpe le texte sur chaque en-tête `## ... {#...}` trouvé, extrait `summary` (normalisé : espaces multiples/retours à la ligne collapsés en un seul espace, `material_docs.py`) et construit `{key: MaterialDoc}`. Renvoie un dict vide si le fichier est manquant/illisible (`OSError`), pour permettre à l'UI de dégrader sans tooltip plutôt que de planter.

## Utilisation

Consommé uniquement par `object_editor.py` (import `material_docs.py`
via `from ryzom_forgery.material_docs import load_material_docs`) :

- Chargé une fois à l'initialisation de l'éditeur : `self.material_docs = load_material_docs` (`object_editor.py`).
- Consulté ponctuellement par clé stable pour afficher une tooltip au survol d'un réglage : ex. `self.material_docs.get("multi-bitmap")` (`object_editor.py`) et `doc = self.material_docs.get(key)` dans une fonction générique de tooltip (`object_editor.py`).

Ce module n'interagit avec aucun des 5 autres modules panoply — il partage
seulement l'appartenance au même dossier `ryzom_forgery/` et le même
consommateur final (`object_editor.py`).

## Points notables / pièges

- Le parsing dépend entièrement du format exact `## Titre {#cle}` (regex ancrée sur ce motif précis, `material_docs.py`) — un en-tête mal formé dans `material_options.md` (accolade manquante, clé avec majuscule ou caractère hors `[a-z0-9-]`) serait silencieusement ignoré plutôt que de lever une erreur.
- `_SUMMARY_RE` s'arrête au premier `\n\n` (paragraphe vide) ou à la fin du texte (`material_docs.py`, `DOTALL`) — un résumé rédigé sur plusieurs paragraphes séparés par une ligne vide ne serait pas capturé en entier, seul le premier paragraphe le serait.
- Aucune validation qu'une section a effectivement un `**Résumé :**` — si absent, `summary` reste une chaîne vide (`material_docs.py`, branche `else ""`), et l'entrée est quand même ajoutée au dict avec un résumé vide.
- Le fichier est relu entièrement et re-parsé à chaque appel de `load_material_docs` — pas de cache module-level comme `panoply_config.py`'s `_DOC` ; dans `object_editor.py`, l'appel unique à l'init suffit à éviter un re-parsing répété.
