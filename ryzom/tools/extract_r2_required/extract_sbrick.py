
import os, re

sbrickPath = "R:\\leveldesign\\game_element\\sbrick"

familyExpr = r"\"FamilyId\"[^\"]+\"[^\"]+\""
indexExpr = r"\"IndexInFamily\"[^\"]+\"[^\"]+\""
sitemExpr = r"\"[^\"]+.sitem\""

fileMap = {}

def listPathExt(path, ext):
	for p in os.listdir(path):
		fp = path + "\\" + p
		if os.path.isdir(fp):
			listPathExt(fp, ext)
		elif os.path.isfile(fp):
			if fp.lower().endswith(ext):
				fileMap[p] = fp

listPathExt(sbrickPath, ".sbrick")

sbrickMap = {}

def loadTsv(filename):
	table = []
	with open(filename, "r") as f:
		for l in f:
			table += [ l.strip().split("\t") ]
	return table;

preserveIndex = True
if preserveIndex and os.path.isfile("sbrick_index.tsv"):
	table = loadTsv("sbrick_index.tsv")
	for entry in table:
		e = filter(None, entry)
		entryName = e[0] + str(int(e[1], 10)).zfill(4) # + name
		if "__missing" in e:
			e.remove("__missing")
		sbrickMap[entryName] = e + [ "__missing" ]

for sbrick in fileMap:
	contents = ""
	name = sbrick.split(".")[0].lower()
	with open(fileMap[sbrick], "r") as f:
		contents = f.read()
	familyId = re.findall(familyExpr, contents)[0].split('"')[-2]
	indexInFamily = re.findall(indexExpr, contents)
	if len(indexInFamily) > 0:
		indexInFamily = indexInFamily[0].split('"')[-2]
	else:
		indexInFamily = ""
	indexNumeric = len(indexInFamily) > 0 and indexInFamily.isdigit()
	if not indexNumeric and len(indexInFamily) > 0 and indexInFamily != "$filename":
		print(name + ", IndexInFamily: " + indexInFamily)
	if not indexNumeric and name.startswith(familyId.lower()):
		indexInFamily = name[len(familyId):]
		indexNumeric = len(indexInFamily) > 0 and indexInFamily.isdigit()
	if not indexNumeric:
		if name[-4:].isdigit():
			indexInFamily = name[-4:]
		elif name[-3:].isdigit():
			indexInFamily = name[-3:]
		elif name[-2:].isdigit():
			indexInFamily = name[-2:]
		indexNumeric = len(indexInFamily) > 0 and indexInFamily.isdigit()
	if not indexNumeric:
		print(name + ", IndexInFamily: " + indexInFamily)
	sitem = re.findall(sitemExpr, contents)
	if len(sitem) > 0:
		sitem = sitem[0].split('"')[-2].split(".")[0]
	else:
		sitem = ""
	#print(familyId)
	#print(indexInFamily)
	#print(name)
	#print(sitem)
	templateName = familyId.lower() + str(int(indexInFamily, 10)).zfill(2)
	entryName = familyId + str(int(indexInFamily, 10)).zfill(4) # + name
	entry = [ familyId, indexInFamily ]
	if name:
		entry += [ name ]
	if sitem != templateName and sitem != name:
		entry += [ sitem ]
	if entryName in sbrickMap:
		if not "__missing" in sbrickMap[entryName][2:]:
			print("Duplicate sbrick")
			print(entry)
			print(sbrickMap[entryName])
		for name in sbrickMap[entryName][2:]:
			if name != "__missing" and not name in entry:
				entry += [ name ]
	sbrickMap[entryName] = entry

w = open("sbrick_index.tsv", "w")

sbrickKeys = sbrickMap.keys()
sbrickKeys.sort()

for k in sbrickKeys:
	w.write("\t".join(sbrickMap[k]) + "\n")

w.flush()
w.close()
