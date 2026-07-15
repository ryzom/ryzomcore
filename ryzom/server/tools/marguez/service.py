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
# Base class for Marguez services
#

import os
import builtins
import marguez.logger

from time import time
from pymemcache.client.base import Client
from pymemcache import serde

class Service():
	
	def __init__(self):
		self.client = Client("localhost", serde=serde.pickle_serde)
		self.last_update = time()
		self.start = time()
		self.log_sections = {}
		self.services_last_updates = {}

	def register(self):
		self.client.set(f"Dag-{self.name}-LastUpdate", time())

	def updateActivity(self, is_ping=True):
		"""Update activity of the service. If it's a simple ping, the update is old of 0.5s.
		So supervisor can check if an update is do in less than 0.5s => it's a real activity, not just a ping"""
		if not is_ping or time() >= self.last_update + 0.5:
			# When a ping, only update each 0.5 seconds
			self.last_update = update = time()
			if is_ping:
				# A ping is old of 0.5s
				update -= 0.5 
			self.client.set(f"Dag-{self.name}-LastUpdate", update)
	
	def getMessage(self, name):
		return self.client.get(name)

	def setMessage(self, name, value, expire=0):
		return self.client.set(name, value, expire=expire)

	def updateInfos(self):
		self.client.set(f"Dag-{self.name}-Infos", self.infos)


