"""Standalone splash-screen process for Ryzom Forgery tool apps.

Launched via subprocess by splash.py's Splash class, which just
terminates it once the real app is ready -- shows a borderless,
always-on-top image and keeps repainting via its own Tk mainloop until
killed. No IPC of any kind: min_duration/timing is entirely the parent's
concern (see Splash.close()), this process only ever needs to know how to
show itself and get out of the way.

Tk/Tcl is not thread-safe, and on macOS a GUI toolkit is additionally
restricted to a process's own *main* thread -- neither constraint is a
problem for a whole separate process whose only job is this window, unlike
running Tk on a background thread inside the main Forgery process (which
used to crash there, racing against that process's own other threads --
e.g. the workspace filesystem watcher/search path reload workers).

Usage: python -m ryzom_forgery._splash_process IMAGE_PATH SUBSAMPLE [TARGET_X TARGET_Y TARGET_WIDTH TARGET_HEIGHT]
"""

import sys
import tkinter as tk


def main(argv=None):
	argv = sys.argv[1:] if argv is None else argv
	image_path = argv[0]
	subsample = int(argv[1])
	center_on = tuple(int(value) for value in argv[2:6]) if len(argv) >= 6 else None

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
	label.image = image  # keep a reference -- PhotoImage has no other owner otherwise
	label.pack()

	# Self-destruct safety net: normally Splash.close() (in the parent
	# process) terminates this process well before this fires. But if the
	# parent dies unexpectedly (crash, kill -9...) without ever calling
	# close(), there's nothing else to stop this window from lingering
	# forever -- an orphaned splash for every failed launch, piling up
	# indefinitely. 5s comfortably covers a normal startup (min_duration
	# above it is only 1.2s) while still bounding the damage from a dead
	# parent to a single lingering window, never an accumulation.
	root.after(5000, root.destroy)

	root.mainloop()


if __name__ == "__main__":
	main()
