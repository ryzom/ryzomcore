# Properties (inspecteur générique en lecture seule)

**Fichier :** `nel/tools/forgery/ryzom_forgery/properties.py` (~64 lignes)

## Rôle

Module utilitaire minuscule et générique : dessine un arbre ImGui en lecture
seule pour n'importe quelle instance de dataclass, en dépliant récursivement
les dataclasses imbriquées comme des noeuds d'arbre. Sert de vue "toutes les
propriétés" brute pour inspecter n'importe quelle structure parsée par
`pynel` (un `.shape`, un matériau, etc.) sans avoir à écrire une UI dédiée
par type — utilisé par l'onglet "All Properties" de Patina
(`properties.py`).

## API principale

- `draw_properties(value, max_items=DEFAULT_MAX_ITEMS)`
 (`properties.py`) : point d'entrée. Si `value` n'est pas une
 dataclass, affiche juste `str(value)` (`properties.py`) ; sinon
 itère `dataclasses.fields(value)` et délègue à `_draw_field` pour chaque
 champ.
- `_draw_field(name, value, max_items)` (`properties.py`) : dispatch
 par type — `bytes` affichés comme `<N bytes>`, `dict` comme
 `{N entries}`, `list`/`tuple` délégués à `_draw_list_field`, dataclass
 imbriquée dessinée comme un noeud d'arbre expansible (récursion dans
 `draw_properties`), tout le reste affiché comme `name: valeur formatée`.
- `_draw_list_field(name, items, max_items)` (`properties.py`) : si la
 liste dépasse `max_items` éléments OU ne contient aucune dataclass, elle
 est réduite à `[N items]` (pas de dump) ; sinon dessinée comme un noeud
 d'arbre avec un sous-élément par entrée (dataclass imbriquée récursée,
 scalaire affiché directement).
- `_format_scalar(value)` (`properties.py`) : formate un `float` avec
 4 décimales, sinon `str(value)`.
- `DEFAULT_MAX_ITEMS = 8` (`properties.py`).

## Utilisation

- `ryzom_forgery/apps/object_editor.py` importe `draw_properties`, appelé
 ligne 3831 comme `draw_properties(self.shape_file.value)` dans l'onglet
 "All Properties" du panneau de droite — l'objet parsé complet du `.shape`
 courant (`self.shape_file.value`, une structure `pynel`) y est affiché
 brut, sans mise en forme spécifique.

## Points notables / pièges

- **Générique par construction, sans liste d'exclusion par type** : le
 docstring de `draw_properties` précise explicitement que le seuil
 `max_items` sur les listes/dicts/bytes sert à éviter de dumper de gros
 tableaux (ex. données brutes de vertex/LOD) tout en restant "reuse[able] on
 any pynel-parsed structure without per-type exclusion lists"
 (`properties.py`) — c'est un choix de design délibéré : aucune
 connaissance du schéma `pynel` n'est codée en dur ici.
- **Lecture seule uniquement** : ce module ne fournit aucune édition, juste
 de l'affichage — cohérent avec son rôle d'inspecteur brut de secours,
 distinct des onglets Textures/Materials qui eux éditent réellement les
 champs.
- **Une liste de dataclasses au-delà de `max_items` reste réduite au
 compteur**, même si elle contient des dataclasses intéressantes à
 inspecter individuellement — pas d'option pour forcer l'affichage complet
 d'une grande liste depuis cette fonction.
