
import os, zlib

meleeSpec = [ "slashing", "piercing", "blunt" ]
curseSpec = [ "blind", "fear", "madness", "root", "sleep", "slow", "snare", "stun" ]
magicSpec = [ "acid", "cold", "rot", "fire", "poison", "electricity", "shockwave" ]

variantSpec = [ "a", "b", "c", "d", "e", "f" ] # Newbie, Basic, Fine, Choice, Excellent, Supreme

aiActionFolder = "R:\\leveldesign\\game_elem\\creature\\npc\\bestiary\\aiaction\\generic"
if not os.path.isdir(aiActionFolder):
	os.makedirs(aiActionFolder)

base = {
	"combat": {
		"fauna": meleeSpec,
		"melee": meleeSpec,
		"range": meleeSpec,
	},
	# "hybrid": {
		# "melee": meleeSpec * magicSpec,
		# "range": meleeSpec * magicSpec,
	# },
	"magic": {
		"damage": magicSpec,
		"dot": magicSpec,
		"heal": [ "hp", "sap", "stamina", "focus" ],
		"curse": curseSpec,
	},
	# "debuff": {
		# "skill": [ "melee", "range", "magic" ],
		# "resist": magicSpec,
	# },
}

# Damage
# ------
# Total Magic Damage = DamageValue + (SpellPowerFactor * creature.AttackLevel)

# Let's say 10k average total HP for homins and creatures at lvl 250
# Linear increase of 40 HP per level, so 400 HP per 10 levels, or 200 HP every 5 levels
maxLevel = 250
averageMaxHP = 10000

# Settle a fight in 10 seconds, that's an attack of 1000 at lvl 250 for a 1 second attack
# Add a base boost of 10 levels, that's 40 base attack
combatTime = 10

# Magic may do double the damage of melee
magicDamageBoost = 2

# Different boosts for each variant
variantBoost = { "a": 0.1, "b": 1.0, "c": 1.25, "d": 1.75, "e": 2.5, "f": 4.0 }
variantBaseLevel = { "a": 5, "b": 5, "c": 10, "d": 15, "e": 20, "f": 25 }
randomVariance = 0.1 # Random variance on the generated sheets

# Spell time
spellBaseTime = 1
spellRandomBaseTime = 2
spellPostTime = 0.5
spellRandomPostTime = 1

# magic_damage
for spec in magicSpec:
	for variant in variantSpec:
		name = "magic_damage_" + spec + "_" + variant
		randBoostFactor = zlib.crc32("SpellPowerFactor" + name) & 0xffffffff
		randBoostFactor = (((randBoostFactor % 2000) - 1000) * randomVariance) / 1000.0
		randBoostAdd = zlib.crc32("DamageValue" + name) & 0xffffffff
		randBoostAdd = (((randBoostAdd % 2000) - 1000) * randomVariance) / 1000.0
		randBaseTime = zlib.crc32("CastingTime" + name) & 0xffffffff
		randBaseTime = ((randBaseTime % 1000) * spellRandomBaseTime) / 1000.0
		randPostTime = zlib.crc32("PostActionTime" + name) & 0xffffffff
		randPostTime = ((randPostTime % 1000) * spellRandomPostTime) / 1000.0
		baseTime = int((spellBaseTime + randBaseTime) * 10) * 0.1
		postTime = int((spellPostTime + randPostTime) * 10) * 0.1
		totalTime = baseTime + postTime
		maxLevelDamage = (averageMaxHP * totalTime * magicDamageBoost) / combatTime
		damagePerLevel = maxLevelDamage / maxLevel
		damagePerLevelFactor = (damagePerLevel + (damagePerLevel * randBoostFactor)) * variantBoost[variant]
		damageAdd = (damagePerLevel + (damagePerLevel * randBoostAdd)) * variantBaseLevel[variant]
		behaviour = "CAST_" + spec.upper()
		if behaviour == "CAST_ELECTRICITY":
			behaviour = "CAST_ELEC"
		if behaviour == "CAST_SHOCKWAVE":
			behaviour = "CAST_SHOCK"
		damageType = spec.upper()
		if damageType == "SHOCKWAVE":
			damageType = "SHOCK"
		with open(aiActionFolder + "\\" + name + ".aiaction", "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <ATOM Name=\"type\" Value=\"DamageSpell\"/>\n")
			f.write("    <ATOM Name=\"Behaviour\" Value=\"" + behaviour + "\"/>\n")
			f.write("    <ATOM Name=\"SpellPowerFactor\" Value=\"" + str(round(damagePerLevelFactor, 3))+ "\"/>\n")
			f.write("    <ATOM Name=\"DamageScore\" Value=\"HitPoints\"/>\n")
			f.write("    <ATOM Name=\"DamageType\" Value=\"" + damageType + "\"/>\n")
			f.write("    <ATOM Name=\"DamageValue\" Value=\"" + str(int(damageAdd)) + "\"/>\n")
			f.write("    <ATOM Name=\"CastingTime\" Value=\"" + str(baseTime) + "\"/>\n")
			f.write("    <ATOM Name=\"PostActionTime\" Value=\"" + str(postTime) + "\"/>\n")
			f.write("  </STRUCT>\n")
			f.write("</FORM>\n")
