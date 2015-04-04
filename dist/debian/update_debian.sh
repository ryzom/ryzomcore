#!/bin/sh

HGBIN="/usr/bin/hg"
DCHBIN="/usr/bin/dch"

if [ ! -e $DCHBIN ]
then
  apt-get install devscripts debhelper
fi

DISTRIB=$1
MINORDISTRIB=$2

if [ -z "$DISTRIB" ]
then
  echo "You must specify a distribution"
  exit 1
fi

if [ -z "$MINORDISTRIB" ]
then
  MINORDISTRIB=1
fi

if [ ! -d "$DISTRIB" ]
then
  echo "$DISTRIB is not supported, you can create the folder or compile for another version."
  exit 1
fi

echo "Targetting $DISTRIB..."

VERSION=$(./ryzomcore_version.sh)

if [ -z "$VERSION" ]
then
  echo "Can't parse version from $VERSION_FILE, aborting..."
  exit 1
fi

REVISION=`$HGBIN identify -n`

DSTFOLDER=ryzom-core-$VERSION.$REVISION

if [ ! -d "$DSTFOLDER.orig" ]
then
  echo "$DSTFOLDER.orig doesn't exist, did you forget to launch ./update.sh?"
  exit 1
fi

# copy files if directory doesn't exist
if [ ! -d $DSTFOLDER ]
then
  # copy all files
  echo "Copying files to $DSTFOLDER..."
  cp -r -p $DSTFOLDER.orig $DSTFOLDER
fi

cd $DSTFOLDER

echo "Copying debian directory..."
# delete debian directory if present
rm -rf debian
# create debian folder
mkdir -p debian
# copy debian folder
cp -r -p ../$DISTRIB/debian .

# returning the line with the version
LAST_VERSION=`grep $VERSION.$REVISION debian/changelog`
FULL_VERSION=$VERSION.$REVISION-1~$DISTRIB$MINORDISTRIB

# adding the new version to changelog
if [ -z "$LAST_VERSION" ]
then
  echo "Adding $FULL_VERSION to debian/changelog for $DISTRIB"
  $DCHBIN --force-distribution -b -v $FULL_VERSION -D $DISTRIB "New upstream release (revision $REVISION)"
else
  echo "Last version is $LAST_VERSION"
fi

echo "Creating source package..."
debuild -S

cd ..

echo "Done."
echo "If you want to upload source to your PPA, type: dput <ppa> ryzom-core_"$FULL_VERSION"_source.changes"
