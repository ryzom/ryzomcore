
import os, zlib

meleeSpec = [ "slashing", "piercing", "blunt" ]
curseSpec = [ "blind", "fear", "madness", "root", "sleep", "slow", "snare", "stun" ]
magicSpec = [ "acid", "cold", "rot", "fire", "poison", "electricity", "shockwave" ]

aiActionFolder = "R:\\leveldesign\\game_elem\\creature\\npc\\bestiary\\aiaction\\generic"
if not os.path.isdir(aiActionFolder):
	os.makedirs(aiActionFolder)

base = {
	"combat": {
		"fauna": meleeSpec,
		"melee": [ "" ],
		"range": meleeSpec,
	},
	# "enchanted": { # These 1.5x the damage (regular melee plus 0.5x magic damage)
		# "melee": meleeSpec * magicSpec,
		# "range": meleeSpec * magicSpec,
	# },
	"magic": {
		"damage": magicSpec,
		"aoe": magicSpec,
		# "dot": magicSpec,
		# "heal": [ "hp", "sap", "stamina", "focus" ],
		"curse": curseSpec,
	},
	# "debuff": {
		# "skill": [ "melee", "range", "magic" ],
		# "resist": magicSpec,
	# },
}

maxLevel = 250.0
minCharacteristic = 10.0
maxCharacteristic = minCharacteristic + maxLevel
minScore = minCharacteristic * 60.0
maxScore = maxCharacteristic * 60.0

# Time to settle a fight, for base damage and HP, assuming all hits are perfectly successful
combatTime = 8.0 # 10

# Magic may do double the damage of melee
# Range is most powerful but requires ammo of course.
# Perhaps allow range without ammo at melee damage levels. Or hybrid range and magic for ammo-less range
# For NPCs, range should have melee-like damage, since there is no special protection...
# Can we turn melee/magic/range into a rock/paper/scissors?
boosts = {
	"fauna": 1.0,
	"melee": 1.0,
	"magic": 2.0,
	"range": 1.0, # 4.0?
}

# Random variance on the generated sheets
randomVariance = 0.1

# Spell time
spellBaseTime = 1.0
spellRandomBaseTime = 2.0
spellPostTime = 0.5
spellRandomPostTime = 1.0

