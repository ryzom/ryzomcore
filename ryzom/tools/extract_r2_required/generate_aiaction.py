
import os, zlib

from balancing_config import *

meleeSpec = specialization["melee"]
curseSpec = specialization["curse"]
magicSpec = specialization["magic"]

npcActionFolder = "R:\\leveldesign\\game_elem\\creature\\npc\\aiaction\\generic"
if not os.path.isdir(npcActionFolder):
	os.makedirs(npcActionFolder)
faunaActionFolder = "R:\\leveldesign\\game_elem\\creature\\fauna\\aiaction\\generic"
if not os.path.isdir(faunaActionFolder):
	os.makedirs(faunaActionFolder)

npcActionListFolder = "R:\\leveldesign\\game_elem\\creature\\npc\\actionlist\\generic"
if not os.path.isdir(npcActionListFolder):
	os.makedirs(npcActionListFolder)
faunaActionListFolder = "R:\\leveldesign\\game_elem\\creature\\fauna\\actionlist\\generic"
if not os.path.isdir(faunaActionListFolder):
	os.makedirs(faunaActionListFolder)

npcActionListComboFolder = "R:\\leveldesign\\game_elem\\creature\\npc\\actionlist\\combo"
if not os.path.isdir(npcActionListComboFolder):
	os.makedirs(npcActionListComboFolder)

base = {
	"combat": {
		"fauna": meleeSpec,
		"melee": [ "" ],
		"range": meleeSpec,
	},
	# "enchanted": { # These 1.5x the damage (regular melee plus 0.5x magic damage)
		# "melee": meleeSpec * magicSpec + curseSpec,
		# "range": meleeSpec * magicSpec + curseSpec,
	# },
	"magic": {
		"damage": magicSpec,
		"aoe": magicSpec,
		"dot": magicSpec,
		# "heal": [ "hp", "sap", "stamina", "focus" ],
		"curse": curseSpec,
		# affliction: curseSpec * magicSpec
	},
	# "debuff": {
		# "skill": [ "melee", "range", "magic" ],
		# "resist": magicSpec,
	# },
}

# Random variance on the generated sheets
randomVariance = 0.1

# Spell time
spellBaseTime = 1.0
spellRandomBaseTime = 2.0
spellPostTime = 0.5
spellRandomPostTime = 1.0
dotDuration = 8.0
dotRandomDuration = 8.0
dotFrequency = 2.0
dotRandomFrequency = 2.0

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
		boost = boosts["magic"]
		if "magic_" + skill in boosts:
			boost = boosts["magic_" + skill]
		maxLevelDamage = (maxScore * totalTime * boost) / combatTime
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
		with open(npcActionFolder + "\\" + name + ".aiaction", "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			if skill == "dot":
				f.write("    <ATOM Name=\"type\" Value=\"DoTSpell\"/>\n")
			else:
				f.write("    <ATOM Name=\"type\" Value=\"DamageSpell\"/>\n")
			if skill == "aoe":
				f.write("    <ATOM Name=\"AreaType\" Value=\"Bomb\"/>\n")
				f.write("    <ATOM Name=\"BombRadius\" Value=\"4\"/>\n")
			f.write("    <ATOM Name=\"Behaviour\" Value=\"" + behaviour + "\"/>\n")
			if skill == "dot":
				randDuration = zlib.crc32("Duration" + name) & 0xffffffff
				randDuration = ((randDuration % 1000) * dotRandomDuration) / 1000.0
				duration = round(dotDuration + randDuration, 1)
				randFrequency = zlib.crc32("UpdateFrequency" + name) & 0xffffffff
				randFrequency = ((randFrequency % 1000) * dotRandomFrequency) / 1000.0
				frequency = round(dotFrequency + randFrequency, 1)
				f.write("    <ATOM Name=\"Duration\" Value=\"" + str(duration) + "\"/>\n")
				f.write("    <ATOM Name=\"DurationType\" Value=\"Normal\"/>\n")
				f.write("    <ATOM Name=\"UpdateFrequency\" Value=\"" + str(frequency) + "\"/>\n")
				nbImpacts = int(duration / frequency) * 1.0
				damagePerLevelFactor /= nbImpacts
				damageAdd /= nbImpacts
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
		with open(npcActionListFolder + "\\" + name + ".actionlist", "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <ARRAY Name=\"actions\">\n")
			f.write("      <ATOM Value=\"" + name + ".aiaction\"/>\n")
			f.write("    </ARRAY>\n")
			f.write("  </STRUCT>\n")
			f.write("</FORM>\n")

for curse in curseSpec:
	for dot in magicSpec:
		name = "magic_affliction_" + curse + "_" + dot
		curseName = "magic_curse_" + curse
		dotName = "magic_dot_" + dot
		with open(npcActionListComboFolder + "\\" + name + ".actionlist", "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <ARRAY Name=\"actions\">\n")
			f.write("      <ATOM Value=\"" + curseName + ".aiaction\"/>\n")
			f.write("      <ATOM Value=\"" + dotName + ".aiaction\"/>\n")
			f.write("    </ARRAY>\n")
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
		folder = npcActionFolder
		if skill == "fauna":
			folder = faunaActionFolder
		with open(folder + "\\" + name + ".aiaction", "w") as f:
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
		folder = npcActionListFolder
		if skill == "fauna":
			folder = faunaActionListFolder
		with open(folder + "\\" + name + ".actionlist", "w") as f:
			f.write("<?xml version=\"1.0\"?>\n")
			f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
			f.write("  <STRUCT>\n")
			f.write("    <ARRAY Name=\"actions\">\n")
			f.write("      <ATOM Value=\"" + name + ".aiaction\"/>\n")
			f.write("    </ARRAY>\n")
			f.write("  </STRUCT>\n")
			f.write("</FORM>\n")

#for spec in meleeSpec:
#	for variant in variantSpec:
#		
