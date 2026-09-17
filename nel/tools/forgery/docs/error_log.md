# error_log.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/error_log.py`

## Rôle

Chaque app/dialog Forgery garde son propre champ `self._x_error = "..."`,
affiché seulement via `imgui.text_colored()` dans le tab/panel où il vit --
facile à manquer si ce panel n'est pas visible au moment de l'erreur.
`report_error()` est appelée à côté de chaque telle assignation (voir
`project-todos/forgery/error_stderr_logging.md`) pour que le même message
atteigne aussi systématiquement le terminal.

## API principale

- `report_error(message: str)` -- écrit `message` sur stderr
  (`print(..., file=sys.stderr)`). Volontairement minimal : pas de module
  `logging`, pas de préfixe timestamp/nom d'app.

## Utilisation

Appelée en parallèle de chaque assignation `self._x_error = "..."` dans les
apps/dialogues Forgery, jamais à la place de cette assignation.
