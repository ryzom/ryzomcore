
import os, zlib

def loadTsv(filename):
	table = []
	with open(filename, "r") as f:
		for l in f:
			table += [ l.strip().split("\t") ]
	return table;

# brick family
# BC brick craft
# BCB tribe
# BCBM melee
# BCBME effect (item)
# BCBMEA *
# BCBR ranged
# BCBREA *
# BCCA armor
# BCCAEA light
# BCCAEB medium
# BCCAEC heavy
# BCCAED caster
# BCCP ammo
# BCCPEA *
# BCCS shield
# BCCSEA *
# BCFJ jewelry
# BCFJC craft
# BCFJE effect (item)
# BCFJEA *

# also generate its sphrase under sphrase\craft\melee_weapon etc, 
# name add 'a' to the start, containing just sbrick and castable false

# brick skill
# SC skill craft (LearnRequiresOneOfSkills "SC 0")
# SCJ jewelry
# SCJR ring
# SCJRAEM *
# SCAMBEM skill craft armor medium boots
# SCAMGEM skill craft armor medium gloves
# SCAMPEM skill craft armor medium pants
# SCAMSEM skill craft armor medium sleeves
# SCAMVEM skill craft armor medium vest
# SCAH heavy
# SCAL light (and caster)


sitemParents = loadTsv("sitem_parents.tsv")
sitemParsed = loadTsv("sitem_parsed.tsv")
shapeParsed = loadTsv("shape_parsed.tsv")
matchSitemShape = loadTsv("match_sitem_shape.tsv")

skills = loadTsv("skills.tsv")
skillTree = { "S": { "tags": [ "*" ], "entries": {} } }

brickFamilies = loadTsv("brick_families.tsv")
brickFamilyTree = { "B": { "tags": [ "*" ], "entries": {} } }

def buildTagTree(tree, table):
	for entry in table:
		e = filter(None, entry)
		name = e[0]
		#print(name)
		tags = e[1:]
		nb = 0
		branch = tree
		if len(tags) > 0:
			for c in name:
				nb += 1
				if nb >= len(name):
					break
				branch = branch[c]["entries"]
			branch[name[-1]] = { "tags": tags, "entries": {} }
	#print(tree)

buildTagTree(skillTree, skills)
buildTagTree(brickFamilyTree, brickFamilies)

def findTreeEntry(tree, tags):
	res = ""
	branch = tree
	while True:
		ok = False
		if len(branch) == 0:
			break
		for entry in branch:
			#print(entry)
			for tag in branch[entry]["tags"]:
				if tag != "*" and tag in tags:
					ok = True
					break
			if ok:
				res += entry
				branch = branch[entry]["entries"]
				break
		if not ok:
			for entry in branch:
				#print(entry)
				#print(branch[entry]["tags"])
				for tag in branch[entry]["tags"]:
					if tag == "*":
						ok = True
						break
				if ok:
					res += entry
					branch = branch[entry]["entries"]
					break
		if not ok:
			break
	return res

def findSkill(tags):
	t = tags[:]
	if "ammo" in t and "ranged" in t:
		t.remove("ranged")
	if "magic" in t and "two-handed" in t:
		t.remove("two-handed")
	if "refugee" in t and not "heavy" in t and not "medium" in t and not "light" in t:
		t += [ "light" ]
	res = findTreeEntry(skillTree, t)
	if len(res) == 7:
		return res
	print("Skill: " + res + ", tags: ")
	print(tags)
	return ""

def findBrickFamily(tags):
	t = tags[:]
	if "ammo" in t and "ranged" in t:
		t.remove("ranged")
	if "caster" in t and "light" in t:
		t.remove("light")
	res = findTreeEntry(brickFamilyTree, t)
	return res

sbrickIndex = loadTsv("sbrick_index.tsv")
sbrickLookup = {}
sbrickAlloc = {}

for entry in sbrickIndex:
	names = entry[2:]
	for name in names:
		sbrickLookup[name] = entry
	family = entry[0]
	index = int(entry[1], 10)
	if not family in sbrickAlloc:
		sbrickAlloc[family] = {}
	sbrickAlloc[family][index] = True

def allocFamilyIndex(family):
	for i in range(1, 64):
		if not i in sbrickAlloc[family]:
			sbrickAlloc[family][i] = True
			return i
	return None

