#!/bin/sh

CODEROOT=../../code

VERSION_FILE=$CODEROOT/CMakeLists.txt

if [ ! -f $VERSION_FILE ]
then
  echo "Unable to find $VERSION_FILE"
  exit 1
fi

parse_version()
{
  FILE=$1
  VAR=$2

  V=$(grep -o -P "NL_$VAR [0-9]+" $FILE | awk '{print $2}' | head -n 1)

  if [ -z "$V" ]
  then
    echo "Can't parse $VAR from $FILE, aborting..."
    exit 1
  fi

  export $VAR=$V
}

parse_version $VERSION_FILE VERSION_MAJOR
parse_version $VERSION_FILE VERSION_MINOR
parse_version $VERSION_FILE VERSION_PATCH

VERSION=$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH

echo $VERSION

exit 0
