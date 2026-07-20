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
# Marguez APP
#

import os
import re
import sys
import json
import time
import getopt
import psutil
import select
import asyncio
import importlib
import subprocess
import configparser
import requests_openapi

from .lib.infos_panel import InfosPanel
from .lib.services_panel import ServicesPanel
from .lib.messages_panel import MessagesPanel
from .lib.ribs import RibsKitchen, RibsRecipes

from glob import glob
from datetime import datetime, timedelta
from pymemcache.client.base import Client
from pymemcache import serde
from marguez.logger import printer
from textual import work
from rich.text import Text
from textual.markup import escape
from rich.errors import MarkupError
from textual.screen import Screen
from textual.reactive import reactive
from textual.app import App, ComposeResult
from textual.logging import TextualHandler
from textual.containers import Horizontal, Vertical, HorizontalGroup, VerticalScroll
from textual.widgets import Input, Label, Rule, Button, TabPane, TabbedContent, Digits, Footer, Header, RichLog, Log, ContentSwitcher, Select, Checkbox

def markup_text(line):
	try:
		return Text.from_markup(line)
	except MarkupError:
		return Text(line)

class Logs(Screen):
	BINDINGS = (
		("l", "app.pop_screen", "Hide"),
		("e", "app.pop_screen", "Hide"),
		("c", "clear", "Clear"),
	)

	def compose(self):
		yield Header()
		yield Footer()
		yield RichLog(id="logs_panel")
	
	def on_mount(self):
		self.logs_panel = self.query_one("#logs_panel")
		with open(printer.out_file, "r") as f:
			for line in f.readlines():
				sline = line.split(" ", 3)
				if len(sline) == 4:
					self.logs_panel.write(Text.from_markup(sline[0]+" "+sline[1]+" "+sline[2]+" ")+markup_text(sline[3].rstrip()))
				elif line:
					self.logs_panel.write(markup_text(line.rstrip()))
		self.begin_capture_print(stderr=False)
	
	def on_print(self, event):
		line = event.text.rstrip()
		if not line:
			return
		sline = line.split(" ", 3)
		if len(sline) == 4:
			self.logs_panel.write(Text.from_markup(sline[0]+" "+sline[1]+" "+sline[2]+" ")+markup_text(sline[3].rstrip()))
		elif line:
			self.logs_panel.write(markup_text(line))

	def action_clear(self):
		self.logs_panel.clear()


class Errors(Screen):
	BINDINGS = (
		("e", "app.pop_screen", "Hide Logs"),
		("l", "app.pop_screen", "Hide Logs"),
	)

	def compose(self):
		yield Header()
		yield Footer()
		yield RichLog(id="errors_panel", markup=True)
	
	def on_mount(self):
		self.query_one(RichLog).write(last_run_errors)
		err_file = "."+sys.argv[0].split("/")[-1].split(".")[0]+".err"
		if os.path.isfile(err_file):
			with open(err_file):
				self.query_one(RichLog).write(f.read())


