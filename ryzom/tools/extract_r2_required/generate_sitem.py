
import os

def loadTsv(filename):
	table = []
	with open(filename, "r") as f:
		for l in f:
			table += [ l.strip().split("\t") ]
	return table;

sitemParents = loadTsv("sitem_parents.tsv")
sitemParsed = loadTsv("sitem_parsed.tsv")
shapeParsed = loadTsv("shape_parsed.tsv")
matchSitemShape = loadTsv("match_sitem_shape.tsv")

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
		print tags
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
			print shapeMale
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
		if "armor" in tags and "refugee" in tags:
			continue # No need to generate these for now
		
		print(path)
		with open(path, "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Revision=\"4.0\" State=\"modified\">\n")
			# f.write("  <PARENT Filename=\"" + parent + ".sitem\"/>\n")
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
			# f.write("      <ATOM Name=\"CraftPlan\" Value=\"" + name + ".sbrick\"/>\n") # TODO: extract sbrick ids
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
			
			# TODO: Find the best parent sheet
			# TODO: Extract the sbrick identifiers
			# TODO: Generate the sbrick

generateParents()
generateSitems()

