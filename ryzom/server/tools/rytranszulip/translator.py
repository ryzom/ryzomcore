#!/bin/env python3
#############################################
#  _______________________________
#  \______   \__    ___/\____    /
#   |       _/ |    |     /     /
#   |    |   \ |    |    /     /_
#   |____|_  / |____|   /_______ \
#          \/                   \/
#
# RyTransZulip - with delicious M.A.R.G.U.E.Z
# M.A.R.G.U.E.Z (Make Awesome all Ryzom's Gossips with the Unreasonable Empowerment of Zulip... and a touch of Deepl :D)
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# -== Ryzom Translator ==-
#
# This script is look in memcached server if a new message require translations, send it to DeepL and put it back into memcached server
#


import os
import re
import sys
import json
import deepl

from time import sleep, time
from ryzom_service import RyzomService, RyzomMessage, printer

class Translator(RyzomService):

	def __init__(self):
		super().__init__()
		self.name = "Translator"
		self.version = "1.2"
		self.scriptfile = __file__
		self.log_sections["messages"] = ("db", "Ryzom-Chat-LastID", "Ryzom-Chat-{}", "")
		self.dst_lang = sys.argv[1]
		self.stats = {"messages": 0, "translated_messages": 0}
		self.last_deep_error = ""
		self.updateStats()

	def updateStats(self):
		messages = self.stats["messages"]
		translated_messages = self.stats["translated_messages"]
		self.infos = f"[bright_green]Messages checked: [bright_yellow]{messages}  \t[bright_green]Translated Messages: [bright_yellow]{translated_messages}\n"
		self.infos += f"[bright_green]Last Deepl Error: [orange1]{self.last_deep_error}"
		self.updateInfos()

	def escapeImages(self, text):
		pattern = re.compile(r"!\[[^\]]*]\([^)]+\)")
		return pattern.sub(lambda m: "<x>"+m.group(0)+"</x>", text)

	def escapeQuotes(self, text, level):
		if level <= 2:
			return text
		stext = text.split("`"*level)
		if len(stext) == 1:
			return self.escapeQuotes(text, level-1)
		final_text = ""
		i = 0
		for e in stext:
			if i % 2 == 0:
				if i == len(stext)-1:
					final_text += self.escapeQuotes(e, level-1)
				else:
					final_text += self.escapeQuotes(e, level-1)+"<x>"+"`"*level
			else:
				final_text += e+"`"*level+"</x>"
			i += 1
		return final_text

	def translateWithDeepl(self, m):
		client = deepl.DeepLClient(self.config["deepl"]["auth_key"])
		text = self.escapeImages(m.text)
		text = self.escapeQuotes(text, 8)
		dst_lang = self.dst_lang.upper()
		try:
			result = client.translate_text(
				text,
				source_lang =  m.source_lang,
				target_lang = "EN-GB" if dst_lang == "EN" else dst_lang,
				model_type = "quality_optimized",
				tag_handling = "xml",
				ignore_tags = "x",
				)
		except deepl.DeepLException as e:
			self.last_deep_error = repr(e)
			print("DeepL Error..."+repr(e))
			return None
		final_text = result.text.replace("<x>", "").replace("</x>", "")
		return (final_text,  result.billed_characters)


	def checkMessages(self):
		last_id = self.getLastChatID()
		for i in range(self.current_id+1, last_id+1, 1):
			message = self.getRyzomMessage(i)
			if message != None:
				self.stats["messages"] += 1
				if message.translated_lang == "WK" and message.channel != "player":
					if message.source_lang != self.dst_lang:
						if message.langs == "*" or self.dst_lang in message.langs.split("-"):
							translation, billed_characters = self.translateWithDeepl(message)
							if translation:
								self.stats["translated_messages"] += 1
							status = "✅" if translation else "🛑"
							#print("✅ "+str(i), repr(message.text), "➡️ ", translation)
							print(f"DEEPL|{message.channel}|{billed_characters}")
							print(f"RYZOM_DEBUG translated Ryzom-Chat-{i} into {self.dst_lang}: source_message_id={message.source_message_id} translation={translation!r}")
							message.translation = translation
							message.translated_lang = self.dst_lang.upper()
							message.source_message_id = i
							self.addRyzomMessage(message)
						#else:
							#print("🚫 "+str(i), repr(message.text))
					#else:
						#print("🔲 "+str(i), repr(message.text))
				self.next_id = i
		self.current_id = self.next_id


	def run(self):
		self.current_id = self.getLastChatID()
		self.next_id = self.current_id
		print("Translating messages")
		while True:
			self.checkMessages()
			sleep(0.1)

if __name__ == "__main__":
	translator = Translator()
	translator.register()
	printer.out_file = "."+sys.argv[0].split("/")[-1].split(".")[0]+"_"+sys.argv[1]+".out"
	printer.setStdOut()
	translator.run()