class Marguez(App):
	""" A Textual app to manage R.I.B.S and Dags. """

	bindings_yn = False
	rich_log = None
	is_running = True
	selected_service = None
	last_selected_service = None
	stoppped_dags = []
	last_updated_service = None
	last_forced_status = 0
	fill_logs = False
	is_web = False
	SCREENS = {"logs": Logs, "errors": Errors}
	CSS_PATH = ["css/app.tcss", "css/ribs.tcss", "css/infos.tcss", "css/services.tcss", "css/messages.tcss"]
	BINDINGS = DEFAULT_BINDINGS = (
		("?", "help", "Help"),
		("q", "ask_quit", "Quit"),
		("l", "push_screen('logs')", "Logs"),
		("e", "push_screen('errors')", "LastRun Errors"),
		("r", "restart", "Restart"),
		("Do you want quit?", "question", " "),
		("y", "quit", "Yes"),
		("n", "cancel", "No")
	)
	
	def parseArgs(self):
		for arg in sys.argv[1:]:
			if arg in ("-h", "--help"):
				print(sys.argv[0]+" [-h|--help] [--web]")
				sys.exit()
			elif arg == "--web":
				self.is_web = True

	def register(self, shard_path, dagu_path):
		self.config = configparser.ConfigParser()
		self.config.read("/etc/ryzom/shard.ini")
		self.shard_path = shard_path
		self.dagu_path = dagu_path
		self.dagu_client = requests_openapi.Client().load_spec_from_file(shard_path+"/tools/dagu_api.yaml")
		self.shard = self.config["shard"]["name"]
		self.hostname = self.config["shard"]["hostname"]
		self.dagu_client.set_server(requests_openapi.Server(url="http://"+self.hostname+":9888/api/v2"))
		self.client = Client("localhost", serde=serde.pickle_serde)
		self.services = {}
		self.services_status = {}
		self.dag_runs = {}
		self.dag_pids = {}

	def compose(self):
		yield Header()
		yield Footer()
		yield ServicesPanel()
		with Vertical():
			yield InfosPanel()
			with ContentSwitcher(initial="messages"):
				yield RibsKitchen(id="kitchen")
				yield MessagesPanel(id="messages")

	def on_ready(self):
		self.last_update = time.time()

		self.infos_panel = self.query_one(InfosPanel)
		self.action_bar = self.infos_panel.query_one("#action_bar")
		self.services_panel = self.query_one(ServicesPanel)
		self.content_panel = self.query_one(ContentSwitcher)
		self.ribs_kitchen = self.query_one(RibsKitchen)
		self.messages_panel = self.query_one(MessagesPanel)
		self.ribs_kitchen.open()
		self.ribs_kitchen.getRecipes()
		self.setLogSections()
		self.checkServices()

	def switchContent(self, current):
		self.content_panel.current = current

	def updateInfosPanel(self, infos_1=None, infos_2=None, infos_3=None):
		self.infos_panel.updateInfos(infos_1, infos_2, infos_3)


	def on_input_submitted(self, event):
		# TODO : use dagu to know what to do with input
		event.input.value = ""
		
		if event.value and event.value[0] == "/":
			# It's a search in logs => send to messages panel
			self.messages_panel.search(event.value[1:])
			event.input.placeholder = event.value
			return
		service = "Ryzom_"+self.selected_service.split("_", 1)[-1]
		try:
			cid = self.client.incr(f"DagRun-{service}-Id", 1)
		except:
			cid = None
		
		if not cid:
			cid = 1
			self.client.set(f"DagRun-{service}-Id", 1)

		print("Send Command", cid, event.value, "to", service)
		self.client.set(f"DagRun-{service}-{cid}", event.value)

	def setLogSections(self, is_new_selected_service=False):
		if self.selected_service and self.selected_service in self.services:
			if not "LogSections" in self.services[self.selected_service]:
				self.updateDagInfos(self.selected_service)
			self.messages_panel.setLogTabs(self.services[self.selected_service]["LogSections"])
	
	def updateDagInfos(self, filename):
		infos = self.dagu_client.getDAGDetails(fileName=filename).json()
		prio = "00"
		status_updater = "dag"
		logs = [["Logs", "rich", {}], ["Errors", "rich", {}], ["Dagu", "dagu", {}]]
		logs[0][2]["file"] = infos["latestDAGRun"]["nodes"][0]["stdout"]
		logs[1][2]["file"] = infos["latestDAGRun"]["nodes"][0]["stderr"]
		logs[2][2]["raw"] = {"infos": infos, "procs": self.getDagProcs(infos), "file": self.dagu_path+"dags/"+filename+".yaml"}
		vars_envs = {}
		for env in infos["dag"]["env"]:
			name, value = env.split("=", 1)
			vars_envs[name] = value
			
		if "env" in infos["dag"]:
			for env in infos["dag"]["env"]:
				name, value = env.split("=", 1)
				for n,v in vars_envs.items():
					value = value.replace("${"+n+"}", v)
				
				if name[:4] == "LOG_":
					if name == "LOG_FILE":
						logs.append(["Dagu-Logs", "rich", {"file": infos["latestDAGRun"]["nodes"][0]["stdout"]}])
					logs[0][2][name[4:].lower()] = json.loads(value.replace("'", "\"")) if name == "LOG_FORMAT_SUBST" else value
				elif name[:4] == "ERR_":
					logs[1][2][name[4:].lower()] = json.loads(value.replace("'", "\"")) if name == "ERR_FORMAT_SUBST" else value
				elif name[:3] == "DB_":
					logs.append([name[3:], "db", json.loads(value.replace("'", "\""))])
				elif name == "STATUS_UPDATER":
					status_updater = value
		self.services[filename]["StatusUpdater"] = status_updater
		self.services[filename]["LogSections"] = logs
		self.last_updated_service = ""

	@work(thread=True)
	def checkServices(self):
		#not started = 0, running = 1, failed = 2,  cancelled = 3, finished = 4, queued = 5, partial success = 6, starting = 7, stopping = 8
		status_emojis = ("⚪️", "🟢", "‼️ ", "🔴", "🔵", "🟣", "🟡", "🟠", "⚫️")
		first = True
		while self.is_running:
			dags = self.dagu_client.listDAGs().json()
			services = {}
			update_services = False
			if first:
				print(dags)
				first = False
			for dag in dags["dags"]:
				if not "group" in dag["dag"]:
					continue
					
				filename = dag["fileName"]
				services[filename] = dag
				status_updater = "dag"
				status = None

				if not filename in self.services:
					self.services[filename] = services[filename]
					update_services = True
				elif dag["latestDAGRun"] != self.services[filename]["latestDAGRun"]:
						self.services[filename]["latestDAGRun"] = dag["latestDAGRun"]
						if "LogSection" in self.services[filename]:
							del(self.services[filename]["LogSection"])
						if self.selected_service == filename:
							self.setLogSections()
							self.last_update = 0

				if "StatusUpdater" in self.services[filename]:
					status_updater = self.services[filename]["StatusUpdater"]
				
				if filename in self.services and "ForceStatus" in self.services[filename]:
					old_status, temp_status = self.services[filename]["ForceStatus"]
					if dag["latestDAGRun"]["status"] == old_status:
						status = temp_status
					else:
						del(self.services[filename]["ForceStatus"])

				if not status:
					status = dag["latestDAGRun"]["status"]
				
				status_emoji = None
				if self.services_panel.project == dag["dag"]["group"]:
					if status_updater != "dag" and dag["latestDAGRun"]["status"] == 1:
						last_update = self.client.get(status_updater+"LastUpdate")
						if last_update:
							last_update = float(last_update)
							status_emoji = self.client.get(status_updater+"Status").decode("utf-8")
							if status_emoji == "✅":
								if time.time() - last_update > 10:
									status_emoji = "⚫️"
								elif time.time() - last_update > 5:
									status_emoji = "🟠"
								elif time.time() - last_update > 2:
									status_emoji = "🟡"
								else:
									status_emoji = "🟢"
				
				if not status_emoji:
					try:
						status_emoji = status_emojis[status]
					except:
						status_emoji = "⁉️"

				if not "Status" in self.services[filename] or self.services[filename]["Status"] != status_emoji:
					try:
						self.query_one("#status_"+filename).update(status_emoji)
					except:
						pass
					else:
						self.services[filename]["Status"] = status_emoji

			if update_services or self.services.keys() != services.keys():
				self.services_panel.services = self.services = services
			
			if self.fill_logs:
				self.messages_panel.startFillingLogs()
				self.fill_logs = False
			
			if time.time() > self.last_update + 1:
				self.last_update = time.time()
				self.call_from_thread(self.updateInfos, self.selected_service)
			
			time.sleep(0.1)


	def getProcessChildrens(self, proc, prefix=""):
		childrens = []
		if proc:
			try:
				childrens.append(str(proc.pid)+" "+prefix+" ".join(proc.cmdline()))
			except:
				pass
			if not prefix:
				prefix = "⮑ "
			else:
				prefix = "  "+prefix
			try:
				proc = psutil.Process(proc.pid)
			except psutil.NoSuchProcess:
				return None
			children = proc.children(recursive=False)
			for child in children:
				childs = self.getProcessChildrens(child, prefix)
				if childs:
					for c in childs:
						childrens.append(c)
		return childrens


	def getDagProc(self, dagRunId):
		proc = None
		for p in psutil.process_iter():
			try:
				cmd = p.cmdline()
			except:
				cmd = ""
			if len(cmd) > 2 and cmd[0] == "/usr/local/bin/dagu":
				if cmd[2] == "--run-id="+dagRunId or cmd[3] == "--run-id="+dagRunId:
					proc = p
		return proc

	def getDagPid(self, dagRunId):
		proc = self.getDagProc(dagRunId)
		return proc.pid if proc else -1

	def getDagProcs(self, infos):
		if "latestDAGRun" in infos and "dagRunId" in infos["latestDAGRun"]:
			return self.getProcessChildrens(self.getDagProc(infos["latestDAGRun"]["dagRunId"]))
		return ""

	@work()
	async def stopDagu(self, infos):
		procs = self.getDagProcs(infos)
		procs.reverse()
		print("Stop DAGU")
		self.dagu_client.terminateDAGRun(name=infos["dag"]["name"], dagRunId=infos["latestDAGRun"]["dagRunId"])
		time.sleep(1)
		for p in procs:
			ps = p.split(" ", 1)
			try:
				proc = psutil.Process(int(ps[0]))
				proc.terminate()
			except psutil.NoSuchProcess:
				pass
			else:
				try:
					proc.wait(5)
				except psutil.TimeoutExpired:
					proc.kill()


	def updateInfos(self, service):
		if not service in self.services:
			return

		infos = self.services[service]
		status = infos["latestDAGRun"]["status"]
		status_label = infos["latestDAGRun"]["statusLabel"].lower()
		
		if "ForceStatus" in infos:
			old_status, temp_status = infos["ForceStatus"]
			if status == old_status:
				status = temp_status
				if temp_status == 7:
					status_label = "starting"
				else:
					status_label = "stopping"
			if self.last_forced_status != status:
				self.last_forced_status = status
				self.last_updated_service = None
		else:
			self.last_forced_status = None
		
		infos_1 = infos_2 = infos_3 = None
		if self.last_updated_service != service:
			infos_1 = "{} [orange1][/orange1][white]{}.yaml[/white]".format(infos["dag"]["name"], infos["fileName"])
			self.action_bar.hideAll()
			if status_label == "running":
				self.action_bar.show("restart")
				self.action_bar.show("stop")
				infos_2 = "[green]{}:[/green] {} [orange1]Pid:[/orange1] {}".format(status_label.title(), infos["latestDAGRun"]["startedAt"][:19].replace("T", " "), self.getDagPid(infos["latestDAGRun"]["rootDAGRunId"]) if "rootDAGRunId" in infos["latestDAGRun"] else "")
			elif status_label == "queued":
				infos_2 = "[violet]{}:[/violet] {}".format(status_label.title(), infos["latestDAGRun"]["queuedAt"][:19].replace("T", " "))
				self.action_bar.show("dequeue")
			elif status_label in ("failed", "canceled"):
				self.action_bar.show("start")
				infos_2 = "[red]{}:[/red] {}".format(status_label.title(), infos["latestDAGRun"]["startedAt"][:19].replace("T", " "))
			elif status_label in ("finished", "partial success"):
				self.action_bar.show("start")
				infos_2 = "[cyan]{}[/cyan]: {}".format(status_label.title(), infos["latestDAGRun"]["finishedAt"][:19].replace("T", " "))
			else:
				infos_2 = "[yellow]{}[/yellow]".format(status_label.title())
			
			if not status_label in ("running", "queue") and self.services_panel.project == "r_i_b_s":
				self.action_bar.show("remove")
	
			self.last_updated_service = service

		if status_label == "running":
			uptime = str(timedelta(seconds=round(time.time() - time.mktime(datetime.strptime(infos["latestDAGRun"]["startedAt"][:19], "%Y-%m-%dT%H:%M:%S").timetuple()))))
			infos_3 = "[orange1]Uptime: [/orange1] {}".format(uptime)
		self.updateInfosPanel(infos_1, infos_2, infos_3)

		
	def check_action(self, action, parameters):
		if action in ["question", "quit", "cancel"]:
			return self.bindings_yn
		if action == "restart":
			return self.is_web
		return not self.bindings_yn
	
	def select_service(self, service):
		print(f"Select Service {service}")
		if self.selected_service:
			try:
				button = self.services_panel.query_one("#"+self.selected_service)
			except:
				pass
			else:
				button.styles.color = "orange"
		try:
			button = self.services_panel.query_one("#"+service)
		except:
			pass
		else:
			button.styles.color = "white"
		self.selected_service = service
		self.last_update = 0
		self.setLogSections(True)
					
	def action_ask_quit(self):
		self.bindings_yn = True
		self.refresh_bindings()
	
	def action_cancel(self):
		self.bindings_yn = False
		self.refresh_bindings() 
	
	def action_restart(self):
		self.app.open_url("http://"+self.hostname, new_tab=False)
		self.exit()

	def action_logs(self):
		pass

	def action_quit(self):
		self.is_running = False
		self.messages_panel.is_locked = False
		self.messages_panel.stopFillingLogs()
		self.exit()
	
	def action_help(self):
		print("ok")

if __name__ == "__main__":
	app = Marguez()
	print("done")
	app.parseArgs()
	app.register()
	app.run()
