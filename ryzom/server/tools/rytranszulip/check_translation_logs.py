import os
import sys
import collections


channels = {}
cont_messages = {}
medium = {}
all_total = 0
for lang in ("fr", "de", "en", "es", "ru"):
	translations = {}
	filename = ".translator_"+lang+".out"
	if not os.path.isfile(filename):
		print(f"{filename} not found, skipped")
		continue
	with open(filename, encoding="utf-8", errors="replace") as f:
		translations = f.read().split("\n")


	total = 0

	start = False
	for tr in translations:
		sline = tr.split(" ", 3)
		if len(sline) > 3:
			sline = sline[3].split("|")
			if len(sline) > 2 and sline[0] == "DEEPL":
				print(sline)
				channel = sline[1].split(" ", 1)
				channel = channel[0]
				try:
					chars = int(sline[2])
				except ValueError:
					continue
				if not channel in channels:
					channels[channel] = chars
					cont_messages[channel] = 1
				else:
					cont_messages[channel] += 1
					channels[channel] += chars
				all_total += chars
				total += 1

for channel in channels.keys():
	medium[channel] = channels[channel] / cont_messages[channel]

print(dict(sorted(channels.items(), key=lambda item: item[1])))
print(dict(sorted(medium.items(), key=lambda item: item[1])))
print(all_total)
