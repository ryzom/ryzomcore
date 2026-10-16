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
import uuid

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
		marker_prefix = "RTZMENTION"+uuid.uuid4().hex
		dst_lang = self.dst_lang.upper()
		marker_count = 0
		def translatePart(value):
			protected_mentions = []
			def protectMention(match):
				nonlocal marker_count
				marker = marker_prefix+str(marker_count)+"END"
				marker_count += 1
				protected_mentions.append((marker, match.group(2)))
				return match.group(1)+"<x>"+marker+"</x>"
			text = re.sub(r"(^|[ \t\n\"'\[(,:;!?])(@[A-Za-z0-9_.()\-\x80-\U0010ffff]+)",
				protectMention, value)
			text = self.escapeQuotes(self.escapeImages(text), 8)
			result = client.translate_text(
				text,
				source_lang = m.source_lang,
				target_lang = "EN-GB" if dst_lang == "EN" else dst_lang,
				model_type = "quality_optimized",
				tag_handling = "xml",
				ignore_tags = "x",
				)
			translated = result.text.replace("<x>", "").replace("</x>", "")
			if any(translated.count(marker) != 1 for marker, token in protected_mentions):
				self.last_deep_error = "DeepL did not preserve a protected mention"
				return (None, 0)
			for marker, token in protected_mentions:
				translated = translated.replace(marker, token, 1)
			return (translated, result.billed_characters)

		try:
			parts = m.chat.get("parts") if m.chat else None
			if parts:
				if "".join(part[1] for part in parts) != m.text:
					return (None, 0)
				translated_parts = []
				visible_parts = []
				billed = 0
				for kind, value in parts:
					translated, characters = translatePart(value) if kind == "text" and value else ("", 0)
					if translated is None:
						return (None, 0)
					translated_parts.append(translated)
					visible_parts.append(value if kind == "reference" else translated)
					billed += characters
				m.chat["translation_parts"] = translated_parts
				return ("".join(visible_parts), billed)
			return translatePart(m.text)
		except deepl.DeepLException as e:
			self.last_deep_error = repr(e)
			print("DeepL Error..."+repr(e))
			return (None, 0)


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