# magic_damage
# egs_static_ai_action.cpp
# Total Magic Damage = DamageValue + (SpellPowerFactor * creature.AttackLevel)
for skill in base["magic"]:
	for spec in base["magic"][skill]:
		# for variant in variantSpec:
		name = "magic_" + skill + "_" + spec
		# randBoostFactor = zlib.crc32("SpellPowerFactor" + name) & 0xffffffff
		# randBoostFactor = (((randBoostFactor % 2000) - 1000) * randomVariance) / 1000.0
		randBoostFactor = 0.0
		# randBoostAdd = zlib.crc32("DamageValue" + name) & 0xffffffff
		# randBoostAdd = (((randBoostAdd % 2000) - 1000) * randomVariance) / 1000.0
		randBoostAdd = 0.0
		randBaseTime = zlib.crc32("CastingTime" + name) & 0xffffffff
		randBaseTime = ((randBaseTime % 1000) * spellRandomBaseTime) / 1000.0
		randPostTime = zlib.crc32("PostActionTime" + name) & 0xffffffff
		randPostTime = ((randPostTime % 1000) * spellRandomPostTime) / 1000.0
		baseTime = int((spellBaseTime + randBaseTime) * 10.0) * 0.1
		postTime = int((spellPostTime + randPostTime) * 10.0) * 0.1
		totalTime = baseTime + postTime
		maxLevelDamage = (maxScore * totalTime * boosts["magic"]) / combatTime
		damagePerLevel = maxLevelDamage / maxCharacteristic
		damagePerLevelFactor = (damagePerLevel + (damagePerLevel * randBoostFactor)) # * variantBoost[variant]
		damageAdd = (damagePerLevel + (damagePerLevel * randBoostAdd)) * minCharacteristic # * variantBaseLevel[variant]
		behaviour = "CAST_" + spec.upper()
		if behaviour == "CAST_ELECTRICITY":
			behaviour = "CAST_ELEC"
		elif behaviour == "CAST_SHOCKWAVE":
			behaviour = "CAST_SHOCK"
		elif behaviour == "CAST_MADNESS":
			behaviour = "CAST_MAD"
		elif behaviour == "CAST_SNARE":
			behaviour = "CAST_SLOW"
		damageType = spec.upper()
		if damageType == "SHOCKWAVE":
			damageType = "SHOCK"
		with open(aiActionFolder + "\\" + name + ".aiaction", "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <ATOM Name=\"type\" Value=\"DamageSpell\"/>\n")
			if skill == "aoe":
				f.write("    <ATOM Name=\"AreaType\" Value=\"Bomb\"/>\n")
				f.write("    <ATOM Name=\"BombRadius\" Value=\"4\"/>\n")
			f.write("    <ATOM Name=\"Behaviour\" Value=\"" + behaviour + "\"/>\n")
			if skill == "curse":
				effectType = spec[0].upper() + spec[1:]
				if effectType == "Sleep":
					effectType = "Mezz"
				elif effectType == "Madness":
					effectType = "MeleeMadness"
				elif effectType == "Slow":
					effectType = "SlowMelee"
				elif effectType == "Snare":
					effectType = "SlowMove"
				referenceTime = spellBaseTime + spellRandomBaseTime / 2.0 + spellPostTime + spellRandomPostTime / 2.0
				effectDuration = 10.0 * totalTime / referenceTime
				f.write("    <ATOM Name=\"EffectType\" Value=\"" + effectType + "\"/>\n")
				f.write("    <ATOM Name=\"EffectValue\" Value=\"75\"/>\n")
				f.write("    <ATOM Name=\"EffectDuration\" Value=\"" + str(round(effectDuration, 1)) + "\"/>\n")
				f.write("    <ATOM Name=\"EffectDurationType\" Value=\"Normal\"/>\n")
			else:
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
	maxLevelDamage = (maxScore * totalTime * boosts["melee"]) / combatTime
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
for skill in base["combat"]:
	for spec in base["combat"][skill]:
		name = "combat_" + skill + "_" + spec
		if spec == "":
			name = "combat_" + skill
		type = "Melee"
		if skill == "range":
			type = "Range"
		combatDamageType = spec.upper()
		behaviour = "UNKNOWN_BEHAVIOUR"
		if skill == "fauna":
			behaviour = "CREATURE_ATTACK_0"
		with open(aiActionFolder + "\\" + name + ".aiaction", "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <ATOM Name=\"type\" Value=\"" + type + "\"/>\n")
			f.write("    <ATOM Name=\"SpeedFactor\" Value=\"0\"/>\n")
			f.write("    <ATOM Name=\"DamageFactor\" Value=\"0\"/>\n")
			f.write("    <ATOM Name=\"DamageAdd\" Value=\"0\"/>\n")
			f.write("    <ATOM Name=\"ArmorAbsorptionFactor\" Value=\"1.0\"/>\n")
			f.write("    <ATOM Name=\"CombatDamageType\" Value=\"" + combatDamageType + "\"/>\n")
			f.write("    <ATOM Name=\"Critic\" Value=\"0.1\"/>\n")
			f.write("    <ATOM Name=\"AimingType\" Value=\"Random\"/>\n")
			if behaviour != "UNKNOWN_BEHAVIOUR":
				f.write("    <ATOM Name=\"Behaviour\" Value=\"" + behaviour + "\"/>\n")
			f.write("  </STRUCT>\n")
			f.write("</FORM>\n")

#for spec in meleeSpec:
#	for variant in variantSpec:
#		
