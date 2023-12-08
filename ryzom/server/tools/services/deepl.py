#!/bin/env python3

import os, sys, json, time, subprocess
import importlib.util
import urllib.request
import urllib.parse
import pyinotify
import pymongo

from admin_modules_itf import queryShard

mongoclient = pymongo.MongoClient("mongodb://arma.ryzom.com:22110/")
mongodb = mongoclient["megacorp_live"]
ryzom_chats = mongodb["ryzom_chats"]

ALL_LANGS = ["de", "en", "es", "fr", "ru"]

logfilename="/home/nevrax/shard/logs/chat/chat.log"
def getLang(lang):
	return ":"+lang+":"

def getDeepLang(lang):
	return lang.upper()

dirs = os.listdir("/home/nevrax/shard/logs/chat/")

def follow(thefile):
	thefile.seek(0,os.SEEK_END)
	file_size = os.stat(logfilename).st_size
	while True:
		line = thefile.readline()
		if line:
			yield line
		else:
			size = os.stat(logfilename).st_size
			if file_size > size:
				sys.exit()
			file_size = size
			time.sleep(0.01)
		continue

only_lang = sys.argv[1]
shard_domain = "("+sys.argv[2][0].upper()+sys.argv[2][1:]+")"
print("Translating only in "+only_lang.upper()+" from "+shard_domain[1:-1]+"..")

# load all terms
terms = {}
for src in ALL_LANGS:
	if only_lang != src:
		terms[src] = {}
		with open("deepl_terms/"+src+"-"+only_lang+".json") as f:
			content = json.load(f)
			for term in content:
				terms[src][term[0].lower()] = term[1]

logfile = open(logfilename, "r", encoding="utf-8", errors="replace")
loglines = follow(logfile)
for line in loglines:

	sline = line.split(" : ")

	orig = " : ".join(sline[1:]).split("|")
	if len(orig) >= 5:
		print("-------------------------------------------------------")
		print(orig)
		print("---")


		command = "chat"
		prefix = ">"
		domain = shard_domain

		### CHANNEL ###
		channel = orig[0]
		rocket_id = ""
		if channel[0] == "[":
			schannel = channel[1:].split("]")
			rocket_id = schannel[0]
			channel = schannel[1]


		if channel[0] == "#":
			channel = channel[1:]
			is_dynamic = True
		else:
			is_dynamic = False

		if channel in ("FACTION_EN", "FACTION_ES", "FACTION_DE", "FACTION_FR", "FACTION_RU") :
			channel = "universe"

		if channel in ("FACTION_RF-EN", "FACTION_RF-ES", "FACTION_RF-DE", "FACTION_RF-FR", "FACTION_RF-RU") :
			channel = "FACTION_RF"

		if not channel in ("say", "shout", "arround", "universe", "FACTION_RF") and channel[:7] != "region:" and channel[:6] != "guild:" and channel[:5] != "team:" and channel[:8] != "FACTION_" and channel[:7] != "league_":
			print("bye...")
			continue

		print("Channel:", channel)
		print("RocketId:", rocket_id)

		### USER ###
		suser = orig[1].split("$")[0].split("@")
		username =  orig[1].split("$")[0]
		if len(suser) == 2:
			user = suser[1].lower()
		else:
			user = suser[0].lower()
		if "(" in user:
			user = user.split("(")[0]

		if user[0] == "~": # it's a rocket chat message, send it as far message
			user = user[1:]
			command = "farChat"
			domain = ""
			prefix = ""

		print("User:", user)
		print("Command:", command)

		### NBR RECEIVERS ###
		nbr_receivers = orig[2]

		### LANGS ###
		langs = orig[3].split("-")
		source = langs[0]

		if not source in ALL_LANGS:
			source = ""

		print("Source:", source)

		### TEXT ###
		text = "|".join(orig[4:]).strip()
		if not text or text == " ":
			continue

		original_text = text.replace('"', '\'\'')

		print("Text: [", original_text, "]")

		if source == "":
			# User lang factions, send only one test
			if only_lang == "en":
				queryShard("ios", command+" "+user+domain+" "+channel+" \""+prefix+original_text+"\"", False)
		else:

			if channel[:12] != "FACTION_USR_":
				if channel == "arround":
					text = text[5:]

				if len(langs) > 1 and langs[1] == "*":
					langs = [source] + ALL_LANGS

				for lang in langs[1:]:
					if lang != source and lang == only_lang:
						stext = text.split(" ")
						final_text = []
						for word in stext:
							w = word.lower()
							if w in terms[source]:
								final_text.append("<x>"+terms[source][w]+"</x>")
							else:
								final_text.append(word)
						text = " ".join(final_text)

						data = {
						"auth_key": "d8d46b41-5a26-d1d2-90e0-b3ab3e736fe6",
						"tag_handling": "xml",
						"ignore_tags": "x",
						"text": text,
						"target_lang": getDeepLang(lang)
						}

						if source:
							data["source_lang"] = getDeepLang(source)

						data = urllib.parse.urlencode(data)
						data = data.encode("utf-8")

						try:
							with urllib.request.urlopen("https://api.deepl.com/v2/translate", data, timeout=2) as f:
								response = f.read()
								translated = json.loads(response.decode("utf-8"))
								source_lang = translated["translations"][0]["detected_source_language"].lower()
								translated = translated["translations"][0]["text"].replace('"', '\'\'')
						except Exception as e:
							print("DeepL Error...", e)
							translated = text
							source_lang = lang

						translated = translated.replace("<x>", "").replace("</x>", "")

						# UNUSED
						if not source_lang in ("en", "de", "es", "fr", "ru"):
							source_lang = "en"

						print("[%s] => [%s] : %s" % (source_lang, lang, translated))
						print("---")
						print(command+" "+user+domain+" "+channel+" \""+prefix+"{"+getLang(lang)+original_text+"}@{ "+translated+"\"")
						print("")
						if channel == "arround":
							queryShard("ios", command+" "+user+domain+" "+channel+" \""+prefix+getLang(lang)+"&EMT&{"+getLang(source_lang)+original_text+"}@{ "+translated+"\" "+rocket_id, False)
						else:
							queryShard("ios", command+" "+user+domain+" "+channel+" \""+prefix+getLang(lang)+"{"+getLang(source_lang)+original_text+"}@{ "+translated+"\" "+rocket_id, False)

			if source == only_lang and (channel == "universe" or channel[:8] == "FACTION_" or channel[:7] == "league_"):
				queryShard("ios", command+" "+user+domain+" "+channel+" \""+prefix+getLang(source)+original_text+"\" "+rocket_id, False)
			
			if source == only_lang and channel[:6] == "guild:":
				chat = { "_id": rocket_id, "username": username, "chat": getLang(source)+original_text, "chatType": "guildId", "chatId": int(channel[9:19], 16)-268435456, "date": time.time()*1000, "ig": True, "autoSub": 1}
				ryzom_chats.insert_one(chat)

