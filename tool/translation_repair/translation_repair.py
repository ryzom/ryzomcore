#
# Copyright (C) 2019-2020  Jan BOON <jan.boon@kaetemi.be>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# This script attempts to repair bad hashes in translation files

import os

path = "R:\\leveldesign\\translation"

def processUxt(wk, tl):
	print(tl)
	inComment = 0
	inString = 0
	lastIndex = ""
	lastHash = ""
	wkMap = {}
	with open(wk, "r") as f:
		for l in f:
			if l.startswith("\ufeff"):
				l = l[1:]
			if not inString and not inComment:
				s = l.split()
				if len(s) > 0:
					if not s[0].startswith("[") and not s[0].startswith("//") and not s[0].startswith("/*"):
						wkMap[s[0]] = { "index": lastIndex, "hash": lastHash }
						# print(s[0] + ", " + lastIndex + ", " + lastHash)
			if l.startswith("// HASH_VALUE") or l.startswith("// INDEX"):
				inString = 0
				inComment = 0
				if l.startswith("// HASH_VALUE"):
					lastHash = l.rstrip()
				if l.startswith("// INDEX"):
					lastIndex = l.rstrip()
				continue
			inString = inString + l.count("[") - l.count("\\[") - l.count("]") + l.count("\\]")
			inComment = inComment + l.count("/*") - l.count("*/")
	inComment = 0
	inString = 0
	reIndex = {}
	atIndex = 0
	with open(tl, "r") as fr:
		with open(tl + ".new", "w") as fw:
			fw.write("\ufeff")
			for l in fr:
				if l.startswith("\ufeff"):
					l = l[1:]
				if not inString and not inComment:
					s = l.split()
					if len(s) > 0:
						if not s[0].startswith("[") and not s[0].startswith("//") and not s[0].startswith("/*") and s[0] in wkMap:
							if not wkMap[s[0]]["index"] in reIndex:
								reIndex[wkMap[s[0]]["index"]] = "// INDEX " + str(atIndex)
								atIndex = atIndex + 1
							fw.write(reIndex[wkMap[s[0]]["index"]] + "\n")
							fw.write(wkMap[s[0]]["hash"] + "\n")
				if l.startswith("// HASH_VALUE") or l.startswith("// INDEX"):
					inString = 0
					inComment = 0
					continue
				fw.write(l)
				inString = inString + l.count("[") - l.count("\\[") - l.count("]") + l.count("\\]")
				inComment = inComment + l.count("/*") - l.count("*/")
	os.replace(tl + ".new", tl)

#processUxt(path + "\\translated\\wk.uxt", path + "\\translated\\en.uxt")
#processUxt(path + "\\translated\\wk.uxt", path + "\\translated\\es.uxt")
#processUxt(path + "\\translated\\wk.uxt", path + "\\translated\\fr.uxt")
#processUxt(path + "\\translated\\wk.uxt", path + "\\translated\\ru.uxt")
#processUxt(path + "\\translated\\wk.uxt", path + "\\translated\\de.uxt")

#processUxt(path + "\\translated\\r2_wk.uxt", path + "\\translated\\r2_en.uxt")
#processUxt(path + "\\translated\\r2_wk.uxt", path + "\\translated\\r2_es.uxt")
#processUxt(path + "\\translated\\r2_wk.uxt", path + "\\translated\\r2_fr.uxt")
#processUxt(path + "\\translated\\r2_wk.uxt", path + "\\translated\\r2_ru.uxt")
#processUxt(path + "\\translated\\r2_wk.uxt", path + "\\translated\\r2_de.uxt")

#processUxt(path + "\\translated\\clause_wk.txt", path + "\\translated\\clause_en.txt")
#processUxt(path + "\\translated\\clause_wk.txt", path + "\\translated\\clause_es.txt")
#processUxt(path + "\\translated\\clause_wk.txt", path + "\\translated\\clause_fr.txt")
#processUxt(path + "\\translated\\clause_wk.txt", path + "\\translated\\clause_ru.txt")
#processUxt(path + "\\translated\\clause_wk.txt", path + "\\translated\\clause_de.txt")

def processPhrase(wk, tl):
	print(tl)
	inComment = 0
	inString = 0
	inPhrase = 0
	lastHash = ""
	wkMap = {}
	with open(wk, "r") as f:
		for l in f:
			if l.startswith("\ufeff"):
				l = l[1:]
			if not inString and not inComment:
				s = l.split()
				if len(s) > 0:
					if not s[0].startswith("[") and not s[0].startswith("//") and not s[0].startswith("/*") and not s[0].startswith("{") and len(lastHash) > 0:
						wkMap[s[0]] = { "hash": lastHash }
						lastHash = ""
						# print(s[0] + ", " + lastIndex + ", " + lastHash)
			if l.startswith("// HASH_VALUE"):
				inString = 0
				inComment = 0
				inPhrase = 0
				lastHash = l.rstrip()
				continue
			inString = inString + l.count("[") - l.count("\\[") - l.count("]") + l.count("\\]")
			inPhrase = inString + l.count("{") - l.count("\\{") - l.count("}") + l.count("\\}")
			inComment = inComment + l.count("/*") - l.count("*/")
	inComment = 0
	inString = 0
	inPhrase = 0
	with open(tl, "r") as fr:
		with open(tl + ".new", "w") as fw:
			fw.write("\ufeff")
			for l in fr:
				if l.startswith("\ufeff"):
					l = l[1:]
				if not inString and not inComment:
					s = l.split()
					if len(s) > 0:
						if not s[0].startswith("[") and not s[0].startswith("//") and not s[0].startswith("/*") and not s[0].startswith("{") and s[0] in wkMap:
							fw.write(wkMap[s[0]]["hash"] + "\n")
				if l.startswith("// HASH_VALUE"):
					inString = 0
					inComment = 0
					inPhrase = 0
					continue
				fw.write(l)
				inString = inString + l.count("[") - l.count("\\[") - l.count("]") + l.count("\\]")
				inPhrase = inString + l.count("{") - l.count("\\{") - l.count("}") + l.count("\\}")
				inComment = inComment + l.count("/*") - l.count("*/")
	os.replace(tl + ".new", tl)

