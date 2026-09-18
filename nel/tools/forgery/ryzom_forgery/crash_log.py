"""Logs any uncaught exception -- main thread or background thread -- to
config_dir()/"crash.log" before letting it propagate normally. A frozen
Windows build runs with no console attached, so an unhandled exception today
just silently kills the process with nothing left for a user to report (bug
found 2026-09-18: a user reported Patina crashing on a specific click, no
traceback recoverable at all). install() must be called once, as early as
possible in each app's own main() (see object_editor.py/landscape_editor.py).
"""

import sys
import threading
import traceback
from datetime import datetime

from .config_dir import config_dir

_LOG_FILE_NAME = "crash.log"


def install():
	sys.excepthook = _log_and_reraise
	threading.excepthook = _log_and_reraise_thread


def _write_log(exc_type, exc_value, exc_tb):
	"""Best-effort only -- a failure to write the log must never mask or
	replace the real exception, which _log_and_reraise()/
	_log_and_reraise_thread() always forward to the default hook regardless."""
	text = "".join(traceback.format_exception(exc_type, exc_value, exc_tb))
	try:
		directory = config_dir()
		directory.mkdir(parents=True, exist_ok=True)
		with (directory / _LOG_FILE_NAME).open("a", encoding="utf-8") as f:
			f.write(f"\n----- {datetime.now().isoformat(timespec='seconds')} -----\n{text}")
	except OSError:
		pass


def _log_and_reraise(exc_type, exc_value, exc_tb):
	_write_log(exc_type, exc_value, exc_tb)
	sys.__excepthook__(exc_type, exc_value, exc_tb)


def _log_and_reraise_thread(args):
	_write_log(args.exc_type, args.exc_value, args.exc_traceback)
	threading.__excepthook__(args)
