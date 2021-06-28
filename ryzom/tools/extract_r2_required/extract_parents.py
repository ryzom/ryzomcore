
import os, re

sitemPath = "R:\\leveldesign\\game_element"
creaturePath = "R:\\leveldesign\\game_elem"

fileMap = {}

def listPath(path):
	for p in os.listdir(path):
		fp = path + "\\" + p
		if os.path.isdir(fp):
			listPath(fp)
		elif os.path.isfile(fp):
			fileMap[p] = fp

listPath(sitemPath)
listPath(creaturePath)

sitemExpr = r"[^\" ]+.sitem"
creatureExpr = r"[^\" ]+.creature"

missing = {}

sitemProcessed = {}
sitemParents = {}

def processSitemSheet(name):
	if not name in sitemProcessed:
		if not name in fileMap:
			missing[name] = True
		else:
			f = open(fileMap[name], "r")
			sheet = f.read()
			f.close()
			sitemProcessed[name] = True
			matches = re.findall(sitemExpr, sheet)
			for n in matches:
				sitemParents[n] = True
				processSitemSheet(n)

with open("sitem_list.txt", "r") as f:
	for l in f:
		lt = l.strip()
		processSitemSheet(lt)

sitemParentsSorted = list(dict.fromkeys(sitemParents))
sitemParentsSorted.sort()
lf = open("sitem_parents.txt", "w")
lf.flush()
for k in sitemParentsSorted:
	lf.write(k + "\n")
lf.close()

creatureProcessed = {}
creatureParents = {}

def processcreatureSheet(name):
	if not name in creatureProcessed:
		if not name in fileMap:
			if not "$hands" in name and not "$level" in name and not "palette." in name and "." in name:
				missing[name] = True
		else:
			f = open(fileMap[name], "r")
			sheet = f.read()
			f.close()
			creatureProcessed[name] = True
			matches = re.findall(creatureExpr, sheet)
			for n in matches:
				creatureParents[n] = True
				processcreatureSheet(n)

with open("creature_list.txt", "r") as f:
	for l in f:
		lt = l.strip()
		processcreatureSheet(lt)

creatureParentsSorted = list(dict.fromkeys(creatureParents))
creatureParentsSorted.sort()
lf = open("creature_parents.txt", "w")
lf.flush()
for k in creatureParentsSorted:
	lf.write(k + "\n")
lf.close()

missingSorted = list(dict.fromkeys(missing))
missingSorted.sort()
lf = open("missing_sheets.txt", "w")
lf.flush()
for k in missingSorted:
	lf.write(k + "\n")
lf.close()

