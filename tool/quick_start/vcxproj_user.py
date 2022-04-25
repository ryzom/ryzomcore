
# A little magic to generate nice .vcxproj.user files to make debugging easy peasy

import sys, os, pathlib
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

dir = sys.argv[1]
tc = NeLToolchains[sys.argv[2]]

envPath = ""
if "EnvPath" in tc and len(tc["EnvPath"]) > 0:
	for ep in tc["EnvPath"]:
		envPath = ep + os.pathsep + envPath
	envPath = "PATH=" + envPath[:-1]
# \nQT_PLUGIN_PATH=C:\2021q4_external_v143_x64\qt5\plugins

def RecurseDir(dir):
	for di in os.listdir(dir):
		if di.startswith("."):
			continue
		if "CMakeFiles" in di:
			continue
		dia = os.path.join(dir, di)
		if os.path.isfile(dia):
			if di.endswith(".vcxproj"):
				if not os.path.isfile(dia + ".user"):
					fi = open(dia, "r")
					line = fi.readline()
					app = False
					while line:
						if "<ConfigurationType>Application</ConfigurationType>" in line:
							app = True
							break
						line = fi.readline()
					fi.close()
					if app:
						# print(dia)
						workingDir = None
						din = di[:-len(".vcxproj")]
						if din in NeLConfig["WorkingDirs"]:
							workingDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["WorkingDirs"][din]))
						fo = open(dia + ".user", "w")
						fo.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
						fo.write("<Project ToolsVersion=\"Current\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n")
						fo.write("  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n")
						if workingDir:
							fo.write("    <LocalDebuggerWorkingDirectory>")
							fo.write(workingDir)
							fo.write("</LocalDebuggerWorkingDirectory>\n")
						if len(envPath) > 0:
							fo.write("    <LocalDebuggerEnvironment>")
							fo.write(envPath)
							fo.write("\n$(LocalDebuggerEnvironment)</LocalDebuggerEnvironment>\n")
						fo.write("  </PropertyGroup>\n")
						fo.write("  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\n")
						if workingDir:
							fo.write("    <LocalDebuggerWorkingDirectory>")
							fo.write(workingDir)
							fo.write("</LocalDebuggerWorkingDirectory>\n")
						if len(envPath) > 0:
							fo.write("    <LocalDebuggerEnvironment>")
							fo.write(envPath)
							fo.write("\n$(LocalDebuggerEnvironment)</LocalDebuggerEnvironment>\n")
						fo.write("  </PropertyGroup>\n")
						fo.write("</Project>\n")
						fo.close();
					# print(dia)
		elif os.path.isdir(dia):
			RecurseDir(dia)

RecurseDir(dir)

# end of file
