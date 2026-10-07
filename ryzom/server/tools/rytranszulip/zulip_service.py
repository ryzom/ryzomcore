
#############################################
#  _______________________________
#  \______   \__    ___/\____    /
#   |       _/ |    |     /     /
#   |    |   \ |    |    /     /_
#   |____|_  / |____|   /_______ \
#          \/                   \/
#
# RyTransZulip - with delicious M.A.R.G.U.E.Z
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# Base class for Ryzom <-> Deepl <-> Zulip system
#

import re
import time
import traceback
import zulip
import requests

from ryzom_service import RyzomService, RyzomMessage

NETWORK_ERRORS = (requests.exceptions.Timeout, requests.exceptions.SSLError, requests.exceptions.ConnectionError)
EVENT_TYPES = ["message", "update_message", "reaction"]

class ZulipClient(zulip.Client):
	def doRegister(self, event_types, narrow, **kwargs):
		while True:
			if event_types is None:
				res = self.register(None, None, **kwargs)
			else:
				res = self.register(event_types, narrow, **kwargs)
			if "error" in res["result"]:
				if self.verbose:
					print("Server returned error:\n{}".format(res["msg"]))
				time.sleep(1)
			else:
				self.queue_id = res["queue_id"]
				self.last_event_id = res["last_event_id"]
				return self.queue_id

	def call(self, callback, event_types, narrow, on_register=None, on_poll=None, **kwargs):
		if narrow is None:
			narrow = []

		# Make long-polling requests with `get_events`. Once a request
		# has received an answer, pass it to the callback and before
		# making a new long-polling request.
		while True:
			try:
				pending = bool(on_poll()) if on_poll is not None else False
			except NETWORK_ERRORS:
				if self.verbose:
					print(f"Connection error checking pending quotes:\n{traceback.format_exc()}")
				time.sleep(1)
				continue
			if pending:
				time.sleep(1)
			try:
				request = {"queue_id": self.queue_id, "last_event_id": self.last_event_id}
				if pending:
					request["dont_block"] = True
				res = self.get_events(**request)
			except NETWORK_ERRORS:
				if self.verbose:
					print(f"Connection error fetching events:\n{traceback.format_exc()}")
				# TODO: Make this use our backoff library
				time.sleep(1)
				continue
			except Exception:
				print(f"Unexpected error:\n{traceback.format_exc()}")
				# TODO: Make this use our backoff library
				time.sleep(1)
				continue

			if "error" in res["result"]:
				if res["result"] == "http-error":
					if self.verbose:
						print("HTTP error fetching events -- probably a server restart")
				else:
					if self.verbose:
						print("Server returned error:\n{}".format(res["msg"]))
					# Eventually, we'll only want the
					# BAD_EVENT_QUEUE_ID check, but we check for the
					# old string to support legacy Zulip servers.  We
					# should remove that legacy check in 2019.
					if res.get("code") == "BAD_EVENT_QUEUE_ID" or res["msg"].startswith(
						"Bad event queue id:"
					):
						# Our event queue went away, probably because
						# we were asleep or the server restarted
						# abnormally.  We may have missed some
						# events while the network was down or
						# something, but there's not really anything
						# we can do about it other than resuming
						# getting new ones.
						#
						self.doRegister(event_types, narrow, **kwargs)
						if on_register is not None:
							on_register(self.queue_id)
				# Add a pause here to cover against potential bugs in this library
				# causing a DoS attack against a server when getting errors.
				# TODO: Make this back off exponentially.
				time.sleep(1)
				continue

			for event in res["events"]:
				if event["type"] == "heartbeat":
					# Heartbeat events are sent to clients regardless
					# of the client's requested event types, and are
					# intended to be an internal part of the Zulip
					# longpolling protocol, not something that clients
					# need to handle.
					self.last_event_id = max(self.last_event_id, int(event["id"]))
					continue
				try:
					callback(event)
				except NETWORK_ERRORS:
					if self.verbose:
						print(f"Connection error processing event:\n{traceback.format_exc()}")
					time.sleep(1)
					break
				self.last_event_id = max(self.last_event_id, int(event["id"]))

	def registerMessages(self, **kwargs):
		self.doRegister(EVENT_TYPES, None, **kwargs)

	def manageMessages(self, callback, **kwargs):
		self.call(callback, EVENT_TYPES, None, **kwargs)


class ZulipService(RyzomService):

	def __init__(self):
		super().__init__()
		while True:
			try:
				self.zulip = ZulipClient(config_file=".zuliprc")
			except Exception as e:
				print("Error Zulip server", self.config["zulip"]["site"])
				print(e)
				print("Retrying...")
				time.sleep(1)
			else:
				break
		print("Connected! to", self.base_url, "!")


	def retryError(self, result):
		if result.get("retry-after"):
			time.sleep(float(result["retry-after"]))
		if result.get("result") == "http-error":
			status = result.get("status_code", 500)
			return status in (408, 429) or status >= 500
		if result.get("result") == "error" and result.get("code") is None:
			return False
		return result.get("code") in (None, "RATE_LIMIT_HIT", "BAD_EVENT_QUEUE_ID")

	def setZulipQueueId(self, queue_id):
		self.client.set("Zulip-Queue-Id", self.zulip.queue_id)

	def getZulipQueueId(self):
		return self.client.get("Zulip-Queue-Id")

	def getLastChatLangID(self, lang):
		lastid = self.client.get("Zulip-Chat-"+lang+"-LastID")
		if lastid != None:
			return int(lastid)
		self.client.set("Zulip-Chat-"+lang+"-LastID", 1)
		return 1

	def setLastChatLangID(self, lang, value):
		self.client.set("Zulip-Chat-"+lang+"-LastID", value)

	def addZulipMessageId(self, chat_id, message_id):
		self.client.set("Zulip-"+str(chat_id)+"-MessageID", message_id, 24*60*60)

	def getZulipMessageId(self, chat_id):
		return self.client.get("Zulip-"+str(chat_id)+"-MessageID")

	def addZulipMessage(self, message, lang):
		self.last_chat_id = self.client.incr("Zulip-Chat-"+lang+"-LastID", 1)
		self.client.set("Zulip-Chat-"+lang+"-"+str(self.last_chat_id), message.get(), 24*60*60)
		return self.last_chat_id

	def getZulipMessage(self, i, lang):
		return self.client.get("Zulip-Chat-"+lang+"-"+str(i))

