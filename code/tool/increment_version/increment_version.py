#
# Copyright (c) 2015  Jan Boon <jan.boon@kaetemi.be>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import fileinput
import os

os.chdir('../../')
RootDir = os.getcwd();

MajorVersion = 0
MinorVersion = 11
PatchVersion = 2
Revision = 0

VersionString = str(MajorVersion) + "." + str(MinorVersion) + "." + str(PatchVersion)
VersionComma = str(MajorVersion) + ", " + str(MinorVersion) + ", " + str(PatchVersion) + ", " + str(Revision)

for line in fileinput.input("CMakeLists.txt", inplace = True):
	if ("#  Version" in line):
		print "#  Version: " + VersionString
	elif ("SET(NL_VERSION_MAJOR" in line):
		print "SET(NL_VERSION_MAJOR " + str(MajorVersion) + ")"
	elif ("SET(NL_VERSION_MINOR" in line):
		print "SET(NL_VERSION_MINOR " + str(MinorVersion) + ")"
	elif ("SET(NL_VERSION_PATCH" in line):
		print "SET(NL_VERSION_PATCH " + str(PatchVersion) + ")"
	else:
		print line.rstrip()

os.chdir(RootDir)
os.chdir("web/public_php/ams/templates/")

for line in fileinput.input("layout.tpl", inplace = True):
	if (" Powered by: " in line):
		print "\t\t\t{if $permission > 1}<p class=\"pull-right\">AMS " + VersionString + " Powered by: <a href=\"http://usman.it/free-responsive-admin-template\">Charisma</a></p>{/if}"
	else:
		print line.rstrip()

os.chdir(RootDir)
os.chdir("ryzom/common/src/game_share/")

for line in fileinput.input("ryzom_version.h", inplace = True):
	if (("\"v" in line) and (" \\" in line)):
		print "\t\"v" + VersionString + "\" \\"
	else:
		print line.rstrip()

os.chdir(RootDir)
os.chdir("nel/tools/3d/plugin_max/")
os.chdir("nel_export")

for line in fileinput.input("nel_export.rc", inplace = True):
	if ("FILEVERSION" in line):
		print " FILEVERSION " + VersionComma
	elif ("PRODUCTVERSION" in line):
		print " PRODUCTVERSION " + VersionComma
	elif (("FileVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"FileVersion\", \"" + VersionString + "\\0\""
	elif (("ProductVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"ProductVersion\", \"" + VersionString + "\\0\""
	else:
		print line.rstrip()

os.chdir("..")
os.chdir("nel_patch_converter")

for line in fileinput.input("nel_patch_converter.rc", inplace = True):
	if ("FILEVERSION" in line):
		print " FILEVERSION " + VersionComma
	elif ("PRODUCTVERSION" in line):
		print " PRODUCTVERSION " + VersionComma
	elif (("FileVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"FileVersion\", \"" + VersionString + "\""
	elif (("ProductVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"ProductVersion\", \"" + VersionString + "\""
	else:
		print line.rstrip()

os.chdir("..")
os.chdir("nel_patch_edit")

for line in fileinput.input("mods.rc", inplace = True):
	if ("FILEVERSION" in line):
		print " FILEVERSION " + VersionComma
	elif ("PRODUCTVERSION" in line):
		print " PRODUCTVERSION " + VersionComma
	elif (("FileVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"FileVersion\", \"" + VersionString + "\""
	elif (("ProductVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"ProductVersion\", \"" + VersionString + "\""
	else:
		print line.rstrip()

os.chdir("..")
os.chdir("nel_patch_paint")

for line in fileinput.input("nel_patch_paint.rc", inplace = True):
	if ("FILEVERSION" in line):
		print " FILEVERSION " + VersionComma
	elif ("PRODUCTVERSION" in line):
		print " PRODUCTVERSION " + VersionComma
	elif (("FileVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"FileVersion\", \"" + VersionString + "\\0\""
	elif (("ProductVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"ProductVersion\", \"" + VersionString + "\\0\""
	else:
		print line.rstrip()

os.chdir("..")
os.chdir("nel_vertex_tree_paint")

for line in fileinput.input("vertex_tree_paint.rc", inplace = True):
	if ("FILEVERSION" in line):
		print " FILEVERSION " + VersionComma
	elif ("PRODUCTVERSION" in line):
		print " PRODUCTVERSION " + VersionComma
	elif (("FileVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"FileVersion\", \"" + VersionString + "\\0\""
	elif (("ProductVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"ProductVersion\", \"" + VersionString + "\\0\""
	else:
		print line.rstrip()

os.chdir("..")
os.chdir("tile_utility")

for line in fileinput.input("tile_utility.rc", inplace = True):
	if ("FILEVERSION" in line):
		print " FILEVERSION " + VersionComma
	elif ("PRODUCTVERSION" in line):
		print " PRODUCTVERSION " + VersionComma
	elif (("FileVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"FileVersion\", \"" + VersionString + "\\0\""
	elif (("ProductVersion" in line) and ("VALUE" in line)):
		print "            VALUE \"ProductVersion\", \"" + VersionString + "\\0\""
	else:
		print line.rstrip()

# end of file
