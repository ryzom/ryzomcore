#!/bin/bash
rm log.log 2> /dev/null

# *** Build shape files (.shape)

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Bin
tga_2_dds='tga2dds.exe'
build_coarse_mesh='build_coarse_mesh.exe'
lightmap_optimizer='lightmap_optimizer.exe'
build_clodtex='build_clodtex.exe'
build_shadow_skin='build_shadow_skin.exe'

# Log error
echo ------- > log.log
echo --- Build ShadowSkin shape >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build ShadowSkin shape
echo ------- 
date >> log.log
date

# build shadow skin?
do_build_shadow_skin=`cat ../../cfg/config.cfg | grep -w "build_shadow_skin" | sed -e 's/build_shadow_skin//' | sed -e 's/ //g' | sed -e 's/=//g'`
build_shadow_skin_ratio=`cat ../../cfg/config.cfg | grep "build_shadow_skin_ratio" | sed -e 's/build_shadow_skin_ratio//' | sed -e 's/ //g' | sed -e 's/=//g'`
build_shadow_skin_maxface=`cat ../../cfg/config.cfg | grep "build_shadow_skin_maxface" | sed -e 's/build_shadow_skin_maxface//' | sed -e 's/ //g' | sed -e 's/=//g'`


# if config wanted then must compute shadowSkin
if ( test "$do_build_shadow_skin" = "1" )
then
	for i in shape_not_optimized/*.[sS][hH][aA][pP][eE] ; do
		if ( test -f $i )
		then
			dest=`echo $i | sed -e 's/shape_not_optimized/shape/g'`
			# if date is newer in shape_not_optimized than in shape, compute
			if ( ! test -e $dest ) || ( test $i -nt $dest )
			then
				# NB: overwrite shape_not_optimized, because will be cloded/copied below to shapes/
				$build_shadow_skin $i $i $build_shadow_skin_ratio $build_shadow_skin_maxface
			fi
		fi
	done
fi


# Log error
echo ------- >> log.log
echo --- Build shape : Copy Shape / build CLodTex >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : Copy Shape / build CLodTex
echo ------- 
date >> log.log
date

# Get the lod config file in the database
clod_config_file=`cat ../../cfg/config.cfg | grep "clod_config_file" | sed -e 's/clod_config_file//' | sed -e 's/ //g' | sed -e 's/=//g'`

# if clod cfg is setup, build clod
if (test -f $database_directory/$clod_config_file)
then
	# build the shape with clod texture. convert from 'shape_not_optimized' to 'shape'
	$build_clodtex -d $database_directory/$clod_config_file clod shape_not_optimized shape
else
	# just copy shape_not_optimized to shape
	./sh/transfert_shape_optimize.bat
fi

# Log error
echo ------- >> log.log
echo --- Build shape : optimize lightmaps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : optimize lightmaps
echo ------- 
date >> log.log
date


# copy lightmap_not_optimized to lightmap
./sh/transfert_lightmap_optimize.bat

quality_flag=`cat ../../cfg/site.cfg | grep "build_quality" | grep "1"`

# Optimize lightmaps if any. Additionnaly, output a file indicating which lightmaps are 8 bits
$lightmap_optimizer ./lightmap ./shape ./tag ./list_lm_8bit.txt

# Convert lightmap in 16 bits mode if they are not 8 bits lightmap

echo ------- >> log.log
echo --- Build shape : convert lightmaps in 16 or 8 bits >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : convert lightmaps in 16 or 8 bits 
echo ------- 
date >> log.log
date

for i in lightmap/*.[tT][gG][aA] ; do

	if ( test -f $i )
	then
		# Destination file
		dest=`echo $i | sed -e 's/lightmap/lightmap_16_bits/g'`

		# Convert the lightmap in 16 bits mode
		if ( ! test -e $dest ) || ( test $i -nt $dest )
		then
			fileTest=`echo $i | sed -e 's&lightmap/&&g'`
			file8Bit=`cat ./list_lm_8bit.txt | grep "$fileTest"`
			if ( test "$file8Bit" = "$fileTest" )
			then
				echo "export $fileTest in 8bit format"
				$tga_2_dds $i -o $dest -a tga8 2>> log.log
			else
				echo "export $fileTest in 16bit format"
				$tga_2_dds $i -o $dest -a tga16 2>> log.log
			fi
		fi
	fi

	# Idle
	../../idle.bat
done

# Log error
echo ------- >> log.log
echo --- Build shape : build coarse meshes >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : build coarse meshes 
echo ------- 
date >> log.log
date

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get texture pathes
map_source_directories=`cat ../../cfg/directories.cfg | grep "map_source_directory" | sed -e 's/map_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the texture mul size
texture_mul_size_value=`cat ../../cfg/config.cfg | grep "texture_mul_size_value" | sed -e 's/texture_mul_size_value//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the coarse mesh texture name
coarse_mesh_texture_names=`cat ../../cfg/config.cfg | grep "coarse_mesh_texture_names" | sed -e 's/coarse_mesh_texture_names//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Copy the config file header
cat cfg/config_header.cfg | sed -e "s/texture_mul_size_value/$texture_mul_size_value/g" > cfg/config_generated.cfg

# Corse meshes for this process ?
if ( test "$coarse_mesh_texture_names" ) then

	# Add the shape directory
	echo '	"'shape_with_coarse_mesh'"', >> cfg/config_generated.cfg

	# For each texture path
	for i in $map_source_directories ; do
		
		# Add the path
		echo '	"'$database_directory/$i'"', >> cfg/config_generated.cfg

		# Idle
		../../idle.bat
	done

	# Add the shape list header
	echo '};' >> cfg/config_generated.cfg
	echo ' ' >> cfg/config_generated.cfg
	echo 'list_mesh =' >> cfg/config_generated.cfg
	echo '{' >> cfg/config_generated.cfg

	# For each shape with coarse mesh
	for i in shape_with_coarse_mesh/*.[sS][hH][aA][pP][eE]; do
		
		if ( test -f $i )
		then
			# Destination file
			src=`echo $i | sed -e 's&shape_with_coarse_mesh/&&g'`
			dest=`echo $i | sed -e 's&shape_with_coarse_mesh&shape_with_coarse_mesh_builded&g'`

			# Add the shape
			echo '	"'$src'", "'$dest'",' >> cfg/config_generated.cfg

			# Destination file
			dest=`echo $i | sed -e 's/lightmap/lightmap_16_bits/g'`
		fi

		# Idle
		../../idle.bat
	done
	echo '};' >> cfg/config_generated.cfg

	# Add output bitmap list
	echo ' ' >> cfg/config_generated.cfg
	echo 'output_textures = {' >> cfg/config_generated.cfg
	# For each shape with coarse mesh
	for i in $coarse_mesh_texture_names ; do
		# Add the path
		echo '	"shape_with_coarse_mesh/'$i'.tga"', >> cfg/config_generated.cfg
	done

	# Close the config file
	echo '};' >> cfg/config_generated.cfg

	# Execute the build
	$build_coarse_mesh cfg/config_generated.cfg 

	# Log error
	echo ------- >> log.log
	echo --- Build shape : convert coarse texture to dds without mipmaps >> log.log
	echo ------- >> log.log
	echo ------- 
	echo --- Build shape : convert coarse texture to dds without mipmaps 
	echo ------- 
	date >> log.log
	date

	# Convert the coarse texture to dds
	for i in $coarse_mesh_texture_names ; do
		if ( test -f shape_with_coarse_mesh/$i.tga )
		then
			$tga_2_dds shape_with_coarse_mesh/$i.tga -o shape_with_coarse_mesh_builded/$i.dds -a 5 2>> log.log
		fi
	done

else

	echo --- No coarse meshes texture defined >> log.log
	echo --- No coarse meshes texture defined 

fi
