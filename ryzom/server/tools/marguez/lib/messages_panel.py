#!/bin/env python3
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
# Info Panel using 3 informations text and the ActionBar
#

import os
import re
import time
import importlib
import subprocess

from pymemcache.client.base import Client
from pymemcache import serde
from rich.text import Text
from rich.pretty import Pretty, pretty_repr
from rich.markup import escape, MarkupError
from textual import work
from textual.reactive import reactive
from textual.containers import Horizontal, VerticalScroll, Vertical
from textual.widgets import Button, Input, Label, Rule, TabbedContent, TabPane, RichLog, ProgressBar


class MessagesTabs(TabbedContent):
	
	def on_mount(self):
		self.app.fill_logs = True


class MessagesPanel(Vertical):
	logs_sections = []
	selected_section = 0
	is_running = False
	fetchers = {}
	last_fetcher_id = 0
	logs_fetchers = []
	logs_infos = []
	dagu_infos = {}
	procs = {}
	logs = {}
	total_errors = 0
	
	def compose(self):
		with MessagesTabs():
			for i in range(10):
				with TabPane(f"Tab{i}", id=f"tab_{i}"):
					yield RichLog(id=f"log_{i}", classes="messages", auto_scroll=False, markup=True)
		yield Horizontal(
			Input(id="input_commands"),
			ProgressBar(total=100, show_eta=False),
			Label("0")
		)

	def setLogTabs(self, logs_infos):
		self.stopFillingLogs()
		self.logs_infos = logs_infos
		m = self.query_one(MessagesTabs)
		for i in range(10):
			t = m.get_tab(f"tab_{i}")
			if i < len(logs_infos):
				t.label = logs_infos[i][0]
				t.styles.display = "block"
			else:
				t.styles.display = "none"
		self.startFillingLogs()

	def on_tabbed_content_tab_activated(self, event):
		tid = int(event.tab.id.split("_")[-1])
		if tid >= len(self.logs_infos):
			return
		self.selected_section = tid
		print(tid)
		print(self.logs_infos[tid][0])
		if self.logs_infos[tid][0] == "Errors":
			self.total_errors = 0
			tab = self.query_one(MessagesTabs).get_tab(f"tab_{tid}")
			tab.styles.color = "white"
			tab.label = f"Errors"
		if len(self.logs_infos) > tid and len(self.logs_infos[tid]) > 2:
			if "search_regex" in self.logs_infos[tid][2]:
				self.query_one("#input_commands").placeholder = "/"+self.logs_infos[tid][2]["search_regex"]
				return
		self.query_one("#input_commands").placeholder = "Enter a command..."

	def write(self, widget, text, scroll_end=True):
		if not widget:
			return False
		try:
			widget.write(text, scroll_end=scroll_end)
		except:
			return False
		return True
		
	def search(self, regex):
		print("STOP fetchers", self.logs_fetchers[self.selected_section])
		self.logs_infos[self.selected_section][2]["search_regex"] = regex
		self.fetchers[self.logs_fetchers[self.selected_section]] = False

	def stopFillingLogs(self):
		print("STOP LOGS")
		self.is_running = False
		k_fetchers = self.fetchers.keys()
		for i in k_fetchers:
			self.fetchers[i] = False
		while self.fetchers:
			time.sleep(0.001)
		print("STOPPED")

	def startFillingLogs(self):
		print("START LOGS")
		self.is_running = True
		self.total_errors = 0
		self.logs_fetchers = [None] * 10
		print("TOTAL:", len(self.logs_infos))
		for i in range(len(self.logs_infos)):
			self.last_fetcher_id += 1
			self.fillSectionLogs(self.last_fetcher_id, i)


	def parseLine(self, is_rich, line, progress, pattern, final, subst, search):
		line = line.strip()
		if not line:
			return line
		
		if line[0] == "[" \
			and line[4] == "%" \
			and line[5] == "]":
				progress.update(progress=int(line[1:4]))
				line = line[6:]
		if is_rich:
			try:
				line = Text.from_markup(line).markup
			except MarkupError:
				line = escape(line)
		else:
			line = escape(line)
		if search:
			if not search.search(line):
				return None
		if pattern:
			line = pattern.sub(final, line)
		if subst:
			for k,v in subst.items():
				line = line.replace(k, v)
		return line

	def updateErrorsTab(self, tab):
		self.total_errors += 1
		tab.styles.color = "pink"
		tab.label = f"Errors({self.total_errors})"

	@work(thread=True)
	def fillSectionLogs(self, fetcher_id, section):
		try:
			w = self.query_one(f"#log_{section}")
			tab = self.query_one(MessagesTabs).get_tab(f"tab_{section}")
		except:
			print("Abort...")
			return
		progress = self.query_one(ProgressBar)
		label = self.query_one(Label)
		nbr_fetchers = len(self.fetchers.keys())
		while self.is_running:
			try:
				w.clear()
			except:
				time.sleep(1)
				break
			
			infos = self.logs_infos[section]
			total_lines = 0
			if nbr_fetchers != len(self.fetchers.keys()):
				nbr_fetchers = len(self.fetchers.keys())
				label.update(str(nbr_fetchers))
			if infos:
				print("Fill Section", infos, "fetcher:", fetcher_id)
				self.fetchers[fetcher_id] = True
				self.logs_fetchers[section] = fetcher_id
			
				if infos[1] in ("rich", "text") :
					log_filename = infos[2]["file"]
					if os.path.isfile(log_filename):
						#p = subprocess.run(["tail", "-n", "1000", log_filename], capture_output=True, text=True)
						with open(log_filename, "r", encoding="utf-8", errors="replace") as log_file:
							pattern = None
							final = ""
							subst = {}
							search = None
							if "format_regex" in infos[2]:
								pattern = re.compile(infos[2]["format_regex"])
							if "format_final" in infos[2]:
								final = infos[2]["format_final"]
							if "format_subst" in infos[2]:
								subst =  infos[2]["format_subst"]
							if "search_regex" in infos[2]:
								search = re.compile(infos[2]["search_regex"])
							firstLine = True

							if not self.write(w, "[orange1]FILE: "+os.path.basename(log_filename)+"[/orange1]", False):
								break
							out = []
							lines = log_file.read().split("\n")
							self.write(w, log_file.read())
							lines.reverse()
							nbr_lines = 0
							for line in lines:
								line = self.parseLine(infos[1] == "rich", line, progress, pattern, final, subst, search)
								if line:
									nbr_lines += 1
									if nbr_lines > 1000:
										break
									if infos[0] == "Errors" and not line in ("Terminated"):
										self.updateErrorsTab(tab)
									out.insert(0, line)
							self.write(w, "\n".join(out))
							
							log_file.seek(0, os.SEEK_END)
							file_size = os.stat(log_filename).st_size
							while self.fetchers[fetcher_id]:
								line = log_file.readline()
								if line:
									line = self.parseLine(infos[1] == "rich", line, progress, pattern, final, subst, search)
									if line:
										if infos[0] == "Errors" and not line in ("Terminated"):
											self.updateErrorsTab(tab)
										if not self.write(w, line):
											break
								elif self.fetchers[fetcher_id]:
									if not os.path.isfile(log_filename):
										time.sleep(1)
										break
									size = os.stat(log_filename).st_size
									if file_size > size:
										break
									file_size = size
									time.sleep(0.001)
						print("End", infos[0], "logs")
					else:
						print("File not found")
						time.sleep(1)
						break
				elif infos[1] == "dagu":
					done = False
					while self.fetchers[fetcher_id]:
						if not done:
							w.clear()
							log_file = infos[2]["raw"]["file"]
							if os.path.isfile(log_file):
								self.write(w, "[orange1]Infos:[/]")
								self.write(w, Pretty(infos[2]))
								self.write(w, " ")
								self.write(w, "[orange1]Config: "+os.path.basename(log_file)+"[/orange1]")
								with open(log_file, "r") as f:
									self.write(w, escape(f.read()))
							w.write("[orange1]Processes:[/]")
							for p in infos[2]["raw"]["procs"]:
								sp = p.split(" ", 1)
								self.write(w, "[cyan]"+sp[0]+"[/cyan] "+sp[1])
							self.write(w, " ")
							done = True
						time.sleep(0.1)
					print("End", infos[0], "logs")
				elif infos[1] == "db":
					current_key = 0
					client = Client("localhost", serde=serde.pickle_serde)
					while self.fetchers[fetcher_id]:
						key = int(client.get(infos[2][0]))
						if current_key != key:
							for i in range(max(current_key+1, key-100), key+1):
								try:
									value = client.get(infos[2][1].format(i))
								except:
									value = "error"
								if value:
									if len(infos[2]) > 3:
										module = importlib.import_module(infos[2][2])
										format_class = getattr(module, infos[2][3])
										fc = format_class()
										fc.set(value)
										value = fc.pprint()
									try:
										self.write(w, f"[bright_yellow]{i}[/bright_yellow] {value}")
									except:
										pass
							current_key = key
						time.sleep(0.1)
					print("End", infos[0], "logs")
				else:
					print(infos)
					break
		print("Del", infos[0], ":", fetcher_id)
		del(self.fetchers[fetcher_id])
		print(self.fetchers)
		print("END OF LOG FILLED")
