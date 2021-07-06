
# Generic action specializations
specialization = {
	"melee": [ "slashing", "piercing", "blunt" ],
	"curse": [ "blind", "fear", "madness", "root", "sleep", "slow", "snare", "stun" ],
	"magic": [ "acid", "cold", "rot", "fire", "poison", "electricity", "shockwave" ],
}

# Level and HP scores
maxLevel = 250.0
minCharacteristic = 10.0
maxCharacteristic = minCharacteristic + maxLevel
minScore = minCharacteristic * 60.0
maxScore = maxCharacteristic * 60.0

def getScore(level):
	scorePerLevel = (maxScore - minScore) / maxLevel
	return minScore + (scorePerLevel * level)

# Regen
regenTimeMin = 25.0 # in seconds, at lowest level
regenTimeMax = 100.0 # in seconds, at max level
regenTimeSittingMax = 25.0
regenTimeAi = 200.0

# Base time to settle a fight, assuming base damage and HP, and assuming all hits are perfectly successful
combatTime = 8.0

# Magic may do double the damage of melee
# Range is most powerful but requires ammo of course.
# Perhaps allow range without ammo at melee damage levels. Or hybrid range and magic for ammo-less range
# For NPCs, range should have melee-like damage, since there is no special protection...
# Can we turn magic/melee/range into a rock/paper/scissors?
boosts = {
	"fauna": 1.0,
	"melee": 1.0,
	"magic": 2.0,
	"magic_dot": 3.0,
	"range": 1.5, # 4.0?
}

newbieLevels = {
#	"a1": 1,
}

levels = {
	"b1": 10, "b2": 20, "b3": 30, "b4": 40,
	"c1": 60, "c2": 70, "c3": 80, "c4": 90,
	"d1": 110, "d2": 120, "d3": 130, "d4": 140,
	"e1": 160, "e2": 170, "e3": 180, "e4": 190,
	"f1": 210, "f2": 220, "f3": 230, "f4": 240,
}

bossLevels = {
	"b5": 50, "b6": 60, "b7": 70, "b8": 60,
	"c5": 100, "c6": 110, "c7": 120, "c8": 110,
	"d5": 150, "d6": 160, "d7": 170, "d8": 160,
	"e5": 200, "e6": 210, "e7": 220, "e8": 210,
	"f5": 250, "f6": 260, "f7": 270, "f8": 260,
}

def getRegionForce(level):
	map = { "a": 1, "b": 2, "c": 3, "d": 4 , "e": 5, "f": 6 }
	return map[level[0]]

def getForceLevel(level):
	return int(level[1])

attackOffset = {
	"d": 0, "j": 0, "f": 0, "l": 0, "p": 0, "g": 4
}

defenseOffset = {
	"d": 0, "j": 0, "f": 0, "l": 0, "p": 0, "g": -4
}

levelOffset = {
	"d": 0, "j": 0, "f": 0, "l": 0, "p": 2, "g": 2
}

levelVariance = [ -3, 3 ] # excluding upper bound, also applies to attack and defense

#oldLevels = {
#	"b": { "a": 10, "b": 30, "c": 50 },
#	"c": { "a": 60, "b": 80, "c": 100 },
#	"d": { "a": 110, "b": 130, "c": 150 },
#	"e": { "a": 160, "b": 180, "c": 200 },
#	"f": { "a": 210, "b": 230, "c": 250 },
#}

# end of file
