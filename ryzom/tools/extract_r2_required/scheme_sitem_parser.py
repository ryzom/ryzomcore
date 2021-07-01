
# base scheme
# see item_words_en.txt
scheme = {
	"": [
		{
			"i": "item",
			"b": "brick",
			"_c": "parent shared",
			"_g": "parent generic",
		},
		{
			# family
			"f": "fyros",
			"m": "matis",
			"m2": "matis two",
			"t": "tryker",
			"z": "zorai",
			"c": "common",
			"_": "unspecified",
			"r": "refugee",
			"kam": "kami",
			"kar": "karavan",
			"b": "tribe",
			"hb": "high tribe",
		},
		{
			# item type
			"a": "armor",
			"a_h": "heavy armor base",
			"a_m": "medium armor base",
			"a_l": "light armor base",
			"a_c": "caster armor base",
			"auw": "light underwear armor",
			"aus": "light sexy underwear armor",
			"ar": "light refugee armor",
			"ah": "heavy armor",
			"am": "medium armor",
			"al": "light armor",
			"ac": "light caster armor",
			"ah2": "second heavy armor",
			"am2": "second medium armor",
			"al2": "second light armor",
			"ac2": "second light caster armor",
			"ah3": "third heavy armor",
			"am3": "third medium armor",
			"al3": "third light armor",
			"ac3": "third light caster armor",
			"ah4": "fourth heavy armor",
			"am4": "fourth medium armor",
			"al4": "fourth light armor",
			"ac4": "fourth light caster armor",
			"s": "shield",
			"m": "melee",
			"r": "ranged",
			"p": "ranged ammo",
			"j": "jewelry",
			"sb": "buckler shield",
			"ss": "large shield",
		},
		{
			"_2": "mq",
			"_3": "hq",
			"e": "electric",
			"b": "burning",
			"w": "waving",
			"l": "living",
		},
		{
			"_2": "mq",
			"_3": "hq",
			"e": "electric",
			"b": "burning",
			"w": "waving",
			"l": "living",
		},
		{
			"_2": "mq",
			"_3": "hq",
			"e": "electric advantage",
			"b": "burning advantage",
			"w": "waving advantage",
			"l": "living advantage",
		},
		{},
	],
	"melee": [
		{},
		{},
		{},
		{
			"1": "one-handed",
			"2": "two-handed",
		},
		{
			# "b": "smashing",
			# "p": "piercing",
			# "s": "slashing",
			"bm": "blunt mace",
			"bs": "blunt staff",
			"pd": "piercing dagger",
			"ps": "piercing spear",
			"pp": "piercing pike",
			"sa": "slashing axe",
			"ss": "slashing sword",
			"ms": "magic amplifier gloves",
		},
		{},
		{},
		{},
		{},
		{},
	],
	"ranged": [
		{},
		{},
		{},
		{
			"1b": "one-handed bowpistol",
			"1p": "one-handed pistol",
			"2a": "two-handed autolauncher",
			"2b": "two-handed bowrifle",
			"2h": "two-handed harpoon",
			"2l": "two-handed launcher",
			"2r": "two-handed rifle",
		},
		{
			"b": "smashing",
			"p": "piercing",
			"s": "slashing",
		},
		{},
		{},
		{},
		{},
		{},
	],
	"armor": [
		{},
		{},
		{},
		{
			"b": "boots",
			"g": "gloves hands",
			"p": "pants",
			"s": "sleeves",
			"v": "vest",
			"h": "helmet",
		},
		{
			"a": "color black",
			"e": "color beige",
			"g": "color green",
			"r": "color red",
			"t": "color turquoise",
			"u": "color blue",
			"v": "color violet",
			"w": "color white",
		},
		{},
		{},
		{},
	],
	"jewelry": [
		{},
		{},
		{},
		{
			"a": "anklet",
			"b": "bracelet",
			"d": "diadem",
			"e": "earring",
			"p": "pendant",
			"r": "ring",
		},
		{},
		{},
		{},
		{},
		{},
		{},
	],
}

# c crafted
for k in scheme[""][1].copy():
	scheme[""][1]["c" + k] = "crafted " + scheme[""][1][k]

def addTags(sub, depth, tags):
	for i in range(len(tags) - 1, -1, -1):
		tag = tags[i]
		if tag in scheme:
			s = scheme[tag]
			largest = 0
			for k in s[depth]:
				if len(k) >= largest and sub.startswith(k):
					largest = len(k)
			if largest > 0:
				for k in s[depth]:
					if len(k) >= largest and sub.startswith(k):
						tags += s[depth][k].split(" ")
						return k
	return ""

def parse(name):
	tags = [ "" ]
	sub = name
	depth = 0
	while len(sub) > 0:
		add = addTags(sub, depth, tags)
		if len(add) == 0:
			return tags[1:] + [ "incomplete", "_" + sub ]
		sub = sub[len(add):]
		depth += 1
	return tags[1:]

def generate(tags):
	ref = [ "" ] + tags
	found = {}
	name = ""
	depth = 0
	while True:
		all = True
		for t in tags:
			if not t in found:
				all = False
				break
		if all:
			break
		any = False
		for i in range(len(ref) - 1, -1, -1):
			tag = ref[i]
			if tag in scheme:
				s = scheme[tag]
				largest = 0
				least = 999
				for k in s[depth]:
					count = 0
					count2 = 0
					st = s[depth][k].split(" ")
					for t in st:
						if t in ref and not t in found:
							count += 1
						if t in ref and t in found:
							count2 += 1
						if t not in ref:
							count2 += 2
					if count > largest:
						largest = count
						least = count2
					if count == largest and count2 < least:
						least = count2
				if largest > 0:
					for k in s[depth]:
						count = 0
						count2 = 0
						st = s[depth][k].split(" ")
						for t in st:
							if t in ref and not t in found:
								count += 1
							if t in ref and t in found:
								count2 += 1
							if t not in ref:
								count2 += 2
						if count >= largest and count2 <= least:
							for t in st:
								if t in ref and not t in found:
									#print("tag:" + t)
									found[t] = True
							#print("name: " + k)
							any = True
							name += k
							break
			if any:
				break
		if not any:
			return name + "_incomplete"
		depth += 1
	return name

# print("ictr2r")
# print(parse("ictr2r"))
# 
# print("iczp2ap")
# print(parse("iczp2ap"))
# 
# print("iczr2a")
# print(parse("iczr2a"))
# 
# print("ictm1pdw")
# print(parse("ictm1pdw"))

missing = {}
with open("missing_sheets.txt", "r") as f:
	for l in f:
		missing[l] = True

def process(f, fw):
	for l in f:
		if not l in missing:
			name = l.strip().split(".")[0]
			tags = parse(name)
			gen = generate(tags)
			if gen != name:
				tags += [ "invalid" ]
			fw.write(name + "\t" + gen)
			for t in tags:
				fw.write("\t" + t)
			fw.write("\n")
			# if "incomplete" in tags:
			# print(name + " -> " + gen)
			# print(tags)
	fw.flush()

with open("sitem_list.txt", "r") as f:
	with open("sitem_parsed.tsv", "w") as fw:
		process(f, fw)

with open("sitem_parents.txt", "r") as f:
	with open("sitem_parents.tsv", "w") as fw:
		process(f, fw)
