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
# Ribs classes
#

# Ribs system are composed by:
#  - Recipes (who are a list of ribs)
#  - Recipes are a list of Ribs and each Ribs can be enabled or not
#  - Ribs have a list of Spices who need be filled or selected by user
# Ribs system work like that:
# 1) User choose the Recipe to use
# 2) User select the Ribs he want to use
# 3) User start the preparation
# 4) When preparation is done, User fill/select the Spices
# 5) User cook the Recipe

import os
import time
import select
import subprocess

from rich.markup import escape, MarkupError
from textual import work
from textual import log
from textual.reactive import reactive
from textual.containers import Horizontal, Vertical, VerticalScroll, Grid
from textual.widgets import Label, Checkbox, Select, TabbedContent, TabPane, RichLog, Button, Input, ContentSwitcher

class RibsLogger(RichLog):
	stderr = True
	stdout = True
	
	def start(self, out, err, callback=None):
		self.printOutput(out, callback)
		self.printOutput(err, None)
	
	
	def printOutput(self, stream, callback):
		while True:
			line = stream.readline()
			if not line:
				self.stdout = False
				return
			else:
				if callback:
					line = line.decode().rstrip()
				else:
					line = "[red]"+line.decode().rstrip()+"[/red]"
				if callback:
					if callback(line):
						try:
							self.write(line)
						except MarkupError:
							self.write(escape(line))
				else:
					try:
						self.write(line)
					except MarkupError:
						self.write(escape(line))
	
	def show(self):
		self.styles.display = "block"

	def hide(self):
		self.styles.display = "none"


class RibsCheckbox(Checkbox):
	""" This checkbox will hide or display the options of the Ribs """

	BUTTON_LEFT = ""
	BUTTON_INNER = "🟧"
	BUTTON_RIGHT = ""

	def on_checkbox_changed(self, event):
		if event.checkbox.value:
			self.parent.query_one("#"+event.checkbox.id+"_ingredients").styles.display = "block"
			self.BUTTON_INNER = "🟧"
		else:
			self.parent.query_one("#"+event.checkbox.id+"_ingredients").styles.display = "none"
			self.BUTTON_INNER = "⬛️"

# TODO: Manage params in runRibs
class RibsSpices(VerticalScroll):
	""" A container who will display all params of the Ribs """

	form = reactive({}, recompose=True)
	
	def compose(self):
		yield Label("🔹🔹🔹 Spices 🔹🔹🔹", classes="ribs_title")
		for form in self.form:
			if form[0] == "label":
				yield Label("🔹"+form[1].replace("\\n", "\n"), markup=True)
			elif form[0] == "select":
				options = []
				selected = ""
				for o in form[1]:
					so = o.split(":")
					if len(so) == 2:
						if so[0][0] == "+":
							so[0] = so[0][1:]
							selected = so[0]
						options.append((so[1], so[0]))
						
				if selected:
					yield Select(options, value=selected, compact=True)
				else:
					yield Select(options, compact=True)
			elif form[0] == "input":
				if len(form[1]) == 2:
					yield Input(value=form[1][1])
				else:
					yield Input()

class RibsRecipes(VerticalScroll):
	""" A container who will display all Ribs recipes """
	
	recipes = reactive({}, recompose=True)
	last_selected = None
	
	def compose(self):
		yield Label("🔹🔹🔹 Recipes 🔹🔹🔹", classes="ribs_title")
		for name, infos in self.recipes.items():
			yield Button(infos["display"], id=name, classes="height1")

	def on_button_pressed(self, event):
		if self.last_selected:
			self.last_selected.styles.color = "orange"
		self.last_selected = event.button
		self.last_selected.styles.color = "white"
		self.app.set_focus(None)
		recipes = self.recipes[event.button.id]
		self.app.updateInfosPanel(None, "[orange1]"+recipes["display"]+"[/orange1]", None)
		self.kitchen.workplan_panel.styles.display = "block"
		self.kitchen.workplan_panel.ribs = recipes["infos"]
		self.kitchen.ready_to_prepare = True
		self.app.action_bar.showOnly("prepare")
		self.selected_recipe = event.button.id
		self.kitchen.query_one("#ribs_preparation_output").hide()


class RibsWorkplan(VerticalScroll):
	""" A container to display Ribs to cook and options """
	ribs = reactive({}, recompose=True)
	
	def compose(self):
		for infos in self.ribs:
			sinfos = infos.strip().split(",")
			if len(sinfos) > 1:
				name = sinfos[1].replace(" ", "_").lower()
				yield RibsCheckbox(sinfos[1], id="ribs_"+name, value=True, classes="ribs")
				with Grid(id="ribs_"+name+"_ingredients", classes="ingredients"):
					for param in sinfos[2:]:
						status = param[0] == "+"
						if status:
							yield Button(param[1:], id="ribs_option_"+param[1:], classes="selected ribs_option")
						else:
							yield Button(param, id="ribs_option_"+param, classes="ribs_option")
	
	

