#!/bin/sh
#
# Usage: ./nelDashBuild.sh <Continuous|Experimental|Nightly> [/path/to/source]
#
# 

# Default the source directory to where this is run from.
NEL_DIR=`pwd`
BUILDTYPE="Continuous"

# Check to make sure we have at least one argument. Output usage.
if [ $# -lt 1 ];
then
  echo "Usage: $0 <Continuous|Experimental|Nightly> [/path/to/source]"
  exit;
else
  # Save the build type.
  BUILDTYPE=$1
fi

# If we have more than one argument assume the 2nd argument is the source dir.
if [ $# -gt 1 ];
then
  NEL_DIR=$2
fi

# Change to the NeL source dir for this build.
if [ -e $NEL_DIR/build ];
then
  cd $NEL_DIR/build
else
  echo "$0: Failed to change to build dir: $NEL_DIR/build"
  exit;
fi

#this is due to a bug, the process extracts information from the svn output which needs to be in english
#export LC_MESSAGES=en_GB

# Start the build
make -j -l 2.0 $1

