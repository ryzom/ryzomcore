#!/bin/sh

HGBIN="/usr/bin/hg"
CODEROOT=../..

# download packaging stuff
echo "Updating packaging files..."
$HGBIN pull && $HGBIN update

echo "Generating changelogs..."
$HGBIN log -M --style $CODEROOT/changelog.template > $CODEROOT/changelog

REVISION=`$HGBIN identify -n`
echo "Found revision $REVISION"

REVISION_H=$CODEROOT/revision.h

# Copy revision.h template
cp $REVISION_H.in $REVISION_H

DATE=$(date "+%Y-%m-%d %H:%M:%S")

# Update revision.h with revision and build date
sed -i 's/#cmakedefine/#define/g' $REVISION_H
sed -i 's/${REVISION}/'$REVISION'/g' $REVISION_H
sed -i 's/${BUILD_DATE}/'"$DATE"'/g' $REVISION_H

VERSION=$(./ryzomcore_version.sh)

if [ -z "$VERSION" ]
then
  echo "Can't parse version from $VERSION_FILE, aborting..."
  exit 1
fi

DSTFOLDER=ryzom-core-$VERSION.$REVISION

# remove destination folder if present
rm -rf $DSTFOLDER.orig
# copy all files
echo "Copying files to $DSTFOLDER..."
cp -r -p $CODEROOT $DSTFOLDER.orig

echo "Removing web files, because we don't need them and they generate lintian errors..."
rm -rf $DSTFOLDER.orig/web

echo "Done. Now launch ./update_debian.sh <distrib>"