class RibsKitchen(Horizontal):
	""" A container to display the Workplan, Recipes and Spices """
	
	opened = False
	ready_to_prepare = False
	ready_to_cook = False
	preparing_ribs = False
	cooking_ribs = False
	
	def compose(self):
		with TabbedContent(id="ribs_kitchen"):
			with TabPane("🥣 Preparation", id="preparation"):
				with Horizontal():
					yield RibsLogger(id="ribs_preparation_output", classes="messages", markup=True)
					yield RibsWorkplan()
					yield RibsRecipes()
			with TabPane("🍲 Cooking", id="cooking"):
				with Horizontal():
					yield RibsLogger(id="ribs_cooking_output", classes="messages", markup=True)
					yield RibsSpices(id="ribs_spices")

	def on_mount(self):
		self.ribs_recipes = self.query_one(RibsRecipes)
		self.ribs_path = self.app.shard_path+"tools/ribs"

	def on_button_pressed(self, event):
		event.button.toggle_class("selected")
	
	def on_tabbed_content_tab_activated(self, event):
		if not self.opened:
			return
		if event.tab.id == "--content-tab-preparation":
			if self.preparing_ribs:
				self.app.action_bar.showOnly("stop")
			elif self.ready_to_prepare:
				self.app.action_bar.showOnly("prepare")
		else:
			if self.cooking_ribs or self.preparing_ribs:
				self.app.action_bar.showOnly("stop")
			elif self.ready_to_cook:
				self.app.action_bar.showOnly("cook")
			else:
				self.app.action_bar.hideAll()
	
	def open(self):
		self.opened = True
		self.app.updateInfosPanel("== R.I.B.S. ==", "[orange1]Please select a recipe at right[/orange1]", "")
		self.app.switchContent("kitchen")
		self.switch = self.query_one("#ribs_kitchen")
		self.switch.active = ""
		self.switch.active = "preparation"
		self.workplan_panel = self.query_one(RibsWorkplan)
		self.recipes_panel = self.query_one(RibsRecipes)
		self.spices_panel = self.query_one(RibsSpices)
		self.workplan_panel.kitchen = self
		self.recipes_panel.kitchen = self
		self.spices_panel.kitchen = self
		self.app.action_bar.hideAll()
		self.app.action_bar.registerAction("prepare", self.prepareRibs)
		self.app.action_bar.registerAction("cook", self.runRibs)

	def updateRecipes(self, line):
		if line[:9] == ":>recipe:":
			infos = line[9:].strip().split("|")
			name = infos[1].replace(" ", "_").lower()
			print(name, infos)
			self.temp_recipes[name] = {"display": infos[0]+" "+infos[1], "infos": infos[2:]}
			return False
		return True

	def updateForm(self, line):
		if line[:2] == ":>":
			sline = line.split(":", 2)
		else:
			return True
		if sline[1] == ">ask":
			self.form.append(("label", sline[2]))
		elif sline[1] == ">select":
			self.form.append(("select", sline[2].split("|")))
		elif sline[1] == ">input":
			self.form.append(("input", sline[2].split("|")))
		else:
			return True
		return False

	@work(thread=True)
	def getRecipes(self):
		self.temp_recipes = {}
		self.query_one("#ribs_preparation_output").show()
		self.ribs_path = self.app.shard_path+"tools/ribs"
		env = {"RIBS_GROUPS" : ":admin:dev:", "RIBS_PATH": self.ribs_path}
		logger = self.query_one("#ribs_preparation_output")
		for file in os.listdir(self.ribs_path+"/recipes/"):
			logger.write(f"[cyan]Getting recipes from[/] [orange1]{file}[/]")
			if file.endswith(".sh"):
				p = subprocess.Popen(["bash", self.ribs_path+"/recipes/"+file], stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
				logger.start(p.stdout, p.stderr, self.updateRecipes)
		print("getRecipes", self.temp_recipes)
		self.ribs_recipes.recipes = self.temp_recipes

	@work(thread=True)
	def prepareRibs(self):
		if self.preparing_ribs:
			self.notify("Allready preparing ribs...", title="== R.I.B.S. ==", severity="warning")
			return

		self.app.action_bar.registerAction("stop", self.stopPrepareRibs)
		self.forms = []
		self.preparing_ribs = True
		self.ready_to_cook = False
		self.switch.active = ""
		self.switch.active = "cooking"
		self.selected_ribs = None

		self.recipes = self.ribs_recipes.recipes
		dagu_name = "Ribs_"+self.ribs_recipes.selected_recipe.title()
		self.query_one("#ribs_preparation_output").hide()
		logger = self.query_one("#ribs_cooking_output")
		logger.clear()
		recipes = self.recipes[self.ribs_recipes.selected_recipe]
		options = recipes["infos"]
		print("prepareRibs", recipes)
		#status = self.query_one("#ribs_spices").query_one("#ribs_status")
		for infos in options:
			ribs_options = ["-"]
			sinfos = infos.split(",")
			self.form = []
			if len(sinfos) > 1:
				#status.update(f"⮞ {sinfos[1]}...")
				name = sinfos[1].replace(" ", "_").lower()
				if self.query_one("#ribs_"+name).value:
					for option in sinfos[2:]:
						option_name = option[1:] if option[0] == "+" else option
						if self.query_one("#ribs_option_"+option_name).has_class("selected"):
							ribs_options.append(option_name)
					cmd = ["bash", self.ribs_path+"/scripts/"+sinfos[0]]+ribs_options
					env = {"RIBS_GROUPS" : ":admin:dev:", "RIBS_PATH": self.ribs_path}
					size = len(sinfos[1])
					if size % 2:
						sinfos[1] += " "
					size = round(0.5 + ((80 - size) / 2))-2
						
					self.proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
					logger.write("[bold blue]"+"═"*size+"╡ "+sinfos[1]+" ╞"+"═"*size+"[/]")
					logger.start(self.proc.stdout, self.proc.stderr, self.updateForm)
					logger.write("[bold blue]"+"―"*80+"[/]\n")
					if self.proc == None:
						break
				if self.form:
					self.forms += self.form
		if self.forms:
			self.spices_panel.form = self.forms
		self.preparing_ribs = False
		if self.proc:
			self.ready_to_cook = True
			self.app.action_bar.showOnly("cook")
			parent_ribs = ""
			ribs_params = {}

			for infos in options:
				sinfos = infos.split(",")
				if len(sinfos) > 1:
					name = sinfos[1].replace(" ", "_").lower()
					file = "Ribs_"+sinfos[1].replace(" ", "_")
					if self.query_one("#ribs_"+name).value:
						self.selected_ribs = file

						ribs_params[file] = {"NAME": sinfos[1], "FILE": sinfos[0], "PARAMS": ""}
						if parent_ribs:
							ribs_params[parent_ribs]["NEXT"] = file
						parent_ribs = file
						opts = ""
						i = 0
						for param in sinfos[2:]:
							i += 1
							param_name = param[1:] if param[0] == "+" else param
							if self.query_one("#ribs_option_"+param_name).has_class("selected"):
								ribs_params[file][f"OPT_{i}"] = param_name
							else:
								ribs_params[file][f"OPT_{i}"] = ""
							ribs_params[file]["PARAMS"] += f" ${{OPT_{i}}}"

			for ribs, rparams in ribs_params.items():
				params = ""
				for n, v in rparams.items():
					if not n in ("NAME", "FILE", "PARAMS"):
						params += f"  - {n}: \"{v}\"\n"
				dag = f"""name: {rparams["NAME"]}
description: Ryzom Ribs
group: r_i_b_s
env:
  - PATH: "${{PATH}}:${{HOME}}/bin:${{HOME}}/.local/bin/"
  - RIBS_GROUPS: ":admin:dev:"
  - RIBS_PATH: "{self.ribs_path}"
params:
{params}
steps:
  - name: run
    dir: {self.ribs_path}/scripts/
    command: bash {rparams["FILE"]} Run{rparams["PARAMS"]}
"""
				if "NEXT" in rparams:
					dag += """handlerOn:
  success:
    command: dagu enqueue ${NEXT} -c /etc/dagu.yaml
"""
				with open(self.app.dagu_path+"dags/"+ribs+".yaml", "w") as f:
					f.write(dag)
		else:
			self.switch.active = ""
			self.switch.active = "preparation"
		self.app.action_bar.forgetAction("stop")



	def stopPrepareRibs(self):
		self.proc.kill()
		self.proc = None

	def runRibs(self):
		if self.selected_ribs:
			subprocess.Popen(["dagu", "enqueue", self.selected_ribs, "-c", "/etc/dagu.yaml"])
			self.app.last_update = 0
			self.app.select_service(self.selected_ribs)
			self.app.switchContent("messages")
		
