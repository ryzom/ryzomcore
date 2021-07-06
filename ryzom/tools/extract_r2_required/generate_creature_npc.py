
import os, zlib

from balancing_config import *

folder = r"R:\leveldesign\game_elem\creature\npc\ring"
if not os.path.isdir(folder):
	os.makedirs(folder)

# cuthroat_b_melee_b_m_f.creature
# cuthroat_b_range_a_t_f.creature
# cuthroat_b, melee/range, a/b/c, f/m/t/z (race), h/f (gender)
# not used

# fyros_guard_l_c_f
# matis_guard_l_f_h
# fyros/matis/tryker/zorai (race), guard_l, b/c/d/e/f (level range), h/f (gender)
# not used

# ring_civil_light_melee_blunt_b4: civilian

# ring_guard_melee_tank_blunt_c3: guard

# ring_light_melee_blunt_c3: milicia, light armor
# ring_melee_damage_dealer_slash_b2: armsman, medium armor
# ring_melee_tank_blunt_d2: warrior, heavy armor

# ring_magic_damage_dealer_acid_d3: elementalist
# ring_magic_aoe_acid_c4: devastator
# ring_magic_curser_blind_d1: illusionist, tormentor

# ring_healer_b2 (b/c/d/e/f2 only)
# not used

# protections are 0 to 100, more melee protection for melee, and more magic protection for magic
# resists are all same as level, varied using level variance

types = {
	"ring_civil_light_melee":   { "action": "combat_melee",     "levelOffset":  0, "attackOffset":  0, "defenseOffset":  0, "xpGain": 1.0, "hpFactor": 0.5, "attackFactor": 0.5,  "meleeProtection": 20, "magicProtection": 20, "parent": "fyhc3.creature" }, # civilian
	"ring_guard_melee_tank":    { "action": "combat_melee",     "levelOffset": 20, "attackOffset":  0, "defenseOffset":  0, "xpGain": 2.0, "hpFactor": 4.0, "attackFactor": 2.0,  "meleeProtection": 60, "magicProtection": 60, "parent": "fyha3.creature" }, # guard

	"ring_light_melee":         { "action": "combat_melee",     "levelOffset":  0, "attackOffset":  0, "defenseOffset":  0, "xpGain": 1.0, "hpFactor": 1.0, "attackFactor": 1.0,  "meleeProtection": 20, "magicProtection": 20, "parent": "fyhc3.creature" }, # milicia
	"ring_melee_damage_dealer": { "action": "combat_melee",     "levelOffset":  0, "attackOffset":  6, "defenseOffset": -6, "xpGain": 1.5, "hpFactor": 0.5, "attackFactor": 2.0,  "meleeProtection": 40, "magicProtection": 20, "parent": "fyhb3.creature" }, # armsman
	"ring_melee_tank":          { "action": "combat_melee",     "levelOffset":  0, "attackOffset": -6, "defenseOffset":  6, "xpGain": 2.0, "hpFactor": 4.0, "attackFactor": 0.25, "meleeProtection": 60, "magicProtection": 20, "parent": "fyha3.creature" }, # warrior

	"ring_magic_damage_dealer": { "action": "magic_damage",     "levelOffset":  0, "attackOffset":  0, "defenseOffset":  0, "xpGain": 1.5, "hpFactor": 1.0 / boosts["magic"],     "meleeProtection": 20, "magicProtection": 40, "parent": "fyhd3.creature" }, # elementalist
	"ring_magic_aoe":           { "action": "magic_aoe",        "levelOffset":  0, "attackOffset":  0, "defenseOffset":  0, "xpGain": 1.0, "hpFactor": 1.0 / boosts["magic"],     "meleeProtection": 20, "magicProtection": 20, "parent": "fyhd3.creature" }, # devastator
	"ring_magic_curser":        { "action": "magic_affliction", "levelOffset":  0, "attackOffset":  0, "defenseOffset":  0, "xpGain": 2.0, "hpFactor": 1.0 / boosts["magic"],     "meleeProtection": 20, "magicProtection": 60, "parent": "fyhd3.creature" }, # illusionist, tormentor
}

xpVariance = 0.1
hpVariance = 0.1

