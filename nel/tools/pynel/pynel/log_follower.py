#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Tail a log file like `tail -F`, surviving rotation, truncation and delete/recreate.

Handles the two logrotate strategies:
  - copytruncate: same inode, file truncated back to 0 -- detected by the
    on-disk size dropping below the current read position, reopens from
    the start of the (same) file.
  - create/rename (default): a new file appears at the same path with a
    different inode -- detected via stat, reopens the new file from the
    start.

Also survives the file being briefly missing during rotation (waits for it
to reappear instead of raising).

Usage:
	from pynel.log_follower import LogFollower
	for line in LogFollower("/var/log/foo.log"):
		if "ERROR" in line:
			...
"""

import os
import time

try:
	import pyinotify
	_HAVE_INOTIFY = True
except ImportError:
	_HAVE_INOTIFY = False


class LogFollower(object):
	"""Iterable yielding new lines appended to `path`, like `tail -F`.

	Uses inotify to wake up as soon as new data is available when pyinotify
	is installed, otherwise falls back to polling every `poll_interval`
	seconds.
	"""

	_INOTIFY_MASK = None
	if _HAVE_INOTIFY:
		_INOTIFY_MASK = (pyinotify.IN_MODIFY | pyinotify.IN_MOVE_SELF |
			pyinotify.IN_ATTRIB | pyinotify.IN_DELETE_SELF)

	def __init__(self, path, from_start=False, poll_interval=0.05, wait_interval=0.2):
		self.path = path
		self.from_start = from_start
		self.poll_interval = poll_interval
		self.wait_interval = wait_interval
		self._fp = None
		self._st = None

	def _wait_for_file(self):
		"""Block until the file exists and is openable (e.g. right after rotation)."""
		while True:
			if os.path.isfile(self.path):
				try:
					with open(self.path, "rb"):
						return
				except OSError:
					pass
			time.sleep(self.wait_interval)

	def _open(self, from_start):
		self._wait_for_file()
		self._fp = open(self.path, "r", encoding="utf-8", errors="replace", buffering=1)
		self._st = os.fstat(self._fp.fileno())
		self._fp.seek(0, os.SEEK_SET if from_start else os.SEEK_END)

	@staticmethod
	def _same_file(st_a, st_b):
		return st_a.st_ino == st_b.st_ino and st_a.st_dev == st_b.st_dev

	def _need_reopen(self):
		"""Return (reopen: bool, cur_stat: os.stat_result or None)."""
		try:
			cur = os.stat(self.path)
		except FileNotFoundError:
			return True, None
		if not self._same_file(self._st, cur):
			return True, cur
		try:
			pos = self._fp.tell()
		except (OSError, ValueError):
			pos = 0
		if cur.st_size < pos:
			return True, cur
		return False, cur

	def _setup_inotify(self):
		if not _HAVE_INOTIFY:
			return None, None
		try:
			wm = pyinotify.WatchManager()
			class _Handler(pyinotify.ProcessEvent):
				def process_default(self, event):
					pass  # only used to wake up the poll loop faster
			notifier = pyinotify.Notifier(wm, _Handler())
			wm.add_watch(self.path, self._INOTIFY_MASK, quiet=True)
			return wm, notifier
		except Exception:
			return None, None

	def __iter__(self):
		self._open(self.from_start)
		wm, notifier = self._setup_inotify()
		try:
			while True:
				line = self._fp.readline()
				if line:
					yield line
					continue

				reopen, cur = self._need_reopen()
				if reopen:
					try:
						self._fp.close()
					except OSError:
						pass
					self._open(from_start=True)
					if wm is not None:
						try:
							wm.rm_watch(list(wm.watches.values()))
							wm.add_watch(self.path, self._INOTIFY_MASK, quiet=True)
						except Exception:
							pass
					continue

				if notifier:
					notifier.process_events()
					time.sleep(self.poll_interval)
					if notifier.check_events(timeout=int(self.poll_interval * 1000)):
						notifier.read_events()
				else:
					time.sleep(self.poll_interval)
		finally:
			try:
				self._fp.close()
			except OSError:
				pass
			if notifier:
				try:
					notifier.stop()
				except Exception:
					pass
