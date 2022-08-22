import os, sys

src = "/home/ulukyn/repos/ryzom-core/ryzom/server/src/"
dst = "/home/ulukyn/repos/ryzom-server/src/"

os.chdir(src)
i = 0
for root, dir, files in os.walk("."):
	for item in files:
		found = False
		print(item[-4:])
		if item[-4:] == ".cpp" or item[-2:] == ".h":
			i = i + 1
			print(src+root[2:]+"/"+item)
			header = []
			with open(src+root[2:]+"/"+item) as f:
				content = f.read()
				if "kaetemi" in content:
					found = True
				for line in content.split("\n"):
					if line[:2] == "//":
						print(line)
						header.append(line)
					else:
						break

			final = header
			with open(dst+root[2:]+"/"+item) as f:
				state = 1
				for line in f.read().split("\n"):
					if state == 1 and line[:2] == "//":
						continue
					else:
						state = 2

					if state == 2:
						final.append(line)

			with open(dst+root[2:]+"/"+item, "w") as f:
				f.write("\n".join(final))
	print()
