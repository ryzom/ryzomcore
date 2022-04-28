
# This generates the pipeline exedll projects

import sys, os, pathlib
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

workspaceDir = os.path.join(NeLConfigDir, "workspace")
commonDir = os.path.join(workspaceDir, "common")
pathlib.Path(commonDir).mkdir(parents=True, exist_ok=True)

def isWindows(client):
	tcn = NeLTargets["Client"][client]["Toolchain"]
	tc = NeLToolchains[tcn]
	return "OS" in tc and tc["OS"].startswith("Win")

# Generate the projects file, this is imported after the user's projects.py
fo = open(os.path.join(workspaceDir, "projects_exedll.py"), "w")
fo.write("#!/usr/bin/python\n")
fo.write("\n")
fo.write("from projects import *\n")
fo.write("\n")
for client in NeLTargets["Client"]:
	clientWindows = isWindows(client)
	fo.write("ProjectsToProcess += [ \"common/exedll_" + client + "\" ]\n")
	fo.write("\n")
	fo.write("ICMainExedllPlatform = { }\n")
	fo.write("ICMainExedllPlatform[\"Name\"] = \"main_exedll_" + client + "\"\n")
	fo.write("ICMainExedllPlatform[\"UnpackTo\"] = \"\"\n")
	fo.write("ICMainExedllPlatform[\"IsOptional\"] = 0\n")
	fo.write("ICMainExedllPlatform[\"IsIncremental\"] = 1\n")
	fo.write("ICMainExedllPlatform[\"StreamedPackages\"] = False\n")
	fo.write("ICMainExedllPlatform[\"Packages\"] = [ ]\n")
	fo.write("ICMainExedllPlatform[\"Packages\"] += [ [ \"exedll_" + client + "\", [ ] ] ]\n")
	if clientWindows:
		fo.write("ICMainExedllPlatform[\"Packages\"] += [ [ \"exedll_" + client + "_lib\", [ ] ] ]\n")
	fo.write("ICMainExedllPlatform[\"Refs\"] = [ ]\n")
	fo.write("InstallClientData += [ ICMainExedllPlatform ]\n")
	fo.write("del ICMainExedllPlatform\n")
	fo.write("\n")
fo.write("# end of file\n")
fo.close()

# end of file
