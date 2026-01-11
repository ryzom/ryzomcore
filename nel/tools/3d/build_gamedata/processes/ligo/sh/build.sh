#!/bin/bash
rm log.log 2> /dev/null

echo > log.log

# ********************
# Make the config file
# ********************

exec_timeout='exec_timeout.exe'

# Get the timeout
land_exporter_timeout=`cat ../../cfg/config.cfg | grep "ligo_build_timeout" | sed -e 's/ligo_build_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

rm land_exporter.cfg
echo "// land_exporter.cfg" > land_exporter.cfg

# OutZoneDir is Where to put all .zone generated

#dir_gamedata=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
#echo "OutZoneDir = \"$dir_gamedata/processes/ligo/output\";" >> land_exporter.cfg
echo "OutZoneDir = \"output\";" >> land_exporter.cfg
echo "OutIGDir = \"../ig/ig_land_ligo\";" >> land_exporter.cfg
echo "AdditionnalIGOutDir = \"../ig/ig_other\";" >> land_exporter.cfg

# RefZoneDir is Where the reference zones are

dir_database=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
dir_ligosrc=`cat ../../cfg/directories.cfg | grep "ligo_source_directory" | sed -e 's/ligo_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
dir_ligobricks=`cat ../../cfg/directories.cfg | grep "ligo_bricks_directory" | sed -e 's/ligo_bricks_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
dir_dfn=`cat ../../cfg/site.cfg | grep "level_design_dfn_directory" | sed -e 's/level_design_dfn_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
continent_file=`cat ../../cfg/config.cfg | grep "continent_file" | sed -e 's/continent_file//' | sed -e 's/ //g' | sed -e 's/=//g'`
dir_world=`cat ../../cfg/site.cfg | grep "level_design_world_directory" | sed -e 's/level_design_world_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
# dir_continents=`cat ../../cfg/site.cfg | grep "continents_directory" | sed -e 's/continents_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`






echo "RefZoneDir = \"$dir_ligobricks/zones\";" >> land_exporter.cfg
echo "RefIGDir = \"$dir_ligobricks/igs\";" >> land_exporter.cfg
echo "AdditionnalIGInDir = \"$dir_ligobricks/igs\";" >> land_exporter.cfg
# echo "ContinentsDir = \"$dir_continents\";" >> land_exporter.cfg
echo "ContinentsDir = \"$dir_world\";" >> land_exporter.cfg




# LigoBankDir is Where all .ligozone are (those used by the .land)

echo "LigoBankDir = \"$dir_ligobricks/zoneligos\";" >> land_exporter.cfg

# TileBankFile is the .bank file (used to know if a tile is oriented and the like)

name_bank=`cat ../../cfg/properties.cfg | grep "bank_name" | sed -e 's/bank_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "TileBankFile = $name_bank" >> land_exporter.cfg

# ColorMapFile

cmf=`cat ../../cfg/config.cfg | grep "ligo_export_colormap" | sed -e 's/ligo_export_colormap//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "ColorMapFile = \"$dir_database/$dir_ligosrc/$cmf\";" >> land_exporter.cfg

# HeightMapFile1 is the grayscale .tga file (127 is 0, 0 is -127*ZFactor and 255 is +128*ZFactor)

hmf1=`cat ../../cfg/config.cfg | grep "ligo_export_heightmap1" | sed -e 's/ligo_export_heightmap1//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "HeightMapFile1 = \"$dir_database/$dir_ligosrc/$hmf1\";" >> land_exporter.cfg

# ZFactor1 is the heightmap factor

zf1=`cat ../../cfg/config.cfg | grep "ligo_export_zfactor1" | sed -e 's/ligo_export_zfactor1//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "ZFactor1 = $zf1;" >> land_exporter.cfg

# HeightMapFile2 is the grayscale .tga file (127 is 0, 0 is -127*ZFactor and 255 is +128*ZFactor)

hmf2=`cat ../../cfg/config.cfg | grep "ligo_export_heightmap2" | sed -e 's/ligo_export_heightmap2//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "HeightMapFile2 = \"$dir_database/$dir_ligosrc/$hmf2\";" >> land_exporter.cfg

# ZFactor2 is the heightmap factor

zf2=`cat ../../cfg/config.cfg | grep "ligo_export_zfactor2" | sed -e 's/ligo_export_zfactor2//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "ZFactor2 = $zf2;" >> land_exporter.cfg

# ZoneLight is Roughly light the zone (0-none, 1-patch, 2-noise)

echo "ZoneLight = 0;" >> land_exporter.cfg

# CellSize is the size of the cell (zone size) in meter

echo "CellSize = 160;" >> land_exporter.cfg

# Threshild is the weld threshold in meter

echo "Threshold = 1;" >> land_exporter.cfg

# Where to take dfn files
echo "DFNDir = \"$dir_dfn\";" >> land_exporter.cfg

# CMB input directory
echo "RefCMBDir = \"$dir_ligobricks/cmb\";" >> land_exporter.cfg

# CMB output directory
echo "OutCMBDir = \"../rbank/cmb\";" >> land_exporter.cfg

#input .continent file
echo "ContinentFile = \"$dir_world/$continent_file\";" >> land_exporter.cfg

# Force export of collisions and additionnal igs
echo "ExportCollisions = 1;" >> land_exporter.cfg
echo "ExportAdditionnalIGs = 1;" >> land_exporter.cfg



# ZoneRegionFile is the .land to compute

dir_current=`pwd`

land_name=`cat ../../cfg/config.cfg | grep "ligo_export_land" | sed -e 's/ligo_export_land//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "ZoneRegionFile = \"$dir_database/$dir_ligosrc/$land_name\";" >> land_exporter.cfg

# if there is no .land then do not generate all zones and zone welded

if test -z "$land_name" ; then
	echo No .land set -- Exiting --
	exit;
fi

# *******************
# launch the exporter
# *******************

echo ------- >> log.log
echo --- Generate ligo zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Generate ligo zone
echo ------- 
date >> log.log
date

echo Exporting
$exec_timeout $land_exporter_timeout land_export.exe land_exporter.cfg

# rename *.[zZ][oO][nN][eE][lL] *.[zZ][oO][nN][eE]
# script is just too slow to do renaming... And we can't call directly dos command

cd output
../sh/renametozone.bat
cd ..

# **************
# Copy the zones
# **************

cd ../zone
mkdir zone_exported 2> /dev/null

# Try to copy ligo zone if any
# ****************************

ligo_flag=`cat ../../cfg/config.cfg | grep "process_to_complete" | grep "ligo"`

dir_current=`pwd`
cd ../ligo/output
list_zone=`ls -1 *.[zZ][oO][nN][eE]*`
for filename in $list_zone ; do
	echo "Checking $filename for update"
	if test -e ../../zone/zone_exported/$filename ; then
		must_update=`diff --binary -q $filename ../../zone/zone_exported/$filename` ;
	else
		must_update=YES ;
	fi
	
	if test -n "$must_update" ; then
		echo "   Updating"
		cp -u -p $filename ../../zone/zone_exported/$filename ;
	fi

	# Idle
	../../../idle.bat
done
cd $dir_current

# delete files only present in the zone_exported directory

if ( test "$ligo_flag" )
then
	cd ./zone_exported
	list_zone=`ls -1 *.[zZ][oO][nN][eE]*`
	for filename in $list_zone ; do
		if test -e ../../ligo/output/$filename ; then
			must_update=NO ;
		else
			echo "Removing $filename"
			rm $filename ;
		fi

		# Idle
		../../../idle.bat
	done
	cd ..
fi
