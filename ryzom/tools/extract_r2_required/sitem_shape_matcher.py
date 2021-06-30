
def loadTsv(filename):
	table = []
	with open(filename, "r") as f:
		for l in f:
			table += [ l.strip().split("\t") ]
	return table;

shapeParsed = loadTsv("shape_parsed.tsv")
sitemParsed = loadTsv("sitem_parsed.tsv")

boostTags = {
	"underwear": 4,
	"vest": 2,
	"gloves": 2,
	"pants": 2,
	"sleeves": 2,
	"helmet": 2,
	"boots": 2,
	"refugee": 2,
#	"hands": 2,
}

unmatchTags = {
#	"tryker": 2,
#	"matis": 2,
#	"zorai": 2,
#	"fyros": 2,
#	"karavan": 2,
#	"kami": 2,
}

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
					if tag in boostTags:
						matches += boostTags[tag]
					else:
						matches += 1
					matching += [ tag ]
				else:
					if tag in unmatchTags:
						unmatches += unmatchTags[tag]
					else:
						unmatches += 1
					unmatching += [ tag ]
		for tag in sitem:
			if tag not in matched:
				if tag in unmatchTags:
					unmatches += unmatchTags[tag]
				else:
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
		if not matches and femaleShape[0].endswith("_hof_underwear_gilet") and maleShape[0] == "tr_hom_underwear_gilet":
			matches = True
		if "ma_hof_armor04_" in femaleShape[0]:
			print(femaleShape[0])
			maleShape[0] = femaleShape[0].replace("ma_hof_armor04_", "ge_hom_armor06_")
			matches = True # Temporary
		if matches:
			f.write(sitem[0] + "\t" + maleShape[0] + "\t" + femaleShape[0])
			for tag in femaleShape[1:]:
				if tag != "female":
					f.write("\t" + tag)
			f.write("\n")
		else:
			f.write(sitem[0] + "\t\t")
			for tag in maleShape:
				f.write("\t" + tag)
			for tag in femaleShape:
				f.write("\t" + tag)
			f.write("\n")