attackTime = 3
attackTimeVariance = 0.2

attackVariance = 0.1

def randomValue(mod, seed):
	rv = zlib.crc32(seed) & 0xffffffff
	return rv % mod

def randomFloat(seed):
	rv = zlib.crc32(seed) & 0xffffffff
	return ((rv % 2000) * 1.0) / 2000.0

def varyFloat(value, variance, seed):
	rv = randomFloat(seed)
	rv = ((rv * 2.0) - 1.0) * variance
	return value + (value * rv)

def varyLevel(level, seed):
	rv = zlib.crc32("Level_" + seed + "_Level") & 0xffffffff
	vrange = levelVariance[1] - levelVariance[0]
	rv = rv % vrange
	vmin = levelVariance[0]
	rv = rv + vmin
	res = level + rv
	if res < 1:
		res = 1
	return res

def varyProtection(protection, seed):
	rv = zlib.crc32("Protection_" + seed + "_Protection") & 0xffffffff
	rv = rv % 10
	rv = rv - 5
	res = int(round(protection + (protection * rv * 0.01), 0))
	if res < 0:
		res = 0
	if res > 100:
		res = 100
	return res

def randomMagic(seed):
	rv = zlib.crc32("Magic_" + seed + "_Magic") & 0xffffffff
	rv = rv % len(specialization["magic"])
	return specialization["magic"][rv]

