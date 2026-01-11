#############################################
#   __   __  _______  ______    _______  __   __  _______  _______
#  |  |_|  ||   _   ||    _ |  |       ||  | |  ||       ||       |
#  |       ||  |_|  ||   | ||  |    ___||  | |  ||    ___||____   |
#  |       ||       ||   |_||_ |   | __ |  |_|  ||   |___  ____|  |
#  |       ||       ||    __  ||   ||  ||       ||    ___|| ______|
#  | ||_|| ||   _   ||   |  | ||   |_| ||       ||   |___ | |_____
#  |_|   |_||__| |__||___|  |_||_______||_______||_______||_______|
#
# M.A.R.G.U.E.Z (The Dyslexic Merguez)
# Copyright (C) 2025 Nuneo (ulukyn@gmail.com)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# Logging tools
#

import io
import os
import sys
import logging
import builtins

from datetime import datetime
from textual.markup import escape
from rich.text import Text
from inspect import currentframe, getframeinfo

class MarguezPrint():

	def __init__(self):
		self.print_buffer = io.StringIO()
		self.builtins_print = builtins.print
		self.out_file = "."+sys.argv[0].split("/")[-1].split(".")[0]+".out"
		self.err_file = "."+sys.argv[0].split("/")[-1].split(".")[0]+".err"
		self.stdout = None
		self.stderr = None
		self.last_out_file = ""

	def setStdOut(self):
		if self.stdout:
			self.stdout.close()
			if os.path.isfile(self.last_out_file):
				os.remove(self.last_out_file)
		if os.path.isfile(self.out_file):
			os.remove(self.out_file)
		self.stdout = open(self.out_file, "a")
		self.last_out_file = self.out_file

	def setStdErr(self):
		if self.stderr:
			self.stderr.close()
			os.remove(self.last_err_file)
		if os.path.isfile(self.err_file):
			self.last_run_errors = open(self.err_file, "r").read()
			os.remove(self.err_file)
		sys.stderr = open(self.err_file, "a")
		self.last_err_file = self.err_file

	def print(self, *args, **kwargs):
		x = datetime.now()
		time = x.strftime("%Y-%m-%d %H:%M:%S")
		cf = currentframe()
		cf = cf.f_back
		filename = getframeinfo(cf).filename.split("/")[-1]

		line = cf.f_lineno
		try:
			self.builtins_print(*args, file=self.print_buffer, **kwargs)
		except:
			return

		out = self.print_buffer.getvalue()
		log = f"[bright_yellow]{time}[/bright_yellow] [bright_cyan]{filename}:{line}[/bright_cyan] {out}"
		self.builtins_print(log)
		self.stdout.write(log)
		self.stdout.flush()
		sys.stdout.flush()
		self.print_buffer.seek(0)
		self.print_buffer.truncate(0)



logging.getLogger("urllib3").propagate = False
logging.getLogger("asyncio").propagate = False

printer = MarguezPrint()
printer.setStdOut()
builtins.print = printer.print

