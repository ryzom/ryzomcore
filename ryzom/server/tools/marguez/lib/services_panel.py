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

from textual.reactive import reactive
from textual.containers import Horizontal, VerticalScroll, Vertical
from textual.widgets import Button, Label, Rule


class ServicesPanel(VerticalScroll):
	
	services = reactive({}, recompose=True)
	project = reactive("r_i_b_s", recompose=True)
	previous_button = None
	last_height = 3
	mounted = False

	def compose(self):
		sorted_services = []
		projects = {"r_i_b_s": []}
		project_options = []
		selected_project = self.project
		for service, infos in self.services.items():
			prio = "00"
			if "tags" in infos["dag"]:
				for tag in infos["dag"]["tags"]:
					if tag[:2] == "p_":
						prio = tag[2:]
			if not "group" in infos["dag"]:
				continue
			
			if not infos["dag"]["group"] in projects:
				projects[infos["dag"]["group"]] = []
			projects[infos["dag"]["group"]].append(prio+service)

		print(projects)
		if not projects.keys():
			return
	
		if selected_project in projects:
			sorted_services = projects[selected_project]
		else:
			sorted_services = []

		project_names = [(project, project) for project in projects.keys()]
		#yield Select(project_names, value=selected_project, allow_blank=False, prompt="Project")
		for name, service in projects.items():
			yield Horizontal(Label("💠" if name != "r_i_b_s" else "⚜️ "), Button(name.replace("_", " ").title(), id="project_"+name, classes="selected project" if selected_project == name else "project"))
		yield Rule()
		sorted_services.sort(reverse=True)
		self.selected_service = sorted_services[0][2:] if sorted_services else None
		with Vertical(id="services"):
			for service in sorted_services:
				filename = service[2:]
				infos = self.services[filename]
				yield Horizontal(Label("🔘" if not "Status" in infos else infos["Status"], id="status_"+filename), Button(infos["dag"]["name"], id=filename, classes="selected" if self.selected_service == filename else ""), classes="service height1")

		if self.selected_service and self.project != "r_i_b_s":
			self.app.select_service(self.selected_service)
		else:
			self.app.selected_service = self.selected_service

	def on_button_pressed(self, event):
		if event.button.id[:8] == "project_":
			self.project = event.button.id[8:]
			self.app.services_status = {}
			if self.project == "r_i_b_s":
				self.app.ribs_kitchen.open()
				self.app.select_service("")
			else:
				self.app.switchContent("messages")
			self.app.last_updated_service = None
		else:
			self.app.switchContent("messages")
			self.app.select_service(event.button.id)
