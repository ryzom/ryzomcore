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
# we have to uncompress them to the right location

# client_default.cfg and ryzom.icns are already in the right location

# uncompress Ryzom
if [ -e "$ROOTPATH/Ryzom.zip" ]
then
  unzip -o "$ROOTPATH/Ryzom.zip" -d "$CONTENTSPATH/.."
fi

# only uncompress Ryzom Installer if found in parent directory
if [ -e "$ROOTPATH/RyzomInstaller.zip" ] && [ -d "$CONTENTSPATH/../../Ryzom Installer.app" ]
then
  rm -rf "$CONTENTSPATH/../../Ryzom Installer.app"
  unzip -o "$ROOTPATH/RyzomInstaller.zip" -d "$CONTENTSPATH/../.."
fi

exit 0
