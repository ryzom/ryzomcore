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
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# Info Panel using 3 informations text and the ActionBar
#

import subprocess

from textual.containers import Horizontal, Vertical, HorizontalGroup, VerticalScroll
from textual.widgets import Button, RichLog

class ActionBar(Horizontal):
	""" Container with all buttons to run actions """

	callbacks = {}

	def compose(self):
		yield Button("🥣 Prepare", id="prepare")
		yield Button("🍲 Cook", id="cook")
		yield Button("▶️ Start", id="start")
		yield Button("🔁 Restart", id="restart")
		yield Button("🟥 Stop", id="stop")
		yield Button("🟥 DeStop", id="dequeue")
		yield Button("🚮 Remove", id="remove")

	def hideAll(self):
		for child in self._nodes:
			child.styles.display = "none"
	
	def showAll(self):
		for child in self._nodes:
			child.styles.display = "block"
	
	def show(self, action):
		self.query_one("#"+action).styles.display = "block"

	def hide(self, action):
		self.query_one("#"+action).styles.display = "none"

	def showOnly(self, action):
		self.hideAll()
		self.show(action)

	def registerAction(self, action, callback):
		self.callbacks[action] = callback

	def forgetAction(self, action):
		del(self.callbacks[action])

	def callAction(self, action):
		if action in self.callbacks:
			self.callbacks[action]()
			return True
		return False

	def on_button_pressed(self, event):
		action = event.button.id
		
		if self.callAction(action):
			return
		
		name = self.app.selected_service
		if name:
			infos = self.app.dagu_client.getDAGDetails(fileName=name).json()
		if action == "start":
			if name:
				subprocess.Popen(["dagu", "enqueue", name, "-c", "/etc/dagu.yaml"])
				self.app.services[name]["ForceStatus"] = (infos["latestDAGRun"]["status"], 7)
				self.app.last_update = 0
		elif action == "stop":
			if name:
				self.app.stopDagu(infos)
				self.app.services[name]["ForceStatus"] = (infos["latestDAGRun"]["status"], 8)
				self.app.last_update = 0
			else: #ribs
				self.app.ribs_panel.stopPrepareRibs()
		elif action == "dequeue":
			self.app.dagu_client.dequeueDAGRun(name=infos["dag"]["name"], dagRunId=infos["latestDAGRun"]["dagRunId"])
			self.app.services[name]["ForceStatus"] = (infos["latestDAGRun"]["status"], 7)
			self.app.last_update = 0


class InfosPanel(Vertical):
	""" Container to display informations and the Actionbar """

	def compose(self):
		yield RichLog(id="infos_1", classes="no-scroll", markup=True)
		yield Horizontal(
				RichLog(id="infos_2", classes="no-scroll", markup=True),
				ActionBar(id="action_bar"),
			)

	def updateInfos(self, infos_1=None, infos_2=None, infos_3=None):
		if infos_1 != None:
			i = self.query_one("#infos_1")
			i.clear()
			if infos_1:
				i.write(infos_1)
		
		if infos_2 != None:
			i = self.query_one("#infos_2")
			i.clear()
			if infos_2:
				i.write(infos_2)
		
		if infos_3:
			self.border_subtitle = infos_3
		else:
			self.border_subtitle = ""

