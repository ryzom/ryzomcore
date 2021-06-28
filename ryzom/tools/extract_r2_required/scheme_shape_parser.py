
scheme = {
	"": [
		{
			"ca_": "karavan",
			"ka_": "kami",
			"fy_": "fyros",
			"tr_": "tryker",
			"zo_": "zorai",
			"ma_": "matis",
			"ge_": "common",
			"ge_zo_": "common zorai",
			"ge_ma_": "common matis",
			"ge_tr_": "common tryker",
			"ge_fy_": "common fyros",
		},
		{
			"acc_": "accessory",
			"wea_": "weapon",
			"wea_trib_": "weapon tribe",
			"wea_high_": "weapon high tribe",
			"hom_": "male",
			"hof_": "female",
		},
		{
			"acc": "accessory",
			"armor00": "medium armor medium01",
			"armor01": "heavy armor heavy01",
			"armor04": "heavy armor heavy02",
			"casque01": "heavy armor helmet heavy01",
			"caster01": "light caster armor caster01",
			"civil01": "light armor light01",
			"cheveux": "hairstyle",
			"underwear": "underwear armor",
			"underwear_hand": "underwear armor gloves hands",
			"refugee": "refugee armor",
		},
		{
			"_gauntlet": "magic amplifier",
			"_armpad": "sleeves",
			"_bottes": "boots",
			"_botte": "boots",
			"_gilet": "vest",
			"_torse": "vest",
			"_hand": "gloves",
			"_pantabottes": "pants",
			"_pantabotte": "pants",
			"_casque": "helmet",
			"_lead": "lead event",
			"_fp": "first-person",
			"_b": "b",
			"_c": "c",
			"_d": "d",
			"_e": "e",
			"_f": "f",
			"_g": "g",
			"_h": "h",
		},
		{
			"_armpad": "sleeves",
			"_bottes": "boots",
			"_botte": "boots",
			"_gilet": "vest",
			"_hand": "gloves",
			"_pantabottes": "pants",
			"_pantabotte": "pants",
			"_casque": "helmet",
			"_fp": "first-person",
			"_gen": "generic",
			"_kami": "kami",
			"_b": "b",
			"_c": "c",
			"_d": "d",
			"_e": "e",
			"_f": "f",
			"_g": "g",
			"_h": "h",
			"_02": "02",
		},
		{
			"_fp": "first-person",
		},
		{},
		{},
	],
	"caster": [
		{},
		{},
		{},
		{
			"_armor00": "medium alternative",
			"_armor01": "heavy alternative",
			"_civil01": "light alternative",
			"_civil": "light alternative",
			"_underwear": "underwear alternative",
		},
		{
			"_armor00": "medium alternative",
			"_armor01": "heavy alternative",
			"_civil01": "light alternative",
			"_civil": "light alternative",
			"_underwear": "underwear alternative",
		},
		{},
		{},
		{},
		{},
	],
	"karavan": [
		{},
		{},
		{
			"armor01": "armor armor01", # todo
			"armor02": "armor armor02", # todo
			"casque01": "armor helmet01", # todo
			"casque02": "armor helmet02", # todo
			"casque03": "armor helmet03", # todo
			"casque04": "armor helmet04", # todo
			"commander": "commander",
			"medic": "medic",
			"ingeneer": "engineer",
			"trooper": "trooper",
		},
		{
			"_hum": "human crafted",
			"_gun": "laser gun",
		},
		{},
		{},
	],
	"kami": [
		{},
		{},
		{},
		{},
		{},
	],
	"common": [
		{},
		{},
		{
			"caster00": "tribe boss light caster armor caster01",
			"armor02": "tribe boss light armor light01",
			"armor03": "tribe boss medium armor medium01",
			"armor04": "tribe boss heavy armor heavy01",
			"armor06": "armor heavy heavy03",
			"casque00": "tribe boss heavy armor heavy01 helmet",
			"casque02": "armor heavy heavy03 helmet",
		},
		{},
		{},
		{},
	],
	"hairstyle": [
		{},
		{},
		{},
		{
			"_artistic": "artistic",
			"_basic": "basic",
			"_long": "long",
			"_medium": "medium",
			"_shave": "shave",
			"_short": "short",
			"_style": "style",
			"_lead": "lead event",
		},
		{
			"01": "01",
			"02": "02",
			"03": "03",
			"04": "04",
			"05": "05",
			"06": "06",
			"07": "07",
			"08": "08",
		},
		{},
		{},
		{},
	],
	"accessory": [
		{},
		{},
		{
			"banner": "event banner",
			"baniere": "event banner",
		},
		{},
		{},
		{},
		{},
		{},
	],
	"weapon": [
		{},
		{},
		{
			"masse1m": "one-handed blunt mace",
			"masse2m": "two-handed blunt mace",
			"baton": "one-handed blunt staff",
			"baton1m": "one-handed blunt staff",
			"batonspellcaster": "magic amplifier staff",
			"dague": "one-handed piercing dagger",
			"lance1m": "one-handed piercing spear",
			"lance2m": "two-handed piercing pike",
			"hache1m": "one-handed slashing axe",
			"hache2m": "two-handed slashing axe",
			"epee1m": "one-handed slashing sword",
			"epee2m": "two-handed slashing sword",
			"ms": "magic amplifier gloves",
			"pistolarc": "one-handed bowpistol",
			"pistolet": "one-handed pistol",
			"gatling": "two-handed autolauncher",
			"gattling": "two-handed autolauncher",
			"fusarc": "two-handed bowrifle",
			# "2h": "two-handed harpoon",
			"lanceroquette": "two-handed launcher",
			"launcher": "two-handed launcher",
			"fusil": "two-handed rifle",
			"petit_bouclier": "buckler shield",
			"grand_bouclier": "large shield",
			"grandbouclier": "large shield",
			"grenade": "grenade", # todo
		},
		{
			"_sousmarin": "underwater",
		},
		{},
		{},
		{},
		{},
		{},
	],
}

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

with open("shape_list.txt", "r") as f:
	with open("shape_parsed.tsv", "w") as fw:
		for l in f:
			if "_mission_" not in l:
				name = l.strip().split(".")[0]
				tags = parse(name)
				if name.startswith("tr_hof_underwear_") and not name.endswith("_gilet") and not name.endswith("_pantabottes"):
					tags.remove("tryker")
				if name.startswith("tr_hom_underwear_") and not name.endswith("_pantabottes"):
					tags.remove("tryker")
				# gen = generate(tags)
				# if gen != name:
				#{ 	tags += [ "invalid" ]
				fw.write(name + "\t")
				for t in tags:
					fw.write("\t" + t)
				fw.write("\n")
				if "incomplete" in tags:
					print(name)
					print(tags)
		fw.flush()
