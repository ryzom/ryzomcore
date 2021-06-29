
import os, re

def loadTsv(filename):
	table = []
	with open(filename, "r") as f:
		for l in f:
			table += [ l.strip().split("\t") ]
	return table;

sbrickIndex = loadTsv("sbrick_index.tsv")
sbrickRename = {}
sbrickRename2 = {}

for sbrick in sbrickIndex:
	name = sbrick[2]
	oldNames = filter(None, sbrick[3:])
	for n in oldNames:
		sbrickRename["\\b" + n + "\\b"] = name
		sbrickRename["\\ba" + n + "\\b"] = "a" + name
		sbrickRename2["\\b" + n + "\\.sbrick\\b"] = name + ".sbrick"
		sbrickRename2["\\ba" + n + "\\.sphrase\\b"] = "a" + name + ".sphrase"

#print(sbrickRename)

def fixFile(file):
	print(file)
	text = None
	with open(file, "r") as f:
		text = f.read()
	for oldName in sbrickRename:
		newName = sbrickRename[oldName]
		text = re.sub(oldName, newName, text)
	with open(file, "w") as f:
		f.write(text)
		f.flush()

def fixDir(path):
	for p in os.listdir(path):
		fp = path + "\\" + p
		if os.path.isdir(fp):
			fixDir(fp)
		elif os.path.isfile(fp):
			fixFile(fp)

def fixFile2(file):
	print(file)
	text = None
	with open(file, "r") as f:
		text = f.read()
	for oldName in sbrickRename2:
		newName = sbrickRename2[oldName]
		text = re.sub(oldName, newName, text)
	with open(file, "w") as f:
		f.write(text)
		f.flush()

def fixDir2(path):
	for p in os.listdir(path):
		fp = path + "\\" + p
		if os.path.isdir(fp):
			fixDir2(fp)
		elif os.path.isfile(fp):
			fixFile2(fp)

fixFile(r"R:\leveldesign\translation\work\sbrick_words_wk.txt")
fixFile(r"R:\leveldesign\translation\work\sphrase_words_wk.txt")

fixFile(r"R:\leveldesign\translation\translated\sbrick_words_de.txt")
fixFile(r"R:\leveldesign\translation\translated\sbrick_words_en.txt")
fixFile(r"R:\leveldesign\translation\translated\sbrick_words_es.txt")
fixFile(r"R:\leveldesign\translation\translated\sbrick_words_fr.txt")
fixFile(r"R:\leveldesign\translation\translated\sbrick_words_ru.txt")
fixFile(r"R:\leveldesign\translation\translated\sbrick_words_wk.txt")

fixFile(r"R:\leveldesign\translation\translated\sphrase_words_de.txt")
fixFile(r"R:\leveldesign\translation\translated\sphrase_words_en.txt")
fixFile(r"R:\leveldesign\translation\translated\sphrase_words_es.txt")
fixFile(r"R:\leveldesign\translation\translated\sphrase_words_fr.txt")
fixFile(r"R:\leveldesign\translation\translated\sphrase_words_ru.txt")
fixFile(r"R:\leveldesign\translation\translated\sphrase_words_wk.txt")

fixDir2(r"R:\leveldesign\game_elem\createperso")
