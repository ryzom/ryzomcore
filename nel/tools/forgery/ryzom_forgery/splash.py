import threading
import time
import tkinter as tk


class Splash:
	"""A borderless splash window shown while a ForgeryApp is starting up.

	Runs its own Tk mainloop on a dedicated thread so it keeps painting
	while the caller's thread is busy doing the actual (blocking) Panda3D
	startup work -- a plain PhotoImage shown once with no mainloop would
	never repaint (e.g. after being covered by another window) and could
	be flagged "not responding" by the OS.
	"""

	def __init__(self, image_path, min_duration=1.2, subsample=1, center_on=None):
		self._min_duration = min_duration
		self._start = time.monotonic()
		self._done = threading.Event()
		ready = threading.Event()
		self._thread = threading.Thread(target=self._run, args=(str(image_path), subsample, center_on, ready), daemon=True)
		self._thread.start()
		ready.wait()

	def _run(self, image_path, subsample, center_on, ready):
		root = tk.Tk()
		root.overrideredirect(True)
		root.attributes("-topmost", True)

		image = tk.PhotoImage(file=image_path)
		if subsample > 1:
			image = image.subsample(subsample, subsample)
		width, height = image.width(), image.height()
		if center_on is not None:
			target_x, target_y, target_width, target_height = center_on
			pos_x = target_x + (target_width - width) // 2
			pos_y = target_y + (target_height - height) // 2
		else:
			screen_width, screen_height = root.winfo_screenwidth(), root.winfo_screenheight()
			pos_x, pos_y = (screen_width - width) // 2, (screen_height - height) // 2
		root.geometry(f"{width}x{height}+{pos_x}+{pos_y}")

		label = tk.Label(root, image=image, borderwidth=0)
		label.image = image
		label.pack()

		def poll_done():
			if self._done.is_set():
				root.destroy()
				return
			root.after(50, poll_done)

		root.after(50, poll_done)
		ready.set()
		root.mainloop()

	def close(self):
		"""Blocks until min_duration has elapsed since construction, then
		tears down the splash window."""
		remaining = self._min_duration - (time.monotonic() - self._start)
		if remaining > 0:
			time.sleep(remaining)
		self._done.set()
		self._thread.join(timeout=2.0)
