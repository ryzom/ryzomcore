#!/bin/sh

if [ -z "$ROOTPATH" ]
then
  echo "upgd_nl.sh can only be launched from updt_nl.sh"
  exit 1
fi

# determine directory where all files reside
CONTENTSPATH=$(dirname "$ROOTPATH")
MACOSPATH=$(dirname "$RYZOM_CLIENT")
SIGNPATH=$CONTENTSPATH/_CodeSignature

# all files of original Bundle are in the same directory
# we have to copy them to the right location

# client_default.cfg and ryzom.icns are already in the right location

# PkgInfo usually doesn't change so don't copy it

# Info.plist contains updated version
cp -p "$ROOTPATH/Info.plist" "$CONTENTSPATH"

cp -p "$ROOTPATH/CodeResources" "$SIGNPATH"

# executable flag for all executables
chmod +x "$ROOTPATH/Ryzom"
chmod +x "$ROOTPATH/CrashReport"
chmod +x "$ROOTPATH/RyzomClientPatcher"
chmod +x "$ROOTPATH/RyzomConfiguration"

# remove previous executables
rm -f "$MACOSPATH/Ryzom"
rm -f "$MACOSPATH/CrashReport"
rm -f "$MACOSPATH/RyzomClientPatcher"
rm -f "$MACOSPATH/RyzomConfiguration"

# copy all binaries in MacOS directory
cp -p "$ROOTPATH/Ryzom" "$MACOSPATH"
cp -p "$ROOTPATH/CrashReport" "$MACOSPATH"
cp -p "$ROOTPATH/RyzomClientPatcher" "$MACOSPATH"
cp -p "$ROOTPATH/RyzomConfiguration" "$MACOSPATH"

exit 0
