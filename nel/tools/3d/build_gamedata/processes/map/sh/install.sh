#!/bin/bash
rm log.log 2> /dev/null

# Install maps in the client data

# Get the skel install directory
bitmap_install_directory=`cat ../../cfg/directories.cfg | grep "bitmap_install_directory" | sed -e 's/bitmap_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install maps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install maps
echo ------- 
date >> log.log
date

cp -u -p -R dds/. $client_directory/$bitmap_install_directory 2>> log.log
cp -u -p -R tga_uncompressed/. $client_directory/$bitmap_install_directory 2>> log.log

panoply_file_list=`cat ../../cfg/config.cfg | grep "panoply_file_list" | sed -e 's/panoply_file_list//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$panoply_file_list" ; then
	cp -u -p $panoply_file_list $client_directory/$bitmap_install_directory 2>> log.log	
	panoply_config_file=`cat ../../cfg/directories.cfg | grep "panoply_config_file" | sed -e 's/panoply_config_file//' | sed -e 's/ //g' | sed -e 's/=//g'`
	for psource in $panoply_config_file ; do
		cp $database_directory/$psource $client_directory/$bitmap_install_directory/panoply.cfg
	done
	ls panoply >> $panoply_file_list
fi


# Install hlsbank in the client data, in the "maps/" directory

# Log error
echo --- Install hlsbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install hlsbank 
echo ------- 
date >> log.log
date

# build the HLSBank (if hlsInfo present, and if build wanted)
hls_bank_file_name=`cat ../../cfg/config.cfg | grep "hls_bank_file_name" | sed -e 's/hls_bank_file_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$hls_bank_file_name" ; then
	cp -u -p -R  $hls_bank_file_name $client_directory/$bitmap_install_directory  2>> log.log
fi

