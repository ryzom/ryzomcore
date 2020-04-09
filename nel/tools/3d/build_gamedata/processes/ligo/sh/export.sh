#!/bin/bash
rm log.log 2> /dev/null

echo > log.log

# *** Export ligo zone files from Max ***
# *** ******************************* ***

echo ------- >> log.log
echo --- Export ligo zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export ligo zone
echo ------- 
date >> log.log
date

exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "ligo_export_timeout" | sed -e 's/ligo_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get current directory
dir_gamedata=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`
dir_gamedata_maxscript=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the bank filename
bank_filename=`cat ../../cfg/properties.cfg | grep "bank_name" | sed -e 's/bank_name//' | sed -e 's/ //g' | sed -e 's/=//g'  | sed -e 's/"//g' | sed -e 's/;//g'`
bankfar_filename=`cat ../../cfg/properties.cfg | grep "bankfar_name" | sed -e 's/bankfar_name//' | sed -e 's/ //g' | sed -e 's/=//g'  | sed -e 's/"//g' | sed -e 's/;//g'`

# Copy the bank file
cp $bank_filename smallbank/ligo.smallbank 2>> log.log
cp $bankfar_filename smallbank/ligo.farbank 2>> log.log

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

# delete ini file

rm $max_directory/plugcfg/nelligo.ini
# echo "$max_directory/plugcfg/nelligo.ini" >> log.log

# create the ini file

echo "[LigoConfig]" > $max_directory/plugcfg/nelligo.ini

# copy the ligo export script
cat maxscript/nel_ligo_export.ms | sed -e "s&output_directory_tag&$dir_gamedata_maxscript/processes/ligo/tag&g" | sed -e "s&output_logfile&$dir_gamedata_maxscript/processes/ligo/log.log&g" | sed -e "s&bankFilename&$dir_gamedata_maxscript/processes/ligo/smallbank/ligo.smallbank&g" > $max_directory/scripts/nel_ligo_export.ms

dir_database=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`
dir_ligosrc=`cat ../../cfg/directories.cfg | grep "ligo_source_directory" | sed -e 's/ligo_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`

echo "LigoPath=$dir_database\\$dir_ligosrc\\max\\" >> $max_directory/plugcfg/nelligo.ini
#echo "LigoExportPath=$dir_database\\$dir_ligosrc\\" >> $max_directory/plugcfg/nelligo.ini

echo "LigoExportPath=$dir_gamedata\\processes\\ligo\\" >> $max_directory/plugcfg/nelligo.ini

# export only if no .land set

land_name=`cat ../../cfg/config.cfg | grep "ligo_export_land" | sed -e 's/ligo_export_land//' | sed -e 's/ //g' | sed -e 's/=//g'`
ligo_export_one_pass=`cat ../../cfg/config.cfg | grep "ligo_export_one_pass" | sed -e 's/ligo_export_one_pass//' | sed -e 's/ //g' | sed -e 's/=//g'  | sed -e 's/"//g' | sed -e 's/;//g'`

if [ -z "$land_name" -o "$ligo_export_one_pass" == "1" ] ; then
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript nel_ligo_export.ms -q -mi -vn

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript nel_ligo_export.ms -q -mi -vn

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript nel_ligo_export.ms -q -mi -vn
fi
