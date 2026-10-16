# import_dialog

**Fichier :** `nel/tools/forgery/ryzom_forgery/import_dialog.py` (~106 lignes)

## Rôle

UI ImGui du flux d'import `.obj`/`.dae`/`.fbx` → `.shape` dans Patina : ouvre un sélecteur de fichier natif pour choisir le mesh source, parse le fichier via `shape_import.py`, puis demande comment l'intégrer — comme un tout nouveau shape, ou en remplaçant la géométrie du shape actuellement ouvert tout en conservant ses matériaux (et les éventuelles modifications déjà faites dessus : blend, alpha-test, double face, Multi Bitmap...). C'est la couche UI au-dessus de `shape_import.find_importer`/`import_obj`/`import_dae`/`import_fbx`.

## API principale

- `ImportDialog.__init__(on_new_shape, on_replace)` (`import_dialog.py`) — reçoit deux callbacks `on_new_shape(mesh, source_path)` et `on_replace(mesh, source_path)`, appelés une fois le mode choisi dans le popup. `source_path` (le chemin du fichier importé) permet au flux "nouveau shape" de préremplir nom/emplacement, et aux deux flux de chercher les textures du mesh dans le dossier source.
- `ImportDialog.open(has_current_shape)` (`import_dialog.py`) — ouvre le sélecteur natif filtré sur `*.obj *.dae *.fbx`. `has_current_shape` détermine si l'option "Replace in current shape" est proposée.
- `ImportDialog.draw` (`import_dialog.py`) — à appeler chaque frame ImGui.
- `_poll_file_dialog` (`import_dialog.py`) — récupère le résultat du sélecteur une fois prêt ; résout l'importeur via `find_importer(path)` (statut d'erreur si extension non supportée) ; appelle l'importeur, capture `OSError`/`ShapeImportError` en cas d'échec de parsing ; sinon ouvre le popup de mode.
- `_draw_mode_popup` (`import_dialog.py`) — popup modal ImGui : affiche le nombre de matériaux du mesh importé, propose "Import as new shape", "Replace in current shape" (désactivé si pas de shape ouvert), un bouton désactivé "Add to current shape (coming soon)", et "Cancel".

## Utilisation

- `apps/object_editor.py` importe `ImportDialog` ; instancié en `object_editor.py` avec `on_new_shape=self._on_import_new_shape, on_replace=self._on_import_replace`.
- `object_editor.py` appelle `self.import_dialog.open(self.shape_file is not None)` (bouton Import dans l'UI).
- `object_editor.py` appelle `self.import_dialog.draw` dans la boucle de frame.
- Dépend directement de `shape_import.py` (`ShapeImportError, find_importer` — voir `shape_import.md`) ; c'est ce module qui fait le vrai travail de parsing/construction du `Mesh` pynel, `import_dialog.py` n'est qu'orchestration UI.

## Points notables / pièges

- "Add to current shape" est un bouton visiblement désactivé avec le libellé "(coming soon)" (`import_dialog.py`) — fonctionnalité non implémentée, affichée comme placeholder délibéré plutôt que masquée.
- Le parsing du mesh source a lieu **avant** l'ouverture du popup de mode (`_poll_file_dialog`, `import_dialog.py`), pas après le choix "new shape" vs "replace" — donc un fichier volumineux ou invalide échoue/bloque dès la sélection du fichier, indépendamment du mode choisi ensuite.
- Aucun format autre que `.obj`/`.dae`/`.fbx` n'est proposé dans le filtre du sélecteur de fichier (`import_dialog.py`), cohérent avec `shape_import.IMPORTERS` qui ne connaît que ces trois extensions.
- Pas de TODO explicite dans le fichier (le "coming soon" du bouton Add en tient lieu implicitement).
