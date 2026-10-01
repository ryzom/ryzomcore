
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

class ZulipClient(zulip.Client):
	def doRegister(self, event_types, narrow, **kwargs):
		while True:
			if event_types is None:
				res = self.register(None, None, **kwargs)
			else:
				res = self.register(event_types, narrow, **kwargs)
			if not isinstance(res, dict) or "error" in res.get("result", "error") or "queue_id" not in res or "last_event_id" not in res:
				if self.verbose:
					print("Server returned error:\n{}".format(res.get("msg", "") if isinstance(res, dict) else res))
				time.sleep(1)
			else:
				self.queue_id = res["queue_id"]
				self.last_event_id = res["last_event_id"]
				return self.queue_id

	def call(self, callback, event_types, narrow, **kwargs):
		if narrow is None:
			narrow = []

		# Make long-polling requests with `get_events`. Once a request
		# has received an answer, pass it to the callback and before
		# making a new long-polling request.
		while True:
			try:
				res = self.get_events(queue_id=self.queue_id, last_event_id=self.last_event_id)
			except (
				requests.exceptions.Timeout,
				requests.exceptions.SSLError,
				requests.exceptions.ConnectionError,
			):
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

			if not isinstance(res, dict):
				print(f"Unexpected get_events response: {res!r}")
				time.sleep(1)
				continue

			if "error" in res.get("result", "error"):
				if res.get("result") == "http-error":
					if self.verbose:
						print("HTTP error fetching events -- probably a server restart")
				else:
					if self.verbose:
						print("Server returned error:\n{}".format(res.get("msg", "")))
					# Eventually, we'll only want the
					# BAD_EVENT_QUEUE_ID check, but we check for the
					# old string to support legacy Zulip servers.  We
					# should remove that legacy check in 2019.
					if res.get("code") == "BAD_EVENT_QUEUE_ID" or str(res.get("msg", "")).startswith(
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
						# Reset queue_id to register a new event queue.
						self.queue_id = None
				# Add a pause here to cover against potential bugs in this library
				# causing a DoS attack against a server when getting errors.
				# TODO: Make this back off exponentially.
				time.sleep(1)
				continue

			for event in res.get("events", []):
				if not isinstance(event, dict):
					print(f"Invalid event: {event!r}")
					continue
				try:
					self.last_event_id = max(self.last_event_id, int(event.get("id", -1)))
				except (TypeError, ValueError):
					print(f"Invalid event id: {event!r}")

				if event.get("type") == "heartbeat":
					# Heartbeat events are sent to clients regardless
					# of the client's requested event types, and are
					# intended to be an internal part of the Zulip
					# longpolling protocol, not something that clients
					# need to handle.
					continue
				# A bad event must never kill the long-polling loop
				try:
					callback(event)
				except Exception:
					print(f"Error handling event {event.get('id')}:\n{traceback.format_exc()}")

	def registerMessages(self, **kwargs):
		self.doRegister(["message", "update_message"], None, **kwargs)

	def manageMessages(self, callback, **kwargs):
		def event_callback(event):
			if event.get("type") in ("message", "update_message"):
				callback(event)
		self.call(event_callback, ["message", "update_message"], None, **kwargs)


class ZulipService(RyzomService):

	def __init__(self):
		super().__init__()
		while True:
			try:
				self.zulip = ZulipClient(config_file=".zuliprc")#email=self.config["zulip"]["email"], api_key=self.config["zulip"]["key"], site=self.base_url)
			except Exception as e:
				print("Error Zulip server", self.config["zulip"]["site"])
				print(e)
				print("Retrying...")
				time.sleep(1)
			else:
				break
		print("Connected! to", self.base_url, "!")


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

