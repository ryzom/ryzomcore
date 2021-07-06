
import os, zlib

folder = r"R:\leveldesign\game_elem\creature\npc\3dset"
if not os.path.isdir(folder):
	os.makedirs(folder)

matrix = [
	{ "fy": "fyros", "ma": "matis", "tr": "tryker", "zo": "zorai" },
	{ "f": "female", "h": "male" },
	{ "a": "ah", "b": "am", "c": "al", "d": "ac" },
	{ 1: "", 2: "_2", 3: "_3" },
	{ "": 0, "mid": 1, "old": 2 },
]

texture = [
	"Lacustre/Low Quality/Young",
	"Desert/Medium Quality/Normal",
	"Jungle/High Quality/Old",
]

# ic t ah b _2

def randomValue(mod, seed):
	rv = zlib.crc32(seed) & 0xffffffff
	return rv % mod

for race in matrix[0]:
	for gender in matrix[1]:
		for armor in matrix[2]:
			for quality in matrix[3]:
				for age in matrix[4]:
					itemPrefix = "ic" + matrix[0][race][0] + matrix[2][armor]
					itemPrefixPants = itemPrefix
					if armor == "d":
						itemPrefix = "ic" + matrix[0][race][0] + "al"
					itemSuffix = matrix[3][quality] + ".sitem"
					itemTexture = texture[quality - 1]
					visageTexture = texture[matrix[4][age]]
					visageName = race + "_visage.sitem"
					setName = race + gender + armor + str(quality) + age + ".creature"
					hairColor = randomValue(6, "HairColor_" + setName) + 1
					eyesColor = randomValue(8, "EyesColor_" + setName) + 1
					parentName = "_" + matrix[0][race] + "_" + matrix[1][gender] + ".creature"
					with open(folder + "\\" + setName, "w") as f:
						f.write("<?xml version=\"1.0\"?>\n")
						f.write("<FORM Version=\"4.0\" State=\"modified\">\n")
						f.write("  <PARENT Filename=\"" + parentName + "\"/>\n")
						f.write("  <STRUCT>\n")
						f.write("    <STRUCT Name=\"Basics\">\n")
						f.write("      <ATOM Name=\"Fame\" Value=\"" + matrix[0][race] + "\"/>\n")
						f.write("      <STRUCT Name=\"Equipment\">\n")
						f.write("        <STRUCT Name=\"Body\">\n")
						f.write("          <ATOM Name=\"Item\" Value=\"" + itemPrefix + "v" + itemSuffix + "\"/>\n")
						f.write("          <ATOM Name=\"Texture\" Value=\"" + itemTexture + "\"/>\n")
						f.write("          <ATOM Name=\"Color\" Value=\"White\"/>\n")
						f.write("        </STRUCT>\n")
						f.write("        <STRUCT Name=\"Legs\">\n")
						f.write("          <ATOM Name=\"Item\" Value=\"" + itemPrefixPants + "p" + itemSuffix + "\"/>\n")
						f.write("          <ATOM Name=\"Texture\" Value=\"" + itemTexture + "\"/>\n")
						f.write("          <ATOM Name=\"Color\" Value=\"White\"/>\n")
						f.write("        </STRUCT>\n")
						f.write("        <STRUCT Name=\"Arms\">\n")
						f.write("          <ATOM Name=\"Item\" Value=\"" + itemPrefix + "s" + itemSuffix + "\"/>\n")
						f.write("          <ATOM Name=\"Texture\" Value=\"" + itemTexture + "\"/>\n")
						f.write("          <ATOM Name=\"Color\" Value=\"White\"/>\n")
						f.write("        </STRUCT>\n")
						f.write("        <STRUCT Name=\"Hands\">\n")
						f.write("          <ATOM Name=\"Item\" Value=\"" + itemPrefix + "g" + itemSuffix + "\"/>\n")
						f.write("          <ATOM Name=\"Texture\" Value=\"" + itemTexture + "\"/>\n")
						f.write("          <ATOM Name=\"Color\" Value=\"White\"/>\n")
						f.write("        </STRUCT>\n")
						f.write("        <STRUCT Name=\"Feet\">\n")
						f.write("          <ATOM Name=\"Item\" Value=\"" + itemPrefix + "b" + itemSuffix + "\"/>\n")
						f.write("          <ATOM Name=\"Texture\" Value=\"" + itemTexture + "\"/>\n")
						f.write("          <ATOM Name=\"Color\" Value=\"White\"/>\n")
						f.write("        </STRUCT>\n")
						f.write("        <STRUCT Name=\"Face\">\n")
						f.write("          <ATOM Name=\"Item\" Value=\"" + visageName + "\"/>\n")
						f.write("          <ATOM Name=\"Texture\" Value=\"" + visageTexture + "\"/>\n")
						f.write("          <ATOM Name=\"Color\" Value=\"Beige\"/>\n")
						f.write("        </STRUCT>\n")
						f.write("      </STRUCT>\n")
						f.write("    </STRUCT>\n")
						f.write("    <STRUCT Name=\"3d data\">\n")
						f.write("      <ATOM Name=\"HairColor\" Value=\"Color " + str(hairColor) + "\"/>\n")
						f.write("      <ATOM Name=\"EyesColor\" Value=\"Color " + str(eyesColor) + "\"/>\n")
						f.write("    </STRUCT>\n")
						f.write("  </STRUCT>\n")
						f.write("</FORM>\n")
						f.flush()