sitemTags = {}
sitemPath = "Y:\\ryzomcore4\\leveldesign\\game_element\\sitem"

parentTags = {}

for sitem in sitemParsed:
	sitemTags[sitem[0]] = sitem[2:]
for sitem in sitemParents:
	sitemTags[sitem[0]] = sitem[2:]

def generateParents():
	for sitem in sitemParents:
		name = sitem[0]
		tags = sitem[2:]
		strippedTags = tags[:]
		
		if "parent" in strippedTags:
			strippedTags.remove("parent")
		if "shared" in strippedTags:
			strippedTags.remove("shared")
		if "unspecified" in strippedTags:
			strippedTags.remove("unspecified")
		if "generic" in strippedTags:
			strippedTags.remove("generic")
		if "base" in strippedTags:
			strippedTags.remove("base")
		if "melee" in strippedTags:
			strippedTags.remove("melee")
		if "ranged" in strippedTags:
			strippedTags.remove("ranged")
		if "light" in strippedTags and "caster" in strippedTags:
			strippedTags.remove("light")
		if "light" in strippedTags and "refugee" in strippedTags:
			strippedTags.remove("light")
		if "shared" in tags:
			parentTags[name] = strippedTags
		displayName = " ".join(strippedTags)
		
		family = "undefined" # item_family.typ
		if "ammo" in tags:
			family = "ammo"
		elif "shield" in tags:
			family = "shield"
		elif "ranged" in tags:
			family = "range weapon"
		elif "melee" in tags:
			family = "melee weapon"
		elif "armor" in tags:
			family = "armor"
		
		origin = "common" # item_origine.typ
		if "refugee" in tags:
			origin = "refugee"
		elif "tribe" in tags:
			origin = "tribe"
		elif "karavan" in tags:
			origin = "karavan"
		elif "kami" in tags:
			origin = "kami"
		elif "fyros" in tags:
			origin = "fyros"
		elif "matis" in tags:
			origin = "matis"
		elif "zorai" in tags:
			origin = "zorai"
		elif "tryker" in tags:
			origin = "tryker"
		
		dropOrSell = not "refugee" in tags
		
		armorCategory = "unknown" # item_category.typ
		if "heavy" in tags:
			armorCategory = "Heavy"
		elif "medium" in tags:
			armorCategory = "medium"
		elif "light" in tags:
			armorCategory = "light"
		elif "melee" in tags:
			armorCategory = "hands"
		elif "ranged" in tags:
			armorCategory = "hands"
		elif "magic" in tags:
			armorCategory = "hands"
		elif "refugee" in tags:
			armorCategory = "light"
		
		iconOverlay = ""
		if "refugee" in tags:
			iconOverlay = "pw_light.png"
		elif "light" in tags:
			iconOverlay = "pw_light.png"
		elif "medium" in tags:
			iconOverlay = "pw_medium.png"
		elif "heavy" in tags:
			iconOverlay = "pw_heavy.png"
		
		neverHide = False
		icon = ""
		animSet = ""
		itemType = "undefined" # item_type.typ
		leftHandSlot = "Undefined" # item_slot_type.typ
		rightHandSlot = "Undefined" # item_slot_type.typ
		if "magic" in tags:
			if "amplifier" in tags:
				if "gloves" in tags:
					itemType = "Magician Staff"
					icon = "mg_glove.png"
					neverHide = True
					rightHandSlot = "Two Hands"
		elif "melee" in tags:
			if "one-handed" in tags:
				animSet = "1H"
				rightHandSlot = "Right Hand"
				if "dagger" in tags:
					itemType = "Dagger"
					animSet = "Dagger"
					icon = "mw_dagger.png"
					leftHandSlot = "Left Hand"
				elif "sword" in tags:
					itemType = "Sword"
					animSet = "1H Sword"
					icon = "mw_sword.png"
				elif "mace" in tags:
					itemType = "Mace"
					icon = "mw_mace.png"
				elif "axe" in tags:
					itemType = "Axe"
					icon = "mw_axe.png"
				elif "spear" in tags:
					itemType = "Spear"
					icon = "mw_lance.png"
				elif "staff" in tags:
					itemType = "Staff"
					icon = "mw_staff.png"
			if "two-handed" in tags:
				animSet = "2H"
				rightHandSlot = "Two Hands"
				if "sword" in tags:
					itemType = "Two Hand Sword"
					animSet = "2H Sword"
					icon = "mw_2h_sword.png"
				elif "axe" in tags:
					itemType = "Two Hand Axe"
					icon = "mw_2h_axe.png"
				elif "pike" in tags:
					itemType = "Pike"
					animSet = "2HLance"
					icon = "mw_2h_lance.png"
				elif "mace" in tags:
					itemType = "Two Hand Mace"
					icon = "mw_2h_mace.png"
		elif "ranged" in tags:
			if "autolauncher" in tags:
				itemType = "Autolauch"
			elif "bowrifle" in tags:
				itemType = "Bowrifle"
			elif "launcher" in tags:
				itemType = "Launcher"
			elif "pistol" in tags:
				itemType = "Pistol"
			elif "bowpistol" in tags:
				itemType = "Bowpistol"
			elif "rifle" in tags:
				itemType = "Rifle"
			elif "harpoon" in tags:
				itemType = "Harpoon"
			if "ammo" in tags and itemType != "undefined":
				itemType += " ammo"
		elif "shield" in tags:
			leftHandSlot = "Left Hand"
			animSet = "Shield"
			if "buckler" in tags:
				icon = "sh_buckler.png"
				itemType = "Buckler"
			elif "large" in tags:
				icon = "sh_large_shield.png"
				itemType = "Shield"
		elif "armor" in tags:
			if "refugee" in tags:
				itemType = "Light "
			elif "light" in tags:
				itemType = "Light "
			elif "medium" in tags:
				itemType = "Medium "
			elif "heavy" in tags:
				itemType = "Heavy "
			if "boots" in tags:
				itemType += "boots"
			elif "gloves" in tags:
				itemType += "gloves"
			elif "pants" in tags:
				itemType += "pants"
			elif "sleeves" in tags:
				itemType += "Sleeves"
			elif "vest" in tags:
				itemType += "vest"
			elif "helmet" in tags and "heavy" in tags:
				itemType += "helmet"
			else:
				itemType = "undefined"
		
		damageType = "undefined" # item_damage_type.typ
		if "piercing" in tags:
			damageType = "Piercing"
		elif "slashing" in tags:
			damageType = "Slashing"
		elif "blunt" in tags or "magic" in tags:
			damageType = "Blunt"
		
		# craftingTool = "None" # item_crafting_tool_type.typ
		# if "armor" in tags:
		# 	craftingTool = "ArmorTool"
		
		folder = "_"
		if "ammo" in tags:
			folder = "ammo"
		elif "ranged" in tags:
			folder = "range_weapon"
		elif "melee" in tags:
			folder = "melee_weapon"
		elif "armor" in tags:
			folder = "armor"
		elif "shield" in tags:
			folder = "shield"
		
		if "shared" in tags and not "melee" in tags:
			malus = 0
			bulk = 4 # TODO: Calibrate
			equipTicks = 10 # TODO: Calibrate
			if "one-handed" in tags:
				malus += 0.01 # TODO: Calibrate
				bulk += 2 # TODO: Calibrate
				equipTicks += 10 # TODO: Calibrate
			if "two-handed" in tags:
				malus += 0.02 # TODO: Calibrate
				bulk += 4 # TODO: Calibrate
				equipTicks += 20 # TODO: Calibrate
			if "medium" in tags:
				malus += 0.01 # TODO: Calibrate
				bulk += 2 # TODO: Calibrate
				equipTicks += 10 # TODO: Calibrate
			if "heavy" in tags:
				malus += 0.02 # TODO: Calibrate
				bulk += 4 # TODO: Calibrate
				equipTicks += 20 # TODO: Calibrate
			if "buckler" in tags:
				malus += 0.01 # TODO: Calibrate
				bulk += 2 # TODO: Calibrate
				equipTicks += 10 # TODO: Calibrate
			if "large" in tags:
				malus += 0.02 # TODO: Calibrate
				bulk += 4 # TODO: Calibrate
				equipTicks += 20 # TODO: Calibrate
			if "magic" in tags:
				malus = 0
				bulk += 2 # TODO: Calibrate
				equipTicks += 10 # TODO: Calibrate
			comments = "TODO: WearEquipmentMalus, Bulk, Time to Equip In Ticks"
			
			dir = sitemPath + "\\" + folder + "\\_parent"
			if not os.path.isdir(dir):
				os.makedirs(dir)
			path = dir + "\\" + name + ".sitem"
			
			print(path)
			with open(path, "w") as f:
				f.write("<?xml version=\"1.0\"?>\n")
				f.write("<FORM Revision=\"4.0\" State=\"modified\">\n")
				f.write("  <STRUCT>\n")
				f.write("    <STRUCT Name=\"basics\">\n")
				f.write("      <ATOM Name=\"name\" Value=\"" + displayName + "\"/>\n")
				if origin != "common":
					f.write("      <ATOM Name=\"origin\" Value=\"" + origin + "\"/>\n")
				if family != "undefined":
					f.write("      <ATOM Name=\"family\" Value=\"" + family + "\"/>\n")
				if not dropOrSell:
					f.write("      <ATOM Name=\"Drop or Sell\" Value=\"false\"/>\n")
				if itemType != "undefined":
					f.write("      <ATOM Name=\"ItemType\" Value=\"" + itemType + "\"/>\n")
				if "armor" not in tags or malus != 0:
					f.write("      <STRUCT Name=\"EquipmentInfo\">\n")
					if "armor" not in tags:
						f.write("        <ARRAY Name=\"EquipmentSlots\">\n")
						if rightHandSlot != "Undefined":
							f.write("          <ATOM Name=\"" + rightHandSlot.lower().replace(" ", "_") + "\" Value=\"" + rightHandSlot + "\"/>\n")
						if leftHandSlot != "Undefined":
							f.write("          <ATOM Name=\"" + leftHandSlot.lower().replace(" ", "_") + "\" Value=\"" + leftHandSlot + "\"/>\n")
						f.write("        </ARRAY>\n")
					if malus != 0:
						f.write("        <ATOM Name=\"WearEquipmentMalus\" Value=\"" + str(malus) + "\"/>\n")
					f.write("      </STRUCT>\n")
				f.write("      <ATOM Name=\"Bulk\" Value=\"" + str(bulk) + "\"/>\n")
				f.write("      <ATOM Name=\"Time to Equip In Ticks\" Value=\"" + str(equipTicks) + "\"/>\n")
				f.write("    </STRUCT>\n")
				if "melee" in tags and damageType != "undefined":
					f.write("    <STRUCT Name=\"melee weapon\">\n")
					if damageType != "undefined":
						f.write("      <ATOM Name=\"damage type\" Value=\"" + damageType + "\">\n")
					f.write("    </STRUCT>\n")
				if "armor" in tags and armorCategory != "unknown":
					f.write("    <STRUCT Name=\"armor\">\n")
					f.write("      <ATOM Name=\"Armor category\" Value=\"" + armorCategory + "\"/>\n")
					f.write("    </STRUCT>\n")
				if "shield" in tags:
					f.write("    <STRUCT Name=\"shield\">\n")
					if "large" in tags:
						f.write("      <ATOM Name=\"Category\" Value=\"large shield\"/>\n")
					else:
						f.write("      <ATOM Name=\"Category\" Value=\"small shield\"/>\n")
					f.write("    </STRUCT>\n")
				f.write("    <STRUCT Name=\"3d\">\n")
				if icon != "":
					f.write("      <ATOM Name=\"icon\" Value=\"" + icon + "\"/>\n")
				if iconOverlay != "":
					f.write("      <ATOM Name=\"icon overlay\" Value=\"" + iconOverlay + "\"/>\n")
				if "armor" in tags:
					f.write("      <ATOM Name=\"color\" Value=\"UserColor\"/>\n")
				if animSet != "":
					f.write("      <ATOM Name=\"anim_set\" Value=\"" + animSet + "\"/>\n")
				if neverHide:
					f.write("      <ATOM Name=\"never hide when equiped\" Value=\"true\"/>\n")
				f.write("    </STRUCT>\n")
				f.write("  </STRUCT>\n")
				f.write("</FORM>\n")
				f.flush()
		# elif "generic" in tags:
		# 	dir = sitemPath + "\\" + folder + "\\_parent\\" + origin
		# 	if not os.path.isdir(dir):
		# 		os.makedirs(dir)
		# 	path = dir + "\\" + name + ".sitem"
		# 	
		# 	print(path)
		# 	with open(path, "w") as f:
		# 		f.write("<?xml version=\"1.0\"?>\n")
		# 		f.write("<FORM Revision=\"4.0\" State=\"modified\">\n")
		# 		f.write("</FORM>\n")
		# 		f.flush()

