
# A little magic to generate nice .vcxproj.user files to make debugging easy peasy

import sys, os, pathlib, re
from xml.sax.saxutils import escape
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

dir = sys.argv[1]
tc = NeLToolchains[sys.argv[2]]

envPath = os.getenv("RC_PREFIX_BIN")
if envPath:
	envPath = "PATH=" + envPath + ";%PATH%"
else:
	envPath = ""
qtPluginDir = os.getenv("RC_QT_PLUGIN_DIR")
if qtPluginDir and len(qtPluginDir) > 0:
	if len(envPath) > 0:
		envPath = envPath + "\n"
	envPath = envPath + "QT_PLUGIN_PATH=" + qtPluginDir

# The options the build tree was configured with, from its CMake cache
def ReadCMakeCache(dir):
	cachePath = os.path.join(dir, "CMakeCache.txt")
	if not os.path.isfile(cachePath):
		sys.exit("Error: " + cachePath + " does not exist: configure the build tree " + dir + " with CMake before writing its .vcxproj.user files")
	values = {}
	fi = open(cachePath, "r", encoding="utf-8", errors="replace")
	for line in fi:
		m = re.match(r"([A-Za-z0-9_]+):[A-Za-z]+=(.*)$", line.rstrip("\r\n"))
		if m:
			values[m.group(1)] = m.group(2)
	fi.close()
	return values

def CMakeBool(value):
	return value.upper() in ("ON", "YES", "TRUE", "Y", "1")

# The debugger arguments are those of a development client (its control endpoint, --ctrl-listen): a final build has
# none of that unless it is built with WITH_RYZOM_CLIENT_CTRL, so a final build tree gets them only then. An option
# missing from the cache has its default of CMakeModules/nel.cmake.
cmakeCache = ReadCMakeCache(dir)
withDebuggerArguments = not CMakeBool(cmakeCache.get("FINAL_VERSION", "ON")) or CMakeBool(cmakeCache.get("WITH_RYZOM_CLIENT_CTRL", "OFF"))

# The command line arguments of the debugger for a project (DebuggerArguments of the configuration), such as the
# control endpoint of the client
def DebuggerArguments(din):
	if not withDebuggerArguments:
		return ""
	return NeLConfig.get("DebuggerArguments", {}).get(din, "")

# The notes about .user files left as they are, each given once for a build tree: the ones given are kept in the tree,
# which the configure step erases with them
notesPath = os.path.join(dir, "vcxproj_user_notes.txt")
notesGiven = set()
if os.path.isfile(notesPath):
	fi = open(notesPath, "r", encoding="utf-8")
	notesGiven = set(fi.read().splitlines())
	fi.close()

def Note(text):
	if text in notesGiven:
		return
	print("Note: " + text)
	notesGiven.add(text)
	fo = open(notesPath, "a", encoding="utf-8")
	fo.write(text + "\n")
	fo.close()

# A .user file written before a project had debugger arguments gets them, unless it has its own: the file is the
# user's then, and is left as it is
def AddDebuggerArguments(userPath, arguments):
	fi = open(userPath, "r")
	content = fi.read()
	fi.close()
	if "<LocalDebuggerCommandArguments>" in content:
		if not arguments in content:
			Note(userPath + " has debugger arguments of its own, without " + arguments)
		return
	line = "    <LocalDebuggerCommandArguments>" + escape(arguments) + "</LocalDebuggerCommandArguments>\n"
	updated, count = re.subn(r"(<PropertyGroup Condition=\"[^\"]*\">\n)", lambda m: m.group(1) + line, content)
	if count == 0:
		Note(userPath + " has no configuration to add the debugger arguments " + arguments + " to")
		return
	fo = open(userPath, "w")
	fo.write(updated)
	fo.close()
	print("Added the debugger arguments " + arguments + " to " + userPath)

def RecurseDir(dir):
	for di in os.listdir(dir):
		if di.startswith("."):
			continue
		if "CMakeFiles" in di:
			continue
		dia = os.path.join(dir, di)
		if os.path.isfile(dia):
			if di.endswith(".vcxproj"):
				arguments = DebuggerArguments(di[:-len(".vcxproj")])
				if arguments and os.path.isfile(dia + ".user"):
					AddDebuggerArguments(dia + ".user", arguments)
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
						if arguments:
							fo.write("    <LocalDebuggerCommandArguments>" + escape(arguments) + "</LocalDebuggerCommandArguments>\n")
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
						if arguments:
							fo.write("    <LocalDebuggerCommandArguments>" + escape(arguments) + "</LocalDebuggerCommandArguments>\n")
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
