
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

# Base time to settle a fight, assuming base damage and HP, and assuming all hits are perfectly successful
combatTime = 8.0

# Magic may do double the damage of melee
# Range is most powerful but requires ammo of course.
# Perhaps allow range without ammo at melee damage levels. Or hybrid range and magic for ammo-less range
# For NPCs, range should have melee-like damage, since there is no special protection...
# Can we turn melee/magic/range into a rock/paper/scissors?
boosts = {
	"fauna": 1.0,
	"melee": 1.0,
	"magic": 2.0,
	"magic_dot": 3.0,
	"range": 1.5, # 4.0?
}

# end of file
