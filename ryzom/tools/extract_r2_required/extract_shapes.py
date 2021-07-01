
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

sfxPath = "R:\\pipeline\\install\\sfx"

with open("sfx_shape_list.txt", "w") as fs:
	with open("sfx_ps_list.txt", "w") as fx:
		for p in os.listdir(sfxPath):
			if p.endswith(".shape"):
				fs.write(p + "\n")
			elif p.endswith(".ps"):
				fx.write(p + "\n")

