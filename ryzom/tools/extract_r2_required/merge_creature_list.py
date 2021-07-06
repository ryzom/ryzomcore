
import os

sitemPath = "R:\\leveldesign\\game_elem\\creature"

fileMap = {}

def listPathExt(path, ext):
	for p in os.listdir(path):
		fp = path + "\\" + p
		if os.path.isdir(fp):
			listPathExt(fp, ext)
		elif os.path.isfile(fp):
			if fp.lower().endswith(ext):
				fileMap[p] = fp

listPathExt(sitemPath, ".creature")

knownMissing = {}
with open("missing_sheets.txt", "r") as r:
	for l in r:
		knownMissing[l.strip()] = True

with open("creature_list.txt", "w") as w:
	with open("creature_missing.txt", "w") as wm:
		with open("creature_list_r2.txt", "r") as r:
			for l in r:
				if not l.strip() in knownMissing:
					w.write(l)
					if not l.strip() in fileMap:
						wm.write(l)
		with open("creature_list_wk.txt", "r") as r:
			for l in r:
				if len(l.strip()) > 0 and not l.startswith(";"):
					if not l.strip() in knownMissing:
						w.write(l)
						if not l.strip() in fileMap:
							wm.write(l)
		wm.flush()
	w.flush()
