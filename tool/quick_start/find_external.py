
# This script detects all external library paths
# Useful for locating all dynamic libraries

from .common_config import *

import os

def FilterExternalDirs(libs, prefixPaths):
	filterMap = {
		"assimp": "include/assimp/mesh.h",
		"boost": "include/boost/algorithm/algorithm.hpp",
		"curl": "include/curl/curl.h",
		"ffmpeg": "include/libavcodec/codec.h",
		"freetype2": "include/freetype2/ft2build.h",
		"gl": "include/GL/glcorearb.h",
		"gles": "include/GLES2/gl2.h",
		"iconv": "include/iconv.h",
		"libjpeg": "include/jpeglib.h",
		"libpng": "include/png.h",
		"libxml2": "include/libxml2/libxml/xmlreader.h",
		"lua": "include/lua.h",
		"luabind": "include/luabind/luabind.hpp",
		"mariadb": "include/mariadb/mysql.h",
		"msquic": "include/msquic.h",
		"ogg": "include/ogg/ogg.h",
		"openal": "include/AL/al.h",
		"openssl": "include/openssl/opensslconf.h",
		"protobuf": "include/google/protobuf/message.h",
		"qt5": "bin/Qt5Core.dll",
		"qt5_static": "lib/cmake/Qt5Widgets/Qt5Widgets_QWindowsVistaStylePlugin_Import.cpp",
		"qt6": "bin/Qt6Core.dll",
		"qt6_static": "lib/cmake/Qt6Widgets/Qt6WidgetsPlugins.cmake",
		"squish": "include/squish.h",
		"vorbis": "include/vorbis/codec.h",
		"zlib": "include/zlib.h"
	}
	res = []
	for lib in libs:
		if lib in filterMap:
			for dir in prefixPaths:
				file = os.path.join(dir, filterMap[lib])
				if os.path.isfile(file):
					res += [ dir ]
					break
	return res

def FindQtPluginPath(prefixPaths):
	for dir in prefixPaths:
		if os.path.isfile(os.path.join(dir, "plugins/platforms/qminimal.dll")):
			return os.path.join(dir, "plugins")
	return None

# The list of folders to potentially pass to CMake as PREFIX
def FindPrefixPaths(externalDir):
	def DirHasAny(dir, includeSearch):
		for search in includeSearch:
			if os.path.isfile(os.path.join(dir, os.path.normcase(search))):
				return True
		return False
	
	tempExternal = {}
	
	# Headers to include
	includeSearch = [
		"include/assimp/mesh.h",
		"include/boost/algorithm/algorithm.hpp",
		"include/curl/curl.h",
		"include/libavcodec/codec.h",
		"include/freetype2/ft2build.h",
		"include/GL/glcorearb.h",
		"include/GLES2/gl2.h",
		"include/iconv.h",
		"include/jpeglib.h",
		"include/png.h",
		"include/libxml2/libxml/xmlreader.h",
		"include/lua.h",
		"include/luabind/luabind.hpp",
		"include/mariadb/mysql.h",
		"include/msquic.h",
		"include/ogg/ogg.h",
		"include/AL/al.h",
		"include/openssl/opensslconf.h",
		"include/google/protobuf/message.h",
		"bin/Qt5Core.dll",
		"lib/cmake/Qt5Widgets/Qt5Widgets_QWindowsVistaStylePlugin_Import.cpp",
		"bin/Qt6Core.dll",
		"lib/cmake/Qt6Widgets/Qt6WidgetsPlugins.cmake",
		"include/squish.h",
		"include/vorbis/codec.h",
		"include/zlib.h"
	]
	
	# Only search one level deep
	if DirHasAny(externalDir, includeSearch):
		tempExternal[externalDir] = True
	for di in os.listdir(externalDir):
		if di.startswith("."):
			continue
		sdi = os.path.join(externalDir, di)
		if not os.path.isdir(sdi):
			continue
		if DirHasAny(sdi, includeSearch):
			tempExternal[sdi] = True
	res = []
	for dir in tempExternal:
		res += [ dir ]
	return res

# The list of folders to potentially pass into the Visual Studio debug PATH env var
def FindBinPaths(prefixPaths):
	tempExternalBinaries = {}
	for dir in prefixPaths:
		for subDir in os.listdir(dir):
			if subDir.startswith("."):
				continue
			binDir = os.path.join(dir, subDir)
			if not os.path.isdir(binDir):
				continue
			for file in os.listdir(binDir):
				if not file.startswith(".") and (file.endswith(".exe") or file.endswith(".dll")):
					tempExternalBinaries[binDir] = True
					break
	res = []
	for dir in tempExternalBinaries:
		res += [ dir ]
	return res

def FindVSPrefixPaths(toolset, platform):
	# 2021q4_external_v143_x64
	dirExt = "_external_" + toolset + "_" + platform
	found = []
	for dir in os.listdir("C:\\"):
		if not dir.startswith(".") and dir.endswith(dirExt):
			found += [ dir ]
	found.sort(reverse = True)
	if len(found):
		return FindPrefixPaths(os.path.join("C:\\", found[0]))
	if toolset == "v143":
		return FindVSPrefixPaths("v142", platform)
	if toolset == "v142":
		return FindVSPrefixPaths("v141", platform)
	if toolset == "v141":
		return FindVSPrefixPaths("v140", platform)
	return []

def FindLuaVersion(prefixPaths):
	for dir in prefixPaths:
		luaHeader = os.path.join(dir, "include/lua.h")
		if os.path.isfile(luaHeader):
			fi = open(luaHeader)
			line = fi.readline()
			while line:
				if "LUA_VERSION_NUM" in line:
					fi.close()
					line = line.strip()
					syms = line.split()
					vers = syms[len(syms) - 1]
					return int(vers)
				line = fi.readline()
			fi.close()
	# TODO: For linux, if hunter, don't care,
	# otherwise, detect which lua version the system luabind depends on!
	return

# end of file
