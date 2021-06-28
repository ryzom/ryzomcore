
def loadTsv(filename):
	table = []
	with open(filename, "r") as f:
		for l in f:
			table += [ l.strip().split("\t") ]
	return table;

shapeParsed = loadTsv("shape_parsed.tsv")
sitemParsed = loadTsv("sitem_parsed.tsv")

def findMatch(name, sitem):
	mostMatches = 0
	leastUnmatches = 0
	bestMatching = ""
	bestMatchingTags = []
	bestUnmatchingTags = []
	if "caster" in sitem and not "pants" in sitem and "light" in sitem:
		sitem.remove("caster")
	for shape in shapeParsed:
		matches = 0
		unmatches = 0
		matched = {}
		matching = []
		unmatching = []
		for tag in shape[2:]:
			if tag not in matched:
				matched[tag] = True
				if tag in sitem:
					matches += 1
					matching += [ tag ]
				else:
					unmatches += 1
					unmatching += [ tag ]
		for tag in sitem:
			if tag not in matched:
				unmatches += 1
				unmatching += [ tag ]
		if matches > mostMatches:
			mostMatches = matches
			leastUnmatches = unmatches
			bestMatching = shape[0]
			bestMatchingTags = matching
			bestUnmatchingTags = unmatching
		elif matches == mostMatches and unmatches < leastUnmatches:
			leastUnmatches = unmatches
			bestMatching = shape[0]
			bestMatchingTags = matching
			bestUnmatchingTags = unmatching
	# print(bestMatchingTags)
	if name == "icfalg":
		print("matching: ")
		print(bestMatchingTags)
		print("unmatching: ")
		print(bestUnmatchingTags)
	return [ bestMatching ] + bestMatchingTags

with open("match_sitem_shape.tsv", "w") as f:
	for sitem in sitemParsed:
		maleShape = findMatch(sitem[0], sitem[2:] + [ "male" ])
		femaleShape = findMatch(sitem[0], sitem[2:] + [ "female" ])
		maleTags = maleShape[1:]
		femaleTags = femaleShape[1:]
		matches = True
		for tag in femaleTags:
			if not tag in maleTags and tag != "female":
				matches = False
				# print(tag)
		for tag in maleTags:
			if not tag in femaleTags and tag != "male":
				matches = False
				# print(tag)
		if matches:
			f.write(sitem[0] + "\t" + maleShape[0] + "\t" + femaleShape[0])
			for tag in maleShape[1:]:
				if tag != "male":
					f.write("\t" + tag)
			f.write("\n")
		else:
			f.write(sitem[0] + "\t\t")
			for tag in maleShape:
				f.write("\t" + tag)
			for tag in femaleShape:
				f.write("\t" + tag)
			f.write("\n")
