
import os, zlib

meleeSpec = [ "slashing", "piercing", "blunt" ]
curseSpec = [ "blind", "fear", "madness", "root", "sleep", "slow", "snare", "stun" ]
magicSpec = [ "acid", "cold", "rot", "fire", "poison", "electricity", "shockwave" ]

#specialSpec = [ "_weak", "", "_strong" ]

# Cannon fodder invasion kitins and newbie creatures use the "a" variant
# Most creatures use "b" and "c". The "d", "e", and "f" variants are for bosses
# Goo infected creatures should use "c" and "d"
#variantSpec = [ "a", "b", "c", "d", "e", "f" ] # Newbie, Basic, Fine, Choice, Excellent, Supreme

aiActionFolder = "R:\\leveldesign\\game_elem\\creature\\npc\\bestiary\\aiaction\\generic"
if not os.path.isdir(aiActionFolder):
	os.makedirs(aiActionFolder)

base = {
	"combat": {
		"fauna": meleeSpec,
		"melee": meleeSpec,
		"range": meleeSpec,
	},
	# "enchanted": { # These 1.5x the damage (regular melee plus 0.5x magic damage)
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
maxLevel = 250.0
# averageMaxHP = 10000.0
minCharacteristic = 10.0
maxCharacteristic = minCharacteristic + maxLevel
minScore = minCharacteristic * 60.0
maxScore = maxCharacteristic * 60.0

# Settle a fight in 10 seconds, that's an attack of 1000 at lvl 250 for a 1 second attack
# Add a base boost of 10 levels, that's 40 base attack
combatTime = 8.0 # 10

# Magic may do double the damage of melee
# Range is most powerful but requires ammo of course.
# Perhaps allow range without ammo at melee damage levels. Or hybrid range and magic for ammo-less range
# For NPCs, range should have melee-like damage
meleeDamageBoost = 1.0
magicDamageBoost = 2.0
rangeDamageBoost = 1.0 # 4

# Boosts for special attack variants
#meleeSpecialBoost = [ 0.5, 1.0, 1.5 ]
#rangeSpecialBoost = [ 0.5, 0.25, 1.0 ]

# Different boosts for each variant
#variantBoost = { "a": 0.1, "b": 1.0, "c": 1.25, "d": 1.75, "e": 2.5, "f": 4.0 }
#variantBaseLevel = { "a": 5, "b": 5, "c": 10, "d": 15, "e": 20, "f": 25 }
# variantBaseLevel = { "a": 25, "b": 25, "c": 30, "d": 35, "e": 40, "f": 45 }
randomVariance = 0.1 # Random variance on the generated sheets

# Spell time
spellBaseTime = 1.0
spellRandomBaseTime = 2.0
spellPostTime = 0.5
spellRandomPostTime = 1.0

# magic_damage
# egs_static_ai_action.cpp
for spec in magicSpec:
	# for variant in variantSpec:
	name = "magic_damage_" + spec
	randBoostFactor = zlib.crc32("SpellPowerFactor" + name) & 0xffffffff
	randBoostFactor = (((randBoostFactor % 2000) - 1000) * randomVariance) / 1000.0
	randBoostAdd = zlib.crc32("DamageValue" + name) & 0xffffffff
	randBoostAdd = (((randBoostAdd % 2000) - 1000) * randomVariance) / 1000.0
	randBaseTime = zlib.crc32("CastingTime" + name) & 0xffffffff
	randBaseTime = ((randBaseTime % 1000) * spellRandomBaseTime) / 1000.0
	randPostTime = zlib.crc32("PostActionTime" + name) & 0xffffffff
	randPostTime = ((randPostTime % 1000) * spellRandomPostTime) / 1000.0
	baseTime = int((spellBaseTime + randBaseTime) * 10.0) * 0.1
	postTime = int((spellPostTime + randPostTime) * 10.0) * 0.1
	totalTime = baseTime + postTime
	maxLevelDamage = (maxScore * totalTime * magicDamageBoost) / combatTime
	damagePerLevel = maxLevelDamage / maxCharacteristic
	damagePerLevelFactor = (damagePerLevel + (damagePerLevel * randBoostFactor)) # * variantBoost[variant]
	damageAdd = (damagePerLevel + (damagePerLevel * randBoostAdd)) * minCharacteristic # * variantBaseLevel[variant]
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

# player melee boosts
# skill -> 10x
# item speed -> 2x
# item dmg -> 2x
# increase damage -> 2x

# player magic boosts
# glove speed -> 2x
# glove dmg -> 2x
# double spell -> 2x

meleeReferenceHitRate = 2.5 # 2h long sword, hits per 10s
meleeReferenceDmg = 1.0 # 2h long sword

regenTimeMin = 25.0 # in seconds, at lowest level
regenTimeMax = 100.0 # in seconds, at max level
regenTimeSittingMax = 25.0

egsMinDamage = None
egsDamageStep = None
def printEgsConfiguration():
	totalTime = 1.0 / (meleeReferenceHitRate / 10.0)
	maxLevelDamage = (maxScore * totalTime * meleeDamageBoost) / combatTime
	damagePerLevel = (maxLevelDamage / maxCharacteristic)
	damagePerLevelFactor = damagePerLevel # * variantBoost["b"]
	damageAdd = damagePerLevel * minCharacteristic
	print("entities_game_service.cfg:")
	print("MinDamage = " + str(round(damageAdd, 3)) + ";")
	print("DamageStep = " + str(round(damagePerLevelFactor, 3)) + ";")
	egsMinDamage = round(damageAdd, 3)
	egsDamageStep = round(damagePerLevelFactor, 3)
	print("// (MaxDamage = " + str(egsMinDamage + egsDamageStep * maxLevel) + ")")
	print("")
	print("PhysicalCharacteristicsBaseValue = 0; // Additional characteristics bonus")
	scoreFactor = maxScore / maxCharacteristic
	print("PhysicalCharacteristicsFactor = " + str(scoreFactor) + ";")
	print("// (MaxPlayerBaseHP = " + str(maxScore) + ")")
	print("")
	regenTimeDiff = regenTimeMax - regenTimeMin
	regenDivisor = regenTimeDiff / maxScore * maxCharacteristic
	minRegenPerSec = minCharacteristic / regenDivisor
	wantedMinRegenPerSec = minScore / regenTimeMin
	regenOffset = wantedMinRegenPerSec - minRegenPerSec
	print("RegenDivisor = " + str(regenDivisor) + "; // Seconds per characteristic to regen a point. (Regen per second is the characteristic divided by this value plus offset)")
	print("RegenReposFactor = " + str(regenTimeMax / regenTimeSittingMax) + "; // Multiplier sitting down, offset not multiplied")
	print("RegenOffset = " + str(regenOffset) + ";  // Additional regen per second")
printEgsConfiguration()

# combat_melee
# egs_static_ai_action.cpp
# CWeaponDamageTable::getInstance().getRefenceDamage(itemQuality, attackerLevel)
#for spec in meleeSpec:
#	for variant in variantSpec:
#		
