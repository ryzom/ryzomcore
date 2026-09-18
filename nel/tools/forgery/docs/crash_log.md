# crash_log.py

**Fichier :** `nel/tools/forgery/ryzom_forgery/crash_log.py`

## Rôle

Écrit toute exception non catchée -- thread principal ou thread de fond --
dans `config_dir()/"crash.log"` avant de la laisser remonter normalement
(comportement console inchangé). Un build Windows sans console attachée ne
laisse sinon aucune trace exploitable d'un crash.

## API principale

- `install()` -- fixe `sys.excepthook` et `threading.excepthook`. Doit être
  appelée une seule fois, le plus tôt possible dans `main()`.

## Utilisation

Appelée en tout premier dans `main()` de chaque app (`object_editor.py`,
`landscape_editor.py`), avant l'instanciation de l'app elle-même. L'écriture
du fichier est best-effort : une erreur d'écriture (`OSError`) ne masque
jamais l'exception réelle, qui continue de remonter au hook par défaut.
