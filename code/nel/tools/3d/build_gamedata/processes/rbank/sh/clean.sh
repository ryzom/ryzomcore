#!/bin/bash
rm log.log 2> /dev/null


# Log error
echo >> log.log
echo ------- > log.log
echo --- Clean rbank >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Clean rbank
echo ------- 
echo 
date >> log.log
date

# Get arguments
rbank_rbank_name=`cat ../../cfg/config.cfg | grep "rbank_rbank_name" | sed -e 's/rbank_rbank_name//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Delete temp files
rm tesselation/*.[tT][eE][sS][sS][eE][lL]
rm smooth/*.[lL][rR]
rm smooth/*.[oO][cC][hH][aA][iI][nN]
rm smooth/*.[gG][rR]
rm smooth/*.[rR][bB][aA][nN][kK]
rm smooth/preproc/*.[lL][rR]
rm retrievers/*.[gG][rR]
rm retrievers/*.[rR][bB][aA][nN][kK]

