#!/bin/sh

# make sure the args are valid
if [ $# -lt 2 ]
then
	echo SYNTAX: $0 \<exe_name\> \[\<packed_sheet_file_name\>...\[\<packed_sheet_file_name\>...\]\]
	exit
fi

# extract the executable name (as the first arg)
EXE_NAME=$1
shift

# run through the remaining args checking that files of the given names exist
REQUIRE_REBUILD=0
while [ $# -gt 0 ]
do
  if [ ! -e $1 ]
	  then
	  REQUIRE_REBUILD=1
  fi
  shift
done

# if one of the files was missing then call the executable to rebuild the packed sheets
if [ $REQUIRE_REBUILD == 1 ]
then
	PACK_SHEETS_FLAGS=`grep PACK_SHEETS_FLAGS ../../../Variables.mk|cut -f2 -d=`
	PACK_SHEETS_FLAGS=`eval echo $PACK_SHEETS_FLAGS`
	echo "$EXE_NAME $PACK_SHEETS_FLAGS"
	$EXE_NAME $PACK_SHEETS_FLAGS
fi
