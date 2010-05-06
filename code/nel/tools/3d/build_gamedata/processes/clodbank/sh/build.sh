#!/bin/bash
rm log.log 2> /dev/null

# Log error
echo ------- > log.log
echo --- Build clod : build .clodbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build clod : build .clodbank
echo ------- 
date >> log.log
date

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the lod config file in the database
clod_config_file=`cat ../../cfg/config.cfg | grep "clod_config_file" | sed -e 's/clod_config_file//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the Lod character bank name
clod_bank_file_name=`cat ../../cfg/config.cfg | grep "clod_bank_file_name" | sed -e 's/clod_bank_file_name//' | sed -e 's/ //g' | sed -e 's/=//g'`


# Execute the build
build_clod_bank.exe  cfg/local_path.cfg  $database_directory/$clod_config_file  clodbank/$clod_bank_file_name

