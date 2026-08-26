import subprocess
import sys
import time


class Splash:
	"""A borderless splash window shown while a ForgeryApp is starting up.

	Runs in a completely separate OS process (see _splash_process.py),
	not a thread in this one -- Tk/Tcl isn't thread-safe (this used to run
	as a background thread here instead, which crashed
	("Tcl_AsyncDelete: async handler deleted by the wrong thread") once
	this app started spawning more of its own background threads, e.g. the
	workspace filesystem watcher, racing against the splash's own Tcl
	teardown), and on macOS a GUI toolkit is additionally restricted to a
	process's own *main* thread outright, which a background thread could
	never satisfy regardless. A separate process sidesteps both
	constraints entirely: this process's own threads can never interact
	with whatever's going on inside the splash's own Tcl interpreter,
	because there's no way for them to -- it's a different process.
	"""

	def __init__(self, image_path, min_duration=1.2, subsample=1, center_on=None):
		self._min_duration = min_duration
		self._start = time.monotonic()
		args = [sys.executable, "-m", "ryzom_forgery._splash_process", str(image_path), str(subsample)]
		if center_on is not None:
			args += [str(value) for value in center_on]
		self._process = subprocess.Popen(args)

	def close(self):
		"""Blocks until min_duration has elapsed since construction, then
		terminates the splash process."""
		remaining = self._min_duration - (time.monotonic() - self._start)
		if remaining > 0:
			time.sleep(remaining)
		self._process.terminate()
		self._process.wait()
