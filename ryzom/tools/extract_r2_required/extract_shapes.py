
import os

armorPath = "R:\\pipeline\\install\\characters_shapes"
objectPath = "R:\\pipeline\\install\\objects"

with open("shape_list.txt", "w") as f:
	for p in os.listdir(armorPath):
		if p.endswith(".shape"):
			f.write(p + "\n")
	for p in os.listdir(objectPath):
		if p.endswith(".shape"):
			f.write(p + "\n")
