
import re

paletteLuaFile = "R:\\code\\ryzom\\common\\data_common\\r2\\r2_palette.lua"
sitemExpr = r"[^\"]+.sitem"
creatureExpr = r"\"[^\"]+.creature\""

pf = open(paletteLuaFile, "r")
paletteLua = pf.read()
pf.close()

sitemMatches = re.findall(sitemExpr, paletteLua)
sitemMap = {}
for k in sitemMatches:
	ka = k.split(":")
	for kk in ka:
		sitemMap[kk] = True
sitemMatches = list(dict.fromkeys(sitemMap))
sitemMatches.sort()
# print(sitemMatches)

lf = open("sitem_list_r2.txt", "w")
lf.flush()
for k in sitemMatches:
	lf.write(k + "\n")
lf.close()

import merge_sitem_list

meleeSpec = [ "blunt", "pierce", "slash" ]
curserSpec = [ "blind", "fear", "madness", "root", "sleep", "slow", "snare", "stun" ]
magicSpec = [ "acid", "cold", "electricity", "fire", "poison", "rot", "shockwave" ]

creatureMatches = re.findall(creatureExpr, paletteLua)
creatureMap = {}
for k in creatureMatches:
	ka = k.split("\"")
	for kk in ka:
		if kk != "":
			if "$level" in kk:
				for i in range(1, 5):
					k2 = kk.replace("$level", str(i))
					if "$hands" in k2:
						if "_melee_" in k2:
							for s in meleeSpec:
								k3 = k2.replace("$hands", s)
								creatureMap[k3] = True
						elif "_curser_" in k2:
							for s in curserSpec:
								k3 = k2.replace("$hands", s)
								creatureMap[k3] = True
						elif "_magic_" in k2:
							for s in magicSpec:
								k3 = k2.replace("$hands", s)
								creatureMap[k3] = True
						else:
							creatureMap[k2] = True
					else:
						creatureMap[k2] = True
			else:
				creatureMap[kk] = True
creatureMatches = list(dict.fromkeys(creatureMap))
creatureMatches.sort()
# print(creatureMatches)

lf = open("creature_list_r2.txt", "w")
lf.flush()
for k in creatureMatches:
	lf.write(k + "\n")
lf.close()

import merge_creature_list
