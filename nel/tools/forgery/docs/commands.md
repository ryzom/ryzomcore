# commands.py — registre de commandes de l'explorateur

**Fichier :** `nel/tools/forgery/ryzom_forgery/commands.py` (40 lignes)

## Rôle

Ce module définit un petit registre d'actions ("commandes") que le menu
contextuel de l'explorateur de fichiers standard (`Explorer`, dans
`explorer.py`) peut proposer sur une sélection de fichiers. Il distingue
deux catégories : des commandes globales, proposées quelle que soit la
sélection, et des commandes par extension, proposées seulement quand tous
les fichiers sélectionnés partagent la même (unique) extension. C'est le
mécanisme générique par lequel chaque app Forgery ajoute ses propres
actions (ex. "Charger dans le viewer" pour un `.shape`) au clic droit dans
l'explorateur, sans que `explorer.py` ait besoin de connaître ces actions
à l'avance.

## API principale

- `Command` (`commands.py`) : `@dataclass` simple, `label: str` +
 `callback: Callable[[list], None]` — une entrée de menu et la fonction
 appelée avec la liste des items sélectionnés.
- `CommandRegistry` (`commands.py`) : le registre lui-même, tenant deux
 structures internes : `_global_commands: list[Command]` et
 `_commands_by_extension: dict[str, list[Command]]`.
  - `register_global(self, label, callback)` (`commands.py`) : ajoute
 une commande toujours proposée.
  - `register_for_extension(self, extension, label, callback)`
 (`commands.py`) : ajoute une commande propre à une extension
 (normalisée en minuscules, `commands.py`).
  - `commands_for_selection(self, items: list) -> list[Command]`
 (`commands.py`) : construit la liste effective des commandes pour
 une sélection donnée — toutes les commandes globales, plus les
 commandes de l'extension commune si (et seulement si) tous les items
 sélectionnés partagent exactement une seule extension
 (`commands.py`, via un `set` d'extensions).

## Utilisation

- `ForgeryApp.__init__` (`app.py`) instancie un `CommandRegistry` par
 app (`self.commands = CommandRegistry`) et le passe à `Explorer`
 (`app.py`).
- `Explorer` (`explorer.py, 91`) reçoit ce registre en paramètre de
 construction et l'utilise pour peupler son menu contextuel :
 `commands_for_selection([item])` pour le menu au survol d'un item
 (`explorer.py`) et `commands_for_selection(items)` pour le menu sur
 une sélection multiple (`explorer.py`).
- `ObjectEditorApp` (`apps/object_editor.py`) enregistre des
 commandes concrètes sur `self.commands`, notamment
 `register_for_extension(".shape", "Load in viewer", self._on_load_command)`.
- `examples/smoke_test.py` montre l'usage minimal d'une commande
 globale : `self.commands.register_global("Print path", lambda items: print([str(i.path) for i in items]))`.

## Points notables / pièges

- Les items de sélection n'ont besoin que d'un attribut `.suffix` — le
 docstring (`commands.py`) précise explicitement que ça marche aussi
 bien avec un `pathlib.Path` qu'avec un `ryzom_forgery.explorer.ExplorerItem`,
 donc le registre est volontairement découplé du type concret utilisé par
 l'Explorer.
- La règle "extension commune" est stricte : `len(extensions) == 1`
 (`commands.py`) — dès que deux fichiers d'extensions différentes sont
 sélectionnés ensemble, aucune commande par-extension n'est proposée,
 seulement les commandes globales. Un item sans extension (`.suffix` vide/
 falsy) est simplement exclu du set via le filtre `if item.suffix`
 (`commands.py`), donc il ne bloque pas la détection d'une extension
 commune parmi les autres items.
- Pas de désenregistrement (`unregister`) ni de vérification de doublons —
 le registre est purement additif ; appeler `register_global`/
 `register_for_extension` deux fois avec le même label ajoute deux entrées
 distinctes.