def writeNpcCreature(name, type, level, spec, actionlist):
	baseLevel = levels[level] + types[type]["levelOffset"]
	baseLevel = varyLevel(baseLevel, "Level_" + name)
	attackLevel = varyLevel(baseLevel + types[type]["attackOffset"], "Attack_" + name)
	defenseLevel = varyLevel(baseLevel + types[type]["defenseOffset"], "Defense_" + name)
	avgLevel = int(round((attackLevel + defenseLevel) / 2, 0))
	xpLevel = int(round((baseLevel + attackLevel + defenseLevel) / 3, 0))
	playerHpLevel = int(getScore(baseLevel) / 100.0)
	hp = round(varyFloat(getScore(baseLevel), hpVariance, "life" + name) * types[type]["hpFactor"], 0)
	regen = varyFloat(hp / regenTimeAi, hpVariance, "LifeRegen" + name)
	totalTime = round(varyFloat(attackTime, attackTimeVariance, "AttackSpeed" + name), 1)
	boost = boosts["melee"]
	if "range" in type:
		boost = boosts["range"]
	attackFactor = 1.0
	if "attackFactor" in types[type]:
		attackFactor = types[type]["attackFactor"]
	totalDamage = (getScore(baseLevel) * totalTime * boost * attackFactor) / combatTime
	totalDamage = varyFloat(totalDamage, attackVariance, "NbHitToKillPlayer" + name)
	hitsToKill = (playerHpLevel * 100.0) / totalDamage
	with open(folder + "\\" + name + ".creature", "w") as f:
		f.write("<?xml version=\"1.0\"?>\n")
		f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
		f.write("  <PARENT Filename=\"" + types[type]["parent"] + "\"/>\n")
		f.write("  <STRUCT>\n")
		f.write("    <STRUCT Name=\"Basics\">\n")
		f.write("      <ATOM Name=\"Fame\" Value=\"none\"/>\n")
		# f.write("      <ATOM Name=\"Level\" Value=\"" + str(avgLevel) + "\"/>\n")
		f.write("      <ATOM Name=\"NbPlayers\" Value=\"1\"/>\n")
		f.write("      <ATOM Name=\"PlayerHpLevel\" Value=\"" + str(playerHpLevel) + "\"/>\n")
		f.write("      <ATOM Name=\"NbHitToKillPlayer\" Value=\"" + str(hitsToKill) + "\"/>\n")
		f.write("      <STRUCT Name=\"Characteristics\">\n")
		f.write("        <ATOM Name=\"DynamicEnergyValue\" Value=\"0.002777778\"/>\n")
		f.write("      </STRUCT>\n")
		f.write("      <STRUCT Name=\"Equipment\">\n")
		f.write("        <STRUCT Name=\"Body\">\n")
		f.write("          <ATOM Name=\"Color\" Value=\"0\"/>\n")
		f.write("        </STRUCT>\n")
		f.write("        <STRUCT Name=\"Legs\">\n")
		f.write("          <ATOM Name=\"Color\" Value=\"0\"/>\n")
		f.write("        </STRUCT>\n")
		f.write("        <STRUCT Name=\"Arms\">\n")
		f.write("          <ATOM Name=\"Color\" Value=\"0\"/>\n")
		f.write("        </STRUCT>\n")
		f.write("        <STRUCT Name=\"Hands\">\n")
		f.write("          <ATOM Name=\"Color\" Value=\"0\"/>\n")
		f.write("        </STRUCT>\n")
		f.write("        <STRUCT Name=\"Feet\">\n")
		f.write("          <ATOM Name=\"Color\" Value=\"0\"/>\n")
		f.write("        </STRUCT>\n")
		f.write("        <STRUCT Name=\"Head\">\n")
		f.write("          <ATOM Name=\"Color\" Value=\"0\"/>\n")
		f.write("        </STRUCT>\n")
		f.write("      </STRUCT>\n")
		f.write("      <STRUCT Name=\"MovementSpeeds\">\n")
		f.write("        <ATOM Name=\"WalkSpeed\" Value=\"1.66\"/>\n")
		f.write("        <ATOM Name=\"RunSpeed\" Value=\"6\"/>\n")
		f.write("      </STRUCT>\n")
		f.write("      <ATOM Name=\"life\" Value=\"" + str(int(hp)) + "\"/>\n")
		f.write("      <ATOM Name=\"AttackSpeed\" Value=\"" + str(totalTime) + "\"/>\n")
		f.write("      <ATOM Name=\"LifeRegen\" Value=\"" + str(regen) + "\"/>\n")
		f.write("      <ATOM Name=\"AttackLevel\" Value=\"" + str(attackLevel) + "\"/>\n")
		f.write("      <ATOM Name=\"DefenseLevel\" Value=\"" + str(defenseLevel) + "\"/>\n")
		f.write("      <ATOM Name=\"XPLevel\" Value=\"" + str(xpLevel) + "\"/>\n")
		f.write("      <ATOM Name=\"TauntLevel\" Value=\"" + str(varyLevel(baseLevel, "TauntLevel" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"RegionForce\" Value=\"" + str(getRegionForce(level)) + "\"/>\n")
		f.write("      <ATOM Name=\"ForceLevel\" Value=\"" + str(getForceLevel(level)) + "\"/>\n") # TODO
		if "magic" in type:
			f.write("      <ATOM Name=\"DodgeAsDefense\" Value=\"true\"/>\n")
		else:
			f.write("      <ATOM Name=\"DodgeAsDefense\" Value=\"false\"/>\n")
		f.write("    </STRUCT>\n")
		f.write("    <STRUCT Name=\"3d data\">\n")
		f.write("      <ATOM Name=\"Scale\" Value=\"1\"/>\n")
		f.write("      <ATOM Name=\"ForceDisplayCreatureName\" Value=\"false\"/>\n")
		f.write("    </STRUCT>\n")
		f.write("    <STRUCT Name=\"Properties\">\n")
		f.write("      <ATOM Name=\"LootHarvestState\" Value=\"NONE\"/>\n")
		f.write("      <ATOM Name=\"XPGainCoef\" Value=\"" + str(round(varyFloat(types[type]["xpGain"], xpVariance, "XPGainCoef" + name), 2)) + "\"/>\n")
		f.write("    </STRUCT>\n")
		f.write("    <STRUCT Name=\"Protections\">\n")
		f.write("      <ATOM Name=\"PiercingFactor\" Value=\"" + str(varyProtection(types[type]["meleeProtection"], "PiercingFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"SlashingFactor\" Value=\"" + str(varyProtection(types[type]["meleeProtection"], "SlashingFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"BluntFactor\" Value=\"" + str(varyProtection(types[type]["meleeProtection"], "BluntFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"RotFactor\" Value=\"" + str(varyProtection(types[type]["magicProtection"], "RotFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"AcidFactor\" Value=\"" + str(varyProtection(types[type]["magicProtection"], "AcidFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"ColdFactor\" Value=\"" + str(varyProtection(types[type]["magicProtection"], "ColdFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"FireFactor\" Value=\"" + str(varyProtection(types[type]["magicProtection"], "FireFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"PoisonFactor\" Value=\"" + str(varyProtection(types[type]["magicProtection"], "PoisonFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"ElectricityFactor\" Value=\"" + str(varyProtection(types[type]["magicProtection"], "ElectricityFactor" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"ShockFactor\" Value=\"" + str(varyProtection(types[type]["magicProtection"], "ShockFactor" + name)) + "\"/>\n")
		f.write("    </STRUCT>\n")
		f.write("    <STRUCT Name=\"Resists\">\n")
		f.write("      <ATOM Name=\"Fear\" Value=\"" + str(varyLevel(baseLevel, "Fear" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Sleep\" Value=\"" + str(varyLevel(baseLevel, "Sleep" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Stun\" Value=\"" + str(varyLevel(baseLevel, "Stun" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Root\" Value=\"" + str(varyLevel(baseLevel, "Root" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Blind\" Value=\"" + str(varyLevel(baseLevel, "Blind" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Snare\" Value=\"" + str(varyLevel(baseLevel, "Snare" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Slow\" Value=\"" + str(varyLevel(baseLevel, "Slow" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Acid\" Value=\"" + str(varyLevel(baseLevel, "Acid" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Cold\" Value=\"" + str(varyLevel(baseLevel, "Cold" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Electricity\" Value=\"" + str(varyLevel(baseLevel, "Electricity" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Fire\" Value=\"" + str(varyLevel(baseLevel, "Fire" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Poison\" Value=\"" + str(varyLevel(baseLevel, "Poison" + name)) + "\"/>\n")
		f.write("      <ATOM Name=\"Rot\" Value=\"" + str(varyLevel(baseLevel, "Rot" + name)) + "\"/>\n")
		f.write("    </STRUCT>\n")
		if "magic" in types[type]["action"]:
			f.write("    <ATOM Name=\"action_cfg\" Value=\"0010f\"/>\n")
			f.write("    <ARRAY Name=\"nuke_cfg\">\n")
			f.write("      <ATOM Value=\"" + actionlist + "\"/>\n")
			f.write("    </ARRAY>\n")
		else:
			f.write("    <ATOM Name=\"action_cfg\" Value=\"1000f\"/>\n")
			f.write("    <ARRAY Name=\"melee_cfg\">\n")
			f.write("      <ATOM Value=\"" + actionlist + "\"/>\n")
			f.write("    </ARRAY>\n")
		f.write("    <ATOM Name=\"creature_level\" Value=\"" + str(avgLevel) + "\"/>\n")
		f.write("  </STRUCT>\n")
		f.write("</FORM>\n")
		f.flush()

for type in types:
	for level in levels:
		if "combat" in types[type]["action"]:
			for spec in specialization["melee"]:
				sn = spec
				if sn == "piercing":
					sn = "pierce"
				if sn == "slashing":
					sn = "slash"
				name = type + "_" + sn + "_" + level
				writeNpcCreature(name, type, level, spec, types[type]["action"] + ".actionlist") # TODO: Specialized melee (blunt adds chance for stun, slash adds chance for extra damage, pierce adds chance to go through armor, 1:9 action list)
		elif types[type]["action"] == "magic_affliction":
			for spec in specialization["curse"]:
				name = type + "_" + spec + "_" + level
				magic = randomMagic(name)
				writeNpcCreature(name, type, level, spec, types[type]["action"] + "_" + spec + "_" + magic + ".actionlist") # TODO: Specialized melee (blunt adds chance for stun, slash adds chance for extra damage, pierce adds chance to go through armor, 1:9 action list)
		elif "magic" in types[type]["action"]:
			for spec in specialization["magic"]:
				name = type + "_" + spec + "_" + level
				writeNpcCreature(name, type, level, spec, types[type]["action"] + "_" + spec + ".actionlist") # TODO: Specialized melee (blunt adds chance for stun, slash adds chance for extra damage, pierce adds chance to go through armor, 1:9 action list)

# end of file
