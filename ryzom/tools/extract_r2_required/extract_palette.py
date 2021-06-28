
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

lf = open("sitem_list.txt", "w")
lf.flush()
for k in sitemMatches:
	lf.write(k + "\n")
lf.close()

creatureMatches = re.findall(creatureExpr, paletteLua)
creatureMap = {}
for k in creatureMatches:
	ka = k.split("\"")
	for kk in ka:
		if kk != "":
			creatureMap[kk] = True
creatureMatches = list(dict.fromkeys(creatureMap))
creatureMatches.sort()
# print(creatureMatches)

lf = open("creature_list.txt", "w")
lf.flush()
for k in creatureMatches:
	lf.write(k + "\n")
lf.close()