##processPhrase(path + "\\translated\\phrase_en.txt", path + "\\translated\\phrase_en.txt")
#processPhrase(path + "\\translated\\phrase_en.txt", path + "\\translated\\phrase_es.txt")
#processPhrase(path + "\\translated\\phrase_en.txt", path + "\\translated\\phrase_fr.txt")
#processPhrase(path + "\\translated\\phrase_en.txt", path + "\\translated\\phrase_ru.txt")
#processPhrase(path + "\\translated\\phrase_en.txt", path + "\\translated\\phrase_de.txt")
#processPhrase(path + "\\translated\\phrase_en.txt", path + "\\translated\\phrase_pt.txt")

def processWords(ck, wk, tl):
	if not os.path.isfile(ck):
		return
	print(tl)
	wkMap = { }
	with open(wk, "r") as f:
		for l in f:
			if l.startswith("\ufeff"):
				l = l[1:]
			s = l.split("\t")
			if len(s) > 1 and len(s[0]) >= 16:
				wkMap[s[1]] = s[0]
	with open(tl, "r") as fr:
		with open(tl + ".new", "w") as fw:
			fw.write("\ufeff")
			for l in fr:
				if l.startswith("\ufeff"):
					l = l[1:]
				s = l.split("\t")
				if len(s) > 1:
					if s[1] in wkMap:
						h = wkMap[s[1]]
						l = h + l[len(s[0]):]
					elif s[0] != "*HASH_VALUE":
						l = "0000000000000000" + l[len(s[0]):]
				fw.write(l)
	os.replace(tl + ".new", tl)


def processWordsDiff(wk, tl):
	if not os.path.isfile(wk):
		return
	print(tl)
	wkMap = { }
	with open(wk, "r") as f:
		for l in f:
			if l.startswith("\ufeff"):
				l = l[1:]
			s = l.split("\t")
			if len(s) > 2 and len(s[1]) >= 16:
				wkMap[s[2]] = s[1]
	with open(tl, "r") as fr:
		with open(tl + ".new", "w") as fw:
			fw.write("\ufeff")
			for l in fr:
				if l.startswith("\ufeff"):
					l = l[1:]
				s = l.split("\t")
				if len(s) > 1:
					if s[1] in wkMap:
						h = wkMap[s[1]]
						l = h + l[len(s[0]):]
					elif s[0] != "*HASH_VALUE" and len(s[0]) < 16:
						l = "_0000000000000000" + l[len(s[0]):]
				fw.write(l)
	os.replace(tl + ".new", tl)

def processWordsAll(name):
	#processWords(path + "\\translated\\" + name + "_wk.txt", path + "\\translated\\" + name + "_en.txt")
	processWordsDiff(path + "\\diff\\" + name + "_en_diff_60C8831C.txt", path + "\\translated\\" + name + "_en.txt")
	processWords(path + "\\diff\\" + name + "_es_diff_60C8831C.txt", path + "\\translated\\" + name + "_en.txt", path + "\\translated\\" + name + "_es.txt")
	processWords(path + "\\diff\\" + name + "_fr_diff_60C8831C.txt", path + "\\translated\\" + name + "_en.txt", path + "\\translated\\" + name + "_fr.txt")
	processWords(path + "\\diff\\" + name + "_ru_diff_60C8831C.txt", path + "\\translated\\" + name + "_en.txt", path + "\\translated\\" + name + "_ru.txt")
	processWords(path + "\\diff\\" + name + "_de_diff_60C8831C.txt", path + "\\translated\\" + name + "_en.txt", path + "\\translated\\" + name + "_de.txt")
	#processWordsDiff(path + "\\diff\\" + name + "_en_diff_60C8808A.txt", path + "\\translated\\" + name + "_en.txt")
	#processWordsDiff(path + "\\diff\\" + name + "_es_diff_60C8808A.txt", path + "\\translated\\" + name + "_es.txt")
	#processWordsDiff(path + "\\diff\\" + name + "_fr_diff_60C8808A.txt", path + "\\translated\\" + name + "_fr.txt")
	#processWordsDiff(path + "\\diff\\" + name + "_ru_diff_60C8808A.txt", path + "\\translated\\" + name + "_ru.txt")
	#processWordsDiff(path + "\\diff\\" + name + "_de_diff_60C8808A.txt", path + "\\translated\\" + name + "_de.txt")

processWordsAll("bodypart_words")
processWordsAll("career_words")
processWordsAll("characteristic_words")
processWordsAll("classificationtype_words")
processWordsAll("creature_words")
processWordsAll("damagetype_words")
processWordsAll("ecosystem_words")
processWordsAll("faction_words")
processWordsAll("item_words")
processWordsAll("job_words")
processWordsAll("outpost_words")
processWordsAll("place_words")
processWordsAll("powertype_words")
processWordsAll("race_words")
processWordsAll("sbrick_words")
processWordsAll("score_words")
processWordsAll("skill_words")
processWordsAll("sphrase_words")
processWordsAll("title_words")

#processWordsAll("damage_words")