def findSitemParent(tags):
	for parent in parentTags:
		pt = parentTags[parent]
		all = len(pt) > 0
		for tag in pt:
			if not tag in tags:
				all = False
				break
		if all:
			return parent
	return None

def generateSitems():
	for match in matchSitemShape:
		name = match[0]
		shapeMale = match[1]
		shapeFemale = match[2]
		tags = sitemTags[name]
		strippedTags = tags[:]
		
		if "item" in strippedTags:
			strippedTags.remove("item")
		if "crafted" in strippedTags:
			strippedTags.remove("crafted")
		if "hands" in strippedTags:
			strippedTags.remove("hands")
		if "light" in strippedTags and "caster" in strippedTags:
			strippedTags.remove("light")
		if "light" in strippedTags and "refugee" in strippedTags:
			strippedTags.remove("light")
		if "light" in strippedTags and "underwear" in strippedTags:
			strippedTags.remove("light")
		if "hq" in strippedTags:
			strippedTags.remove("hq")
			#strippedTags = [ "high quality" ] + strippedTags
		if "mq" in strippedTags:
			strippedTags.remove("mq")
			#strippedTags = [ "medium quality" ] + strippedTags
		if "armor" in strippedTags:
			strippedTags.remove("armor")
		if "melee" in strippedTags:
			strippedTags.remove("melee")
		if "ranged" in strippedTags:
			strippedTags.remove("ranged")
		displayName = " ".join(strippedTags)
		
		origin = "common" # item_origine.typ
		iconBackground = ""
		if "refugee" in tags:
			origin = "refugee"
			iconBackground = "bk_generic.png"
		elif "tribe" in tags:
			origin = "tribe"
			iconBackground = "bk_generic.png"
		elif "karavan" in tags:
			origin = "karavan"
			iconBackground = "bk_karavan.png"
		elif "kami" in tags:
			origin = "kami"
			iconBackground = "bk_kami.png"
		elif "fyros" in tags:
			origin = "fyros"
			iconBackground = "bk_fyros.png"
		elif "matis" in tags:
			origin = "matis"
			iconBackground = "bk_matis.png"
		elif "zorai" in tags:
			origin = "zorai"
			iconBackground = "bk_zorai.png"
		elif "tryker" in tags:
			origin = "tryker"
			iconBackground = "bk_tryker.png"
		elif "common" in tags:
			origin = "common"
			iconBackground = "bk_generic.png"
		
		mapVariant = "Default"
		if "mq" in tags:
			mapVariant = "Medium Quality"
		elif "hq" in tags:
			mapVariant = "High Quality"
		
		itemType = "undefined"
		armorSlot = "Undefined" # item_slot_type.typ
		icon = ""
		if "armor" in tags:
			if "refugee" in tags:
				itemType = "Light "
			elif "light" in tags:
				itemType = "Light "
			elif "medium" in tags:
				itemType = "Medium "
			elif "heavy" in tags:
				itemType = "Heavy "
			if "boots" in tags:
				itemType += "boots"
				armorSlot = "Feet"
				icon = "ar_botte.png"
			elif "gloves" in tags:
				itemType += "gloves"
				armorSlot = "Hands"
				icon = "ar_hand.png"
			elif "pants" in tags:
				itemType += "pants"
				armorSlot = "Legs"
				icon = "ar_pantabotte.png"
			elif "sleeves" in tags:
				itemType += "Sleeves"
				armorSlot = "Arms"
				icon = "ar_armpad.png"
			elif "vest" in tags:
				itemType += "vest"
				armorSlot = "Chest"
				icon = "ar_gilet.png"
			elif "helmet" in tags and "heavy" in tags:
				itemType += "helmet"
				armorSlot = "Head"
				icon = "ar_helmet.png"
			else:
				itemType = "undefined"
		
		color = ""
		if "red" in tags:
			color = "Red"
		elif "black" in tags:
			color = "Black"
		elif "beige" in tags:
			color = "Beige"
		elif "green" in tags:
			color = "Green"
		elif "turquoise" in tags:
			color = "Turquoise"
		elif "blue" in tags:
			color = "Blue"
		elif "violet" in tags:
			color = "Violet"
		elif "white" in tags:
			color = "White"
		
		subfolder = origin
		###print tags
		if "underwear" in tags:
			subfolder = "underwear"
		
		folder = "_unspecified"
		if "ammo" in tags:
			folder = "ammo\\" + subfolder
		elif "ranged" in tags:
			folder = "range_weapon\\" + subfolder
		elif "melee" in tags:
			folder = "melee_weapon\\" + subfolder
		elif "armor" in tags:
			###print shapeMale
			folder = "armor\\" + subfolder
			if "caster01" in shapeMale or "caster" in tags:
				folder += "\\caster_armor"
			elif ("civil01" in shapeMale or ("underwear" in shapeMale and not "underwear" in tags)) and "light" in tags:
				folder += "\\light_armor"
			elif "armor00" in shapeMale and "medium" in tags:
				folder += "\\medium_armor"
			elif ("armor01" in shapeMale or "casque01" in shapeMale) and "heavy" in tags:
				folder += "\\heavy_armor"
		elif "shield" in tags:
			folder = "shield\\" + subfolder
		
		dir = sitemPath + "\\" + folder
		if not os.path.isdir(dir):
			os.makedirs(dir)
		path = dir + "\\" + name + ".sitem"
		
		# print(name)
		# print(shapeMale)
		# print(shapeFemale)
		# print(tags)
		
		if not "armor" in tags:
			continue
		
		if "armor" in tags and "caster" in tags and not "pants" in tags:
			continue # Only include caster pants
		if "armor" in tags and "refugee" in tags and len(name) > 5:
			continue # No need to generate these for now
		
		brickFamily = findBrickFamily(tags)
		
		sbrickEntry = None
		if name in sbrickLookup:
			sbrickEntry = sbrickLookup[name]
		else:
			print("New sbrick entry: " + name + ", Family: " + brickFamily)
		if sbrickEntry and sbrickEntry[0] != brickFamily:
			print("Brick family changed: " + name + ", New: " + brickFamily + ", Old: " + sbrickEntry[0])
			sbrickEntry = None
		sbrickIndex = None
		if sbrickEntry:
			sbrickIndex = int(sbrickEntry[1], 10)
		else:
			sbrickIndex = allocFamilyIndex(brickFamily)
			if sbrickIndex:
				sbrickEntry = [ brickFamily, str(sbrickIndex) ]
				print(sbrickEntry)
		sbrickName = brickFamily.lower() + str(sbrickIndex).zfill(2) + "_" + name[2:];
		
		parent = findSitemParent(tags)
		if not parent:
			print("No parent for sitem: " + name + ", tags: " + tags)
		
		print(path)
		with open(path, "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Revision=\"4.0\" State=\"modified\">\n")
			if parent:
				f.write("  <PARENT Filename=\"" + parent + ".sitem\"/>\n")
			f.write("  <STRUCT>\n")
			f.write("    <STRUCT Name=\"basics\">\n")
			f.write("      <ATOM Name=\"name\" Value=\"" + displayName + "\"/>\n")
			if itemType != "undefined":
				f.write("      <ATOM Name=\"ItemType\" Value=\"" + itemType + "\"/>\n")
			if armorSlot != "Undefined":
				f.write("      <STRUCT Name=\"EquipmentInfo\">\n")
				f.write("        <ARRAY Name=\"EquipmentSlots\">\n")
				f.write("          <ATOM Value=\"" + armorSlot + "\"/>\n")
				f.write("        </ARRAY>\n")
				f.write("      </STRUCT>\n")
			f.write("      <ATOM Name=\"CraftPlan\" Value=\"" + sbrickName + ".sbrick\"/>\n") # TODO: extract sbrick ids
			f.write("    </STRUCT>\n")
			f.write("    <STRUCT Name=\"3d\">\n")
			f.write("      <ATOM Name=\"shape\" Value=\"" + shapeMale + ".shape\"/>\n")
			if shapeFemale != shapeMale:
				f.write("      <ATOM Name=\"shape_female\" Value=\"" + shapeFemale + ".shape\"/>\n")
			if mapVariant != "Default":
				f.write("      <ATOM Name=\"map_variant\" Value=\"" + mapVariant + "\"/>\n")
			if icon != "":
				f.write("      <ATOM Name=\"icon\" Value=\"" + icon + "\"/>\n")
			if iconBackground != "":
				f.write("      <ATOM Name=\"icon background\" Value=\"" + iconBackground + "\"/>\n")
			if color != "":
				f.write("      <ATOM Name=\"color\" Value=\"" + color + "\"/>\n")
			f.write("    </STRUCT>\n")
			f.write("  </STRUCT>\n")
			f.write("</FORM>\n")
			f.flush()
		#print(brickFamily)
		
		# Generate the sbrick
		sbrickRoot = "Y:\\ryzomcore4\\leveldesign\\game_element\\sbrick\\craft\\effect"
		sbrickFolder = sbrickRoot + "\\" + folder
		sbrickFile = sbrickFolder + "\\" + sbrickName + ".sbrick"
		print(sbrickFile)
		
		skill = findSkill(tags)
		# if "magic" in tags:
		# 	print("TODO: Double check magic")
		# 	print(skill)
		# 	print(tags)
		
		sbrickIcon = icon # ar_botte
		sbrickIconBack = "bk_generic_brick.png" # bk_zorai_brick
		if "fyros" in tags:
			sbrickIconBack = "bk_fyros_brick.png"
		elif "matis" in tags:
			sbrickIconBack = "bk_matis_brick.png"
		elif "tryker" in tags:
			sbrickIconBack = "bk_tryker_brick.png"
		elif "zorai" in tags:
			sbrickIconBack = "bk_zorai_brick.png"
		
		minMat = 2
		randMat = 4
		if "one-handed" in tags:
			minMat += 1
			randMat += 2
		if "two-handed" in tags:
			minMat += 2
			randMat += 4
		if "medium" in tags:
			minMat += 2
			randMat += 4
		if "heavy" in tags:
			minMat += 4
			randMat += 8
		if "buckler" in tags:
			minMat += 1
			randMat += 3
		if "large" in tags:
			minMat += 2
			randMat += 6
		if "magic" in tags:
			minMat += 3
			randMat += 6
		if "mq" in tags:
			minMat += 1
			randMat += 2
		if "hq" in tags:
			minMat += 2
			randMat += 4
		minMat *= 3
		randMat *= 3
		# rand0 = zlib.crc32(name + brickFamily + skill) & 0xffffffff
		rand1 = zlib.crc32(name + skill + brickFamily) & 0xffffffff
		rand2 = zlib.crc32(skill + name + brickFamily) & 0xffffffff
		rand3 = zlib.crc32(skill + brickFamily + name) & 0xffffffff
		rand4 = zlib.crc32(brickFamily + name + skill) & 0xffffffff
		rand5 = zlib.crc32(brickFamily + skill + name) & 0xffffffff
		mp1 = minMat + (rand1 % randMat)
		mp2 = minMat + (rand2 % randMat)
		mp3 = minMat + (rand3 % randMat)
		mp4 = minMat + (rand4 % randMat)
		mp5 = minMat + (rand5 % randMat)
		
		if not os.path.isdir(sbrickFolder):
			os.makedirs(sbrickFolder)
		with open(sbrickFile, "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Revision=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <STRUCT Name=\"Basics\">\n")
			f.write("      <ATOM Name=\"FamilyId\" Value=\"" + brickFamily + "\"/>\n")
			f.write("      <ATOM Name=\"IndexInFamily\" Value=\"" + str(sbrickIndex) + "\"/>\n")
			f.write("      <ATOM Name=\"SPCost\" Value=\"5\"/>\n")
			f.write("      <ATOM Name=\"LearnRequiresOneOfSkills\" Value=\"SC 0\"/>\n")
			f.write("      <ATOM Name=\"Action Nature\" Value=\"CRAFT\"/>\n")
			f.write("      <ATOM Name=\"Skill\" Value=\"" + skill + "\"/>\n")
			# f.write("      <ATOM Name=\"LearnRequiresBricks\" Value=\"bcpa01.sbrick\"/>\n") # TODO: Craft actions
			f.write("      <ATOM Name=\"CivRestriction\" Value=\"common\"/>\n")
			f.write("    </STRUCT>\n")
			f.write("    <STRUCT Name=\"Client\">\n")
			if sbrickIcon != "":
				f.write("      <ATOM Name=\"Icon\" Value=\"" + sbrickIcon + "\"/>\n")
			if sbrickIconBack != "":
				f.write("      <ATOM Name=\"IconBack\" Value=\"" + sbrickIconBack + "\"/>\n")
			f.write("      <ATOM Name=\"IconOver\" Value=\"fp_over.png\"/>\n")
			f.write("      <ATOM Name=\"IconOver2\"/>\n")
			f.write("    </STRUCT>\n")
			if "armor" in tags:
				f.write("    <STRUCT Name=\"faber\">\n")
				f.write("      <ATOM Name=\"Tool type\" Value=\"ArmorTool\"/>\n")
				# f.write("      <ATOM Name=\"Durability Factor\" Value=\"1\"/>\n")
				f.write("      <STRUCT Name=\"Create\">\n")
				f.write("        <ATOM Name=\"Crafted Item\" Value=\"" + name + ".sitem\"/>\n")
				f.write("        <ATOM Name=\"Nb built items\" Value=\"1\"/>\n")
				if "light" in tags:
					f.write("        <ATOM Name=\"MP 1\" Value=\"Raw Material for Clothes\"/>\n")
				else:
					f.write("        <ATOM Name=\"MP 1\" Value=\"Raw Material for Armor shell\"/>\n")
				f.write("        <ATOM Name=\"Quantity 1\" Value=\"" + str(int(mp1 / 4)) + "\"/>\n") # TODO: Calibrate
				f.write("        <ATOM Name=\"MP 2\" Value=\"Raw Material for Armor interior coating\"/>\n")
				f.write("        <ATOM Name=\"Quantity 2\" Value=\"" + str(int(mp2 / 4)) + "\"/>\n") # TODO: Calibrate
				f.write("        <ATOM Name=\"MP 3\" Value=\"Raw Material for Armor interior stuffing\"/>\n")
				f.write("        <ATOM Name=\"Quantity 3\" Value=\"" + str(int(mp3 / 4)) + "\"/>\n") # TODO: Calibrate
				f.write("        <ATOM Name=\"MP 4\" Value=\"Raw Material for Armor clip\"/>\n")
				f.write("        <ATOM Name=\"Quantity 4\" Value=\"" + str(int(mp4 / 4)) + "\"/>\n") # TODO: Calibrate
				f.write("        <ATOM Name=\"MP 5\"/>\n")
				f.write("        <ATOM Name=\"Quantity 5\" Value=\"0\"/>\n")
				f.write("      </STRUCT>\n")
				f.write("    </STRUCT>\n")
			f.write("  </STRUCT>\n")
			f.write("</FORM>\n")
			f.flush()
			
		sphraseName = "a" + sbrickName;
		sphraseRoot = "Y:\\ryzomcore4\\leveldesign\\game_element\\sphrase\\craft\\effect"
		sphraseFolder = sphraseRoot + "\\" + folder
		sphraseFile = sphraseFolder + "\\" + sphraseName + ".sphrase"
		print(sphraseFile)
		
		if not os.path.isdir(sphraseFolder):
			os.makedirs(sphraseFolder)
		with open(sphraseFile, "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Revision=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <ATOM Name=\"brick 0\" Value=\"" + sbrickName + ".sbrick\"/>\n")
			f.write("    <ATOM Name=\"castable\" Value=\"false\"/>\n")
			if "fyros" not in tags and "matis" not in tags and "zorai" not in tags and "tryker" not in tags:
				f.write("    <ATOM Name=\"ShowInAPOnlyIfLearnt\" Value=\"true\"/>\n")
			f.write("  </STRUCT>\n")
			f.write("</FORM>\n")
			f.flush()

generateParents()
generateSitems()

