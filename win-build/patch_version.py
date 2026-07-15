import os
rcRoot = os.environ['RC_ROOT']
newVersion = 1
vstr = str(newVersion).zfill(5)
vpath = rcRoot + "\\pipeline\\client_patch\\patch\\" + vstr
while os.path.exists(vpath):
	newVersion = newVersion + 1
	vstr = str(newVersion).zfill(5)
	vpath = rcRoot + "\\pipeline\\client_patch\\patch\\" + vstr
clientPatchVersion = newVersion
newVersion = 1
vstr = str(newVersion).zfill(6)
vpath = rcRoot + "\\pipeline\\bridge_server\\" + vstr
while os.path.exists(vpath):
	newVersion = newVersion + 1
	vstr = str(newVersion).zfill(6)
	vpath = rcRoot + "\\pipeline\\bridge_server\\" + vstr
serverPatchVersion = newVersion
bf = open(rcRoot + "\\build\\patch_version_set.bat", "w")
bf.write("set CLIENT_PATCH_VERSION=" + str(clientPatchVersion) + "\n")
bf.write("set SERVER_PATCH_VERSION=" + str(serverPatchVersion) + "\n")
bf.close()
