#!/bin/bash
rm log.log 2> /dev/null

# *** Build map files (.tga / .dds)

# Bin
tga_2_dds='tga2dds.exe'

# Log error
echo ------- > log.log
echo --- Build map >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build map 
echo -------
date >> log.log
date


#*** Build panoply files (.tga / .dds), and copy the result in the tga directory

# Copy panoply containt into cache if the process as been stopped before the end of build.
echo Copy panoply into cache 
cp -u -p -R panoply/. cache 2>> log.log
echo Remove panoply directory
rm -r panoply 2>> log.log
echo Copy hlsinfo into cache 
cp -u -p -R hlsinfo/. cache 2>> log.log
echo Remove hlsinfo directory
rm -r hlsinfo 2>> log.log

mkdir panoply
mkdir hlsinfo

# Bin
panoply_maker='panoply_maker.exe'

# Log error
echo ------- > log.log
echo --- Build panoply >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build panoply
echo -------
date >> log.log
date

database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Reduce bitmap size factor
reduce_bitmap_factor=`cat ../../cfg/config.cfg | grep "reduce_bitmap_factor" | sed -e 's/reduce_bitmap_factor//' | sed -e 's/ //g' | sed -e 's/=//g'`

panoply_file_list=`cat ../../cfg/config.cfg | grep "panoply_file_list" | sed -e 's/panoply_file_list//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$panoply_file_list" ; then
	rm $panoply_file_list
	panoply_config_file=`cat ../../cfg/directories.cfg | grep "panoply_config_file" | sed -e 's/panoply_config_file//' | sed -e 's/ //g' | sed -e 's/=//g'`
	for psource in $panoply_config_file ; do
		cp $database_directory/$psource current_panoply.cfg
		echo "output_path=\"panoply\";" >> current_panoply.cfg
		echo "cache_path=\"cache\";" >> current_panoply.cfg
		panoply_maker.exe current_panoply.cfg

		# Idle
		../../idle.bat
	done
	ls panoply >> $panoply_file_list
fi

# For each directoy
for i in tga/*.[tT][gG][aA] ; do

	# Destination file
	dest=`echo $i | sed -e 's/[tT][gG][aA]/dds/' | sed -e 's/\.[tT][gG][aA]/.dds/g'`
	dds=`echo $i | sed -e 's/\.[tT][gG][aA]/.dds/g'`

	# Convert the lightmap in 16 bits mode
	if ( ! test -e $dest ) || ( test $i -nt $dest )
	then
		# Copy the dds file
		if (test -f $dds)
		then
			cp $dds $dest
		fi

		# Convert
		$tga_2_dds $i -o $dest -m -r$reduce_bitmap_factor 2>> log.log
	fi

	# Idle
	../../idle.bat
done

for i in panoply/*.[tT][gG][aA] ; do

	# Destination file
	dest=`echo $i | sed -e 's%panoply/%dds/%g' | sed -e 's/[tT][gG][aA]/dds/g'`
	dds=`echo $i | sed -e 's/\.[tT][gG][aA]/.dds/g'`

	# Convert the lightmap in 16 bits mode
	if ( ! test -e $dest ) || ( test $i -nt $dest )
	then
		# Copy the dds file
		if (test -f $dds)
		then
			cp $dds $dest
		fi

		# Convert
		$tga_2_dds $i -o $dest -m -r$reduce_bitmap_factor 2>> log.log
	fi

	# Idle
	../../idle.bat
done


# Bin
hls_bank_maker='hls_bank_maker.exe'

# build the HLSBank (if hlsInfo present, and if build wanted)
hls_bank_file_name=`cat ../../cfg/config.cfg | grep "hls_bank_file_name" | sed -e 's/hls_bank_file_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$hls_bank_file_name" ; then
	rm $hls_bank_file_name
	$hls_bank_maker hlsinfo $hls_bank_file_name
fi


# Put old panoply in cache

echo Remove the panoply cache
rm -r cache 2>> log.log

echo Rename panoply as cache
mv panoply cache 2>> log.log

echo Move hlsinfo into cache
cp -u -p -R hlsinfo/. cache 2>> log.log

echo Remove the hlsinfo
rm -r hlsinfo 2>> log.log
